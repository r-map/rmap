#!/bin/sh

set -x
cd bin
VERSION="2.12"
NAME="Rmap"

rm $NAME-$VERSION-release-unaligned.apk  $NAME-$VERSION-release-signed.apk

jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 keystore ~/my-release-key.keystore -signedjar $NAME-$VERSION-release-unaligned.apk $NAME-$VERSION-release-unsigned.apk alias_name
~/.buildozer/android/platform/android-sdk-21/build-tools/22.0.1/zipalign -v 4 $NAME-$VERSION-release-unaligned.apk $NAME-$VERSION-release-signed.apk

cd ..

