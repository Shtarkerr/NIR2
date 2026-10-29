#include "io/TxtExporter.h"

#include "model/AnnotationDocument.h"

#include <QFileInfo>
#include <QSaveFile>
#include <QTextStream>

QString TxtExporter::toText(const AnnotationDocument &doc)
{
    QString text;
    QTextStream out(&text);

    // В Qt 5 operator<<(const char *) считает строку Latin-1, поэтому кириллицу
    // передаём через QStringLiteral.
    out << QStringLiteral("Разметка изображения (Image Annotator)\n")
        << (doc.isBlankCanvas()
                ? QStringLiteral("Изображение: пустой холст, цвет %1\n").arg(doc.canvasColor().name())
                : QStringLiteral("Изображение: %1\n").arg(QFileInfo(doc.imagePath()).fileName()))
        << QStringLiteral("Размер: %1 x %2 пикселей\n").arg(doc.imageSize().width()).arg(doc.imageSize().height())
        << QStringLiteral("Объектов: %1\n").arg(doc.count())
        << '\n'
        << QStringLiteral("Координаты даны в пикселях изображения: x растёт вправо, y вниз,\n"
                          "точка (0, 0) в левом верхнем углу. Рамка фигуры проходит посередине\n"
                          "её границы. У контура перечислены вершины и все пиксели его средней\n"
                          "линии по порядку обхода.\n")
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
