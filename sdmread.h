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

#ifndef __SDMREAD_H
#define __SDMREAD_H

#include <config.h>
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
# include "datatypes.h"
#endif

#include "arearead.h"
#include "utility.h"

class StatEngine;

class SdmRead : public AreaRead
{
public:
    // Constructor and destructor
    SdmRead(const char *path, bool hasarrivetime);
    virtual ~SdmRead();

    // Transfer function
    virtual bool Transfer(time_t starttime, StatEngine &destination);

protected:
    char    *areapath;
    bool    isopus;

    struct sdmhead_s
    {
        uint8_t   fromusername[36];
        uint8_t   tousername[36];
        uint8_t   subject[72];
        uint8_t   datetime[20];       // "Dd Mmm Yy  HH:MM:SS"
                                    // "Www Dd Mmm Yy HH:MM"
        uint16_t  timesread;
        uint16_t  destnode;
        uint16_t  orignode;
        uint16_t  cost;
        uint16_t  orignet;
        uint16_t  destnet;
        union
        {
            struct // FTSC style
            {
                uint16_t  destzone;
                uint16_t  origzone;
                uint16_t  destpoint;
                uint16_t  origpoint;
            } ftsc;
            struct // Opus style
            {
                stamp_s written;
                stamp_s arrived;
            } opus;
        };
        uint16_t  replyto;
        uint16_t  attribute;
        uint16_t  nextreply;
    };
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
