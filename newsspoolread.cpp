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

#include <config.h>
#include <iostream.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#ifdef HAS_EMX_FINDFIRST
# include <emx/syscalls.h>
# include <io.h>
#elif defined(HAVE_LIBCRTDLL)
# include <io.h>
#else
# ifdef HAVE_DIRENT_H
# include <dirent.h>
#  define NAMLEN(dirent) strlen((dirent)->d_name)
# else
#  define dirent direct
#  define NAMLEN(dirent) (dirent)->d_namlen
#  ifdef HAVE_SYS_NDIR_H
#   include <sys/ndir.h>
#  endif
#  ifdef HAVE_SYS_DIR_H
#   include <sys/dir.h>
#  endif
#  ifdef HAVE_NDIR_H
#   include <ndir.h>
#  endif
# endif
#endif
#ifndef HAVE_LIBCRTDLL
# include <sys/types.h>
# include <sys/stat.h>
#endif

#include "newsspoolread.h"
#include "utility.h"
#include "statengine.h"
#include "output.h"

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

    // Get the output object
    ProgressDisplay *display = ProgressDisplay::GetOutputObject();

    // This is a news spool
    destination.NewsArea();

    // Open the message directory
#if defined(HAS_EMX_FINDFIRST) || (HAVE_LIBCRTDLL)
    string dirname = string(areapath);
    if (dirname[dirname.length() - 1] != '\\')
    {
        dirname += '\\';
    }

    string searchpath = dirname + string("*");

# ifdef HAS_EMX_FINDFIRST
    struct _find spooldir;
    int rc = __findfirst(searchpath.c_str(), 0x2f, &spooldir);
# else
    struct _finddata_t spooldir;
    int spoolhandle = _findfirst(searchpath.c_str(), &spooldir);
    int rc = spoolhandle;
# endif

    if (-1 == rc)
    {
        display->ErrorMessage("Unable to open spool directory");
        return false;
    }
#else // no HAS_EMX_FINDFIRST or HAVE_LIBCRTDLL
    DIR *spooldir = opendir(areapath);
    if (!spooldir)
    {
        display->ErrorMessage("Unable to open spool directory");
        return false;
    }

    string dirname = string(areapath);
    if (dirname[dirname.length() - 1] != '/')
    {
        dirname += '/';
    }
#endif

    FILE *msg = NULL;
    unsigned long msglen;
    char *msgbuf = NULL, *ctrlbuf = NULL, *p;
    time_t arrived;
    unsigned long msgn = 0;
    long length, thislength;
    string from, subject;
#ifndef HAVE_LIBCRTDLL
    struct stat msgstat;
#endif

    display->SetMessagesTotal(-1);

#ifdef HAS_EMX_FINDFIRST
# define FILENAME spooldir.name
# define FILESIZE (spooldir.size_lo | (spooldir.size_hi << 16))
    while (0 == rc)
#elif defined(HAVE_LIBCRTDLL)
# define FILENAME spooldir.name
# define FILESIZE spooldir.size
    while (rc != -1)
#else // no HAS_EMX_FINDFIRST or HAVE_LIBCRTDLL
# define FILENAME spooldirent_p->d_name
# define FILESIZE msgstat.st_size
    struct dirent *spooldirent_p;

    while (NULL != (spooldirent_p = readdir(spooldir)))
#endif
    {
        // Check that we really have a news file (all-digit name)
        string filename = FILENAME;
        string thisfile = dirname + filename;
        for (unsigned int i = 0; i < filename.length(); i ++)
            if (!isdigit(filename[i]))
            {
                msg = NULL;
                goto out2;
            }

        msg = fopen(thisfile.c_str(), "r");
        if (!msg)
        {
            string msg = string("Cannot open ") + thisfile;
            display->WarningMessage(msg);
            goto out2;
        }

#ifdef HAVE_LIBCRTDLL
        arrived = spooldir.time_create;
#else
        stat(thisfile.c_str(), &msgstat);
        arrived = msgstat.st_mtime;
#endif

#ifdef HAVE_TIMEZONE
        arrived -= timezone * 60;
#elif defined(HAVE_UTIMEZONE)
        arrived -= _timezone * 60;
#endif

        if (arrived < starttime) goto out2;

        // Read the message header
        length = FILESIZE;
        ctrlbuf = new char[length + 1];
        if (!ctrlbuf)
        {
            string msg = string("Unable to allocate memory for control "
                                "data for ") + thisfile;
            display->WarningMessage(msg);
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

        msglen = FILESIZE - ftell(msg) + 1;

        msgbuf = new char[msglen];
        if (!msgbuf)
        {
            string msg = string("Unable to allocate memory for message "
                                "body for ") + thisfile;
            display->WarningMessage(msg);
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

        display->UpdateProgress(++ msgn);
        if (msg) fclose(msg);

#ifdef HAS_EMX_FINDFIRST
        rc = __findnext(&spooldir);
#elif defined(HAVE_LIBCRTDLL)
        rc = _findnext(spoolhandle, &spooldir);
#endif
    }

#ifdef HAVE_LIBCRTDLL
    _findclose(spoolhandle);
#elif !defined(HAS_EMX_FINDFIRST)
    closedir(spooldir);
#endif

    return true;
}
