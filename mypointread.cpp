// Copyright (c) 1999-2007 Peter Karlsson
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
#include <stdlib.h>
#include <string>
#ifdef HAVE_IOSTREAM
# include <iostream>
#else
# include <iostream.h>
#endif
#include <time.h>
#include <stdio.h>

#include "mypointread.h"
#include "utility.h"
#include "statengine.h"
#include "output.h"

MyPointRead::MyPointRead(const char *path, unsigned areanum)
{
    areapath = strdup(path);
    areanumber = areanum;
}

MyPointRead::~MyPointRead()
{
    if (areapath) free(areapath);
}

bool MyPointRead::Transfer(time_t starttime, time_t endtime,
                           StatEngine &destination)
{
    // Get the output object
    TDisplay *display = TDisplay::GetOutputObject();

    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        display->InternalErrorQuit(TDisplay::area_not_allocated, 1);
    }

    // Calculate area number ending
    char ending[4] = { 0, 0, 0, 0};
    ending[0] = 'A';
    ending[1] = '0' + areanumber / 26;
    ending[2] = 'A' + areanumber % 26;

    // Sanity check
    if (ending[1] > '9' || ending[2] > 'Z')
    {
        display->ErrorMessage(TDisplay::illegal_area);
        return false;
    }

    // Open the message area files
    string basepath = areapath;
#ifdef BACKSLASH_PATHS
    if (basepath[basepath.length() - 1] != '/' &&
        basepath[basepath.length() - 1] != '\\')
        basepath += '\\';
#else
    if (basepath[basepath.length() - 1] != '/')
        basepath += '/';
