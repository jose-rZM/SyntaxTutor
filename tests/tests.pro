QT += core gui widgets testlib printsupport svg

CONFIG += c++20 testcase console
CONFIG -= app_bundle

TARGET = tutor_tests
DEFINES += SYNTAXTUTOR_TESTING

INCLUDEPATH += \
    $$PWD/.. \
    $$PWD/../src \
    $$PWD/../src/app \
    $$PWD/../src/gui \
    $$PWD/../src/widgets \
    $$PWD/../src/backend \
    $$PWD/helpers \
    $$PWD/fixtures/grammars

OBJECTS_DIR = $$OUT_PWD/.objects
MOC_DIR = $$OUT_PWD/.moc
RCC_DIR = $$OUT_PWD/.rcc
UI_DIR = $$OUT_PWD/.ui
DESTDIR = $$OUT_PWD/.bin

SOURCES += \
    ../src/backend/grammar.cpp \
    ../src/backend/grammar_factory.cpp \
    ../src/backend/ll1_parser.cpp \
    ../src/backend/lr0_item.cpp \
    ../src/backend/slr1_parser.cpp \
    ../src/backend/symbol_table.cpp \
    ../src/widgets/customtextedit.cpp \
    ../src/widgets/grammarview.cpp \
    ../src/widgets/tutorialmanager.cpp \
    ../src/gui/lltabledialog.cpp \
    ../src/gui/lltutorwindow.cpp \
    ../src/gui/mainwindow.cpp \
    ../src/gui/slrtabledialog.cpp \
    ../src/gui/slrtutorwindow.cpp \
    fixtures/grammars/tutor_grammar_fixtures.cpp \
    tutor/ll_tutor_window_test.cpp \
    tutor/main_window_test.cpp \
    tutor/slr_tutor_window_test.cpp \
    tutor/tutor_test_main.cpp

HEADERS += \
    ../src/backend/grammar.hpp \
    ../src/backend/grammar_factory.hpp \
    ../src/backend/ll1_parser.hpp \
    ../src/backend/lr0_item.hpp \
    ../src/backend/slr1_parser.hpp \
    ../src/backend/state.hpp \
    ../src/backend/symbol_table.hpp \
    ../src/widgets/customtextedit.h \
    ../src/widgets/grammarview.h \
    ../src/widgets/tutorialmanager.h \
    ../src/gui/lltabledialog.h \
    ../src/gui/lltutorwindow.h \
    ../src/gui/mainwindow.h \
    ../src/gui/slrtabledialog.h \
    ../src/gui/slrtutorwindow.h \
    ../src/gui/slrwizard.h \
    ../src/gui/slrwizardpage.h \
    fixtures/grammars/tutor_grammar_fixtures.h \
    helpers/ll1_tutor_test_utils.h \
    helpers/qt_modal_test_utils.h \
    helpers/slr_tutor_test_utils.h \
    helpers/tutor_scenario.h \
    tutor/tutor_window_test.h

FORMS += \
    ../src/gui/lltutorwindow.ui \
    ../src/gui/mainwindow.ui \
    ../src/gui/slrtutorwindow.ui

RESOURCES += \
    ../resources.qrc
