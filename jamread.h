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

#ifndef __JAMREAD_H
#define __JAMREAD_H

#include <string>

#include "arearead.h"
#include "statengine.h"
#include "datatypes.h"

#ifdef __EMX__
# pragma pack(1)
#endif

class JamRead : public AreaRead
{
public:
    // Constructor and destructor
    JamRead(const char *path);
    virtual ~JamRead();

    // Transfer function
    virtual bool Transfer(time_t starttime, StatEngine &destination);

protected:
    char        *areapath;

    // .JHR
    struct jamhdr_header_s
    {
        UINT32  signature;      // 'JAM\0'
        UINT32  datecreated;
        UINT32  modcounter;
        UINT32  activemsgs;
        UINT32  passwordcrc;
        UINT32  basemsgnum;
        UINT8   _reserved[1000];
    };

    struct jamhdr_msg_s
    {
        UINT32  signature;      // 'JAM\0'
        UINT16  revision;
        UINT16  _reserved;
        UINT32  subfieldlen;
        UINT32  timesread;
        UINT32  msgidcrc;
        UINT32  replycrc;
        UINT32  replyto;
        UINT32  reply1st;
        UINT32  replynext;
        UINT32  datewritten;
        UINT32  datereceived;
        UINT32  dateprocessed;
        UINT32  messagenumber;
        UINT32  attribute;
        UINT32  _attribute2;
        UINT32  offset;
        UINT32  txtlen;
        UINT32  passwordcrc;
        UINT32  cost;
    };

    const Jam_deleted = 0x80000000L;

    struct jamhdr_subhdr_s
    {
        UINT16  loid;
        UINT16  _hiid;
        UINT32  datlen;
     /* UINT8   buffer[datlen]; */
    };

    const Jam_sender      = 2;
    const Jam_recipient   = 3;
    const Jam_MSGID       = 4;
    const Jam_subject     = 6;
    const Jam_PID         = 7;

    // *.JDX
    struct jamjdx_s
    {
        UINT32  recipientcrc;
        UINT32  jhroffset;
    };

    const Jam_msgbaseversion = 1;
};

#endif
