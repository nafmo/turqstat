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

    if (!strchr(areapath, '.') || strlen(areapath) < 11)
    {
        cerr << "Illegal area path (need to know the extension of .A?? "
                "or .F?? file" << endl;
        return false;
    }

    areapath[strlen(areapath) - 3] = 'A'; // Area file
    FILE *areaf = fopen(areapath, "rb");
    if (!areaf)
    {
        cerr << "Error: Cannot open " << areapath << endl;
        return false;
    }

    areapath[strlen(areapath) - 3] = 'F'; // Flags file
    FILE *flagf = fopen(areapath, "rb");
    if (!flagf)
    {
        cerr << "Error: Cannot open " << areapath << endl;
        return false;
    }

    header_s hdr;
    fread(&hdr, sizeof (hdr), 1, areaf);

    if (Mypoint_msgbaseversion < hdr.areaprf[0])
    {
        cerr << "Error: Illegal MyPoint message base version" << endl;
        return false;
    }

    fseek(flagf, sizeof (flags_s), SEEK_SET);

    // Read messages one by one
    bool stay = true;
    ltrhdr_s ltrhdr;
    ltrtrl_s ltrtrl;
    flags_s flags;
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

        fread(&flags,  sizeof (flags),  1, flagf);

        cout << msgnum << " messages done\r";
        msgnum ++;

        if (ltrhdr.delim != Mypoint_delimeter)
        {
            // Something went wrong
            cerr << "Message area garbled (illegal delimeter)!" << endl;
            fclose(areaf);
            return false;
        }

        // Only proceed if message is inside interval and not deleted
        if (ltrhdr.arrtim >= starttime &&
            0 == (flags.bits & Mypoint_delete))
        {
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
                cerr << "Message area garbled (footer does not match header)!"
                     << endl;
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
        }
        else
        {
            // Seek past letter structure
            fseek(areaf, ltrhdr.ltrsiz - sizeof (ltrhdr_s), SEEK_CUR);
        }
    out:;
    }

    fclose(areaf);
    fclose(flagf);

    return true;
}
