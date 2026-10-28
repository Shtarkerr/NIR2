#pragma once

#include <QDockWidget>

class AnnotationDocument;
class QTreeWidget;
class QTreeWidgetItem;
class QUndoStack;

// Панель со списком объектов разметки. Щелчок выбирает объект на холсте,
// двойной щелчок позволяет изменить метку.
class AnnotationListDock : public QDockWidget
{
    Q_OBJECT

public:
    AnnotationListDock(AnnotationDocument *document, QUndoStack *undoStack,
                       QWidget *parent = nullptr);

public slots:
    void setSelectedId(int id);

signals:
    void annotationSelected(int id);

private:
    void rebuild();
    void applySelection();
    void editLabel(QTreeWidgetItem *item);

    AnnotationDocument *document_;
    QUndoStack *undoStack_;
    QTreeWidget *tree_;
    int selectedId_ = 0;
    bool updating_ = false;
};
