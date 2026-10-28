#pragma once

#include "tools/Tool.h"
#include "ui/BrushCursorItem.h"

class ContourAnnotation;

// Произвольный контур. Можно обвести область с зажатой кнопкой мыши,
// а можно ставить вершины щелчками и завершить двойным щелчком или Enter.
// Backspace убирает последнюю вершину, Esc отменяет контур.
// Вместо курсора показывается круг размером с кисть, толщина линии равна размеру кисти.
class ContourTool : public Tool
{
public:
    using Tool::Tool;

    QString name() const override;
    Qt::CursorShape cursorShape() const override { return Qt::BlankCursor; }
    void mousePress(const QPointF &pos, QMouseEvent *event) override;
    void mouseMove(const QPointF &pos, QMouseEvent *event) override;
    void mouseRelease(const QPointF &pos, QMouseEvent *event) override;
    void mouseDoubleClick(const QPointF &pos, QMouseEvent *event) override;
    bool keyPress(QKeyEvent *event) override;
    void cancel() override;
    void deactivate() override;
    void leave() override;
    void styleChanged() override;

private:
    ContourAnnotation *contour() const;
    void finish();
    void moveBrush(const QPointF &pos, Qt::MouseButtons buttons);

    std::unique_ptr<BrushCursorItem> brush_;

    bool dragging_ = false;
    bool dragged_ = false;
};
