TEMPLATE = app

QT += qml quick widgets gui

SOURCES += main.cpp \
    RedTimer.cpp \
    Settings.cpp \
    Models.cpp

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
    Models.h

CONFIG += c++11

CONFIG(release, debug|release): LIBS += -L$$PWD/qtredmine/release/ -lqtredmine
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/qtredmine/debug/ -lqtredmine

INCLUDEPATH += $$PWD/qtredmine/debug
DEPENDPATH += $$PWD/qtredmine/debug

DISTFILES += \
    .travis.yml \
    README.md \
