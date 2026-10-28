#pragma once

#include <QGraphicsItem>

// Курсор-кисть: круг с диаметром кисти в пикселях изображения и крестик в центре.
// Рисуется инверсией цвета, поэтому виден на любом фоне.
class BrushCursorItem : public QGraphicsItem
{
public:
    enum { Type = UserType + 2 };

    BrushCursorItem();

    int type() const override { return Type; }
    void setDiameter(qreal diameter);
    // Вызывается после изменения масштаба вида.
    void refresh();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    QRectF computeBounds() const;

    qreal diameter_ = 1;
    // Граница кэшируется и меняется только после prepareGeometryChange().
    QRectF bounds_;
};
