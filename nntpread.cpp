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
#include <sys/types.h>
#if defined(HAVE_NETDB_H)
# include <netdb.h>
#endif
#if defined(HAVE_SYS_SOCKET_H)
# include <sys/socket.h>
#elif defined(HAVE_WINSOCK_H)
# include <winsock.h>
#endif
#if defined(HAVE_NETINET_IN_H)
# include <netinet/in.h>
#endif
#if !defined(HAVE_WORKING_SOCKET_FDOPEN)
# if defined(HAVE_ERRNO_H)
#  include <errno.h>
# elif defined(HAVE_SYS_ERRNO_H)
#  include <sys/errno.h>
# else
#  error "Missing <errno.h>"
# endif
#endif

#include "nntpread.h"
#include "statengine.h"
#include "output.h"

#if !defined(INVALID_SOCKET)
// This is used for winsock compatibility
# define INVALID_SOCKET -1
#endif

#if !defined(HAVE_SNPRINTF)
# define snprintf4(a,b,c,d)   sprintf(a,c,d)
# define snprintf5(a,b,c,d,e) sprintf(a,c,d,e)
#else
# define snprintf4 snprintf
# define snprintf5 snprintf
#endif

NntpRead::NntpRead(const char *_server, const char *_newsgroup)
{
    server = strdup(_server);
    group = strdup(_newsgroup);
    sockfd = INVALID_SOCKET;
    articles = NULL;
#if !defined(HAVE_WORKING_SOCKET_FDOPEN)
    datainbuffer = 0;
#endif
}

