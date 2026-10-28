#pragma once

#include <QPoint>
#include <QVector>

namespace Raster {

// Пиксели отрезка от a до b (алгоритм Брезенхэма), оба конца включены.
QVector<QPoint> line(const QPoint &a, const QPoint &b);

// Пиксели ломаной подряд, без повторов на стыках отрезков.
// Если closed и вершин хотя бы три, добавляется отрезок от последней вершины к первой.
QVector<QPoint> polyline(const QVector<QPoint> &vertices, bool closed);

} // namespace Raster
