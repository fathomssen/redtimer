#!/bin/bash

dir=$(dirname $0)
export LD_LIBRARY_PATH=$dir

missing="$(ldd $dir/RedTimerClient | grep 'not found')"
if [ -n "$missing" ]; then
  echo "Missing dependencies"
  echo
  echo "Please install the following packages:"
  echo
  echo "- Ubuntu 14.04"
  echo "  $ sudo apt-get install libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-randr0 libxcb-render-util0 libxcb-xinerama0"
  echo
  echo "- Ubuntu 16.04"
  echo "  $ sudo apt-get install qtdeclarative5-controls-plugin libqt5x11extras5"

  exit 1
fi

# @todo: Other desktops as well?
if [ "${XDG_CURRENT_DESKTOP}" == "XFCE" ]; then
  export QT_STYLE_OVERRIDE="gtk2"
fi

$dir/RedTimerClient $@
