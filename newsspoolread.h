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

#include "arearead.h"

class StatEngine;

/**
 * Class that reads Usenet news spools.
 * This class reads message bases stored in the standard text-based Usenet
 * news spool format.
 */
class NewsSpoolRead : public AreaRead
{
public:
    /** Standard constructor.
     * Creates the Usenet spool reader object.
     * @param path Directory for the news spool.
     */
    NewsSpoolRead(const char *path);
    /** Standard destructor. */
    virtual ~NewsSpoolRead();

    /**
     * Transfer function. 
     * This function transfers all messages in the message bases, received
     * after the specified starting date, to the specified statistics engine.
     *
     * @param starttime   Date to start retrieve statistcs from.
     * @param destination Engine object to transfer data to.
     * @return True if the message base was read correctly (even if no
     *         messages fits the condition.
     */
    virtual bool Transfer(time_t starttime, StatEngine &destination);

protected:
    /** Path to spool directory. */
    char    *areapath;
};

#endif
