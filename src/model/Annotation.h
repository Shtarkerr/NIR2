#pragma once

#include <QColor>
#include <QJsonObject>
#include <QPointF>
#include <QRectF>
#include <QString>

#include <memory>

class QPainter;
class QPen;
class QTextStream;

// Базовый класс объектов разметки. Координаты в пикселях исходного изображения.
// toJson()/fromJson()/writeTxt() пишут общие поля, геометрию добавляют наследники.
class Annotation
{
public:
    enum class Type { Point, Rect, Ellipse, Contour };

    virtual ~Annotation() = default;

    int id() const { return id_; }
    void setId(int id) { id_ = id; }

    const QString &label() const { return label_; }
    void setLabel(const QString &label) { label_ = label; }

    const QColor &color() const { return color_; }
    void setColor(const QColor &color) { color_ = color; }

    virtual Type type() const = 0;
    // Имя типа в файлах: "point", "rect", "ellipse", "contour".
    virtual QString typeName() const = 0;
    // Название типа для человека: "Точка", "Прямоугольник" и т. д.
    virtual QString displayName() const = 0;
    // Краткое описание геометрии для списка объектов.
    virtual QString describe() const = 0;
    virtual QRectF boundingRect() const = 0;
    // Попадает ли точка pos в объект с допуском tolerance.
    virtual bool hitTest(const QPointF &pos, qreal tolerance) const = 0;
    virtual void translate(const QPointF &delta) = 0;
    // scale - текущий масштаб вида, нужен чтобы маркеры не меняли размер на экране.
    virtual void paint(QPainter &painter, qreal scale) const = 0;
    virtual std::unique_ptr<Annotation> clone() const = 0;

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &obj);
    void writeTxt(QTextStream &out) const;

    static QString colorToString(const QColor &color);
    static QString formatNumber(qreal value);
    // Точка в виде "(x, y)".
    static QString formatPoint(const QPointF &point);

protected:
    Annotation() = default;
    Annotation(const Annotation &) = default;
    Annotation &operator=(const Annotation &) = default;

    virtual void writeJsonGeometry(QJsonObject &obj) const = 0;
    virtual bool readJsonGeometry(const QJsonObject &obj) = 0;
    virtual void writeTxtGeometry(QTextStream &out) const = 0;

    QPen outlinePen() const;
    // Линия толщиной width пикселей изображения при масштабе вида scale.
    QPen strokePen(int width, qreal scale) const;

private:
    int id_ = 0;
    QString label_;
    QColor color_ = Qt::red;
};
