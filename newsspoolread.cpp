// Copyright (c) 2000-2007 Peter Krefting
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
#ifdef HAVE_IOSTREAM
# include <iostream>
#else
# include <iostream.h>
#endif
#include <time.h>
#include <string.h>
#include <stdio.h>
#ifdef HAVE_EMX_FINDFIRST
# include <emx/syscalls.h>
# include <io.h>
#elif defined(HAVE_DJGPP_FINDFIRST)
# include <dir.h>
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
    if (areapath) free(areapath);
}

bool NewsSpoolRead::Transfer(time_t starttime, time_t endtime,
                             StatEngine &destination)
{
    // Get the output object
    TDisplay *display = TDisplay::GetOutputObject();

    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        display->InternalErrorQuit(TDisplay::area_not_allocated, 1);
    }

    // This is a news spool
    destination.NewsArea();

    // Open the message directory
#if defined(HAVE_EMX_FINDFIRST) || defined(HAVE_LIBCRTDLL) || defined(HAVE_DJGPP_FINDFIRST)
    string dirname = string(areapath);
    if (dirname[dirname.length() - 1] != '\\')
    {
        dirname += '\\';
    }

    string searchpath = dirname + string("*");

# ifdef HAVE_EMX_FINDFIRST
    struct _find spooldir;
    int rc = __findfirst(searchpath.c_str(), 0x2f, &spooldir);
# elif defined(HAVE_DJGPP_FINDFIRST)
    struct ffblk sdmdir;
    int rc = findfirst(searchpath.c_str(), FA_RDONLY | FA_ARCH);
# else
    struct _finddata_t spooldir;
    intptr_t spoolhandle = _findfirst(searchpath.c_str(), &spooldir);
    intptr_t rc = spoolhandle;
# endif

    if (-1 == rc)
    {
        display->ErrorMessage(TDisplay::cannot_open_spool);
        return false;
    }
#else // no HAVE_EMX_FINDFIRST or HAVE_LIBCRTDLL
    DIR *spooldir = opendir(areapath);
    if (!spooldir)
    {
        display->ErrorMessage(TDisplay::cannot_open_spool);
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
    long length;
	size_t thislength;
    string from, subject;
#if !defined(HAVE_LIBCRTDLL) && !defined(HAVE_DJGPP_FINDFIRST)
    struct stat msgstat;
#endif

    display->SetMessagesTotal(-1);

#ifdef HAVE_EMX_FINDFIRST
# define FILENAME spooldir.name
# define FILESIZE (spooldir.size_lo | (spooldir.size_hi << 16))
    while (0 == rc)
#elif defined(HAVE_DJGPP_FINDFIRST)
# define FILENAME spooldir.ff_name
# define FILESIZE spooldir.ff_size
    while (0 == rc)
#elif defined(HAVE_LIBCRTDLL)
# define FILENAME spooldir.name
# define FILESIZE spooldir.size
    while (rc != -1)
#else // no HAVE_EMX_FINDFIRST or HAVE_LIBCRTDLL
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
            display->WarningMessage(TDisplay::cannot_open_file, thisfile);
            goto out2;
        }

#ifdef HAVE_LIBCRTDLL
        arrived = spooldir.time_create;
#elif defined(HAVE_DJGPP_FINDFIRST)
        if (0 == strcmp(spooldir.lfn_magic, "LFN32"))
        {
            stamp_s create_time = { spooldir.lfn_cdate, spooldir.lfn_ctime };
            arrived = stampToTimeT(&create_time);
        }
        else
        {
            stamp_s file_time = { spooldir.ff_fdate, spooldir.ff_ftime };
            arrived = stampToTimeT(&file_time);
        }
#else
        stat(thisfile.c_str(), &msgstat);
        arrived = msgstat.st_mtime;
#endif

#if !defined(HAVE_DJGPP_FINDFIRST)
# ifdef HAVE_TIMEZONE
        arrived -= timezone * 60;
# elif defined(HAVE_UTIMEZONE)
        arrived -= _timezone * 60;
# endif
#endif

        // Check if message is outside time range
        if (arrived < starttime || arrived > endtime) goto out2;

        // Read the message header
        length = FILESIZE;
        ctrlbuf = new char[length + 1];
        if (!ctrlbuf)
        {
            display->WarningMessage(TDisplay::cannot_allocate_control_file,
                                    thisfile);
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
            length -= static_cast<long>(thislength);
        }

        if (!p)
        {
            // Message contains only a header; skip it
            goto out;
        }

        // Zero terminate
        p[0] = 0;

        msglen = FILESIZE - ftell(msg) + 1;

        msgbuf = new char[msglen];
        if (!msgbuf)
        {
            display->WarningMessage(TDisplay::cannot_allocate_body_file,
                                    thisfile);
            goto out;
        }

        fread(msgbuf, msglen, 1, msg);

        // Add to statistics
        destination.AddData(string(""), string(""), string(""),
                            string(ctrlbuf), string(msgbuf),
                            0, arrived);

        // Clean up our mess
        delete[] msgbuf;
out:;
        delete[] ctrlbuf;
out2:;

        display->UpdateProgress(++ msgn);
        if (msg) fclose(msg);

#ifdef HAVE_EMX_FINDFIRST
        rc = __findnext(&spooldir);
#elif defined(HAVE_DJGPP_FINDFIRST)
        rc = findnext(&spooldir);
#elif defined(HAVE_LIBCRTDLL)
        rc = _findnext(spoolhandle, &spooldir);
#endif
    }

#ifdef HAVE_LIBCRTDLL
    _findclose(spoolhandle);
#elif !defined(HAVE_EMX_FINDFIRST)
    closedir(spooldir);
#endif

    return true;
}
