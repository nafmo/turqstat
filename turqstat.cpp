// Turquoise SuperStat
//
// A statistic collection program for Fidonet systems
// Version 1.2
//
// Copyright (c) 1998-1999 Peter Karlsson
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
#include <string>
#include <unistd.h>
#include <stdlib.h>
#ifdef __CYGWIN__
# include <getopt.h>
#endif

#include "statengine.h"
#include "statview.h"
#include "arearead.h"
#include "squishread.h"
#include "fdapxread.h"
#include "jamread.h"
#include "mypointread.h"
#include "sdmread.h"
#include "version.h"

class StatRetr
{
public:
    enum basetype_e { squish, sdm, opus, fdapx, jam, mypoint };

    StatRetr(char *areapath, char *outputfilepath, unsigned areanum,
             unsigned days,
             basetype_e basetype, unsigned maxnumber,
             bool quoters, bool topwritten, bool topreceived,
             bool topsubjects, bool topprograms,
             bool weekstats, bool daystats, bool showversions,
             bool showallnums);
    ~StatRetr();
};

int main(int argc, char *argv[])
{
    unsigned days = 0;
    unsigned maxnum = 15;
    unsigned areanum = 0;
    StatRetr::basetype_e basetype = StatRetr::squish;
    bool quoters = true, topwritten = true, topreceived = true,
         topsubjects = true, topprograms = true, weekstats = true,
         daystats = true, versions = true, allnums = false;

    // We don't want timezones here
#if defined(__GNUC__) || defined(__EMX__)
    timezone = 0;
    daylight = 0;
#endif

    // Display banner
    cout << "Turquoise " << version
         << " - Statistics tool for Fidonet message bases"
         << endl;
    cout << "(c) Copyright 1998-1999 Peter Karlsson. "
            "GNU GPL 2 licensed." << endl;
    cout << "A Softwolves Software Release in 1999." << endl;
    cout << endl;

    // Handle arguments
    int c;
    while (EOF != (c = getopt(argc, argv, "d:n:a:smofjpQWRSPHDVN?")))
    {
        switch (c)
        {
            case 'd':   days = strtoul(optarg, NULL, 10);           break;
            case 'n':   maxnum = strtoul(optarg, NULL, 10);         break;
            case 'a':   areanum = strtoul(optarg, NULL, 10);        break;

            case 's':   basetype = StatRetr::squish;                break;
            case 'm':   basetype = StatRetr::sdm;                   break;
            case 'o':   basetype = StatRetr::opus;                  break;
            case 'f':   basetype = StatRetr::fdapx;                 break;
            case 'j':   basetype = StatRetr::jam;                   break;
            case 'p':   basetype = StatRetr::mypoint;               break;

            case 'Q':   quoters = false;                            break;
            case 'W':   topwritten = false;                         break;
            case 'R':   topreceived = false;                        break;
            case 'S':   topsubjects = false;                        break;
            case 'P':   topprograms = false;                        break;
            case 'H':   daystats = false;                           break;
            case 'D':   weekstats = false;                          break;

            case 'V':   versions = false;                           break;
            case 'N':   allnums = true;                             break;

            case '?':
            default:
                cout << "Usage: turqstat [options] areapath outputfile" << endl;
                cout << endl;
                cout << "Available options:" << endl;
                cout << "  -d days        Days back to count messages from"
                     << endl;
                cout << "  -n num         Maximum entries in toplists" << endl;
                cout << "  -a num         Area number (for FDAPX/w and MyPoint)" << endl;
                cout << endl;
                cout << "  -s             Squish style message area (default)"
                     << endl;
                cout << "  -m             FTSC *.MSG style message area"
                     << endl;
                cout << "  -o             Opus *.MSG style message area"
                     << endl;
                cout << "  -j             JAM style message base" << endl;
                cout << "  -f             FDAPX/w style message base (needs -a)"
                     << endl;
                cout << "  -p             MyPoint style message base (needs -a)"
                     << endl;
                cout << endl;
                cout << "  -Q -W -R -S -P Turn quoters/written/received/"
                                         "subjects/programs toplist off"
                     << endl;
                cout << "  -H -D          Turn hour/day statistics off"
                     << endl;
                cout << "  -V             Don't provide version info for "
                                         "programs toplist"
                     << endl;
                cout << "  -N             Show all numbers in toplists, even "
                                         "for same score"
                     << endl;
                return 1;
        }
    }

    if (argc - optind != 2)
    {
        cerr << "Illegal number of arguments (-? for help)." << endl;
        return 1;
    }

    StatRetr s(argv[optind], argv[optind + 1], areanum, days,
               basetype, maxnum, quoters, topwritten, topreceived, topsubjects,
               topprograms, weekstats, daystats, versions, allnums);

    return 0;
}

StatRetr::StatRetr(char *areapath, char *outputfilepath, unsigned areanum,
                   unsigned days,
                   basetype_e basetype, unsigned maxnumber,
                   bool quoters, bool topwritten, bool topreceived,
                   bool topsubjects, bool topprograms,
                   bool weekstats, bool daystats, bool showversions,
                   bool showallnums)
{
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
    AreaRead *area = NULL;
    switch (basetype)
    {
        case squish:
            area = new SquishRead(areapath);
            break;

        case sdm:
        case opus:
            area = new SdmRead(areapath, basetype == opus);
            break;

        case fdapx:
            area = new FdApxRead(areapath, areanum);
            break;

        case jam:
            area = new JamRead(areapath);
            break;

        case mypoint:
            area = new MyPointRead(areapath, areanum);
            break;

        default:
            cerr << "Internal error." << endl;
            exit(1);
    }

    StatEngine engine;
    if (!(area->Transfer(from, engine))) return;

    // Create output
    StatView view;
    view.CreateReport(&engine, outputfilepath, maxnumber,
                      quoters, topwritten, topreceived, topsubjects,
                      topprograms, weekstats, daystats, showversions,
                      showallnums);
}

StatRetr::~StatRetr()
{
    cout << "Turquoise finished." << endl;
}
