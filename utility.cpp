// Copyright (c) 1998-2000 Peter Karlsson
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
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef USE_OWN_GETOPT
# include <string.h>
#endif

#include "utility.h"

// Compare two strings case in-sensitively
// Why isn't this functionality available in ANSI C++? *sigh*
int fcompare(const string &s1, const string &s2, unsigned int max)
{
    int ls1 = s1.length() <? max, ls2 = s2.length() <? max;

    for (int i = 0; i < ls1 && i < ls2; i ++)
    {
        if (toupper(s1[i]) != toupper(s2[i]))
        {
            return toupper(s1[i]) - toupper(s2[i]);
        }
    }

    // If we fall out, the shortest one is smallest. If we have counted to
    // max, ls1 == ls2, which gives 0.
    return ls1 - ls2;
}

// The following is from MsgEd 4.30:
//  Written on 10-Jul-94 by John Dennis and released to the public domain.
time_t stampToTimeT(struct stamp_s *st)
{
    time_t tt;
    struct tm tms;
    if (0 == st->date.da || 0 == st->date.mo)
    {
        return 0;
    }
    tms.tm_sec = st->time.ss << 1;
    tms.tm_min = st->time.mm;
    tms.tm_hour = st->time.hh;
    tms.tm_mday = st->date.da;
    tms.tm_mon = st->date.mo - 1;
    tms.tm_year = st->date.yr + 80;
    tms.tm_isdst = 0;
#ifdef HAS_TM_GMTOFF
    tms.tm_gmtoff = 0;
#endif
    tt = mktime(&tms);
    return tt;
}

static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

// Convert FTSC style time-stamp to time_t
time_t asciiToTimeT(const char *datetime)
{
    time_t tt;
    struct tm tms;
    char month[4] = { 0,0,0,0 };

    if (' ' == datetime[2])
    { // "Dd Mmm Yy  HH:MM:SS"
        sscanf(datetime, "%d %s %d %d:%d:%d",
               &tms.tm_mday, month, &tms.tm_year,
               &tms.tm_hour, &tms.tm_min, &tms.tm_sec);
    }
    else if (' ' == datetime[3])
    { // "Www Dd Mmm Yy HH:MM"
        sscanf(&datetime[4], "%d %s %d %d:%d",
               &tms.tm_mday, month, &tms.tm_year,
               &tms.tm_hour, &tms.tm_min);
        tms.tm_sec = 0;
    }
    else
        return 0;

    // Check month
    char *c_p = strstr(months, month);
    if (!c_p) return 0;
    tms.tm_mon = ((int) (c_p - months)) / 3;

    // Check year
    // FIXME: This need to be corrected to handle dates >2080
    //        better use some sliding-window technique
    if (tms.tm_year < 80) tms.tm_year += 100;

    tms.tm_isdst = 0;
#ifdef HAS_TM_GMTOFF
    tms.tm_gmtoff = 0;
#endif

    tt = mktime(&tms);
    return tt;
}

// Convert RFC stype time-stamp to time_t
time_t rfcToTimeT(string datetime)
{
    // "[Www, ]Dd Mmm [Yy]yy HH:MM:SS[ +ZZZZ]"
    time_t tt;
    struct tm tms;
    char month[4] = { 0,0,0,0 };

    // Chop weekday (if any)
    int pos = datetime.find(',');
    if (pos >= 0 && pos < 10) datetime = datetime.substr(pos + 1);

    // "[ ]Dd Mmm [Yy]yy HH:MM:SS[ +ZZZZ]"
    // Note: timezones are ignored!
    sscanf(datetime.c_str(), "%d %s %d %d:%d:%d",
           &tms.tm_mday, month, &tms.tm_year,
           &tms.tm_hour, &tms.tm_min, &tms.tm_sec);

    // RFC years should be four-digit
    if (tms.tm_year >= 1900) tms.tm_year -= 1900;

    // Check month
    char *c_p = strstr(months, month);
    if (!c_p) return 0;
    tms.tm_mon = ((int) (c_p - months)) / 3;

    tms.tm_isdst = 0;
#ifdef HAS_TM_GMTOFF
    tms.tm_gmtoff = 0;
#endif

    tt = mktime(&tms);
    return tt;
}

