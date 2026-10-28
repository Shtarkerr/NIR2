#pragma once

#include <QColor>
#include <QDialog>

class QSpinBox;
class QToolButton;

// Диалог создания пустого холста: размер и цвет фона.
class NewCanvasDialog : public QDialog
{
    Q_OBJECT

public:
    NewCanvasDialog(const QSize &size, const QColor &color, QWidget *parent = nullptr);

    QSize canvasSize() const;
    QColor canvasColor() const { return color_; }

private:
    void chooseColor();
    void updateColorButton();

    QSpinBox *widthSpin_;
    QSpinBox *heightSpin_;
    QToolButton *colorButton_;
    QColor color_;
};
