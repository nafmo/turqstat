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

#ifndef __SDMREAD_H
#define __SDMREAD_H

#include <config.h>

#include "datatypes.h"
#include "arearead.h"
#include "utility.h"

class StatEngine;

/**
 * Class that reads "star-dot-MSG" message bases.
 * This class reads message bases stored in standard Fidonet *.MSG format,
 * either in standard FTSC format (for instance TerMail), or in the
 * Opus variant (used by, for instance, SquishMail).
 */
class SdmRead : public AreaRead
{
public:
    /** Standard constructor.
     * Creates the Squish message base reader object.
     * @param path Directory and base name for message base.
     * @param hasarrivaltime True if it is an Opus style message base, which
     *                       stores arrival times.
     */
    SdmRead(const char *path, bool hasarrivetime);
    /** Standard destructor. */
    virtual ~SdmRead();

    /**
     * Transfer function. 
     * This function transfers all messages in the message bases, received
     * (for Opus style areas) or written (for FTSC style areas) after the
     * specified starting date, to the specified statistics engine.
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
    char    *areapath;
    /** Flag telling whether this is an Opus or FTSC style message base. */
    bool    isopus;

    /** Structure of the header of the MSG file. */
    struct sdmhead_s
    {
        uint8_t     fromusername[36];
        uint8_t     tousername[36];
        uint8_t     subject[72];
        uint8_t     datetime[20];   // "Dd Mmm Yy  HH:MM:SS"
                                    // "Www Dd Mmm Yy HH:MM"
        le_uint16_t timesread;
        le_uint16_t destnode;
        le_uint16_t orignode;
        le_uint16_t cost;
        le_uint16_t orignet;
        le_uint16_t destnet;
        union
        {
            /** FTSC style MSG structure. */
            struct
            {
                le_uint16_t  destzone;
                le_uint16_t  origzone;
                le_uint16_t  destpoint;
                le_uint16_t  origpoint;
            } ftsc;
            /** Opus style MSG structure. */
            struct
            {
                stamp_s written;
                stamp_s arrived;
            } opus;
        };
        le_uint16_t  replyto;
        le_uint16_t  attribute;
        le_uint16_t  nextreply;
    };
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
