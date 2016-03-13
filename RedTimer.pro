TEMPLATE = app

QT += qml quick widgets gui

SOURCES += main.cpp \
    RedTimer.cpp \
    Settings.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    RedTimer.h \
    Settings.h \
    logging.h

CONFIG += c++11

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/build-qtredmine-Desktop_Qt_5_5_1_MinGW_32bit-Debug/release/ -lqtredmine
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/build-qtredmine-Desktop_Qt_5_5_1_MinGW_32bit-Debug/debug/ -lqtredmine
else:unix: LIBS += -L$$PWD/build-qtredmine-Desktop_Qt_5_5_1_MinGW_32bit-Debug/ -lqtredmine

INCLUDEPATH += $$PWD/build-qtredmine-Desktop_Qt_5_5_1_MinGW_32bit-Debug/debug
DEPENDPATH += $$PWD/build-qtredmine-Desktop_Qt_5_5_1_MinGW_32bit-Debug/debug
