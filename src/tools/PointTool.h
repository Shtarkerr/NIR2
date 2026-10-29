#pragma once

#include "tools/Tool.h"

// Отметка отдельных точек щелчком мыши. Размер точки равен размеру кисти,
// вместо курсора показывается круг кисти.
class PointTool : public Tool
{
public:
    using Tool::Tool;

    QString name() const override;
    bool usesBrush() const override { return true; }
    void mousePress(const QPointF &pos, QMouseEvent *event) override;
    void mouseMove(const QPointF &pos, QMouseEvent *event) override;
    void mouseRelease(const QPointF &pos, QMouseEvent *event) override;
};
