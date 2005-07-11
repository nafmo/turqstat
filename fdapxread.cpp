// Copyright (c) 1999-2005 Peter Karlsson
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

#include <config.h>
#include <string>
#ifdef HAVE_IOSTREAM
# include <iostream>
#else
# include <iostream.h>
#endif
#include <time.h>
#include <stdio.h>

#include "fdapxread.h"
#include "utility.h"
#include "statengine.h"
#include "output.h"

FdApxRead::FdApxRead(const char *path, unsigned areanum)
{
    areapath = strdup(path);
    areanumber = areanum;
}

FdApxRead::~FdApxRead()
{
    if (areapath) free(areapath);
}

bool FdApxRead::Transfer(time_t starttime, time_t endtime,
                          StatEngine &destination)
{
    // Get the output object
    TDisplay *display = TDisplay::GetOutputObject();

    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        display->InternalErrorQuit(TDisplay::area_not_allocated, 1);
    }

    // Check that the folder number is valid
    if (areanumber < 1 || areanumber > 1999)
    {
        display->ErrorMessage(TDisplay::area_out_of_range, 1999);
        return false;
    }

    // Open the message area files
    // msgstat.apx - contains a record of area info
    string basepath = string(areapath);
#ifdef BACKSLASH_PATHS
    if (basepath[basepath.length() - 1] != '/' &&
        basepath[basepath.length() - 1] != '\\')
        basepath += '\\';
#else
    if (basepath[basepath.length() - 1] != '/')
        basepath += '/';
#endif

    string filepath = basepath + "msgstat.apx";
    FILE *msgstat = fopen(filepath.c_str(), "rb");
    if (!msgstat)
    {
        display->ErrorMessage(TDisplay::cannot_open, filepath);
        return false;
    }

    msgstatapx_s msgstatapx;
    if (1 != fread(&msgstatapx, sizeof (msgstatapx_s), 1, msgstat))
    {
        display->ErrorMessage(TDisplay::cannot_open, filepath);
        return false;
    }
    fclose(msgstat);

    if (Fdapx_msgbaseversion != msgstatapx.msgbaseversion)
    {
        display->ErrorMessage(TDisplay::illegal_fdapx_version);
        return false;
    }

    filepath = basepath + "msghdr.apx";
    FILE *msghdr = fopen(filepath.c_str(), "rb");
    if (!msghdr)
    {
        display->ErrorMessage(TDisplay::cannot_open, filepath);
        return false;
    }

    filepath = basepath + "msgtxt.apx";
    FILE *msgtxt = fopen(filepath.c_str(), "rb");
    if (!msgstat)
    {
        display->ErrorMessage(TDisplay::cannot_open, filepath);
        return false;
    }

    // Read messages one by one
    bool stay = true;
    msghdrapx_s msghdrapx;
    unsigned msgnum = 0, high = msgstatapx.totalmsgs[areanumber - 1];
    char *buf, *ctrlbuf;

    display->SetMessagesTotal(high);

    if (0 == high) stay = false;
    while (stay)
    {
        if (1 != fread(&msghdrapx, sizeof (msghdrapx), 1, msghdr))
        {
            // Nothing more to read, we are done
            stay = false;
            break;
        }

        if  (msghdrapx.foldernumber == areanumber &&
             0 == (msghdrapx.statusflags & Fdapx_deleted))
        {
            // This is message is in the correct folder
            msgnum ++;
            display->UpdateProgress(msgnum);
            if (high == msgnum) stay = false;

            // Check if message is outside time range
            if (msghdrapx.statusflags & Fdapx_local)
            {
                if (time_t(msghdrapx.timewritten) < starttime ||
                    time_t(msghdrapx.timewritten) > endtime)
                    goto out;
            }
            else if (time_t(msghdrapx.timewritten) < starttime ||
                     time_t(msghdrapx.timewritten) > endtime)
                goto out;

            // Retrieve message body (includes kludges)
            fseek(msgtxt, msghdrapx.txtpos, SEEK_SET);
            buf = new char[msghdrapx.txtsize + 1];
            if (!buf)
            {
                display->WarningMessage(TDisplay::cannot_allocate_body,
                                        msgnum);
                goto out;
            }

            fread(buf, msghdrapx.txtsize, 1, msgtxt);
            buf[msghdrapx.txtsize] = 0;

            ctrlbuf = new char[msghdrapx.txtsize + 1];
            if (!ctrlbuf)
            {
                display->WarningMessage(TDisplay::cannot_allocate_control,
                                        msgnum);
            }

            // Separate kludges from text
            fixupctrlbuffer(buf, ctrlbuf);

            destination.AddData(string((char *) msghdrapx.sendername),
                                string((char *) msghdrapx.recipientname),
                                string((char *) msghdrapx.subject),
                                string(ctrlbuf), string(buf),
                                msghdrapx.timewritten,
                                (msghdrapx.statusflags & Fdapx_local)
                                    ? msghdrapx.timewritten
                                    : msghdrapx.timesentrcvd);

            delete[] buf;
            delete[] ctrlbuf;
        out:;
        }
    }

    fclose(msgtxt);
    fclose(msghdr);

    return true;
}
