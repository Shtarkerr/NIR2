#include "ui/MainWindow.h"

#include "commands/AnnotationCommands.h"
#include "io/JsonSerializer.h"
#include "io/TxtExporter.h"
#include "model/AnnotationDocument.h"
#include "tools/ContourTool.h"
#include "tools/DragShapeTool.h"
#include "tools/PointTool.h"
#include "tools/SelectTool.h"
#include "ui/AnnotationListDock.h"
#include "ui/Icons.h"
#include "ui/ImageCanvas.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QUndoStack>

namespace {

QString imageFilter()
{
    QStringList patterns;
    for (const QByteArray &format : QImageReader::supportedImageFormats())
        patterns << QStringLiteral("*.") + QString::fromLatin1(format);
    return MainWindow::tr("Изображения (%1);;Все файлы (*)").arg(patterns.join(' '));
}

QString annotationFilter()
{
    return MainWindow::tr("Разметка JSON (*.json)");
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      document_(new AnnotationDocument(this)),
      undoStack_(new QUndoStack(this)),
      canvas_(new ImageCanvas(this)),
      listDock_(new AnnotationListDock(document_, undoStack_, this))
{
    setCentralWidget(canvas_);
    addDockWidget(Qt::RightDockWidgetArea, listDock_);
    canvas_->setDocument(document_);

    toolContext_.canvas = canvas_;
    toolContext_.document = document_;
    toolContext_.undoStack = undoStack_;

    createTools();
    createActions();
    createToolBars();
    createStatusBar();

    connect(undoStack_, &QUndoStack::cleanChanged, this, [this](bool clean) { setWindowModified(!clean); });
    connect(document_, &AnnotationDocument::labelsChanged, this, &MainWindow::updateLabelChoices);
    connect(canvas_, &ImageCanvas::selectedIdChanged, listDock_, &AnnotationListDock::setSelectedId);
    connect(canvas_, &ImageCanvas::selectedIdChanged, this, &MainWindow::updateActions);
    connect(listDock_, &AnnotationListDock::annotationSelected, this, [this](int id) {
        canvas_->setSelectedId(id);
        canvas_->ensureAnnotationVisible(id);
    });

    setAcceptDrops(true);
    resize(1280, 800);

    QSettings settings;
    restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
    restoreState(settings.value(QStringLiteral("windowState")).toByteArray());

    setColor(Qt::red);
    activateTool(0);
    updateTitle();
    updateActions();
    canvas_->setFocus();
}

MainWindow::~MainWindow()
{
    // Инструменты уничтожаются раньше дочерних виджетов, а холст ещё получает события
    // (например, потерю фокуса). Документ удаляется раньше холста, элементы которого
    // ссылаются на его объекты. Поэтому отвязываем всё заранее.
    canvas_->setTool(nullptr);
    canvas_->setDocument(nullptr);
    undoStack_->disconnect(this);
}

void MainWindow::createTools()
{
    tools_.push_back(std::make_unique<SelectTool>(toolContext_));
    tools_.push_back(std::make_unique<PointTool>(toolContext_));
    tools_.push_back(std::make_unique<RectTool>(toolContext_));
    tools_.push_back(std::make_unique<EllipseTool>(toolContext_));
    tools_.push_back(std::make_unique<ContourTool>(toolContext_));
}

void MainWindow::createActions()
{
    // Файл
    QMenu *fileMenu = menuBar()->addMenu(tr("&Файл"));
    QAction *openImageAction = fileMenu->addAction(tr("Открыть &изображение..."), this, &MainWindow::openImage);
    openImageAction->setShortcut(QKeySequence::Open);
    QAction *openAnnotationsAction = fileMenu->addAction(tr("Открыть &разметку..."), this, &MainWindow::openAnnotations);
    openAnnotationsAction->setShortcut(QKeySequence(tr("Ctrl+Shift+O")));
    fileMenu->addSeparator();
    saveAction_ = fileMenu->addAction(tr("&Сохранить разметку"), this, &MainWindow::saveAnnotations);
    saveAction_->setShortcut(QKeySequence::Save);
    saveAsAction_ = fileMenu->addAction(tr("Сохранить разметку &как..."), this, &MainWindow::saveAnnotationsAs);
    saveAsAction_->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));
    exportTxtAction_ = fileMenu->addAction(tr("&Экспорт в TXT..."), this, &MainWindow::exportTxt);
    exportTxtAction_->setShortcut(QKeySequence(tr("Ctrl+E")));
    fileMenu->addSeparator();
    QAction *quitAction = fileMenu->addAction(tr("В&ыход"), this, &QWidget::close);
    quitAction->setShortcut(QKeySequence::Quit);

