#pragma once

#include "tools/Tool.h"

// Выбор и перемещение объектов мышью или стрелками (с Shift шаг 10 пикселей).
// Ctrl + щелчок добавляет объект к выделению, выделенную группу можно тащить целиком.
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
    // Сдвигает графические элементы выделенных объектов, не трогая документ.
    void offsetSelectedItems(const QPointF &offset);
    void moveSelection(const QPointF &delta);

    bool dragging_ = false;
    QPointF dragStart_;
};
