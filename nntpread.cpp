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

#include <config.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "nntpread.h"
#include "statengine.h"
#include "output.h"

NntpRead::NntpRead(const char *_server, const char *_newsgroup)
{
    server = strdup(_server);
    group = strdup(_newsgroup);
    sockfd = -1;
}

NntpRead::~NntpRead()
{
    if (server) free(server);
    if (group) free(group);
    if (sockfd != -1)
    {
        SendCommand("QUIT\r\n");
        close socket;
    }
}

bool NntpRead::Transfer(time_t starttime, time_t endtime,
                             StatEngine &destination)
{
    // Get the output object
    TDisplay *display = TDisplay::GetOutputObject();

    // Check that we got the path correctly in initialization
    if (!server || !group)
    {
        display->InternalErrorQuit(TDisplay::area_not_allocated, 1);
    }

    // This is a news spool
    destination.NewsArea();

    // We are unable to retrieve information on arrival times
    destination.NoArrivalTime();

    // Connect to the news server
    if (!ConnectServer()) return false;

    // Set reader mode (RFC 2980 extension, might not be understood)
    int response = SendCommand("MODE READER\r\n");
    if (-1 == response)
    {
        display->ErrorMessage(TDisplay::nntp_communication_problem);
        return false;
    }
    else if (response != 200 && response != 201 && response != 500)
    {
        display->ErrorMessage(TDisplay::nntp_unexpected_result, response);
        return false;
    }

    // Open group
    char command[512];
    snprintf(command, 512, "GROUP %s\r\n", group);
    response = SendCommand(command);
    if (-1 == response)
    {
        display->ErrorMessage(TDisplay::nntp_communication_problem);
        return false;
    }
    else if (response != 211)
    {
        display->ErrorMessage(TDisplay::nntp_unexpected_result, response);
        return false;
    }

    unsigned int numarticles, first, last;
    if (sscanf(buffer, "%u %u %u", &numarticles, &first, &last) != 3)
    {
        display->ErrorMessage(TDisplay::nntp_communication_problem);
        return false;
    }

    display->SetMessagesTotal(numarticles);

    // Retrieve all articles
    char *msgbuf = NULL, *ctrlbuf = NULL, *p;
    time_t posttime;
    int msgn = 0;

    for (unsigned int article = first; article <= last; article ++)
    {
        // Retrieve article head
        snprintf(command, 512, "HEAD %u\r\n", article);
        response = SendCommand(command);
        if (-1 == response)
        {
            display->ErrorMessage(TDisplay::nntp_communication_problem);
            return false;
        }
        else if (412 == response)
        {
            // "No news group has been selected"
            display->ErrorMessage(TDisplay::nntp_unexpected_result, response);
            return false;
        }
        else if (221 != response)
        {
            // This article cannot be retrieved; go to the next one instead
            continue;
        }

        // ### Read data until we get "." on an otherwise empty line.
        // Store in ctrlbuf (^A separated)
/*
        while (NULL != (p = fgets(p, length, msg)))
        {
            // Insert a ^A marker before buffer start
            *(p - 1) = 1;

            // Leave loop if we found an empty line (=message body starts)
            thislength = strlen(p);
if (strcmp(p, "Date

            if (1 == thislength)
                break;

            // Point past newline marker (which will be overwritten)
            p += thislength;
            length -= thislength;
        }
*/

        // ### Check age
        if (posttime < starttime || posttime > endtime)
        {
            // Outside wanted interval; go to the next one
            continue;
        }

        // Retrieve article body
        snprintf(command, 512, "BODY %u\r\n", article);
        respone = SendCommand(command);
        if (-1 == response)
        {
            display->ErrorMessage(TDisplay::nntp_communication_problem);
            return false;
        }
        else if (412 == response)
        {
            // "No news group has been selected"
            display->ErrorMessage(TDisplay::nntp_unexpected_result, response);
            return false;
        }
        else if (221 != response)
        {
            // This article cannot be retrieved; go to the next one instead
            continue;
        }

        // ### Read data until we get "." on an otherwise empty line.
        // Store in msgbuf (raw).

        // Add to statistics
        destination.AddData(string(""), string(""), string(""),
                            string(ctrlbuf), string(msgbuf),
                            0, arrived);

        // Clean up our mess
        delete[] msgbuf;
out:;
        delete[] ctrlbuf;
out2:;

        display->UpdateProgress(++ msgn);
        if (msg) fclose(msg);
    }

    return true;
}

/**
 * Connect to the NNTP server.
 * @return Whether the operation succeeded. It succeeds if the sevrer
 *         could be reached AND it allowed the connection.
 */
bool NntpRead::ConnectServer()
{
    // ### lookup host
    // ### sockfd = open socket blocking

    return true;
}

/**
 * Send command over NNTP connection.
 * @param command Command to send, must be CRLF terminated.
 * @return Numeric response code returned, rest of line is stored in
 *         the line buffer.
 */
int NntpRead::SendCommand(const char *command)
{
    // Send command
    int cmdlen = strlen(command);
    int bytes = write(sockfd, command, cmdlen);
    if (-1 == bytes || bytes != cmdlen) return -1;

    // Retrieve result

    // ###

    return strtol(xxx);
}
