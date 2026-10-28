#include "tools/ContourTool.h"

#include "model/ContourAnnotation.h"

#include "ui/ImageCanvas.h"

#include <QCoreApplication>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMouseEvent>

#include <cmath>

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

void ContourTool::deactivate()
{
    cancel();
    brush_.reset();
}

void ContourTool::leave()
{
    if (brush_)
        brush_->hide();
}

void ContourTool::styleChanged()
{
    if (brush_)
        brush_->setDiameter(context_.style.brushSize);
    // Размер кисти можно менять и посреди рисования контура.
    if (contour()) {
        contour()->setWidth(context_.style.brushSize);
        draftChanged();
    }
}

void ContourTool::moveBrush(const QPointF &pos, Qt::MouseButtons buttons)
{
    // За пределами картинки кисть не нужна, там холст показывает обычный курсор.
    // Пока кнопка зажата, кисть остаётся видна, чтобы не пропадать посреди штриха.
    const bool drawing = buttons & Qt::LeftButton;
    if (!drawing && !context_.canvas->imageRect().contains(pos)) {
        if (brush_)
            brush_->hide();
        return;
    }

    if (!brush_) {
        brush_ = std::make_unique<BrushCursorItem>();
        context_.canvas->scene()->addItem(brush_.get());
    }
    brush_->setDiameter(context_.style.brushSize);
    // Линия проходит через центры пикселей, туда же ставим кисть. Координату не
    // ограничиваем краями картинки, иначе кисть прилипает к краю, когда мышь уходит.
    brush_->setPos(QPointF(std::floor(pos.x()) + 0.5, std::floor(pos.y()) + 0.5));
    brush_->show();
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
