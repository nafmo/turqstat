// Copyright (c) 2000 Peter Karlsson
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
# include <dirent.h>
# include <unistd.h>
#endif
#ifdef __EMX__
# include <emx/syscalls.h>
# include <io.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include "newsspoolread.h"
#include "utility.h"

NewsSpoolRead::NewsSpoolRead(const char *path)
{
    areapath = strdup(path);
}

NewsSpoolRead::~NewsSpoolRead()
{
    if (areapath) delete areapath;
}

bool NewsSpoolRead::Transfer(time_t starttime, StatEngine &destination)
{
    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        internalerrorquit(area_not_allocated, 1);
    }

    // This is a news spool
    destination.NewsArea();

    // Open the message directory
#if defined(UNIX)
    DIR *spooldir = opendir(areapath);
    if (!spooldir)
    {
        cerr << "Unable to open spool directory" << endl;
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

    string searchpath = dirname + string("*");

    struct _find spooldir;
    int rc = __findfirst(searchpath.c_str(), 0x2f, &spooldir);

    if (!rc)
    {
        cerr << "Unable to open spool directory" << endl;
        return false;
    }
#endif

    FILE *msg = NULL;
    unsigned long msglen;
    char *msgbuf = NULL, *ctrlbuf = NULL, *p;
    time_t arrived;
    unsigned long msgn = 0;
    long length, thislength;
    string from, subject;
    struct stat   msgstat;

#if defined(UNIX)
# define FILENAME spooldirent_p->d_name
# define FILESIZE msgstat.st_size
    struct dirent *spooldirent_p;

    while (NULL != (spooldirent_p = readdir(spooldir)))
#elif defined(__EMX__)
# define FILENAME spooldir.name
# define FILESIZE (spooldir.size_lo | (spooldir.size_hi << 16))
    while (0 == rc)
#endif
    {
        // Check that we really have a news file (all-digit name)
        string filename = FILENAME;
        string thisfile = dirname + filename;
        for (unsigned int i = 0; i < filename.length(); i ++)
            if (!isdigit(filename[i]))
                goto out2;

        msg = fopen(thisfile.c_str(), "rt");
        if (!msg)
        {
            cerr << "Unable to open " << thisfile << endl;
            goto out2;
        }

        stat(thisfile.c_str(), &msgstat);

        arrived = msgstat.st_mtime;
        if (arrived < starttime) goto out2;

        // Read the message header
        length = FILESIZE;
        ctrlbuf = new char[length + 1];
        if (!ctrlbuf)
        {
            cerr << "Unable to allocate memory for control data ("
                 << thisfile.c_str() << ')' << endl;
            goto out2;
        }

        ctrlbuf[0] = 0;
        p = ctrlbuf + 1;
        while (NULL != (p = fgets(p, length, msg)))
        {
            // Insert a ^A marker before buffer start
            *(p - 1) = 1;

            // Leave loop if we found an empty line (=message body starts)
            thislength = strlen(p);
            if (1 == thislength)
                break;

            // Point past newline marker (which will be overwritten)
            p += thislength;
            length -= thislength;
        }

        // Zero terminate
        p[0] = 0;

        msglen = length;

        msgbuf = new char[msglen];
        if (!msgbuf)
        {
            cerr << "Unable to allocate memory for message body ("
                 << thisfile.c_str() << ')' << endl;
            goto out;
        }

        fread(msgbuf, msglen, 1, msg);


        // Add to statistics
        destination.AddData(string(""), string(""), string(""),
                            string(ctrlbuf), string(msgbuf),
                            0, arrived);

        // Clean up our mess
        delete msgbuf;
out:;
        delete ctrlbuf;
out2:;

        cout << ++ msgn << " done\r";
        if (msg) fclose(msg);

#if defined(__EMX__)
        rc = __findnext(&spooldir);
#endif
    }

#if defined(UNIX)
    closedir(spooldir);
#endif

    return true;
}
