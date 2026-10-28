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
    doc.add(std::move(point));

    auto rect = std::make_unique<RectAnnotation>(QRectF(10, 20, 200, 150));
    rect->setLabel("cat \"Tom\"");
    rect->setColor(QColor("#ff0000"));
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

        QVERIFY(TxtExporter::toText(doc).contains("# canvas: #336699\n"));
    }

    void contourWithoutWidthDefaultsToOne()
    {
        const QJsonObject json{{"type", "contour"}, {"color", "#ff0000"},
                               {"vertices", QJsonArray{QJsonArray{1, 2}, QJsonArray{5, 6}}}};
        std::unique_ptr<Annotation> contour = AnnotationFactory::fromJson(json);
        QVERIFY(contour);
        QCOMPARE(static_cast<ContourAnnotation *>(contour.get())->width(), 1);
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

        QVERIFY(text.contains(QStringLiteral("# Image Annotator: экспорт разметки\n")));
        QVERIFY(text.contains("# image: photo.png"));
        QVERIFY(text.contains("# size: 640x480"));
        QVERIFY(text.contains("POINT id=1 label=\"eye\" color=#00ff00 x=120 y=45"));
        QVERIFY(text.contains("RECT id=2 label=\"cat \\\"Tom\\\"\" color=#ff0000 filled=0 "
                              "x1=10 y1=20 x2=210 y2=20 x3=210 y3=170 x4=10 y4=170"));
        QVERIFY(text.contains("ELLIPSE id=3 label=\"ball\" color=#800000ff filled=1 cx=400 cy=300 rx=50 ry=30.5"));

        // Количество пикселей в заголовке контура совпадает со строкой P.
        const auto *contour = static_cast<const ContourAnnotation *>(doc.annotations().back().get());
        const int pixelCount = contour->rasterize().size();
        QVERIFY(text.contains(QStringLiteral("CONTOUR id=4 label=\"tree\" color=#ffaa00 closed=1 width=7 vertices=4 pixels=%1")
                                  .arg(pixelCount)));
        QVERIFY(text.contains("\n  V 100 100; 150 90; 160 140; 110 150\n"));

        const QRegularExpressionMatch match = QRegularExpression("\n  P ([^\n]*)\n").match(text);
        QVERIFY(match.hasMatch());
        QCOMPARE(match.captured(1).split("; ").size(), pixelCount);
    }
};

QTEST_GUILESS_MAIN(TestSerialization)
#include "tst_serialization.moc"
