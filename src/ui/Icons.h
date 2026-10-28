#pragma once

#include <QColor>
#include <QIcon>

// Иконки рисуются программно, чтобы не зависеть от файлов ресурсов и SVG-плагина.
namespace Icons {

enum class Glyph { Select, Point, Rect, Ellipse, Contour, Fill };

QIcon glyph(Glyph kind);
QIcon colorSwatch(const QColor &color, int size = 16);

} // namespace Icons
