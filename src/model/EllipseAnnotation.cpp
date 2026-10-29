#include "model/EllipseAnnotation.h"

#include <QPainter>
#include <QTextStream>

void EllipseAnnotation::setFromRect(const QRectF &rect)
{
    const QRectF r = rect.normalized();
    center_ = r.center();
    rx_ = r.width() / 2;
    ry_ = r.height() / 2;
}

QString EllipseAnnotation::describe() const
{
    const QString geometry = QStringLiteral("центр (%1, %2), полуоси %3 и %4")
        .arg(formatNumber(center_.x()), formatNumber(center_.y()),
             formatNumber(rx_), formatNumber(ry_));
    return isFilled() ? geometry : QStringLiteral("%1, рамка %2 px").arg(geometry).arg(width());
}

QRectF EllipseAnnotation::boundingRect() const
{
    const qreal m = strokeMargin();
    return QRectF(center_.x() - rx_ - m, center_.y() - ry_ - m, 2 * (rx_ + m), 2 * (ry_ + m));
}

bool EllipseAnnotation::hitTest(const QPointF &pos, qreal tolerance) const
{
    tolerance += strokeMargin();
    const QPointF d = pos - center_;
    const auto inside = [&d](qreal a, qreal b) {
        if (a <= 0 || b <= 0)
            return false;
        return (d.x() * d.x()) / (a * a) + (d.y() * d.y()) / (b * b) <= 1.0;
    };

    if (!inside(rx_ + tolerance, ry_ + tolerance))
        return false;
    if (isFilled())
        return true;
    return !inside(rx_ - tolerance, ry_ - tolerance);
}

void EllipseAnnotation::paint(QPainter &painter, qreal scale) const
{
    painter.setPen(shapePen(scale));
    painter.setBrush(fillBrush());
    painter.drawEllipse(center_, rx_, ry_);
}

std::unique_ptr<Annotation> EllipseAnnotation::clone() const
{
    return std::make_unique<EllipseAnnotation>(*this);
}

void EllipseAnnotation::writeJsonGeometry(QJsonObject &obj) const
{
    obj["filled"] = isFilled();
    obj["width"] = width();
    obj["cx"] = center_.x();
    obj["cy"] = center_.y();
    obj["rx"] = rx_;
    obj["ry"] = ry_;
}

bool EllipseAnnotation::readJsonGeometry(const QJsonObject &obj)
{
    for (const char *key : {"cx", "cy", "rx", "ry"}) {
        if (!obj.value(key).isDouble())
            return false;
    }
    const qreal rx = obj.value("rx").toDouble();
    const qreal ry = obj.value("ry").toDouble();
    if (rx < 0 || ry < 0)
        return false;
    setFilled(obj.value("filled").toBool(false));
    setWidth(obj.value("width").toInt(1));
    center_ = QPointF(obj.value("cx").toDouble(), obj.value("cy").toDouble());
    rx_ = rx;
    ry_ = ry;
    return true;
}

void EllipseAnnotation::writeTxtGeometry(QTextStream &out) const
{
    out << QStringLiteral("  Вид: %1\n").arg(styleText())
        << QStringLiteral("  Центр: %1\n").arg(formatPoint(center_))
        << QStringLiteral("  Полуоси: по горизонтали %1, по вертикали %2\n")
               .arg(formatNumber(rx_), formatNumber(ry_));
}
