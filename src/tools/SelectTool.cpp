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

    ImageCanvas *canvas = context_.canvas;
    const int id = annotationAt(pos);

    if (event->modifiers() & Qt::ControlModifier) {
        if (id) {
            QList<int> ids = canvas->selectedIds();
            if (ids.contains(id))
                ids.removeAll(id);
            else
                ids.append(id);
            canvas->setSelectedIds(ids);
        }
        return;
    }

    // Щелчок по уже выделенному объекту не сбрасывает группу, чтобы её можно было перетащить.
    if (!canvas->isSelected(id))
        canvas->setSelectedId(id);
    if (id) {
        dragging_ = true;
        dragStart_ = pos;
    }
}

void SelectTool::mouseMove(const QPointF &pos, QMouseEvent *)
{
    // Во время перетаскивания сдвигаются только графические элементы,
    // документ меняется один раз при отпускании кнопки.
    if (dragging_)
        offsetSelectedItems(dragDelta(pos));
}

void SelectTool::mouseRelease(const QPointF &pos, QMouseEvent *event)
{
    if (!dragging_ || event->button() != Qt::LeftButton)
        return;
    dragging_ = false;
    offsetSelectedItems(QPointF());
    moveSelection(dragDelta(pos));
}

bool SelectTool::keyPress(QKeyEvent *event)
{
    if (context_.canvas->selectedIds().isEmpty())
        return false;

    const qreal step = (event->modifiers() & Qt::ShiftModifier) ? 10 : 1;
    switch (event->key()) {
    case Qt::Key_Left:  moveSelection(QPointF(-step, 0)); return true;
    case Qt::Key_Right: moveSelection(QPointF(step, 0)); return true;
    case Qt::Key_Up:    moveSelection(QPointF(0, -step)); return true;
    case Qt::Key_Down:  moveSelection(QPointF(0, step)); return true;
    case Qt::Key_Escape:
        context_.canvas->setSelectedId(0);
        return true;
    default:
        return false;
    }
}

void SelectTool::cancel()
{
    if (dragging_) {
        offsetSelectedItems(QPointF());
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

void SelectTool::offsetSelectedItems(const QPointF &offset)
{
    for (int id : context_.canvas->selectedIds()) {
        if (AnnotationGraphicsItem *item = context_.canvas->itemFor(id))
            item->setPos(offset);
    }
}

void SelectTool::moveSelection(const QPointF &delta)
{
    const QList<int> ids = context_.canvas->selectedIds();
    if (delta.isNull() || ids.isEmpty())
        return;

    QUndoStack *undo = context_.undoStack;
    if (ids.size() > 1)
        undo->beginMacro(QCoreApplication::translate("Tool", "Перемещение объектов: %1").arg(ids.size()));
    for (int id : ids)
        undo->push(new MoveAnnotationCommand(context_.document, id, delta));
    if (ids.size() > 1)
        undo->endMacro();
}
