// Turquoise SuperStat
//
// A statistic collection program for Fidonet and Usenet systems
// Version 2.0
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

#include <config.h>
#include <iostream.h>
#include <string>
#ifdef HAS_GETOPT_IN_UNISTD
# include <unistd.h>
#elif defined(HAS_GETOPT_IN_GETOPT)
# include <getopt.h>
#endif
#include <stdlib.h>
#include <time.h>
#ifdef HAVE_LOCALE_H
# include <locale.h>
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
#include "mytime.h"
#include "output.h"

class StatRetr
{
public:
    enum basetype_e { squish, sdm, opus, fdapx, jam, mypoint, tanstaafl,
                      newsspool };

    StatRetr(StatEngine &, StatView &,
             char **areapath, int numpaths, char *outputfilepath,
             char *basepath, basetype_e basetype,
             unsigned days);
    ~StatRetr();
};

void helpscreen(void);

int main(int argc, char *argv[])
{
    unsigned days = 0;
    unsigned maxnum = 15;
    char *basepath = NULL;
    StatRetr::basetype_e basetype = StatRetr::squish;
    bool quoters = true, topwritten = true, topreceived = true,
         topsubjects = true, topprograms = true, weekstats = true,
         daystats = true, versions = true, allnums = false,
         toporiginal = true, topnets = true, topdomains = true;
#ifdef HAVE_LOCALE_H
    bool uselocale = false;
#endif

    // Setup locale
#ifdef HAVE_LOCALE_H
    setlocale(LC_TIME, "");
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
#ifdef HAVE_LOCALE_H
    while (EOF != (c = getopt(argc, argv, "d:n:a:smofjptuQWRSPOHDVNTAL?")))
#else
    while (EOF != (c = getopt(argc, argv, "d:n:a:smofjptuQWRSPOHDVNTA?")))
#endif
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
            case 'N':   topnets = false;                            break;
            case 'T':   topdomains = false;                         break;
            case 'H':   daystats = false;                           break;
            case 'D':   weekstats = false;                          break;

            case 'V':   versions = false;                           break;
            case 'A':   allnums = true;                             break;

#ifdef HAVE_LOCALE_H
            case 'L':   uselocale = true;                           break;
#endif

            case '?':
            default:
                helpscreen();
                return 1;
        }
    }

    if (argc - optind < 2)
    {
        cerr << "Illegal number of arguments (-? for help)." << endl;
        return 1;
    }

    // Check parameters for integrity
    if ((StatRetr::tanstaafl == basetype ||
         StatRetr::fdapx == basetype ||
         StatRetr::mypoint == basetype) &&
        !basepath)
    {
        cerr << "Base message path missing (-? for help)." << endl;
        return 1;
    }

    // Create engine object
    StatEngine engine;

    // Create view object
    StatView view;

    // Enable toplists we want
    view.EnableQuoters(quoters);
    view.EnableTopWritten(topwritten);
    view.EnableTopOriginal(toporiginal);
    view.EnableTopNets(topnets);
    if (StatRetr::newsspool == basetype)
        view.EnableTopDomains(topdomains);
    else
        view.EnableTopDomains(!topdomains);
    view.EnableTopReceived(topreceived);
    view.EnableTopSubjects(topsubjects);
    view.EnableTopPrograms(topprograms);
    view.EnableWeekStats(weekstats);
    view.EnableDayStats(daystats);
#ifdef HAVE_LOCALE_H
    view.UseLocale(uselocale);
#endif

    // Set toplist parameters
    view.ShowVersions(versions);
    view.ShowAllNums(allnums);
    view.SetMaxEntries(maxnum);

    // Create retrieval object, and do the work
    StatRetr s(engine, view,
               &argv[optind + 1], argc - optind - 1, argv[optind],
               basepath, basetype, days);

    return 0;
}

StatRetr::StatRetr(StatEngine &engine, StatView &view,
                   char **areapath, int numpaths, char *outputfilepath,
                   char *basepath, basetype_e basetype,
                   unsigned days)
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
    from = my_mktime(fromtm);

    // Transfer from area(s) to engine
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
                TDisplay::GetOutputObject()->
                    InternalErrorQuit(TDisplay::message_base_mismatch, 1);
                exit(1);
        }

        if (!area)
        {
            TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory_area,
                                                   1);
        }
        if (!(area->Transfer(from, engine))) return;

        engine.AreaDone();
        cout << "Finished reading " << areapath[counter] << endl;

        delete area;
        counter ++;
        numpaths --;
    }

    // Write output
    view.CreateReport(&engine, outputfilepath);
    cout << "Wrote " << outputfilepath << endl;
}

StatRetr::~StatRetr()
{
    cout << "Turquoise finished." << endl;
}

void helpscreen(void)
{
    cout << "Usage: turqstat [options] outputfile areadef(s)" << endl;
    cout << endl;
    cout << "Data selection options:" << endl;
    cout << "  -d days  Days back to count   -a path  Base path (see below)"
         << endl;
    cout << "  -n num   Maximum entries" << endl;
    cout << endl;
    cout << "Input selection options:" << endl;
    cout << "  -s  Squish       -j  JAM          -p  MyPoint*" << endl;
    cout << "  -m  FTSC *.MSG   -f  FDAPX/w*" << endl;
    cout << "  -o  Opus *.MSG   -t  Tanstaafl*   -u  Usenet news spool" << endl;
    cout << "    * = requires -a parameter" << endl;
    cout << endl;
    cout << "Toplist selection options (turns off respective list):" << endl;
    cout << "  -Q  Quoters     -S  Subjects   -N  Fidonet nets   -H Hour stats"
         << endl;
    cout << "  -W  Writers     -P  Programs   -T  Topdomains*  "
            "  -D Weekday stats" << endl;
    cout << "  -R  Receivers   -O  Original" << endl;
    cout << "  -V  No program version info   -A  Show all numbers in toplists"
         << endl;
#ifdef HAVE_LOCALE_H
    cout << "  -L  Use locale's date format" << endl;
#endif
    cout << "    * = turn off for Usenet, turn on for Fidonet" << endl;
}
