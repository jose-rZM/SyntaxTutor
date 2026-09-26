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

#ifndef APPFONTS_H
#define APPFONTS_H

#include <QFont>
#include <QFontDatabase>

/**
 * @brief Application-wide monospace font.
 *
 * macOS keeps the native fixed font (Menlo); Windows and Linux use the
 * bundled JetBrains Mono, loaded in main(), because their system fixed
 * fonts (Courier New, distro-dependent) degrade the visuals.
 */
inline QFont appMonospaceFont() {
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
#ifndef Q_OS_MACOS
    font.setFamily(QStringLiteral("JetBrains Mono"));
    font.setStyleHint(QFont::Monospace);
#endif
    return font;
}

#endif // APPFONTS_H
