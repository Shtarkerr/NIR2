#include "io/JsonSerializer.h"

#include "model/AnnotationDocument.h"
#include "model/AnnotationFactory.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

namespace {

void setError(QString *error, const QString &message)
{
    if (error)
        *error = message;
}

} // namespace

QJsonObject JsonSerializer::toJson(const AnnotationDocument &doc, const QDir &baseDir)
{
    QJsonObject root;
    root["format"] = QStringLiteral("image-annotator");
    root["version"] = FormatVersion;
    root["image"] = doc.imagePath().isEmpty() ? QString() : baseDir.relativeFilePath(doc.imagePath());
    root["width"] = doc.imageSize().width();
    root["height"] = doc.imageSize().height();
    root["labels"] = QJsonArray::fromStringList(doc.labels());

    QJsonArray annotations;
    for (const auto &annotation : doc.annotations())
        annotations.append(annotation->toJson());
    root["annotations"] = annotations;
    return root;
}

bool JsonSerializer::fromJson(AnnotationDocument &doc, const QJsonObject &root,
                              const QDir &baseDir, QString *error)
{
    if (root.value("format").toString() != QLatin1String("image-annotator")) {
        setError(error, QObject::tr("Файл не является разметкой Image Annotator"));
        return false;
    }
    if (root.value("version").toInt() > FormatVersion) {
        setError(error, QObject::tr("Файл создан более новой версией программы"));
        return false;
    }
    if (!root.value("annotations").isArray()) {
        setError(error, QObject::tr("Отсутствует список объектов разметки"));
        return false;
    }

    // Сначала разбираем всё, и только потом меняем документ.
    AnnotationDocument::AnnotationList annotations;
    const QJsonArray items = root.value("annotations").toArray();
    for (int i = 0; i < items.size(); ++i) {
        std::unique_ptr<Annotation> annotation = AnnotationFactory::fromJson(items[i].toObject());
        if (!annotation) {
            setError(error, QObject::tr("Некорректный объект разметки №%1").arg(i + 1));
            return false;
        }
        annotations.push_back(std::move(annotation));
    }

    const QString image = root.value("image").toString();
    const QString imagePath = image.isEmpty() ? QString()
                                              : QDir::cleanPath(baseDir.absoluteFilePath(image));
    const QSize size(root.value("width").toInt(), root.value("height").toInt());

    QStringList labels;
    for (const QJsonValue &label : root.value("labels").toArray())
        labels.append(label.toString());

    doc.reset(imagePath, size);
    doc.setLabels(labels);
    for (auto &annotation : annotations)
        doc.add(std::move(annotation));
    return true;
}

bool JsonSerializer::save(const AnnotationDocument &doc, const QString &fileName, QString *error)
{
    const QDir baseDir = QFileInfo(fileName).absoluteDir();
    const QByteArray data = QJsonDocument(toJson(doc, baseDir)).toJson(QJsonDocument::Indented);

    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        setError(error, file.errorString());
        return false;
    }
    file.write(data);
    if (!file.commit()) {
        setError(error, file.errorString());
        return false;
    }
    return true;
}

bool JsonSerializer::load(AnnotationDocument &doc, const QString &fileName, QString *error)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(error, file.errorString());
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument json = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        setError(error, QObject::tr("Ошибка JSON (позиция %1): %2")
                            .arg(parseError.offset).arg(parseError.errorString()));
        return false;
    }
    if (!json.isObject()) {
        setError(error, QObject::tr("Ожидался JSON-объект"));
        return false;
    }
    return fromJson(doc, json.object(), QFileInfo(fileName).absoluteDir(), error);
}
