// Copyright (c) 1999-2001 Peter Krefting
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
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "squishread.h"
#include "utility.h"
#include "statengine.h"
#include "output.h"

SquishRead::SquishRead(const char *path)
{
    areapath = strdup(path);
}

SquishRead::~SquishRead()
{
    if (areapath) free(areapath);
}

bool SquishRead::Transfer(time_t starttime, time_t endtime,
                          StatEngine &destination)
{
    // Get the output object
    TDisplay *display = TDisplay::GetOutputObject();

    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        display->InternalErrorQuit(TDisplay::area_not_allocated, 1);
    }

    // Open the message area files
    // <name>.sqd - contains everything we need
    string filepath = string(areapath) + ".sqd";
    FILE *sqd = fopen(filepath.c_str(), "rb");
    if (!sqd)
    {
        char *p = strrchr(areapath, '.');
        if (p)
        {
            *p = 0;
            filepath = string(areapath) + ".sqd";
            sqd = fopen(filepath.c_str(), "rb");
        }
        if (!sqd)
        {
            display->ErrorMessage(TDisplay::cannot_open, filepath);
            return false;
        }
    }

    sqbase_s baseheader;
    if (1 != fread(&baseheader, sizeof (sqbase_s), 1, sqd))
    {
        display->ErrorMessage(TDisplay::cannot_read, filepath);
        return false;
    }

    int32_t offset = baseheader.len - sizeof (sqbase_s);
    if (offset > 0)
    {
        fseek(sqd, offset, SEEK_CUR);
    }
    else if (offset < 0)
    {
        display->ErrorMessage(TDisplay::strange_squish_header);
        fclose(sqd);
        return false;
    }

    int32_t sqhdroffset = baseheader.sz_sqhdr - sizeof(sqhdr_s);

    // Read messages
    uint32_t current = baseheader.begin_frame;
    if (0 == current)
    {
        display->ErrorMessage(TDisplay::message_base_empty);
        fclose(sqd);
        return true;
    }
    else if (current < sizeof (sqbase_s))
    {
        display->ErrorMessage(TDisplay::strange_squish_offset);
        fclose(sqd);
        return false;
    }

    display->SetMessagesTotal(baseheader.high_msg);

    uint32_t msgn, msglen;
    char *ctrlbuf = NULL, *msgbuf = NULL;
    sqhdr_s sqhdr;
    xmsg_s xmsg;
    time_t arrivaltime;

    for (msgn = 1L; msgn <= baseheader.high_msg; msgn ++)
    {
        fseek(sqd, current, SEEK_SET);

        // Read the SQHDR
        if (1 != fread(&sqhdr, sizeof (sqhdr_s), 1, sqd))
        {
            display->ErrorMessage(TDisplay::premature_squish_end);
            fclose(sqd);
            return false;
        }

        if (sqhdr.id != Squish_id)
        {
            display->ErrorMessage(TDisplay::illegal_squish_header);
            fclose(sqd);
            return false;
        }

        current = sqhdr.next_frame;

        if (Squish_type_normal != sqhdr.frame_type)
        {
            display->WarningMessage(TDisplay::abnormal_squish_frame, msgn);
            continue;
        }

        if (sqhdroffset) fseek(sqd, sqhdroffset, SEEK_CUR);

        // Read the XMSG
        if (1 != fread(&xmsg, sizeof (xmsg_s), 1, sqd))
        {
            display->ErrorMessage(TDisplay::premature_squish_end);
            fclose(sqd);
            return false;
        }

        ctrlbuf = new char[sqhdr.clen + 1];
        if (!ctrlbuf)
        {
            display->WarningMessage(TDisplay::cannot_allocate_control, msgn);
            goto out;
        }
        fread(ctrlbuf, sqhdr.clen, 1, sqd);
        ctrlbuf[sqhdr.clen] = 0;

        msglen = sqhdr.msg_length - sizeof(xmsg_s) - sqhdr.clen;
        msgbuf = new char[msglen + 1];
        if (!msgbuf)
        {
            display->WarningMessage(TDisplay::cannot_allocate_body, msgn);
            goto out2;
        }

        fread(msgbuf, msglen, 1, sqd);
        msgbuf[msglen] = 0;
        // Remove SEEN-BY and PATH that are in Squish part of the body
        fixupctrlbuffer(msgbuf, NULL);

        // Add to statistics
        arrivaltime = stampToTimeT(&xmsg.date_arrived);
        if (arrivaltime >= starttime && arrivaltime <= endtime)
        {
            destination.AddData(string((char *) xmsg.from),
                                string((char *) xmsg.to),
                                string((char *) xmsg.subject),
                                string(ctrlbuf), string(msgbuf),
                                stampToTimeT(&xmsg.date_written),
                                arrivaltime);
        }

        // Clean up our mess
out:;
        delete[] ctrlbuf;
out2:;
        delete[] msgbuf;

        display->UpdateProgress(msgn);
    }

    fclose(sqd);

    return true;
}
