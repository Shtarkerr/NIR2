#include "tools/ContourTool.h"

#include "model/ContourAnnotation.h"
#include "ui/ImageCanvas.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLineF>
#include <QMouseEvent>

QString ContourTool::name() const
{
    return QCoreApplication::translate("Tool", "Контур");
}

void ContourTool::mousePress(const QPointF &pos, QMouseEvent *event)
{
    moveBrush(pos, event->buttons());
    if (event->button() != Qt::LeftButton)
        return;

    const QPoint pixel = pixelAt(pos);
    if (!contour()) {
        auto created = std::make_unique<ContourAnnotation>();
        created->addVertex(pixel);
        created->setClosed(false);
        created->setWidth(context_.style.brushSize);
        startDraft(std::move(created));
        dragging_ = true;
        dragged_ = false;
        pressPos_ = pos;
        return;
    }

    // Режим многоугольника: каждый щелчок добавляет вершину.
    if (contour()->addVertex(pixel))
        draftChanged();
}

void ContourTool::mouseMove(const QPointF &pos, QMouseEvent *event)
{
    moveBrush(pos, event->buttons());
    if (!contour() || !dragging_ || !(event->buttons() & Qt::LeftButton))
        return;
    // Если рука дрогнула при щелчке, это ещё не рисование от руки. Порог в экранных
    // пикселях, иначе при уменьшенном масштабе сдвиг на один пиксель экрана
    // попадает в другой пиксель картинки и контур сразу завершается.
    const qreal moved = QLineF(pressPos_, pos).length() * context_.canvas->viewScale();
    if (!dragged_ && moved < QApplication::startDragDistance())
        return;
    if (contour()->addVertex(pixelAt(pos))) {
        dragged_ = true;
        draftChanged();
    }
}

void ContourTool::mouseRelease(const QPointF &pos, QMouseEvent *event)
{
    moveBrush(pos, event->buttons());
    if (!contour() || event->button() != Qt::LeftButton || !dragging_)
        return;
    dragging_ = false;
    // Если мышь двигали с зажатой кнопкой, контур нарисован от руки и готов.
    // Если нет, это был первый щелчок в режиме многоугольника.
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

void ContourTool::styleChanged()
{
    Tool::styleChanged();
    // Размер кисти можно менять и посреди рисования контура.
    if (contour()) {
        contour()->setWidth(context_.style.brushSize);
        draftChanged();
    }
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
    // Из двух вершин получается просто отрезок, его не замыкаем.
    contour()->setClosed(count >= 3);
    commitDraft();
}
