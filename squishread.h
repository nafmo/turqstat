// Copyright (c) 1999 Peter Karlsson
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

#include <string>

#include "arearead.h"
#include "statengine.h"
#include "datatypes.h"
#include "utility.h"

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

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
        UINT16  len;
        UINT16  reserved;
        UINT32  num_msg;
        UINT32  high_msg;
        UINT32  skip_msg;
        UINT32  high_water;
        UINT32  uid;
        UINT8   base[80];
        UINT32  begin_frame;
        UINT32  last_frame;
        UINT32  free_frame;
        UINT32  last_free_frame;
        UINT32  end_frame;
        UINT32  max_msg;
        UINT16  keep_days;
        UINT16  sz_sqhdr;
        UINT8   reserved2[124];
    };

    struct sqhdr_s
    {
        UINT32  id;     //0xAFAE4453
        UINT32  next_frame;
        UINT32  prev_frame;
        UINT32  frame_length;
        UINT32  msg_length; //-sizeof(xmsg_s)-clen
        UINT32  clen;
        UINT16  frame_type;
        UINT16  reserved;
    };

    static const UINT32 Squish_id = 0xAFAE4453;
    static const UINT16 Squish_type_normal = 0;
    static const UINT16 Squish_type_free = 1;

    struct xmsg_s
    {
        UINT32  attr;
        UINT8   from[36];
        UINT8   to[36];
        UINT8   subject[72];
        UINT16  origzone;
        UINT16  orignet;
        UINT16  orignode;
        UINT16  origpoint;
        UINT16  destzone;
        UINT16  destnet;
        UINT16  destnode;
        UINT16  destpoint;
        stamp_s date_written;
        stamp_s date_arrived;
        SINT16  utc_ofs;
        UINT32  replyto;
        UINT32  replies[9];
        UINT32  umsgid;
        UINT8   ftsc_date[20];
    };
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
