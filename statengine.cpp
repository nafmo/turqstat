// Copyright (c) 1998-2000 Peter Karlsson
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

#include <limits.h>
#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>

#include "statengine.h"
#include "utility.h"

StatEngine::StatEngine(void)
{
    // Initialize count variables
    msgcount = 0;
    for (int i = 0; i < 7; i ++) daycount[i] = 0;
    for (int i = 0; i < 24; i ++) hourcount[i] = 0;
    numpeople = numsubjects = numprograms = numareas = numnets = 0;
    totallines = totalqlines = totalbytes = totalqbytes = 0;
    people_p = NULL;
    programs_p = NULL;
    subjects_p = NULL;
    nets_p = NULL;
    persontoplist_p = NULL;
    subjecttoplist_p = NULL;
    programtoplist_p = NULL;
    currversion = NULL;
    nettoplist_p = NULL;
    wdatevalid = false;
    rdatevalid = false;
    earliestwritten = latestwritten = earliestreceived = latestreceived = 0;
    currpersontype = None;
    hasarrivaltime = true;
    newsarea = false;
}

StatEngine::~StatEngine(void)
{
    // Deallocate any leftover data structures
    if (people_p) delete people_p;
    if (programs_p) delete programs_p;
    if (subjects_p) delete subjects_p;
    if (nets_p) delete nets_p;

    if (persontoplist_p) delete[] persontoplist_p;
    if (subjecttoplist_p) delete[] subjecttoplist_p;
    if (programtoplist_p) delete[] programtoplist_p;
}

