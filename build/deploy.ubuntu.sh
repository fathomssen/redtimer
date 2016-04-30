#!/bin/bash

set -ex

function usage()
{
  echo "Usage: $0 [-qt|--deploy-qt] [-h|--help]"
  echo
  echo "    -qt   Deploy Qt with RedTimer"
  echo "    -h    Display this help"
}

while [ $# -gt 0 ]; do
  case $1 in
    "--")
      shift
      break
      ;;
    "-qt"|"--deploy-qt")
      deploy_qt=true
      ;;
    "-h"|"--help")
      usage
      exit 0
      ;;
    *)
      usage
      exit 1
      break
      ;;
  esac

  shift
done

execdir=$(dirname $0)
src=$execdir/..

ver=$(cat RedTimer.pro | grep "^VERSION")
ver=${ver##VERSION = }

dist=$(lsb_release -i -s)
dist=${dist,,}
distver=$(lsb_release -r -s)

machine=$(uname -m)

dir="redtimer-v${ver}-${dist}${distver}-$machine"

distdir=$src/build/dist/$dir

mkdir -p $distdir

cp -a $src/LICENSE $distdir
cp -a $src/RedTimer $distdir
cp -a $src/redtimer.sh $distdir

cp -a $src/qtredmine/libqtredmine.so* $distdir

if [ $deploy_qt ]; then
  # Get Qt library directory from RedTimer executable
  # This required the build environment to be still intact
  qtlibdir=$(dirname $(LD_LIBRARY_PATH=. ldd RedTimer | grep libQt5Core | sed 's/.*=> *//' | sed 's/ *(.*//'))
  qtdir=$qtlibdir/..
  qtqmldir=$qtdir/qml
  qtpluginsdir=$qtdir/plugins

  cp -a $qtlibdir/libQt5Core.so* $distdir
  cp -a $qtlibdir/libQt5DBus.so* $distdir
  cp -a $qtlibdir/libQt5Gui.so* $distdir
  cp -a $qtlibdir/libQt5Network.so* $distdir
  cp -a $qtlibdir/libQt5Qml.so* $distdir
  cp -a $qtlibdir/libQt5Quick.so* $distdir
  cp -a $qtlibdir/libQt5Svg.so* $distdir
  cp -a $qtlibdir/libQt5Widgets.so* $distdir
  cp -a $qtlibdir/libQt5XcbQpa.so* $distdir

  cp -a $qtqmldir/QtQuick $distdir
  cp -a $qtqmldir/QtQuick.2 $distdir

  mkdir -p $distdir/iconengines
  cp -a $qtpluginsdir/iconengines/libqsvgicon.so $distdir/iconengines

  mkdir -p $distdir/imageformats
  cp -a $qtpluginsdir/imageformats/libqsvg.so $distdir/imageformats

  mkdir -p $distdir/platforms
  cp -a $qtpluginsdir/platforms/libqxcb.so $distdir/platforms

  mkdir -p $distdir/platformthemes
  cp -a $qtpluginsdir/platformthemes/libqgtk2.so $distdir/platformthemes

  mkdir -p $distdir/xcbglintegrations
  cp -a $qtpluginsdir/xcbglintegrations/libqxcb-glx-integration.so $distdir/xcbglintegrations
fi

distfile=$dir.tar.bz2

tar jcf $distfile -C $distdir/.. $dir

echo $distfile
