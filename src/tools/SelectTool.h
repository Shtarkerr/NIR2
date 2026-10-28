#pragma once

#include "tools/Tool.h"

// Выбор и перемещение готовых объектов мышью или стрелками (Shift — шаг 10 пикселей).
class SelectTool : public Tool
{
public:
    using Tool::Tool;

    QString name() const override;
    Qt::CursorShape cursorShape() const override { return Qt::ArrowCursor; }
    void mousePress(const QPointF &pos, QMouseEvent *event) override;
    void mouseMove(const QPointF &pos, QMouseEvent *event) override;
    void mouseRelease(const QPointF &pos, QMouseEvent *event) override;
    bool keyPress(QKeyEvent *event) override;
    void cancel() override;

private:
    int annotationAt(const QPointF &pos) const;
    QPointF dragDelta(const QPointF &pos) const;

    bool dragging_ = false;
    QPointF dragStart_;
};