void StatEngine::AddData(string fromname, string toname, string subject,
                         string controldata, string msgbody,
                         time_t timewritten, time_t timereceived)
{
    enum { None, Left, Right } direction;
    string internetaddress;

    // In news areas, we search the control information for From and
    // subject lines.
    if (newsarea)
    {
        string fromstring;

        int pos = controldata.find((char) 1);
        while (pos != -1)
        {
            pos ++;

            // Fidonet-gated news areas often prefix headers with "RFC-"
            if (fcompare(controldata.substr(pos, 4), "RFC-") == 0)
            {
                pos += 4;
            }

            if (fcompare(controldata.substr(pos, 5), "From:") == 0)
            {
                // Find where from name starts
                pos += 5;
                while (isspace(controldata[pos])) pos ++;

                // And where it ends
                int nextpos = controldata.find((char) 1, pos);

                // And copy it
                if (nextpos == -1)
                    fromstring = controldata.substr(pos);
                else
                    fromstring = controldata.substr(pos, nextpos - pos);

                // Prepare for next iteration
                pos = nextpos;
            }
            else if (fcompare(controldata.substr(pos, 8), "Subject:") == 0)
            {
                pos += 8;
                while (isspace(controldata[pos])) pos ++;
                int nextpos = controldata.find((char) 1, pos);

                if (nextpos == -1)
                    subject = controldata.substr(pos);
                else
                    subject = controldata.substr(pos, nextpos - pos);

                pos = nextpos;
            }
            else if (fcompare(controldata.substr(pos, 5), "Date:") == 0)
            {
                pos += 5;
                while (isspace(controldata[pos])) pos ++;

                int nextpos = controldata.find((char) 1, pos);

                if (nextpos == -1)
                    timewritten = rfcToTimeT(controldata.substr(pos));
                else
                    timewritten =
                        rfcToTimeT(controldata.substr(pos, nextpos - pos));

                pos = nextpos;
            }
            else
            {
                pos = controldata.find((char) 1, pos);
            }
        }

        // Parse From: string into name and address
        // "email@domain.com"
        // "email@domain.com (name)"
        // "name <email@domain.com>"
        int at    = fromstring.find('@');
        int left  = fromstring.find('(');
        int right = fromstring.find(')');
        if (-1 == left || -1 == right || at > left)
        {
            left  = fromstring.find('<');
            right = fromstring.find('>');
            if (-1 == left || -1 == right)
            {
                // "email@domain.com"
                fromname        = fromstring;
                internetaddress = fromstring;
            }
            else
            {
                // "name <email@domain.com>"
                fromname        = fromstring.substr(0, left - 1);
                internetaddress = fromstring.substr(left + 1, right - left - 1);
            }
        }
        else
        {
            // "email@domain.com (name)"
            fromname        = fromstring.substr(left + 1, right - left - 1);
            internetaddress = fromstring.substr(0, left - 1);
        }

        // Fixup name
        // remove quotes
        if ('\"' == fromname[0] && '\"' == fromname[fromname.length() - 1])
            fromname = fromname.substr(1, fromname.length() - 2);

        // kill QP
        fromname = DeQP(fromname);
        subject = DeQP(subject);

        if (0 == timewritten)
            timewritten = timereceived;
    }

    // Locate sender's name in database, and update statistics
    persondata_s *perstrav_p = people_p, *persfound_p = perstrav_p;
    if (NULL == perstrav_p)
    {
        people_p = new persondata_s;
        if (!people_p) errorquit(out_of_memory, 2);
        people_p->name = fromname;
        if (newsarea)
            people_p->address = internetaddress;
        else
            people_p->address = ParseAddress(controldata, msgbody);
        persfound_p = people_p;
        numpeople ++;
    }
    else
    {
        direction = None;
        do
        {
            // trav_p points to leaf above
            if (Left == direction) perstrav_p = perstrav_p->left;
            if (Right == direction) perstrav_p = perstrav_p->right;

            // found_p points to the leaf (NULL at the end, which is where
            // we need the leaf above to add the leaf to)
            if (fcompare(fromname, perstrav_p->name) < 0)
            {
                direction = Left;
                persfound_p = perstrav_p->left;
            }
            if (fcompare(fromname, perstrav_p->name) > 0)
            {
                direction = Right;
                persfound_p = perstrav_p->right;
            }
        } while (NULL != persfound_p &&
                 fcompare(persfound_p->name, fromname) != 0);

        if (NULL == persfound_p)
        {
            // No name was found, add a leaf with the new name
            persfound_p = new persondata_s;
            if (!persfound_p) errorquit(out_of_memory, 2);
            if (Left == direction) perstrav_p->left = persfound_p;
            if (Right == direction) perstrav_p->right = persfound_p;

            persfound_p->name = fromname;
            if (newsarea)
                persfound_p->address = internetaddress;
            else
                persfound_p->address = ParseAddress(controldata, msgbody);

            numpeople ++;
        }

        if ("N/A" == persfound_p->address)
        {
            persfound_p->address = ParseAddress(controldata, msgbody);
        }
    }

    persfound_p->messageswritten ++;

    // Loop over the message body and count:
    //  number of lines and bytes in message
    //  number of quoted lines and bytes in message
    unsigned lines = 0, bytes = 0, qlines = 0, qbytes = 0;
    unsigned lastlinebytes = 0;
    unsigned currindex = 0;
    bool isquoted = false, foundtear = false;
    do
    {
        int nextcr = msgbody.find('\n', currindex);
        int nextcr2 = msgbody.find('\r', currindex);
        if (nextcr2 >= 0 && (nextcr2 < nextcr || -1 == nextcr))
            nextcr = nextcr2;
        if (-1 == nextcr) nextcr = msgbody.length();
        string thisline = msgbody.substr((int) currindex, nextcr - currindex);

        // Break when we find a tear or Origin line
        if (!newsarea)
        {
            if (("--- " == thisline.substr(0, 4) ||
                thisline.length() == 3 && "---" == thisline))
            {
                foundtear = true;
            }
            else if (" * Origin: " == thisline.substr(0, 11))
            {
                break;
            }
            else
                foundtear = false;
        }

        // Is this a quote?
        int gt = thisline.find('>'), lt = thisline.find('<');
        if (gt >= 0 && gt < 6 && (-1 == lt || gt < lt))
        {
            isquoted = true;
        }

        // Test for some common Usenet style quotes
        if (newsarea && !isquoted)
        {
            int pipe = thisline.find('|');

            if (0 == pipe) isquoted = true;
        }

        lastlinebytes = nextcr - currindex;

        lines ++;
        bytes += lastlinebytes;
        if (isquoted)
        {
            qlines ++;
            qbytes += lastlinebytes;
            isquoted = false;
        }
        currindex = nextcr + 1;
    } while (currindex < msgbody.length());

    // Remove the previous line (tear line) from statistics
    if (foundtear)
    {
        lines --;
        bytes -= lastlinebytes;
    }

    persfound_p->byteswritten += bytes;
    persfound_p->lineswritten += lines;
    persfound_p->bytesquoted += qbytes;
    persfound_p->linesquoted += qlines;

    totalbytes  += bytes;
    totallines  += lines;
    totalqbytes += qbytes;
    totalqlines += qlines;
    msgcount ++;

    // Locate sender's net in database and update
    if (!newsarea && persfound_p->address != "N/A")
    {
        string myaddress = ParseAddress(controldata, msgbody);
        int colon = myaddress.find(':');
        int slash = myaddress.find('/', colon);
        if (colon != -1 && slash != -1)
        {
            const char *address = myaddress.c_str();
            unsigned zone = strtoul(address, NULL, 10);
            unsigned net  = strtoul(address + colon + 1, NULL, 10);

            netdata_s *nettrav_p = nets_p, *netfound_p = nettrav_p;

            if (NULL == nets_p)
            {
                nets_p = new netdata_s;
                if (!nets_p) errorquit(out_of_memory, 2);
                nets_p->zone = zone;
                nets_p->net  = net;

                netfound_p = nets_p;
                numnets ++;
            }
            else
            {
                direction = None;
                do
                {
                    // trav_p points to leaf abovie
                    if (Left == direction) nettrav_p = nettrav_p->left;
                    if (Right == direction) nettrav_p = nettrav_p->right;

                    // found_p points to the leaf (NULL at the end, which is
                    // where we need the leaf above to add the leaf to)
                    if (net == nettrav_p->net)
                    {
                        if (zone < nettrav_p->zone)
                        {
                            direction = Left;
                            netfound_p = nettrav_p->left;
                        }
                        else if (zone > nettrav_p->zone)
                        {
                            direction = Right;
                            netfound_p = nettrav_p->right;
                        }
                    }
                    else if (net < nettrav_p->net)
                    {
                        direction = Left;
                        netfound_p = nettrav_p->left;
                    }
                    else // net > nettrav_p->net
                    {
                        direction = Right;
                        netfound_p = nettrav_p->right;
                    }

                } while (NULL != netfound_p &&
                         (zone != netfound_p->zone || net != netfound_p->net));

                if (NULL == netfound_p)
                {
                    // No net was found
                    netfound_p = new netdata_s;
                    if (!netfound_p) errorquit(out_of_memory, 2);
                    if (Left == direction) nettrav_p->left = netfound_p;
                    if (Right == direction) nettrav_p->right = netfound_p;

                    netfound_p->zone = zone;
                    netfound_p->net  = net;

                    numnets ++;
                }
            }

            netfound_p->count ++;
            netfound_p->bytes += bytes;
        }
    }

    // Locate recipient's name in database, and update statistics
    if (!newsarea)
    {
        persfound_p = perstrav_p = people_p; // people_p can't be NULL here
        direction = None;
        do
        {
            // trav_p points to leaf above
            if (Left == direction) perstrav_p = perstrav_p->left;
            if (Right == direction) perstrav_p = perstrav_p->right;

            // found_p points to the leaf (NULL at the end, which is where
            // we need the leaf above to add the leaf to)
            if (fcompare(toname, perstrav_p->name) < 0)
            {
                direction = Left;
                persfound_p = perstrav_p->left;
            }
            if (fcompare(toname, perstrav_p->name) > 0)
            {
                direction = Right;
                persfound_p = perstrav_p->right;
            }
        } while (NULL != persfound_p &&
                 fcompare(persfound_p->name, toname) != 0);

        if (NULL == persfound_p)
        {
            // No name was found, add a leaf with the new name
            persfound_p = new persondata_s;
            if (!persfound_p) errorquit(out_of_memory, 2);
            if (Left == direction) perstrav_p->left = persfound_p;
            if (Right == direction) perstrav_p->right = persfound_p;

            persfound_p->name = toname;
            persfound_p->address = "N/A";

            numpeople ++;
        }

        persfound_p->messagesreceived ++;
    }

    if (subject.length() > 3)
    {
        while (fcompare(subject.substr(0, 3), "re:") == 0)
        {
            if (' ' == subject[3])
            {
                subject = subject.substr(4, subject.length() - 4);
            }
            else
            {
                subject = subject.substr(3, subject.length() - 3);
            }
        }
    }

    // Locate subject line in database, and update statistics
    subjectdata_s *subtrav_p = subjects_p, *subfound_p = subtrav_p;
    if (NULL == subjects_p)
    {
        subjects_p = new subjectdata_s;
        if (!subjects_p) errorquit(out_of_memory, 2);
        subjects_p->subject = subject;
        subfound_p = subjects_p;

        numsubjects ++;
    }
    else
    {
        direction = None;
        do
        {
            // trav_p points to leaf above
            if (Left == direction) subtrav_p = subtrav_p->left;
            if (Right == direction) subtrav_p = subtrav_p->right;

            // found_p points to the leaf (NULL at the end, which is where
            // we need the leaf above to add the leaf to)
            if (fcompare(subject, subtrav_p->subject) < 0)
            {
                direction = Left;
                subfound_p = subtrav_p->left;
            }
            if (fcompare(subject, subtrav_p->subject) > 0)
            {
                direction = Right;
                subfound_p = subtrav_p->right;
            }
        } while (NULL != subfound_p &&
                 fcompare(subfound_p->subject, subject) != 0);

        if (NULL == subfound_p)
        {
            // No name was found, add a leaf with the new name
            subfound_p = new subjectdata_s;
            if (!subfound_p) errorquit(out_of_memory, 2);
            if (Left == direction) subtrav_p->left = subfound_p;
            if (Right == direction) subtrav_p->right = subfound_p;

            subfound_p->subject = subject;

            numsubjects ++;
        }
    }

    subfound_p->count ++;
    subfound_p->bytes += bytes;

    // Locate program name and version in database, and update statistics
    string program = "";
    string programname;
    int space1, space2;

    if (newsarea)
    {
        // User-Agent takes precedence over X-Newsreader
        int where;
        if ((where = controldata.find("X-Newsreader:")) != -1)
        {
            int howfar = controldata.find((char) 1, where);
            where += 14;
            while (isspace(controldata[where])) where ++;
            program = controldata.substr(where, howfar - where);
        }
        else if ((where = controldata.find("User-Agent:")) != -1)
        {
            int howfar = controldata.find((char) 1, where);
            where += 11;
            while (isspace(controldata[where])) where ++;
            program = controldata.substr(where, howfar - where);
        }

        // Split program name and version
        // It is believed to be divided as such:
        // "Program<space>Version<eol>" or
        // "Program<space>Version<space>(extra)<eol>" or
        // "Program/Version<space>rest" or
        // "Program/Version<space>rest<space>(extra)<eol>" or
        // "Program<space>(version)<eol>"

        space1 = program.rfind(' ');
        int firstspace = program.find(' ');
        int slash = program.find('/');
        int paren = program.find('(');
        if (-1 == space1) firstspace = space1 = program.length();
        if (slash != -1 && slash < firstspace) space1 = slash;
        if (paren != -1 && paren < space1) space1 = paren;

        // Special case: "Gnus v#.##/Emacs..."
        if ('v' == program[firstspace + 1] &&
            isdigit(program[firstspace + 2]))
            space1 = firstspace;

        paren = program.find('(', space1 + 1);
        space2 = program.find(' ', space1 + 1);
        if (-1 == space2)
            if (-1 == paren)
                space2 = program.length();
            else
                space2 = paren;

        programname = program.substr(0, space1);
    }
    else
    {
        // PID takes precedence over tearline
        int where;
        if ((where = controldata.find("PID: ")) != -1)
        {
            int howfar = controldata.find((char) 1, where);
            if (-1 == howfar) howfar = controldata.length();
            program = controldata.substr(where + 5, howfar - where - 5);
        }
        else if ((where = msgbody.rfind("--- ")) != -1)
        {
            int howfar = msgbody.find('\n', where);
            int nextcr = msgbody.find('\r', where);
            if (-1 == howfar || (nextcr != -1 && nextcr < howfar))
                howfar = nextcr;
            if (-1 == howfar) howfar = msgbody.length();
            program = msgbody.substr(where + 4, howfar - where - 4);
        }

        // Split program name and version
        // It is believed to be divided as such:
        // "Program<space>Version<space>Other"
        // the last part is not counted

        space1 = program.find(' ');
        space2 = program.find(' ', space1 + 1);
        if (-1 == space1) space1 = space2 = program.length();
        if (-1 == space2) space2 = program.length();

        if (space1 && '+' == program[space1 - 1])
            programname = program.substr(0, space1 - 1);
        else
            programname = program.substr(0, space1);
    }

    if (programname != "")
    {
        programdata_s *progtrav_p = programs_p, *progfound_p = progtrav_p;
        if (NULL == programs_p)
        {
            programs_p = new programdata_s;
            if (!programs_p) errorquit(out_of_memory, 2);
            programs_p->programname = programname;
            progfound_p = programs_p;

            numprograms ++;
        }
        else
        {
            direction = None;
            do
            {
                // trav_p points to leaf above
                if (Left == direction) progtrav_p = progtrav_p->left;
                if (Right == direction) progtrav_p = progtrav_p->right;

                // found_p points to the leaf (NULL at the end, which is where
                // we need the leaf above to add the leaf to)
                if (fcompare(programname, progtrav_p->programname) < 0)
                {
                    direction = Left;
                    progfound_p = progtrav_p->left;
                }
                if (fcompare(programname, progtrav_p->programname) > 0)
                {
                    direction = Right;
                    progfound_p = progtrav_p->right;
                }
            } while (NULL != progfound_p &&
                     fcompare(progfound_p->programname, programname) != 0);

            if (NULL == progfound_p)
            {
                // No name was found, add a leaf with the new name
                progfound_p = new programdata_s;
                if (!progfound_p) errorquit(out_of_memory, 2);
                if (Left == direction) progtrav_p->left = progfound_p;
                if (Right == direction) progtrav_p->right = progfound_p;

                progfound_p->programname = programname;

                numprograms ++;
            }
        }

        progfound_p->count ++;

        if (space2 - space1 > 1)
        {
            // Locate version number in the linked list, if we do not
            // find it, add it to it.
            string programvers;

            if (space2 && '+' == program[space2 - 1] && space2 - space1 > 2)
                programvers = program.substr(space1 + 1, space2 - space1 - 2);
            else
                programvers = program.substr(space1 + 1, space2 - space1 - 1);

            programversion_s **vertrav_pp = &(progfound_p->versions_p);
            while (*vertrav_pp != NULL &&
                   fcompare((*vertrav_pp)->version, programvers) < 0)
            {
                vertrav_pp = &((*vertrav_pp)->next);
            }

            // If we didn't find it, add it here.
            if (NULL == *vertrav_pp ||
                0 != fcompare((*vertrav_pp)->version, programvers))
            {
                programversion_s *next_p = *vertrav_pp; // =NULL if at end
                *vertrav_pp = new programversion_s;
                if (!*vertrav_pp) errorquit(out_of_memory, 2);
                (*vertrav_pp)->version = programvers;
                (*vertrav_pp)->next    = next_p;
            }

            // Increase count
            (*vertrav_pp)->count ++;
        }
    }

    // Check writing and reception date and add to statistics
    if (timewritten > 0) // zero or negative date indicates error
    {
        if (!wdatevalid || timewritten  < earliestwritten)
            earliestwritten  = timewritten;
        if (!wdatevalid || timewritten  > latestwritten)
            latestwritten    = timewritten;
        wdatevalid = true;

        struct tm *tm_p = localtime(&timewritten);
        if (tm_p)
        {
            daycount[tm_p->tm_wday] ++;
            hourcount[tm_p->tm_hour] ++;
        }
    }

    if (timereceived > 0)
    {
        if (!rdatevalid || timereceived < earliestreceived)
            earliestreceived = timereceived;
        if (!rdatevalid || timereceived > latestreceived)
            latestreceived   = timereceived;
        rdatevalid = true;
    }
}

