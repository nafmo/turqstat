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

#ifndef __JAMREAD_H
#define __JAMREAD_H

#include <config.h>

#include "datatypes.h"
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
        uint8_t     signature[4];   // 'JAM\0'
        le_uint32_t datecreated;
        le_uint32_t modcounter;
        le_uint32_t activemsgs;
        le_uint32_t passwordcrc;
        le_uint32_t basemsgnum;
        uint8_t     _reserved[1000];
    };

    struct jamhdr_msg_s
    {
        uint8_t     signature[4];   // 'JAM\0'
        le_uint16_t revision;
        le_uint16_t _reserved;
        le_uint32_t subfieldlen;
        le_uint32_t timesread;
        le_uint32_t msgidcrc;
        le_uint32_t replycrc;
        le_uint32_t replyto;
        le_uint32_t reply1st;
        le_uint32_t replynext;
        le_uint32_t datewritten;
        le_uint32_t datereceived;
        le_uint32_t dateprocessed;
        le_uint32_t messagenumber;
        le_uint32_t attribute;
        le_uint32_t _attribute2;
        le_uint32_t offset;
        le_uint32_t txtlen;
        le_uint32_t passwordcrc;
        le_uint32_t cost;
    };

    static const uint32_t Jam_deleted = 0x80000000L;

    struct jamhdr_subhdr_s
    {
        le_uint16_t loid;
        le_uint16_t _hiid;
        le_uint32_t datlen;
     /* uint8_t     buffer[datlen]; */
    };

    static const uint16_t Jam_oaddress    = 0;      // Ignored
    static const uint16_t Jam_daddress    = 1;      // Ignored
    static const uint16_t Jam_sender      = 2;
    static const uint16_t Jam_recipient   = 3;
    static const uint16_t Jam_MSGID       = 4;
    static const uint16_t Jam_REPLY       = 5;      // Ignored
    static const uint16_t Jam_subject     = 6;
    static const uint16_t Jam_PID         = 7;
    static const uint16_t Jam_VIA         = 8;      // Ignored
    static const uint16_t Jam_fileattach  = 9;      // Ignored
    static const uint16_t Jam_filealias   = 10;     // Ignored
    static const uint16_t Jam_freq        = 11;     // Ignored
    static const uint16_t Jam_filewildcard= 12;     // Ignored
    static const uint16_t Jam_attachlist  = 13;     // Ignored
    static const uint16_t Jam_reserved    = 1000;   // Ignored
    static const uint16_t Jam_kludge      = 2000;
    static const uint16_t Jam_SEENBY      = 2001;   // Ignored
    static const uint16_t Jam_PATH        = 2002;   // Ignored
    static const uint16_t Jam_FLAGS       = 2003;   // Ignored
    static const uint16_t Jam_TZUTC       = 2004;   // Ignored

    // *.JDX
    struct jamjdx_s
    {
        le_uint32_t recipientcrc;
        le_uint32_t jhroffset;
    };

    static const uint16_t Jam_msgbaseversion = 1;
    static const uint8_t  Jam_signature[4];
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
