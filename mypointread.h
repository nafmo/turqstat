// Copyright (c) 1999-2000 Peter Karlsson
//
// $Id$
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef __MYPOINTREAD_H
#define __MYPOINTREAD_H

#include "arearead.h"
#include "datatypes.h"

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

class StatEngine;

class MyPointRead : public AreaRead
{
public:
    // Constructor and destructor
    MyPointRead(const char *path, unsigned areanum);
    virtual ~MyPointRead();

    // Transfer function
    virtual bool Transfer(time_t starttime, StatEngine &destination);

protected:
    char        *areapath;
    unsigned    areanumber;

    // Header of area file (.A??)

    struct header_s
    {
        UINT8       areaprf[2];
        char        echoid[71];
        char        deskr[26];
        char        origin[51];
        UINT16      zone;
        UINT16      net;
        UINT16      node;
        UINT16      point;
        UINT16      bosszone;
        UINT16      bossnet;
        UINT16      bossnode;
        UINT16      bosspoint;
        UINT16      routezone;
        UINT16      routenet;
        UINT16      routenode;
        UINT16      routepoint;
        UINT16      delday;
        UINT8       ro;
        UINT8       arenum;
        UINT16      delnum;
        char        spare[14];      // Total size: 192
        char        fill[256-192-2];// Fill to 256 bytes
    };

    // Letter structure in area file

    union ltrhdr_u
    {
        struct
        {
            UINT32      delim;          // $feffffff
            UINT16      ltrsiz;
            UINT32      arrtim;
            UINT8       area;
            UINT8       chrset;
            UINT16      destzone;
            UINT16      destnet;
            UINT16      destnode;
            UINT16      destpoint;
            UINT16      origzone;
            UINT16      orignet;
            UINT16      orignode;
            UINT16      origpoint;
            UINT32      ltrtim;
            UINT16      fromp;
            UINT16      subjp;
            UINT16      ltridp;
            UINT16      qltrid;
            UINT16      replyp;
            UINT16      qreply;
            UINT16      textp;
            UINT16      originp;
            UINT16      lflags;
            UINT16      flags1;
            UINT8       flags2;
            UINT8       split;
            UINT16      spare;
        } version2;
        struct
        {
            UINT32      delim;          // $feffffff
            UINT16      ltrsiz;
            UINT32      arrtim;
            UINT8       area;
            UINT8       chrset;
            UINT16      destzone;
            UINT16      destnet;
            UINT16      destnode;
            UINT16      destpoint;
            UINT16      origzone;
            UINT16      orignet;
            UINT16      orignode;
            UINT16      origpoint;
            UINT32      ltrtim;
            UINT16      fromp;
            UINT16      subjp;
            UINT16      ltridp;
            UINT32      qltrid;
            UINT16      replyp;
            UINT32      qreply;
            UINT16      textp;
            UINT16      originp;
            UINT16      lflags;
            UINT16      flags1;
            UINT8       flags2;
            UINT8       split;          // Total size: 58 byte
            UINT8       spare[64-58];   // Fill to 64 bytes
        } version3;
    };

    union ltrtrl_u
    {
        struct
        {
            UINT16      ltrsiz;
            UINT16      ltrnum;
        } version2;
        struct
        {
            UINT16      ltrsiz;
            UINT32      ltrnum;
        } version3;
    };

    // Flag files (.F??)
    struct flags_s
    {
        UINT8       bits;
        UINT8       number;
        UINT8       rdrpy;
    };

    static const UINT8  Mypoint_msgbaseversion2 = 2;
    static const UINT8  Mypoint_msgbaseversion3 = 3;
    static const UINT32 Mypoint_delimeter       = 0xfeffffff;
    static const UINT8  Mypoint_delete          = 0x40;
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
