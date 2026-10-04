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

#ifndef APPICON_H
#define APPICON_H

#include <QIcon>
#include <QString>

namespace AppIcon {

/// @brief Sizes of the bitmap icon shipped in the resources, in pixels.
constexpr int kSizes[] = {16, 24, 32, 48, 64, 128, 256, 512};

/**
 * @brief The application icon, with a drawing for each size.
 *
 * Every size is its own bitmap, so a 16 px title bar icon is drawn for 16
 * px rather than shrunk from a large image. Set once on the application,
 * it is the icon of every window and dialog.
 */
inline QIcon icon() {
    QIcon icon;
    for (const int size : kSizes) {
        icon.addFile(
            QStringLiteral(":/resources/icon/png/syntaxtutor-%1.png").arg(size),
            QSize(size, size));
    }
    return icon;
}

} // namespace AppIcon

#endif // APPICON_H
