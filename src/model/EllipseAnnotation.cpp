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
    return QStringLiteral("центр (%1, %2), r = %3×%4")
        .arg(formatNumber(center_.x()), formatNumber(center_.y()),
             formatNumber(rx_), formatNumber(ry_));
}

QRectF EllipseAnnotation::boundingRect() const
{
    return QRectF(center_.x() - rx_, center_.y() - ry_, 2 * rx_, 2 * ry_);
}

bool EllipseAnnotation::hitTest(const QPointF &pos, qreal tolerance) const
{
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

void EllipseAnnotation::paint(QPainter &painter, qreal /*scale*/) const
{
    painter.setPen(outlinePen());
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
    center_ = QPointF(obj.value("cx").toDouble(), obj.value("cy").toDouble());
    rx_ = rx;
    ry_ = ry;
    return true;
}

void EllipseAnnotation::writeTxtGeometry(QTextStream &out) const
{
    out << " filled=" << (isFilled() ? 1 : 0)
        << " cx=" << formatNumber(center_.x())
        << " cy=" << formatNumber(center_.y())
        << " rx=" << formatNumber(rx_)
        << " ry=" << formatNumber(ry_);
}
