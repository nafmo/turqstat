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

#include <iostream.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#ifdef UNIX
# include <sys/types.h>
# include <dirent.h>
# include <sys/stat.h>
# include <unistd.h>
#endif
#ifdef __EMX__
# include <emx/syscalls.h>
#endif

#include "sdmread.h"
#include "utility.h"

SdmRead::SdmRead(const char *path, bool hasarrivetime) : isopus(hasarrivetime)
{
    areapath = strdup(path);
}

SdmRead::~SdmRead()
{
    if (areapath) delete areapath;
}

bool SdmRead::Transfer(time_t starttime, StatEngine &destination)
{
    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        internalerrorquit(area_not_allocated, 1);
    }

    // Non-Opus SDM doesn't have arrival times
    if (!isopus)
    {
        destination.NoArrivalTime();
    }

    // Open the message directory
#if defined(UNIX)
    DIR *sdmdir = opendir(areapath);
    if (!sdmdir)
    {
        cerr << "Unable to open *.MSG directory" << endl;
        return false;
    }

    string dirname = string(areapath);
    if (dirname[dirname.length() - 1] != '/')
    {
        dirname += '/';
    }
#elif defined(__EMX__)
    string dirname = string(areapath);
    if (dirname[dirname.length() - 1] != '\\')
    {
        dirname += '\\';
    }

    string searchpath = dirname + string("*.msg");

    struct _find sdmdir;
    int rc = __findfirst(searchpath.c_str(), 0x2f, &sdmdir);

    if (!rc)
    {
        cerr << "Unable to open *.MSG directory" << endl;
        return false;
    }
#endif

    sdmhead_s sdmhead;
    FILE *msg = NULL;
    unsigned long msglen;
    char *msgbuf = NULL, *ctrlbuf = NULL;
    time_t written, arrived;
    UINT32 msgn = 0;

#if defined(UNIX)
# define FILENAME sdmdirent_p->d_name
# define FILESIZE sdmstat.st_size
    struct dirent *sdmdirent_p;
    struct stat   sdmstat;

    while (NULL != (sdmdirent_p = readdir(sdmdir)))
#elif defined(__EMX__)
# define FILENAME sdmdir.name
# define FILESIZE (sdmdir.size_lo | (sdmdir.size_hi << 16))
    while (0 == rc)
#endif
    {
        // Check that we really have a *.msg file
        // Unix readdir gives us all files, and DOS findfirst is buggy
        string thisfile = dirname + FILENAME;
        if (fcompare(thisfile.substr(thisfile.length() - 3, 3), "msg"))
            goto out;

        msg = fopen(thisfile.c_str(), "rb");
        if (!msg)
        {
            cerr << "Unable to open " << thisfile << endl;
            goto out;
        }

#ifdef UNIX
        stat(thisfile.c_str(), &sdmstat);
#endif

        // Read the message header
        if (1 != fread(&sdmhead, sizeof (sdmhead_s), 1, msg))
        {
            cerr << "Broken MSG file " << thisfile << endl;
            goto out;
        }

        msglen = FILESIZE - sizeof (sdmhead_s);

        msgbuf = new char[msglen];
        if (!msgbuf)
        {
            cerr << "Unable to allocate memory for message body ("
                 << thisfile.c_str() << ')' << endl;
            goto out2;
        }

        ctrlbuf = new char[msglen];
        if (!ctrlbuf)
        {
            cerr << "Unable to allocate memory for control data ("
                 << thisfile.c_str() << ')' << endl;
            goto out;
        }

        fread(msgbuf, msglen, 1, msg);

        // Separate kludges from text
        fixupctrlbuffer(msgbuf, ctrlbuf);

        written = asciiToTimeT((char *) sdmhead.datetime);
        if (isopus)
        {
            arrived = stampToTimeT(&sdmhead.opus.arrived);
        }
        else
        {
            arrived = written;
        }

        // Add to statistics
        if (arrived >= starttime)
        {
            destination.AddData(string((char *) sdmhead.fromusername),
                                string((char *) sdmhead.tousername),
                                string((char *) sdmhead.subject),
                                string(ctrlbuf), string(msgbuf),
                                written, arrived);
        }

        // Clean up our mess
out:;
        delete ctrlbuf;
out2:;
        delete msgbuf;

        cout << ++ msgn << " done\r";
        if (msg) fclose(msg);

#if defined(__EMX__)
        rc = __findnext(&sdmdir);
#endif
    }

#if defined(UNIX)
    closedir(sdmdir);
#endif

    return true;
}
