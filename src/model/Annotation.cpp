#include "model/Annotation.h"

#include <QJsonValue>
#include <QPen>
#include <QTextStream>

QJsonObject Annotation::toJson() const
{
    QJsonObject obj;
    obj["id"] = id_;
    obj["type"] = typeName();
    obj["label"] = label_;
    obj["color"] = colorToString(color_);
    writeJsonGeometry(obj);
    return obj;
}

bool Annotation::fromJson(const QJsonObject &obj)
{
    if (obj.value("type").toString() != typeName())
        return false;

    const QColor color(obj.value("color").toString());
    if (!color.isValid())
        return false;

    id_ = obj.value("id").toInt(0);
    label_ = obj.value("label").toString();
    color_ = color;
    return readJsonGeometry(obj);
}

void Annotation::writeTxt(QTextStream &out) const
{
    // Перевод строки в метке разорвал бы описание объекта, поэтому он становится пробелом.
    const QString label = label_.simplified();
    QString color = color_.name(QColor::HexRgb);
    if (color_.alpha() != 255)
        color += QStringLiteral(", непрозрачность %1%").arg(qRound(color_.alphaF() * 100));

    // В Qt 5 operator<<(const char *) считает строку Latin-1, поэтому кириллица через QStringLiteral.
    out << QStringLiteral("Объект №%1. %2\n").arg(id_).arg(displayName())
        << QStringLiteral("  Метка: %1\n").arg(label.isEmpty() ? QStringLiteral("(без метки)") : label)
        << QStringLiteral("  Цвет: %1\n").arg(color);
    writeTxtGeometry(out);
    out << '\n';
}

QString Annotation::colorToString(const QColor &color)
{
    return color.alpha() == 255 ? color.name(QColor::HexRgb) : color.name(QColor::HexArgb);
}

QString Annotation::formatNumber(qreal value)
{
    return QString::number(value, 'g', 10);
}

QString Annotation::formatPoint(const QPointF &point)
{
    return QStringLiteral("(%1, %2)").arg(formatNumber(point.x()), formatNumber(point.y()));
}

QPen Annotation::outlinePen() const
{
    QPen pen(color_, 2);
    pen.setCosmetic(true);
    pen.setJoinStyle(Qt::RoundJoin);
    return pen;
}

QPen Annotation::strokePen(int width, qreal scale) const
{
    // Тонкую линию рисуем постоянной экранной толщины, иначе при уменьшении она пропадает.
    QPen pen = outlinePen();
    if (width * scale > 2) {
        pen.setCosmetic(false);
        pen.setWidthF(width);
    }
    return pen;
}
