#!/bin/bash

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
mkdir -p turqstat-$ver/debian turqstat-$ver/po turqstat-$ver/mappings
autoconf
autoheader
cp -p *.spec *.def *.cpp *.h Makefile.in config.h.in doxygen.conf.in configure configure.in ChangeLog COPYING turqstat-$ver/
rm turqstat-$ver/config.h
rm turqstat-$ver/moc*.cpp

cp -p turqstat.do{c,k} {,x}turqstat{-sv,}.1 turqstat-$ver/
cp -p debian/* turqstat-$ver/debian
cp -p po/*.po po/update-po.sh po/Makefile.in turqstat-$ver/po/
cp -p mappings/*.txt mappings/Makefile.in mappings/*.{cpp,h,pl} turqstat-$ver/mappings/

fakeroot tar cvfz turqstat_$ver.tar.gz turqstat-$ver/*
if [ "$2" != "nodeb" ]; then
  cd turqstat-$ver
  fakeroot debian/rules binary
  cd ..
fi
rm -rf turqstat-$ver/
