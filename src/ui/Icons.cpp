#include "ui/Icons.h"

#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>

#include <iterator>

namespace Icons {

namespace {

QColor withAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}

} // namespace

QIcon glyph(Glyph kind)
{
    // Рисуем в двойном размере, чтобы на экранах с масштабированием иконки были чёткими.
    constexpr int Size = 64;
    QPixmap pixmap(Size, Size);
    pixmap.fill(Qt::transparent);

    // Контур берётся из цвета текста текущей темы: на тёмной теме он светлый,
    // на светлой тёмный. Заливка цветная и полупрозрачная, видна на любом фоне.
    const QPalette palette = QGuiApplication::palette();
    const QColor fg = palette.color(QPalette::WindowText);
    const QColor bg = palette.color(QPalette::Window);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(2, 2);
    QPen pen(fg, 2.5);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    switch (kind) {
    case Glyph::Select: {
        QPainterPath arrow;
        arrow.moveTo(9, 4);
        arrow.lineTo(9, 26);
        arrow.lineTo(14.5, 21);
        arrow.lineTo(18.5, 29);
        arrow.lineTo(22, 27.5);
        arrow.lineTo(18, 19.5);
        arrow.lineTo(25, 19.5);
        arrow.closeSubpath();
        p.setPen(QPen(bg, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(fg);
        p.drawPath(arrow);
        break;
    }
    case Glyph::Point:
        p.setBrush(QColor(235, 64, 52));
        p.drawEllipse(QPointF(16, 16), 7, 7);
        break;
    case Glyph::Rect:
        p.setBrush(withAlpha(QColor(66, 135, 245), 170));
        p.drawRect(QRectF(4, 8, 24, 16));
        break;
    case Glyph::Ellipse:
        p.setBrush(withAlpha(QColor(52, 199, 89), 170));
        p.drawEllipse(QRectF(3, 7, 26, 18));
        break;
    case Glyph::Contour: {
        // Неровный многоугольник с точками в вершинах, чтобы не путать с эллипсом.
        const QPointF vertices[] = {{5, 23}, {8, 9}, {17, 4}, {27, 11}, {22, 18}, {26, 27}, {13, 25}};
        p.setBrush(withAlpha(QColor(255, 149, 0), 170));
        p.drawPolygon(vertices, int(std::size(vertices)));
        p.setPen(Qt::NoPen);
        p.setBrush(fg);
        for (const QPointF &v : vertices)
            p.drawEllipse(v, 2.2, 2.2);
        break;
    }
    case Glyph::Fill: {
        // Наклонённое ведро с краской и капля, как в графических редакторах.
        const QColor paint(66, 135, 245);
        p.save();
        p.translate(13, 18);
        p.rotate(35);
        p.drawArc(QRectF(-6, -16, 12, 12), 0, 180 * 16);
        p.setBrush(withAlpha(fg, 40));
        p.drawRoundedRect(QRectF(-8, -8, 16, 15), 2, 2);
        p.setBrush(paint);
        p.drawEllipse(QRectF(-8, -10, 16, 5));
        p.restore();

        QPainterPath drop;
        drop.moveTo(26, 15);
        drop.cubicTo(30.5, 21, 30.5, 27, 26, 27);
        drop.cubicTo(21.5, 27, 21.5, 21, 26, 15);
        p.setPen(QPen(fg, 1.5));
        p.setBrush(paint);
        p.drawPath(drop);
        break;
    }
    }
    return QIcon(pixmap);
}

QIcon colorSwatch(const QColor &color, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    // Шахматный фон показывает прозрачность цвета.
    const int half = size / 2;
    p.fillRect(0, 0, size, size, Qt::white);
    p.fillRect(0, 0, half, half, Qt::lightGray);
    p.fillRect(half, half, size - half, size - half, Qt::lightGray);
    p.fillRect(0, 0, size, size, color);
    // Рамка цвета текста темы, чтобы образец не терялся на фоне такого же цвета.
    p.setPen(QGuiApplication::palette().color(QPalette::WindowText));
    p.drawRect(0, 0, size - 1, size - 1);
    return QIcon(pixmap);
}

} // namespace Icons
