#pragma once

#include "tools/Tool.h"

class ContourAnnotation;

// Произвольный контур. Можно обвести область с зажатой кнопкой мыши,
// а можно ставить вершины щелчками и завершить двойным щелчком или Enter.
// Backspace убирает последнюю вершину, Esc отменяет контур.
class ContourTool : public Tool
{
public:
    using Tool::Tool;

    QString name() const override;
    void mousePress(const QPointF &pos, QMouseEvent *event) override;
    void mouseMove(const QPointF &pos, QMouseEvent *event) override;
    void mouseRelease(const QPointF &pos, QMouseEvent *event) override;
    void mouseDoubleClick(const QPointF &pos, QMouseEvent *event) override;
    bool keyPress(QKeyEvent *event) override;
    void cancel() override;

private:
    ContourAnnotation *contour() const;
    void finish();

    bool dragging_ = false;
    bool dragged_ = false;
};
