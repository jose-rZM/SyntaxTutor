#include "mainwindow.h"

#include <QApplication>
#include <QFontDatabase>
#include <QImageReader>

void loadFonts()
{
    int id1 = QFontDatabase::addApplicationFont(":/resources/NotoSans-Regular.ttf");
    int id2 = QFontDatabase::addApplicationFont(":/resources/NotoSans-Italic.ttf");
    int id3 = QFontDatabase::addApplicationFont(":/resources/NotoSans-Bold.ttf");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
<<<<<<< Updated upstream
<<<<<<< Updated upstream
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QString fontFamily = loadFonts();
    if (!fontFamily.isEmpty()) {
        QFont defaultFont(fontFamily);
        defaultFont.setPointSizeF(10);

        QApplication::setFont(defaultFont);

    }
=======

    loadFonts();
>>>>>>> Stashed changes
=======

    loadFonts();
>>>>>>> Stashed changes
    MainWindow w;
    w.show();
    return a.exec();
}
