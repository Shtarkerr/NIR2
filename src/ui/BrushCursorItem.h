#pragma once

#include <QGraphicsItem>
#include <QString>

// Курсор-кисть: круг с диаметром кисти в пикселях изображения и крестик в центре.
// Рисуется инверсией цвета, поэтому виден на любом фоне. Рядом с крестиком можно
// показать подпись, например размер кисти, пока его меняют.
class BrushCursorItem : public QGraphicsItem
{
public:
    enum { Type = UserType + 2 };

    BrushCursorItem();

    int type() const override { return Type; }
    void setDiameter(qreal diameter);
    void setText(const QString &text);
    // Вызывается после изменения масштаба вида.
    void refresh();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    QRectF computeBounds() const;
    QRectF textRect() const;

    qreal diameter_ = 1;
    QString text_;
    // Граница кэшируется и меняется только после prepareGeometryChange().
    QRectF bounds_;
};
