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

#ifndef __UTILITY_H
#define __UTILITY_H

#include <string>
#include <time.h>

// MS-DOS style time stamp
#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

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

// Compare two strings case in-sensitively
int fcompare(const string &s1, const string &s2);
// Convert DOS style time-stamp to time_t
time_t stampToTimeT(struct stamp_s *st);
// Convert FTSC style time-stamp to time_t
time_t asciiToTimeT(const char *datetime);
// Convert RFC stype time-stamp to time_t
time_t rfcToTimeT(string datetime);
// Separate kludges and body
void fixupctrlbuffer(char *body_p, char *ctrl_p);
// Print error message and quit
void errorquit(const char *message, int rc);
// Print internal error message and quit
void internalerrorquit(const char *message, int rc);

// Error messages for errorquit
extern const char *area_not_allocated;
extern const char *out_of_memory;

#ifdef USE_OWN_GETOPT
// Index to next non-option parameter
extern int optind;
// Pointer to option argument
extern char *optarg;
// Retrieve command line parameters
int getopt(int _argc, char **_argv, const char *opts);
#endif

#endif
