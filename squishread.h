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

#ifndef __SQUISHREAD_H
#define __SQUISHREAD_H

#ifdef __EMX__
# include <Strng.h>
#else
# include <String.h>
#endif

#include "arearead.h"
#include "statengine.h"
#include "msgapi.h"

class SquishRead : public AreaRead
{
public:
    // Constructor and destructor
    SquishRead(String areapath);
    ~SquishRead();

    // Transfer function
    virtual bool Transfer(StatEngine &destination);

protected:
    char    *areapath;
};

#endif
