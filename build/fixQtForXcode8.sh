#!/bin/bash

export QTDIR=$(brew info qt5 | grep "^/usr/local/Cellar/qt5" | cut -f 1 -d " ")
export PRF=${QTDIR}/mkspecs/features/mac/default_pre.prf

sed -i sav 's!/usr/bin/xcrun -find xcrun!/usr/bin/xcrun -find xcodebuild!' $PRF
