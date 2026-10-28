#pragma once

#include "model/Annotation.h"

#include <QPoint>
#include <QVector>

// Выделение произвольным контуром: ломаная по пикселям изображения.
class ContourAnnotation : public Annotation
{
public:
    ContourAnnotation() = default;
    explicit ContourAnnotation(const QVector<QPoint> &vertices, bool closed = true)
        : vertices_(vertices), closed_(closed) {}

    const QVector<QPoint> &vertices() const { return vertices_; }
    int vertexCount() const { return vertices_.size(); }
    // Если вершина совпадает с последней, она не добавляется и вернётся false.
    bool addVertex(const QPoint &vertex);
    void removeLastVertex();

    bool isClosed() const { return closed_; }
    void setClosed(bool closed) { closed_ = closed; }

    // Последовательные пиксели контура (алгоритм Брезенхэма между вершинами).
    QVector<QPoint> rasterize() const;

    Type type() const override { return Type::Contour; }
    QString typeName() const override { return QStringLiteral("contour"); }
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
    static QPointF pixelCenter(const QPoint &p) { return QPointF(p) + QPointF(0.5, 0.5); }

    QVector<QPoint> vertices_;
    bool closed_ = true;
};
