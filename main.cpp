#include "mainwindow.h"

#include <QApplication>
#include <QFontDatabase>
#include <QImageReader>

void loadFonts()
{
    QFontDatabase::addApplicationFont(":/resources/NotoSans-Regular.ttf");
    QFontDatabase::addApplicationFont(":/resources/NotoSans-Italic.ttf");
    QFontDatabase::addApplicationFont(":/resources/NotoSans-Bold.ttf");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    loadFonts();
    MainWindow w;
    w.show();
    return a.exec();
}
