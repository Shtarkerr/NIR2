#include "commands/AnnotationCommands.h"

#include "model/AnnotationDocument.h"

AddAnnotationCommand::AddAnnotationCommand(AnnotationDocument *doc,
                                           std::unique_ptr<Annotation> annotation,
                                           QUndoCommand *parent)
    : QUndoCommand(parent), doc_(doc), annotation_(std::move(annotation))
{
    // id назначается заранее, чтобы undo/redo всегда работали с одним и тем же объектом.
    if (annotation_->id() <= 0 || doc_->find(annotation_->id()))
        annotation_->setId(doc_->reserveId());
    id_ = annotation_->id();
    setText(QObject::tr("Добавление объекта #%1").arg(id_));
}

void AddAnnotationCommand::redo()
{
    doc_->add(std::move(annotation_), index_);
}

void AddAnnotationCommand::undo()
{
    annotation_ = doc_->take(id_, &index_);
}

RemoveAnnotationCommand::RemoveAnnotationCommand(AnnotationDocument *doc, int id,
                                                 QUndoCommand *parent)
    : QUndoCommand(parent), doc_(doc), id_(id)
{
    setText(QObject::tr("Удаление объекта #%1").arg(id_));
}

void RemoveAnnotationCommand::redo()
{
    annotation_ = doc_->take(id_, &index_);
}

void RemoveAnnotationCommand::undo()
{
    doc_->add(std::move(annotation_), index_);
}

MoveAnnotationCommand::MoveAnnotationCommand(AnnotationDocument *doc, int id,
                                             const QPointF &delta, QUndoCommand *parent)
    : QUndoCommand(parent), doc_(doc), id_(id), delta_(delta)
{
    setText(QObject::tr("Перемещение объекта #%1").arg(id_));
}

void MoveAnnotationCommand::redo()
{
    doc_->translate(id_, delta_);
}

void MoveAnnotationCommand::undo()
{
    doc_->translate(id_, -delta_);
}

SetLabelCommand::SetLabelCommand(AnnotationDocument *doc, int id, const QString &label,
                                 QUndoCommand *parent)
    : QUndoCommand(parent), doc_(doc), id_(id), newLabel_(label)
{
    if (const Annotation *annotation = doc_->find(id_))
        oldLabel_ = annotation->label();
    setText(QObject::tr("Изменение метки объекта #%1").arg(id_));
}

void SetLabelCommand::redo()
{
    doc_->setLabel(id_, newLabel_);
}

void SetLabelCommand::undo()
{
    doc_->setLabel(id_, oldLabel_);
}
