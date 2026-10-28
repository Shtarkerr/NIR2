#include "tools/SelectTool.h"

#include "commands/AnnotationCommands.h"
#include "model/AnnotationDocument.h"
#include "ui/AnnotationGraphicsItem.h"
#include "ui/ImageCanvas.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QUndoStack>

#include <cmath>

QString SelectTool::name() const
{
    return QCoreApplication::translate("Tool", "Выбор");
}

void SelectTool::mousePress(const QPointF &pos, QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    const int id = annotationAt(pos);
    context_.canvas->setSelectedId(id);
    if (id) {
        dragging_ = true;
        dragStart_ = pos;
    }
}

void SelectTool::mouseMove(const QPointF &pos, QMouseEvent *)
{
    if (!dragging_)
        return;
    // Во время перетаскивания сдвигается только графический элемент,
    // документ меняется один раз при отпускании кнопки.
    if (AnnotationGraphicsItem *item = context_.canvas->itemFor(context_.canvas->selectedId()))
        item->setPos(dragDelta(pos));
}

void SelectTool::mouseRelease(const QPointF &pos, QMouseEvent *event)
{
    if (!dragging_ || event->button() != Qt::LeftButton)
        return;
    dragging_ = false;

    const int id = context_.canvas->selectedId();
    if (AnnotationGraphicsItem *item = context_.canvas->itemFor(id))
        item->setPos(0, 0);

    const QPointF delta = dragDelta(pos);
    if (!delta.isNull())
        context_.undoStack->push(new MoveAnnotationCommand(context_.document, id, delta));
}

bool SelectTool::keyPress(QKeyEvent *event)
{
    const int id = context_.canvas->selectedId();
    if (!id)
        return false;

    const qreal step = (event->modifiers() & Qt::ShiftModifier) ? 10 : 1;
    QPointF delta;
    switch (event->key()) {
    case Qt::Key_Left:  delta = QPointF(-step, 0); break;
    case Qt::Key_Right: delta = QPointF(step, 0); break;
    case Qt::Key_Up:    delta = QPointF(0, -step); break;
    case Qt::Key_Down:  delta = QPointF(0, step); break;
    case Qt::Key_Escape:
        context_.canvas->setSelectedId(0);
        return true;
    default:
        return false;
    }
    context_.undoStack->push(new MoveAnnotationCommand(context_.document, id, delta));
    return true;
}

void SelectTool::cancel()
{
    if (dragging_) {
        if (AnnotationGraphicsItem *item = context_.canvas->itemFor(context_.canvas->selectedId()))
            item->setPos(0, 0);
        dragging_ = false;
    }
}

int SelectTool::annotationAt(const QPointF &pos) const
{
    // Допуск 6 пикселей на экране при любом масштабе.
    const qreal tolerance = 6.0 / context_.canvas->viewScale();
    const auto &annotations = context_.document->annotations();
    // Последние добавленные объекты рисуются поверх, поэтому ищем с конца.
    for (auto it = annotations.rbegin(); it != annotations.rend(); ++it) {
        if ((*it)->hitTest(pos, tolerance))
            return (*it)->id();
    }
    return 0;
}

QPointF SelectTool::dragDelta(const QPointF &pos) const
{
    const QPointF d = pos - dragStart_;
    return QPointF(std::round(d.x()), std::round(d.y()));
}
