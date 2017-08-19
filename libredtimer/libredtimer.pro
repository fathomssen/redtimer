QT       -= gui

TARGET = redtimer
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++14

HEADERS += include/redtimer/CliOptions.h

SOURCES += CliOptions.cpp

DISTFILES += \
    libredtimer.pri \

unix {
    target.path = /usr/lib
    INSTALLS += target
    CONFIG += staticlib
}

include($$PWD/libredtimer.pri)

include($$PWD/../libqtredmine/qtredmine.pri)