void StatEngine::AreaDone(void)
{
    numareas ++;
}

string StatEngine::ParseAddress(string controldata, string msgbody)
{
    // Locate Origin
    // Handles these formats:
    // (address)
    // (network address)
    // (address@network)
    // (network#address)
    int index = msgbody.rfind(" * Origin: ");
    if (-1 != index)
    {
        // Locate last '(' and ')' parenthesis on line
        int endsat = msgbody.find('\n', index);
        int endsat2 = msgbody.find('\r', index);
        if (endsat2 >= 0 && (endsat2 < endsat || -1 == endsat))
            endsat = endsat2;
        if (-1 == endsat) endsat = msgbody.length();
        int leftparen = msgbody.find('(', index), prevleftparen = leftparen;
        while (leftparen != -1 && leftparen < endsat)
        {
            prevleftparen = leftparen;
            leftparen = msgbody.find('(', leftparen + 1);
        }
        leftparen = prevleftparen; // for clarity
        int rightparen = msgbody.find(')', leftparen);
        if (-1 != rightparen && rightparen < endsat)
        {
            // Okay, we've found them, get the address
            int space  = msgbody.find(' ', leftparen + 1);
            int atsign = msgbody.find('@', leftparen + 1);
            int number = msgbody.find('#', leftparen + 1);

            if (space  != -1 && space  < rightparen) leftparen  = space;
            if (atsign != -1 && atsign < rightparen) rightparen = atsign;
            if (number != -1 && number < rightparen) leftparen  = number;

            return msgbody.substr(leftparen + 1, rightparen - leftparen - 1);
        }
    }

    // Locate MSGID
    // Handles these formats:
    // MSGID network#address serial
    // MSGID address@network serial
    // MSGID address serial
    index = controldata.find("MSGID: ");
    if (-1 != index)
    {
        int space = controldata.find(' ', index + 1);
        if (-1 != space)
        {
            int ends   = controldata.find(' ', space + 1);
            int atsign = controldata.find('@', space + 1);
            int number = controldata.find('#', space + 1);

            if (atsign != -1 && atsign < ends) ends = atsign;
            if (number != -1 && number < ends) space = number;
            if (-1 != space)
            {
                return controldata.substr(space + 1, ends - index - 7);
            }
        }
    }

    // Give up
    return string("N/A");
}

