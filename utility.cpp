// Copyright (c) 1998-2001 Peter Karlsson
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
#if defined(__EMX__)
# define INCL_DOSNLS
# include <os2.h>
#endif
#if defined(__WIN32__)
# include <windows.h>
#endif

#include "utility.h"
#include "mytime.h"

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
// Modified to use my_mktime to return a localtime time_t
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
    tt = my_mktime(&tms);
    return tt;
}

/** Month names as used in FTSC and RFC date-time strings. */
static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

// Convert FTSC style time-stamp to time_t (in local time)
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
    tms.tm_mon = int(c_p - months) / 3;

    // Check year
    // FIXME: This need to be corrected to handle dates >2080
    //        better use some sliding-window technique
    if (tms.tm_year < 80) tms.tm_year += 100;

    tt = my_mktime(&tms);
    return tt;
}

// Convert RFC stype time-stamp to time_t (local time, ignores time zone
// identifier)
time_t rfcToTimeT(const string &datetime)
{
    // "[Www, ]Dd Mmm [Yy]yy HH:MM:SS[ +ZZZZ]"
    time_t tt;
    struct tm tms;
    char month[4] = { 0,0,0,0 };
    int rc;

    // Chop weekday (if any)
    int pos = datetime.find(',');
    if (pos >= 0 && pos < 10) datetime = datetime.substr(pos + 1);

    // "[ ]Dd Mmm [Yy]yy HH:MM:SS[ +ZZZZ]"
    // Note: timezones are ignored!
    rc = sscanf(datetime.c_str(), "%d %s %d %d:%d:%d",
                &tms.tm_mday, month, &tms.tm_year,
                &tms.tm_hour, &tms.tm_min, &tms.tm_sec);

    if (rc != 6)
    {
        return time_t(-1);
    }

    // RFC years should be four-digit
    if (tms.tm_year >= 1900) tms.tm_year -= 1900;

    // Check month
    char *c_p = strstr(months, month);
    if (!c_p) return 0;
    tms.tm_mon = ((int) (c_p - months)) / 3;

    tt = my_mktime(&tms);
    return tt;
}

// Convert standard timespecs to time_t (in local time)
time_t timespectoTimeT(const string &datetime)
{
    time_tt tt;
    struct tm tms;
    int rc;

    // YyyyMmDdTHHMMSS or YyyyMmDd only
    if (8 == datetime.length())
    {
        rc = sscanf(datetime.c_str(), "%4d%2d%2d",
                    &tms.tm_year, &tms.tm_mon, &tms.tm_mday);
        if (rc != 3)
        {
            return time_t(-1);
        }
        tms.tm_hour = 0;
        tms.tm_min  = 0;
        tms.tm_sec  = 0;
    }
    else if (15 == datetime.length())
    {
        rc = sscanf(datetime.c_str(), "%4d%2d%2dT%2d%2d%2d",
                    &tms.tm_year, &tms.tm_mon, &tms.tm_mday,
                    &tms.tm_hour, &tms.tm_min, &tms.tm_sec);
        if (rc != 6)
        {
            return time_t(-1);
        }
    }
    else
    {
        return time_t(-1);
    }

    // Years are always four digit here
    tms.tm_year -= 1900;
    // struct tm Months are zero based
    tms.tm_mon --;

    tt = my_mktime(&tms);
    return tt;
}

// Copy out kludges and body to separate buffers
// kludges go into ctrlbuf, body stays in buf, but is
// relocated.
void fixupctrlbuffer(char *body_p, char *ctrl_p)
{
    char *newbody_p = body_p;
    char *nextseenby_p = strstr(body_p, "SEEN-BY");

    if (!body_p) return;

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

#if defined(__EMX__)
void localetimestring(const struct tm *time, size_t len, char *out)
{
    static _COUNTRYINFO countryinfo;
    static bool datavalid = false;

    if (!datavalid)
    {
        // Retrieve locale information on first call
        COUNTRYCODE country = { 0, 0 };
        ULONG datalength;
        DosQueryCtryInfo(sizeof countryinfo, &country, &countryinfo,
                         &datalength);
        datavalid = true;
    }

    int usedlength;

    // First print date
    switch (countryinfo.fsDateFmt)
    {
        case 0: // MDY
            usedlength = snprintf(out, len, "%02%s%02d%s%04d",
                                  time->tm_mon + 1,
                                  countryinfo.szDateSeparator,
                                  time->tm_mday,
                                  countryinfo.szDateSeparator,
                                  time->tm_year + 1900);
            break;

        case 1: // DMY
            usedlength = snprintf(out, len, "%02%s%02d%s%04d",
                                  time->tm_mday,
                                  countryinfo.szDateSeparator,
                                  time->tm_mon + 1,
                                  countryinfo.szDateSeparator,
                                  time->tm_year + 1900);
            break;

        case 2: // YMD
            usedlength = snprintf(out, len, "%04d%s%02d%s%02d",
                                  time->tm_year + 1900,
                                  countryinfo.szDateSeparator,
                                  time->tm_mon + 1,
                                  countryinfo.szDateSeparator,
                                  time->tm_mday);
            break;

    }
    out += usedlength;
    len -= usedlength;

    // Then print time
    char *ampm = "";
    int hour = time->tm_hour;
    if (0 == countryinfo.fsTimeFmt)
    {
        // 12-hour clock
        if (hour >= 12)
        {
            ampm = "PM";
            hour -= 12;
        }
        else
        {
            ampm = "AM";
        }

        // Hour 0 = hour 12
        if (0 == hour)
        {
            hour += 12;
        }
    }

    snprintf(out, len, " %d%s%02d%s",
             hour,
             countryinfo.szTimeSeparator,
             time->tm_min,
             ampm);
}
#elif defined(__WIN32__)
void localetimestring(const struct tm *time, size_t len, char *out)
{
    // Convert C time struct to WinAPI time struct
    // (why Micro$oft had to re-invent the wheel and use another structure
    //  is beyond me)
    SYSTEMTIME wintime;
    wintime.wHour       = time->tm_hour;
    wintime.wMinute     = time->tm_min;
    wintime.wSecond     = time->tm_sec;
    wintime.wYear       = time->tm_year + 1900;
    wintime.wMonth      = time->tm_mon + 1;
    wintime.wDay        = time->tm_mday;

    // First print date
    int usedlength =
        GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &wintime, NULL,
                      out, len);

    len -= usedlength;
    out += usedlength;
    if (usedlength)
    {
        // GetDateFormat null terminates and includes the null in the
        // count
        out[-1] = ' ';
    }

    // Then print time
    GetTimeFormat(LOCALE_USER_DEFAULT, 0, &wintime, NULL, out, len);
}
#elif defined(HAVE_LOCALE_H)
void localetimestring(const struct tm *time, size_t len, char *out)
{
    strftime(out, len, "%x %X", time);
}
#endif


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
                    cerr << "Option requires an argument: "
                         << (char) option << endl;
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
