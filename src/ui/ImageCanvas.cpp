#include "ui/ImageCanvas.h"

#include "model/AnnotationDocument.h"
#include "tools/Tool.h"
#include "ui/AnnotationGraphicsItem.h"
#include "ui/BrushCursorItem.h"

#include <QCursor>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QSet>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace {

constexpr qreal ZoomStep = 1.25;
constexpr qreal MinScale = 0.02;
constexpr qreal MaxScale = 64.0;

} // namespace

ImageCanvas::ImageCanvas(QWidget *parent)
    : QGraphicsView(parent),
      scene_(new QGraphicsScene(this)),
      pixmapItem_(new QGraphicsPixmapItem)
{
    pixmapItem_->setZValue(-1);
    // Без сглаживания при увеличении видно отдельные пиксели, так удобнее размечать.
    pixmapItem_->setTransformationMode(Qt::FastTransformation);
    scene_->addItem(pixmapItem_);

    setScene(scene_);
    setBackgroundBrush(QColor(60, 60, 60));
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    // Перерисовываются только изменившиеся участки, а не весь холст с картинкой.
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(false);
    viewport()->setAcceptDrops(false);
}

ImageCanvas::~ImageCanvas() = default;

void ImageCanvas::setImage(const QImage &image)
{
    if (tool_)
        tool_->cancel();
    pixmapItem_->setPixmap(QPixmap::fromImage(image));
    scene_->setSceneRect(QRectF(QPointF(0, 0), image.size()));
    zoomToFit();
    updateCursor();
}

bool ImageCanvas::hasImage() const
{
    return !pixmapItem_->pixmap().isNull();
}

QSize ImageCanvas::imageSize() const
{
    return pixmapItem_->pixmap().size();
}

QRectF ImageCanvas::imageRect() const
{
    return QRectF(QPointF(0, 0), imageSize());
}

void ImageCanvas::setDocument(AnnotationDocument *document)
{
    if (document_)
        disconnect(document_, nullptr, this, nullptr);
    document_ = document;
    if (document_) {
        connect(document_, &AnnotationDocument::annotationAdded, this, &ImageCanvas::addItem);
        connect(document_, &AnnotationDocument::annotationRemoved, this, &ImageCanvas::removeItem);
        connect(document_, &AnnotationDocument::annotationChanged, this, &ImageCanvas::updateItem);
        connect(document_, &AnnotationDocument::documentReset, this, &ImageCanvas::rebuildItems);
    }
    rebuildItems();
}

void ImageCanvas::setTool(Tool *tool)
{
    if (tool_ == tool)
        return;
    if (tool_)
        tool_->deactivate();
    tool_ = tool;
    resizingBrush_ = false;
    updateCursor();
    updateToolHover();
}

qreal ImageCanvas::viewScale() const
{
    return transform().m11();
}

AnnotationGraphicsItem *ImageCanvas::itemFor(int id) const
{
    return items_.value(id, nullptr);
}

void ImageCanvas::setSelectedIds(const QList<int> &ids)
{
    QList<int> valid;
    QSet<int> seen;
    for (int id : ids) {
        if (itemFor(id) && !seen.contains(id)) {
            seen.insert(id);
            valid.append(id);
        }
    }
    if (valid == selectedIds_)
        return;

    for (int id : qAsConst(selectedIds_)) {
        if (AnnotationGraphicsItem *item = itemFor(id))
            item->setHighlighted(false);
    }
    selectedIds_ = valid;
    for (int id : qAsConst(selectedIds_))
        itemFor(id)->setHighlighted(true);
    emit selectionChanged(selectedIds_);
}

void ImageCanvas::setSelectedId(int id)
{
    setSelectedIds(id ? QList<int>{id} : QList<int>());
}

void ImageCanvas::ensureAnnotationVisible(int id)
{
    if (AnnotationGraphicsItem *item = itemFor(id))
        ensureVisible(item);
}

void ImageCanvas::zoomIn()
{
    applyZoom(ZoomStep);
}

void ImageCanvas::zoomOut()
{
    applyZoom(1 / ZoomStep);
}

void ImageCanvas::zoomToFit()
{
    if (!hasImage())
        return;
    fitInView(imageRect(), Qt::KeepAspectRatio);
    // Маленькие изображения не растягиваем сверх 100%.
    if (viewScale() > 1.0)
        setTransform(QTransform());
    viewScaleChanged();
}

void ImageCanvas::zoomActual()
{
    setTransform(QTransform());
    viewScaleChanged();
}

void ImageCanvas::applyZoom(qreal factor)
{
    const qreal target = std::clamp(viewScale() * factor, MinScale, MaxScale);
    const qreal actual = target / viewScale();
    scale(actual, actual);
    viewScaleChanged();
}

void ImageCanvas::viewScaleChanged()
{
    AnnotationGraphicsItem::setViewScale(viewScale());
    // Размер подписей и маркеров зависит от масштаба, поэтому пересчитываем
    // границы всех элементов, в том числе черновика инструмента.
    for (QGraphicsItem *item : scene_->items()) {
        if (auto *annotationItem = qgraphicsitem_cast<AnnotationGraphicsItem *>(item))
            annotationItem->refresh();
        else if (auto *brush = qgraphicsitem_cast<BrushCursorItem *>(item))
            brush->refresh();
    }
    emit zoomChanged(viewScale());
}

void ImageCanvas::addItem(int id)
{
    const Annotation *annotation = document_->find(id);
    if (!annotation || items_.contains(id))
        return;
    auto *item = new AnnotationGraphicsItem(annotation);
    scene_->addItem(item);
    items_.insert(id, item);
}

