#pragma once

#include <QDir>
#include <QJsonObject>
#include <QString>

class AnnotationDocument;

// Сохранение и загрузка разметки в формате JSON (основной формат проекта).
// Путь к изображению хранится относительно каталога файла разметки.
class JsonSerializer
{
public:
    static constexpr int FormatVersion = 1;

    static QJsonObject toJson(const AnnotationDocument &doc, const QDir &baseDir);
    // При ошибке документ не изменяется.
    static bool fromJson(AnnotationDocument &doc, const QJsonObject &root,
                         const QDir &baseDir, QString *error = nullptr);

    static bool save(const AnnotationDocument &doc, const QString &fileName,
                     QString *error = nullptr);
    static bool load(AnnotationDocument &doc, const QString &fileName,
                     QString *error = nullptr);
};
