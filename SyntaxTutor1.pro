QT       += core gui printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    backend/grammar.cpp \
    backend/grammar_factory.cpp \
    backend/ll1_parser.cpp \
    backend/lr0_item.cpp \
    backend/slr1_parser.cpp \
    backend/symbol_table.cpp \
    dialogtablell.cpp \
    lltutorwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    slrtabledialog.cpp \
    slrtutorwindow.cpp

HEADERS += \
    UniqueQueue.h \
    backend/grammar.hpp \
    backend/grammar_factory.hpp \
    backend/ll1_parser.hpp \
    backend/lr0_item.hpp \
    backend/slr1_parser.hpp \
    backend/state.hpp \
    backend/symbol_table.hpp \
    backend/tabulate.hpp \
    dialogtablell.h \
    lltutorwindow.h \
    mainwindow.h \
    slrtabledialog.h \
    slrtutorwindow.h

FORMS += \
    dialogtablell.ui \
    lltutorwindow.ui \
    mainwindow.ui \
    slrtutorwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
