#include "io/TxtExporter.h"

#include "model/AnnotationDocument.h"

#include <QFileInfo>
#include <QSaveFile>
#include <QTextStream>

QString TxtExporter::toText(const AnnotationDocument &doc)
{
    QString text;
    QTextStream out(&text);

    // В Qt 5 operator<<(const char *) читает строку как Latin-1, поэтому
    // текст с кириллицей передаётся через QStringLiteral.
    out << QStringLiteral("# Image Annotator: экспорт разметки\n")
        << "# image: " << QFileInfo(doc.imagePath()).fileName() << '\n'
        << "# size: " << doc.imageSize().width() << 'x' << doc.imageSize().height() << '\n'
        << "# objects: " << doc.count() << '\n'
        << "#\n"
        << QStringLiteral("# Координаты в пикселях изображения, начало — левый верхний угол.\n")
        << QStringLiteral("# POINT   x y — отмеченный пиксель\n")
        << QStringLiteral("# RECT    filled x1 y1 .. x4 y4 — вершины по часовой стрелке от левой верхней\n")
        << QStringLiteral("# ELLIPSE filled cx cy rx ry — центр и полуоси\n")
        << QStringLiteral("# CONTOUR closed vertices pixels; V — вершины, P — последовательные пиксели контура\n")
        << '\n';

    for (const auto &annotation : doc.annotations())
        annotation->writeTxt(out);

    out.flush();
    return text;
}

bool TxtExporter::save(const AnnotationDocument &doc, const QString &fileName, QString *error)
{
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error)
            *error = file.errorString();
        return false;
    }
    file.write(toText(doc).toUtf8());
    if (!file.commit()) {
        if (error)
            *error = file.errorString();
        return false;
    }
    return true;
}
