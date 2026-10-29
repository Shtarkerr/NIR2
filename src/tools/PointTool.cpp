#include "tools/PointTool.h"

#include "model/PointAnnotation.h"
#include "ui/ImageCanvas.h"

#include <QCoreApplication>
#include <QMouseEvent>

QString PointTool::name() const
{
    return QCoreApplication::translate("Tool", "Точка");
}

void PointTool::mousePress(const QPointF &pos, QMouseEvent *event)
{
    moveBrush(pos, event->buttons());
    // Щелчок по серому полю вокруг картинки точку не ставит.
    if (event->button() != Qt::LeftButton || !context_.canvas->imageRect().contains(pos))
        return;
    auto point = std::make_unique<PointAnnotation>(pixelAt(pos));
    point->setSize(context_.style.brushSize);
    startDraft(std::move(point));
    commitDraft();
}

void PointTool::mouseMove(const QPointF &pos, QMouseEvent *event)
{
    moveBrush(pos, event->buttons());
}

void PointTool::mouseRelease(const QPointF &pos, QMouseEvent *event)
{
    moveBrush(pos, event->buttons());
}
