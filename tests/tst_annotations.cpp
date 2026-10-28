#include "model/AnnotationDocument.h"
#include "model/ContourAnnotation.h"
#include "model/EllipseAnnotation.h"
#include "model/PointAnnotation.h"
#include "model/RectAnnotation.h"

#include <QSignalSpy>
#include <QtTest>

class TestAnnotations : public QObject
{
    Q_OBJECT

private slots:
    void pointHitTest()
    {
        PointAnnotation point(QPoint(10, 10));
        QVERIFY(point.hitTest(QPointF(10.5, 10.5), 0));
        QVERIFY(point.hitTest(QPointF(13, 10.5), 3));
        QVERIFY(!point.hitTest(QPointF(20, 20), 3));
    }

    void rectOutlineVsFilled()
    {
        RectAnnotation rect(QRectF(0, 0, 100, 50));
        const QPointF center(50, 25);
        const QPointF border(0.5, 25);
        QVERIFY(rect.hitTest(border, 2));
        QVERIFY(!rect.hitTest(center, 2));
        rect.setFilled(true);
        QVERIFY(rect.hitTest(center, 2));
        QVERIFY(!rect.hitTest(QPointF(200, 200), 2));
    }

    void rectIsNormalized()
    {
        RectAnnotation rect(QRectF(QPointF(30, 40), QPointF(10, 20)));
        QCOMPARE(rect.rect(), QRectF(10, 20, 20, 20));
    }

    void ellipseFromRect()
    {
        EllipseAnnotation ellipse;
        ellipse.setFromRect(QRectF(10, 20, 40, 20));
        QCOMPARE(ellipse.center(), QPointF(30, 30));
        QCOMPARE(ellipse.radiusX(), 20.0);
        QCOMPARE(ellipse.radiusY(), 10.0);
        QCOMPARE(ellipse.boundingRect(), QRectF(10, 20, 40, 20));
    }

    void ellipseHitTest()
    {
        EllipseAnnotation ellipse(QPointF(0, 0), 20, 10);
        QVERIFY(ellipse.hitTest(QPointF(20, 0), 1));
        QVERIFY(!ellipse.hitTest(QPointF(0, 0), 1));
        QVERIFY(!ellipse.hitTest(QPointF(20, 10), 1));
        ellipse.setFilled(true);
        QVERIFY(ellipse.hitTest(QPointF(0, 0), 1));
    }

    void contourEditing()
    {
        ContourAnnotation contour;
        QVERIFY(contour.addVertex(QPoint(1, 1)));
        QVERIFY(!contour.addVertex(QPoint(1, 1)));
        QVERIFY(contour.addVertex(QPoint(5, 1)));
        QCOMPARE(contour.vertexCount(), 2);
        contour.removeLastVertex();
        QCOMPARE(contour.vertexCount(), 1);
    }

    void contourHitTestAndBounds()
    {
        ContourAnnotation contour({{0, 0}, {10, 0}, {10, 10}}, true);
        QVERIFY(contour.hitTest(QPointF(5.5, 0.5), 1));
        // Замыкающая сторона от (10,10) к (0,0).
        QVERIFY(contour.hitTest(QPointF(5.5, 5.5), 1));
        QVERIFY(!contour.hitTest(QPointF(8, 3), 1));
        QCOMPARE(contour.boundingRect(), QRectF(0, 0, 11, 11));
        contour.setClosed(false);
        QVERIFY(!contour.hitTest(QPointF(5.5, 5.5), 1));
    }

    void thickContour()
    {
        ContourAnnotation contour({{0, 0}, {10, 0}}, false);
        QVERIFY(!contour.hitTest(QPointF(5.5, 4.5), 1));
        contour.setWidth(9);
        // Половина толщины 4.5 плюс допуск 1.
        QVERIFY(contour.hitTest(QPointF(5.5, 4.5), 1));
        QCOMPARE(contour.boundingRect(), QRectF(-4, -4, 19, 9));
        contour.setWidth(0);
        QCOMPARE(contour.width(), 1);
    }

    void translateAndClone()
    {
        RectAnnotation rect(QRectF(0, 0, 10, 10));
        rect.setLabel("cat");
        rect.setColor(Qt::green);
        rect.setFilled(true);

        std::unique_ptr<Annotation> copy = rect.clone();
        rect.translate(QPointF(5, 5));

        auto *copied = dynamic_cast<RectAnnotation *>(copy.get());
        QVERIFY(copied);
        QCOMPARE(copied->rect(), QRectF(0, 0, 10, 10));
        QCOMPARE(copied->label(), QString("cat"));
        QCOMPARE(copied->color(), QColor(Qt::green));
        QVERIFY(copied->isFilled());
        QCOMPARE(rect.rect(), QRectF(5, 5, 10, 10));
    }

    void documentAssignsUniqueIds()
    {
        AnnotationDocument doc;
        QSignalSpy added(&doc, &AnnotationDocument::annotationAdded);

        const int a = doc.add(std::make_unique<PointAnnotation>(QPoint(1, 1)));
        const int b = doc.add(std::make_unique<PointAnnotation>(QPoint(2, 2)));
        QVERIFY(a > 0 && b > 0 && a != b);
        QCOMPARE(added.count(), 2);

        // Занятый id заменяется новым.
        auto duplicate = std::make_unique<PointAnnotation>(QPoint(3, 3));
        duplicate->setId(a);
        const int c = doc.add(std::move(duplicate));
        QVERIFY(c != a && c != b);
        QCOMPARE(doc.count(), 3);
    }

    void documentTakeAndReinsert()
    {
        AnnotationDocument doc;
        doc.add(std::make_unique<PointAnnotation>(QPoint(1, 1)));
        const int middle = doc.add(std::make_unique<PointAnnotation>(QPoint(2, 2)));
        doc.add(std::make_unique<PointAnnotation>(QPoint(3, 3)));

        QSignalSpy removed(&doc, &AnnotationDocument::annotationRemoved);
        int index = -1;
        std::unique_ptr<Annotation> taken = doc.take(middle, &index);
        QVERIFY(taken);
        QCOMPARE(index, 1);
        QCOMPARE(removed.count(), 1);
        QVERIFY(!doc.find(middle));

        doc.add(std::move(taken), index);
        QCOMPARE(doc.indexOf(middle), 1);
    }

    void documentLabels()
    {
        AnnotationDocument doc;
        auto point = std::make_unique<PointAnnotation>(QPoint(1, 1));
        point->setLabel("eye");
        const int id = doc.add(std::move(point));
        QCOMPARE(doc.labels(), QStringList{"eye"});

        doc.setLabel(id, "nose");
        QCOMPARE(doc.find(id)->label(), QString("nose"));
        QCOMPARE(doc.labels(), (QStringList{"eye", "nose"}));

        doc.reset("image.png", QSize(10, 10));
        QCOMPARE(doc.count(), 0);
        QVERIFY(doc.labels().isEmpty());
    }

    void documentTranslate()
    {
        AnnotationDocument doc;
        const int id = doc.add(std::make_unique<PointAnnotation>(QPoint(1, 1)));
        QSignalSpy changed(&doc, &AnnotationDocument::annotationChanged);
        doc.translate(id, QPointF(2, 3));
        QCOMPARE(changed.count(), 1);
        QCOMPARE(static_cast<PointAnnotation *>(doc.find(id))->pos(), QPoint(3, 4));
    }
};

QTEST_GUILESS_MAIN(TestAnnotations)
#include "tst_annotations.moc"
