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

#ifndef __TANSTAAFLREAD_H
#define __TANSTAAFLREAD_H

#include <string>

#include "arearead.h"
#include "statengine.h"
#include "datatypes.h"

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

class TanstaaflRead : public AreaRead
{
public:
    // Constructor and destructor
    TanstaaflRead(const char *path, unsigned areanum);
    virtual ~TanstaaflRead();

    // Transfer function
    virtual bool Transfer(time_t starttime, StatEngine &destination);

protected:
    char        *areapath;
    unsigned    areanumber;

    struct addr_s
    {
        UINT16  zone, net, node, point;
    };

    struct msghdrtfl_s /* msghdr.tfl */
    {
        char    recipientname[36];
        char    sendername[36];
        char    subject[72];
        UINT32  msgidcrc;
        UINT32  replycrc;
        UINT32  replyto;
        UINT32  replynext;
        UINT32  replyprev;
        UINT32  timewritten;
        UINT32  replychild;
        addr_s  recipientaddress;
        addr_s  senderaddress;
        UINT32  statusflags;
        UINT32  txtpos;
        UINT32  txtsize;
        UINT16  foldernumber;
        UINT16  foldernumberreplyto;
        UINT16  foldernumberreplynext;
        UINT16  foldernumberreplyprev;
    };

    static const UINT32 Tanstaafl_local   = 0x01;
    static const UINT32 Tanstaafl_sent    = 0x10;
    static const UINT32 Tanstaafl_deleted = 0x80000000;

    struct msgstattfl_s /* msgstat.tfl */
    {
        UINT16  msgbaseversion;
        UINT8   reserved[254];
        UINT32  totalmsgs[2000];
        UINT32  lastreadptrs[2000];
        UINT32  unread[2000];
    };

    static const UINT32 Tanstaafl_msgbaseversion = 1;
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
