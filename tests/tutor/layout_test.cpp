#include "tutor_window_test.h"

#include "applayout.h"
#include "apptypography.h"
#include "examreportdialog.h"
#include "examsession.h"
#include "grammareditordialog.h"
#include "layout_audit.h"
#include "lltutorwindow.h"
#include "mainwindow.h"
#include "production_typography.h"
#include "slrtutorwindow.h"
#include "tutor_grammar_fixtures.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QDialog>
#include <QImage>
#include <QListWidget>
#include <QScrollBar>
#include <QSettings>
#include <QTest>
#include <QTimer>

namespace {

// Text sizes the checks run at: the smallest the user can pick, the design
// size, and two large ones where wrapping changes the most.
constexpr int kAuditedPercents[] = {80, 100, 150, 200};

void clearLayoutTestSettings() {
    QSettings settings("UMA-Test", "SyntaxTutor-Test");
    settings.clear();
    settings.sync();
}

// Layout requests are posted events; a few short turns of the event loop
// deliver them and whatever they trigger in turn.
void settle() {
    for (int i = 0; i < 3; ++i) {
        QCoreApplication::processEvents();
        QTest::qWait(5);
    }
}

// Wrapped text only breaks at the width where it gains a line, so a single
// size proves little. Checks a spread of widths from the widget's minimum.
QStringList auditAcrossWidths(QWidget* widget) {
    QStringList problems;
    const int   from   = qMax(widget->minimumWidth(), 320);
    const int   height = qMax(widget->minimumHeight(), widget->height());
    for (int width = from; width <= from + 480; width += 40) {
        widget->resize(width, height);
        settle();
        for (const QString& problem :
             LayoutAudit::findClippedWidgets(widget)) {
            problems << QStringLiteral("width %1: %2").arg(width).arg(problem);
        }
    }
    return problems;
}

// Opens a modal through @p open and audits it while it runs.
QStringList auditModal(const std::function<void()>& open) {
    QStringList problems;
    bool        seen = false;
    QTimer::singleShot(100, [&problems, &seen]() {
        QWidget* modal = QApplication::activeModalWidget();
        if (modal == nullptr) {
            return;
        }
        seen     = true;
        problems = auditAcrossWidths(modal);
        if (auto* dialog = qobject_cast<QDialog*>(modal)) {
            dialog->reject();
        }
    });
    open();
    settle();
    if (!seen) {
        problems << QStringLiteral("the dialog never opened");
    }
    return problems;
}

QString report(int percent, const QString& what, const QStringList& problems) {
    return QStringLiteral("%1 at %2%:\n  ").arg(what).arg(percent) +
           problems.join(QStringLiteral("\n  "));
}

// The size the main window gives a tutor page, before any clamping to the
// screen: the test screen is smaller than a real one.
QSize tutorPageSize() {
    return {qMax(800, AppTypography::lengthForText(800)),
            qMax(600, AppTypography::lengthForText(600))};
}

} // namespace

// -----------------------------------------------------------------------------
// Test: layoutHomeShowsAllTextAtEveryTextSize
// Expected:
//   At every text size and window width the home shows its whole text: no
//   wrapped title given one line too few, no button or option squeezed.
// -----------------------------------------------------------------------------
void TutorWindowTest::layoutHomeShowsAllTextAtEveryTextSize() {
    for (const int percent : kAuditedPercents) {
        clearLayoutTestSettings();
        ProductionTypography typography(percent);
        MainWindow           window;
        window.show();
        settle();

        const QStringList problems = auditAcrossWidths(&window);
        QVERIFY2(problems.isEmpty(),
                 qPrintable(report(percent, "home", problems)));
    }
}

