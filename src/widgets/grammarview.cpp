#include "grammarview.h"

#include <QFontDatabase>
#include <QGridLayout>
#include <QLabel>
#include <QSizePolicy>

namespace {

QFont grammarFont() {
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(15);
    return font;
}

QLabel* makeCell(const QString& text, const QFont& font, QWidget* parent,
                 const QString& color = QString()) {
    auto* label = new QLabel(text, parent);
    label->setFont(font);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    if (!color.isEmpty()) {
        label->setStyleSheet(QString("color: %1;").arg(color));
    }
    return label;
}

}  // namespace

GrammarView::GrammarView(QWidget* parent) : QFrame(parent), gridLayout(new QGridLayout(this)) {
    setObjectName("grammarView");
    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    gridLayout->setContentsMargins(18, 18, 18, 18);
    gridLayout->setHorizontalSpacing(14);
    gridLayout->setVerticalSpacing(4);
    gridLayout->setColumnStretch(3, 1);
    gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
}

void GrammarView::clearRows() {
    while (QLayoutItem* item = gridLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void GrammarView::setRows(const QVector<Row>& rows) {
    currentRows = rows;
    clearRows();

    const QFont monoFont = grammarFont();

    for (int i = 0; i < currentRows.size(); ++i) {
        const Row& row = currentRows.at(i);

        auto* indexLabel  = makeCell(row.index, monoFont, this, "#F1F1F1");
        auto* lhsLabel    = makeCell(row.lhs, monoFont, this, "#F1F1F1");
        auto* markerLabel = makeCell(row.marker, monoFont, this, "#F1F1F1");
        auto* rhsLabel    = makeCell(row.rhs, monoFont, this, "#F1F1F1");

        gridLayout->addWidget(indexLabel, i, 0, Qt::AlignLeft | Qt::AlignTop);
        gridLayout->addWidget(lhsLabel, i, 1, Qt::AlignLeft | Qt::AlignTop);
        gridLayout->addWidget(markerLabel, i, 2, Qt::AlignLeft | Qt::AlignTop);
        gridLayout->addWidget(rhsLabel, i, 3, Qt::AlignLeft | Qt::AlignTop);
    }

    gridLayout->setRowStretch(currentRows.size(), 1);
    adjustSize();
}
