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
#include <String.h>
#include <unistd.h>
#include <stdlib.h>

#include "statengine.h"
#include "statview.h"
#include "arearead.h"
#include "squishread.h"

class StatRetr
{
public:
    StatRetr(char *areapath, char *outputfilepath, unsigned days);
    ~StatRetr();
};

int main(int argc, char *argv[])
{
    unsigned days = 0;

    // We don't want timezones here
    timezone = 0;
    daylight = 0;

    // Handle arguments
    int c;
    while (EOF != (c = getopt(argc, argv, "d:?")))
    {
        switch (c)
        {
            case 'd':
                days = strtoul(optarg, NULL, 10);
                break;

            case '?':
            default:
                cout << "Usage: " << argv[0]
                     << " [-d days] squisharea outputfile" << endl;
                return 1;
        }
    }

    StatRetr s(argv[optind], argv[optind + 1], days);

    return 0;
}

StatRetr::StatRetr(char *areapath, char *outputfilepath, unsigned days)
{
    cout << "Turqoise SuperStat 0.1 (c) Copyright 1998-1999 Peter Karlsson." << endl;

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
    AreaRead *area = new SquishRead(areapath);
    StatEngine engine;
    area->Transfer(from, engine);

    // Create output
    StatView view;
    view.CreateReport(&engine, outputfilepath, 15,
                      true, true, true, true, true, true, true);
}

StatRetr::~StatRetr(String areapath, String outputfilepath)
{
    cout << "Turqoise finished." << endl;
}