    // Правка
    QMenu *editMenu = menuBar()->addMenu(tr("&Правка"));
    QAction *undoAction = undoStack_->createUndoAction(this, tr("&Отменить"));
    undoAction->setShortcut(QKeySequence::Undo);
    QAction *redoAction = undoStack_->createRedoAction(this, tr("&Повторить"));
    redoAction->setShortcuts({QKeySequence(tr("Ctrl+Shift+Z")), QKeySequence(tr("Ctrl+Y"))});
    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    deleteAction_ = editMenu->addAction(tr("&Удалить объект"), this, &MainWindow::deleteSelected);
    deleteAction_->setShortcut(QKeySequence::Delete);

    // Вид
    QMenu *viewMenu = menuBar()->addMenu(tr("&Вид"));
    QAction *zoomInAction = viewMenu->addAction(tr("У&величить"), canvas_, &ImageCanvas::zoomIn);
    zoomInAction->setShortcuts({QKeySequence::ZoomIn, QKeySequence(tr("Ctrl+="))});
    QAction *zoomOutAction = viewMenu->addAction(tr("У&меньшить"), canvas_, &ImageCanvas::zoomOut);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    QAction *fitAction = viewMenu->addAction(tr("&Вписать в окно"), canvas_, &ImageCanvas::zoomToFit);
    fitAction->setShortcut(QKeySequence(tr("Ctrl+0")));
    QAction *actualAction = viewMenu->addAction(tr("&Исходный размер (100%)"), canvas_, &ImageCanvas::zoomActual);
    actualAction->setShortcut(QKeySequence(tr("Ctrl+1")));
    viewMenu->addSeparator();
    viewMenu->addAction(listDock_->toggleViewAction());

    // Инструменты
    QMenu *toolsMenu = menuBar()->addMenu(tr("&Инструменты"));
    toolGroup_ = new QActionGroup(this);
    toolGroup_->setExclusive(true);
    const Icons::Glyph glyphs[] = {Icons::Glyph::Select, Icons::Glyph::Point, Icons::Glyph::Rect,
                                   Icons::Glyph::Ellipse, Icons::Glyph::Contour};
    const char *shortcuts[] = {"V", "P", "R", "E", "C"};
    for (int i = 0; i < static_cast<int>(tools_.size()); ++i) {
        QAction *action = new QAction(Icons::glyph(glyphs[i]), tools_[i]->name(), toolGroup_);
        action->setCheckable(true);
        action->setShortcut(QKeySequence(QString::fromLatin1(shortcuts[i])));
        action->setToolTip(QStringLiteral("%1 (%2)").arg(tools_[i]->name(), QString::fromLatin1(shortcuts[i])));
        connect(action, &QAction::triggered, this, [this, i] { activateTool(i); });
        toolsMenu->addAction(action);
    }
    toolsMenu->addSeparator();

    fillAction_ = new QAction(Icons::glyph(Icons::Glyph::Fill), tr("&Заливка фигур"), this);
    fillAction_->setCheckable(true);
    fillAction_->setShortcut(QKeySequence(tr("F")));
    fillAction_->setToolTip(tr("Сплошная заливка прямоугольников и эллипсов (F)"));
    connect(fillAction_, &QAction::toggled, this, [this](bool on) { toolContext_.style.filled = on; });
    toolsMenu->addAction(fillAction_);
    toolsMenu->addAction(tr("&Цвет..."), this, &MainWindow::chooseColor);

    // Справка
    QMenu *helpMenu = menuBar()->addMenu(tr("&Справка"));
    helpMenu->addAction(tr("&Управление"), this, &MainWindow::showShortcuts)->setShortcut(QKeySequence::HelpContents);
    helpMenu->addAction(tr("&О программе"), this, &MainWindow::showAbout);
}

