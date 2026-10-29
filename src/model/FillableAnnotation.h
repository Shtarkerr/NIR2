#pragma once

#include "model/Annotation.h"

#include <QBrush>
#include <QPen>

// Фигура, которую можно выделить либо рамкой, либо сплошной заливкой.
class FillableAnnotation : public Annotation
{
public:
    bool isFilled() const { return filled_; }
    void setFilled(bool filled) { filled_ = filled; }

    // Толщина рамки (размер кисти) в пикселях изображения. Рамка проходит
    // посередине границы фигуры. У залитой фигуры толщина не используется.
    int width() const { return width_; }
    void setWidth(int width) { width_ = width < 1 ? 1 : width; }

protected:
    QBrush fillBrush() const { return filled_ ? QBrush(color()) : QBrush(Qt::NoBrush); }
    QPen shapePen(qreal scale) const { return filled_ ? outlinePen() : strokePen(width_, scale); }
    // На сколько рамка выходит за границу фигуры.
    qreal strokeMargin() const { return filled_ ? 0 : width_ / 2.0; }

private:
    bool filled_ = false;
    int width_ = 1;
};
