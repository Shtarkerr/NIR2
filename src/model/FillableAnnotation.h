#pragma once

#include "model/Annotation.h"

#include <QBrush>

// Фигура, которую можно выделить либо границей, либо сплошной заливкой.
class FillableAnnotation : public Annotation
{
public:
    bool isFilled() const { return filled_; }
    void setFilled(bool filled) { filled_ = filled; }

protected:
    QBrush fillBrush() const { return filled_ ? QBrush(color()) : QBrush(Qt::NoBrush); }

private:
    bool filled_ = false;
};