string StatEngine::DeQP(string qp)
{
    string rc = "", hex = "";
    int qpchar, pos, endpos, current = 0;

    pos = qp.find("=?");
    while (pos >= 0)
    {
        if (0 == fcompare(qp.substr(pos, 15), string("=?iso-8859-1?Q?")))
        {
            rc += qp.substr(current, pos - current);
            endpos = qp.find("?=", pos + 15);
            current = endpos + 2;
            for (int i = pos + 15; i < endpos; i ++)
            {
                if ('=' == qp[i])
                {
                    hex = qp.substr(i + 1, 2);
                    sscanf(hex.c_str(), "%x", &qpchar);
                    rc += (char) qpchar;
                    i += 2;
                }
                else if ('_' == qp[i])
                    rc += ' ';
                else
                    rc += qp[i];
            }
            pos = current;
        }
        else
            pos ++;

        pos = qp.find("=?", pos);
    }

    rc += qp.substr(current);

    return rc;
}

bool StatEngine::GetTop(bool restart, persstat_s &result,
    int (*compar)(const void *, const void *), list_t type)
{
    if (restart || currpersontype != type || NULL == persontoplist_p)
    {
        // Generate a top list
        // 1. Flatten the binary tree into the array
        if (NULL != persontoplist_p) delete[] persontoplist_p;

        persontoplist_p = new persstat_s[numpeople];
        if (!persontoplist_p) errorquit(out_of_memory, 2);
        flattenindex = 0;
        FlattenPeople(persontoplist_p, people_p);

        // 2. Sort the list according to number of written texts
        qsort(persontoplist_p, numpeople, sizeof(persstat_s),
              compar);
        currperson = 0;
        currpersontype = type;
    }
    else
        currperson ++;

    if (currperson >= numpeople)
    {
        delete[] persontoplist_p;
        persontoplist_p = NULL;
        return false;
    }

    result = persontoplist_p[currperson];

    return true;
}

