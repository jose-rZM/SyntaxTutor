#pragma once

#include "grammar.hpp"
#include "ll1_parser.hpp"
#include "lltabledialog.h"
#include "slrtabledialog.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QTableWidget>
#include <QTest>
#include <QTimer>

#include <functional>
#include <memory>

namespace QtModalTestUtils {

inline void scheduleUntilHandled(const std::function<bool()>& handler) {
    auto retry = std::make_shared<std::function<void()>>();
    *retry = [handler, retry]() {
        if (handler()) {
            return;
        }
        QTimer::singleShot(10, [retry]() { (*retry)(); });
    };

    QTimer::singleShot(0, [retry]() { (*retry)(); });
}

template <typename T> T* findVisibleTopLevelWidget() {
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        if (auto* candidate = qobject_cast<T*>(widget);
            candidate != nullptr && candidate->isVisible()) {
            return candidate;
        }
    }
    return nullptr;
}

template <typename T> T* waitForVisibleTopLevelWidget(int timeoutMs = 2000) {
    const int intervalMs = 20;
    int       elapsed    = 0;

    while (elapsed <= timeoutMs) {
        if (T* candidate = findVisibleTopLevelWidget<T>()) {
            return candidate;
        }
        QTest::qWait(intervalMs);
        elapsed += intervalMs;
    }

    return nullptr;
}

inline QPushButton* findButtonByText(QWidget* parent, const QString& text) {
    if (parent == nullptr) {
        return nullptr;
    }

    for (QPushButton* button : parent->findChildren<QPushButton*>()) {
        if (button->text() == text) {
            return button;
        }
    }

    return nullptr;
}

inline void scheduleMessageBoxResponse(QMessageBox::StandardButton button) {
    scheduleUntilHandled([button]() {
        auto* messageBox = findVisibleTopLevelWidget<QMessageBox>();
        if (messageBox == nullptr) {
            return false;
        }

        QPushButton* target =
            qobject_cast<QPushButton*>(messageBox->button(button));
        if (target == nullptr) {
            target = findButtonByText(messageBox, button == QMessageBox::Yes
                                                      ? QStringLiteral("Si")
                                                      : QStringLiteral("No"));
        }
        if (target == nullptr) {
            target = findButtonByText(messageBox, QStringLiteral("OK"));
        }
        if (target == nullptr) {
            target = findButtonByText(messageBox, QStringLiteral("Aceptar"));
        }
        if (target == nullptr) {
            return false;
        }

        QTest::mouseClick(target, Qt::LeftButton);
        return true;
    });
}

inline void scheduleFileDialogSelection(const QString& filePath) {
    scheduleUntilHandled([filePath]() {
        auto* dialog = findVisibleTopLevelWidget<QFileDialog>();
        if (dialog == nullptr) {
            return false;
        }

        dialog->selectFile(filePath);
        QMetaObject::invokeMethod(dialog, "accept", Qt::DirectConnection);
        return true;
    });
}

inline QStringList horizontalHeaders(QTableWidget* table) {
    QStringList headers;
    for (int col = 0; col < table->columnCount(); ++col) {
        headers.append(table->horizontalHeaderItem(col)->text());
    }
    return headers;
}

inline QStringList verticalHeaders(QTableWidget* table) {
    QStringList headers;
    for (int row = 0; row < table->rowCount(); ++row) {
        headers.append(table->verticalHeaderItem(row)->text());
    }
    return headers;
}

inline QVector<QVector<QString>> buildExpectedTable(const Grammar& grammar,
                                                    QTableWidget*  table) {
    LL1Parser parser(grammar);
    parser.CreateLL1Table();

    const QStringList rows = verticalHeaders(table);
    const QStringList cols = horizontalHeaders(table);

    QVector<QVector<QString>> raw(rows.size(), QVector<QString>(cols.size()));
    for (int row = 0; row < rows.size(); ++row) {
        for (int col = 0; col < cols.size(); ++col) {
            const auto rowIt = parser.ll1_t_.find(rows.at(row).toStdString());
            if (rowIt == parser.ll1_t_.end()) {
                continue;
            }

            const auto colIt = rowIt->second.find(cols.at(col).toStdString());
            if (colIt == rowIt->second.end() || colIt->second.empty()) {
                continue;
            }

            QStringList production;
            for (const std::string& symbol : colIt->second.front()) {
                production.append(QString::fromStdString(symbol));
            }
            raw[row][col] = production.join(' ');
        }
    }

    return raw;
}

inline QVector<QVector<QString>> buildWrongTable(const Grammar& grammar,
                                                 QTableWidget*  table) {
    QVector<QVector<QString>> raw = buildExpectedTable(grammar, table);

    for (int row = 0; row < raw.size(); ++row) {
        for (int col = 0; col < raw[row].size(); ++col) {
            if (!raw[row][col].isEmpty()) {
                raw[row][col] = QStringLiteral("WRONG");
                return raw;
            }
        }
    }

    if (!raw.isEmpty() && !raw.first().isEmpty()) {
        raw[0][0] = QStringLiteral("WRONG");
    }
    return raw;
}

inline void setTableData(QTableWidget* table, const QVector<QVector<QString>>& raw) {
    for (int row = 0; row < table->rowCount(); ++row) {
        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem* item = table->item(row, col);
            if (item == nullptr) {
                item = new QTableWidgetItem();
                table->setItem(row, col, item);
            }
            item->setText(raw[row][col]);
        }
    }
}

inline QColor cellBackground(QTableWidget* table, int row, int col) {
    QTableWidgetItem* item = table->item(row, col);
    return item == nullptr ? QColor() : item->background().color();
}

inline int firstNonEmptyCellRow(const QVector<QVector<QString>>& raw) {
    for (int row = 0; row < raw.size(); ++row) {
        for (int col = 0; col < raw[row].size(); ++col) {
            if (!raw[row][col].isEmpty()) {
                return row;
            }
        }
    }
    return -1;
}

inline int firstNonEmptyCellCol(const QVector<QVector<QString>>& raw) {
    for (int row = 0; row < raw.size(); ++row) {
        for (int col = 0; col < raw[row].size(); ++col) {
            if (!raw[row][col].isEmpty()) {
                return col;
            }
        }
    }
    return -1;
}

inline void submitLlTableDialog(LLTableDialog*                dialog,
                                const QVector<QVector<QString>>& raw) {
    auto* table = dialog->findChild<QTableWidget*>("llTableWidget");
    auto* button = dialog->findChild<QPushButton*>("llTableSubmitButton");
    QVERIFY(table != nullptr);
    QVERIFY(button != nullptr);

    setTableData(table, raw);
    QTest::mouseClick(button, Qt::LeftButton);
}

inline void submitSlrTableDialog(SLRTableDialog*               dialog,
                                 const QVector<QVector<QString>>& raw) {
    auto* table = dialog->findChild<QTableWidget*>("slrTableWidget");
    auto* button = dialog->findChild<QPushButton*>("slrTableSubmitButton");
    QVERIFY(table != nullptr);
    QVERIFY(button != nullptr);

    setTableData(table, raw);
    QTest::mouseClick(button, Qt::LeftButton);
}

inline void requestSlrGuidedMode(SLRTableDialog*               dialog,
                                 const QVector<QVector<QString>>& raw) {
    auto* table = dialog->findChild<QTableWidget*>("slrTableWidget");
    auto* button = dialog->findChild<QPushButton*>("slrTableGuidedButton");
    QVERIFY(table != nullptr);
    QVERIFY(button != nullptr);

    setTableData(table, raw);
    QTest::mouseClick(button, Qt::LeftButton);
}

} // namespace QtModalTestUtils
