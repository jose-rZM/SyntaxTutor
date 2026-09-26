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

#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QTextEdit>

class QPropertyAnimation;

class CustomTextEdit : public QTextEdit {
    Q_OBJECT
    Q_PROPERTY(int growHeight READ growHeight WRITE setGrowHeight)
  public:
    explicit CustomTextEdit(QWidget* parent = nullptr);

    /// User-visible name of the newline shortcut for the current platform.
    static QString newlineShortcutText();

    /**
     * @brief Makes the height follow the content, animated, between
     * @p minLines and @p maxLines lines of text. Content beyond that scrolls.
     *
     * The bounds are measured from the widget's own font and style, so they
     * stay right at any text size and on any platform.
     */
    void setAutoGrowLines(int minLines, int maxLines);

    /// @brief Height of the box when it holds its minimum number of lines.
    int minimumGrowHeight() const { return minGrowHeight_; }

  signals:
    void sendRequested();

    /// @brief The height for the minimum number of lines has changed.
    void minimumGrowHeightChanged(int height);

  protected:
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

  private:
    int  growHeight() const;
    void setGrowHeight(int height);
    void updateGrowHeight();
    void refreshGrowBounds();
    int  heightForLines(int lines) const;

    /// @brief Cached frame + padding, sampled only when geometry is settled.
    int                 chrome_        = 0;
    int                 minGrowLines_  = 0;
    int                 maxGrowLines_  = 0;
    int                 minGrowHeight_ = 0;
    int                 maxGrowHeight_ = 0;
    QPropertyAnimation* growAnimation_ = nullptr;
};

#endif // CUSTOMTEXTEDIT_H
