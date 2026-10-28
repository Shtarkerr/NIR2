#pragma once

#include <QColor>
#include <QIcon>

// Иконки рисуются в коде, чтобы не возиться с файлами ресурсов.
namespace Icons {

enum class Glyph { Select, Point, Rect, Ellipse, Contour, Fill };

QIcon glyph(Glyph kind);
QIcon colorSwatch(const QColor &color, int size = 16);

} // namespace Icons
