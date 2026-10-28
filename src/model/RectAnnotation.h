#pragma once

#include "model/FillableAnnotation.h"

// Прямоугольное выделение (рамкой или заливкой).
class RectAnnotation : public FillableAnnotation
{
public:
    RectAnnotation() = default;
    explicit RectAnnotation(const QRectF &rect) : rect_(rect.normalized()) {}

    QRectF rect() const { return rect_; }
    void setRect(const QRectF &rect) { rect_ = rect.normalized(); }

    Type type() const override { return Type::Rect; }
    QString typeName() const override { return QStringLiteral("rect"); }
    QString describe() const override;
    QRectF boundingRect() const override { return rect_; }
    bool hitTest(const QPointF &pos, qreal tolerance) const override;
    void translate(const QPointF &delta) override { rect_.translate(delta); }
    void paint(QPainter &painter, qreal scale) const override;
    std::unique_ptr<Annotation> clone() const override;

protected:
    void writeJsonGeometry(QJsonObject &obj) const override;
    bool readJsonGeometry(const QJsonObject &obj) override;
    void writeTxtGeometry(QTextStream &out) const override;

private:
    QRectF rect_;
};
