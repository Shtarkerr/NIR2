#pragma once

#include "model/Annotation.h"

#include <QObject>
#include <QSize>
#include <QStringList>

#include <memory>
#include <vector>

// Документ разметки: изображение, список объектов и метки.
// Об изменениях сообщает сигналами.
class AnnotationDocument : public QObject
{
    Q_OBJECT

public:
    using AnnotationList = std::vector<std::unique_ptr<Annotation>>;

    explicit AnnotationDocument(QObject *parent = nullptr);
    ~AnnotationDocument() override;

    QString imagePath() const { return imagePath_; }
    QSize imageSize() const { return imageSize_; }

    const AnnotationList &annotations() const { return annotations_; }
    int count() const { return static_cast<int>(annotations_.size()); }
    Annotation *find(int id) const;
    int indexOf(int id) const;

    int reserveId() { return nextId_++; }

    // Вставляет объект в позицию index (-1 значит в конец), возвращает его id.
    // Объекту без id (или с уже занятым id) назначается новый.
    int add(std::unique_ptr<Annotation> annotation, int index = -1);
    // Извлекает объект из документа; index получает его прежнюю позицию.
    std::unique_ptr<Annotation> take(int id, int *index = nullptr);

    void translate(int id, const QPointF &delta);
    void setLabel(int id, const QString &label);

    const QStringList &labels() const { return labels_; }
    void addLabel(const QString &label);
    void setLabels(const QStringList &labels);

    // Очищает документ и связывает его с новым изображением.
    void reset(const QString &imagePath = QString(), const QSize &imageSize = QSize());

signals:
    void annotationAdded(int id);
    void annotationRemoved(int id);
    void annotationChanged(int id);
    void documentReset();
    void labelsChanged();

private:
    QString imagePath_;
    QSize imageSize_;
    AnnotationList annotations_;
    QStringList labels_;
    int nextId_ = 1;
};
