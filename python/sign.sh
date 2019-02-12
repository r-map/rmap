#!/bin/sh

set -x
cd bin
VERSION="8.0"
NAME="Rmap"

rm $NAME-$VERSION-release-unaligned.apk  $NAME-$VERSION-release-signed.apk

jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/my-release-key.keystore  $NAME-$VERSION-release.apk  rmap
~/.buildozer/android/platform/android-sdk-20/build-tools/23.0.1/zipalign -v 4 $NAME-$VERSION-release-unaligned.apk $NAME-$VERSION-release-signed.apk

cd ..

