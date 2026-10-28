#pragma once

#include <QString>

class AnnotationDocument;

// Экспорт разметки в построчный текстовый формат:
// координаты вершин, центры эллипсов, последовательные пиксели контуров и метки.
class TxtExporter
{
public:
    static QString toText(const AnnotationDocument &doc);
    static bool save(const AnnotationDocument &doc, const QString &fileName,
                     QString *error = nullptr);
};
