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

#ifndef __FDAPXREAD_H
#define __FDAPXREAD_H

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

class FdApxRead : public AreaRead
{
public:
    // Constructor and destructor
    FdApxRead(const char *path, unsigned areanum);
    virtual ~FdApxRead();

    // Transfer function
    virtual bool Transfer(time_t starttime, StatEngine &destination);

protected:
    char        *areapath;
    unsigned    areanumber;

    struct addr_s
    {
        uint16_t  zone, net, node, point;
    };

    struct msghdrapx_s /* MSGHDR.APX */
    {
        uint8_t   recipientname[36];
        uint8_t   sendername[36];
        uint8_t   subject[72];
        uint32_t  msgidcrc;
        uint32_t  replycrc;
        uint16_t  replyto;
        uint16_t  replynext;
        uint16_t  reply1st;
        uint32_t  timewritten;
        uint32_t  timesentrcvd;
        addr_s    recipientaddress;
        addr_s    senderaddress;
        uint32_t  statusflags;
        uint32_t  txtpos;
        uint16_t  txtsize;
        uint16_t  foldernumber;
        uint16_t  foldernumberreplyto;
        uint16_t  foldernumberreplynext;
        uint16_t  foldernumberreply1st;
    };

    static const uint32_t Fdapx_local   = 0x01;
    static const uint32_t Fdapx_sent    = 0x10;
    static const uint32_t Fdapx_deleted = 0x80000000;

    struct msgstatapx_s /* MSGSTAT.APX */
    {
        uint16_t  msgbaseversion;
        uint8_t   reserved[254];
        uint16_t  totalmsgs[2000];
        uint16_t  highestmsg[2000];
        uint16_t  lastreadptrs[2000];
    };

    static const uint32_t Fdapx_msgbaseversion = 3;
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
