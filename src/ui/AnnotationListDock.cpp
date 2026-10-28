#include "ui/AnnotationListDock.h"

#include "commands/AnnotationCommands.h"
#include "model/AnnotationDocument.h"
#include "ui/Icons.h"

#include <QHeaderView>
#include <QInputDialog>
#include <QTreeWidget>
#include <QUndoStack>

namespace {

enum Column { IdColumn, TypeColumn, LabelColumn, GeometryColumn, ColumnCount };

QString displayTypeName(Annotation::Type type)
{
    switch (type) {
    case Annotation::Type::Point:   return AnnotationListDock::tr("Точка");
    case Annotation::Type::Rect:    return AnnotationListDock::tr("Прямоугольник");
    case Annotation::Type::Ellipse: return AnnotationListDock::tr("Эллипс");
    case Annotation::Type::Contour: return AnnotationListDock::tr("Контур");
    }
    return QString();
}

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
    tree_->setSelectionMode(QAbstractItemView::SingleSelection);
    tree_->setAlternatingRowColors(true);
    tree_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tree_->header()->setStretchLastSection(true);
    setWidget(tree_);

    connect(document_, &AnnotationDocument::annotationAdded, this, &AnnotationListDock::rebuild);
    connect(document_, &AnnotationDocument::annotationChanged, this, &AnnotationListDock::rebuild);
    connect(document_, &AnnotationDocument::documentReset, this, &AnnotationListDock::rebuild);
    // annotationRemoved приходит до удаления объекта из документа — откладываем перестройку.
    connect(document_, &AnnotationDocument::annotationRemoved, this,
            &AnnotationListDock::rebuild, Qt::QueuedConnection);

    connect(tree_, &QTreeWidget::itemSelectionChanged, this, [this] {
        if (updating_)
            return;
        const QList<QTreeWidgetItem *> selected = tree_->selectedItems();
        selectedId_ = selected.isEmpty() ? 0 : selected.first()->data(IdColumn, Qt::UserRole).toInt();
        emit annotationSelected(selectedId_);
    });
    connect(tree_, &QTreeWidget::itemDoubleClicked, this,
            [this](QTreeWidgetItem *item) { editLabel(item); });

    rebuild();
}

void AnnotationListDock::setSelectedId(int id)
{
    if (id == selectedId_)
        return;
    selectedId_ = id;
    applySelection();
}

void AnnotationListDock::applySelection()
{
    updating_ = true;
    tree_->clearSelection();
    for (int i = 0; i < tree_->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = tree_->topLevelItem(i);
        if (item->data(IdColumn, Qt::UserRole).toInt() == selectedId_) {
            item->setSelected(true);
            tree_->scrollToItem(item);
            break;
        }
    }
    updating_ = false;
}

void AnnotationListDock::rebuild()
{
    updating_ = true;
    tree_->clear();
    for (const auto &annotation : document_->annotations()) {
        auto *item = new QTreeWidgetItem(tree_);
        item->setData(IdColumn, Qt::UserRole, annotation->id());
        item->setText(IdColumn, QString::number(annotation->id()));
        item->setIcon(TypeColumn, Icons::colorSwatch(annotation->color(), 12));
        item->setText(TypeColumn, displayTypeName(annotation->type()));
        item->setText(LabelColumn, annotation->label());
        item->setText(GeometryColumn, annotation->describe());
    }
    updating_ = false;
    if (!document_->find(selectedId_))
        selectedId_ = 0;
    applySelection();
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
