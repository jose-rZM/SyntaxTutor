#include "mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QImageReader>
#include <QSettings>
#include <QTranslator>

void loadFonts() {
    QFontDatabase::addApplicationFont(":/resources/NotoSans-Regular.ttf");
    QFontDatabase::addApplicationFont(":/resources/NotoSans-Italic.ttf");
    QFontDatabase::addApplicationFont(":/resources/NotoSans-Bold.ttf");
}

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("SyntaxTutor");
    QGuiApplication::setApplicationDisplayName("SyntaxTutor");
    QSettings   settings("UMA", "SyntaxTutor");
    QString     langCode = settings.value("lang/language", "es").toString();
    QTranslator translator;
    if (langCode == "en") {
        bool x = translator.load(":/translations/st_en.qm");
    } else {
        bool x = translator.load(":/translations/st_es.qm");
    }
    a.installTranslator(&translator);
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
