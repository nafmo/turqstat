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
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "nntpread.h"
#include "statengine.h"
#include "output.h"

#define BUFSIZE 65536

NntpRead::NntpRead(const char *_server, const char *_newsgroup)
{
    server = strdup(_server);
    group = strdup(_newsgroup);
    sockfd = -1;
    articles = NULL;
}

NntpRead::~NntpRead()
{
    if (server) free(server);
    if (group) free(group);
    if (sockfd != -1)
    {
        SendCommand("QUIT\r\n");

        fclose(sock);
        shutdown(sockfd, 2);
        close(sockfd);
    }
    delete[] articles;
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
    int response = ConnectServer();
    if (-1 == response)
    {
        display->ErrorMessage(TDisplay::nntp_communication_problem);
        return false;
    }
    else if (response != 200 && response != 201)
    {
        display->ErrorMessage(TDisplay::nntp_unexpected_result, response);
        return false;
    }

    // Set reader mode (RFC 2980 extension, might not be understood)
    response = SendCommand("MODE READER\r\n");
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
    else if (411 == response)
    {
        // No such group
        display->ErrorMessage(TDisplay::cannot_open_file, group);
        return false;
    }
    else if (response != 211)
    {
        display->ErrorMessage(TDisplay::nntp_unexpected_result, response);
        return false;
    }

    unsigned int numarticles, first, last;
    if (sscanf(&buffer[4], "%u %u %u", &numarticles, &first, &last) != 3)
    {
        display->ErrorMessage(TDisplay::nntp_communication_problem);
        return false;
    }

    // Retrieve list of all available articles (RFC 2980 extension, might
    // not be understood)
    bool havearticlelist = false;
    response = SendCommand("LISTGROUP\r\n");
    if (-1 == response)
    {
        display->ErrorMessage(TDisplay::nntp_communication_problem);
        return false;
    }
    else if (211 == response)
    {
        // 211 List of article numbers follow.
        articles = new bool[last - first + 1];
        for (unsigned int i = first; i <= last; i ++)
        {
            articles[i - first] = false;
        }
        // Read list of articles terminated by period.
        // Here, we (ab)use the GetResponse method, since the format is
        // almost the same. When we find the final ".", we will get a
        // -1 reply.
        int article;
        havearticlelist = true;
        numarticles = 0;
        while (-1 != (article = GetResponse()))
        {
            if ((unsigned int) article >= first && (unsigned int) article <= last)
            {
                articles[article - first] = true;
                numarticles ++;
            }
        }
    }
    else if (response != 500 && response != 502)
    {
        // 500 Command not understood
        // 502 No permission
        display->ErrorMessage(TDisplay::nntp_unexpected_result, response);
        return false;
    }

    // Use XPAT to retrieve the Date header of all active messages,
    // and filter out those not within range (RFC 2980 extension, might
    // not be understood)
    snprintf(command, 512, "XPAT date %u-%u *\r\n", first, last);
    response = SendCommand(command);
    if (-1 == response)
    {
        display->ErrorMessage(TDisplay::nntp_communication_problem);
        return false;
    }
    else if (221 == response)
    {
        // 221 Header follows
        if (!havearticlelist)
        {
            // LISTGROUP failed, but XPAT succeeded; set up the articles
            // array to have true for each entry.
            havearticlelist = true;
            articles = new bool[last - first + 1];
            for (unsigned int i = first; i <= last; i ++)
            {
                articles[i - first] = true;
            }
        }

        // Headers are in <num> Header format, so (ab)use GetResponse()
        // to retrieve them. List of headers is terminated with a ".",
        // which gives -1.
        unsigned int article;
        while ((unsigned int) -1 != (article = (unsigned int) GetResponse()))
        {
            if (article >= first || article <= last &&
                articles[article - first])
            {
                // Article is within bounds, and is flagged as active
                char *p = strchr(buffer, ' ');
                time_t posted = rfcToTimeT(p + 1);
                if (posted && (posted < starttime || posted > endtime))
                {
                    // Outside wanted interval; flag article as inactive.
                    articles[article - first] = false;
                    numarticles --;
                }
            }
        }
    }

    display->SetMessagesTotal(numarticles);

    // Retrieve all articles
    char *msgbuf = NULL, *ctrlbuf = NULL, *p;
    time_t posttime;
    size_t bufused, buflen, length, thislength;
    int msgn = 0;
    char line[BUFSIZ];

    for (unsigned int article = first; article <= last; article ++)
    {
        // If we know which articles there are, check if this one exists.
        if (havearticlelist && !articles[article - first]) continue;

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
            // 412 No news group has been selected
            display->ErrorMessage(TDisplay::nntp_unexpected_result, response);
            return false;
        }
        else if (221 != response)
        {
            // This article cannot be retrieved; go to the next one instead
            continue;
        }

        // Read data until we get "." on an otherwise empty line.
        // Store in ctrlbuf (^A separated)
        posttime = time_t(0);
        p = ctrlbuf = new char[BUFSIZE];
        length = BUFSIZE;
        if (!ctrlbuf) goto out;

        while (NULL != (p = fgets(p, length, sock)))
        {
            // Leave loop if at end of header
            if ('.' == *p && ('\r' == *(p + 1) || '\n' == *(p + 1)))
            {
                // Terminate header buffer
                *(p - 1) = 0;
                break;
            }
//printf("| %s", p);

            // Insert a ^A marker before buffer start
            *(p - 1) = 1;

            thislength = strlen(p);
            if (thislength > 2)
            {
                if ('\r' ==p[thislength - 2] || '\n' == p[thislength - 2])
                {
                    thislength --;
                }
            }

            if (!posttime && thislength > 5 && 0 == fcompare(p, "DATE:", 5))
            {
                unsigned char *date_p = (unsigned char *) p + 5;
                while (isspace(*date_p)) date_p ++;
                posttime = rfcToTimeT(string((char *) date_p));
//printf(">> posttime=%d\n", (int) posttime);
            }

            // Point past newline marker (which will be overwritten)
            p += thislength;
            length -= thislength;
        }

        if (posttime < starttime || posttime > endtime)
        {
//printf(">> outside interval %d-%d\n", (int) starttime, (int) endtime);
            // Outside wanted interval; go to the next one
            goto out;
        }

        // Retrieve article body
        msgbuf = (char *) malloc(sizeof (char *) * BUFSIZE);
        if (!msgbuf) goto out;

        snprintf(command, 512, "BODY %u\r\n", article);
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
        else if (222 != response)
        {
            // This article cannot be retrieved; go to the next one instead
            continue;
        }

        // Read data until we get "." on an otherwise empty line.
        // Store in msgbuf (raw).
        bufused = 0;
        buflen = BUFSIZE;
        while (NULL != fgets(line, BUFSIZ, sock))
        {
            // Leave loop if at end of body
            if ('.' == line[0] && ('\r' == line[1] || '\n' == line[1]))
            {
                // Terminate body buffer
                msgbuf[bufused] = 0;
                break;
            }

            // Remove CR and LF terminators
//printf("| %s", line);
            thislength = strlen(line);
            while (thislength && ('\n' == line[thislength - 1] ||
                                  '\r' == line[thislength - 1]))
            {
                thislength --;
            }

            // Check that we have enough space, reallocate if necessary
            while (bufused + thislength + 2 > buflen)
            {
                buflen += BUFSIZE;
//printf(">> realloc (%d)\n", buflen);
                msgbuf = (char *) realloc(msgbuf, sizeof (char *) * buflen);
                if (!msgbuf)
                {
                    // Panic!
                    display->ErrorMessage(TDisplay::out_of_memory);
                    delete[] ctrlbuf;
                    return false;
                }
            }

            // Copy
            strncpy(msgbuf + bufused, line, thislength);
            bufused += thislength;
            msgbuf[bufused ++] = '\n';
            msgbuf[bufused] = 0;
assert(bufused < buflen);
//printf(">> bufused=%u buflen=%u\n", bufused, buflen);
        }
//printf("----------\n%s\n----------\n%s\n----------\n", ctrlbuf, msgbuf);

        // Add to statistics
        destination.AddData(string(""), string(""), string(""),
                            string(ctrlbuf), string(msgbuf),
                            posttime, posttime);

        // Clean up our mess
        free(msgbuf);
out:;
        delete[] ctrlbuf;

        display->UpdateProgress(++ msgn);
    }

    return true;
}

int NntpRead::ConnectServer()
{
    // Lookup address
    hostent *host;
    host = gethostbyname(server);
    if (NULL == host) return -1;

    // Open socket
    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
        return -1;

    // Connect socket
    union
    {
        sockaddr x;
        sockaddr_in in;
    } nntpaddr;
    nntpaddr.in.sin_port = htons(119);
    nntpaddr.in.sin_addr.s_addr = *(unsigned long *) host->h_addr_list[0];
    nntpaddr.in.sin_family = AF_INET;
    if (-1 == (connect(sockfd, &nntpaddr.x, sizeof nntpaddr))) return -1;

    // Connect socket to a FILE *
    sock = fdopen(sockfd, "r+");
    return GetResponse();
}

int NntpRead::SendCommand(const char *command)
{
    // Send command
//printf("> %s", command);
    fflush(sock);
    fputs(command, sock);
    fflush(sock);

    return GetResponse();
}

int NntpRead::GetResponse()
{
    // Retrieve response code
    char *p;
    fgets(buffer, sizeof buffer, sock);
//printf("< %s", buffer);
    long int rc = strtol(buffer, &p, 10);
    if (p == buffer) rc = -1;
//printf(": rc=%ld\n", rc);

    return rc;
}