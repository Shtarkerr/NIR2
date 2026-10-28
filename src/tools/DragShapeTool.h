#pragma once

#include "tools/Tool.h"

#include <QRectF>

// Инструмент, задающий фигуру протягиванием мыши по диагонали ограничивающего
// прямоугольника. С зажатым Shift прямоугольник становится квадратом.
class DragShapeTool : public Tool
{
public:
    using Tool::Tool;

    void mousePress(const QPointF &pos, QMouseEvent *event) override;
    void mouseMove(const QPointF &pos, QMouseEvent *event) override;
    void mouseRelease(const QPointF &pos, QMouseEvent *event) override;

protected:
    virtual std::unique_ptr<Annotation> createShape() const = 0;
    virtual void setShapeRect(Annotation *shape, const QRectF &rect) const = 0;

private:
    QPointF snap(const QPointF &pos) const;

    QPointF start_;
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
