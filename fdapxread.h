// Copyright (c) 1999-2001 Peter Karlsson
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

#ifndef __FDAPXREAD_H
#define __FDAPXREAD_H

#include <config.h>

#include "datatypes.h"
#include "arearead.h"

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

class StatEngine;

/**
 * Class that reads FDAPX/w message bases.
 * This class reads message bases stored in FDAPX/w format, revision 3.
 */
class FdApxRead : public AreaRead
{
public:
    /** Standard constructor.
     * Creates the FDAPX/w message base reader object.
     * @param path    Directory for message base.
     * @param areanum Number of area to read.
     */
    FdApxRead(const char *path, unsigned areanum);
    /** Standard destructor. */
    virtual ~FdApxRead();

    /**
     * Transfer function. 
     * This function transfers all messages in the message bases, received
     * after the specified starting date, to the specified statistics engine.
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
        uint16_t  zone, net, node, point;
    };

    /** Structure of header for each message (msghdr.apx). */
    struct msghdrapx_s
    {
        uint8_t     recipientname[36];
        uint8_t     sendername[36];
        uint8_t     subject[72];
        le_uint32_t msgidcrc;
        le_uint32_t replycrc;
        le_uint16_t replyto;
        le_uint16_t replynext;
        le_uint16_t reply1st;
        le_uint32_t timewritten;
        le_uint32_t timesentrcvd;
        addr_s      recipientaddress;
        addr_s      senderaddress;
        le_uint32_t statusflags;
        le_uint32_t txtpos;
        le_uint16_t txtsize;
        le_uint16_t foldernumber;
        le_uint16_t foldernumberreplyto;
        le_uint16_t foldernumberreplynext;
        le_uint16_t foldernumberreply1st;
    };

    static const uint32_t Fdapx_local   = 0x01;
    static const uint32_t Fdapx_sent    = 0x10;
    static const uint32_t Fdapx_deleted = 0x80000000;

    /** Structure for the area data file (msgstat.apx). */
    struct msgstatapx_s
    {
        le_uint16_t msgbaseversion;
        uint8_t     reserved[254];
        le_uint16_t totalmsgs[2000];
        le_uint16_t highestmsg[2000];
        le_uint16_t lastreadptrs[2000];
    };

    /** Version of FDAPX/w message bases supported. */
    static const uint32_t Fdapx_msgbaseversion = 3;
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
