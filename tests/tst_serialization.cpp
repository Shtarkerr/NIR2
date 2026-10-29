#include "io/JsonSerializer.h"
#include "io/TxtExporter.h"
#include "model/AnnotationDocument.h"
#include "model/AnnotationFactory.h"
#include "model/ContourAnnotation.h"
#include "model/EllipseAnnotation.h"
#include "model/PointAnnotation.h"
#include "model/RectAnnotation.h"

#include <QJsonArray>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QtTest>

namespace {

void fillDocument(AnnotationDocument &doc, const QString &imagePath)
{
    doc.reset(imagePath, QSize(640, 480));

    auto point = std::make_unique<PointAnnotation>(QPoint(120, 45));
    point->setLabel("eye");
    point->setColor(QColor("#00ff00"));
    point->setSize(5);
    doc.add(std::move(point));

    auto rect = std::make_unique<RectAnnotation>(QRectF(10, 20, 200, 150));
    rect->setLabel("cat \"Tom\"");
    rect->setColor(QColor("#ff0000"));
    rect->setWidth(3);
    doc.add(std::move(rect));

    auto ellipse = std::make_unique<EllipseAnnotation>(QPointF(400, 300), 50, 30.5);
    ellipse->setLabel("ball");
    ellipse->setFilled(true);
    ellipse->setColor(QColor(0, 0, 255, 128));
    doc.add(std::move(ellipse));

    auto contour = std::make_unique<ContourAnnotation>(QVector<QPoint>{{100, 100}, {150, 90}, {160, 140}, {110, 150}}, true);
    contour->setLabel("tree");
    contour->setColor(QColor("#ffaa00"));
    contour->setWidth(7);
    doc.add(std::move(contour));
}

} // namespace

class TestSerialization : public QObject
{
    Q_OBJECT

private slots:
    void annotationJsonRoundTrip()
    {
        AnnotationDocument doc;
        fillDocument(doc, QString());
        for (const auto &original : doc.annotations()) {
            const QJsonObject json = original->toJson();
            std::unique_ptr<Annotation> restored = AnnotationFactory::fromJson(json);
            QVERIFY2(restored, qPrintable(original->typeName()));
            QCOMPARE(restored->toJson(), json);
            QCOMPARE(restored->color(), original->color());
        }
    }

    void factoryRejectsBadInput()
    {
        QVERIFY(!AnnotationFactory::fromJson(QJsonObject{{"type", "hexagon"}}));
        QVERIFY(!AnnotationFactory::fromJson(QJsonObject{{"type", "rect"}, {"color", "#ff0000"}}));
        QVERIFY(!AnnotationFactory::fromJson(
            QJsonObject{{"type", "contour"}, {"color", "#ff0000"}, {"vertices", QJsonArray{}}}));
    }

    void documentFileRoundTrip()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString imagePath = dir.filePath("images/photo.png");
        const QString jsonPath = dir.filePath("photo.json");

        AnnotationDocument doc;
        fillDocument(doc, imagePath);
        doc.addLabel("unused");
        QString error;
        QVERIFY2(JsonSerializer::save(doc, jsonPath, &error), qPrintable(error));

        // Путь к изображению хранится относительно файла разметки.
        QFile file(jsonPath);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY(file.readAll().contains("\"image\": \"images/photo.png\""));

