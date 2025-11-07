/*
 * SyntaxTutor - Interactive Tutorial About Syntax Analyzers
 * Copyright (C) 2025 Jose R. (jose-rzm)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "appversion.h"
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
    QCoreApplication::setApplicationVersion(SyntaxTutor::Version::current());
    QSettings   settings("UMA", "SyntaxTutor");
    QString     langCode = settings.value("lang/language", "es").toString();
    QTranslator translator;
    if (langCode == "en") {
        translator.load(":/translations/st_en.qm");
    } else {
        translator.load(":/translations/st_es.qm");
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
