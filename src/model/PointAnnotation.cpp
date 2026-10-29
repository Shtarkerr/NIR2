#include "model/PointAnnotation.h"

#include <QLineF>
#include <QPainter>
#include <QTextStream>

#include <algorithm>

QString PointAnnotation::describe() const
{
    return QStringLiteral("(%1, %2), размер %3 px").arg(pos_.x()).arg(pos_.y()).arg(size_);
}

QRectF PointAnnotation::boundingRect() const
{
    const qreal r = size_ / 2.0;
    return QRectF(center() - QPointF(r, r), QSizeF(size_, size_));
}

bool PointAnnotation::hitTest(const QPointF &pos, qreal tolerance) const
{
    return QLineF(center(), pos).length() <= tolerance + size_ / 2.0;
}

void PointAnnotation::translate(const QPointF &delta)
{
    pos_ += delta.toPoint();
}

void PointAnnotation::paint(QPainter &painter, qreal scale) const
{
    // Маленькая точка рисуется не меньше нескольких экранных пикселей, иначе её не видно.
    const qreal radius = std::max(4.0 / scale, size_ / 2.0);
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
    obj["size"] = size_;
}

bool PointAnnotation::readJsonGeometry(const QJsonObject &obj)
{
    if (!obj.contains("x") || !obj.contains("y"))
        return false;
    pos_ = QPoint(obj.value("x").toInt(), obj.value("y").toInt());
    setSize(obj.value("size").toInt(1));
    return true;
}

void PointAnnotation::writeTxtGeometry(QTextStream &out) const
{
    out << QStringLiteral("  Пиксель: %1\n").arg(formatPoint(pos_))
        << QStringLiteral("  Размер: %1 px\n").arg(size_);
}
