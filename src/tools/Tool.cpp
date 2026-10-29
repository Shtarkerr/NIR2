#include "tools/Tool.h"

#include "commands/AnnotationCommands.h"
#include "model/FillableAnnotation.h"
#include "ui/AnnotationGraphicsItem.h"
#include "ui/BrushCursorItem.h"
#include "ui/ImageCanvas.h"

#include <QGraphicsScene>
#include <QUndoStack>

#include <algorithm>
#include <cmath>

Tool::Tool(ToolContext &context)
    : context_(context)
{
}

Tool::~Tool() = default;

void Tool::mousePress(const QPointF &, QMouseEvent *) {}
void Tool::mouseMove(const QPointF &, QMouseEvent *) {}
void Tool::mouseRelease(const QPointF &, QMouseEvent *) {}
void Tool::mouseDoubleClick(const QPointF &, QMouseEvent *) {}

bool Tool::keyPress(QKeyEvent *)
{
    return false;
}

void Tool::cancel()
{
    discardDraft();
}

void Tool::deactivate()
{
    cancel();
    brush_.reset();
}

void Tool::hover(const QPointF &pos)
{
    if (usesBrush())
        moveBrush(pos, Qt::NoButton);
}

void Tool::leave()
{
    hideBrushCursor();
}

void Tool::styleChanged()
{
    if (brush_)
        brush_->setDiameter(brushSize());
}

void Tool::beginBrushResize(const QPointF &pos)
{
    resizeAnchor_ = pos;
    resizeStartSize_ = context_.style.brushSize;
    showBrushCursor(pos);
    showBrushSize();
}

void Tool::resizeBrush(const QPointF &pos)
{
    // Край круга сдвигается вместе с мышью, поэтому диаметр меняется на двойной сдвиг.
    const int size = resizeStartSize_ + static_cast<int>(std::lround(2 * (pos.x() - resizeAnchor_.x())));
    if (context_.setBrushSize)
        context_.setBrushSize(size);
    showBrushSize();
}

void Tool::endBrushResize(const QPointF &pos)
{
    if (brush_)
        brush_->setText(QString());
    moveBrush(pos, Qt::NoButton);
}

void Tool::showBrushSize()
{
    // Размер подписан прямо у кисти. Всплывающая подсказка тут не годится: её окно
    // создаётся посреди обработки мыши, и Qt может упасть, повторно войдя в этот код.
    if (brush_)
        brush_->setText(QStringLiteral("%1 px").arg(context_.style.brushSize));
}

void Tool::moveBrush(const QPointF &pos, Qt::MouseButtons buttons)
{
    // За пределами картинки кисть не нужна, там холст показывает обычный курсор.
    const bool drawing = buttons & Qt::LeftButton;
    if (!drawing && !context_.canvas->imageRect().contains(pos))
        hideBrushCursor();
    else
        showBrushCursor(pos);
}

QPointF Tool::brushCenter(const QPointF &pos) const
{
    // Линия проходит через центры пикселей. Координату не ограничиваем краями
    // картинки, иначе кисть прилипает к краю, когда мышь уходит.
    return QPointF(std::floor(pos.x()) + 0.5, std::floor(pos.y()) + 0.5);
}

void Tool::showBrushCursor(const QPointF &pos)
{
    if (!brush_) {
        brush_ = std::make_unique<BrushCursorItem>();
        context_.canvas->scene()->addItem(brush_.get());
    }
    brush_->setDiameter(brushSize());
    brush_->setPos(brushCenter(pos));
    brush_->show();
}

void Tool::hideBrushCursor()
{
    if (brush_)
        brush_->hide();
}

QPointF Tool::clampToImage(const QPointF &pos) const
{
    const QRectF r = context_.canvas->imageRect();
    return QPointF(std::clamp(pos.x(), r.left(), r.right()),
                   std::clamp(pos.y(), r.top(), r.bottom()));
}

QPoint Tool::pixelAt(const QPointF &pos) const
{
    const QSize size = context_.canvas->imageSize();
    const int x = static_cast<int>(std::floor(pos.x()));
    const int y = static_cast<int>(std::floor(pos.y()));
    return QPoint(std::clamp(x, 0, std::max(0, size.width() - 1)),
                  std::clamp(y, 0, std::max(0, size.height() - 1)));
}

void Tool::startDraft(std::unique_ptr<Annotation> draft)
{
    discardDraft();

    draft->setColor(context_.style.color);
    draft->setLabel(context_.style.label);
    if (auto *fillable = dynamic_cast<FillableAnnotation *>(draft.get())) {
        fillable->setFilled(context_.style.filled);
        fillable->setWidth(brushSize());
    }

    draft_ = std::move(draft);
    draftItem_ = std::make_unique<AnnotationGraphicsItem>(draft_.get());
    draftItem_->setZValue(2);
    context_.canvas->scene()->addItem(draftItem_.get());
}

void Tool::draftChanged()
{
    if (draftItem_)
        draftItem_->refresh();
}

void Tool::commitDraft()
{
    if (!draft_)
        return;
    draftItem_.reset();
    context_.undoStack->push(new AddAnnotationCommand(context_.document, std::move(draft_)));
}

void Tool::discardDraft()
{
    draftItem_.reset();
    draft_.reset();
}
