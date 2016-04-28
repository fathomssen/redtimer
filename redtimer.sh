#!/bin/bash

# @todo: Other desktops as well?
if [ "${XDG_CURRENT_DESKTOP}" == "XFCE" ]; then
  export QT_STYLE_OVERRIDE="gtk2"
fi

dir=$(dirname $0)

LD_LIBRARY_PATH=$dir $dir/RedTimer $@
