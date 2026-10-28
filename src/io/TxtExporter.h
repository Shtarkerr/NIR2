#pragma once

#include <QString>

class AnnotationDocument;

// Экспорт разметки в текстовый файл: вершины, центры эллипсов, пиксели контуров, метки.
class TxtExporter
{
public:
    static QString toText(const AnnotationDocument &doc);
    static bool save(const AnnotationDocument &doc, const QString &fileName,
                     QString *error = nullptr);
};
