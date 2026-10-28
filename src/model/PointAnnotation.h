#pragma once

#include "model/Annotation.h"

#include <QPoint>

// Отметка отдельного пикселя изображения.
class PointAnnotation : public Annotation
{
public:
    PointAnnotation() = default;
    explicit PointAnnotation(const QPoint &pos) : pos_(pos) {}

    QPoint pos() const { return pos_; }
    void setPos(const QPoint &pos) { pos_ = pos; }

    Type type() const override { return Type::Point; }
    QString typeName() const override { return QStringLiteral("point"); }
    QString describe() const override;
    QRectF boundingRect() const override;
    bool hitTest(const QPointF &pos, qreal tolerance) const override;
    void translate(const QPointF &delta) override;
    void paint(QPainter &painter, qreal scale) const override;
    std::unique_ptr<Annotation> clone() const override;

protected:
    void writeJsonGeometry(QJsonObject &obj) const override;
    bool readJsonGeometry(const QJsonObject &obj) override;
    void writeTxtGeometry(QTextStream &out) const override;

private:
    QPointF center() const { return QPointF(pos_) + QPointF(0.5, 0.5); }

    QPoint pos_;
};
