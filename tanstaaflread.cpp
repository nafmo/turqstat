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

#include <config.h>
#include <string>
#include <iostream.h>
#include <time.h>
#include <stdio.h>

#include "tanstaaflread.h"
#include "utility.h"
#include "statengine.h"
#include "output.h"

TanstaaflRead::TanstaaflRead(const char *path, unsigned areanum)
{
    areapath = strdup(path);
    areanumber = areanum;
}

TanstaaflRead::~TanstaaflRead()
{
    if (areapath) delete areapath;
}

bool TanstaaflRead::Transfer(time_t starttime, StatEngine &destination)
{
    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        internalerrorquit(area_not_allocated, 1);
    }

    // Get the output object
    ProgressDisplay *display = ProgressDisplay::GetOutputObject();

    // Tanstaafl format lacks arrival times
    destination.NoArrivalTime();

    // Check that the folder number is valid
    if (areanumber < 1 || areanumber > 1999)
    {
        display->ErrorMessage("Invalid area number chosen "
                              "(must be between 1-1999)");
        return false;
    }

    // Open the message area files
    // msgstat.tfl - contains a record of area info
    string basepath = string(areapath);
#ifdef BACKSLASH_PATHS
    if (basepath[basepath.length() - 1] != '/' &&
        basepath[basepath.length() - 1] != '\\')
        basepath += '\\';
#else
    if (basepath[basepath.length() - 1] != '/')
        basepath += '/';
#endif

    string filepath = basepath + "msgstat.tfl";
    FILE *msgstat = fopen(filepath.c_str(), "rb");
    if (!msgstat)
    {
        string msg = string("Cannot open ") + filepath;
        display->ErrorMessage(msg);
        return false;
    }

    msgstattfl_s msgstattfl;
    if (1 != fread(&msgstattfl, sizeof (msgstattfl_s), 1, msgstat))
    {
        string msg = string("Could not read from ") + filepath;
        display->ErrorMessage(msg);
        return false;
    }
    fclose(msgstat);

    // Current message base version is 1, but it is incorrectly written
    // as 0 (or rather, never initialized) in 0.0.4.
    if (Tanstaafl_msgbaseversion < msgstattfl.msgbaseversion)
    {
        display->ErrorMessage("Illegal tanstaafl message base version");
        return false;
    }

    if (Tanstaafl_msgbaseversion == 0)
    {
        display->WarningMessage("tanstaafl message base version is 0, "
                                "assuming 1");
    }

    filepath = basepath + "msghdr.tfl";
    FILE *msghdr = fopen(filepath.c_str(), "rb");
    if (!msghdr)
    {
        string msg = string("Cannot open ") + filepath;
        display->ErrorMessage(msg);
        return false;
    }

    filepath = basepath + "msgtxt.tfl";
    FILE *msgtxt = fopen(filepath.c_str(), "rb");
    if (!msgstat)
    {
        string msg = string("Cannot open ") + filepath;
        display->ErrorMessage(msg);
        return false;
    }

    // Read messages one by one
    bool stay = true;
    msghdrtfl_s msghdrtfl;
    unsigned msgnum = 0, high = msgstattfl.totalmsgs[areanumber - 1];
    char *buf, *ctrlbuf;

    display->SetMessagesTotal(high);

    if (0 == high) stay = false;
    while (stay)
    {
        if (1 != fread(&msghdrtfl, sizeof (msghdrtfl), 1, msghdr))
        {
            // Nothing more to read, we are done
            stay = false;
            break;
        }

        if  (msghdrtfl.foldernumber == areanumber &&
             0 == (msghdrtfl.statusflags & Tanstaafl_deleted))
        {
            // This is message is in the correct folder
            msgnum ++;
            display->UpdateProgress(msgnum);
            if (high == msgnum) stay = false;

            // Check if message is too old
            if ((time_t) msghdrtfl.timewritten < starttime)
                goto out;

            // Retrieve message body (includes kludges)
            fseek(msgtxt, msghdrtfl.txtpos, SEEK_SET);
            buf = new char[msghdrtfl.txtsize + 1];
            if (!buf)
            {
                display->WarningMessage("Unable to allocate memory for "
                                        "message body #", msgnum);
                goto out;
            }

            fread(buf, msghdrtfl.txtsize, 1, msgtxt);
            buf[msghdrtfl.txtsize] = 0;

            ctrlbuf = new char[msghdrtfl.txtsize + 1];
            if (!ctrlbuf)
            {
                display->WarningMessage("Unable to allocate memory for "
                                        "control data #", msgnum);
            }

            // Separate kludges from text
            fixupctrlbuffer(buf, ctrlbuf);

            destination.AddData(string((char *) msghdrtfl.sendername),
                                string((char *) msghdrtfl.recipientname),
                                string((char *) msghdrtfl.subject),
                                string(ctrlbuf), string(buf),
                                msghdrtfl.timewritten,
                                msghdrtfl.timewritten);

            delete buf;
            delete ctrlbuf;
        out:;
        }
    }

    fclose(msgtxt);
    fclose(msghdr);

    return true;
}
