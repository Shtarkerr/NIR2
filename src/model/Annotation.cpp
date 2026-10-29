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
    QString escaped = label_;
    escaped.replace('\\', QStringLiteral("\\\\")).replace('"', QStringLiteral("\\\""))
           .replace('\n', QStringLiteral("\\n")).replace('\r', QStringLiteral("\\r"));

    out << typeName().toUpper()
        << " id=" << id_
        << " label=\"" << escaped << '"'
        << " color=" << colorToString(color_);
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
