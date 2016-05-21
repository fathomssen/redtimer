TEMPLATE = app

QT += gui qml quick svg widgets
QT += gui-private # Required for global shortcuts on some Linux distributions
QMAKE_CXXFLAGS += -std=c++11

RC_ICONS = icons/clock_red.ico
ICON = icons/clock_red.icns

# Global shortcuts
include(qxtglobalshortcut5/qxt.pri)

VERSION = 1.0.90.2
QMAKE_TARGET_COMPANY = "Thomssen IT"
QMAKE_TARGET_PRODUCT = "RedTimer"
QMAKE_TARGET_DESCRIPTION = "Redmine Time Tracker"
QMAKE_TARGET_COPYRIGHT = "LGPL 3"

HEADERS += \
    Settings.h \
    logging.h \
    Models.h \
    IssueSelector.h \
    IssueCreator.h \
    Window.h \
    MainWindow.h

SOURCES += main.cpp \
    Settings.cpp \
    Models.cpp \
    IssueSelector.cpp \
    IssueCreator.cpp \
    Window.cpp \
    MainWindow.cpp

DISTFILES += \
    .travis.yml \
    README.md \
    README.Ubuntu.md \
    build/deploy.ubuntu.sh

RESOURCES += qml.qrc

# No debug statements in release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG(debug, debug|release):DEFINES += DEBUG_OUTPUT

win32:CONFIG(release, debug|release): LIBS += -static -L$$PWD/qtredmine/Release/release/ -lqtredmine
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/qtredmine/Debug/debug/ -lqtredmine
else:unix: LIBS += -L$$PWD/qtredmine/ -lqtredmine

INCLUDEPATH += $$PWD/qtredmine
DEPENDPATH += $$PWD/qtredmine

# Default rules for deployment.
include(deployment.pri)
