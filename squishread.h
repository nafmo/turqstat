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

#ifndef __SQUISHREAD_H
#define __SQUISHREAD_H

#include <config.h>
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
# include "datatypes.h"
#endif

#include "arearead.h"
#include "utility.h"

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

class StatEngine;

class SquishRead : public AreaRead
{
public:
    // Constructor and destructor
    SquishRead(const char *path);
    virtual ~SquishRead();

    // Transfer function
    virtual bool Transfer(time_t starttime, StatEngine &destination);

protected:
    char    *areapath;

    // .SQD
    struct sqbase_s
    {
        uint16_t  len;
        uint16_t  reserved;
        uint32_t  num_msg;
        uint32_t  high_msg;
        uint32_t  skip_msg;
        uint32_t  high_water;
        uint32_t  uid;
        uint8_t   base[80];
        uint32_t  begin_frame;
        uint32_t  last_frame;
        uint32_t  free_frame;
        uint32_t  last_free_frame;
        uint32_t  end_frame;
        uint32_t  max_msg;
        uint16_t  keep_days;
        uint16_t  sz_sqhdr;
        uint8_t   reserved2[124];
    };

    struct sqhdr_s
    {
        uint32_t  id;     //0xAFAE4453
        uint32_t  next_frame;
        uint32_t  prev_frame;
        uint32_t  frame_length;
        uint32_t  msg_length; //-sizeof(xmsg_s)-clen
        uint32_t  clen;
        uint16_t  frame_type;
        uint16_t  reserved;
    };

    static const uint32_t Squish_id = 0xAFAE4453;
    static const uint16_t Squish_type_normal = 0;
    static const uint16_t Squish_type_free = 1;

    struct xmsg_s
    {
        uint32_t  attr;
        uint8_t   from[36];
        uint8_t   to[36];
        uint8_t   subject[72];
        uint16_t  origzone;
        uint16_t  orignet;
        uint16_t  orignode;
        uint16_t  origpoint;
        uint16_t  destzone;
        uint16_t  destnet;
        uint16_t  destnode;
        uint16_t  destpoint;
        stamp_s   date_written;
        stamp_s   date_arrived;
        int16_t   utc_ofs;
        uint32_t  replyto;
        uint32_t  replies[9];
        uint32_t  umsgid;
        uint8_t   ftsc_date[20];
    };
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
