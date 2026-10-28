#pragma once

#include "model/Annotation.h"

#include <QStringList>

// Фабрика объектов разметки по имени типа (используется при загрузке файлов).
class AnnotationFactory
{
public:
    static QStringList typeNames();
    static std::unique_ptr<Annotation> create(const QString &typeName);
    // Создаёт объект по JSON-описанию; nullptr, если тип неизвестен или данные некорректны.
    static std::unique_ptr<Annotation> fromJson(const QJsonObject &obj);
};
