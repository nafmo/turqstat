// Copyright (c) 2001 Peter Karlsson
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

#ifndef __NNTPREAD_H
#define __NNTPREAD_H

#include <stdio.h>
#include <config.h>
#if defined(HAVE_WINSOCK_H)
# include <winsock.h>
#else
# define SOCKET int
#endif

#include "arearead.h"

class StatEngine;

/**
 * Class that reads Usenet news via the NNTP protocol.
 * This class reads message bases stored on a NNTP server over the network.
 */
class NntpRead : public AreaRead
{
public:
    /** Standard constructor.
     * Creates the Usenet news NNTP reader object.
     * @param server Name of server hosting the news groups.
     * @param newsgroup Name of newsgroup to retrieve data from.
     */
    NntpRead(const char *server, const char *newsgroup);

    /** Standard destructor. */
    virtual ~NntpRead();

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
    /**
     * Connect to the NNTP server.
     * @return Whether the operation succeeded. Returns the numeric response
     *         code given, or fails with -1 if the sevrer could not be reached
     *         or it did not allow the connection.
     */
    int ConnectServer();

    /**
     * Send command over NNTP connection.
     * @param command Command to send, must be CRLF terminated.
     * @return Numeric response code returned, the entire line is stored in
     *         the line buffer. Returns -1 if communiction errors occur.
     */
    int SendCommand(const char *command);

    /**
     * Get NNTP response code.
     * @return Numeric response code return, the entire line is stored in
     *         the line buffer. Returns -1 if communication errors occur.
     */
    int GetResponse();

    /** Name of server. */
    char    *server;
    /** Name of newsgroup. */
    char    *group;
    /** Communications socket. */
    SOCKET  sockfd;
    FILE    *sock;
    /** Line buffer for NNTP response code. */
    char    buffer[512];
    /** Active articles in group. */
    bool    *articles;
};

#endif
