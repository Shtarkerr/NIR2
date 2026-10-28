#pragma once

#include "tools/Tool.h"

#include <QMainWindow>

#include <memory>
#include <vector>

class AnnotationDocument;
class AnnotationListDock;
class ImageCanvas;
class QAction;
class QActionGroup;
class QComboBox;
class QLabel;
class QToolButton;
class QUndoStack;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Открывает картинку или разметку (.json), смотрит по расширению.
    bool openPath(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void openImage();
    void openAnnotations();
    bool saveAnnotations();
    bool saveAnnotationsAs();
    void exportTxt();
    void chooseColor();
    void deleteSelected();
    void showShortcuts();
    void showAbout();

private:
    void createTools();
    void createActions();
    void createToolBars();
    void createStatusBar();

    bool openImageFile(const QString &path);
    bool openAnnotationFile(const QString &path);
    bool loadImage(const QString &path, QImage &image);
    bool maybeSave();
    void setColor(const QColor &color);
    void activateTool(int index);
    void updateLabelChoices();
    void updateTitle();
    void updateActions();
    QString defaultOutputPath(const QString &suffix) const;
    QString lastDir() const;
    void rememberDir(const QString &filePath);

    AnnotationDocument *document_;
    QUndoStack *undoStack_;
    ImageCanvas *canvas_;
    AnnotationListDock *listDock_;

    ToolContext toolContext_;
    std::vector<std::unique_ptr<Tool>> tools_;
    QActionGroup *toolGroup_ = nullptr;

    QAction *saveAction_ = nullptr;
    QAction *saveAsAction_ = nullptr;
    QAction *exportTxtAction_ = nullptr;
    QAction *deleteAction_ = nullptr;
    QAction *fillAction_ = nullptr;
    QToolButton *colorButton_ = nullptr;
    QComboBox *labelCombo_ = nullptr;

    QLabel *cursorLabel_ = nullptr;
    QLabel *zoomLabel_ = nullptr;
    QLabel *sizeLabel_ = nullptr;

    QString annotationPath_;
};
