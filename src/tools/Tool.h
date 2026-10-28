#pragma once

#include <QColor>
#include <QPointF>
#include <QString>

#include <memory>

class Annotation;
class AnnotationDocument;
class AnnotationGraphicsItem;
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
};

// Общее окружение инструментов; принадлежит главному окну.
struct ToolContext
{
    ImageCanvas *canvas = nullptr;
    AnnotationDocument *document = nullptr;
    QUndoStack *undoStack = nullptr;
    DrawingStyle style;
};

// Инструмент разметки (паттерн «Стратегия»): холст передаёт ему события мыши
// и клавиатуры в координатах изображения.
//
// Создаваемый объект сначала существует как черновик: он рисуется на холсте,
// но в документ попадает только после commitDraft() через команду отмены.
class Tool
{
public:
    explicit Tool(ToolContext &context);
    virtual ~Tool();

    Tool(const Tool &) = delete;
    Tool &operator=(const Tool &) = delete;

    virtual QString name() const = 0;
    virtual Qt::CursorShape cursorShape() const { return Qt::CrossCursor; }

    virtual void mousePress(const QPointF &pos, QMouseEvent *event);
    virtual void mouseMove(const QPointF &pos, QMouseEvent *event);
    virtual void mouseRelease(const QPointF &pos, QMouseEvent *event);
    virtual void mouseDoubleClick(const QPointF &pos, QMouseEvent *event);
    // Возвращает true, если клавиша обработана.
    virtual bool keyPress(QKeyEvent *event);
    // Прерывает незавершённое действие (например, при смене инструмента).
    virtual void cancel();

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

    ToolContext &context_;

private:
    std::unique_ptr<Annotation> draft_;
    std::unique_ptr<AnnotationGraphicsItem> draftItem_;
};
