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
    IssueCreator.h \
    IssueSelector.h \
    Models.h \
    MainWindow.h \
    ProfileData.h \
    Settings.h \
    Window.h

SOURCES += \
    main.cpp \
    Models.cpp \
    IssueSelector.cpp \
    IssueCreator.cpp \
    MainWindow.cpp \
    ProfileData.cpp \
    Settings.cpp \
    Window.cpp

RESOURCES += RedTimerClient.qrc

# External projects
include($$PWD/qxtglobalshortcut5/qxt.pri)
include($$PWD/../libqtredmine/qtredmine.pri)

# Default rules for deployment.
include($$PWD/RedTimerClient.pri)