void MainWindow::createToolBars()
{
    QToolBar *toolBar = addToolBar(tr("Инструменты"));
    toolBar->setObjectName(QStringLiteral("ToolsToolBar"));
    toolBar->setIconSize(QSize(24, 24));
    toolBar->addActions(toolGroup_->actions());
    toolBar->addSeparator();

    colorButton_ = new QToolButton(toolBar);
    colorButton_->setIconSize(QSize(20, 20));
    connect(colorButton_, &QToolButton::clicked, this, &MainWindow::chooseColor);
    toolBar->addWidget(colorButton_);
    toolBar->addAction(fillAction_);
    toolBar->addSeparator();

    toolBar->addWidget(new QLabel(tr(" Метка: "), toolBar));
    labelCombo_ = new QComboBox(toolBar);
    labelCombo_->setEditable(true);
    labelCombo_->setInsertPolicy(QComboBox::NoInsert);
    labelCombo_->setMinimumWidth(180);
    labelCombo_->lineEdit()->setPlaceholderText(tr("без метки"));
    labelCombo_->setToolTip(tr("Метка для новых объектов. Введите новую или выберите из списка."));
    connect(labelCombo_, &QComboBox::currentTextChanged, this,
            [this](const QString &text) { toolContext_.style.label = text.trimmed(); });
    connect(labelCombo_->lineEdit(), &QLineEdit::returnPressed, canvas_,
            [this] { canvas_->setFocus(); });
    toolBar->addWidget(labelCombo_);
}

void MainWindow::createStatusBar()
{
    cursorLabel_ = new QLabel(this);
    sizeLabel_ = new QLabel(this);
    zoomLabel_ = new QLabel(this);
    cursorLabel_->setMinimumWidth(140);
    statusBar()->addPermanentWidget(cursorLabel_);
    statusBar()->addPermanentWidget(sizeLabel_);
    statusBar()->addPermanentWidget(zoomLabel_);

    connect(canvas_, &ImageCanvas::cursorPixelChanged, this, [this](const QPoint &p, bool inside) {
        cursorLabel_->setText(inside ? tr("x: %1, y: %2").arg(p.x()).arg(p.y()) : QString());
    });
    connect(canvas_, &ImageCanvas::zoomChanged, this, [this](qreal scale) {
        zoomLabel_->setText(tr("Масштаб: %1%").arg(qRound(scale * 100)));
    });
}

bool MainWindow::openPath(const QString &path)
{
    if (!maybeSave())
        return false;
    if (QFileInfo(path).suffix().compare(QLatin1String("json"), Qt::CaseInsensitive) == 0)
        return openAnnotationFile(path);
    return openImageFile(path);
}

void MainWindow::openImage()
{
    if (!maybeSave())
        return;
    const QString path = QFileDialog::getOpenFileName(this, tr("Открыть изображение"), lastDir(), imageFilter());
    if (!path.isEmpty())
        openImageFile(path);
}

void MainWindow::openAnnotations()
{
    if (!maybeSave())
        return;
    const QString path = QFileDialog::getOpenFileName(this, tr("Открыть разметку"), lastDir(), annotationFilter());
    if (!path.isEmpty())
        openAnnotationFile(path);
}

bool MainWindow::loadImage(const QString &path, QImage &image)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    image = reader.read();
    if (image.isNull()) {
        QMessageBox::warning(this, tr("Ошибка"),
                             tr("Не удалось открыть изображение «%1»:\n%2")
                                 .arg(QFileInfo(path).fileName(), reader.errorString()));
        return false;
    }
    return true;
}

bool MainWindow::openImageFile(const QString &path)
{
    QImage image;
    if (!loadImage(path, image))
        return false;

    // Набор меток сохраняется при переходе к следующему изображению серии.
    const QStringList labels = document_->labels();
    canvas_->setImage(image);
    document_->reset(QFileInfo(path).absoluteFilePath(), image.size());
    document_->setLabels(labels);
    undoStack_->clear();
    annotationPath_.clear();

    rememberDir(path);
    updateTitle();
    updateActions();
    statusBar()->showMessage(tr("Открыто изображение %1").arg(QFileInfo(path).fileName()), 3000);
    return true;
}

