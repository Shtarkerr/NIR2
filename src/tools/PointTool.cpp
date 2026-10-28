#include "tools/PointTool.h"

#include "model/PointAnnotation.h"

#include <QCoreApplication>
#include <QMouseEvent>

QString PointTool::name() const
{
    return QCoreApplication::translate("Tool", "Точка");
}

void PointTool::mousePress(const QPointF &pos, QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    startDraft(std::make_unique<PointAnnotation>(pixelAt(pos)));
    commitDraft();
}
