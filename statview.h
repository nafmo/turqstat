// Copyright © 1998 Peter Karlsson
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

#ifndef __STATVIEW_H
#define __STATVIEW_H

#ifdef __EMX__
# include <Strng.h>
#else
# include <String.h>
#endif
#include "statengine.h"

class StatView
{
public:
    bool CreateReport(StatEngine *engine, String filename,
                      unsigned maxnumber,
                      bool quoters, bool topwritten, bool topreceived,
                      bool topsubjects, bool topprograms,
                      bool weekstats, bool daystats);

protected:
    String Mangle(String, unsigned);
};

#endif
