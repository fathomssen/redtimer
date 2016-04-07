TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

QT += qml quick widgets gui svg

SOURCES += main.cpp \
    RedTimer.cpp \
    Settings.cpp \
    Models.cpp \
    IssueSelector.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# No debug statements in release
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG(debug, debug|release):DEFINES += DEBUG_OUTPUT

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    RedTimer.h \
    Settings.h \
    logging.h \
    Models.h \
    IssueSelector.h

CONFIG += c++11

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/qtredmine/release/ -lqtredmine
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/qtredmine/debug/ -lqtredmine
else:unix: LIBS += -L$$PWD/qtredmine/ -lqtredmine

INCLUDEPATH += $$PWD/qtredmine
DEPENDPATH += $$PWD/qtredmine

DISTFILES += \
    .travis.yml \
    README.md \
