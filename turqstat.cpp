// Turqoise SuperStat
//
// A statistic collection program for Fidonet systems
// Version 1.0
//
// Copyright (c) 1998-1999 Peter Karlsson
//
// $Id$
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
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
#include <string>
#include <unistd.h>
#include <stdlib.h>

#include "statengine.h"
#include "statview.h"
#include "arearead.h"
#include "squishread.h"
#include "fdapxread.h"

class StatRetr
{
public:
    enum basetype_e { unspecified, squish, sdm, fdapx };

    StatRetr(char *areapath, char *outputfilepath, unsigned areanum,
             unsigned days,
             basetype_e basetype, unsigned maxnumber,
             bool quoters, bool topwritten, bool topreceived,
             bool topsubjects, bool topprograms,
             bool weekstats, bool daystats);
    ~StatRetr();
};

int main(int argc, char *argv[])
{
    unsigned days = 0;
    unsigned maxnum = 15;
    unsigned areanum = 0;
#ifdef HAS_SMAPI
    StatRetr::basetype_e basetype = StatRetr::squish;
#else
    StatRetr::basetype_e basetype = StatRetr::unspecified;
#endif
    bool quoters = true, topwritten = true, topreceived = true,
         topsubjects = true, topprograms = true, weekstats = true,
         daystats = true;

    // We don't want timezones here
    timezone = 0;
    daylight = 0;

    // Handle arguments
    int c;
    while (EOF != (c = getopt(argc, argv, "d:n:a:smfQWRSPHD?")))
    {
        switch (c)
        {
            case 'd':   days = strtoul(optarg, NULL, 10);           break;
            case 'n':   maxnum = strtoul(optarg, NULL, 10);         break;
            case 'a':   areanum = strtoul(optarg, NULL, 10);        break;

#ifdef HAS_SMAPI
            case 's':   basetype = StatRetr::squish;                break;
            case 'm':   basetype = StatRetr::sdm;                   break;
#else
            case 's':
            case 'm':   cerr << "Message base format not supported "
                                "in this version" << endl;
                        return 1;
#endif
            case 'f':   basetype = StatRetr::fdapx;                 break;

            case 'Q':   quoters = false;                            break;
            case 'W':   topwritten = false;                         break;
            case 'R':   topreceived = false;                        break;
            case 'S':   topsubjects = false;                        break;
            case 'P':   topprograms = false;                        break;
            case 'H':   daystats = false;                           break;
            case 'D':   weekstats = false;                          break;

            case '?':
            default:
                cout << "Turqoise - Statistics tool for Fidonet message bases"
                     << endl;
                cout << endl;
                cout << "Usage: turqoise [options] areapath outputfile" << endl;
                cout << endl;
                cout << "Available options:" << endl;
                cout << "  -d days        Days back to count messages from"
                     << endl;
                cout << "  -n num         Maximum entries in toplists" << endl;
                cout << "  -a num         Area number (for FDAPX/w)" << endl;
                cout << endl;
#ifdef HAS_SMAPI
                cout << "  -s             Squish style message area (default)"
                     << endl;
                cout << "  -m             *.MSG style message area"
                     << endl;
#endif
                cout << "  -f            FDAPX/w style message base (needs -a)"
                     << endl;
                cout << endl;
                cout << "  -Q -W -R -S -P Turn quoters/written/received/"
                                         "subjects/programs toplist off"
                     << endl;
                cout << "  -H -D          Turn hour/day statistics off"
                     << endl;
                return 1;
        }
    }

    if (argc - optind != 2)
    {
        cerr << "Illegal number of arguments (-? for help)." << endl;
        return 1;
    }

#ifndef HAS_SMAPI
    if (StatRetr::unspecified == basetype)
    {
        cerr << "No message base format was specified." << endl;
        return 1;
    }
#endif

    StatRetr s(argv[optind], argv[optind + 1], areanum, days,
               basetype, maxnum, quoters, topwritten, topreceived, topsubjects,
               topprograms, weekstats, daystats);

    return 0;
}

StatRetr::StatRetr(char *areapath, char *outputfilepath, unsigned areanum,
                   unsigned days,
                   basetype_e basetype, unsigned maxnumber,
                   bool quoters, bool topwritten, bool topreceived,
                   bool topsubjects, bool topprograms,
                   bool weekstats, bool daystats)
{
    cout << "Turqoise SuperStat 0.1 (c) Copyright 1998-1999 Peter Karlsson."
         << endl;

    // Compute starting time
    time_t from;
    if (0 == days)
        from = 0;
    else
        from = time(NULL) - days * 86400L;
    struct tm *fromtm = localtime(&from);
    fromtm->tm_hour = 0;
    fromtm->tm_min  = 0;
    fromtm->tm_sec  = 0;
    from = mktime(fromtm);

    // Transfer from area to engine
    AreaRead *area;
    switch (basetype)
    {
#ifdef HAS_SMAPI
        case squish:
        case sdm:
            area = new SquishRead(areapath, sdm == basetype);
            break;
#endif

        case fdapx:
            area = new FdApxRead(areapath, areanum);
            break;

        default:
            cerr << "Internal error." << endl;
            exit(1);
    }

    StatEngine engine;
    area->Transfer(from, engine);

    // Create output
    StatView view;
    view.CreateReport(&engine, outputfilepath, maxnumber,
                      quoters, topwritten, topreceived, topsubjects,
                      topprograms, weekstats, daystats);
}

StatRetr::~StatRetr()
{
    cout << "Turqoise finished." << endl;
}
