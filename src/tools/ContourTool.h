#pragma once

#include "tools/Tool.h"

class ContourAnnotation;

// Выделение произвольным контуром. Два режима:
//  - свободное рисование: зажать левую кнопку и обвести область;
//  - многоугольник: щелчками ставить вершины, завершить двойным щелчком или Enter.
// Backspace удаляет последнюю вершину, Esc отменяет контур.
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
