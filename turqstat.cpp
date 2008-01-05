// Turquoise SuperStat
//
// A statistic collection program for Fidonet and Usenet systems
// Version 3.0
//
// Copyright (c) 1998-2008 Peter Karlsson
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
#ifdef HAVE_IOSTREAM
# include <iostream>
#else
# include <iostream.h>
#endif
#include <string>
#ifdef HAVE_GETOPT_IN_UNISTD
# include <unistd.h>
#elif defined(HAVE_GETOPT_IN_STDIO)
# include <stdio.h>
#elif defined(HAVE_GETOPT_IN_GETOPT)
# include <getopt.h>
#endif
#include <stdlib.h>
#include <time.h>
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_WIN32_GETMODULEFILENAME
# include <windows.h>
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
#if defined(HAVE_NNTP)
# include "nntpread.h"
#endif
#include "version.h"
#include "utility.h"
#include "mytime.h"
#include "output.h"
#include "template.h"

/**
 * Wrapper class to retrieve data. This class is called from the main
 * procedure to retrieve data from the specified message bases into the
 * engine, and to connect that engine to the view to create a report file.
 */
class StatRetr
{
public:
    /**
     * Numeric constants describing the message base types that are
     * supported by Turquoise SuperStat.
     */
    enum basetype_e
    {
        squish,     ///< SquishMail message base format.
        sdm,        ///< Standard FTSC message base format.
        opus,       ///< Opus SDM message base format.
        fdapx,      ///< FDAPX/w message base format.
        jam,        ///< JAM message base format.
        mypoint,    ///< MyPoint message base format.
        tanstaafl,  ///< Tanstaafl message base format.
#if defined(HAVE_NNTP)
        nntp,       ///< Usenet news server (nntp).
#endif
        newsspool   ///< Usenet news spool.
    };

    /**
     * Constructor that does the work.
     * @param engine         Reference to engine that stores data.
     * @param view           Reference to view that creates report.
     * @param areapath       Paths to areas to be examined.
     * @param numpaths       Number of areas in array.
     * @param outputfilepath Path to output file to create.
     * @param basepath       Base path of message base (if required).
     * @param basetype       Message base type for input.
     * @param rangestart     First date to save data for.
     * @param rangeend       Last date to save data for.
     */
    StatRetr(StatEngine &, StatView &,
             char **areapath, int numpaths, char *outputfilepath,
             char *basepath, basetype_e basetype,
             time_t rangestart, time_t rangeend);

    /** Standard destructor. */
    ~StatRetr();
};

/** Display the program's help screen. */
void helpscreen(void);

/** Evaluate a date-time range as given on the command line.
 * @param rangespec      Range specification string.
 * @param range_start_pp Where to create the range start time.
 * @param range_end_pp   Where to create the range end time.
 */
void evaluaterange(const char *rangespec,
                   time_t **range_start_pp, time_t **range_end_pp);

