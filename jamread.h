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

/**
 * Class that reads JAM message bases.
 * This class reads message bases stored in the JAM format.
 */
class JamRead : public AreaRead
{
public:
    /** Standard constructor.
     * Creates the JAM message base reader object.
     * @param path Directory and base name for message base.
     */
    JamRead(const char *path);
    /** Standard destructor. */
    virtual ~JamRead();

    /**
     * Transfer function. 
     * This function transfers all messages in the message bases, received
     * after the specified starting date, to the specified statistics engine.
     *
     * @param starttime   Date to start retrieve statistcs from.
     * @param destination Engine object to transfer data to.
     * @return True if the message base was read correctly (even if no
     *         messages fits the condition.
     */
    virtual bool Transfer(time_t starttime, StatEngine &destination);

protected:
    /** Path to message base. */
    char        *areapath;

    /** Structure of the header in the JHR file. */
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

    /** Structure of the JHR entry for individual messages. */
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

    /** JAM attribute denoting a deleted message. */
    static const uint32_t Jam_deleted = 0x80000000L;

    /** Common data fields for the JHR message subheaders. */
    struct jamhdr_subhdr_s
    {
        uint16_t  loid;
        uint16_t  _hiid;
        uint32_t  datlen;
     /* uint8_t   buffer[datlen]; */
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

    /** Structure of the JDX entry JDX for individual messages. */
    struct jamjdx_s
    {
        uint32_t  recipientcrc;
        uint32_t  jhroffset;
    };

    /** Version of JAM message bases supported. */
    static const uint16_t Jam_msgbaseversion = 1;
    /** Magic number for JAM. */
    static const uint8_t  Jam_signature[4];
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
