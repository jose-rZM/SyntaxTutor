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

#include "slrtabledialog.h"
#include <QApplication>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QStyledItemDelegate>

class CenterAlignDelegate : public QStyledItemDelegate {
  public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void initStyleOption(QStyleOptionViewItem* opt,
                         const QModelIndex&    idx) const override {
        QStyledItemDelegate::initStyleOption(opt, idx);
        opt->displayAlignment = Qt::AlignCenter;
        opt->font             = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    }
};

SLRTableDialog::SLRTableDialog(int rowCount, int colCount,
                               const QStringList& colHeaders, QWidget* parent,
                               QVector<QVector<QString>>* initialData)
    : QDialog(parent) {
    setProperty("tableDialog", true);
    setObjectName("slrTableDialog");
    table = new QTableWidget(rowCount, colCount, this);
    table->setObjectName("slrTableWidget");
    table->horizontalHeader()->setFont(table->font());
    table->verticalHeader()->setFont(table->font());
    table->setHorizontalHeaderLabels(colHeaders);
    table->setItemDelegate(new CenterAlignDelegate(table));
    table->setAlternatingRowColors(true);
    QStringList rowLabels;
    for (int i = 0; i < rowCount; ++i)
        rowLabels << tr("State %1").arg(i);
    table->setVerticalHeaderLabels(rowLabels);

    table->resizeColumnsToContents();
    table->resizeRowsToContents();

    // Wider columns
    for (int i = 0; i < table->columnCount(); ++i) {
        table->setColumnWidth(i, table->columnWidth(i) + 40);
    }

    // Taller rows
    for (int i = 0; i < table->rowCount(); ++i) {
        table->setRowHeight(i, table->rowHeight(i) + 5);
    }

    table->horizontalHeader()->setStretchLastSection(true);

    submitButton = new QPushButton(tr("Finalizar"), this);
    submitButton->setObjectName("slrTableSubmitButton");
    submitButton->setFont(submitButton->font());
    submitButton->setCursor(Qt::PointingHandCursor);
    submitButton->setProperty("role", "primary");

    guidedButton = new QPushButton(tr("Modo guiado"), this);
    guidedButton->setObjectName("slrTableGuidedButton");
    guidedButton->setFont(guidedButton->font());
    guidedButton->setCursor(Qt::PointingHandCursor);
    guidedButton->setProperty("role", "primary");

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(table);

    auto* buttons = new QHBoxLayout;
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setSpacing(10);
    buttons->addStretch();
    buttons->addWidget(guidedButton);
    buttons->addWidget(submitButton);

    layout->addLayout(buttons);
    layout->setContentsMargins(10, 10, 10, 10);
    setLayout(layout);

    setWindowTitle(tr("Completar tabla SLR"));

    int width = table->verticalHeader()->width();

    for (int i = 0; i < table->columnCount(); ++i)
        width += table->columnWidth(i);

    int height = table->horizontalHeader()->height();
    for (int i = 0; i < table->rowCount(); ++i)
        height += table->rowHeight(i);

    width += 60;   // extra
    height += 100; // extra

    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
    width            = qMin(width, screenSize.width() - 100);
    height           = qMin(height, screenSize.height() - 100);

    if (initialData != nullptr) {
        setInitialData(*initialData);
    }

    resize(width, height);

    connect(submitButton, &QPushButton::clicked, this,
            [this]() {
                commitPendingEdit();
                emit submitted(getTableData());
            });
    connect(guidedButton, &QPushButton::clicked, this,
            [this]() {
                commitPendingEdit();
                emit guidedRequested(getTableData());
            });
}

void SLRTableDialog::commitPendingEdit() {
    if (QWidget* editor = QApplication::focusWidget();
        editor != nullptr && table->isAncestorOf(editor)) {
        submitButton->setFocus(Qt::OtherFocusReason);
        QApplication::processEvents();
    }
}

QVector<QVector<QString>> SLRTableDialog::getTableData() const {
    QVector<QVector<QString>> data;
    for (int i = 0; i < table->rowCount(); ++i) {
        QVector<QString> row;
        for (int j = 0; j < table->columnCount(); ++j) {
            QTableWidgetItem* item = table->item(i, j);
            row.append(item ? item->text() : "");
        }
        data.append(row);
    }
    return data;
}

void SLRTableDialog::setInitialData(const QVector<QVector<QString>>& data) {
    const int rows = qMin(data.size(), table->rowCount());
    const int cols =
        (rows > 0) ? qMin(data[0].size(), table->columnCount()) : 0;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QTableWidgetItem* item = table->item(i, j);

            if (!item) {
                item = new QTableWidgetItem();
                item->setTextAlignment(Qt::AlignCenter);
                table->setItem(i, j, item);
            }

            item->setText(data[i][j]);
        }
    }
}

void SLRTableDialog::setGuidedModeActive(bool active) {
    guidedButton->setEnabled(!active);
    submitButton->setEnabled(!active);
}

void SLRTableDialog::highlightIncorrectCells(
    const QList<QPair<int, int>>& coords) {
    for (int r = 0; r < table->rowCount(); ++r) {
        for (int c = 0; c < table->columnCount(); ++c) {
            QTableWidgetItem* item = table->item(r, c);
            if (!item) {
                continue;
            }
            item->setBackground(Qt::NoBrush);
            item->setForeground(QBrush());
        }
    }

    const QColor err("#D9534F");
    for (auto [r, c] : coords) {
        QTableWidgetItem* item = table->item(r, c);
        if (!item) {
            item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            table->setItem(r, c, item);
        }
        item->setBackground(err);
        item->setForeground(Qt::white);
    }
}

void SLRTableDialog::highlightInvalidCells(
    const QList<QPair<int, int>>& coords) {
    const QColor warn("#F0AD4E");
    for (auto [r, c] : coords) {
        QTableWidgetItem* item = table->item(r, c);
        if (!item) {
            item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            table->setItem(r, c, item);
        }
        item->setBackground(warn);
        item->setForeground(Qt::black);
    }
}