NntpRead::~NntpRead()
{
    if (server) free(server);
    if (group) free(group);
    if (sockfd != INVALID_SOCKET)
    {
        SendCommand("QUIT\r\n");

#if defined(HAVE_WORKING_SOCKET_FDOPEN)
        fclose(sock);
#endif
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

#if defined(HAVE_WINSOCK_H)
    WORD versionrequest = MAKEWORD(1, 1);
    WSADATA wsadata;
    int err = WSAStartup(versionrequest, &wsadata);
    if (err != 0)
    {
        display->ErrorMessage(TDisplay::nntp_communication_problem);
        return false;
    }
#endif

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
    snprintf4(command, 512, "GROUP %s\r\n", group);
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
    snprintf5(command, 512, "XPAT date %u-%u *\r\n", first, last);
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
    time_t posttime;
    size_t thislength;
    int msgn = 0;
    char line[BUFSIZ];

    for (unsigned int article = first; article <= last; article ++)
    {
        // If we know which articles there are, check if this one exists.
        if (havearticlelist && !articles[article - first]) continue;

        // Retrieve article head
        snprintf4(command, 512, "HEAD %u\r\n", article);
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
        string ctrlbuf, msgbuf;

        while (GetLine(line, BUFSIZ))
        {
            // Leave loop if at end of header
            if ('.' == line[0] && ('\r' == line[1] || '\n' == line[1]))
            {
                break;
            }

            // Insert a ^A marker before buffer start
            ctrlbuf += '\1';

            thislength = strlen(line);
            if (thislength > 2)
            {
                if ('\r' == line[thislength - 2] ||
                    '\n' == line[thislength - 2])
                {
                    thislength -= 2;
                }
            }

            if (!posttime && thislength > 5 &&
                0 == fcompare(line, "DATE:", 5))
            {
                unsigned char *date_p = (unsigned char *) &line[5];
                while (isspace(*date_p)) date_p ++;
                posttime = rfcToTimeT(string((char *) date_p));
            }

            // Copy string to ctrl buffer
            ctrlbuf.append(line, thislength);
        }

        if (posttime < starttime || posttime > endtime)
        {
            // Outside wanted interval; go to the next one
            goto out;
        }

        // Retrieve article body
        snprintf4(command, 512, "BODY %u\r\n", article);
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
        while (GetLine(line, BUFSIZ))
        {
            // Leave loop if at end of body
            if ('.' == line[0] && ('\r' == line[1] || '\n' == line[1]))
            {
                break;
            }

            // Remove CR and LF terminators
            thislength = strlen(line);
            while (thislength && ('\n' == line[thislength - 1] ||
                                  '\r' == line[thislength - 1]))
            {
                thislength --;
            }

            // Copy
            msgbuf.append(line, thislength);
            msgbuf += '\n';
        }

        // Add to statistics
        destination.AddData(string(""), string(""), string(""),
                            ctrlbuf, msgbuf,
                            posttime, posttime);

        // Clean up our mess
out:;

        display->UpdateProgress(++ msgn);
    }

#if defined(HAVE_WINSOCK_H)
    WSACleanup();
#endif

    return true;
}

int NntpRead::ConnectServer()
{
    // Lookup address
    hostent *host;
    host = gethostbyname(server);
    if (NULL == host) return -1;

    // Open socket
    if (INVALID_SOCKET == (sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
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

#if defined(HAVE_WORKING_SOCKET_FDOPEN)
    // Connect socket to a FILE *
    sock = fdopen(sockfd, "r+");
#endif
    return GetResponse();
}

int NntpRead::SendCommand(const char *command)
{
    // Send command
    if (!SendLine(command))
    {
        return -1;
    }
    return GetResponse();
}

int NntpRead::GetResponse()
{
    // Retrieve response code
    char *p;
    GetLine(buffer, sizeof buffer);
    long int rc = strtol(buffer, &p, 10);
    if (p == buffer) rc = -1;

    return rc;
}

#if !defined(HAVE_WORKING_SOCKET_FDOPEN)
// fdopen on a socket does not work reliably on Win32 and EMX, so I do
// my own buffering on these platforms.

bool NntpRead::SendLine(const char *line)
{
    size_t linelength = strlen(line);
    unsigned tries = 0;

    // Flush buffers
    datainbuffer = 0;

    // Iterate until entire line is sent
    while (linelength)
    {
        int rc = send(sockfd, line, linelength, 0);

        // Check for possible errors
        if (-1 == rc)
        {
            // Ignore "interrupted", "try again" and "would block" errors.
            if (EINTR != errno && EAGAIN != errno && EWOULDBLOCK != errno)
            {
                return false;
            }
        }
        else
        {
            assert(rc <= linelength);

            // Remove the handled sent part of the buffer
            linelength -= rc;
            line += rc;

            // If we fail too many times, give up
            tries ++;
            if (tries > 10)
            {
                return false;
            }

            // Pause a second before trying again
            sleep(1);
        }
    }
    return true;
}

bool NntpRead::GetLine(char *outbuffer, size_t maxlen)
{
    // Always leave space for null termination
    maxlen --;

    bool foundlf = false;
    bool copiedanything = false;
    while (!foundlf)
    {
        // Copy any data in buffer until the LF
        if (datainbuffer)
        {
            // Find LF or end-of-buffer
            size_t bytes = datainbuffer;
            for (size_t i = 0; i < datainbuffer && !foundlf; i ++)
            {
                if ('\n' == socketbuffer[i])
                {
                    bytes = i + 1;
                    foundlf = true;
                }
            }

            // If there is any output buffer space left, copy data
            if (maxlen)
            {
                copiedanything = true;
                size_t copylen = bytes <? maxlen;
                memcpy(outbuffer, socketbuffer, copylen);
                outbuffer += copylen;
                maxlen -= copylen;
            }

            // Move the rest of the buffer down
            datainbuffer -= bytes;
            if (datainbuffer)
            {
                memmove(socketbuffer, socketbuffer + bytes, datainbuffer);
            }
        }

        // Read another chunk of data unless we found a LF already
        if (!foundlf)
        {
            int rc = recv(sockfd, socketbuffer, BUFSIZ, 0);
            if (-1 == rc)
            {
                // Ignore "interrupted" and "try again" errors.
                if (EINTR != errno && EAGAIN != errno)
                {
                    return false;
                }
            }
            else if (0 == rc)
            {
                // This indicates end-of-file
                // (or at least it did when I was using read(), I'll just
                //  keep it in for now)
                if (copiedanything)
                {
                    *outbuffer = 0;
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                datainbuffer = rc;
            }
        }
    }

    // Terminate buffer
    *outbuffer = 0;
    return true;
}
#endif
