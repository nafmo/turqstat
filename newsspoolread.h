// Copyright (c) 2000 Peter Karlsson
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

#ifndef __NEWSSPOOLREAD_H
#define __NEWSSPOOLREAD_H

#include <string>

#include "arearead.h"
#include "statengine.h"
#include "utility.h"

class NewsSpoolRead : public AreaRead
{
public:
    // Constructor and destructor
    NewsSpoolRead(const char *path);
    virtual ~NewsSpoolRead();

    // Transfer function
    virtual bool Transfer(time_t starttime, StatEngine &destination);

protected:
    char    *areapath;
};

#endif
