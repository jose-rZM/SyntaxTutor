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
#include "apptextscale.h"
#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFontDatabase>
#include <QSettings>
#include <QStyleHints>
#include <QTranslator>

void applyAppStyle(QApplication& app) {
    QFile qssFile(":/resources/styles/app.qss");
    if (!qssFile.open(QFile::ReadOnly | QFile::Text)) {
        return;
    }
    app.setStyleSheet(
        AppTextScale::scaleStyleSheet(QString::fromUtf8(qssFile.readAll())));
}

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
#endif
    QApplication::setStyle("fusion");
    AppTextScale::baseApplicationFont() = QApplication::font();
#ifndef Q_OS_MACOS
    QFontDatabase::addApplicationFont(
        ":/resources/fonts/JetBrainsMono-Regular.ttf");
    QFontDatabase::addApplicationFont(
        ":/resources/fonts/JetBrainsMono-Bold.ttf");
#endif
    QCoreApplication::setApplicationName("SyntaxTutor");
    QGuiApplication::setApplicationDisplayName("SyntaxTutor");
    QCoreApplication::setApplicationVersion(SyntaxTutor::Version::current());
    QSettings settings("UMA", "SyntaxTutor");
    if (settings.contains(AppTextScale::settingsKey())) {
        AppTextScale::currentPercent() = AppTextScale::clampPercent(
            settings.value(AppTextScale::settingsKey()).toInt());
    } else {
        AppTextScale::currentPercent() = AppTextScale::percentFromLegacyValue(
            settings.value(AppTextScale::legacySettingsKey()).toString());
    }
    QString   langCode = settings.value("lang/language", "es").toString();
    QTranslator   translator;
    const QString qmPath = langCode == QStringLiteral("en")
                               ? QStringLiteral(":/translations/st_en.qm")
                               : QStringLiteral(":/translations/st_es.qm");
    if (translator.load(qmPath)) {
        a.installTranslator(&translator);
    } else {
        // Not fatal: tr() then falls back to the Spanish source strings.
        qWarning() << "Could not load translations from" << qmPath;
    }
    a.setFont(AppTextScale::scaledApplicationFont());
    applyAppStyle(a);
    MainWindow w;
    w.show();
    return a.exec();
}
