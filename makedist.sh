#!/bin/bash

if [ "$1" == "" ]; then
  echo -n "Ange versionsnummer: "
  read ver
  test -z ver && exit
else
  ver=$1
fi

echo '* Makedist: Turquoise SuperStat version '$ver

mkdir -p turqstat-$ver/debian
autoconf
autoheader
cp -p *.1 *.def *.cpp *.h Makefile.in config.h.in configure configure.in ChangeLog COPYING turqstat-$ver/
iso2dos -i turqstat.doc turqstat-$ver/turqstat.doc
iso2dos -i turqstat.dok turqstat-$ver/turqstat.dok
cp -p debian/* turqstat-$ver/debian
fakeroot tar cvfz turqstat-$ver.tar.gz turqstat-$ver/*
rm -rf turqstat-$ver/