// Copy out kludges and body to separate buffers
// kludges go into ctrlbuf, body stays in buf, but is
// relocated.
void fixupctrlbuffer(char *body_p, char *ctrl_p)
{
    char *newbody_p = body_p;
    char *nextseenby_p = strstr(body_p, "SEEN-BY");

    bool iskludge = false;
    bool wascr = true;
    while (*body_p)
    {
        if (wascr && 1 == *body_p)
        {
            iskludge = true;
        }
        else if (wascr && nextseenby_p == body_p)
        {
            // SEEN-BY doesn't start with ^A
            iskludge = true;
            nextseenby_p = strstr(body_p + 1, "SEEN-BY");
            if (ctrl_p) *(ctrl_p ++) = 1;
        }

        if (iskludge)
        {
            // We don't do CR/LF in the kludge buffer
            if ('\r' != *body_p && '\n' != *body_p && ctrl_p)
                *(ctrl_p ++) = *body_p;
        }
        else
        {
            *(newbody_p ++) = *body_p;
        }

        if ('\r' == *body_p || '\n' == *body_p)
        {
            iskludge = false;
            wascr = true;
        }
        else
        {
            wascr = false;
        }

        body_p ++;

        if (nextseenby_p && body_p > nextseenby_p)
        {
            // If we go past a SEEN-BY embedded in the text, search for the
            // next occurance.
            nextseenby_p = strstr(body_p, "SEEN-BY");
        }
    }

    // Zero terminate what we got
    if (ctrl_p) *ctrl_p = 0;
    *newbody_p = 0;
}

// Print error message and quit
void errorquit(const char *message, int rc)
{
    if (message)
        cerr << message << endl;
    else
        internalerrorquit("No error message", rc);
    cerr << "Program halted";
    exit(rc);
}

const char *area_not_allocated = "Area path was not allocated properly.";
const char *out_of_memory = "Out of memory.";

// Print internal error message and quit
void internalerrorquit(const char *message, int rc)
{
    if (message)
       cerr << "Internal error: " << message << endl;
    else
       cerr << "Internal error." << endl;
    cerr << "Program halted";
    exit(rc);
}

#ifdef USE_OWN_GETOPT
int optind = 0;
char *optarg = NULL;

// Retrieve command line parameters
int getopt(int _argc, char **_argv, const char *opts)
{
    static int curarg = 1, curind = 0;

redo:
    // Have we reached the last or a non-option argument?
    if (curarg >= _argc || (0 == curind && *_argv[curarg] != '-'))
    {
        optind = curarg;
        return EOF;
    }

    // Check for -- and end if so
    if (0 == curind && '-' == _argv[curarg][1])
    {
        optind = curarg + 1;
        return EOF;
    }

    // Skip dash
    if (0 == curind) curind ++;

    // Move to next argument if at end
    if (0 == _argv[curarg][curind])
    {
        curarg ++;
        optind = curarg;
        curind = 0;
        goto redo;
    }

    // Check this option
    int option = (int) (unsigned char) _argv[curarg][curind];

    char *p;
    if (NULL != (p = strchr(opts, option)))
    {
        // Does it take an argument?
        if (':' == *(p + 1))
        {
            // Yes.
            // More characters in this parameter?
            curind ++;
            if (_argv[curarg][curind] == '\0')
            {
                // No, next parameter is argument
                if (curarg + 1 >= _argc)
                {
                    fprintf(stderr, "Option requires an argument: %c\n",
                            (char) option);
                    return (int) (unsigned char) '?';
                }
                optarg = _argv[curarg + 1];
                curarg += 2;
                curind = 0;
            }
            else
            {
                // Yes, rest of this parameter is argument
                optarg = &_argv[curarg][curind];
                curarg ++;
                curind = 0;
            }
        }
        else
        {
            // No.
            // Point to next character for next run
            curind ++;
            optarg = NULL;
        }
        return option;
    }
    curind ++;

    // Unknown
    return (int) (unsigned char) '?';
}
#endif // USE_OWN_GETOPT
