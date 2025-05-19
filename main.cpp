#include "mainwindow.h"

#include <QApplication>
#include <QFontDatabase>
#include <QImageReader>
#include <QFont>

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
#ifdef Q_OS_WIN
    QFont notoSans("Noto Sans");
    notoSans.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
    QApplication::setFont(notoSans);
#else
    QFont notoSans("Noto Sans");
    notoSans.setStyleStrategy(QFont::PreferQuality);
    QApplication::setFont(notoSans);
#endif
    MainWindow w;
    w.show();
    return a.exec();
}
