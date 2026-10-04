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
#include "appicon.h"
#include "appsettings.h"
#include "apppalette.h"
#include "apptypography.h"
#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QFontDatabase>
#include <QSettings>
#include <QStyleHints>
#include <QTranslator>

void applyAppStyle(QApplication& app) {
    const QString styleSheet = AppTypography::loadStyleSheet();
    if (!styleSheet.isEmpty()) {
        app.setStyleSheet(styleSheet);
    }
}

int main(int argc, char* argv[]) {
#ifdef Q_OS_LINUX
    // X11 first - through XWayland on a Wayland desktop, the tested path -
    // and native Wayland only where there is no X server. Without this, Qt
    // picks Wayland on its own as soon as its plugin is present. Native
    // Wayland can still be asked for with QT_QPA_PLATFORM=wayland.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "xcb;wayland");
    }
#endif
    QApplication a(argc, argv);
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
#endif
    QApplication::setStyle("fusion");
    AppPalette::apply();
    QApplication::setWindowIcon(AppIcon::icon());
    AppTypography::systemFont() = QApplication::font();
#ifndef Q_OS_MACOS
    QFontDatabase::addApplicationFont(
        ":/resources/fonts/JetBrainsMono-Regular.ttf");
    QFontDatabase::addApplicationFont(
        ":/resources/fonts/JetBrainsMono-Bold.ttf");
#endif
    AppSettings::registerIdentity();
    QGuiApplication::setApplicationDisplayName("SyntaxTutor");
    QCoreApplication::setApplicationVersion(SyntaxTutor::Version::current());
    QSettings settings = AppSettings::open();
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
    a.setFont(AppTypography::applicationFont());
    applyAppStyle(a);
    MainWindow w;
    w.show();
    return a.exec();
}
