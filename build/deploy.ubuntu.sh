#!/bin/bash

set -e

function usage()
{
  echo "Usage: $0 [-qt|--deploy-qt] [-h|--help] <version>"
  echo
  echo "    -qt   Deploy Qt with RedTimer"
  echo "    -h    Display this help"
  echo
  echo "This will produce <version>.tar.bz2"
}

if [ $# -eq 0 ]; then
  usage
  exit 1
fi

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
      if [ -z "$redtimer" ]; then
        redtimer=$1
        shift
        continue
      fi
      usage
      exit 1
      ;;
  esac

  shift
done

execdir=$(dirname $0)
src=$execdir/..

distdir=$execdir/$redtimer

mkdir -p $distdir

cp -a $src/LICENSE $distdir
cp -a $src/client/RedTimerClient $distdir

if [ $deploy_qt ]; then
  # Get Qt libraries from RedTimer executable  
  # This required the build environment to be still intact
  
  qtlibsarr=()

  function findQtLibraries()
  {
    for lib in $@; do
      for qtlibdir in $qtlibdirs; do
	file=$qtlibdir/$lib
	if [ -e $file ]; then
	  qtlibsarr+=(${file})
	fi
      done
    done
  }

  # Look in typical locations for QML and Qt plugins relative to the library dir
  reldirs="qt5 . ../qt5 .."

  function findQtQmlDir()
  {
    for qtlibdir in $qtlibdirs; do
      for reldir in $reldirs; do
	dir=$qtlibdir/$reldir/qml
        if [ -d $dir ]; then
          qtqmldir=$dir
	  break
        fi
      done
    done

    if [ -z "$qtqmldir" ]; then
      exit 1
    fi
  }
  
  function findQtPluginsDir()
  {
    for qtlibdir in $qtlibdirs; do
      for reldir in $reldirs; do
	dir=$qtlibdir/$reldir/plugins
        if [ -d $dir ]; then
          qtpluginsdir=$dir
	  break
        fi
      done
    done

    if [ -z "$qtpluginsdir" ]; then
      exit 1
    fi
  }
  
  # Get all Qt libraries required by RedTimer
  qtlibs=$(ldd -v client/RedTimerClient | grep libQt | grep "=>" | sed 's/.*=> *//' | sed 's/\.so.*/.so/' | sort -u)
  for lib in $qtlibs; do
    qtlibsarr+=($lib)
  done
  
  # Get all Qt libraries' directories
  qtlibdirs=$(dirname $(echo "$qtlibs") | sort -u)
  
  findQtLibraries libQt5DBus.so libQt5Svg.so libQt5XcbQpa.so
  
  for lib in ${qtlibsarr[@]}; do
    cp -a ${lib}* $distdir
  done

  findQtQmlDir
  findQtPluginsDir

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

distfile=$redtimer.tar.bz2

tar jcf -9 $distfile -C $distdir/.. $redtimer
