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
    customtextedit.cpp \
    lltabledialog.cpp \
    lltutorwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    slrtabledialog.cpp \
    slrtutorwindow.cpp \
    tutorialmanager.cpp

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
    customtextedit.h \
    lltabledialog.h \
    lltutorwindow.h \
    mainwindow.h \
    slrtabledialog.h \
    slrtutorwindow.h \
    slrwizard.h \
    slrwizardpage.h \
    tutorialmanager.h

FORMS += \
    lltutorwindow.ui \
    mainwindow.ui \
    slrtutorwindow.ui

win32:CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS_RELEASE += -O3 -flto -DNDEBUG -pipe
    QMAKE_LFLAGS_RELEASE   += -s -flto
}

unix:!mac:CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS_RELEASE += \
        -O3 \
        -flto \
        -DNDEBUG \
        -fvisibility=hidden \
        -fvisibility-inlines-hidden \
        -pipe

    QMAKE_LFLAGS_RELEASE += \
        -flto \
        -s \
        -Wl,--as-needed

    # Tras link: strippear de nuevo por si quedan símbolos
    QMAKE_POST_LINK += $$QMAKE_STRIP $$DESTDIR/$${TARGET}
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