void StatEngine::FlattenPeople(persstat_s *array, persondata_s *p)
{
    if (!p) return;

    array[flattenindex].name             = p->name;
    array[flattenindex].address          = p->address;
    array[flattenindex].byteswritten     = p->byteswritten;
    array[flattenindex].lineswritten     = p->lineswritten;
    array[flattenindex].bytesquoted      = p->bytesquoted;
    array[flattenindex].linesquoted      = p->linesquoted;
    array[flattenindex].messageswritten  = p->messageswritten;
    array[flattenindex].messagesreceived = p->messagesreceived;

    flattenindex ++;

    FlattenPeople(array, p->left);
    FlattenPeople(array, p->right);
}

bool StatEngine::GetTopSubjects(bool restart, subjstat_s &result)
{
    if (restart || NULL == subjecttoplist_p)
    {
        // Generate a top list
        // 1. Flatten the binary tree into the array
        if (NULL != subjecttoplist_p) delete[] subjecttoplist_p;

        subjecttoplist_p = new subjstat_s[numsubjects];
        if (!subjecttoplist_p) errorquit(out_of_memory, 2);
        flattenindex = 0;
        FlattenSubjects(subjecttoplist_p, subjects_p);

        // 2. Sort the list according to number of written texts
        qsort(subjecttoplist_p, numsubjects, sizeof(subjstat_s),
              comparesubject);
        currsubject = 0;
    }
    else
        currsubject ++;

    if (currsubject >= numsubjects)
    {
        delete[] subjecttoplist_p;
        subjecttoplist_p = NULL;
        return false;
    }

    result = subjecttoplist_p[currsubject];

    return true;
}

