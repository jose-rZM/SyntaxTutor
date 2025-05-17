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
    loadFonts();
    MainWindow w;
    w.show();
    return a.exec();
}
