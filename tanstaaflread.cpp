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

#include <string>
#include <iostream.h>
#include <time.h>
#include <stdio.h>

#include "tanstaaflread.h"
#include "utility.h"

#if defined(UNIX)
# define DIRSEP '/'
#else
# define DIRSEP '\\'
#endif

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
        cerr << "Internal error: Area path was not allocated properly" << endl;
        return false;
    }

    // Tanstaafl format lacks arrival times
    destination.NoArrivalTime();

    // Check that the folder number is valid
    if (areanumber < 1 || areanumber > 1999)
    {
        cerr << "Error: Invalid area number choosen (must be between 1-1999)"
             << endl;
        return false;
    }

    // Open the message area files
    // msgstat.tfl - contains a record of area info
    string basepath = string(areapath);
    if (DIRSEP != basepath[basepath.length() - 1])
        basepath += DIRSEP;

    string filepath = basepath + "msgstat.tfl";
    FILE *msgstat = fopen(filepath.c_str(), "rb");
    if (!msgstat)
    {
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }

    msgstattfl_s msgstattfl;
    if (1 != fread(&msgstattfl, sizeof (msgstattfl_s), 1, msgstat))
    {
        cerr << "Error: Couldn't read from " << filepath << endl;
        return false;
    }
    fclose(msgstat);

    if (Tanstaafl_msgbaseversion != msgstattfl.msgbaseversion)
    {
        cerr << "Error: Illegal tanstaafl message base version" << endl;
        return false;
    }

    filepath = basepath + "msghdr.tfl";
    FILE *msghdr = fopen(filepath.c_str(), "rb");
    if (!msghdr)
    {
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }


    filepath = basepath + "msgtxt.tfl";
    FILE *msgtxt = fopen(filepath.c_str(), "rb");
    if (!msgstat)
    {
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }

    // Read messages one by one
    bool stay = true;
    msghdrtfl_s msghdrtfl;
    unsigned msgnum = 0, high = msgstattfl.totalmsgs[areanumber - 1];
    char *buf, *ctrlbuf;
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
            cout << 100 * msgnum / high << "% done\r";
            if (high == msgnum) stay = false;

            // Check if message is too old
            if ((time_t) msghdrtfl.timewritten < starttime)
                goto out;

            // Retrieve message body (includes kludges)
            fseek(msgtxt, msghdrtfl.txtpos, SEEK_SET);
            buf = new char[msghdrtfl.txtsize + 1];
            if (!buf)
            {
                cerr << "Unable to allocate memory for message body (msg #"
                     << msgnum << ')' << endl;
                goto out;
            }

            fread(buf, msghdrtfl.txtsize, 1, msgtxt);
            buf[msghdrtfl.txtsize] = 0;

            ctrlbuf = new char[msghdrtfl.txtsize + 1];
            if (!ctrlbuf)
            {
                cerr << "Unable to allocate memory for control data (msg #"
                     << msgnum << ')' << endl;
            }

            // Separate kludges from text
            fixupctrlbuffer(buf, ctrlbuf);

            destination.AddData(string(msghdrtfl.sendername),
                                string(msghdrtfl.recipientname),
                                string(msghdrtfl.subject),
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