void StatEngine::FlattenSubjects(subjstat_s *array, subjectdata_s *p)
{
    if (!p) return;

    array[flattenindex].subject = p->subject;
    array[flattenindex].bytes   = p->bytes;
    array[flattenindex].count   = p->count;

    flattenindex ++;

    FlattenSubjects(array, p->left);
    FlattenSubjects(array, p->right);
}

bool StatEngine::GetTopPrograms(bool restart, progstat_s &result)
{
    if (restart || NULL == programtoplist_p)
    {
        // Generate a top list
        // 1. Flatten the binary tree into the array
        if (NULL != programtoplist_p) delete[] programtoplist_p;

        programtoplist_p = new progstat_s[numprograms];
        if (!programtoplist_p) errorquit(out_of_memory, 2);
        flattenindex = 0;
        FlattenPrograms(programtoplist_p, programs_p);

        // 2. Sort the list according to number of written texts
        qsort(programtoplist_p, numprograms, sizeof(progstat_s),
              compareprogram);
        currprogram = 0;
    }
    else
        currprogram ++;

    if (currprogram >= numprograms)
    {
        delete[] programtoplist_p;
        programtoplist_p = NULL;
        return false;
    }

    result = programtoplist_p[currprogram];

    return true;
}

void StatEngine::FlattenPrograms(progstat_s *array, programdata_s *p)
{
    if (!p) return;

    array[flattenindex].program  = p->programname;
    array[flattenindex].versions = p->versions_p;
    array[flattenindex].count    = p->count;

    flattenindex ++;

    FlattenPrograms(array, p->left);
    FlattenPrograms(array, p->right);
}

