#include "ui/MainWindow.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("NIR"));
    QApplication::setApplicationName(QStringLiteral("ImageAnnotator"));

    // Интерфейс программы на русском, поэтому и стандартные кнопки и диалоги Qt
    // (Cancel, Save, выбор цвета) переводятся на русский независимо от языка системы.
    // Рядом с программой перевод ищется на случай сборки для Windows через windeployqt.
    QTranslator qtTranslator;
    if (qtTranslator.load(QStringLiteral("qtbase_ru"), QLibraryInfo::location(QLibraryInfo::TranslationsPath))
        || qtTranslator.load(QStringLiteral("qtbase_ru"), QApplication::applicationDirPath() + QStringLiteral("/translations")))
        QApplication::installTranslator(&qtTranslator);

    MainWindow window;
    window.show();

    // Изображение или файл разметки можно передать аргументом командной строки.
    const QStringList args = QApplication::arguments();
    if (args.size() > 1)
        window.openPath(args.at(1));

    return app.exec();
}
