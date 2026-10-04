#pragma once

#include "lltutorwindow.h"

#include <QTest>

struct TutorStep {
    QString input;
    QString expectedState;
    int     expectedRight;
    int     expectedWrong;
};

inline void runScenario(LLTutorWindow& tutor, const QList<TutorStep>& steps) {
    for (qsizetype i = 0; i < steps.size(); ++i) {
        const TutorStep& step = steps.at(i);
        tutor.setAnswerForTest(step.input);
        tutor.submitForTest();

        QCOMPARE(tutor.currentStateForTest(), step.expectedState);
        QCOMPARE(tutor.rightCountForTest(), step.expectedRight);
        QCOMPARE(tutor.wrongCountForTest(), step.expectedWrong);
    }
}
