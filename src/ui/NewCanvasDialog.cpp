#include "ui/NewCanvasDialog.h"

#include "ui/Icons.h"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QToolButton>

namespace {

constexpr int MaxSide = 8000;

QSpinBox *makeSideSpin(int value, QWidget *parent)
{
    auto *spin = new QSpinBox(parent);
    spin->setRange(1, MaxSide);
    spin->setSuffix(QStringLiteral(" px"));
    spin->setValue(value);
    return spin;
}

} // namespace

NewCanvasDialog::NewCanvasDialog(const QSize &size, const QColor &color, QWidget *parent)
    : QDialog(parent),
      widthSpin_(makeSideSpin(size.width(), this)),
      heightSpin_(makeSideSpin(size.height(), this)),
      colorButton_(new QToolButton(this)),
      color_(color)
{
    setWindowTitle(tr("Новый холст"));

    colorButton_->setIconSize(QSize(20, 20));
    colorButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(colorButton_, &QToolButton::clicked, this, &NewCanvasDialog::chooseColor);
    updateColorButton();

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QFormLayout(this);
    layout->addRow(tr("Ширина:"), widthSpin_);
    layout->addRow(tr("Высота:"), heightSpin_);
    layout->addRow(tr("Цвет:"), colorButton_);
    layout->addRow(buttons);
}

QSize NewCanvasDialog::canvasSize() const
{
    return QSize(widthSpin_->value(), heightSpin_->value());
}

void NewCanvasDialog::chooseColor()
{
    const QColor color = QColorDialog::getColor(color_, this, tr("Цвет холста"));
    if (color.isValid()) {
        color_ = color;
        updateColorButton();
    }
}

void NewCanvasDialog::updateColorButton()
{
    colorButton_->setIcon(Icons::colorSwatch(color_, 20));
    colorButton_->setText(color_.name());
}
