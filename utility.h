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

#ifndef __UTILITY_H
#define __UTILITY_H

#include <string>
#include <time.h>
#include <limits.h>

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

/** MS-DOS style time stamp. */
struct stamp_s
{
    struct
    {
        unsigned short da: 5;
        unsigned short mo: 4;
        unsigned short yr: 7;
    } date;

    struct
    {
        unsigned short ss: 5;
        unsigned short mm: 6;
        unsigned short hh: 5;
    } time;
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

/**
 * Compare two strings case in-sensitively.
 * @param s1  First string to compare.
 * @param s2  Second string to compare.
 * @param max Maximum number of characters to compare.
 * @return Less than zero if s1 < s2, zero if equal,
 *         greater than zero if s2 > s1.
 */
int fcompare(const string &s1, const string &s2, unsigned int max = UINT_MAX);

/**
 * Convert DOS style time-stamp to time_t.
 * @param st Pointer to DOS style time-stamp.
 * @return Number of seconds since epoch (ignoring timezones).
 */
time_t stampToTimeT(struct stamp_s *st);

/**
 * Convert FTSC style date-time string to time_t.
 * @param datetime Pointer to FTSC style date-time string.
 * @return Number of seconds since epoch (ignoring timezones).
 */
time_t asciiToTimeT(const char *datetime);

/**
 * Convert RFC stype date-time string to time_t.
 * @param datetime Pointer to RFC style date-time string.
 * @return Number of seconds since epoch (ignoring timezones).
 */
time_t rfcToTimeT(string datetime);

/**
 * Separate kludges and body in a Fidonet mesage.
 * @param body_p Pointer to mesasge body containing both kludges and body
 *               text, will on exit only contain the body text.
 * @param ctrl_p Pointer to pre-allocated buffer where kludges will be
 *               written. Should be allocated as large as body_p. If set to
 *               NULL, kludge data will be thrown away.
 */
void fixupctrlbuffer(char *body_p, char *ctrl_p);

#if defined(HAVE_LOCALE_H) || defined(__EMX__) || defined(__WIN32__)
/**
 * Create a string in the locale defined date format.
 * @param time Pointer to a structure containing the time to be created.
 * @param len  Maximum length of buffer.
 * @param out  Pointer to a string where the locale date will be saved.
 */
void localetimestring(const struct tm *time, size_t len, char *out);
#endif

#ifdef USE_OWN_GETOPT
/** Index to next non-option parameter. */
extern int optind;

/** Pointer to option argument. */
extern char *optarg;

/**
 * Retrieve command line options.
 * @param _argc Original argc parameter passed to main.
 * @param _argv Pointer to the argv array passed to main.
 * @param opts  String describing valid option characters. Suffix option
 *              character with colon (":") to indicate that the option takes
 *              an argument (stored in optarg).
 * @return Next option on command line, or '?' if an invalid option was found.
 */
int getopt(int _argc, char **_argv, const char *opts);
#endif

#endif
