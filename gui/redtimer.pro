TEMPLATE = app

CONFIG += c++14

QT += gui qml quick svg widgets

unix:!macx {
    # Required for global shortcuts on X11
    QT += gui-private
}

RC_ICONS = icons/clock_red.ico
ICON = icons/clock_red.icns

QMAKE_INFO_PLIST = Info.plist

win32:VERSION = 1.0.0.0

QMAKE_TARGET_COMPANY = "Thomssen IT"
QMAKE_TARGET_PRODUCT = "RedTimer"
QMAKE_TARGET_DESCRIPTION = "Redmine Time Tracker"
QMAKE_TARGET_COPYRIGHT = "© Thomssen IT"

HEADERS += \
    IssueCreator.h \
    IssueSelector.h \
    Models.h \
    MainWindow.h \
    ProfileSelector.h \
    Settings.h \
    Window.h

SOURCES += \
    main.cpp \
    Models.cpp \
    IssueSelector.cpp \
    IssueCreator.cpp \
    MainWindow.cpp \
    ProfileSelector.cpp \
    Settings.cpp \
    Window.cpp

RESOURCES += redtimer.qrc

DISTFILES += Info.plist

# External projects
include($$PWD/qxtglobalshortcut5/qxt.pri)
include($$PWD/../libqtredmine/qtredmine.pri)
include($$PWD/../libredtimer/libredtimer.pri)

# Default rules for deployment.
include($$PWD/redtimer.pri)
