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

#ifndef APPSHORTCUTS_H
#define APPSHORTCUTS_H

#include <QString>

namespace AppShortcuts {

/**
 * @brief Name of the key behind Qt::ControlModifier on this platform.
 *
 * On macOS Qt maps ControlModifier to the Command key, so a shortcut
 * written for Ctrl is pressed with Cmd there. Use this in any text that
 * names that key, so it matches what the user actually presses.
 */
inline QString primaryModifierName() {
#ifdef Q_OS_MACOS
    return QStringLiteral("Cmd");
#else
    return QStringLiteral("Ctrl");
#endif
}

} // namespace AppShortcuts

#endif // APPSHORTCUTS_H
