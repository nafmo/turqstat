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

#include "mypointread.h"

MyPointRead::MyPointRead(const char *path)
{
    areapath = strdup(path);
}

MyPointRead::~MyPointRead()
{
    if (areapath) delete areapath;
}

bool MyPointRead::Transfer(time_t starttime, StatEngine &destination)
{
    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        cerr << "Internal error: Area path was not allocated properly" << endl;
        return false;
    }

    FILE *areaf = fopen(areapath, "rb");
    if (!areaf)
    {
        cerr << "Error: Cannot open " << areapath << endl;
        return false;
    }

    header_s hdr;
    fread(&hdr, sizeof (hdr), 1, areaf);

    // Read messages one by one
    bool stay = true;
    ltrhdr_s ltrhdr;
    ltrtrl_s ltrtrl;
    unsigned msgnum = 0;
    bool iskludge, wascr;
    unsigned tosize, fromsize, subjsize, bodysize;
    char *ctrlbuf, *buf, *to_p, *from_p, *sub_p,
         *ctrl_p, *body_p, *newbody_p;
    while (stay)
    {
        if (1 != fread(&ltrhdr, sizeof (ltrhdr), 1, areaf))
        {
            // Nothing more to read, we are done
            stay = false;
            break;
        }

        cout << msgnum << " messages done\r";
        msgnum ++;

        if (ltrhdr.delim != Mypoint_delimeter)
        {
cout << "sizeof(header_s) = " << sizeof (header_s) << endl;
cout << "sizeof(ltrhdr_s) = " << sizeof (ltrhdr_s) << endl;
cout << "msgnum = " << msgnum << endl;
cout << "ltrhdr.delim = " << ltrhdr.delim << endl;
            // Something went wrong
            cerr << "Message area garbled!" << endl;
            fclose(areaf);
            return false;
        }

        // Check if message is too old
        if (ltrhdr.arrtim < starttime)
            goto out;

        // Retrieve sender, recipient, subject
        tosize   = ltrhdr.fromp - sizeof (ltrhdr_s);
        fromsize = ltrhdr.subjp - ltrhdr.fromp;
        subjsize = ltrhdr.textp - ltrhdr.subjp;
        bodysize = ltrhdr.ltrsiz- ltrhdr.textp - sizeof (ltrtrl_s);

        to_p    = new char[tosize  ];
        from_p  = new char[fromsize];
        sub_p   = new char[subjsize];
        buf     = new char[bodysize];
        ctrlbuf = new char[bodysize];

        if (!buf || !ctrlbuf || !to_p || !from_p || !sub_p)
        {
            cerr << "Unable to allocate memory for message body (msg #"
                 << msgnum << ')' << endl;

            if (to_p)       delete to_p;
            if (from_p)     delete from_p;
            if (sub_p)      delete sub_p;
            if (buf)        delete buf;
            if (ctrlbuf)    delete ctrlbuf;
            goto out;
        }

        fread(to_p,    tosize,            1, areaf);
        fread(from_p,  fromsize,          1, areaf);
        fread(sub_p,   subjsize,          1, areaf);
        fread(buf,     bodysize,          1, areaf);    // Text + kludges
        fread(&ltrtrl, sizeof (ltrtrl_s), 1, areaf);

        // Check for inconsistencies
        if (ltrtrl.ltrsiz != ltrhdr.ltrsiz)
        {
            cerr << "Message area garbled!" << endl;
            fclose(areaf);
            return false;
        }

        // Copy out kludges and body to separate buffers
        // kludges go into ctrlbuf, body stays in buf, but is
        // relocated.
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

        destination.AddData(string(from_p),
                            string(to_p),
                            string(sub_p),
                            string(ctrlbuf), string(buf),
                            ltrhdr.ltrtim,
                            ltrhdr.arrtim);

        // Deallocate
        delete buf;
        delete ctrlbuf;
        delete to_p;
        delete sub_p;
        delete from_p;

    out:;
    }

    fclose(areaf);

    return true;
}
