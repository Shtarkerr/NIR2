#include "model/ContourAnnotation.h"

#include "model/Raster.h"

#include <QJsonArray>
#include <QLineF>
#include <QPainter>
#include <QPolygon>
#include <QTextStream>

#include <algorithm>

namespace {

qreal distanceToSegment(const QPointF &p, const QPointF &a, const QPointF &b)
{
    const QPointF ab = b - a;
    const qreal len2 = QPointF::dotProduct(ab, ab);
    if (len2 == 0)
        return QLineF(p, a).length();
    const qreal t = std::clamp(QPointF::dotProduct(p - a, ab) / len2, 0.0, 1.0);
    return QLineF(p, a + t * ab).length();
}

void writePoints(QTextStream &out, const char *prefix, const QVector<QPoint> &points)
{
    out << "\n  " << prefix;
    for (int i = 0; i < points.size(); ++i)
        out << (i == 0 ? " " : "; ") << points[i].x() << ' ' << points[i].y();
}

} // namespace

bool ContourAnnotation::addVertex(const QPoint &vertex)
{
    if (!vertices_.isEmpty() && vertices_.last() == vertex)
        return false;
    vertices_.append(vertex);
    return true;
}

void ContourAnnotation::removeLastVertex()
{
    if (!vertices_.isEmpty())
        vertices_.removeLast();
}

QVector<QPoint> ContourAnnotation::rasterize() const
{
    return Raster::polyline(vertices_, closed_);
}

QString ContourAnnotation::describe() const
{
    return QStringLiteral("вершин: %1, %2, кисть %3 px").arg(vertices_.size())
        .arg(closed_ ? QStringLiteral("замкнутый") : QStringLiteral("открытый"))
        .arg(width_);
}

QRectF ContourAnnotation::boundingRect() const
{
    if (vertices_.isEmpty())
        return QRectF();
    // QPolygon::boundingRect() включает граничные пиксели целиком,
    // толстая линия выходит за них ещё на половину толщины.
    const qreal extra = std::max(0.0, width_ / 2.0 - 0.5);
    return QRectF(QPolygon(vertices_).boundingRect()).adjusted(-extra, -extra, extra, extra);
}

bool ContourAnnotation::hitTest(const QPointF &pos, qreal tolerance) const
{
    if (vertices_.isEmpty())
        return false;
    tolerance += width_ / 2.0;
    if (vertices_.size() == 1)
        return QLineF(pixelCenter(vertices_.first()), pos).length() <= tolerance;

    const int segments = closed_ ? vertices_.size() : vertices_.size() - 1;
    for (int i = 0; i < segments; ++i) {
        const QPointF a = pixelCenter(vertices_[i]);
        const QPointF b = pixelCenter(vertices_[(i + 1) % vertices_.size()]);
        if (distanceToSegment(pos, a, b) <= tolerance)
            return true;
    }
    return false;
}

void ContourAnnotation::translate(const QPointF &delta)
{
    const QPoint d = delta.toPoint();
    for (QPoint &v : vertices_)
        v += d;
}

void ContourAnnotation::paint(QPainter &painter, qreal scale) const
{
    if (vertices_.isEmpty())
        return;

    if (vertices_.size() == 1) {
        const qreal r = std::max(width_ / 2.0, 2.0 / scale);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color());
        painter.drawEllipse(pixelCenter(vertices_.first()), r, r);
        return;
    }

    QPen pen = strokePen(width_, scale);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    QPolygonF polygon;
    polygon.reserve(vertices_.size());
    for (const QPoint &v : vertices_)
        polygon.append(pixelCenter(v));

    if (closed_)
        painter.drawPolygon(polygon);
    else
        painter.drawPolyline(polygon);
}

std::unique_ptr<Annotation> ContourAnnotation::clone() const
{
    return std::make_unique<ContourAnnotation>(*this);
}

void ContourAnnotation::writeJsonGeometry(QJsonObject &obj) const
{
    QJsonArray points;
    for (const QPoint &v : vertices_)
        points.append(QJsonArray{v.x(), v.y()});
    obj["closed"] = closed_;
    obj["width"] = width_;
    obj["vertices"] = points;
}

bool ContourAnnotation::readJsonGeometry(const QJsonObject &obj)
{
    const QJsonValue value = obj.value("vertices");
    if (!value.isArray())
        return false;

    QVector<QPoint> vertices;
    for (const QJsonValue &item : value.toArray()) {
        const QJsonArray pair = item.toArray();
        if (pair.size() != 2 || !pair[0].isDouble() || !pair[1].isDouble())
            return false;
        vertices.append(QPoint(pair[0].toInt(), pair[1].toInt()));
    }
    if (vertices.isEmpty())
        return false;

    vertices_ = vertices;
    closed_ = obj.value("closed").toBool(true);
    setWidth(obj.value("width").toInt(1));
    return true;
}

void ContourAnnotation::writeTxtGeometry(QTextStream &out) const
{
    const QVector<QPoint> pixels = rasterize();
    out << " closed=" << (closed_ ? 1 : 0)
        << " width=" << width_
        << " vertices=" << vertices_.size()
        << " pixels=" << pixels.size();
    writePoints(out, "V", vertices_);
    writePoints(out, "P", pixels);
}