bool StatEngine::GetProgramVersions(bool restart, verstat_s &result)
{
    if (NULL == programtoplist_p || currprogram >= numprograms)
        return false;

    if (restart)
        currversion =
            (programversion_s *) programtoplist_p[currprogram].versions;
    else
        currversion = currversion->next;

    if (NULL == currversion)
        return false;

    result.version = currversion->version;
    result.count =   currversion->count;
    return true;
}

bool StatEngine::GetTopNets(bool restart, netstat_s &result)
{
    if (restart || NULL == nettoplist_p)
    {
        // Generate a top list
        // 1. Flatten the binary tree into the array
        if (NULL != nettoplist_p) delete[] nettoplist_p;

        nettoplist_p = new netstat_s[numnets];
        if (!nettoplist_p) errorquit(out_of_memory, 2);
        flattenindex = 0;
        FlattenNets(nettoplist_p, nets_p);

        // 2. Sort the list according to number of occurances
        qsort(nettoplist_p, numnets, sizeof(netstat_s),
              comparenets);
        currnet = 0;
    }
    else
        currnet ++;

    if (currnet >= numnets)
    {
        delete[] nettoplist_p;
        nettoplist_p = NULL;
        return false;
    }

    result = nettoplist_p[currnet];

    return true;
}

void StatEngine::FlattenNets(netstat_s *array, netdata_s *p)
{
    if (!p) return;

    array[flattenindex].zone     = p->zone;
    array[flattenindex].net      = p->net;
    array[flattenindex].messages = p->count;
    array[flattenindex].bytes    = p->bytes;

    flattenindex ++;

    FlattenNets(array, p->left);
    FlattenNets(array, p->right);
}

