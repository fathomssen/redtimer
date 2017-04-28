[![Travis CI build](https://travis-ci.org/fathomssen/redtimer.svg?branch=master "Travis CI build")](https://travis-ci.org/fathomssen/redtimer)
[![AppVeyor CI build](https://ci.appveyor.com/api/projects/status/github/fathomssen/redtimer "AppVeyor CI build")](https://ci.appveyor.com/project/fathomssen/redtimer)

Redmine Time Tracker
====================

RedTimer is an easy-to-use platform-independent time tracker which allows the user to track time while working
on an issue. Furthermore, a user can create new issues using RedTimer during which the time tracking will
already start.

Usage
-----

After starting RedTimer for the first time, the settings window will be displayed. In the settings window,
you can specify the URL of the Redmine instance and the API key which can be found in the Redmine account
settings.

![Settings window](images/settings.png?raw=true "Settings window")

In the main window, you can enter or select an issue. After issue selection, the issue data will be displayed
and the timer can be started.

![Main window](images/main_window.png?raw=true "Main window")

To select an issue, you can use the Issue Selector which lists available issues according to the selected
project, assignee and version.

![Issue Selector](images/issue_selector.png?raw=true "Issue Selector")

To create a new issue, you can use the Issue Creator which will start tracking time while you type the new
issue data.

![Issue Creator](images/issue_creator.png?raw=true "Issue Creator")

Installation instructions
-------------------------

For Ubuntu instructions, please have a look at the [Ubuntu README](README.Ubuntu.md).

The simplest way is to pick a binary from GitHub (https://github.com/fathomssen/redtimer/releases).

The latest development build for Windows can be found at
https://ci.appveyor.com/project/fathomssen/redtimer/build/artifacts.

To use Redmine custom fields in RedTimer, please install the `redmine_shared_api` plugin from
https://github.com/anovitsky/redmine_shared_api.

The following instructions can be used to build the application from source.

```
git clone https://github.com/fathomssen/redtimer.git
cd redtimer
git submodule update --init

qmake -r
make
```

This requires for you to have Qt 5.5+ and GCC 4.8.4+ installed and in your path.

Alternatively, you can use a QtCreator distribution from https://www.qt.io. In QtCreator, you can open the
project file `RedTimer.pro` and start the build.