#endif
    basepath += "MYPOINT.";

    string filepath = basepath + ending;
    FILE *areaf = fopen(filepath.c_str(), "rb");
    if (!areaf)
    {
        display->ErrorMessage(TDisplay::cannot_open, filepath);
        return false;
    }

    ending[0] = 'F';
    filepath = basepath + ending;
    FILE *flagf = fopen(filepath.c_str(), "rb");
    if (!flagf)
    {
        display->ErrorMessage(TDisplay::cannot_open, filepath);
        return false;
    }

    header_s hdr;
    fread(&hdr, sizeof (hdr), 1, areaf);

    // Check for valid message base versions
    uint8_t msgbaserevision = hdr.areaprf[0];
    if (Mypoint_msgbaseversion2 != msgbaserevision &&
        Mypoint_msgbaseversion3 != msgbaserevision)
    {
        display->ErrorMessage(TDisplay::illegal_mypoint_version,
                              msgbaserevision);
        return false;
    }

    fseek(flagf, sizeof (flags_s), SEEK_SET);

    // Check size of ltrhdr structure;
    size_t sizeof_ltrhdr, sizeof_ltrtrl;
    ltrhdr_u ltrhdr;
    ltrtrl_u ltrtrl;
    if (Mypoint_msgbaseversion2 == msgbaserevision)
    {
        sizeof_ltrhdr = sizeof (ltrhdr.version2);
        sizeof_ltrtrl = sizeof (ltrtrl.version2);
    }
    else //if (Mypoint_msgbaseversion3 == msgbaserevision)
    {
        sizeof_ltrhdr = sizeof (ltrhdr.version3);
        sizeof_ltrtrl = sizeof (ltrtrl.version3);
    }

    // Seek past dummy message trailer
    fseek(areaf, static_cast<long>(sizeof_ltrtrl), SEEK_CUR);

    // Read messages one by one
    bool stay = true;
    flags_s flags;
    unsigned msgnum = 0;
    size_t tosize, fromsize, subjsize, bodysize;
    char *ctrlbuf, *buf, *to_p, *from_p, *sub_p;
    uint16_t fromp, subjp, textp, ltrsiz, ltrtrl_ltrsiz;
    uint32_t delim, arrtim, ltrtim;

    display->SetMessagesTotal(-1);

    while (stay)
    {
        if (1 != fread(&ltrhdr, sizeof_ltrhdr, 1, areaf))
        {
            // Nothing more to read, we are done
            stay = false;
            break;
        }

        fread(&flags,  sizeof (flags),  1, flagf);

        display->UpdateProgress(msgnum ++);

        if (Mypoint_msgbaseversion2 == msgbaserevision)
        {
            delim = ltrhdr.version2.delim;
            arrtim= ltrhdr.version2.arrtim;
            ltrtim= ltrhdr.version2.ltrtim;
            fromp = ltrhdr.version2.fromp;
            subjp = ltrhdr.version2.subjp;
            textp = ltrhdr.version2.textp;
            ltrsiz= ltrhdr.version2.ltrsiz;
        }
        else //if (Mypoint_msgbaseversion3 == msgbaserevision)
        {
            delim = ltrhdr.version3.delim;
            arrtim= ltrhdr.version3.arrtim;
            ltrtim= ltrhdr.version3.ltrtim;
            fromp = ltrhdr.version3.fromp;
            subjp = ltrhdr.version3.subjp;
            textp = ltrhdr.version3.textp;
            ltrsiz= ltrhdr.version3.ltrsiz;
        }

        if (delim != Mypoint_delimeter)
        {
            // Something went wrong
            display->ErrorMessage(TDisplay::mypoint_area_garbled);
            fclose(areaf);
            return false;
        }

        // Only proceed if message is inside interval and not deleted
        if (time_t(arrtim) >= starttime &&
            time_t(arrtim) <= endtime &&
            0 == (flags.bits & Mypoint_delete))
        {
            // Retrieve sender, recipient, subject
            tosize   = fromp - sizeof_ltrhdr;
            fromsize = subjp - fromp;
            subjsize = textp - subjp;
            bodysize = ltrsiz- textp - sizeof_ltrtrl;

            to_p    = new char[tosize  ];
            from_p  = new char[fromsize];
            sub_p   = new char[subjsize];
            buf     = new char[bodysize];
            ctrlbuf = new char[bodysize];

            if (!buf || !ctrlbuf || !to_p || !from_p || !sub_p)
            {
                display->WarningMessage(TDisplay::cannot_allocate_body,
                                        msgnum);

                if (to_p)       delete[] to_p;
                if (from_p)     delete[] from_p;
                if (sub_p)      delete[] sub_p;
                if (buf)        delete[] buf;
                if (ctrlbuf)    delete[] ctrlbuf;
                goto out;
            }

            fread(to_p,    tosize,        1, areaf);
            fread(from_p,  fromsize,      1, areaf);
            fread(sub_p,   subjsize,      1, areaf);
            fread(buf,     bodysize,      1, areaf);    // Text + kludges
            fread(&ltrtrl, sizeof_ltrtrl, 1, areaf);

            if (Mypoint_msgbaseversion2 == msgbaserevision)
            {
                ltrtrl_ltrsiz = ltrtrl.version2.ltrsiz;
            }
            else //if (Mypoint_msgbaseversion3 == msgbaserevision)
            {
                ltrtrl_ltrsiz = ltrtrl.version3.ltrsiz;
            }

            // Check for inconsistencies
            if (ltrtrl_ltrsiz != ltrsiz)
            {
                display->ErrorMessage(TDisplay::mypoint_area_garbled_2);
                fclose(areaf);
                return false;
            }

            // Separate kludges from text
            fixupctrlbuffer(buf, ctrlbuf);

            destination.AddData(string(from_p),
                                string(to_p),
                                string(sub_p),
                                string(ctrlbuf), string(buf),
                                ltrtim,
                                arrtim);

            // Deallocate
            delete[] buf;
            delete[] ctrlbuf;
            delete[] to_p;
            delete[] sub_p;
            delete[] from_p;
        }
        else
        {
            // Seek past letter structure
            fseek(areaf, static_cast<long>(ltrsiz - sizeof_ltrhdr), SEEK_CUR);
        }
    out:;
    }

    fclose(areaf);
    fclose(flagf);

    return true;
}
