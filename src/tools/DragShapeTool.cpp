#include "tools/DragShapeTool.h"

#include "model/EllipseAnnotation.h"
#include "model/RectAnnotation.h"

#include <QCoreApplication>
#include <QMouseEvent>

#include <algorithm>
#include <cmath>

void DragShapeTool::mousePress(const QPointF &pos, QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    start_ = snap(pos);
    startDraft(createShape());
    setShapeRect(draft(), QRectF(start_, start_));
    draftChanged();
}

void DragShapeTool::mouseMove(const QPointF &pos, QMouseEvent *event)
{
    if (!draft())
        return;

    QPointF end = snap(pos);
    if (event->modifiers() & Qt::ShiftModifier) {
        const QPointF d = end - start_;
        const qreal side = std::max(std::abs(d.x()), std::abs(d.y()));
        end = clampToImage(start_ + QPointF(d.x() < 0 ? -side : side, d.y() < 0 ? -side : side));
    }
    setShapeRect(draft(), QRectF(start_, end).normalized());
    draftChanged();
}

void DragShapeTool::mouseRelease(const QPointF &, QMouseEvent *event)
{
    if (!draft() || event->button() != Qt::LeftButton)
        return;

    // Случайный щелчок без протягивания не создаёт вырожденную фигуру.
    const QRectF r = draft()->boundingRect();
    if (r.width() >= 1 && r.height() >= 1)
        commitDraft();
    else
        discardDraft();
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
