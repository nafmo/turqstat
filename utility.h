// Copyright (c) 1998-2005 Peter Karlsson
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

#include <config.h>
#include <string>
#include <time.h>
#include <limits.h>

#if !defined(HAVE_IMPLICIT_NAMESPACE)
using namespace std;
#endif

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

/** Highest possible time_t value */
#if !defined(HAVE_CHAR_BIT)
# define CHAR_BIT 8
#endif

#define DISTANT_FUTURE ((1UL << (CHAR_BIT * SIZEOF_TIME_T - 1)) - 1)

/**
 * Compare two strings case in-sensitively.
 * @param s1  First string to compare.
 * @param s2  Second string to compare.
 * @param max Maximum number of characters to compare.
 * @return Less than zero if s1 < s2, zero if equal,
 *         greater than zero if s2 > s1.
 */
int fcompare(const string &s1, const string &s2, unsigned int max = UINT_MAX);

#ifndef HAVE_WORKING_WSTRING
class wstring;
#endif

/**
 * Compare two wide strings case in-sensitively.
 * @param s1  First string to compare.
 * @param s2  Second string to compare.
 * @param max Maximum number of characters to compare.
 * @return Less than zero if s1 < s2, zero if equal,
 *         greater than zero if s2 > s1.
 */
int fcompare(const wstring &s1, const wstring &s2, unsigned int max = UINT_MAX);

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
 * @param datetime RFC style date-time string.
 * @return Number of seconds since epoch (ignoring timezones). -1 on error.
 */
time_t rfcToTimeT(string datetime);

/**
 * Convert time specification as used in command line date ranges to time_t.
 * @param datetime Time specification string.
 * @return Number of seconds since epoch (ignoring timezones). -1 on error.
 */
time_t timespecToTimeT(const string &datetime);

/**
 * Separate kludges and body in a Fidonet mesage.
 * @param body_p Pointer to mesasge body containing both kludges and body
 *               text, will on exit only contain the body text.
 * @param ctrl_p Pointer to pre-allocated buffer where kludges will be
 *               written. Should be allocated as large as body_p. If set to
 *               NULL, kludge data will be thrown away.
 */
void fixupctrlbuffer(char *body_p, char *ctrl_p);

#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
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
#endif // USE_OWN_GETOPT

#if !defined(HAVE_WORKING_WSTRING)
# if defined(HAVE_WCHAR_H)
#  include <wchar.h>
# endif
/**
 * Simple wide-string class to use instead of the C++ standard one.
 * Provides justa about enough functionality for use with this program.
 */
class wstring
{
public:
    /**
     * Create a wide string object pre-allocating size for it.
     * Growing can be done, but it is not very efficient.
     * @param n Number of characters to allocate space for. Include room
     *          for a terminating null character.
     */
    wstring(size_t n = 32);

    /**
     * Create a wide string object by copying an existing one.
     * Growing can be done, but it is not very efficient.
     * @param s String to copy.
     */
    wstring(const wstring &s);

    /**
     * Create a wide string object by copying a wide character C string.
     * Growing can be done, but it is not very efficient.
     * @param s String to copy.
     */
    wstring(const wchar_t *s);

    /**
     * Standard destructor.
     */
    ~wstring();

    /**
     * Assignment operator. Copies the data of another wstring into this
     * string.
     * @param s String to copy.
     */
    wstring &operator=(const wstring &s);

    /**
     * Append characters to the end of the string.
     * @param s String to append.
     */
    void append(const wstring &s);

    /**
     * Append a character to the end of the string.
     * @param c Character to append.
     */
    void append(wchar_t c);

    /**
     * Retrieve string as a C wide string pointer.
     * @return A pointer to the internal data.
     */
    inline wchar_t *c_str() const { return data_p; };

    /**
     * Retrieve a character at an index.
     * @param n Index to retrieve character at.
     * @return The character requested.
     */
    wchar_t operator[](size_t n) const;

    /**
     * Retrieve length of the string.
     * @return Number of characters (not bytes) in string.
     */
# if defined(HAVE_WCHAR_H)
    inline size_t length() const { return wcslen(data_p); };
# else
    inline size_t length() const { return length(data_p); };
# endif

    /**
     * Remove the first characters of the string.
     * @param n Number of characters to skip
     */
    void skip(size_t n);

private:
    /** Contained string. */
    wchar_t *data_p;

    /** Length of data buffer in characters (not bytes). */
    size_t size;

    /**
     * Replacement for the wcscpy library function.
     * @param s The string to copy into this one.
     */
# if defined(HAVE_WCHAR_H)
    inline void copy(const wchar_t *s) { wcscpy(data_p, s); };
# else
    inline void copy(const wchar_t *s) { copy(data_p, s); };
# endif

    /**
     * Replacement for the wcscpy library function.
     * @param d The destination to copy to.
     * @param s The string to copy into this one.
     */
# if defined(HAVE_WCHAR_H)
    static inline void copy(wchar_t *d, const wchar_t *s)
        { wcscpy(d, s); };
# else
    static void copy(wchar_t *d, const wchar_t *s);
# endif

    /**
     * Replacement for the wcslen library function.
     * @param s The string to check the length of.
     * @return Number of characters in string.
     */
# if defined(HAVE_WCHAR_H)
    static inline size_t length(const wchar_t *s)
        { return wcslen(s); };
# else
    static size_t length(const wchar_t *s);
# endif
};
#endif // !HAVE_WORKING_WSTRING

#endif
