// Copyright (c) 1999-2001 Peter Karlsson
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

#include <config.h>

#include "datatypes.h"
#include "arearead.h"

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
        uint8_t       areaprf[2];
        uint8_t       echoid[71];
        uint8_t       deskr[26];
        uint8_t       origin[51];
        le_uint16_t   zone;
        le_uint16_t   net;
        le_uint16_t   node;
        le_uint16_t   point;
        le_uint16_t   bosszone;
        le_uint16_t   bossnet;
        le_uint16_t   bossnode;
        le_uint16_t   bosspoint;
        le_uint16_t   routezone;
        le_uint16_t   routenet;
        le_uint16_t   routenode;
        le_uint16_t   routepoint;
        le_uint16_t   delday;
        uint8_t       ro;
        uint8_t       arenum;
        le_uint16_t   delnum;
        uint8_t       spare[14];      // Total size: 192
        uint8_t       fill[256-192-2];// Fill to 256 bytes
    };

    // Letter structure in area file

    union ltrhdr_u
    {
        struct
        {
            le_uint32_t   delim;          // $feffffff
            le_uint16_t   ltrsiz;
            le_uint32_t   arrtim;
            uint8_t       area;
            uint8_t       chrset;
            le_uint16_t   destzone;
            le_uint16_t   destnet;
            le_uint16_t   destnode;
            le_uint16_t   destpoint;
            le_uint16_t   origzone;
            le_uint16_t   orignet;
            le_uint16_t   orignode;
            le_uint16_t   origpoint;
            le_uint32_t   ltrtim;
            le_uint16_t   fromp;
            le_uint16_t   subjp;
            le_uint16_t   ltridp;
            le_uint16_t   qltrid;
            le_uint16_t   replyp;
            le_uint16_t   qreply;
            le_uint16_t   textp;
            le_uint16_t   originp;
            le_uint16_t   lflags;
            le_uint16_t   flags1;
            uint8_t       flags2;
            uint8_t       split;
            le_uint16_t      spare;
        } version2;
        struct
        {
            le_uint32_t   delim;          // $feffffff
            le_uint16_t   ltrsiz;
            le_uint32_t   arrtim;
            uint8_t       area;
            uint8_t       chrset;
            le_uint16_t   destzone;
            le_uint16_t   destnet;
            le_uint16_t   destnode;
            le_uint16_t   destpoint;
            le_uint16_t   origzone;
            le_uint16_t   orignet;
            le_uint16_t   orignode;
            le_uint16_t   origpoint;
            le_uint32_t   ltrtim;
            le_uint16_t   fromp;
            le_uint16_t   subjp;
            le_uint16_t   ltridp;
            le_uint32_t   qltrid;
            le_uint16_t   replyp;
            le_uint32_t   qreply;
            le_uint16_t   textp;
            le_uint16_t   originp;
            le_uint16_t   lflags;
            le_uint16_t   flags1;
            uint8_t       flags2;
            uint8_t       split;          // Total size: 58 byte
            uint8_t       spare[64-58];   // Fill to 64 bytes
        } version3;
    };

    union ltrtrl_u
    {
        struct
        {
            le_uint16_t   ltrsiz;
            le_uint16_t   ltrnum;
        } version2;
        struct
        {
            le_uint16_t   ltrsiz;
            le_uint32_t   ltrnum;
        } version3;
    };

    // Flag files (.F??)
    struct flags_s
    {
        uint8_t       bits;
        uint8_t       number;
        uint8_t       rdrpy;
    };

    static const uint8_t  Mypoint_msgbaseversion2 = 2;
    static const uint8_t  Mypoint_msgbaseversion3 = 3;
    static const uint32_t Mypoint_delimeter       = 0xfeffffff;
    static const uint8_t  Mypoint_delete          = 0x40;
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
