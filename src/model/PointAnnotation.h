#pragma once

#include "model/Annotation.h"

#include <QPoint>

// Отмеченная точка: кружок с центром в пикселе pos.
class PointAnnotation : public Annotation
{
public:
    PointAnnotation() = default;
    explicit PointAnnotation(const QPoint &pos) : pos_(pos) {}

    QPoint pos() const { return pos_; }
    void setPos(const QPoint &pos) { pos_ = pos; }

    // Диаметр точки (размер кисти) в пикселях изображения.
    int size() const { return size_; }
    void setSize(int size) { size_ = size < 1 ? 1 : size; }

    Type type() const override { return Type::Point; }
    QString typeName() const override { return QStringLiteral("point"); }
    QString displayName() const override { return QStringLiteral("Точка"); }
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
    int size_ = 1;
};
