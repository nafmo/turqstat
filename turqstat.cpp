// Turquoise SuperStat
//
// A statistic collection program for Fidonet and Usenet systems
// Version 1.3
//
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
#include "tanstaaflread.h"
#include "newsspoolread.h"
#include "version.h"
#include "utility.h"

class StatRetr
{
public:
    enum basetype_e { squish, sdm, opus, fdapx, jam, mypoint, tanstaafl,
                      newsspool };

    StatRetr(char **areapath, int numpaths, char *outputfilepath,
             char *basepath,
             unsigned days,
             basetype_e basetype, unsigned maxnumber,
             bool quoters, bool topwritten, bool topreceived,
             bool topsubjects, bool topprograms,
             bool weekstats, bool daystats, bool showversions,
             bool showallnums, bool toporiginal);
    ~StatRetr();
};

int main(int argc, char *argv[])
{
    unsigned days = 0;
    unsigned maxnum = 15;
    char *basepath = NULL;
    StatRetr::basetype_e basetype = StatRetr::squish;
    bool quoters = true, topwritten = true, topreceived = true,
         topsubjects = true, topprograms = true, weekstats = true,
         daystats = true, versions = true, allnums = false,
         toporiginal = true;

    // We don't want timezones here
#if defined(__GNUC__) || defined(__EMX__)
# if !defined(__CYGWIN__)
    timezone = 0;
    daylight = 0;
# endif
#endif

    // Display banner
    cout << "Turquoise " << version
         << " - Statistics tool for Fidonet and Usenet message bases"
         << endl;
    cout << copyright << ". GNU GPL 2 licensed." << endl;
    cout << "A Softwolves Software Release in " << releaseyear << "." << endl;
    cout << endl;

    // Handle arguments
    int c;
    while (EOF != (c = getopt(argc, argv, "d:n:a:smofjptuQWRSPOHDVN?")))
    {
        switch (c)
        {
            case 'd':   days = strtoul(optarg, NULL, 10);           break;
            case 'n':   maxnum = strtoul(optarg, NULL, 10);         break;
            case 'a':   basepath = optarg;                          break;

            case 's':   basetype = StatRetr::squish;                break;
            case 'm':   basetype = StatRetr::sdm;                   break;
            case 'o':   basetype = StatRetr::opus;                  break;
            case 'f':   basetype = StatRetr::fdapx;                 break;
            case 'j':   basetype = StatRetr::jam;                   break;
            case 'p':   basetype = StatRetr::mypoint;               break;
            case 't':   basetype = StatRetr::tanstaafl;             break;
            case 'u':   basetype = StatRetr::newsspool;             break;

            case 'Q':   quoters = false;                            break;
            case 'W':   topwritten = false;                         break;
            case 'R':   topreceived = false;                        break;
            case 'S':   topsubjects = false;                        break;
            case 'P':   topprograms = false;                        break;
            case 'O':   toporiginal = false;                        break;
            case 'H':   daystats = false;                           break;
            case 'D':   weekstats = false;                          break;

            case 'V':   versions = false;                           break;
            case 'N':   allnums = true;                             break;

            case '?':
            default:
                cout << "Usage: turqstat [options] outputfile areadef(s)" << endl;
                cout << endl;
                cout << "Available options:" << endl;
                cout << "  -d days        Days back to count messages from"
                     << endl;
                cout << "  -n num         Maximum entries in toplists" << endl;
                cout << "  -a path        Base path (for FDAPX/w and MyPoint)" << endl;
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
                cout << "  -t             Tanstaafl style message base (needs -a)"
                     << endl;
                cout << "  -p             MyPoint style message base (needs -a)"
                     << endl;
                cout << "  -u             Usenet News spool"
                     << endl;
                cout << endl;
                cout << "  -Q -W -R -S -P Turn quoters/written/received/"
                                         "subjects/programs toplist off"
                     << endl;
                cout << "  -O             Turn original per msg toplist off"
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

    if (argc - optind < 2)
    {
        cerr << "Illegal number of arguments (-? for help)." << endl;
        return 1;
    }

    StatRetr s(&argv[optind + 1], argc - optind - 1, argv[optind],
               basepath, days, basetype, maxnum, quoters, topwritten,
               topreceived, topsubjects, topprograms, weekstats, daystats,
               versions, allnums, toporiginal);

    return 0;
}

StatRetr::StatRetr(char **areapath, int numpaths, char *outputfilepath,
                   char *basepath,
                   unsigned days,
                   basetype_e basetype, unsigned maxnumber,
                   bool quoters, bool topwritten, bool topreceived,
                   bool topsubjects, bool topprograms,
                   bool weekstats, bool daystats, bool showversions,
                   bool showallnums, bool toporiginal)
{
    unsigned areanum;

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

    // Transfer from area(s) to engine
    StatEngine engine;
    AreaRead *area;
    int counter = 0;
    while (numpaths)
    {
        area = NULL;
        switch (basetype)
        {
            case squish:
                area = new SquishRead(areapath[counter]);
                break;

            case sdm:
            case opus:
                area = new SdmRead(areapath[counter], basetype == opus);
                break;

            case fdapx:
                areanum = strtoul(areapath[counter], NULL, 10);
                area = new FdApxRead(basepath, areanum);
                break;

            case jam:
                area = new JamRead(areapath[counter]);
                break;

            case mypoint:
                areanum = strtoul(areapath[counter], NULL, 10);
                area = new MyPointRead(basepath, areanum);
                break;

            case tanstaafl:
                areanum = strtoul(areapath[counter], NULL, 10);
                area = new TanstaaflRead(basepath, areanum);
                break;

            case newsspool:
                area = new NewsSpoolRead(areapath[counter]);
                break;

            default:
                internalerrorquit("Message base format mismatch.", 1);
                exit(1);
        }

        if (!area)
            errorquit("Out of memory allocating area object.", 1);
        if (!(area->Transfer(from, engine))) return;

        engine.AreaDone();
        cout << "Finished reading " << areapath[counter] << endl;

        delete area;
        counter ++;
        numpaths --;
    }

    // Create output
    StatView view;
    view.CreateReport(&engine, outputfilepath, maxnumber,
                      quoters, topwritten, topreceived, topsubjects,
                      topprograms, weekstats, daystats, showversions,
                      showallnums, toporiginal);
}

StatRetr::~StatRetr()
{
    cout << "Turquoise finished." << endl;
}
