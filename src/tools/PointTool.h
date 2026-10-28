#pragma once

#include "tools/Tool.h"

// Отметка отдельных точек щелчком мыши.
class PointTool : public Tool
{
public:
    using Tool::Tool;

    QString name() const override;
    void mousePress(const QPointF &pos, QMouseEvent *event) override;
};
