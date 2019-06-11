#!/bin/sh

set -x
cd bin
VERSION="8.0"
NAME="rmap"

rm  $NAME-$VERSION-release-signed-aligned.apk

jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/my-release-key.keystore  $NAME-$VERSION-release-unsigned.apk  alias_name

~/.buildozer/android/platform/android-sdk/build-tools/28.0.3/zipalign -v 4 $NAME-$VERSION-release-unsigned.apk $NAME-$VERSION-release-signed-aligned.apk

cd ..

