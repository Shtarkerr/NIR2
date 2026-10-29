#include "ui/BrushCursorItem.h"

#include "ui/AnnotationGraphicsItem.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>

#include <algorithm>

namespace {

constexpr qreal CrossSize = 4;   // половина крестика в экранных пикселях
constexpr qreal TextPadding = 3;

const QFont &textFont()
{
    static const QFont font = [] {
        QFont f;
        f.setPixelSize(12);
        return f;
    }();
    return font;
}

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

void BrushCursorItem::setText(const QString &text)
{
    if (text == text_)
        return;
    text_ = text;
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
    const QRectF circle(-r, -r, 2 * r, 2 * r);
    return text_.isEmpty() ? circle : circle.united(textRect());
}

QRectF BrushCursorItem::textRect() const
{
    // Подпись справа от крестика, её размер не зависит от масштаба.
    const qreal scale = AnnotationGraphicsItem::viewScale();
    const QFontMetricsF metrics(textFont());
    const qreal w = (metrics.horizontalAdvance(text_) + 2 * TextPadding) / scale;
    const qreal h = (metrics.height() + 2) / scale;
    return QRectF((CrossSize + 4) / scale, -h / 2, w, h);
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

    if (text_.isEmpty())
        return;
    // Подпись рисуется обычным способом, на тёмной подложке, в экранных координатах.
    const QRectF rect = textRect();
    painter->save();
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->translate(rect.topLeft());
    painter->scale(1 / scale, 1 / scale);
    const QRectF box(0, 0, rect.width() * scale, rect.height() * scale);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 180));
    painter->drawRoundedRect(box, 3, 3);
    painter->setFont(textFont());
    painter->setPen(Qt::white);
    painter->drawText(box, Qt::AlignCenter, text_);
    painter->restore();
}
