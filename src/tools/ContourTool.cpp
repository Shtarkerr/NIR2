#include "tools/ContourTool.h"

#include "model/ContourAnnotation.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QMouseEvent>

QString ContourTool::name() const
{
    return QCoreApplication::translate("Tool", "Контур");
}

void ContourTool::mousePress(const QPointF &pos, QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    const QPoint pixel = pixelAt(pos);
    if (!contour()) {
        auto created = std::make_unique<ContourAnnotation>();
        created->addVertex(pixel);
        created->setClosed(false);
        startDraft(std::move(created));
        dragging_ = true;
        dragged_ = false;
        return;
    }

    // Режим многоугольника: каждый щелчок добавляет вершину.
    if (contour()->addVertex(pixel))
        draftChanged();
}

void ContourTool::mouseMove(const QPointF &pos, QMouseEvent *event)
{
    if (!contour() || !dragging_ || !(event->buttons() & Qt::LeftButton))
        return;
    if (contour()->addVertex(pixelAt(pos))) {
        dragged_ = true;
        draftChanged();
    }
}

void ContourTool::mouseRelease(const QPointF &, QMouseEvent *event)
{
    if (!contour() || event->button() != Qt::LeftButton || !dragging_)
        return;
    dragging_ = false;
    // Если мышь двигали с зажатой кнопкой — контур нарисован от руки и готов.
    // Иначе это был первый щелчок многоугольника.
    if (dragged_)
        finish();
}

void ContourTool::mouseDoubleClick(const QPointF &, QMouseEvent *event)
{
    if (contour() && event->button() == Qt::LeftButton)
        finish();
}

bool ContourTool::keyPress(QKeyEvent *event)
{
    if (!contour())
        return false;

    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        finish();
        return true;
    case Qt::Key_Escape:
        cancel();
        return true;
    case Qt::Key_Backspace:
        contour()->removeLastVertex();
        if (contour()->vertexCount() == 0)
            cancel();
        else
            draftChanged();
        return true;
    default:
        return false;
    }
}

void ContourTool::cancel()
{
    dragging_ = false;
    dragged_ = false;
    Tool::cancel();
}

ContourAnnotation *ContourTool::contour() const
{
    return static_cast<ContourAnnotation *>(draft());
}

void ContourTool::finish()
{
    dragging_ = false;
    dragged_ = false;

    const int count = contour()->vertexCount();
    if (count < 2) {
        discardDraft();
        return;
    }
    // Две вершины — это отрезок, а не замкнутая область.
    contour()->setClosed(count >= 3);
    commitDraft();
}