        AnnotationDocument loaded;
        QVERIFY2(JsonSerializer::load(loaded, jsonPath, &error), qPrintable(error));
        QCOMPARE(loaded.imagePath(), imagePath);
        QCOMPARE(loaded.imageSize(), QSize(640, 480));
        QCOMPARE(loaded.labels(), doc.labels());
        QCOMPARE(loaded.count(), doc.count());
        for (int i = 0; i < doc.count(); ++i) {
            QCOMPARE(loaded.annotations()[static_cast<size_t>(i)]->toJson(),
                     doc.annotations()[static_cast<size_t>(i)]->toJson());
        }
    }

    void blankCanvasRoundTrip()
    {
        QTemporaryDir dir;
        const QString jsonPath = dir.filePath("canvas.json");

        AnnotationDocument doc;
        doc.reset(QString(), QSize(300, 200), QColor("#336699"));
        doc.add(std::make_unique<PointAnnotation>(QPoint(10, 20)));
        QVERIFY(doc.isBlankCanvas());

        QString error;
        QVERIFY2(JsonSerializer::save(doc, jsonPath, &error), qPrintable(error));
        AnnotationDocument loaded;
        QVERIFY2(JsonSerializer::load(loaded, jsonPath, &error), qPrintable(error));
        QVERIFY(loaded.isBlankCanvas());
        QCOMPARE(loaded.canvasColor(), QColor("#336699"));
        QCOMPARE(loaded.imageSize(), QSize(300, 200));
        QCOMPARE(loaded.count(), 1);

        QVERIFY(TxtExporter::toText(doc).contains(QStringLiteral("Изображение: пустой холст, цвет #336699\n")));
    }

    void contourWithoutWidthDefaultsToOne()
    {
        const QJsonObject json{{"type", "contour"}, {"color", "#ff0000"},
                               {"vertices", QJsonArray{QJsonArray{1, 2}, QJsonArray{5, 6}}}};
        std::unique_ptr<Annotation> contour = AnnotationFactory::fromJson(json);
        QVERIFY(contour);
        QCOMPARE(static_cast<ContourAnnotation *>(contour.get())->width(), 1);
    }

    void shapeWithoutWidthDefaultsToOne()
    {
        const QJsonObject json{{"type", "rect"}, {"color", "#ff0000"},
                               {"x", 1}, {"y", 2}, {"w", 3}, {"h", 4}};
        std::unique_ptr<Annotation> rect = AnnotationFactory::fromJson(json);
        QVERIFY(rect);
        QCOMPARE(static_cast<RectAnnotation *>(rect.get())->width(), 1);

        std::unique_ptr<Annotation> point = AnnotationFactory::fromJson(
            QJsonObject{{"type", "point"}, {"color", "#ff0000"}, {"x", 1}, {"y", 2}});
        QVERIFY(point);
        QCOMPARE(static_cast<PointAnnotation *>(point.get())->size(), 1);
    }

    void loadErrorKeepsDocument()
    {
        QTemporaryDir dir;
        const QString jsonPath = dir.filePath("broken.json");
        QFile file(jsonPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{\"format\":\"image-annotator\",\"version\":1,\"annotations\":[{\"type\":\"rect\"}]}");
        file.close();

        AnnotationDocument doc;
        fillDocument(doc, QString());
        QString error;
        QVERIFY(!JsonSerializer::load(doc, jsonPath, &error));
        QVERIFY(!error.isEmpty());
        QCOMPARE(doc.count(), 4);
    }

    void txtExport()
    {
        AnnotationDocument doc;
        fillDocument(doc, "/tmp/photo.png");
        const QString text = TxtExporter::toText(doc);

        QVERIFY(text.startsWith(QStringLiteral("Разметка изображения (Image Annotator)\n"
                                               "Изображение: photo.png\n"
                                               "Размер: 640 x 480 пикселей\n"
                                               "Объектов: 4\n")));
        QVERIFY(text.contains(QStringLiteral("Объект №1. Точка\n"
                                             "  Метка: eye\n"
                                             "  Цвет: #00ff00\n"
                                             "  Пиксель: (120, 45)\n"
                                             "  Размер: 5 px\n")));
        QVERIFY(text.contains(QStringLiteral("Объект №2. Прямоугольник\n"
                                             "  Метка: cat \"Tom\"\n"
                                             "  Цвет: #ff0000\n"
                                             "  Вид: рамка толщиной 3 px\n"
                                             "  Левый верхний угол (10, 20), ширина 200, высота 150\n"
                                             "  Вершины по часовой стрелке от левой верхней:\n"
                                             "    (10, 20)  (210, 20)  (210, 170)  (10, 170)\n")));
        QVERIFY(text.contains(QStringLiteral("Объект №3. Эллипс\n"
                                             "  Метка: ball\n"
                                             "  Цвет: #0000ff, непрозрачность 50%\n"
                                             "  Вид: сплошная заливка\n"
                                             "  Центр: (400, 300)\n"
                                             "  Полуоси: по горизонтали 50, по вертикали 30.5\n")));

        const auto *contour = static_cast<const ContourAnnotation *>(doc.annotations().back().get());
        const QVector<QPoint> pixels = contour->rasterize();
        QVERIFY(text.contains(QStringLiteral("Объект №4. Контур\n"
                                             "  Метка: tree\n"
                                             "  Цвет: #ffaa00\n"
                                             "  Вид: замкнутый контур, толщина кисти 7 px\n"
                                             "  Вершин: 4\n"
                                             "    (100, 100)  (150, 90)  (160, 140)  (110, 150)\n"
                                             "  Пикселей средней линии по порядку: %1\n").arg(pixels.size())));

        // Все пиксели контура перечислены по порядку, по 8 в строке.
        const QString list = text.mid(text.indexOf(QStringLiteral("по порядку:")));
        const QRegularExpression pointRe(QStringLiteral("\\((-?\\d+), (-?\\d+)\\)"));
        QVector<QPoint> listed;
        for (auto it = pointRe.globalMatch(list); it.hasNext();) {
            const QRegularExpressionMatch m = it.next();
            listed.append(QPoint(m.captured(1).toInt(), m.captured(2).toInt()));
        }
        QCOMPARE(listed, pixels);
        QStringList firstLine;
        for (int i = 0; i < 8; ++i)
            firstLine.append(QStringLiteral("(%1, %2)").arg(pixels[i].x()).arg(pixels[i].y()));
        QVERIFY(text.contains(QStringLiteral("по порядку: %1\n    %2\n")
                                  .arg(pixels.size()).arg(firstLine.join(QStringLiteral("  ")))));
    }

    void txtLabelOnOneLine()
    {
        AnnotationDocument doc;
        auto point = std::make_unique<PointAnnotation>(QPoint(1, 2));
        point->setLabel("first\nsecond");
        doc.add(std::move(point));
        auto empty = std::make_unique<PointAnnotation>(QPoint(3, 4));
        doc.add(std::move(empty));
        const QString text = TxtExporter::toText(doc);
        QVERIFY(text.contains(QStringLiteral("  Метка: first second\n")));
        QVERIFY(text.contains(QStringLiteral("  Метка: (без метки)\n")));
    }
};

QTEST_GUILESS_MAIN(TestSerialization)
#include "tst_serialization.moc"
