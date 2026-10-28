#include "ui/MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("NIR"));
    QApplication::setApplicationName(QStringLiteral("ImageAnnotator"));

    MainWindow window;
    window.show();

    // Изображение или файл разметки можно передать аргументом командной строки.
    const QStringList args = QApplication::arguments();
    if (args.size() > 1)
        window.openPath(args.at(1));

    return app.exec();
}
