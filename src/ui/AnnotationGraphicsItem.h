#pragma once

#include <QGraphicsItem>

class Annotation;

// Объект разметки на сцене. Сам объект не хранит, рисует через Annotation::paint().
// Границу элемента кэширует: Qt требует, чтобы boundingRect() менялся только после
// prepareGeometryChange(), иначе индекс сцены портится и программа может упасть.
class AnnotationGraphicsItem : public QGraphicsItem
{
public:
    enum { Type = UserType + 1 };

    explicit AnnotationGraphicsItem(const Annotation *annotation, QGraphicsItem *parent = nullptr);

    int type() const override { return Type; }

    void setHighlighted(bool highlighted);
    // Вызывается после изменения объекта или масштаба вида: пересчитывает границу.
    void refresh();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    // Масштаб нужен, чтобы подписи и маркеры не меняли размер на экране.
    static void setViewScale(qreal scale);
    static qreal viewScale() { return s_viewScale; }

private:
    QRectF labelRect() const;
    QRectF computeBounds() const;

    const Annotation *annotation_;
    QRectF bounds_;
    bool highlighted_ = false;

    static qreal s_viewScale;
};
