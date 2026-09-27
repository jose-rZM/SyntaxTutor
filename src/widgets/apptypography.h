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

#ifndef APPTYPOGRAPHY_H
#define APPTYPOGRAPHY_H

#include "apptextscale.h"

#include <QFile>
#include <QFont>
#include <QFontInfo>
#include <QGuiApplication>
#include <QLatin1StringView>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QtGlobal>

/**
 * @brief The application's type scale: every text size on screen, in points.
 *
 * Fonts are sized in points so Qt can map them to each platform's logical
 * DPI; nothing here reads the DPI or the device pixel ratio. The user's text
 * size preference (@ref AppTextScale) is a plain multiplier on top.
 *
 * `app.qss` never spells a size out. It names a role, as in
 * `font-size: $font-body;`, and @ref resolveStyleSheet substitutes the
 * scaled value when the sheet is loaded. C++ code asks for the same roles
 * through @ref font or @ref cssSize, so both channels share one table.
 *
 * Paper is a separate domain: the PDF exports keep their own `pt` sizes and
 * never follow the screen preference.
 */
namespace AppTypography {

/// @brief What a piece of text is for, which fixes its size.
enum class Role {
    Micro,      ///< Automaton edge labels.
    Caption,    ///< Uppercase captions and the progress panel.
    Label,      ///< Eyebrows, badges, small buttons and hints.
    Body,       ///< Default UI text: chat, tables, dialog buttons.
    BodyLarge,  ///< Primary buttons and subtitles.
    Reading,    ///< Main reading size: answers, hints, grammar, home options.
    Action,     ///< The large actions on the home page.
    Headline,   ///< Transient celebration text.
    TitleSmall, ///< Titles of the small information dialogs.
    Title,      ///< Titles of the large dialogs.
    Hero,       ///< The home page title.
    Display,    ///< The exam grade.
};

/// @brief One entry of the type scale.
struct Token {
    Role              role;
    QLatin1StringView name;   ///< Name used in app.qss after `$font-`.
    qreal             points; ///< Size at a 100% text scale.
};

/**
 * @brief The type scale.
 *
 * The values reproduce the original pixel design exactly on macOS, the
 * platform it was drawn on, where one point and one logical pixel coincide.
 * Other platforms get the same physical size instead of the same pixel
 * count.
 */
inline constexpr Token kTokens[] = {
    {Role::Micro, QLatin1StringView("micro"), 10.0},
    {Role::Caption, QLatin1StringView("caption"), 11.0},
    {Role::Label, QLatin1StringView("label"), 12.0},
    {Role::Body, QLatin1StringView("body"), 13.0},
    {Role::BodyLarge, QLatin1StringView("body-large"), 14.0},
    {Role::Reading, QLatin1StringView("reading"), 15.0},
    {Role::Action, QLatin1StringView("action"), 16.0},
    {Role::Headline, QLatin1StringView("headline"), 20.0},
    {Role::TitleSmall, QLatin1StringView("title-small"), 24.0},
    {Role::Title, QLatin1StringView("title"), 28.0},
    {Role::Hero, QLatin1StringView("hero"), 30.0},
    {Role::Display, QLatin1StringView("display"), 56.0},
};

/// @brief Size of @p role at a 100% text scale, in points.
inline qreal basePoints(Role role) {
    for (const Token& token : kTokens) {
        if (token.role == role) {
            return token.points;
        }
    }
    Q_UNREACHABLE_RETURN(13.0);
}

/// @brief Size of @p role at the current text scale, in points.
inline qreal points(Role role) {
    return basePoints(role) * AppTextScale::factor();
}

/// @brief A point size written the way CSS and QSS expect it, e.g. `14.5pt`.
inline QString formatPoints(qreal points) {
    return QString::number(points, 'g', 6) + QStringLiteral("pt");
}

/// @brief Current size of @p role as a CSS length, for rich text and QSS.
inline QString cssSize(Role role) { return formatPoints(points(role)); }

/**
 * @brief The system font as it was before the app touched it.
 *
 * Captured once at startup. Only its family and rendering hints are used:
 * the size always comes from the type scale.
 */
inline QFont& systemFont() {
    static QFont font;
    return font;
}

/// @brief The application font: the system family at the body size.
inline QFont applicationFont() {
    QFont font = systemFont();
    font.setPointSizeF(points(Role::Body));
    return font;
}

/// @brief @p base resized to @p role, keeping its family and style.
inline QFont font(Role role, QFont base = QGuiApplication::font()) {
    base.setPointSizeF(points(role));
    return base;
}

/**
 * @brief Replaces every `$font-<name>` in @p styleSheet with its size.
 *
 * Names that are not in the type scale are left untouched, so Qt drops the
 * declaration, and are reported through @p unresolved when given.
 */
inline QString resolveStyleSheet(const QString& styleSheet,
                                 QStringList*   unresolved = nullptr) {
    static const QRegularExpression tokenRe(
        QStringLiteral("\\$font-([a-z]+(?:-[a-z]+)*)"));

    QString   out;
    qsizetype last = 0;
    auto      it   = tokenRe.globalMatch(styleSheet);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        out += styleSheet.mid(last, match.capturedStart() - last);
        last = match.capturedEnd();

        const QString name  = match.captured(1);
        bool          found = false;
        for (const Token& token : kTokens) {
            if (name == token.name) {
                out += formatPoints(token.points * AppTextScale::factor());
                found = true;
                break;
            }
        }
        if (!found) {
            out += match.captured(0);
            if (unresolved != nullptr) {
                unresolved->append(name);
            }
        }
    }
    out += styleSheet.mid(last);
    return out;
}

/// @brief The application style sheet with its sizes resolved.
inline QString loadStyleSheet() {
    QFile file(QStringLiteral(":/resources/styles/app.qss"));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return {};
    }
    return resolveStyleSheet(QString::fromUtf8(file.readAll()));
}

/**
 * @brief A length that has to hold text, grown to how big text renders here.
 *
 * @p designPx is the length the layout was drawn with, when body text was
 * @ref Role::Body points - that is, as many pixels - high. It grows in step
 * with the body font as it actually renders, which covers both the user's
 * text scale and the platform's point-to-pixel mapping. Use it for fixed or
 * minimum sizes of things that contain text; leave decoration in plain
 * logical pixels.
 */
inline int lengthForText(int designPx) {
    const QFontInfo rendered(QGuiApplication::font());
    const qreal     textPixels = rendered.pixelSize() > 0
                                     ? rendered.pixelSize()
                                     : basePoints(Role::Body);
    return qRound(designPx * textPixels / basePoints(Role::Body));
}

} // namespace AppTypography

#endif // APPTYPOGRAPHY_H