bool MainWindow::openAnnotationFile(const QString &path)
{
    AnnotationDocument loaded;
    QString error;
    if (!JsonSerializer::load(loaded, path, &error)) {
        QMessageBox::warning(this, tr("Ошибка"),
                             tr("Не удалось загрузить разметку «%1»:\n%2").arg(QFileInfo(path).fileName(), error));
        return false;
    }

    QString imagePath = loaded.imagePath();
    if (imagePath.isEmpty() || !QFileInfo::exists(imagePath)) {
        QMessageBox::information(this, tr("Изображение не найдено"),
                                 tr("Изображение «%1» не найдено. Укажите его расположение.")
                                     .arg(QFileInfo(imagePath).fileName()));
        imagePath = QFileDialog::getOpenFileName(this, tr("Изображение для разметки"),
                                                 QFileInfo(path).absolutePath(), imageFilter());
        if (imagePath.isEmpty())
            return false;
    }

    QImage image;
    if (!loadImage(imagePath, image))
        return false;
    if (loaded.imageSize().isValid() && loaded.imageSize() != image.size()) {
        QMessageBox::warning(this, tr("Внимание"),
                             tr("Размер изображения (%1×%2) не совпадает с указанным в разметке (%3×%4).")
                                 .arg(image.width()).arg(image.height())
                                 .arg(loaded.imageSize().width()).arg(loaded.imageSize().height()));
    }

    canvas_->setImage(image);
    document_->reset(QFileInfo(imagePath).absoluteFilePath(), image.size());
    document_->setLabels(loaded.labels());
    for (const auto &annotation : loaded.annotations())
        document_->add(annotation->clone());
    undoStack_->clear();
    annotationPath_ = QFileInfo(path).absoluteFilePath();

    rememberDir(path);
    updateTitle();
    updateActions();
    statusBar()->showMessage(tr("Загружено объектов: %1").arg(document_->count()), 3000);
    return true;
}

bool MainWindow::saveAnnotations()
{
    if (!canvas_->hasImage())
        return false;
    if (annotationPath_.isEmpty())
        return saveAnnotationsAs();

    QString error;
    if (!JsonSerializer::save(*document_, annotationPath_, &error)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось сохранить разметку:\n%1").arg(error));
        return false;
    }
    undoStack_->setClean();
    updateTitle();
    statusBar()->showMessage(tr("Разметка сохранена в %1").arg(annotationPath_), 3000);
    return true;
}

bool MainWindow::saveAnnotationsAs()
{
    if (!canvas_->hasImage())
        return false;
    QString path = QFileDialog::getSaveFileName(
        this, tr("Сохранить разметку"),
        annotationPath_.isEmpty() ? defaultOutputPath(QStringLiteral("json")) : annotationPath_,
        annotationFilter());
    if (path.isEmpty())
        return false;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".json");
    annotationPath_ = path;
    rememberDir(path);
    return saveAnnotations();
}

void MainWindow::exportTxt()
{
    if (!canvas_->hasImage())
        return;
    QString path = QFileDialog::getSaveFileName(this, tr("Экспорт в текстовый файл"),
                                                defaultOutputPath(QStringLiteral("txt")),
                                                tr("Текстовые файлы (*.txt)"));
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".txt");

    QString error;
    if (!TxtExporter::save(*document_, path, &error)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось экспортировать разметку:\n%1").arg(error));
        return;
    }
    statusBar()->showMessage(tr("Разметка экспортирована в %1").arg(path), 3000);
}

void MainWindow::chooseColor()
{
    const QColor color = QColorDialog::getColor(toolContext_.style.color, this, tr("Цвет выделения"),
                                                QColorDialog::ShowAlphaChannel);
    if (color.isValid())
        setColor(color);
}

void MainWindow::setColor(const QColor &color)
{
    toolContext_.style.color = color;
    colorButton_->setIcon(Icons::colorSwatch(color, 20));
    colorButton_->setToolTip(tr("Текущий цвет: %1").arg(Annotation::colorToString(color)));
}

void MainWindow::deleteSelected()
{
    const int id = canvas_->selectedId();
    if (id)
        undoStack_->push(new RemoveAnnotationCommand(document_, id));
}

