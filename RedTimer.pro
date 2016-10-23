TEMPLATE = subdirs

SUBDIRS = \
    client \
    libqtredmine \

client.file = client/RedTimerClient.pro
libqtredmine.file = libqtredmine/qtredmine.pro

client.depends = libqtredmine

DISTFILES += \
    .travis/* \
    images/* \
    .travis.yml \
    appveyor.yml \
    LICENSE \
    README.md \
    README.Ubuntu.md \
