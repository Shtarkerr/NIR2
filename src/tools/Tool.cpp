#include "tools/Tool.h"

#include "commands/AnnotationCommands.h"
#include "model/FillableAnnotation.h"
#include "ui/AnnotationGraphicsItem.h"
#include "ui/ImageCanvas.h"

#include <QGraphicsScene>
#include <QUndoStack>

#include <algorithm>
#include <cmath>

Tool::Tool(ToolContext &context)
    : context_(context)
{
}

Tool::~Tool() = default;

void Tool::mousePress(const QPointF &, QMouseEvent *) {}
void Tool::mouseMove(const QPointF &, QMouseEvent *) {}
void Tool::mouseRelease(const QPointF &, QMouseEvent *) {}
void Tool::mouseDoubleClick(const QPointF &, QMouseEvent *) {}

bool Tool::keyPress(QKeyEvent *)
{
    return false;
}

void Tool::cancel()
{
    discardDraft();
}

QPointF Tool::clampToImage(const QPointF &pos) const
{
    const QRectF r = context_.canvas->imageRect();
    return QPointF(std::clamp(pos.x(), r.left(), r.right()),
                   std::clamp(pos.y(), r.top(), r.bottom()));
}

QPoint Tool::pixelAt(const QPointF &pos) const
{
    const QSize size = context_.canvas->imageSize();
    const int x = static_cast<int>(std::floor(pos.x()));
    const int y = static_cast<int>(std::floor(pos.y()));
    return QPoint(std::clamp(x, 0, std::max(0, size.width() - 1)),
                  std::clamp(y, 0, std::max(0, size.height() - 1)));
}

void Tool::startDraft(std::unique_ptr<Annotation> draft)
{
    discardDraft();

    draft->setColor(context_.style.color);
    draft->setLabel(context_.style.label);
    if (auto *fillable = dynamic_cast<FillableAnnotation *>(draft.get()))
        fillable->setFilled(context_.style.filled);

    draft_ = std::move(draft);
    draftItem_ = std::make_unique<AnnotationGraphicsItem>(draft_.get());
    draftItem_->setZValue(2);
    context_.canvas->scene()->addItem(draftItem_.get());
}

void Tool::draftChanged()
{
    if (draftItem_)
        draftItem_->refresh();
}

void Tool::commitDraft()
{
    if (!draft_)
        return;
    draftItem_.reset();
    context_.undoStack->push(new AddAnnotationCommand(context_.document, std::move(draft_)));
}

void Tool::discardDraft()
{
    draftItem_.reset();
    draft_.reset();
}
