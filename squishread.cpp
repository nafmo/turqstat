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

#include <iostream.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "squishread.h"

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

    // Open the message area files
    // <name>.sqd - contains everything we need
    string filepath = string(areapath) + ".sqd";
    FILE *sqd = fopen(filepath.c_str(), "rb");
    if (!sqd)
    {
        cerr << "Error: Cannot open " << filepath << endl;
        return false;
    }

    sqbase_s baseheader;
    if (1 != fread(&baseheader, sizeof (sqbase_s), 1, sqd))
    {
        cerr << "Error: Couldn't read from " << filepath << endl;
        return false;
    }
    
    SINT32 offset = baseheader.len - sizeof (sqbase_s);
    if (offset > 0)
    {
        fseek(sqd, offset, SEEK_CUR);
    }
    else if (offset < 0)
    {
        cerr << "Strange Squish header length" << endl;
        fclose(sqd);
        return false;
    }

    SINT32 sqhdroffset = baseheader.sz_sqhdr - sizeof(sqhdr_s);

    // Read messages
    UINT32 current = baseheader.begin_frame;
    if (current < sizeof (sqbase_s))
    {
        cerr << "Strange Squish header offset" << endl;
        fclose(sqd);
        return false;
    }

    UINT32 msgn, msglen;
    char *ctrlbuf = NULL, *msgbuf = NULL;
    sqhdr_s sqhdr;
    xmsg_s xmsg;

    for (msgn = 1L; msgn <= baseheader.high_msg; msgn ++)
    {
        fseek(sqd, current, SEEK_SET);

        // Read the SQHDR
        if (1 != fread(&sqhdr, sizeof (sqhdr_s), 1, sqd))
        {
            cerr << "Premature end of Squish data" << endl;
            fclose(sqd);
            return false;
        }

        if (sqhdr.id != Squish_id)
        {
            cerr << "Illegal Squish header ID" << endl;
            fclose(sqd);
            return false;
        }

        current = sqhdr.next_frame;

        if (Squish_type_normal != sqhdr.frame_type)
        {
            cerr << "Not normal Squish frame: " << msgn << endl;
            continue;
        }

        if (sqhdroffset) fseek(sqd, sqhdroffset, SEEK_CUR);

        // Read the XMSG
        if (1 != fread(&xmsg, sizeof (xmsg_s), 1, sqd))
        {
            cerr << "Premature end of Squish data" << endl;
            fclose(sqd);
            return false;
        }

        ctrlbuf = new char[sqhdr.clen + 1];
        if (!ctrlbuf)
        {
            cerr << "Unable to allocate memory for control data (msg #"
                 << msgn << ')' << endl;
            goto out;
        }
        fread(ctrlbuf, sqhdr.clen, 1, sqd);
        ctrlbuf[sqhdr.clen] = 0;

        msglen = sqhdr.msg_length - sizeof(xmsg_s) - sqhdr.clen;
        msgbuf = new char[msglen + 1];
        if (!msgbuf)
        {
            cerr << "Unable to allocate memory for message body (msg #"
                 << msgn << ')' << endl;
            goto out2;
        }
        
        fread(msgbuf, msglen, 1, sqd);
        msgbuf[msglen] = 0;
        // Remove SEEN-BY and PATH that are in Squish part of the body
        fixupctrlbuffer(msgbuf, NULL);

        // Add to statistics
        if (stampToTimeT(&xmsg.date_arrived) >= starttime)
        {
            destination.AddData(string((char *) xmsg.from),
                                string((char *) xmsg.to),
                                string((char *) xmsg.subject),
                                string(ctrlbuf), string(msgbuf),
                                stampToTimeT(&xmsg.date_written),
                                stampToTimeT(&xmsg.date_arrived));
        }

        // Clean up our mess
out:;        
        delete ctrlbuf;
out2:;
        delete msgbuf;

        cout << 100 * msgn / baseheader.high_msg << "% done\r";
    }

    fclose(sqd);

    return true;
}
