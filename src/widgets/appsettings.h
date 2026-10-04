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

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QCoreApplication>
#include <QSettings>
#include <QString>

/**
 * @brief Where the app keeps its settings.
 *
 * The identity is registered once at startup and every QSettings is built
 * from it, so each platform files the settings its own way:
 * - macOS: ~/Library/Preferences/me.jram.SyntaxTutor.plist (the domain)
 * - Windows: HKEY_CURRENT_USER\Software\SyntaxTutor\SyntaxTutor
 * - Linux: ~/.config/SyntaxTutor/SyntaxTutor.conf
 */
namespace AppSettings {

/// @brief Registers the organisation, domain and application name.
inline void registerIdentity() {
    QCoreApplication::setOrganizationName(QStringLiteral("SyntaxTutor"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("jram.me"));
    QCoreApplication::setApplicationName(QStringLiteral("SyntaxTutor"));
}

/**
 * @brief The app's settings.
 *
 * Built without explicit names so macOS uses the registered domain. Test
 * builds use a store of their own, so running the tests never touches the
 * user's progress.
 */
inline QSettings open() {
#ifdef SYNTAXTUTOR_TESTING
    return QSettings(QStringLiteral("SyntaxTutor-Test"),
                     QStringLiteral("SyntaxTutor-Test"));
#else
    return QSettings();
#endif
}

} // namespace AppSettings

#endif // APPSETTINGS_H
