TEMPLATE = subdirs

SUBDIRS = \
    gui \
    libqtredmine \

gui.file = gui/redtimer.pro
libqtredmine.file = libqtredmine/qtredmine.pro

gui.depends = libqtredmine

DISTFILES += \
    .travis/* \
    images/* \
    .travis.yml \
    appveyor.yml \
    LICENSE \
    README.md \
    README.Ubuntu.md \
