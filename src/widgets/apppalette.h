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

#ifndef APPPALETTE_H
#define APPPALETTE_H

#include <QApplication>
#include <QColor>
#include <QPalette>

namespace AppPalette {

/// @brief Colour of links in rich text: the teal accent of the theme.
inline QColor linkColor() { return QColor(0x36, 0xC5, 0xCC); }

/**
 * @brief Palette entries the theme needs that app.qss cannot set.
 *
 * Links in rich text (QLabel, QTextBrowser) take their colour from the
 * palette; a style sheet rule on `a` has no effect. The platform's light
 * palette makes them dark blue, barely readable on this dark theme - which
 * is what Linux showed, while macOS hid it with a dark palette of its own.
 */
inline void apply() {
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Link, linkColor());
    palette.setColor(QPalette::LinkVisited, linkColor());
    QApplication::setPalette(palette);
}

} // namespace AppPalette

#endif // APPPALETTE_H
