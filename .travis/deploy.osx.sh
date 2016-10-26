#!/bin/bash

if [ $# -ne 1 ]; then
  echo "Usage: $0 <output prefix>"
  exit 1
fi

export TRAVISDIR=$(cd "$(dirname "$0")"; pwd)
export ROOTDIR=$TRAVISDIR/..
export PREFIX=$ROOTDIR/$1

cd $ROOTDIR/client

macdeployqt RedTimerClient.app -appstore-compliant -qmldir=qml
export QTDIR=$(brew info qt5 | grep "^/usr/local/Cellar/qt5" | cut -f 1 -d " ")
cp -a $QTDIR/plugins/{bearer,iconengines} RedTimerClient.app/Contents/PlugIns
python $TRAVISDIR/macdeployqtfix/macdeployqtfix.py RedTimerClient.app/Contents/MacOS/RedTimerClient $QTDIR/
cp -a $TRAVISDIR/Info.plist RedTimerClient.app/Contents
sed -i '' "s/__VERSION__/${TRAVIS_TAG}/" RedTimerClient.app/Contents/Info.plist

# Thanks to https://medium.com/juan-cruz-viotti/how-to-code-sign-os-x-electron-apps-in-travis-ci-6b6a0756c04a#.x84x807hk
export CERTIFICATE_P12="Certificate.p12"
echo $CERTIFICATE_OSX_P12 | base64 -D > $CERTIFICATE_P12
export KEYCHAIN=build.keychain
security create-keychain -p mysecretpassword $KEYCHAIN
security default-keychain -s $KEYCHAIN
security unlock-keychain -p mysecretpassword $KEYCHAIN
security import $CERTIFICATE_P12 -k $KEYCHAIN -P $CERTIFICATE_PASSWORD -T /usr/bin/codesign

# Use first ID
set -x
security find-identity -v $KEYCHAIN
export ID=$(security find-identity -v $KEYCHAIN | grep "1)" | sed "s/^ *1) *\([^ ]*\).*/\1/")
echo $ID
codesign --deep --force --verbose --sign $ID --keychain $KEYCHAIN RedTimerClient.app
set +x

# Thanks to https://asmaloney.com/2013/07/howto/packaging-a-mac-os-x-application-using-a-dmg
hdiutil create -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -srcfolder RedTimerClient.app -volname RedTimerClient tmp.dmg
export DEVICE=$(hdiutil attach -readwrite -noverify tmp.dmg | egrep '^/dev/' | sed 1q | awk '{print $1}')
pushd /Volumes/RedTimerClient
ln -s /Applications
popd
sync
hdiutil detach "${DEVICE}"
hdiutil convert tmp.dmg -format UDZO -imagekey zlib-level=9 -o $PREFIX.dmg

# Generate tar.bz2 file
tar jcf $PREFIX.tar.bz2 RedTimerClient.app
