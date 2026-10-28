#pragma once

#include <QGraphicsItem>

class Annotation;

// Графическое представление объекта разметки на сцене.
// Не владеет объектом: рисование делегируется Annotation::paint().
class AnnotationGraphicsItem : public QGraphicsItem
{
public:
    enum { Type = UserType + 1 };

    explicit AnnotationGraphicsItem(const Annotation *annotation, QGraphicsItem *parent = nullptr);

    int type() const override { return Type; }
    const Annotation *annotation() const { return annotation_; }
    int annotationId() const;

    void setHighlighted(bool highlighted);
    // Вызывается после изменения геометрии объекта или масштаба вида.
    void refresh();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    // Масштаб вида нужен, чтобы подписи и маркеры имели постоянный экранный размер.
    static void setViewScale(qreal scale);
    static qreal viewScale() { return s_viewScale; }

private:
    QRectF labelRect() const;

    const Annotation *annotation_;
    bool highlighted_ = false;

    static qreal s_viewScale;
};
