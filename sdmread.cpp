// Copyright (c) 1999-2001 Peter Karlsson
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
#ifdef HAVE_EMX_FINDFIRST
# include <emx/syscalls.h>
#elif defined(HAVE_LIBCRTDLL)
# include <io.h>
#else
# include <sys/types.h>
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
# include <sys/stat.h>
# include <unistd.h>
#endif

#include "sdmread.h"
#include "utility.h"
#include "statengine.h"
#include "output.h"

SdmRead::SdmRead(const char *path, bool hasarrivetime) : isopus(hasarrivetime)
{
    areapath = strdup(path);
}

SdmRead::~SdmRead()
{
    if (areapath) free(areapath);
}

bool SdmRead::Transfer(time_t starttime, time_t endtime,
                       StatEngine &destination)
{
    // Get the output object
    TDisplay *display = TDisplay::GetOutputObject();

    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        display->InternalErrorQuit(TDisplay::area_not_allocated, 1);
    }

    // Non-Opus SDM doesn't have arrival times
    if (!isopus)
    {
        destination.NoArrivalTime();
    }

    // Open the message directory
#if defined(HAVE_EMX_FINDFIRST) || (HAVE_LIBCRTDLL)
    string dirname = string(areapath);
    if (dirname[dirname.length() - 1] != '\\')
    {
        dirname += '\\';
    }

    string searchpath = dirname + string("*.msg");

# ifdef HAVE_EMX_FINDFIRST
    struct _find sdmdir;
    int rc = __findfirst(searchpath.c_str(), 0x2f, &sdmdir);
# else
    struct _finddata_t sdmdir;
    int sdmhandle = _findfirst(searchpath.c_str(), &sdmdir);
    int rc = sdmhandle;
# endif

    if (-1 == rc)
    {
        display->ErrorMessage(TDisplay::cannot_open_msg_directory);
        return false;
    }
#else // no HAVE_EMX_FINDFIRST or HAVE_LIBCRTDLL
    DIR *sdmdir = opendir(areapath);
    if (!sdmdir)
    {
        display->ErrorMessage(TDisplay::cannot_open_msg_directory);
        return false;
    }

    string dirname = string(areapath);
    if (dirname[dirname.length() - 1] != '/')
    {
        dirname += '/';
    }
#endif // else no HAVE_EMX_FINDFIRST

    sdmhead_s sdmhead;
    FILE *msg = NULL;
    unsigned long msglen;
    char *msgbuf = NULL, *ctrlbuf = NULL;
    time_t written, arrived;
    uint32_t msgn = 0;

    display->SetMessagesTotal(-1);

#ifdef HAVE_EMX_FINDFIRST
# define FILENAME sdmdir.name
# define FILESIZE (sdmdir.size_lo | (sdmdir.size_hi << 16))
    while (0 == rc)
#elif defined(HAVE_LIBCRTDLL)
# define FILENAME sdmdir.name
# define FILESIZE sdmdir.size
    while (rc != -1)
#else // no HAVE_EMX_FINDFIRST or HAVE_LIBCRTDLL
# define FILENAME sdmdirent_p->d_name
# define FILESIZE sdmstat.st_size
    struct dirent *sdmdirent_p;
    struct stat   sdmstat;

    while (NULL != (sdmdirent_p = readdir(sdmdir)))
#endif
    {
        // Check that we really have a *.msg file
        // Unix readdir gives us all files, and DOS findfirst is buggy
        string thisfile = dirname + FILENAME;
        if (fcompare(thisfile.substr(thisfile.length() - 3, 3), "msg"))
        {
            msg = NULL;
            goto out;
        }

        msg = fopen(thisfile.c_str(), "rb");
        if (!msg)
        {
            display->WarningMessage(TDisplay::cannot_open, thisfile);
            goto out;
        }

#if !defined(HAVE_EMX_FINDFIRST) && !defined(HAVE_LIBCRTDLL)
        stat(thisfile.c_str(), &sdmstat);
#endif

        // Read the message header
        if (1 != fread(&sdmhead, sizeof (sdmhead_s), 1, msg))
        {
            display->WarningMessage(TDisplay::broken_msg, thisfile);
            goto out;
        }

        msglen = FILESIZE - sizeof (sdmhead_s);

        msgbuf = new char[msglen];
        if (!msgbuf)
        {
            display->WarningMessage(TDisplay::cannot_allocate_body_file,
                                    thisfile);
            goto out2;
        }

        ctrlbuf = new char[msglen];
        if (!ctrlbuf)
        {
            display->WarningMessage(TDisplay::cannot_allocate_control_file,
                                    thisfile);
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
        if (arrived >= starttime && arrived <= endtime)
        {
            destination.AddData(string((char *) sdmhead.fromusername),
                                string((char *) sdmhead.tousername),
                                string((char *) sdmhead.subject),
                                string(ctrlbuf), string(msgbuf),
                                written, arrived);
        }

        // Clean up our mess
out:;
        delete[] ctrlbuf;
out2:;
        delete[] msgbuf;

        display->UpdateProgress(++ msgn);
        if (msg) fclose(msg);

#ifdef HAVE_EMX_FINDFIRST
        rc = __findnext(&sdmdir);
#elif defined(HAVE_LIBCRTDLL)
        rc = _findnext(sdmhandle, &sdmdir);
#endif
    }

#ifdef HAVE_LIBCRTDLL
    _findclose(sdmhandle);
#elif !defined(HAVE_EMX_FINDFIRST)
    closedir(sdmdir);
#endif

    return true;
}
