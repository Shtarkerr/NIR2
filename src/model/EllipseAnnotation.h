#pragma once

#include "model/FillableAnnotation.h"

// Выделение эллипсом с осями, параллельными осям изображения.
class EllipseAnnotation : public FillableAnnotation
{
public:
    EllipseAnnotation() = default;
    EllipseAnnotation(const QPointF &center, qreal rx, qreal ry)
        : center_(center), rx_(rx), ry_(ry) {}

    QPointF center() const { return center_; }
    qreal radiusX() const { return rx_; }
    qreal radiusY() const { return ry_; }
    // Эллипс, вписанный в прямоугольник.
    void setFromRect(const QRectF &rect);

    Type type() const override { return Type::Ellipse; }
    QString typeName() const override { return QStringLiteral("ellipse"); }
    QString describe() const override;
    QRectF boundingRect() const override;
    bool hitTest(const QPointF &pos, qreal tolerance) const override;
    void translate(const QPointF &delta) override { center_ += delta; }
    void paint(QPainter &painter, qreal scale) const override;
    std::unique_ptr<Annotation> clone() const override;

protected:
    void writeJsonGeometry(QJsonObject &obj) const override;
    bool readJsonGeometry(const QJsonObject &obj) override;
    void writeTxtGeometry(QTextStream &out) const override;

private:
    QPointF center_;
    qreal rx_ = 0;
    qreal ry_ = 0;
};
