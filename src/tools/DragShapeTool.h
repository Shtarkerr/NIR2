#pragma once

#include "tools/Tool.h"

#include <QRectF>

// Инструмент, задающий фигуру протягиванием мыши по диагонали ограничивающего
// прямоугольника. С зажатым Shift прямоугольник становится квадратом, Esc отменяет фигуру.
// Толщина рамки равна размеру кисти, вместо курсора показывается круг кисти.
// У залитой фигуры рамки нет, тогда кисть постоянного размера и служит только курсором.
class DragShapeTool : public Tool
{
public:
    using Tool::Tool;

    bool usesBrush() const override { return true; }
    bool canResizeBrush() const override { return !context_.style.filled; }
    void mousePress(const QPointF &pos, QMouseEvent *event) override;
    void mouseMove(const QPointF &pos, QMouseEvent *event) override;
    void mouseRelease(const QPointF &pos, QMouseEvent *event) override;
    bool keyPress(QKeyEvent *event) override;
    void styleChanged() override;

protected:
    QPointF brushCenter(const QPointF &pos) const override;
    int brushSize() const override;
    virtual std::unique_ptr<Annotation> createShape() const = 0;
    virtual void setShapeRect(Annotation *shape, const QRectF &rect) const = 0;

private:
    QPointF snap(const QPointF &pos) const;

    QPointF start_;
    QRectF rect_;
};

class RectTool : public DragShapeTool
{
public:
    using DragShapeTool::DragShapeTool;

    QString name() const override;

protected:
    std::unique_ptr<Annotation> createShape() const override;
    void setShapeRect(Annotation *shape, const QRectF &rect) const override;
};

class EllipseTool : public DragShapeTool
{
public:
    using DragShapeTool::DragShapeTool;

    QString name() const override;

protected:
    std::unique_ptr<Annotation> createShape() const override;
    void setShapeRect(Annotation *shape, const QRectF &rect) const override;
};
