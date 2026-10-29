#include "ui/AnnotationListDock.h"

#include "commands/AnnotationCommands.h"
#include "model/AnnotationDocument.h"
#include "ui/Icons.h"

#include <QAction>
#include <QHeaderView>
#include <QInputDialog>
#include <QTreeWidget>
#include <QUndoStack>

namespace {

enum Column { IdColumn, TypeColumn, LabelColumn, GeometryColumn, ColumnCount };

} // namespace

AnnotationListDock::AnnotationListDock(AnnotationDocument *document, QUndoStack *undoStack,
                                       QWidget *parent)
    : QDockWidget(tr("Объекты"), parent),
      document_(document),
      undoStack_(undoStack),
      tree_(new QTreeWidget(this))
{
    setObjectName(QStringLiteral("AnnotationListDock"));
    tree_->setColumnCount(ColumnCount);
    tree_->setHeaderLabels({tr("№"), tr("Тип"), tr("Метка"), tr("Геометрия")});
    tree_->setRootIsDecorated(false);
    // Протягивание мышью по строкам выделяет их все, Shift и Ctrl тоже работают.
    tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_->setContextMenuPolicy(Qt::ActionsContextMenu);
    tree_->setAlternatingRowColors(true);
    // Колонки умещаются в ширину панели, длинный текст обрезается многоточием.
    // Без горизонтальной прокрутки список не уезжает вбок при выделении протягиванием.
    tree_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tree_->setTextElideMode(Qt::ElideRight);
    tree_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tree_->header()->setSectionResizeMode(LabelColumn, QHeaderView::Interactive);
    tree_->header()->resizeSection(LabelColumn, 120);
    tree_->header()->setStretchLastSection(true);
    setWidget(tree_);

    connect(document_, &AnnotationDocument::annotationAdded, this, &AnnotationListDock::addRow);
    connect(document_, &AnnotationDocument::annotationChanged, this, &AnnotationListDock::updateRow);
    connect(document_, &AnnotationDocument::annotationRemoved, this, &AnnotationListDock::removeRow);
    connect(document_, &AnnotationDocument::documentReset, this, &AnnotationListDock::rebuild);

    connect(tree_, &QTreeWidget::itemSelectionChanged, this, [this] {
        if (updating_)
            return;
        // Идём по строкам, а не по selectedItems(), чтобы порядок совпадал со списком.
        selectedIds_.clear();
        for (int i = 0; i < tree_->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = tree_->topLevelItem(i);
            if (item->isSelected())
                selectedIds_.append(item->data(IdColumn, Qt::UserRole).toInt());
        }
        emit selectionChanged(selectedIds_);
    });
    connect(tree_, &QTreeWidget::itemDoubleClicked, this,
            [this](QTreeWidgetItem *item) { editLabel(item); });

    rebuild();
}

void AnnotationListDock::addContextAction(QAction *action)
{
    tree_->addAction(action);
}

void AnnotationListDock::setSelectedIds(const QList<int> &ids)
{
    if (ids == selectedIds_)
        return;
    selectedIds_ = ids;
    applySelection();
}

void AnnotationListDock::applySelection()
{
    updating_ = true;
    tree_->clearSelection();
    QTreeWidgetItem *first = nullptr;
    for (int i = 0; i < tree_->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = tree_->topLevelItem(i);
        if (selectedIds_.contains(item->data(IdColumn, Qt::UserRole).toInt())) {
            item->setSelected(true);
            if (!first)
                first = item;
        }
    }
    if (first)
        tree_->scrollToItem(first);
    updating_ = false;
}

void AnnotationListDock::rebuild()
{
    updating_ = true;
    tree_->clear();
    for (const auto &annotation : document_->annotations()) {
        auto *item = new QTreeWidgetItem(tree_);
        fillRow(item, *annotation);
    }
    updating_ = false;
    selectedIds_.clear();
}

void AnnotationListDock::addRow(int id)
{
    const Annotation *annotation = document_->find(id);
    if (!annotation)
        return;
    auto *item = new QTreeWidgetItem;
    fillRow(item, *annotation);
    // Строки идут в том же порядке, что и объекты в документе (важно для отмены удаления).
    tree_->insertTopLevelItem(document_->indexOf(id), item);
}

void AnnotationListDock::updateRow(int id)
{
    QTreeWidgetItem *item = rowFor(id);
    const Annotation *annotation = document_->find(id);
    if (item && annotation)
        fillRow(item, *annotation);
}

void AnnotationListDock::removeRow(int id)
{
    selectedIds_.removeAll(id);
    // Холст сам снимет выделение с удалённого объекта, поэтому сигнал не отправляем.
    updating_ = true;
    delete rowFor(id);
    updating_ = false;
}

QTreeWidgetItem *AnnotationListDock::rowFor(int id) const
{
    for (int i = 0; i < tree_->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = tree_->topLevelItem(i);
        if (item->data(IdColumn, Qt::UserRole).toInt() == id)
            return item;
    }
    return nullptr;
}

void AnnotationListDock::fillRow(QTreeWidgetItem *item, const Annotation &annotation) const
{
    item->setData(IdColumn, Qt::UserRole, annotation.id());
    item->setText(IdColumn, QString::number(annotation.id()));
    item->setIcon(TypeColumn, Icons::colorSwatch(annotation.color(), 12));
    item->setText(TypeColumn, annotation.displayName());
    item->setText(LabelColumn, annotation.label());
    item->setText(GeometryColumn, annotation.describe());
    item->setToolTip(GeometryColumn, annotation.describe());
}

void AnnotationListDock::editLabel(QTreeWidgetItem *item)
{
    const int id = item->data(IdColumn, Qt::UserRole).toInt();
    const Annotation *annotation = document_->find(id);
    if (!annotation)
        return;

    QStringList choices = QStringList(annotation->label()) + document_->labels();
    choices.removeDuplicates();

    bool ok = false;
    const QString label = QInputDialog::getItem(
        this, tr("Метка объекта"), tr("Метка объекта #%1:").arg(id),
        choices, 0, true, &ok).trimmed();
    if (ok && label != annotation->label())
        undoStack_->push(new SetLabelCommand(document_, id, label));
}
