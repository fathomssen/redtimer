QT       += network
QT       -= gui

TARGET = redtimer
TEMPLATE = lib

DEFINES += LIBREDTIMER_LIBRARY

SOURCES += RedTimerConnector.cpp

HEADERS += RedTimerConnector.h\
        libredtimer_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    libredtimer.pri

include($$PWD/libredtimer.pri)
