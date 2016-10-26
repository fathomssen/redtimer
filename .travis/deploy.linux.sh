#!/bin/bash

if [ $# -ne 1 ]; then
  echo "Usage: $0 <output file>"
  exit 1
fi

export TRAVISDIR=$(cd "$(dirname "$0")"; pwd)
export ROOTDIR=$TRAVISDIR/..
export DEPLOYFILE=$1

cd $ROOTDIR

# Create a new dist folder
rm -rf dist
mkdir dist

# Include binary and dist files
cp client/RedTimerClient dist
cp client/icons/clock_red.svg dist
cp .travis/RedTimerClient.desktop dist

# Include SVG icon engine
mkdir -p dist/plugins/iconengines
cp -a /opt/qt57/plugins/iconengines/libqsvgicon.so dist/plugins/iconengines

# First run
.travis/linuxdeployqt.AppImage dist/RedTimerClient -qmldir=client/qml -appimage -bundle-non-qt-libs -verbose=2

# Second run, to include xcbglintegration
rm dist.AppImage
.travis/linuxdeployqt.AppImage dist/RedTimerClient -qmldir=client/qml -appimage -bundle-non-qt-libs -verbose=2

mv dist.AppImage $DEPLOYFILE
