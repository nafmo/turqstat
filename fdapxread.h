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

#ifndef __FDAPXREAD_H
#define __FDAPXREAD_H

#include <string>

#include "arearead.h"
#include "statengine.h"
#include "datatypes.h"

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

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
        UINT16  zone, net, node, point;
    };

    struct msghdrapx_s /* MSGHDR.APX */
    {
        char    recipientname[36];
        char    sendername[36];
        char    subject[72];
        UINT32  msgidcrc;
        UINT32  replycrc;
        UINT16  replyto;
        UINT16  replynext;
        UINT16  reply1st;
        UINT32  timewritten;
        UINT32  timesentrcvd;
        addr_s  recipientaddress;
        addr_s  senderaddress;
        UINT32  statusflags;
        UINT32  txtpos;
        UINT16  txtsize;
        UINT16  foldernumber;
        UINT16  foldernumberreplyto;
        UINT16  foldernumberreplynext;
        UINT16  foldernumberreply1st;
    };

    static const UINT32 Fdapx_local   = 0x01;
    static const UINT32 Fdapx_sent    = 0x10;
    static const UINT32 Fdapx_deleted = 0x80000000;

    struct msgstatapx_s /* MSGSTAT.APX */
    {
        UINT16  msgbaseversion;
        UINT8   reserved[254];
        UINT16  totalmsgs[2000];
        UINT16  highestmsg[2000];
        UINT16  lastreadptrs[2000];
    };

    static const UINT32 Fdapx_msgbaseversion = 3;
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
