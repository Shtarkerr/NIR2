#include "ui/Icons.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

namespace Icons {

QIcon glyph(Glyph kind)
{
    constexpr int Size = 32;
    QPixmap pixmap(Size, Size);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor(40, 40, 40), 2.5);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    switch (kind) {
    case Glyph::Select: {
        QPainterPath arrow;
        arrow.moveTo(9, 5);
        arrow.lineTo(9, 26);
        arrow.lineTo(14, 21);
        arrow.lineTo(18, 28);
        arrow.lineTo(21, 26);
        arrow.lineTo(17, 19);
        arrow.lineTo(24, 19);
        arrow.closeSubpath();
        p.setBrush(Qt::white);
        p.drawPath(arrow);
        break;
    }
    case Glyph::Point:
        p.setBrush(QColor(220, 50, 50));
        p.drawEllipse(QPointF(16, 16), 6, 6);
        break;
    case Glyph::Rect:
        p.drawRect(QRectF(5, 8, 22, 16));
        break;
    case Glyph::Ellipse:
        p.drawEllipse(QRectF(4, 8, 24, 16));
        break;
    case Glyph::Contour: {
        QPainterPath path;
        path.moveTo(6, 20);
        path.cubicTo(4, 8, 14, 4, 20, 8);
        path.cubicTo(28, 12, 28, 22, 22, 25);
        path.cubicTo(16, 28, 9, 27, 6, 20);
        p.drawPath(path);
        break;
    }
    case Glyph::Fill:
        p.setBrush(QColor(90, 140, 220));
        p.drawRect(QRectF(5, 8, 22, 16));
        break;
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
    p.setPen(Qt::darkGray);
    p.drawRect(0, 0, size - 1, size - 1);
    return QIcon(pixmap);
}

} // namespace Icons
