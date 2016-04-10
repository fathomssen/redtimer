[![Travis](https://travis-ci.org/fathomssen/redtimer.svg?branch=master)](https://travis-ci.org/fathomssen/redtimer)

Redmine Time Tracker
====================

RedTimer is an easy-to-use platform-independent time tracker which allows the user to track time while working
on an issue.

Usage
-----

Usually, you can start the RedTimer executable from your file manager. After closing, RedTimer will create a
settings file called `RedTimer.ini` in your current working directory.

If you experience problems with the system tray icon, e.g. on Linux, you can deactivate the system tray icon
by calling `RedTimer --no-tray-icon`.

Installation instructions
-------------------------

The simplest way is to pick a binary from GitHub (https://github.com/fathomssen/redtimer/releases).

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

This requires for you to have Qt 5.6+ and GCC 4.8.4+ installed and in your path.

Alternatively, you can use a QtCreator distribution from https://www.qt.io. In QtCreator, you can open the
project files `qtredmine/qtredmine.pro` and `RedTimer.pro` and start the build.

Todo
----

* Issue creator
* Cache for offline operation
* Spooler for unsaved time entries
