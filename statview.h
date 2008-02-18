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

#ifndef __STATVIEW_H
#define __STATVIEW_H

#include <string>

#if !defined(HAVE_IMPLICIT_NAMESPACE)
using namespace std;
#endif

class StatEngine;
class Template;

/**
 * The class that saves the data. This is the class that saves the data
 * retrieved with the StatEngine to a text file, containing the toplists
 * that are specified as wanted.
 */
class StatView
{
public:
    /** Standard constructor. */
    StatView();

    // Format selectors
    /** Tell view which output template to use. */
    void SetTemplate(const Template *output_template) { m_template = output_template; };

    // Toggles for toplist selection
    /** Tell view whether or not to enable the quoters toplist. */
    inline void EnableQuoters(bool yes)     { quoters = yes;      };

    /** Tell view whether or not to enable the writers toplist. */
    inline void EnableTopWritten(bool yes)  { topwritten = yes;   };

    /** Tell view whether or not to enable the original content toplist. */
    inline void EnableTopOriginal(bool yes) { toporiginal = yes;  };

    /** Tell view whether or not to enable the Fidonet networks toplist. */
    inline void EnableTopNets(bool yes)     { topnets = yes;      };

    /** Tell view whether or not to enable the receivers toplist. */
    inline void EnableTopReceived(bool yes) { topreceived = yes;  };

    /** Tell view whether or not to enable the subjects toplist. */
    inline void EnableTopSubjects(bool yes) { topsubjects = yes;  };

    /** Tell view whether or not to enable the programs toplist. */
    inline void EnableTopPrograms(bool yes) { topprograms = yes;  };

    /** Tell view whether or not to enable the Internet topdomain toplist. */
    inline void EnableTopDomains(bool yes)  { topdomains = yes;   };

    /** Tell view whether or not to enable the weekday statistics. */
    inline void EnableWeekStats(bool yes)   { weekstats = yes;    };

    /** Tell view whether or not to enable the hour statistics. */
    inline void EnableDayStats(bool yes)    { daystats = yes;     };

    // Output format
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    /** Tell view whether dates should be written in locale format. */
    inline void UseLocale(bool yes)         { uselocale = yes;    };
#endif

    // Toplist flags
    /** Tell view whether to print details about versions of programs. */
    inline void ShowVersions(bool yes)      { showversions = yes; };

    /** Tell view whether to show all numbers in the toplist. */
    inline void ShowAllNums(bool yes)       { showallnums = yes;  };

    /** Tell view the maximum number of entries to display in toplist. */
    inline void SetMaxEntries(unsigned max) { maxnumber = max;    };

    /** Tell view which character set to use for output. */
    void SetCharset(const char *in_charset) { charset = in_charset; };

    // This does the actual work
    /**
     * Create a report. This method is the one that does the actual work,
     * it retrieves the information stored in the engine and saves it to the
     * named file.
     * @param engine   Pointer to engine containing data.
     * @param filename Name of file to create.
     * @return True if the file was created successfully.
     */
    bool CreateReport(StatEngine *engine, string filename);

protected:
    bool        quoters, topwritten, toporiginal, topnets, topreceived,
                topsubjects, topprograms, topdomains, weekstats, daystats;

    bool        showversions, showallnums;

#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    bool        uselocale;
#endif

    unsigned    maxnumber;

    string      charset;
	string		m_dayname[7];

	bool		m_html;
	
    const Template *m_template;
};

#endif
