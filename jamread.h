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

#ifndef __JAMREAD_H
#define __JAMREAD_H

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
        uint8_t   signature[4];   // 'JAM\0'
        uint32_t  datecreated;
        uint32_t  modcounter;
        uint32_t  activemsgs;
        uint32_t  passwordcrc;
        uint32_t  basemsgnum;
        uint8_t   _reserved[1000];
    };

    struct jamhdr_msg_s
    {
        uint8_t   signature[4];   // 'JAM\0'
        uint16_t  revision;
        uint16_t  _reserved;
        uint32_t  subfieldlen;
        uint32_t  timesread;
        uint32_t  msgidcrc;
        uint32_t  replycrc;
        uint32_t  replyto;
        uint32_t  reply1st;
        uint32_t  replynext;
        uint32_t  datewritten;
        uint32_t  datereceived;
        uint32_t  dateprocessed;
        uint32_t  messagenumber;
        uint32_t  attribute;
        uint32_t  _attribute2;
        uint32_t  offset;
        uint32_t  txtlen;
        uint32_t  passwordcrc;
        uint32_t  cost;
    };

    static const uint32_t Jam_deleted = 0x80000000L;

    struct jamhdr_subhdr_s
    {
        uint16_t  loid;
        uint16_t  _hiid;
        uint32_t  datlen;
     /* uint8_t   buffer[datlen]; */
    };

    static const uint16_t Jam_sender      = 2;
    static const uint16_t Jam_recipient   = 3;
    static const uint16_t Jam_MSGID       = 4;
    static const uint16_t Jam_subject     = 6;
    static const uint16_t Jam_PID         = 7;

    // *.JDX
    struct jamjdx_s
    {
        uint32_t  recipientcrc;
        uint32_t  jhroffset;
    };

    static const uint16_t Jam_msgbaseversion = 1;
    static const uint8_t  Jam_signature[4];//= "JAM";
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
