// Copyright (c) 1999-2001 Peter Karlsson
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

#ifndef __AREAREAD_H
#define __AREAREAD_H

class StatEngine;

/**
 * Abstract class that reads message areas.
 * This is the superclass for the classes that implement reading of message
 * area types.
 */
class AreaRead
{
public:
    /**
     * Transfer function. 
     * This function transfers all messages in the message bases, received
     * after the specified starting date, to the specified statistics engine.
     *
     * @param starttime   Date to start retrieve statistics from.
     * @param endtime     Date to stop retrieve statistics at.
     * @param destination Engine object to transfer data to.
     * @return True if the message base was read correctly (even if no
     *         messages fits the condition).
     */
    virtual bool Transfer(time_t starttime, time_t endtime,
                          StatEngine &destination) = 0;

    /*
     * Virtual destructor. Overridden by subclasses which need destruction.
     */
    virtual ~AreaRead() {};
};

#endif
