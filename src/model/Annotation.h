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

// Базовый класс всех объектов разметки.
// Все координаты задаются в пикселях исходного изображения.
//
// Сериализация построена по шаблонному методу: toJson()/fromJson()/writeTxt()
// обрабатывают общие поля (id, метка, цвет), а геометрию дописывают наследники.
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
    // Имя типа в файлах разметки: "point", "rect", "ellipse", "contour".
    virtual QString typeName() const = 0;
    // Краткое описание геометрии для списка объектов.
    virtual QString describe() const = 0;
    virtual QRectF boundingRect() const = 0;
    // Попадает ли точка pos в объект с допуском tolerance (в пикселях изображения).
    virtual bool hitTest(const QPointF &pos, qreal tolerance) const = 0;
    virtual void translate(const QPointF &delta) = 0;
    // scale — текущий масштаб вида; нужен для элементов постоянного экранного размера.
    virtual void paint(QPainter &painter, qreal scale) const = 0;
    virtual std::unique_ptr<Annotation> clone() const = 0;

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &obj);
    void writeTxt(QTextStream &out) const;

    static QString colorToString(const QColor &color);
    static QString formatNumber(qreal value);

protected:
    Annotation() = default;
    Annotation(const Annotation &) = default;
    Annotation &operator=(const Annotation &) = default;

    virtual void writeJsonGeometry(QJsonObject &obj) const = 0;
    virtual bool readJsonGeometry(const QJsonObject &obj) = 0;
    virtual void writeTxtGeometry(QTextStream &out) const = 0;

    QPen outlinePen() const;

private:
    int id_ = 0;
    QString label_;
    QColor color_ = Qt::red;
};
