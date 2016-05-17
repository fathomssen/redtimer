Ubuntu Instructions
===================

Installation
------------

To run RedTimer in Ubuntu, you can download a binary version from
https://github.com/fathomssen/redtimer/releases.

### Ubuntu 14.04 ###

For Ubuntu 14.04, the required Qt5 libraries are deployed in the package. You will still have to install some
dependencies though:

```
sudo apt-get install libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-randr0 libxcb-render-util0 libxcb-xinerama0
```

### Ubuntu 16.04 ###

For Ubuntu 16.04, you will have to install the following package:

```
sudo apt-get install qtdeclarative5-controls-plugin
```

Building
--------

### Ubuntu 14.04 ###

You first have to install current Qt 5 packages in Ubuntu:

```
sudo add-apt-repository ppa:beineri/opt-qt56-trusty
sudo apt-get install qt56base qt56quickcontrols qt56quickcontrols2 qt56svg qt56x11extras mesa-common-dev libgl1-mesa-dev
source /opt/qt56/bin/qt56-env.sh
```

Then, you can follow the build instructions from the [main README](README.md).

### Ubuntu 16.04 ###

You first have to install some Qt 5 development packages in Ubuntu:

```
sudo apt-get install build-essential qtbase5-dev qtdeclarative5-dev libqt5svg5-dev libqt5x11extras5-dev qtbase5-private-dev

```

Then, you can follow the build instructions from the README.md file. If you have Qt 4 and Qt 5 installed, you
have to use the command `qmake -qt5 -r` instead of just `qmake -r`.
