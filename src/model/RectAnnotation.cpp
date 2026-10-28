#include "model/RectAnnotation.h"

#include <QPainter>
#include <QTextStream>

QString RectAnnotation::describe() const
{
    return QStringLiteral("(%1, %2) %3×%4")
        .arg(formatNumber(rect_.x()), formatNumber(rect_.y()),
             formatNumber(rect_.width()), formatNumber(rect_.height()));
}

bool RectAnnotation::hitTest(const QPointF &pos, qreal tolerance) const
{
    const QRectF outer = rect_.adjusted(-tolerance, -tolerance, tolerance, tolerance);
    if (!outer.contains(pos))
        return false;
    if (isFilled())
        return true;
    // Для рамки попаданием считается только близость к границе.
    const QRectF inner = rect_.adjusted(tolerance, tolerance, -tolerance, -tolerance);
    return !(inner.isValid() && inner.contains(pos));
}

void RectAnnotation::paint(QPainter &painter, qreal /*scale*/) const
{
    painter.setPen(outlinePen());
    painter.setBrush(fillBrush());
    painter.drawRect(rect_);
}

std::unique_ptr<Annotation> RectAnnotation::clone() const
{
    return std::make_unique<RectAnnotation>(*this);
}

void RectAnnotation::writeJsonGeometry(QJsonObject &obj) const
{
    obj["filled"] = isFilled();
    obj["x"] = rect_.x();
    obj["y"] = rect_.y();
    obj["w"] = rect_.width();
    obj["h"] = rect_.height();
}

bool RectAnnotation::readJsonGeometry(const QJsonObject &obj)
{
    for (const char *key : {"x", "y", "w", "h"}) {
        if (!obj.value(key).isDouble())
            return false;
    }
    const qreal w = obj.value("w").toDouble();
    const qreal h = obj.value("h").toDouble();
    if (w < 0 || h < 0)
        return false;
    setFilled(obj.value("filled").toBool(false));
    rect_ = QRectF(obj.value("x").toDouble(), obj.value("y").toDouble(), w, h);
    return true;
}

void RectAnnotation::writeTxtGeometry(QTextStream &out) const
{
    // Вершины по часовой стрелке, начиная с левой верхней.
    const QPointF corners[] = {rect_.topLeft(), rect_.topRight(),
                               rect_.bottomRight(), rect_.bottomLeft()};
    out << " filled=" << (isFilled() ? 1 : 0);
    for (int i = 0; i < 4; ++i) {
        out << " x" << i + 1 << '=' << formatNumber(corners[i].x())
            << " y" << i + 1 << '=' << formatNumber(corners[i].y());
    }
}