// -----------------------------------------------------------------------------
// Test: layoutDialogsShowAllTextAtEveryTextSize
// Expected:
//   The hand-built dialogs - language, text size, about, grammar editor and
//   exam report - grow to fit their wrapped text at every size and width.
// -----------------------------------------------------------------------------
void TutorWindowTest::layoutDialogsShowAllTextAtEveryTextSize() {
    for (const int percent : kAuditedPercents) {
        clearLayoutTestSettings();
        ProductionTypography typography(percent);
        MainWindow           window;
        window.show();
        settle();

        QStringList problems = auditModal([&window]() {
            QMetaObject::invokeMethod(&window, "on_idiom_clicked");
        });
        QVERIFY2(problems.isEmpty(),
                 qPrintable(report(percent, "language dialog", problems)));

        problems = auditModal([&window]() {
            window.findChild<QAction*>("actionTextSizeCustom")->trigger();
        });
        QVERIFY2(problems.isEmpty(),
                 qPrintable(report(percent, "text size dialog", problems)));

        problems = auditModal([&window]() {
            QMetaObject::invokeMethod(&window,
                                      "on_actionSobre_la_aplicaci_n_triggered");
        });
        QVERIFY2(problems.isEmpty(),
                 qPrintable(report(percent, "about dialog", problems)));

        for (const auto mode : {GrammarEditorDialog::Mode::LL1,
                                GrammarEditorDialog::Mode::SLR1}) {
            GrammarEditorDialog editor(mode);
            editor.show();
            settle();
            problems = auditAcrossWidths(&editor);
            QVERIFY2(problems.isEmpty(),
                     qPrintable(report(percent, "grammar editor", problems)));
        }

        ExamSession session;
        session.record(QStringLiteral("¿Cuántas filas tiene la tabla LL(1)?"),
                       QStringLiteral("3"), QStringLiteral("4"), false);
        session.record(QStringLiteral("CAB(A)"), QStringLiteral("{a}"),
                       QStringLiteral("{a}"), true);
        ExamReportDialog examReport(session, QStringLiteral("Examen LL(1)"));
        examReport.show();
        settle();
        problems = auditAcrossWidths(&examReport);
        QVERIFY2(problems.isEmpty(),
                 qPrintable(report(percent, "exam report", problems)));
    }
}

// -----------------------------------------------------------------------------
// Test: layoutTutorsShowAllTextAtEveryTextSize
// Expected:
//   Both tutors, at the size the main window gives them and wider, keep the
//   chat, the grammar panel and the side panel readable: nothing squeezed.
// -----------------------------------------------------------------------------
void TutorWindowTest::layoutTutorsShowAllTextAtEveryTextSize() {
    for (const int percent : kAuditedPercents) {
        ProductionTypography typography(percent);
        const QSize          page = tutorPageSize();

        LLTutorWindow ll(TutorGrammarFixtures::makeLl1BranchingGrammar(),
                         nullptr);
        ll.setMinimumSize(page);
        ll.show();
        settle();
        QStringList problems = auditAcrossWidths(&ll);
        QVERIFY2(problems.isEmpty(),
                 qPrintable(report(percent, "LL(1) tutor", problems)));

        SLRTutorWindow slr(TutorGrammarFixtures::makeSlrSimpleGrammar(),
                           nullptr);
        slr.setMinimumSize(page);
        slr.show();
        settle();
        problems = auditAcrossWidths(&slr);
        QVERIFY2(problems.isEmpty(),
                 qPrintable(report(percent, "SLR(1) tutor", problems)));
    }
}

// -----------------------------------------------------------------------------
// Test: layoutScrollBarsFollowTheDarkTheme
// Expected:
//   A scroll bar that appears in the chat is drawn by app.qss: dark track,
//   no arrow buttons. The test platform has a light palette, so a bar left
//   to the platform style comes out light, as it did on Linux.
// -----------------------------------------------------------------------------
void TutorWindowTest::layoutScrollBarsFollowTheDarkTheme() {
    ProductionTypography typography(150);

    LLTutorWindow tutor(TutorGrammarFixtures::makeLl1BranchingGrammar(),
                        nullptr);
    tutor.resize(tutorPageSize().width(), 380);
    tutor.show();
    settle();

    auto* chat = tutor.findChild<QListWidget*>("listWidget");
    QVERIFY(chat != nullptr);
    QScrollBar* bar = chat->verticalScrollBar();
    QVERIFY2(bar->isVisible(), "the chat was expected to overflow");

    // 10px wide as styled; the platform bar is wider and has arrows.
    QCOMPARE(bar->width(), 10);

    // Wherever the handle sits, no part of the bar may be light: the
    // platform groove this replaced was #e6e6e6.
    // Grabbed through the window, as it is seen: a grab of the bar alone
    // fills its transparent margins with the palette instead of what is
    // really behind them.
    const QImage image = tutor.grab().toImage();
    const QRect  area(bar->mapTo(&tutor, QPoint(0, 0)), bar->size());
    const qreal  ratio = image.devicePixelRatio();
    for (int y = area.top(); y <= area.bottom(); ++y) {
        const QColor pixel = image.pixelColor(
            qRound(area.center().x() * ratio), qRound(y * ratio));
        QVERIFY2(pixel.lightnessF() < 0.5,
                 qPrintable(QStringLiteral("light pixel %1 at y=%2")
                                .arg(pixel.name())
                                .arg(y)));
    }
}
