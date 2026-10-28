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
    // Щелчок по серому полю вокруг картинки точку не ставит.
    if (event->button() != Qt::LeftButton || !context_.canvas->imageRect().contains(pos))
        return;
    startDraft(std::make_unique<PointAnnotation>(pixelAt(pos)));
    commitDraft();
}
