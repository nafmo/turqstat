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

// JAM(mbp) - Copyright 1993 Joaquim Homrighausen, Andrew Milner,
//                           Mats Birch, Mats Wallin.
//                           ALL RIGHTS RESERVED.

#include <string>
#include <iostream.h>
#include <time.h>
#include <stdio.h>

#include "jamread.h"

JamRead::JamRead(const char *path)
{
    areapath = strdup(path);
}

JamRead::~JamRead()
{
    if (areapath) delete areapath;
}

bool JamRead::Transfer(time_t starttime, StatEngine &destination)
{
    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        cerr << "Internal error: Area path was not allocated properly" << endl;
        return false;
    }

    // Open the message area files
    // <name>.jdx - contains one record per message
    string filepath = string(areapath) + ".jdx";
    FILE *jdx = fopen(filepath.c_str(), "rb");
    if (!jdx)
    {
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }

    // <name>.jhr - contains header information for messages
    filepath = string(areapath) + ".jhr";
    FILE *jhr = fopen(filepath.c_str(), "rb");
    if (!jhr)
    {
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }

    jamhdr_header_s baseheader;
    if (1 != fread(&baseheader, sizeof (jamhdr_header_s), 1, jhr))
    {
        cerr << "Error: Couldn't read from " << filepath << endl;
        return false;
    }

    if (memcmp(baseheader.signature, Jam_signature, sizeof(Jam_signature)) != 0)
    {
        cerr << "Error: Illegal JAM header" << endl;
        return false;
    }

    // <name>.jdt - contains message text
    filepath = string(areapath) + ".jdt";
    FILE *jdt = fopen(filepath.c_str(), "rb");
    if (!jdt)
    {
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }

    // Read messages one by one
    bool stay = true;
    unsigned active = 0, found = 0, total = baseheader.activemsgs,
             subfieldtotal;
    jamjdx_s jdxinfo;
    jamhdr_msg_s hdrinfo;
    jamhdr_subhdr_s subheader;
    string ctrlinfo, sender, recipient, subject;
    char *buf, subtmp[101];
    while (stay)
    {
        if (1 != fread(&jdxinfo, sizeof (jamjdx_s), 1, jdx))
        {
            // Nothing more to read, we are done
            stay = false;
            break;
        }
        found ++;

        fseek(jhr, jdxinfo.jhroffset, SEEK_SET);

        if (1 != fread(&hdrinfo, sizeof (jamhdr_msg_s), 1, jhr))
        {
            // Could not read message, quit
            cerr << "Unable to read header #" << found + baseheader.basemsgnum;
        }

        if  (0 == (hdrinfo.attribute & Jam_deleted))
        {
            // This is message is active
            active ++;
            cout << 100 * active / total << "% done\r";
            if (active == total) stay = false;

            // Check if message is too old
            if (0 == hdrinfo.dateprocessed)
            {
                if ((time_t) hdrinfo.datewritten < starttime)
                    goto out;
            }
            else if ((time_t) hdrinfo.dateprocessed < starttime)
                goto out;

            // Retrieve message body
            fseek(jdt, hdrinfo.offset, SEEK_SET);
            buf = new char[hdrinfo.txtlen + 1];

            if (!buf)
            {
                cerr << "Unable to allocate memory for message body (msg #"
                     << found + baseheader.basemsgnum << ')' << endl;
                goto out;
            }

            fread(buf, hdrinfo.txtlen, 1, jdt);
            buf[hdrinfo.txtlen] = 0;

            // Locate sender, recipient, subject and PID and MSGID kludges
            // from subfields.
            sender = "";
            recipient = "";
            subject = "";
            ctrlinfo = "";

            // Scan through the subfields
            subfieldtotal = 0;
            while (subfieldtotal < hdrinfo.subfieldlen)
            {
                if (1 == fread(&subheader, sizeof (jamhdr_subhdr_s),
                               1, jhr))
                {
                    subfieldtotal += sizeof (jamhdr_subhdr_s) +
                                     subheader.datlen;

                    switch (subheader.loid)
                    {
                        case Jam_sender:
                            fread(&subtmp, subheader.datlen, 1, jhr);
                            subtmp[subheader.datlen] = 0;
                            sender = subtmp;
                            break;

                        case Jam_recipient:
                            fread(&subtmp, subheader.datlen, 1, jhr);
                            subtmp[subheader.datlen] = 0;
                            recipient = subtmp;
                            break;

                        case Jam_subject:
                            fread(&subtmp, subheader.datlen, 1, jhr);
                            subtmp[subheader.datlen] = 0;
                            subject = subtmp;
                            break;

                        case Jam_MSGID:
                            fread(&subtmp, subheader.datlen, 1, jhr);
                            subtmp[subheader.datlen] = 0;
                            ctrlinfo += "\1MSGID: ";
                            ctrlinfo += subtmp;
                            break;

                        case Jam_PID:
                            fread(&subtmp, subheader.datlen, 1, jhr);
                            subtmp[subheader.datlen] = 0;
                            ctrlinfo += "\1PID: ";
                            ctrlinfo += subtmp;
                            break;

                        default:
                            fseek(jhr, subheader.datlen, SEEK_CUR);
                            break;
                    }
                }
                else
                    break;
            }

            destination.AddData(sender, recipient, subject,
                                ctrlinfo, string(buf),
                                hdrinfo.datewritten,
                                (0 == hdrinfo.dateprocessed)
                                    ? hdrinfo.datewritten
                                    : hdrinfo.dateprocessed);

            delete buf;
        out:;
        }
    }

    fclose(jhr);
    fclose(jdt);
    fclose(jdx);

    return true;
}
