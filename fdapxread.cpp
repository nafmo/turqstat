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

#include <string>
#include <iostream.h>
#include <time.h>
#include <stdio.h>

#include "fdapxread.h"

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
        cerr << "Internal error: Area path was not allocated properly" << endl;
        return false;
    }

    // Check that the folder number is valid
    if (areanumber < 1 || areanumber > 1999)
    {
        cerr << "Error: Invalid area number choosen (must be between 1-1999"
             << endl;
        return false;
    }

    // Open the message area files
    // msgstat.apx - contains a record of area info
    string filepath = string(areapath) + "\\msgstat.apx";
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

#if 0
    if (Fdapx_msgbaseversion != msgstatapx.msgbaseversion)
    {
        cerr << "Error: Illegal FDAPX/w message base version" << endl;
        return false;
    }
#endif

    filepath = string(areapath) + "\\msghdr.apx";
    FILE *msghdr = fopen(filepath.c_str(), "rb");
    if (!msghdr)
    {
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }


    filepath = string(areapath) + "\\msgtxt.apx";
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
    bool iskludge, wascr;
    char *ctrl_p, *body_p, *newbody_p, *buf, *ctrlbuf;
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

            // Check if message is too old
            if (msghdrapx.statusflags & Fdapx_local)
            {
                if (msghdrapx.timewritten < starttime)
                    goto out;
            }
            else if (msghdrapx.timesentrcvd < starttime)
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

            // Copy out kludges and body to separate buffers
            // kludges go into ctrlbuf, body stays in buf, but is
            // relocated.
            ctrlbuf = new char[msghdrapx.txtsize + 1];
            if (!ctrlbuf)
            {
                cerr << "Unable to allocate memory for control data (msg #"
                     << msgnum << ')' << endl;
            }

            ctrl_p = ctrlbuf;
            body_p = buf;
            newbody_p = buf;

            iskludge = false;
            wascr = true;
            while (*body_p)
            {
                if (wascr && 1 == *body_p)
                    iskludge = true;


                if (iskludge)
                {
                    // We don't CR/LF in the kludge buffer
                    if ('\r' != *body_p && '\n' != *body_p)
                        *(ctrl_p ++) = *body_p;
                }
                else
                    *(newbody_p ++) = *body_p;

                if ('\r' == *body_p || '\n' == *body_p)
                {
                    iskludge = false;
                    wascr = true;
                }
                else
                    wascr = false;

                body_p ++;
            }

            // Zero terminate what we got
            *ctrl_p = 0;
            *newbody_p = 0;

            destination.AddData(string(msghdrapx.sendername),
                                string(msghdrapx.recipientname),
                                string(msghdrapx.subject),
                                string(ctrlbuf), string(buf),
                                msghdrapx.timewritten,
                                (msghdrapx.statusflags & Fdapx_local)
                                    ? msghdrapx.timewritten
                                    : msghdrapx.timesentrcvd);

            delete buf;
        out:;
        }
    }

    fclose(msgtxt);
    fclose(msghdr);

    return true;
}
