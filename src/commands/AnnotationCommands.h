#pragma once

#include "model/Annotation.h"

#include <QUndoCommand>

#include <memory>

class AnnotationDocument;

// Команды изменения документа для отмены и повтора действий.

class AddAnnotationCommand : public QUndoCommand
{
public:
    AddAnnotationCommand(AnnotationDocument *doc, std::unique_ptr<Annotation> annotation,
                         QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    AnnotationDocument *doc_;
    std::unique_ptr<Annotation> annotation_;
    int id_;
    int index_ = -1;
};

class RemoveAnnotationCommand : public QUndoCommand
{
public:
    RemoveAnnotationCommand(AnnotationDocument *doc, int id, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    AnnotationDocument *doc_;
    std::unique_ptr<Annotation> annotation_;
    int id_;
    int index_ = -1;
};

class MoveAnnotationCommand : public QUndoCommand
{
public:
    MoveAnnotationCommand(AnnotationDocument *doc, int id, const QPointF &delta,
                          QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    AnnotationDocument *doc_;
    int id_;
    QPointF delta_;
};

class SetLabelCommand : public QUndoCommand
{
public:
    SetLabelCommand(AnnotationDocument *doc, int id, const QString &label,
                    QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    AnnotationDocument *doc_;
    int id_;
    QString oldLabel_;
    QString newLabel_;
};
