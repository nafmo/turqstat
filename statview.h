// Copyright © 1998 Peter Karlsson
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
#include "statengine.h"

class StatView
{
public:
    bool CreateReport(StatEngine *engine, string filename,
                      unsigned maxnumber,
                      bool quoters, bool topwritten, bool topreceived,
                      bool topsubjects, bool topprograms,
                      bool weekstats, bool daystats, bool showversions,
                      bool showallnums);

protected:
    string Mangle(string, unsigned);
};

#endif
