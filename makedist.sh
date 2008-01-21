#!/bin/bash

if [ "$1" == "" ]; then
  echo -n "Ange versionsnummer: "
  read ver
  test -z ver && exit
else
  ver=$1
fi

git archive --format=tar --prefix=turqstat-$ver/ HEAD | gzip -9 > turqstat-$ver.tar.gz