void ImageCanvas::removeItem(int id)
{
    if (isSelected(id)) {
        QList<int> rest = selectedIds_;
        rest.removeAll(id);
        setSelectedIds(rest);
    }
    delete items_.take(id);
}

void ImageCanvas::updateItem(int id)
{
    if (AnnotationGraphicsItem *item = itemFor(id))
        item->refresh();
}

void ImageCanvas::rebuildItems()
{
    setSelectedId(0);
    qDeleteAll(items_);
    items_.clear();
    if (!document_)
        return;
    for (const auto &annotation : document_->annotations())
        addItem(annotation->id());
}

void ImageCanvas::updateCursor()
{
    if (panning_)
        viewport()->setCursor(Qt::ClosedHandCursor);
    else if (spaceHeld_)
        viewport()->setCursor(Qt::OpenHandCursor);
    else if (tool_ && hasImage() && (pointerInImage_ || buttonsHeld_))
        viewport()->setCursor(tool_->cursorShape());
    else
        viewport()->setCursor(Qt::ArrowCursor);
}

void ImageCanvas::updateToolHover()
{
    if (!tool_ || !hasImage())
        return;
    const QPoint pos = viewport()->mapFromGlobal(QCursor::pos());
    // С пробелом и во время прокрутки вместо кисти курсор-рука.
    if (viewport()->rect().contains(pos) && !spaceHeld_ && !panning_)
        tool_->hover(mapToScene(pos));
    else
        tool_->leave();
}

void ImageCanvas::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    buttonsHeld_ = true;
    // Пока меняется размер кисти, другие кнопки ничего не делают.
    if (resizingBrush_) {
        event->accept();
        return;
    }
    if (event->button() == Qt::RightButton && event->buttons() == Qt::RightButton
        && tool_ && hasImage() && tool_->canResizeBrush()) {
        resizingBrush_ = true;
        tool_->beginBrushResize(mapToScene(event->pos()));
        event->accept();
        return;
    }
    if (event->button() == Qt::MiddleButton || (spaceHeld_ && event->button() == Qt::LeftButton)) {
        panning_ = true;
        panLast_ = event->pos();
        updateCursor();
        updateToolHover();
        event->accept();
        return;
    }
    if (tool_ && hasImage())
        tool_->mousePress(mapToScene(event->pos()), event);
    event->accept();
}

void ImageCanvas::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF scenePos = mapToScene(event->pos());
    const bool inside = hasImage() && imageRect().contains(scenePos);
    const bool held = event->buttons() != Qt::NoButton;
    if (inside != pointerInImage_ || held != buttonsHeld_) {
        pointerInImage_ = inside;
        buttonsHeld_ = held;
        updateCursor();
    }
    if (hasImage()) {
        const QPoint pixel(static_cast<int>(std::floor(scenePos.x())),
                           static_cast<int>(std::floor(scenePos.y())));
        emit cursorPixelChanged(pixel, inside);
    }

    if (panning_) {
        const QPoint delta = event->pos() - panLast_;
        panLast_ = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }
    if (resizingBrush_) {
        tool_->resizeBrush(scenePos);
        event->accept();
        return;
    }
    if (spaceHeld_ && !held) {
        event->accept();
        return;
    }
    if (tool_ && hasImage())
        tool_->mouseMove(scenePos, event);
    event->accept();
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    buttonsHeld_ = event->buttons() != Qt::NoButton;
    pointerInImage_ = hasImage() && imageRect().contains(mapToScene(event->pos()));
    if (resizingBrush_) {
        if (event->button() == Qt::RightButton) {
            resizingBrush_ = false;
            tool_->endBrushResize(mapToScene(event->pos()));
            updateCursor();
        }
        event->accept();
        return;
    }
    if (panning_ && (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)) {
        panning_ = false;
        updateCursor();
        updateToolHover();
        event->accept();
        return;
    }
    if (tool_ && hasImage())
        tool_->mouseRelease(mapToScene(event->pos()), event);
    updateCursor();
    event->accept();
}

void ImageCanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Второе быстрое нажатие правой кнопки приходит как двойной щелчок,
    // но размер кисти должен меняться и после него.
    if (event->button() == Qt::RightButton) {
        mousePressEvent(event);
        return;
    }
    if (tool_ && hasImage() && !panning_)
        tool_->mouseDoubleClick(mapToScene(event->pos()), event);
    event->accept();
}

void ImageCanvas::wheelEvent(QWheelEvent *event)
{
    const int delta = event->angleDelta().y();
    if (delta == 0) {
        QGraphicsView::wheelEvent(event);
        return;
    }
    applyZoom(std::pow(ZoomStep, delta / 120.0));
    event->accept();
}

void ImageCanvas::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        if (!event->isAutoRepeat()) {
            spaceHeld_ = true;
            updateCursor();
            updateToolHover();
        }
        event->accept();
        return;
    }
    if (tool_ && tool_->keyPress(event)) {
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void ImageCanvas::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        spaceHeld_ = false;
        updateCursor();
        updateToolHover();
        event->accept();
        return;
    }
    QGraphicsView::keyReleaseEvent(event);
}

void ImageCanvas::focusOutEvent(QFocusEvent *event)
{
    spaceHeld_ = false;
    updateCursor();
    QGraphicsView::focusOutEvent(event);
}

void ImageCanvas::leaveEvent(QEvent *event)
{
    pointerInImage_ = false;
    emit cursorPixelChanged(QPoint(), false);
    if (tool_)
        tool_->leave();
    QGraphicsView::leaveEvent(event);
}
