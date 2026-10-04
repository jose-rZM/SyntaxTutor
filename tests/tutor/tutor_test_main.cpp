#include "tutor_window_test.h"

#include <QApplication>
#include <QTest>

int main(int argc, char* argv[]) {
    QApplication   app(argc, argv);
    TutorWindowTest test;
    return QTest::qExec(&test, argc, argv);
}
