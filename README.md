[![Travis CI](https://travis-ci.org/fathomssen/redtimer.svg?branch=master)](https://travis-ci.org/fathomssen/redtimer)
[![AppVeyor CI](https://ci.appveyor.com/api/projects/status/github/fathomssen/redtimer)](https://ci.appveyor.com/project/fathomssen/redtimer)

Redmine Time Tracker
====================

RedTimer is an easy-to-use platform-independent time tracker which allows the user to track time while working
on an issue.

Usage
-----

You can start the RedTimer executable from your file manager.

Installation instructions
-------------------------

For Ubuntu instructions, please have a look at the [Ubuntu README](README.Ubuntu.md).

The simplest way is to pick a binary from GitHub (https://github.com/fathomssen/redtimer/releases).

To use Redmine custom fields in RedTimer, please install the `redmine_shared_api` plugin from
https://github.com/anovitsky/redmine_shared_api.

The following instructions can be used to build the application from source.

```
git clone https://github.com/fathomssen/redtimer.git
cd redtimer
git submodule update --init

cd qtredmine
qmake -r
make

cd ..
qmake -r
make
```

This requires for you to have Qt 5.5+ and GCC 4.8.4+ installed and in your path.

Alternatively, you can use a QtCreator distribution from https://www.qt.io. In QtCreator, you can open the
project files `qtredmine/qtredmine.pro` and `RedTimer.pro` and start the build.

Notes on how to build a statically linked version of Qt 5 with MinGW including OpenSSL support (for HTTPS) can
be found on
https://www.thomssen-it.de/blog/how-to-compile-qt-5-statically-with-mingw-including-openssl-on-windows.
