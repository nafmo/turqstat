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

#include "arearead.h"
#include "datatypes.h"
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
        UINT8   fromusername[36];
        UINT8   tousername[36];
        UINT8   subject[72];
        UINT8   datetime[20];       // "Dd Mmm Yy  HH:MM:SS"
                                    // "Www Dd Mmm Yy HH:MM"
        UINT16  timesread;
        UINT16  destnode;
        UINT16  orignode;
        UINT16  cost;
        UINT16  orignet;
        UINT16  destnet;
        union
        {
            struct // FTSC style
            {
                UINT16  destzone;
                UINT16  origzone;
                UINT16  destpoint;
                UINT16  origpoint;
            } ftsc;
            struct // Opus style
            {
                stamp_s written;
                stamp_s arrived;
            } opus;
        };
        UINT16  replyto;
        UINT16  attribute;
        UINT16  nextreply;
    };
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
