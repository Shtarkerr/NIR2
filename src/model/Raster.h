#pragma once

#include <QPoint>
#include <QVector>

// Растеризация отрезков и ломаных в последовательности пикселей.
namespace Raster {

// Пиксели отрезка [a, b] по алгоритму Брезенхэма, включая оба конца.
// Соседние пиксели результата 8-связны.
QVector<QPoint> line(const QPoint &a, const QPoint &b);

// Последовательные пиксели ломаной без повторов в местах стыка отрезков.
// Для замкнутой ломаной (не менее трёх вершин) добавляется отрезок от последней
// вершины к первой; сама первая вершина в конце не дублируется.
QVector<QPoint> polyline(const QVector<QPoint> &vertices, bool closed);

} // namespace Raster
