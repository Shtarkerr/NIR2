#include "ui/BrushCursorItem.h"

#include "ui/AnnotationGraphicsItem.h"

#include <QPainter>

#include <algorithm>

namespace {

constexpr qreal CrossSize = 4;   // половина крестика в экранных пикселях

} // namespace

BrushCursorItem::BrushCursorItem()
    : bounds_(computeBounds())
{
    setZValue(10);
    setAcceptedMouseButtons(Qt::NoButton);
}

void BrushCursorItem::setDiameter(qreal diameter)
{
    if (qFuzzyCompare(diameter, diameter_))
        return;
    diameter_ = diameter;
    refresh();
}

void BrushCursorItem::refresh()
{
    const QRectF bounds = computeBounds();
    if (bounds != bounds_) {
        prepareGeometryChange();
        bounds_ = bounds;
    }
    update();
}

QRectF BrushCursorItem::boundingRect() const
{
    return bounds_;
}

QRectF BrushCursorItem::computeBounds() const
{
    const qreal scale = AnnotationGraphicsItem::viewScale();
    const qreal r = std::max(diameter_ / 2, CrossSize / scale) + 2 / scale;
    return QRectF(-r, -r, 2 * r, 2 * r);
}

void BrushCursorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const qreal scale = AnnotationGraphicsItem::viewScale();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setCompositionMode(QPainter::CompositionMode_Difference);

    QPen pen(Qt::white, 1);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    const qreal r = diameter_ / 2;
    // Совсем маленький круг на экране не виден, тогда остаётся только крестик.
    if (r * scale >= 1.5)
        painter->drawEllipse(QPointF(0, 0), r, r);

    const qreal c = CrossSize / scale;
    painter->drawLine(QPointF(-c, 0), QPointF(c, 0));
    painter->drawLine(QPointF(0, -c), QPointF(0, c));
}
