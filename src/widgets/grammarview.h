#pragma once

#include <QFrame>
#include <QString>
#include <QVector>

class QGridLayout;

class GrammarView : public QFrame {
    Q_OBJECT
  public:
    struct Row {
        QString index;
        QString lhs;
        QString marker;
        QString rhs;
    };

    explicit GrammarView(QWidget* parent = nullptr);

    void setRows(const QVector<Row>& rows);

    /**
     * @brief Width the card needs for its content.
     *
     * Taken from the grid's minimum size rather than sizeHint(): the scroll
     * area that holds the card gives it a fixed width, and reading a hint
     * that depends on the current width feeds back into itself, shrinking
     * the panel a little more on every text size change.
     */
    int naturalWidth() const;

    /// @brief Rebuilds the rows so a new text size takes effect.
    void refresh();

  private:
    void clearRows();

    QGridLayout*   gridLayout;
    QVector<Row>   currentRows;
};
