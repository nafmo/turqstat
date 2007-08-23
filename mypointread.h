// Copyright (c) 1999-2001 Peter Karlsson
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

    /** Letter structure in the area file (.A??). */
    union ltrhdr_u
    {
        /** Structure for version 2 message bases. */
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
            le_uint16_t   spare;
        } version2;
        /** Structure for version 3 message bases. */
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

    /** Structure for the letter trailer in the area file. */
    union ltrtrl_u
    {
        /** Structure for version 2 message bases. */
        struct
        {
            le_uint16_t   ltrsiz;
            le_uint16_t   ltrnum;
        } version2;
        /** Structure for version 3 message bases. */
        struct
        {
            le_uint16_t   ltrsiz;
            le_uint32_t   ltrnum;
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
