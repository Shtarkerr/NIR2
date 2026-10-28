#include "model/Raster.h"

#include <algorithm>
#include <cstdlib>

namespace Raster {

QVector<QPoint> line(const QPoint &a, const QPoint &b)
{
    int x = a.x();
    int y = a.y();
    const int dx = std::abs(b.x() - x);
    const int dy = -std::abs(b.y() - y);
    const int sx = x < b.x() ? 1 : -1;
    const int sy = y < b.y() ? 1 : -1;
    int err = dx + dy;

    QVector<QPoint> pixels;
    pixels.reserve(std::max(dx, -dy) + 1);
    while (true) {
        pixels.append(QPoint(x, y));
        if (x == b.x() && y == b.y())
            break;
        const int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
    return pixels;
}

static void appendSegment(QVector<QPoint> &out, const QPoint &a, const QPoint &b, bool skipLast)
{
    const QVector<QPoint> segment = line(a, b);
    // Первый пиксель отрезка совпадает с последним пикселем предыдущего.
    const int end = segment.size() - (skipLast ? 1 : 0);
    for (int i = 1; i < end; ++i)
        out.append(segment[i]);
}

QVector<QPoint> polyline(const QVector<QPoint> &vertices, bool closed)
{
    QVector<QPoint> pixels;
    if (vertices.isEmpty())
        return pixels;

    pixels.append(vertices.first());
    for (int i = 1; i < vertices.size(); ++i)
        appendSegment(pixels, vertices[i - 1], vertices[i], false);

    if (closed && vertices.size() >= 3)
        appendSegment(pixels, vertices.last(), vertices.first(), true);

    return pixels;
}

} // namespace Raster
