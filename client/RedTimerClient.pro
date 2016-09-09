TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

QT += gui qml quick svg widgets
QT += gui-private # Required for global shortcuts on some Linux distributions

RC_ICONS = icons/clock_red.ico
ICON = icons/clock_red.icns

QMAKE_TARGET_COMPANY = "Thomssen IT"
QMAKE_TARGET_PRODUCT = "RedTimerClient"
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

SOURCES += \
    main.cpp \
    Settings.cpp \
    Models.cpp \
    IssueSelector.cpp \
    IssueCreator.cpp \
    Window.cpp \
    MainWindow.cpp

DISTFILES += \
    .travis.yml \
    appveyor.yml \
    README.md \
    README.Ubuntu.md \
    build/deploy.ubuntu.sh \
    build/redtimer.sh

RESOURCES += RedTimerClient.qrc

# No debug statements in release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG(debug, debug|release):DEFINES += DEBUG_OUTPUT

# External projects
include($$PWD/qxtglobalshortcut5/qxt.pri)
include($$PWD/../libqtredmine/qtredmine.pri)
include($$PWD/../libredtimer/libredtimer.pri)

# Default rules for deployment.
include($$PWD/RedTimerClient.pri)