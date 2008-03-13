// Copyright (c) 1999-2001 Peter Karlsson
// Copyright (c) 2007-2008 Stephen Hurd
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

#ifndef __SMBREAD_H
#define __SMBREAD_H

#include <config.h>

#include <smblib.h>

#include "datatypes.h"
#include "arearead.h"

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack(1)
#endif

class StatEngine;

/**
 * Class that reads SMB message bases.
 * This class reads message bases stored in the SMB format.
 */
class SMBRead : public AreaRead
{
public:
    /** Standard constructor.
     * Creates the SMB message base reader object.
     * @param path Directory and base name for message base.
     */
    SMBRead(const char *path);
    /** Standard destructor. */
    virtual ~SMBRead();

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
                          StatEngine &destination);

protected:
    /** Path to message base. */
    char        *areapath;

	smb_t		smb;
};

#if defined(__GNUC__) || defined(__EMX__)
# pragma pack()
#endif

#endif
