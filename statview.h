// Copyright © 1998-2000 Peter Karlsson
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

#ifndef __STATVIEW_H
#define __STATVIEW_H

#include <string>

class StatEngine;

class StatView
{
public:
    StatView();

    // Toggles for toplist selection
    inline void EnableQuoters(bool yes)     { quoters = yes;      };
    inline void EnableTopWritten(bool yes)  { topwritten = yes;   };
    inline void EnableTopOriginal(bool yes) { toporiginal = yes;  };
    inline void EnableTopNets(bool yes)     { topnets = yes;      };
    inline void EnableTopReceived(bool yes) { topreceived = yes;  };
    inline void EnableTopSubjects(bool yes) { topsubjects = yes;  };
    inline void EnableTopPrograms(bool yes) { topprograms = yes;  };
    inline void EnableTopDomains(bool yes)  { topdomains = yes;   };
    inline void EnableWeekStats(bool yes)   { weekstats = yes;    };
    inline void EnableDayStats(bool yes)    { daystats = yes;     };

    // Output format
#ifdef HAVE_LOCALE_H
    inline void UseLocale(bool yes)         { uselocale = yes;    };
#endif

    // Toplist flags
    inline void ShowVersions(bool yes)      { showversions = yes; };
    inline void ShowAllNums(bool yes)       { showallnums = yes;  };

    inline void SetMaxEntries(unsigned max) { maxnumber = max;    };

    // This does the actual work
    bool CreateReport(StatEngine *engine, string filename);

protected:
    bool        quoters, topwritten, toporiginal, topnets, topreceived,
                topsubjects, topprograms, topdomains, weekstats, daystats;

    bool        showversions, showallnums;

#ifdef HAVE_LOCALE_H
    bool        uselocale;
#endif

    unsigned    maxnumber;
};

#endif