void MainWindow::activateTool(int index)
{
    canvas_->setTool(tools_[static_cast<size_t>(index)].get());
    toolGroup_->actions().at(index)->setChecked(true);
}

void MainWindow::updateLabelChoices()
{
    const QString current = labelCombo_->currentText();
    const QSignalBlocker blocker(labelCombo_);
    labelCombo_->clear();
    labelCombo_->addItems(document_->labels());
    labelCombo_->setEditText(current);
}

void MainWindow::updateTitle()
{
    const QString name = canvas_->hasImage() ? QFileInfo(document_->imagePath()).fileName()
                                             : tr("нет изображения");
    setWindowTitle(tr("%1[*] — Разметка изображений").arg(name));
    setWindowModified(!undoStack_->isClean());
    sizeLabel_->setText(canvas_->hasImage()
                            ? tr("%1×%2").arg(canvas_->imageSize().width()).arg(canvas_->imageSize().height())
                            : QString());
}

void MainWindow::updateActions()
{
    const bool hasImage = canvas_->hasImage();
    saveAction_->setEnabled(hasImage);
    saveAsAction_->setEnabled(hasImage);
    exportTxtAction_->setEnabled(hasImage);
    deleteAction_->setEnabled(canvas_->selectedId() != 0);
}

QString MainWindow::defaultOutputPath(const QString &suffix) const
{
    const QFileInfo image(document_->imagePath());
    return image.absolutePath() + QLatin1Char('/') + image.completeBaseName() + QLatin1Char('.') + suffix;
}

QString MainWindow::lastDir() const
{
    return QSettings().value(QStringLiteral("lastDir")).toString();
}

void MainWindow::rememberDir(const QString &filePath)
{
    QSettings().setValue(QStringLiteral("lastDir"), QFileInfo(filePath).absolutePath());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!maybeSave()) {
        event->ignore();
        return;
    }
    QSettings settings;
    settings.setValue(QStringLiteral("geometry"), saveGeometry());
    settings.setValue(QStringLiteral("windowState"), saveState());
    event->accept();
}

bool MainWindow::maybeSave()
{
    if (undoStack_->isClean())
        return true;
    const auto answer = QMessageBox::warning(
        this, tr("Несохранённые изменения"), tr("Разметка изменена. Сохранить изменения?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (answer == QMessageBox::Save)
        return saveAnnotations();
    return answer == QMessageBox::Discard;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty() && urls.first().isLocalFile())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty() || !urls.first().isLocalFile())
        return;
    event->acceptProposedAction();
    openPath(urls.first().toLocalFile());
}

void MainWindow::showShortcuts()
{
    QMessageBox::information(this, tr("Управление"), tr(
        "<table cellspacing='4'>"
        "<tr><td><b>V</b></td><td>выбор и перемещение объектов (стрелки — сдвиг на 1 px, Shift — на 10 px)</td></tr>"
        "<tr><td><b>P</b></td><td>отметка точки</td></tr>"
        "<tr><td><b>R</b></td><td>прямоугольник (Shift — квадрат)</td></tr>"
        "<tr><td><b>E</b></td><td>эллипс (Shift — круг)</td></tr>"
        "<tr><td><b>C</b></td><td>контур: обвести с зажатой кнопкой или ставить вершины щелчками; "
        "двойной щелчок или Enter — завершить, Backspace — убрать вершину, Esc — отменить</td></tr>"
        "<tr><td><b>F</b></td><td>заливка прямоугольников и эллипсов</td></tr>"
        "<tr><td><b>Del</b></td><td>удалить выбранный объект</td></tr>"
        "<tr><td><b>Колесо</b></td><td>масштаб</td></tr>"
        "<tr><td><b>Пробел + мышь, средняя кнопка</b></td><td>прокрутка изображения</td></tr>"
        "<tr><td><b>Ctrl+Z / Ctrl+Y</b></td><td>отменить / повторить</td></tr>"
        "</table>"));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("О программе"), tr(
        "<h3>Разметка изображений</h3>"
        "<p>Программа-помощник для разметки изображений точками, прямоугольниками, "
        "эллипсами и произвольными контурами.</p>"
        "<p>НИР, 2 курс. C++17, Qt %1.</p>").arg(QString::fromLatin1(qVersion())));
}
