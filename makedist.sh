#!/bin/bash
# $Id$

if [ "$1" == "" ]; then
  echo -n "Ange versionsnummer: "
  read ver
  test -z ver && exit
else
  ver=$1
fi

if [ "$2" == "nodeb" ]; then
  echo "Bygger inget Debianarkiv"
fi

echo '* Makedist: Turquoise SuperStat version '$ver

rm -rf turqstat-$ver/
mkdir -p turqstat-$ver/debian
autoconf
autoheader
cp -p *.spec *.def *.cpp *.h Makefile.in config.h.in configure configure.in ChangeLog COPYING turqstat-$ver/
rm turqstat-$ver/config.h
rm turqstat-$ver/moc*.cpp
iso2dos -i turqstat.doc  turqstat-$ver/turqstat.doc
touch -r   turqstat.doc  turqstat-$ver/turqstat.doc
iso2dos -i turqstat.dok  turqstat-$ver/turqstat.dok
touch -r   turqstat.dok  turqstat-$ver/turqstat.dok
iso2dos -i turqstat.1    turqstat-$ver/turqstat.1
touch -r   turqstat.1    turqstat-$ver/turqstat.1
iso2dos -i turqstat-sv.1 turqstat-$ver/turqstat-sv.1
touch -r   turqstat-sv.1 turqstat-$ver/turqstat-sv.1
cp -p debian/* turqstat-$ver/debian

fakeroot tar cvfz turqstat-$ver.tar.gz turqstat-$ver/*
if [ "$2" != "nodeb" ]; then
  cd turqstat-$ver
  fakeroot debian/rules binary
  cd ..
fi
rm -rf turqstat-$ver/
