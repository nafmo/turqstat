// Copyright (c) 1999 Peter Karlsson
//
// $Id$
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
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

#include <string>

#include "arearead.h"
#include "statengine.h"
#include "datatypes.h"

#ifdef __EMX__
# pragma pack(1)
#endif

class MyPointRead : public AreaRead
{
public:
    // Constructor and destructor
    MyPointRead(const char *path);
    virtual ~MyPointRead();

    // Transfer function
    virtual bool Transfer(time_t starttime, StatEngine &destination);

protected:
    char        *areapath;

    // Header of area file (.A??)

    struct header_s
    {
        UINT16      areaprf;
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
        char        spare[14];      /* Total size: 192 */
        char        fill[256-192-2];/* Fill to 256 bytes */
        UINT32      null;
    };

    // Letter structure in area file

    struct ltrhdr_s
    {
        UINT32      delim;          /* $feffffff */
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
    };

    struct ltrtrl_s
    {
        UINT16      ltrsiz;
        UINT16      ltrnum;
    };

    // Flag files (.F??)
    struct flags_s
    {
        UINT8       bits;
        UINT8       number;
        UINT8       rdrpy;
    };

    const Mypoint_msgbaseversion    = 2;
    const UINT32 Mypoint_delimeter  = 0xfeffffff;
    const Mypoint_delete            = 0x40;
};

#endif
