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

#ifndef __TANSTAAFLREAD_H
#define __TANSTAAFLREAD_H

#include <config.h>

#include "datatypes.h"
#include "arearead.h"

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

class StatEngine;

/**
 * Class that reads Tanstaafl message bases.
 * This class reads message bases stored in Tanstaafl (There's No Such
 * Thing As A Free Lunch, a GPL'ed Fidonet reader for MS-DOS and Linux)
 * format. The Tanstaafl mesasge base format is based on the one used by
 * FDAPX/w.
 */
class TanstaaflRead : public AreaRead
{
public:
    /** Standard constructor.
     * Creates the Tanstaafl message base reader object.
     * @param path    Directory for message base.
     * @param areanum Number of area to read.
     */
    TanstaaflRead(const char *path, unsigned areanum);
    /** Standard destructor. */
    virtual ~TanstaaflRead();

    /**
     * Transfer function. 
     * This function transfers all messages in the message bases, written
     * after the specified starting date, to the specified statistics engine.
     * Tanstaafl does not store reception dates, so we cannot use that as a
     * reference.
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

    /** Structure for a Fidonet address entry. */
    struct addr_s
    {
        le_uint16_t  zone, net, node, point;
    };

    /** Structure of header for each message (msghdr.tfl). */
    struct msghdrtfl_s
    {
        uint8_t     recipientname[36];
        uint8_t     sendername[36];
        uint8_t     subject[72];
        le_uint32_t msgidcrc;
        le_uint32_t replycrc;
        le_uint32_t replyto;
        le_uint32_t replynext;
        le_uint32_t replyprev;
        le_uint32_t timewritten;
        le_uint32_t replychild;
        addr_s      recipientaddress;
        addr_s      senderaddress;
        le_uint32_t statusflags;
        le_uint32_t txtpos;
        le_uint32_t txtsize;
        le_uint16_t foldernumber;
        le_uint16_t foldernumberreplyto;
        le_uint16_t foldernumberreplynext;
        le_uint16_t foldernumberreplyprev;
    };

    static const uint32_t Tanstaafl_local   = 0x01;
    static const uint32_t Tanstaafl_sent    = 0x10;
    static const uint32_t Tanstaafl_deleted = 0x80000000;

    /** Structure for the area data file (msgstat.tfl). */
    struct msgstattfl_s
    {
        le_uint16_t msgbaseversion;
        uint8_t     reserved[254];
        le_uint32_t totalmsgs[2000];
        le_uint32_t lastreadptrs[2000];
        le_uint32_t unread[2000];
    };

    /** Version of Tanstaafl message bases supported. */
    static const uint32_t Tanstaafl_msgbaseversion = 1;
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