int main(int argc, char *argv[])
{
    unsigned days = 0;
    unsigned maxnum = 15;
    char *basepath = NULL;
	const char *templatefile = NULL;
    StatRetr::basetype_e basetype = StatRetr::squish;
    bool quoters = true, topwritten = true, topreceived = true,
         topsubjects = true, topprograms = true, weekstats = true,
         daystats = true, versions = true, allnums = false,
         toporiginal = true, topnets = true, topdomains = true;
    time_t *range_start_p = NULL, *range_end_p = NULL;
#if defined(__EMX__) || defined(__CYGWIN__) || defined (__MINGW32__)
    const char *charset = "IBMPC";
#else
    const char *charset = "LATIN-1";
#endif
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    bool uselocale = false;
#endif
#if defined(HAVE_WIN32_GETMODULEFILENAME)
	char defaulttemplate[_MAX_PATH];
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
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    while (EOF != (c = getopt(argc, argv, "d:n:a:r:C:F:smofjptuQWRSPOHDVNTALU?")))
#else
    while (EOF != (c = getopt(argc, argv, "d:n:a:r:C:F:smofjptuQWRSPOHDVNTAU?")))
#endif
    {
        switch (c)
        {
            case 'd':   days = strtoul(optarg, NULL, 10);           break;
            case 'n':   maxnum = strtoul(optarg, NULL, 10);         break;
            case 'a':   basepath = optarg;                          break;
            case 'r':   evaluaterange(optarg, &range_start_p, &range_end_p);
                                                                    break;
            case 'F':   templatefile = optarg;                      break;

            case 's':   basetype = StatRetr::squish;                break;
            case 'm':   basetype = StatRetr::sdm;                   break;
            case 'o':   basetype = StatRetr::opus;                  break;
            case 'f':   basetype = StatRetr::fdapx;                 break;
            case 'j':   basetype = StatRetr::jam;                   break;
            case 'p':   basetype = StatRetr::mypoint;               break;
            case 't':   basetype = StatRetr::tanstaafl;             break;
#if defined(HAVE_NNTP)
            case 'U':   basetype = StatRetr::nntp;                  break;
#endif
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

#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
            case 'L':   uselocale = true;                           break;
#endif

            case 'C':   charset = optarg;                           break;

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

    // Load template
    if (!templatefile)
    {
        // Use default template if none was supplied.
#if defined(HAVE_WIN32_GETMODULEFILENAME)
		// Windows: Template is in the same directory as the EXE file.
		GetModuleFileNameA(NULL, defaulttemplate, _MAX_PATH);
		char *p = strrchr(defaulttemplate, '\\');
		if (p && p < defaulttemplate + _MAX_PATH - 13)
		{
			strcpy(p + 1, "default.tpl");
			templatefile = defaulttemplate;
		}
#else
		// Unix-like environment: Template is in a pre-defined installation
		// directory.
        templatefile = DATA
#ifdef BACKSLASH_PATHS
            "\\"
#else
            "/"
#endif
            "default.tpl";
#endif
    }

    bool is_error = false;
    Template *output_template_p = Template::Parse(templatefile, is_error);
    if (!output_template_p || is_error)
    {
		TDisplay *display = TDisplay::GetOutputObject();
		display->ErrorQuit(TDisplay::template_parse_error, 2);
    }

    // Create engine object
    StatEngine engine;

    // Create view object
    StatView view;

    // Select output format
    view.SetTemplate(output_template_p);

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
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    view.UseLocale(uselocale);
#endif

    // Set toplist parameters
    view.ShowVersions(versions);
    view.ShowAllNums(allnums);
    view.SetMaxEntries(maxnum);
    view.SetCharset(charset);

    // Compute starting time if days parameter is given
    if (days != 0 && !range_start_p)
    {
        range_start_p = new time_t(time(NULL) - days * 86400L);
        struct tm *fromtm = localtime(range_start_p);
        fromtm->tm_hour = 0;
        fromtm->tm_min  = 0;
        fromtm->tm_sec  = 0;
        *range_start_p = my_mktime(fromtm);
    }

    // Create retrieval object, and do the work
    StatRetr s(engine, view,
               &argv[optind + 1], argc - optind - 1, argv[optind],
               basepath, basetype, range_start_p ? *range_start_p : time_t(0),
               range_end_p ? *range_end_p : time_t(DISTANT_FUTURE));

    delete range_start_p;
    delete range_end_p;
    delete output_template_p;

    return 0;
}

StatRetr::StatRetr(StatEngine &engine, StatView &view,
                   char **areapath, int numpaths, char *outputfilepath,
                   char *basepath, basetype_e basetype,
                   time_t rangestart, time_t rangeend)
{
    unsigned areanum;

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

#if defined(HAVE_NNTP)
            case nntp:
                area = new NntpRead(basepath, areapath[counter]);
                break;
#endif

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
        if (!(area->Transfer(rangestart, rangeend, engine))) return;

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
    cout << "Data selection and display options:" << endl;
    cout << "  -a path  Base path (see below)    -d days   Days back to count"
         << endl;
    cout << "  -n num   Maximum entries          -r range  Date range"
         << endl;
    cout << "  -F file  Use template file (default: default.tpl)" << endl;
    cout << endl;
    cout << "Input selection options:" << endl;
    cout << "  -s  Squish       -j  JAM          -p  MyPoint*" << endl;
    cout << "  -m  FTSC *.MSG   -f  FDAPX/w*"
#if defined(HAVE_NNTP)
                                            "     -U  Usenet nntp server**"
#endif
         << endl;
    cout << "  -o  Opus *.MSG   -t  Tanstaafl*   -u  Usenet news spool" << endl;
    cout << "    * = requires -a parameter         ** = use -a for server" << endl;
    cout << endl;
    cout << "Output selection options (turns off respective list):" << endl;
    cout << "  -Q  Quoters     -S  Subjects   -N  Fidonet nets   -H Hour stats"
         << endl;
    cout << "  -W  Writers     -P  Programs   -T  Topdomains*  "
            "  -D Weekday stats" << endl;
    cout << "  -R  Receivers   -O  Original       * = off for Usenet, on for "
            "Fidonet" << endl;
    cout << "  -V          No version info    -A  Show all numbers in toplists"
         << endl;
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    cout << "  -C charset  Output charset     -L  Use locale's date "
            "format" << endl;
#else
    cout << "  -C charset  Output charset" << endl;
#endif
}

void evaluaterange(const char *rangespec,
                   time_t **range_start_pp, time_t **range_end_pp)
{
    // Allocate times if not already
    if (!*range_start_pp) *range_start_pp = new time_t;
    if (!*range_end_pp)   *range_end_pp = new time_t;

    // Evaluate range as given. It is on the form
    // "[yyyymmdd[Thhmmss]]-[yyyymmdd[Thhmmss]]"
    bool valid = false;
    const char *separator = strchr(rangespec, '-');
    if (separator)
    {
        if (0 == separator - rangespec)
        {
            **range_start_pp = time_t(0);
        }
        else
        {
            **range_start_pp =
                timespecToTimeT(string(rangespec, separator - rangespec));
        }

        if (time_t(-1) != **range_start_pp)
        {
            const char *rangespec2 = separator + 1;
            size_t charsleft = strlen(rangespec2);
            if (0 == charsleft)
            {
                **range_end_pp = time_t(DISTANT_FUTURE);
            }
            else
            {
                **range_end_pp =
                    timespecToTimeT(string(rangespec2, charsleft));
            }

            if (time_t(-1) != **range_end_pp)
            {
                valid = true;
            }
        }
    }

    if (!valid)
    {
        cerr << "Invalid range specification: " << rangespec << endl;
        exit(1);
    }
}
