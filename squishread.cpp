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

#include <iostream.h>
#include <time.h>
#include <string.h>
extern "C" {
#include <msgapi.h>
}
#include "squishread.h"

static time_t stampToTimeT(struct _stamp *st);

SquishRead::SquishRead(const char *path)
{
    areapath = strdup(path);
}

SquishRead::~SquishRead()
{
    if (areapath) delete areapath;
}

bool SquishRead::Transfer(time_t starttime, StatEngine &destination)
{
    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        cerr << "Internal error: Area path was not allocated properly" << endl;
        return false;
    }

    // Open the MSGAPI
    struct _minf mi;
    memset(&mi, 0, sizeof(mi));
    //mi.def_zone = 2;
    MsgOpenApi(&mi);

    // Open the area
    HAREA areahandle;
    areahandle = MsgOpenArea((unsigned char *) areapath, MSGAREA_NORMAL,
                             MSGTYPE_SQUISH);
    if (!areahandle)
    {
        cerr << "Cannot open area " << areapath << endl;
        return false;
    }

    // Lock area
    MsgLock(areahandle);

    // Read messages
    HMSG msghandle;
    XMSG msg;
    dword msgn, ctrllen, msglen, high = MsgHighMsg(areahandle);
    char *ctrlbuf, *msgbuf;

    for (msgn = 1L; msgn <= high; msgn ++)
    {
        msghandle = MsgOpenMsg(areahandle, MOPEN_READ, msgn);
        if (msghandle)
        {
            // Opening message succeeded
            ctrllen = MsgGetCtrlLen(msghandle);
            ctrlbuf = new char[ctrllen];
            if (!ctrlbuf)
            {
                cerr << "Unable to allocate memory for control data (msg #"
                     << msgn << ')' << endl;
                goto out1;
            }

            msglen = MsgGetTextLen(msghandle);
            msgbuf = new char[msglen];
            if (!msgbuf)
            {
                cerr << "Unable to allocate memory for message body (msg #"
                     << msgn << ')' << endl;
                goto out2;
            }

            MsgReadMsg(msghandle, &msg, 0L, msglen, (unsigned char *) msgbuf,
                       ctrllen, (unsigned char *) ctrlbuf);

            // Add to statistics
            if (stampToTimeT(&msg.date_arrived) >= starttime)
            {
                destination.AddData(string((char *) msg.from),
                                    string((char *) msg.to),
                                    string((char *) msg.subj),
                                    string(ctrlbuf), string(msgbuf),
                                    stampToTimeT(&msg.date_written),
                                    stampToTimeT(&msg.date_arrived));
            }

            // Clean up our mess

            delete msgbuf;
out2:;
            delete ctrlbuf;
out1:;
            MsgCloseMsg(msghandle);
        }

        cout << 100 * msgn / high << "% done\r";
    }

    // Close area and API
    MsgCloseArea(areahandle);
    MsgCloseApi();

    return true;
}

// The following is from MsgEd 4.30:
//  Written on 10-Jul-94 by John Dennis and released to the public domain.

static time_t stampToTimeT(struct _stamp *st)
{
    time_t tt;
    struct tm tms;
    if (0 == st->date.da || 0 == st->date.mo)
    {
        return 0;
    }
    tms.tm_sec = st->time.ss << 1;
    tms.tm_min = st->time.mm;
    tms.tm_hour = st->time.hh;
    tms.tm_mday = st->date.da;
    tms.tm_mon = st->date.mo - 1;
    tms.tm_year = st->date.yr + 80;
    tms.tm_isdst = -1;
    tt = mktime(&tms);
    return tt;
}
