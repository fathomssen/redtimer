#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Usage: $0 <output prefix> <version>"
  exit 1
fi

# Set by /opt/qtXX/bin/qtXX-env.sh
# These may not be set for linuxdeployqt
unset LD_LIBRARY_PATH
unset PKG_CONFIG_PATH

export TRAVISDIR=$(cd "$(dirname "$0")"; pwd)
export ROOTDIR=$TRAVISDIR/..
export PREFIX=$1
export VERSION=$2

cd $ROOTDIR

##################### AppImage creation #####################

# Create a new dist folder
rm -rf dist
mkdir -p dist/opt/redtimer

# Include binary and dist files
cp gui/redtimer dist/opt/redtimer
cp cli/redtimercli dist/opt/redtimer
cp gui/icons/clock_red.svg dist/opt/redtimer/redtimer.svg
cp deploy/redtimer.desktop dist/opt/redtimer

# Include things possibly missing from linuxdeployqt
mkdir -p dist/opt/redtimer/lib
cp -a $QTDIR/lib/$(readlink $QTDIR/lib/libQt5Svg.so.5) dist/opt/redtimer/lib/libQt5Svg.so.5
mkdir -p dist/opt/redtimer/plugins
cp -a $QTDIR/plugins/xcbglintegrations dist/opt/redtimer/plugins

# Create AppImage
deploy/linuxdeployqt dist/opt/redtimer/redtimercli -qmldir=gui/qml -verbose=2
deploy/linuxdeployqt dist/opt/redtimer/redtimer -qmldir=gui/qml -appimage -bundle-non-qt-libs -verbose=2

mv RedTimer*.AppImage $PREFIX.AppImage

##################### DEB/RPM creation #####################

if [ -z "$VERSION" ]; then
  export VERSION="0.1-pre0"
  echo "Building dummy version $VERSION"
fi

mkdir -p dist/usr/bin
mkdir -p dist/usr/share/applications
mkdir -p dist/usr/share/icons/hicolor/scalable/apps

ln -sf /opt/redtimer/redtimer dist/usr/bin/redtimer
ln -sf /opt/redtimer/redtimercli dist/usr/bin/redtimercli
mv dist/opt/redtimer/redtimer.desktop dist/usr/share/applications
mv dist/opt/redtimer/redtimer.svg dist/usr/share/icons/hicolor/scalable/apps
rm -f dist/opt/redtimer/default.png

# Add Debian-specific files
export TIMESTAMP=$(date -R)
cat > CHANGELOG <<- END
redtimer ($VERSION) unstable; urgency=low

  * Dummy changelog to please lintian

 -- Frederick Thomssen <thomssen@thomssen-it.de>  $TIMESTAMP
END

mkdir -p dist/usr/share/doc/redtimer
cp $TRAVISDIR/debian/copyright dist/usr/share/doc/redtimer

# Set correct filesystem permissions
find dist/* -type d -exec chmod 755 {} \;
find dist/* -type f -exec chmod 644 {} \;
chmod 755 dist/opt/redtimer/redtimer
chmod 755 dist/opt/redtimer/redtimercli

export MAINT="Frederick Thomssen <thomssen@thomssen-it.de>"
export DESCR="Redmine Time Tracker\n\n
RedTimer is an easy-to-use platform-independent time tracker which allows the\n
user to track time while working on an issue. Furthermore, a user can create\n
new issues using RedTimer during which the time tracking will already start."

rm -f $PREFIX.deb $PREFIX.rpm

# .deb
export DEBVERSION=${VERSION//-/\~} # Convert - to ~
fpm -s dir -t deb -p $PREFIX.deb -m "${MAINT}" --description "${DESCR}" -n redtimer -v ${DEBVERSION} -C dist --category net --deb-no-default-config-files --deb-changelog CHANGELOG --no-depends

# .rpm
export RPMVERSION=${VERSION//-/\_} # Convert - to _
fpm -s dir -t rpm -p $PREFIX.rpm -m "${MAINT}" --description "${DESCR}" -n redtimer -v ${RPMVERSION} -C dist --category net --deb-no-default-config-files
