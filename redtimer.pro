TEMPLATE = subdirs

SUBDIRS = \
    cli \
    gui \
    libqtredmine \
    libredtimer

cli.file = cli/redtimercli.pro
gui.file = gui/redtimer.pro
libqtredmine.file = libqtredmine/qtredmine.pro

cli.depends = libqtredmine libredtimer
gui.depends = libredtimer
libredtimer.depends = libqtredmine

DISTFILES += \
    .travis/* \
    docs/* \
    images/* \
    .travis.yml \
    appveyor.yml \
    LICENSE \
    README.md \
    README.Ubuntu.md \
