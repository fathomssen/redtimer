TEMPLATE = app

QT += gui qml quick svg widgets
QMAKE_CXXFLAGS += -std=c++11

RC_ICONS = icons/clock_red.ico
ICON = icons/clock_red.icns

VERSION = 0.0.9
QMAKE_TARGET_COMPANY = "Thomssen IT"
QMAKE_TARGET_PRODUCT = "RedTimer"
QMAKE_TARGET_DESCRIPTION = "Redmine Time Tracker"
QMAKE_TARGET_COPYRIGHT = "LGPL 3"

HEADERS += \
    RedTimer.h \
    Settings.h \
    logging.h \
    Models.h \
    IssueSelector.h \
    IssueCreator.h \
    Window.h

SOURCES += main.cpp \
    RedTimer.cpp \
    Settings.cpp \
    Models.cpp \
    IssueSelector.cpp \
    IssueCreator.cpp \
    Window.cpp

DISTFILES += \
    .travis.yml \
    README.md \
    README.Ubuntu.md \
    build/deploy.ubuntu.sh

RESOURCES += qml.qrc

# No debug statements in release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG(debug, debug|release):DEFINES += DEBUG_OUTPUT

win32:CONFIG(release, debug|release): LIBS += -static -L$$PWD/qtredmine/release/ -lqtredmine
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/qtredmine/debug/ -lqtredmine
else:unix: LIBS += -L$$PWD/qtredmine/ -lqtredmine

INCLUDEPATH += $$PWD/qtredmine
DEPENDPATH += $$PWD/qtredmine

# Default rules for deployment.
include(deployment.pri)
