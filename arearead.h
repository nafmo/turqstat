// Copyright (c) 1999 Peter Karlsson
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

#ifndef __AREAREAD_H
#define __AREAREAD_H

#ifdef __EMX__
# include <Strng.h>
#else
# include <String.h>
#endif

#include "statengine.h"

class AreaRead
{
public:
    // Transfer function
    virtual bool Transfer(time_t starttime, StatEngine &destination) = 0;
};

#endif
