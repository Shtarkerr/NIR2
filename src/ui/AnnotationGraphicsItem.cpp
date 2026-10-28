#include "ui/AnnotationGraphicsItem.h"

#include "model/Annotation.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>

qreal AnnotationGraphicsItem::s_viewScale = 1.0;

namespace {

constexpr qreal MarkerMargin = 8;   // экранные пиксели вокруг фигуры (маркеры точек, рамка выделения)
constexpr qreal LabelPadding = 3;

QFont labelFont()
{
    QFont font;
    font.setPixelSize(12);
    return font;
}

} // namespace

AnnotationGraphicsItem::AnnotationGraphicsItem(const Annotation *annotation, QGraphicsItem *parent)
    : QGraphicsItem(parent), annotation_(annotation)
{
    setZValue(1);
}

int AnnotationGraphicsItem::annotationId() const
{
    return annotation_->id();
}

void AnnotationGraphicsItem::setHighlighted(bool highlighted)
{
    if (highlighted_ == highlighted)
        return;
    highlighted_ = highlighted;
    update();
}

void AnnotationGraphicsItem::refresh()
{
    prepareGeometryChange();
    update();
}

void AnnotationGraphicsItem::setViewScale(qreal scale)
{
    if (scale > 0)
        s_viewScale = scale;
}

QRectF AnnotationGraphicsItem::labelRect() const
{
    if (annotation_->label().isEmpty())
        return QRectF();

    const QFontMetricsF metrics(labelFont());
    const qreal w = (metrics.horizontalAdvance(annotation_->label()) + 2 * LabelPadding) / s_viewScale;
    const qreal h = (metrics.height() + 2) / s_viewScale;
    const QPointF topLeft = annotation_->boundingRect().topLeft();
    return QRectF(topLeft.x(), topLeft.y() - h, w, h);
}

QRectF AnnotationGraphicsItem::boundingRect() const
{
    const qreal m = MarkerMargin / s_viewScale;
    QRectF rect = annotation_->boundingRect().adjusted(-m, -m, m, m);
    const QRectF label = labelRect();
    if (!label.isNull())
        rect = rect.united(label);
    return rect;
}

void AnnotationGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    annotation_->paint(*painter, s_viewScale);

    if (highlighted_) {
        QPen pen(Qt::white, 1, Qt::DashLine);
        pen.setCosmetic(true);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        const qreal m = 4 / s_viewScale;
        painter->drawRect(annotation_->boundingRect().adjusted(-m, -m, m, m));
        pen.setColor(Qt::black);
        pen.setDashOffset(4);
        painter->setPen(pen);
        painter->drawRect(annotation_->boundingRect().adjusted(-m, -m, m, m));
    }

    const QRectF label = labelRect();
    if (label.isNull())
        return;

    // Подпись рисуется в экранных координатах, чтобы не зависеть от масштаба.
    painter->save();
    painter->translate(label.topLeft());
    painter->scale(1 / s_viewScale, 1 / s_viewScale);
    const QRectF box(0, 0, label.width() * s_viewScale, label.height() * s_viewScale);
    const QColor background = annotation_->color();
    painter->setPen(Qt::NoPen);
    painter->setBrush(background);
    painter->drawRect(box);
    painter->setFont(labelFont());
    // Цвет текста выбирается по воспринимаемой яркости фона (коэффициенты ITU-R BT.601).
    const qreal luma = 0.299 * background.redF() + 0.587 * background.greenF() + 0.114 * background.blueF();
    painter->setPen(luma > 0.5 ? Qt::black : Qt::white);
    painter->drawText(box, Qt::AlignCenter, annotation_->label());
    painter->restore();
}
