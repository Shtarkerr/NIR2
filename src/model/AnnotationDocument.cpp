#include "model/AnnotationDocument.h"

#include <algorithm>

AnnotationDocument::AnnotationDocument(QObject *parent)
    : QObject(parent)
{
}

AnnotationDocument::~AnnotationDocument() = default;

Annotation *AnnotationDocument::find(int id) const
{
    const int index = indexOf(id);
    return index < 0 ? nullptr : annotations_[static_cast<size_t>(index)].get();
}

int AnnotationDocument::indexOf(int id) const
{
    const auto it = std::find_if(annotations_.begin(), annotations_.end(),
                                 [id](const auto &a) { return a->id() == id; });
    return it == annotations_.end() ? -1 : static_cast<int>(it - annotations_.begin());
}

int AnnotationDocument::add(std::unique_ptr<Annotation> annotation, int index)
{
    if (!annotation)
        return 0;

    if (annotation->id() <= 0 || find(annotation->id()))
        annotation->setId(reserveId());
    else
        nextId_ = std::max(nextId_, annotation->id() + 1);

    const int id = annotation->id();
    addLabel(annotation->label());

    if (index < 0 || index > count())
        index = count();
    annotations_.insert(annotations_.begin() + index, std::move(annotation));

    emit annotationAdded(id);
    return id;
}

std::unique_ptr<Annotation> AnnotationDocument::take(int id, int *index)
{
    const int i = indexOf(id);
    if (index)
        *index = i;
    if (i < 0)
        return nullptr;

    // Сигнал отправляется до удаления из списка: подписчики ещё могут обратиться к объекту.
    emit annotationRemoved(id);
    auto it = annotations_.begin() + i;
    std::unique_ptr<Annotation> annotation = std::move(*it);
    annotations_.erase(it);
    return annotation;
}

void AnnotationDocument::translate(int id, const QPointF &delta)
{
    if (Annotation *annotation = find(id)) {
        annotation->translate(delta);
        emit annotationChanged(id);
    }
}

void AnnotationDocument::setLabel(int id, const QString &label)
{
    if (Annotation *annotation = find(id)) {
        annotation->setLabel(label);
        addLabel(label);
        emit annotationChanged(id);
    }
}

void AnnotationDocument::addLabel(const QString &label)
{
    const QString trimmed = label.trimmed();
    if (trimmed.isEmpty() || labels_.contains(trimmed))
        return;
    labels_.append(trimmed);
    emit labelsChanged();
}

void AnnotationDocument::setLabels(const QStringList &labels)
{
    labels_.clear();
    for (const QString &label : labels) {
        const QString trimmed = label.trimmed();
        if (!trimmed.isEmpty() && !labels_.contains(trimmed))
            labels_.append(trimmed);
    }
    emit labelsChanged();
}

void AnnotationDocument::reset(const QString &imagePath, const QSize &imageSize,
                               const QColor &canvasColor)
{
    // Старые объекты живут до конца функции: подписчики documentReset (холст)
    // ещё держат на них указатели и должны успеть от них отказаться.
    AnnotationList old;
    old.swap(annotations_);

    imagePath_ = imagePath;
    imageSize_ = imageSize;
    canvasColor_ = canvasColor;
    labels_.clear();
    nextId_ = 1;
    emit documentReset();
    emit labelsChanged();
}
