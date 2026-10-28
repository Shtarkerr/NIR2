#include "model/PointAnnotation.h"

#include <QLineF>
#include <QPainter>
#include <QTextStream>

#include <algorithm>

QString PointAnnotation::describe() const
{
    return QStringLiteral("(%1, %2)").arg(pos_.x()).arg(pos_.y());
}

QRectF PointAnnotation::boundingRect() const
{
    return QRectF(pos_, QSizeF(1, 1));
}

bool PointAnnotation::hitTest(const QPointF &pos, qreal tolerance) const
{
    return QLineF(center(), pos).length() <= tolerance + 0.5;
}

void PointAnnotation::translate(const QPointF &delta)
{
    pos_ += delta.toPoint();
}

void PointAnnotation::paint(QPainter &painter, qreal scale) const
{
    const qreal radius = std::max(4.0 / scale, 0.5);
    QPen pen(Qt::black, 1);
    pen.setCosmetic(true);
    painter.setPen(pen);
    painter.setBrush(color());
    painter.drawEllipse(center(), radius, radius);
}

std::unique_ptr<Annotation> PointAnnotation::clone() const
{
    return std::make_unique<PointAnnotation>(*this);
}

void PointAnnotation::writeJsonGeometry(QJsonObject &obj) const
{
    obj["x"] = pos_.x();
    obj["y"] = pos_.y();
}

bool PointAnnotation::readJsonGeometry(const QJsonObject &obj)
{
    if (!obj.contains("x") || !obj.contains("y"))
        return false;
    pos_ = QPoint(obj.value("x").toInt(), obj.value("y").toInt());
    return true;
}

void PointAnnotation::writeTxtGeometry(QTextStream &out) const
{
    out << " x=" << pos_.x() << " y=" << pos_.y();
}
