#pragma once

#include <QDockWidget>

class Annotation;
class AnnotationDocument;
class QAction;
class QTreeWidget;
class QTreeWidgetItem;
class QUndoStack;

// Панель со списком объектов разметки. Несколько строк выделяются протягиванием
// мыши, а также с Shift и Ctrl. Двойной щелчок позволяет изменить метку.
class AnnotationListDock : public QDockWidget
{
    Q_OBJECT

public:
    AnnotationListDock(AnnotationDocument *document, QUndoStack *undoStack,
                       QWidget *parent = nullptr);

    // Действие для контекстного меню списка (например, удаление).
    void addContextAction(QAction *action);

public slots:
    void setSelectedIds(const QList<int> &ids);

signals:
    // Пользователь изменил выделение в списке.
    void selectionChanged(const QList<int> &ids);

private:
    // Строки обновляются по одной, полностью список строится только при смене документа.
    void rebuild();
    void addRow(int id);
    void updateRow(int id);
    void removeRow(int id);
    QTreeWidgetItem *rowFor(int id) const;
    void fillRow(QTreeWidgetItem *item, const Annotation &annotation) const;
    void applySelection();
    void editLabel(QTreeWidgetItem *item);

    AnnotationDocument *document_;
    QUndoStack *undoStack_;
    QTreeWidget *tree_;
    QList<int> selectedIds_;
    bool updating_ = false;
};
