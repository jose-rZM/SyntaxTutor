#include "mainwindow.h"

#include <QApplication>
#include <QFontDatabase>
#include <QImageReader>

QString loadFonts()
{
    int id1 = QFontDatabase::addApplicationFont(":/resources/NotoSans-Regular.ttf");
    int id2 = QFontDatabase::addApplicationFont(":/resources/NotoSans-Italic.ttf");
    int id3 = QFontDatabase::addApplicationFont(":/resources/NotoSans-Bold.ttf");

    if (id1 == -1 || id2 == -1 || id3 == -1)
        return "";

    QString family = QFontDatabase::applicationFontFamilies(id1).at(0);
    return family;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString fontFamily = loadFonts();
    if (!fontFamily.isEmpty()) {
        QFont defaultFont(fontFamily);
        QApplication::setFont(defaultFont);
    }
    MainWindow w;
    w.show();
    return a.exec();
}
