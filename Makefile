# Makefile for OS/2, EMX and IBM nmake
#
# $Id$

CFLAGS=-Wall -Zcrtdll -Zomf -Ismapi -Lsmapi -O3

all: turqoise.exe

turqoise.exe: turqoise.obj statengine.obj statview.obj turqoise.def squishread.obj smapi/smapiemo.lib
        gcc $(CFLAGS) -o turqoise.exe *.obj *.def -lsmapiemo -lstdcpp -lgpp

turqoise.obj: turqoise.cpp statengine.h statview.h arearead.h squishread.h
        gcc $(CFLAGS) -c turqoise.cpp

statengine.obj: statengine.cpp statengine.h
        gcc $(CFLAGS) -c statengine.cpp

statview.obj: statview.cpp statview.h statengine.h
        gcc $(CFLAGS) -c statview.cpp

squishread.obj: squishread.cpp squishread.h arearead.h statengine.h
        gcc $(CFLAGS) -c squishread.cpp

smapi/smapiemo.lib:
        cd smapi
        nmake -f makefile.emo
        cd ..

dist: turqoise.exe source.rar
        rar -av a turqoise.rar turqoise.exe turqoise.doc turqoise.dok source.rar COPYING
        del source.rar

source.rar:
        rar -s -av a source.rar *.def *.cpp *.h Makefile smapi\*.c smapi\*.h smapi\*.asm smapi\*.txt smapi\*.diz smapi\makefile.*

clean: almostclean
        -del smapi\*.obj
        -del smapi\*.lib
        -del smapi\*.bak

almostclean:
        -del *.obj
        -del *.exe
