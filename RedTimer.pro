TEMPLATE = subdirs

SUBDIRS = \
    client \
    libqtredmine \
    libredtimer \

client.file = client/RedTimerClient.pro
libqtredmine.file = libqtredmine/qtredmine.pro

client.depends = libqtredmine libredtimer

DISTFILES += \
    build/deploy.osx.sh \
    build/deploy.ubuntu.sh \
    build/fixQtForXcode8.sh \
    build/redtimer.sh \
    .travis.yml \
    appveyor.yml \
    LICENSE \
    README.md \
    README.Ubuntu.md \
