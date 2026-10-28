#pragma once

#include <QGraphicsView>
#include <QHash>

class AnnotationDocument;
class AnnotationGraphicsItem;
class QGraphicsPixmapItem;
class Tool;

// Область просмотра изображения. Координаты сцены совпадают с пикселями картинки.
// Колесом меняется масштаб, средней кнопкой или пробелом с левой кнопкой картинка
// прокручивается. Остальные события мыши и клавиатуры получает текущий инструмент.
class ImageCanvas : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageCanvas(QWidget *parent = nullptr);
    ~ImageCanvas() override;

    void setImage(const QImage &image);
    bool hasImage() const;
    QSize imageSize() const;
    QRectF imageRect() const;

    void setDocument(AnnotationDocument *document);
    void setTool(Tool *tool);

    qreal viewScale() const;
    AnnotationGraphicsItem *itemFor(int id) const;
    int selectedId() const { return selectedId_; }

public slots:
    void setSelectedId(int id);
    void ensureAnnotationVisible(int id);
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    void zoomActual();

signals:
    void cursorPixelChanged(const QPoint &pixel, bool insideImage);
    void zoomChanged(qreal scale);
    void selectedIdChanged(int id);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void addItem(int id);
    void removeItem(int id);
    void updateItem(int id);
    void rebuildItems();
    void applyZoom(qreal factor);
    void viewScaleChanged();
    void updateCursor();

    QGraphicsScene *scene_;
    QGraphicsPixmapItem *pixmapItem_;
    AnnotationDocument *document_ = nullptr;
    Tool *tool_ = nullptr;
    QHash<int, AnnotationGraphicsItem *> items_;
    int selectedId_ = 0;

    bool spaceHeld_ = false;
    bool panning_ = false;
    QPoint panLast_;
};
