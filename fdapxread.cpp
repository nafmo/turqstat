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

#include "fdapxread.h"
#include "utility.h"
#include "statengine.h"

FdApxRead::FdApxRead(const char *path, unsigned areanum)
{
    areapath = strdup(path);
    areanumber = areanum;
}

FdApxRead::~FdApxRead()
{
    if (areapath) delete areapath;
}

bool FdApxRead::Transfer(time_t starttime, StatEngine &destination)
{
    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        internalerrorquit(area_not_allocated, 1);
    }

    // Check that the folder number is valid
    if (areanumber < 1 || areanumber > 1999)
    {
        cerr << "Error: Invalid area number choosen (must be between 1-1999)"
             << endl;
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
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }

    msgstatapx_s msgstatapx;
    if (1 != fread(&msgstatapx, sizeof (msgstatapx_s), 1, msgstat))
    {
        cerr << "Error: Couldn't read from " << filepath << endl;
        return false;
    }
    fclose(msgstat);

    if (Fdapx_msgbaseversion != msgstatapx.msgbaseversion)
    {
        cerr << "Error: Illegal FDAPX/w message base version" << endl;
        return false;
    }

    filepath = basepath + "msghdr.apx";
    FILE *msghdr = fopen(filepath.c_str(), "rb");
    if (!msghdr)
    {
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }


    filepath = basepath + "msgtxt.apx";
    FILE *msgtxt = fopen(filepath.c_str(), "rb");
    if (!msgstat)
    {
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }

    // Read messages one by one
    bool stay = true;
    msghdrapx_s msghdrapx;
    unsigned msgnum = 0, high = msgstatapx.totalmsgs[areanumber - 1];
    char *buf, *ctrlbuf;

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
            cout << 100 * msgnum / high << "% done\r";
            if (high == msgnum) stay = false;

            // Check if message is too old
            if (msghdrapx.statusflags & Fdapx_local)
            {
                if ((time_t) msghdrapx.timewritten < starttime)
                    goto out;
            }
            else if ((time_t) msghdrapx.timesentrcvd < starttime)
                goto out;

            // Retrieve message body (includes kludges)
            fseek(msgtxt, msghdrapx.txtpos, SEEK_SET);
            buf = new char[msghdrapx.txtsize + 1];
            if (!buf)
            {
                cerr << "Unable to allocate memory for message body (msg #"
                     << msgnum << ')' << endl;
                goto out;
            }

            fread(buf, msghdrapx.txtsize, 1, msgtxt);
            buf[msghdrapx.txtsize] = 0;

            ctrlbuf = new char[msghdrapx.txtsize + 1];
            if (!ctrlbuf)
            {
                cerr << "Unable to allocate memory for control data (msg #"
                     << msgnum << ')' << endl;
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

            delete buf;
            delete ctrlbuf;
        out:;
        }
    }

    fclose(msgtxt);
    fclose(msghdr);

    return true;
}
