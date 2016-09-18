#!/bin/bash

macdeployqt RedTimerClient.app -appstore-compliant -qmldir=qml
export QTDIR=$(brew info qt5 | grep "^/usr/local/Cellar/qt5" | cut -f 1 -d " ")
cp -a $QTDIR/plugins/{bearer,iconengines} RedTimerClient.app/Contents/PlugIns
python ../build/macdeployqtfix/macdeployqtfix.py RedTimerClient.app/Contents/MacOS/RedTimerClient $QTDIR/

# Thanks to https://medium.com/juan-cruz-viotti/how-to-code-sign-os-x-electron-apps-in-travis-ci-6b6a0756c04a#.x84x807hk
export CERTIFICATE_P12="Certificate.p12"
echo $CERTIFICATE_OSX_P12 | base64 -D > $CERTIFICATE_P12
export KEYCHAIN=build.keychain
security create-keychain -p mysecretpassword $KEYCHAIN
security default-keychain -s $KEYCHAIN
security unlock-keychain -p mysecretpassword $KEYCHAIN
security import $CERTIFICATE_P12 -k $KEYCHAIN -P $CERTIFICATE_PASSWORD -T /usr/bin/codesign

# Use first ID
export ID=$(find-identity -v $KEYCHAIN | grep "1)" | sed "s/.*) \([^ ]*\).*/\1/")
codesign --deep --force --verbose --sign $ID --keychain $KEYCHAIN RedTimerClient.app

# Thanks to https://asmaloney.com/2013/07/howto/packaging-a-mac-os-x-application-using-a-dmg
hdiutil create -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -srcfolder RedTimerClient.app -volname RedTimerClient tmp.dmg
export DEVICE=$(hdiutil attach -readwrite -noverify tmp.dmg | egrep '^/dev/' | sed 1q | awk '{print $1}')
pushd /Volumes/RedTimerClient
ln -s /Applications
popd
sync
hdiutil detach "${DEVICE}"
hdiutil convert tmp.dmg -format UDZO -imagekey zlib-level=9 -o RedTimerClient.dmg
