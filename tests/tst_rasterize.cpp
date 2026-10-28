#include "model/ContourAnnotation.h"
#include "model/Raster.h"

#include <QtTest>

#include <cstdlib>

namespace {

// Соседние пиксели отличаются не более чем на 1 по каждой оси и не совпадают.
bool isConnected(const QVector<QPoint> &pixels)
{
    for (int i = 1; i < pixels.size(); ++i) {
        const QPoint d = pixels[i] - pixels[i - 1];
        if (std::abs(d.x()) > 1 || std::abs(d.y()) > 1 || d.isNull())
            return false;
    }
    return true;
}

} // namespace

class TestRasterize : public QObject
{
    Q_OBJECT

private slots:
    void singlePixel()
    {
        QCOMPARE(Raster::line(QPoint(3, 4), QPoint(3, 4)), QVector<QPoint>{QPoint(3, 4)});
    }

    void lines_data()
    {
        QTest::addColumn<QPoint>("a");
        QTest::addColumn<QPoint>("b");
        QTest::newRow("горизонталь") << QPoint(0, 0) << QPoint(10, 0);
        QTest::newRow("вертикаль вверх") << QPoint(5, 9) << QPoint(5, -3);
        QTest::newRow("диагональ") << QPoint(0, 0) << QPoint(7, 7);
        QTest::newRow("пологая") << QPoint(-4, 2) << QPoint(17, 9);
        QTest::newRow("крутая назад") << QPoint(10, 0) << QPoint(6, 25);
    }

    void lines()
    {
        QFETCH(QPoint, a);
        QFETCH(QPoint, b);
        const QVector<QPoint> pixels = Raster::line(a, b);

        QCOMPARE(pixels.first(), a);
        QCOMPARE(pixels.last(), b);
        QVERIFY(isConnected(pixels));
        // Брезенхэм даёт ровно max(|dx|, |dy|) + 1 пикселей.
        const int expected = std::max(std::abs(b.x() - a.x()), std::abs(b.y() - a.y())) + 1;
        QCOMPARE(pixels.size(), expected);
    }

    void openPolylineHasNoDuplicateJoints()
    {
        const QVector<QPoint> vertices{{0, 0}, {5, 0}, {5, 5}};
        const QVector<QPoint> pixels = Raster::polyline(vertices, false);
        QCOMPARE(pixels.size(), 11);
        QCOMPARE(pixels.first(), QPoint(0, 0));
        QCOMPARE(pixels.last(), QPoint(5, 5));
        QVERIFY(isConnected(pixels));
    }

    void closedPolylineReturnsToStart()
    {
        const QVector<QPoint> vertices{{0, 0}, {10, 0}, {10, 6}, {0, 6}};
        const QVector<QPoint> pixels = Raster::polyline(vertices, true);
        // Периметр прямоугольника 10×6 по пикселям: 2 * (10 + 6).
        QCOMPARE(pixels.size(), 32);
        QVERIFY(isConnected(pixels));
        const QPoint d = pixels.last() - pixels.first();
        QVERIFY(std::abs(d.x()) <= 1 && std::abs(d.y()) <= 1 && !d.isNull());
    }

    void closedWithTwoVerticesIsNotDoubled()
    {
        const QVector<QPoint> vertices{{0, 0}, {4, 0}};
        QCOMPARE(Raster::polyline(vertices, true).size(), 5);
    }

    void contourUsesRaster()
    {
        ContourAnnotation contour({{2, 2}, {8, 3}, {4, 9}}, true);
        const QVector<QPoint> pixels = contour.rasterize();
        QCOMPARE(pixels, Raster::polyline(contour.vertices(), true));
        QVERIFY(isConnected(pixels));
    }

    void emptyContour()
    {
        QVERIFY(Raster::polyline({}, true).isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestRasterize)
#include "tst_rasterize.moc"
