#pragma once

#include <QColor>
#include <QPointF>
#include <QString>

#include <functional>
#include <memory>

class Annotation;
class AnnotationDocument;
class AnnotationGraphicsItem;
class BrushCursorItem;
class ImageCanvas;
class QKeyEvent;
class QMouseEvent;
class QUndoStack;

// Текущие параметры рисования, задаваемые пользователем.
struct DrawingStyle
{
    QColor color = Qt::red;
    bool filled = false;
    QString label;
    int brushSize = 5;
};

// То, с чем работают инструменты. Хранится в главном окне.
struct ToolContext
{
    ImageCanvas *canvas = nullptr;
    AnnotationDocument *document = nullptr;
    QUndoStack *undoStack = nullptr;
    DrawingStyle style;
    // Меняет размер кисти у всех инструментов и в поле на панели.
    std::function<void(int)> setBrushSize;
};

// Инструмент разметки. Холст передаёт ему события мыши и клавиатуры
// в координатах изображения.
// Новый объект сначала живёт как черновик: он виден на холсте, но в документ
// попадает только в commitDraft(), через команду, которую можно отменить.
class Tool
{
public:
    explicit Tool(ToolContext &context);
    virtual ~Tool();

    Tool(const Tool &) = delete;
    Tool &operator=(const Tool &) = delete;

    virtual QString name() const = 0;
    virtual Qt::CursorShape cursorShape() const { return usesBrush() ? Qt::BlankCursor : Qt::CrossCursor; }
    // Инструмент рисует кистью, вместо курсора показывается круг кисти.
    virtual bool usesBrush() const { return false; }
    // Можно ли сейчас менять размер кисти правой кнопкой.
    virtual bool canResizeBrush() const { return usesBrush(); }

    virtual void mousePress(const QPointF &pos, QMouseEvent *event);
    virtual void mouseMove(const QPointF &pos, QMouseEvent *event);
    virtual void mouseRelease(const QPointF &pos, QMouseEvent *event);
    virtual void mouseDoubleClick(const QPointF &pos, QMouseEvent *event);
    // Возвращает true, если клавиша обработана.
    virtual bool keyPress(QKeyEvent *event);
    // Прерывает незавершённое действие (например, при смене инструмента).
    virtual void cancel();

    // Пользователь переключился на другой инструмент.
    virtual void deactivate();
    // Курсор стоит над холстом в точке pos, кнопки не нажаты. Нужно, чтобы кисть
    // появилась сразу после смены инструмента, не дожидаясь движения мыши.
    void hover(const QPointF &pos);
    // Курсор ушёл с холста.
    virtual void leave();
    // Поменялись параметры рисования (цвет, заливка, размер кисти).
    virtual void styleChanged();

    // Размер кисти меняется движением мыши вправо и влево с зажатой правой кнопкой,
    // как в Photoshop. Круг кисти стоит там, где нажали кнопку, а его край идёт за мышью.
    // Когда кнопку отпускают, кисть возвращается под мышь.
    void beginBrushResize(const QPointF &pos);
    void resizeBrush(const QPointF &pos);
    void endBrushResize(const QPointF &pos);

protected:
    // Точка, ограниченная рамками изображения.
    QPointF clampToImage(const QPointF &pos) const;
    // Пиксель изображения, содержащий точку.
    QPoint pixelAt(const QPointF &pos) const;

    void startDraft(std::unique_ptr<Annotation> draft);
    Annotation *draft() const { return draft_.get(); }
    void draftChanged();
    void commitDraft();
    void discardDraft();

    // Курсор-кисть у инструментов, которые рисуют кистью. Двигается за мышью,
    // за пределами картинки прячется, но пока зажата левая кнопка, остаётся видна,
    // чтобы не пропадать посреди штриха.
    void moveBrush(const QPointF &pos, Qt::MouseButtons buttons);
    void hideBrushCursor();
    // Куда ставить центр кисти, когда мышь в точке pos.
    virtual QPointF brushCenter(const QPointF &pos) const;
    // Размер кисти, которым рисует инструмент.
    virtual int brushSize() const { return context_.style.brushSize; }

    ToolContext &context_;

private:
    void showBrushCursor(const QPointF &pos);
    void showBrushSize();

    std::unique_ptr<BrushCursorItem> brush_;
    QPointF resizeAnchor_;
    int resizeStartSize_ = 1;
    std::unique_ptr<Annotation> draft_;
    std::unique_ptr<AnnotationGraphicsItem> draftItem_;
};
