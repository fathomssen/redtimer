INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD

!equals(_PRO_FILE_PWD_, $$PWD) {
  win32:CONFIG(release, debug|release): LIBS += -L$$shadowed($$PWD)/release -lredtimer
  else:win32:CONFIG(debug, debug|release): LIBS += -L$$shadowed($$PWD)/debug -lredtimer
  else:unix: LIBS += -L$$shadowed($$PWD) -lredtimer
}
