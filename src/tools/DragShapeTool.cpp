#include "tools/DragShapeTool.h"

#include "model/EllipseAnnotation.h"
#include "model/FillableAnnotation.h"
#include "model/RectAnnotation.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QMouseEvent>

#include <algorithm>
#include <cmath>

namespace {

constexpr int FilledBrushSize = 5;

} // namespace

void DragShapeTool::mousePress(const QPointF &pos, QMouseEvent *event)
{
    moveBrush(pos, event->buttons());
    if (event->button() != Qt::LeftButton)
        return;
    start_ = snap(pos);
    rect_ = QRectF(start_, start_);
    startDraft(createShape());
    setShapeRect(draft(), rect_);
    draftChanged();
}

void DragShapeTool::mouseMove(const QPointF &pos, QMouseEvent *event)
{
    moveBrush(pos, event->buttons());
    if (!draft())
        return;

    QPointF end = snap(pos);
    if (event->modifiers() & Qt::ShiftModifier) {
        const QPointF d = end - start_;
        const qreal side = std::max(std::abs(d.x()), std::abs(d.y()));
        end = clampToImage(start_ + QPointF(d.x() < 0 ? -side : side, d.y() < 0 ? -side : side));
    }
    rect_ = QRectF(start_, end).normalized();
    setShapeRect(draft(), rect_);
    draftChanged();
}

void DragShapeTool::mouseRelease(const QPointF &pos, QMouseEvent *event)
{
    moveBrush(pos, event->buttons());
    if (!draft() || event->button() != Qt::LeftButton)
        return;

    // Случайный щелчок без протягивания не создаёт вырожденную фигуру.
    // boundingRect() тут не подходит, в него входит толщина рамки.
    if (rect_.width() >= 1 && rect_.height() >= 1)
        commitDraft();
    else
        discardDraft();
}

bool DragShapeTool::keyPress(QKeyEvent *event)
{
    if (!draft() || event->key() != Qt::Key_Escape)
        return false;
    // Кнопка мыши ещё может быть зажата, но без черновика движения и отпускание ничего не делают.
    cancel();
    return true;
}

void DragShapeTool::styleChanged()
{
    Tool::styleChanged();
    // Заливку и толщину рамки можно поменять, не отпуская фигуру.
    if (draft()) {
        auto *shape = static_cast<FillableAnnotation *>(draft());
        shape->setFilled(context_.style.filled);
        shape->setWidth(brushSize());
        draftChanged();
    }
}

QPointF DragShapeTool::brushCenter(const QPointF &pos) const
{
    // Углы фигуры лежат на границах пикселей, туда же ставим кисть.
    return QPointF(std::round(pos.x()), std::round(pos.y()));
}

int DragShapeTool::brushSize() const
{
    return context_.style.filled ? FilledBrushSize : context_.style.brushSize;
}

QPointF DragShapeTool::snap(const QPointF &pos) const
{
    const QPointF p = clampToImage(pos);
    return QPointF(std::round(p.x()), std::round(p.y()));
}

QString RectTool::name() const
{
    return QCoreApplication::translate("Tool", "Прямоугольник");
}

std::unique_ptr<Annotation> RectTool::createShape() const
{
    return std::make_unique<RectAnnotation>();
}

void RectTool::setShapeRect(Annotation *shape, const QRectF &rect) const
{
    static_cast<RectAnnotation *>(shape)->setRect(rect);
}

QString EllipseTool::name() const
{
    return QCoreApplication::translate("Tool", "Эллипс");
}

std::unique_ptr<Annotation> EllipseTool::createShape() const
{
    return std::make_unique<EllipseAnnotation>();
}

void EllipseTool::setShapeRect(Annotation *shape, const QRectF &rect) const
{
    static_cast<EllipseAnnotation *>(shape)->setFromRect(rect);
}
