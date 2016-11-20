TEMPLATE = subdirs

SUBDIRS = \
    cli \
    gui \
    libqtredmine

cli.file = cli/redtimercli.pro
gui.file = gui/redtimer.pro
libqtredmine.file = libqtredmine/qtredmine.pro

cli.depends = libqtredmine
gui.depends = libqtredmine

DISTFILES += \
    .travis/* \
    docs/* \
    images/* \
    .travis.yml \
    appveyor.yml \
    LICENSE \
    README.md \
    README.Ubuntu.md \
