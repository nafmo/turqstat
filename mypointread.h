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
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
# include "datatypes.h"
#endif

#include "arearead.h"

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

class StatEngine;

/**
 * Class that reads MyPoint message bases.
 * This class reads message bases stored in MyPoint format, either
 * revision 2 or revision 3.
 */
class MyPointRead : public AreaRead
{
public:
    /** Standard constructor.
     * Creates the MyPoint message base reader object.
     * @param path    Directory for message base.
     * @param areanum Number of area to read.
     */
    MyPointRead(const char *path, unsigned areanum);
    /** Standard destructor. */
    virtual ~MyPointRead();

    /**
     * Transfer function. 
     * This function transfers all messages in the message bases, received
     * after the specified starting date, to the specified statistics engine.
     *
     * @param starttime   Date to start retrieve statistics from.
     * @param endtime     Date to stop retrieve statistics at.
     * @param destination Engine object to transfer data to.
     * @return True if the message base was read correctly (even if no
     *         messages fits the condition).
     */
    virtual bool Transfer(time_t starttime, time_t endtime,
                          StatEngine &destination);

protected:
    /** Path to message base. */
    char        *areapath;
    /** Number of area to read. */
    unsigned    areanumber;

    /** Header of the area file (.A??). */
    struct header_s
    {
        uint8_t       areaprf[2];
        uint8_t       echoid[71];
        uint8_t       deskr[26];
        uint8_t       origin[51];
        uint16_t      zone;
        uint16_t      net;
        uint16_t      node;
        uint16_t      point;
        uint16_t      bosszone;
        uint16_t      bossnet;
        uint16_t      bossnode;
        uint16_t      bosspoint;
        uint16_t      routezone;
        uint16_t      routenet;
        uint16_t      routenode;
        uint16_t      routepoint;
        uint16_t      delday;
        uint8_t       ro;
        uint8_t       arenum;
        uint16_t      delnum;
        uint8_t       spare[14];      // Total size: 192
        uint8_t       fill[256-192-2];// Fill to 256 bytes
    };

    /** Letter structure in the area file (.A??). */
    union ltrhdr_u
    {
        /** Structure for version 2 message bases. */
        struct
        {
            uint32_t      delim;          // $feffffff
            uint16_t      ltrsiz;
            uint32_t      arrtim;
            uint8_t       area;
            uint8_t       chrset;
            uint16_t      destzone;
            uint16_t      destnet;
            uint16_t      destnode;
            uint16_t      destpoint;
            uint16_t      origzone;
            uint16_t      orignet;
            uint16_t      orignode;
            uint16_t      origpoint;
            uint32_t      ltrtim;
            uint16_t      fromp;
            uint16_t      subjp;
            uint16_t      ltridp;
            uint16_t      qltrid;
            uint16_t      replyp;
            uint16_t      qreply;
            uint16_t      textp;
            uint16_t      originp;
            uint16_t      lflags;
            uint16_t      flags1;
            uint8_t       flags2;
            uint8_t       split;
            uint16_t      spare;
        } version2;
        /** Structure for version 3 message bases. */
        struct
        {
            uint32_t      delim;          // $feffffff
            uint16_t      ltrsiz;
            uint32_t      arrtim;
            uint8_t       area;
            uint8_t       chrset;
            uint16_t      destzone;
            uint16_t      destnet;
            uint16_t      destnode;
            uint16_t      destpoint;
            uint16_t      origzone;
            uint16_t      orignet;
            uint16_t      orignode;
            uint16_t      origpoint;
            uint32_t      ltrtim;
            uint16_t      fromp;
            uint16_t      subjp;
            uint16_t      ltridp;
            uint32_t      qltrid;
            uint16_t      replyp;
            uint32_t      qreply;
            uint16_t      textp;
            uint16_t      originp;
            uint16_t      lflags;
            uint16_t      flags1;
            uint8_t       flags2;
            uint8_t       split;          // Total size: 58 byte
            uint8_t       spare[64-58];   // Fill to 64 bytes
        } version3;
    };

    /** Structure for the letter trailer in the area file. */
    union ltrtrl_u
    {
        /** Structure for version 2 message bases. */
        struct
        {
            uint16_t      ltrsiz;
            uint16_t      ltrnum;
        } version2;
        /** Structure for version 3 message bases. */
        struct
        {
            uint16_t      ltrsiz;
            uint32_t      ltrnum;
        } version3;
    };

    /** Structure for the flag files (.F??). */
    struct flags_s
    {
        uint8_t       bits;
        uint8_t       number;
        uint8_t       rdrpy;
    };

    /** Indicator for version 2 message bases. */
    static const uint8_t  Mypoint_msgbaseversion2 = 2;
    /** Indicator for version 3 message bases. */
    static const uint8_t  Mypoint_msgbaseversion3 = 3;
    static const uint32_t Mypoint_delimeter       = 0xfeffffff;
    static const uint8_t  Mypoint_delete          = 0x40;
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