int comparenumwritten(const void *p1, const void *p2)
{
    unsigned d1 = (((StatEngine::persstat_s *) p2)->messageswritten);
    unsigned d2 = (((StatEngine::persstat_s *) p1)->messageswritten);

    if (d1 == d2)
        return ((int) (((StatEngine::persstat_s *) p2)->byteswritten)) -
               ((int) (((StatEngine::persstat_s *) p1)->byteswritten));

    if (d1 < d2) return -1;
    return 1;
}

int comparenumreceived(const void *p1, const void *p2)
{
    unsigned d1 = (((StatEngine::persstat_s *) p2)->messagesreceived);
    unsigned d2 = (((StatEngine::persstat_s *) p1)->messagesreceived);

    if (d1 == d2)
        return ((int) (((StatEngine::persstat_s *) p1)->messageswritten)) -
               ((int) (((StatEngine::persstat_s *) p2)->messageswritten));

    if (d1 < d2) return -1;
    return 1;
}

int comparenumquoted(const void *p1, const void *p2)
{
    // Special handling of the case with no bytes written (recipient only)
    if (0 == (((StatEngine::persstat_s *) p1)->byteswritten))
    {
        if (0 == (((StatEngine::persstat_s *) p2)->byteswritten))
            return 0;
        else
            return -1;
    }
    else if (0 == (((StatEngine::persstat_s *) p2)->byteswritten))
        return 1;

    unsigned long long d1 =
        ((unsigned long long)
         (((StatEngine::persstat_s *) p1)->bytesquoted)) *
        ((unsigned long long)
         (((StatEngine::persstat_s *) p2)->byteswritten));
    unsigned long long d2 =
        ((unsigned long long)
         (((StatEngine::persstat_s *) p2)->bytesquoted)) *
        ((unsigned long long)
         (((StatEngine::persstat_s *) p1)->byteswritten));

    if (d1 < d2) return 1;
    if (d1 > d2) return -1;
    return 0;
}

int comparebytepermsg(const void *p1, const void *p2)
{
    unsigned long d1 =
        ((unsigned long) (((StatEngine::persstat_s *) p1)->byteswritten)) *
        ((unsigned long) (((StatEngine::persstat_s *) p2)->messageswritten));
    unsigned long d2 =
        ((unsigned long) (((StatEngine::persstat_s *) p2)->byteswritten)) *
        ((unsigned long) (((StatEngine::persstat_s *) p1)->messageswritten));

    if (d1 < d2) return -1;
    if (d1 > d2) return 1;
    return 0;
}

int compareoriginalpermsg(const void *p1, const void *p2)
{
    // Special handling of the case with no messages written (recipient only)
    if (0 == (((StatEngine::persstat_s *) p1)->messageswritten))
    {
        if (0 == (((StatEngine::persstat_s *) p2)->messageswritten))
            return 0;
        else
            return 1;
    }
    else if (0 == (((StatEngine::persstat_s *) p2)->messageswritten))
        return -1;

    unsigned long d1 =
        ((unsigned long) (((StatEngine::persstat_s *) p1)->byteswritten -
                          ((StatEngine::persstat_s *) p1)->bytesquoted)) *
        ((unsigned long) ((StatEngine::persstat_s *) p2)->messageswritten);
    unsigned long d2 =
        ((unsigned long) (((StatEngine::persstat_s *) p2)->byteswritten -
                          ((StatEngine::persstat_s *) p2)->bytesquoted)) *
        ((unsigned long) ((StatEngine::persstat_s *) p1)->messageswritten);

    if (d1 > d2) return -1;
    if (d1 < d2) return 1;
    return 0;
}

int comparesubject(const void *p1, const void *p2)
{
    unsigned d1 = (((StatEngine::subjstat_s *) p2)->count);
    unsigned d2 = (((StatEngine::subjstat_s *) p1)->count);
    if (d1 == d2)
        return ((int) (((StatEngine::subjstat_s *) p2)->bytes)) -
               ((int) (((StatEngine::subjstat_s *) p1)->bytes));
    if (d1 < d2) return -1;
    return 1;
}

int compareprogram(const void *p1, const void *p2)
{
    return ((int) (((StatEngine::progstat_s *) p2)->count)) -
           ((int) (((StatEngine::progstat_s *) p1)->count));
}

int comparenets(const void *p1, const void *p2)
{
    return ((int) (((StatEngine::netstat_s *) p2)->messages)) -
           ((int) (((StatEngine::netstat_s *) p1)->messages));
}
