#pragma once

#include "model/Annotation.h"

// Создание объектов разметки по имени типа, нужно при загрузке файлов.
class AnnotationFactory
{
public:
    static std::unique_ptr<Annotation> create(const QString &typeName);
    // Возвращает nullptr, если тип неизвестен или данные битые.
    static std::unique_ptr<Annotation> fromJson(const QJsonObject &obj);
};
