// Copyright (c) 1998-2001 Peter Karlsson
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

#include <limits.h>
#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>

#include "statengine.h"
#include "output.h"

StatEngine::StatEngine(void)
{
    // Initialize count variables
    msgcount = 0;
    for (int i = 0; i < 7; i ++) daycount[i] = 0;
    for (int i = 0; i < 24; i ++) hourcount[i] = 0;
    numpeople = numsubjects = numprograms = numareas = numnets = numdomains = 0;
    totallines = totalqlines = totalbytes = totalqbytes = 0;
    people_p = NULL;
    programs_p = NULL;
    subjects_p = NULL;
    nets_p = NULL;
    domains_p = NULL;
    persontoplist_p = NULL;
    subjecttoplist_p = NULL;
    programtoplist_p = NULL;
    currversion = NULL;
    nettoplist_p = NULL;
    domaintoplist_p = NULL;
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
    if (domains_p) delete domains_p;

    if (persontoplist_p) delete[] persontoplist_p;
    if (subjecttoplist_p) delete[] subjecttoplist_p;
    if (programtoplist_p) delete[] programtoplist_p;
    if (domaintoplist_p) delete[] domaintoplist_p;
}

void StatEngine::AddData(string in_fromname, string in_toname, string in_subject,
                         string controldata, string msgbody,
                         time_t timewritten, time_t timereceived)
{
    enum { None, Left, Right } direction;
    string internetaddress;
#ifdef HAVE_WORKING_WSTRING
    wstring fromname, toname, subject;
#else
    wstring fromname(32), toname(32), subject(32);
#endif

    // Create a decoder for input data
    Decoder *decoder_p = NULL;
    if (newsarea)
    {
        decoder_p = Decoder::GetDecoderByMIMEHeaders(controldata.c_str());
    }
    else
    {
        decoder_p = Decoder::GetDecoderByKludges(controldata.c_str());
    }

    // In news areas, we search the control information for From and
    // subject lines.
    if (newsarea || controldata.find("\x1""RFC-From") != string::npos ||
        controldata.find("\x1""REPLYADDR") != string::npos)
    {
        string fromstring;
        bool foundfrom = false;

        string::size_type pos = controldata.find((char) 1);
        while (pos != string::npos)
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
                string::size_type nextpos = controldata.find((char) 1, pos);

                // And copy it
                if (string::npos == nextpos)
                    fromstring = controldata.substr(pos);
                else
                    fromstring = controldata.substr(pos, nextpos - pos);

                // Indicate found
                foundfrom = true;

                // Prepare for next iteration
                pos = nextpos;
            }
            // Fidonet gated RFC messages (FSC-35)
            else if (fcompare(controldata.substr(pos, 10), "REPLYADDR ") == 0)
            {
                // Find where from name starts
                pos += 9;
                while (isspace(controldata[pos])) pos ++;

                // And where it ends
                string::size_type nextpos = controldata.find((char) 1, pos);

                // And copy it
                if (string::npos == nextpos)
                    fromstring = controldata.substr(pos);
                else
                    fromstring = controldata.substr(pos, nextpos - pos);

                // Indicate found
                foundfrom = true;

                // Prepare for next iteration
                pos = nextpos;
            }
            else if (fcompare(controldata.substr(pos, 8), "Subject:") == 0)
            {
                pos += 8;
                while (isspace(controldata[pos])) pos ++;
                string::size_type nextpos = controldata.find((char) 1, pos);

                if (string::npos == nextpos)
                    in_subject = controldata.substr(pos);
                else
                    in_subject = controldata.substr(pos, nextpos - pos);

                pos = nextpos;
            }
            else if (fcompare(controldata.substr(pos, 5), "Date:") == 0)
            {
                pos += 5;
                while (isspace(controldata[pos])) pos ++;

                string::size_type nextpos = controldata.find((char) 1, pos);

                if (string::npos == nextpos)
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

        if (foundfrom)
        {
            // Parse From: string into name and address
            // "email@domain.com"
            // "email@domain.com (name)"
            // "name <email@domain.com>"
            string::size_type at    = fromstring.find('@');
            string::size_type left  = fromstring.find('(');
            string::size_type right = fromstring.find(')');
            if (string::npos == left || string::npos == right || at > left)
            {
                left  = fromstring.find('<');
                right = fromstring.find('>');
                if (string::npos == left || string::npos == right)
                {
                    // "email@domain.com"
                    if (in_fromname.empty())
                    {
                        in_fromname = fromstring;
                    }
                    internetaddress = fromstring;
                }
                else
                {
                    // "name <email@domain.com>"
                    in_fromname = fromstring.substr(0, left - 1);
                    internetaddress = fromstring.substr(left + 1,
                                                        right - left - 1);
                }
            }
            else
            {
                // "email@domain.com (name)"
                in_fromname = fromstring.substr(left + 1, right - left - 1);
                internetaddress = fromstring.substr(0, left - 1);
            }

            // Fixup name
            // remove quotes
            if ('"' == in_fromname[0] &&
                '"' == in_fromname[in_fromname.length() - 1])
                in_fromname = in_fromname.substr(1, in_fromname.length() - 2);
        }

        // kill QP
        if (foundfrom || !newsarea) fromname = DeQP(in_fromname, decoder_p);
        if (in_subject.length()) subject = DeQP(in_subject, decoder_p);

        if (0 == timewritten)
            timewritten = timereceived;
    }
    else
    {
        subject = decoder_p->Decode(in_subject);
        fromname = decoder_p->Decode(in_fromname);
    }

    toname = decoder_p->Decode(in_toname);

    // Locate sender's name in database, and update statistics
    persondata_s *perstrav_p = people_p, *persfound_p = perstrav_p;
    if (NULL == perstrav_p)
    {
        people_p = new persondata_s;
        if (!people_p)
        {
            TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory, 2);
        }
        people_p->name = fromname;
        if (newsarea || !internetaddress.empty())
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
            else if (fcompare(fromname, perstrav_p->name) > 0)
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
            if (!persfound_p)
            {
                TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory,
                                                       2);
            }
            if (Left == direction) perstrav_p->left = persfound_p;
            if (Right == direction) perstrav_p->right = persfound_p;

            persfound_p->name = fromname;
            if (newsarea || !internetaddress.empty())
                persfound_p->address = internetaddress;
            else
                persfound_p->address = ParseAddress(controldata, msgbody);

            numpeople ++;
        }

        if ("N/A" == persfound_p->address)
        {
            if (newsarea || !internetaddress.empty())
                persfound_p->address = internetaddress;
            else
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
        string::size_type nextcr = msgbody.find('\n', currindex);
        string::size_type nextcr2 = msgbody.find('\r', currindex);
        if (nextcr2 != string::npos &&
            (nextcr2 < nextcr || string::npos == nextcr))
            nextcr = nextcr2;
        if (string::npos == nextcr) nextcr = msgbody.length();
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
        string::size_type gt = thisline.find('>'), lt = thisline.find('<');
        if (gt >= 0 && gt < 6 && (string::npos == lt || gt < lt))
        {
            isquoted = true;
        }

        // Test for some common Usenet style quotes
        if (newsarea && !isquoted)
        {
            string::size_type pipe = thisline.find('|');

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
        string::size_type colon = myaddress.find(':');
        string::size_type slash = myaddress.find('/', colon);
        if (colon != string::npos && slash != string::npos)
        {
            const char *address = myaddress.c_str();
            unsigned zone = strtoul(address, NULL, 10);
            unsigned net  = strtoul(address + colon + 1, NULL, 10);

            netdata_s *nettrav_p = nets_p, *netfound_p = nettrav_p;

            if (NULL == nets_p)
            {
                nets_p = new netdata_s;
                if (!nets_p)
                {
                    TDisplay::GetOutputObject()->
                        ErrorQuit(TDisplay::out_of_memory, 2);
                }

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
                    else if (net > nettrav_p->net)
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
                    if (!netfound_p)
                    {
                        TDisplay::GetOutputObject()->
                            ErrorQuit(TDisplay::out_of_memory, 2);
                    }
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

    // Locate sender's domain in database and update
    if (!internetaddress.empty())
    {
        string::size_type lastperiod = internetaddress.rfind('.');
        if (lastperiod != string::npos)
        {
            string topdomain = internetaddress.substr(lastperiod + 1);

            // Make sure top domain is in lowercase
            for (unsigned i = 0; i < topdomain.length(); i ++)
                if (isupper(topdomain[i]))
                    topdomain[i] = tolower(topdomain[i]);

            domaindata_s *domaintrav_p = domains_p,
                         *domainfound_p = domaintrav_p;

            if (NULL == domains_p)
            {
                domains_p = new domaindata_s;
                if (!domains_p)
                {
                    TDisplay::GetOutputObject()->
                        ErrorQuit(TDisplay::out_of_memory, 2);
                }
                domains_p->topdomain = topdomain;

                domainfound_p = domains_p;
                numdomains ++;
            }
            else
            {
                direction = None;
                do
                {
                    // trav_p points to leaf above
                    if (Left == direction) domaintrav_p = domaintrav_p->left;
                    if (Right == direction) domaintrav_p = domaintrav_p->right;

                    // found_p points to the leaf (NULL at the end, which is
                    // where we need the leaf above to add the leaf to)
                    if (topdomain < domaintrav_p->topdomain)
                    {
                        direction = Left;
                        domainfound_p = domaintrav_p->left;
                    }
                    else if (topdomain > domaintrav_p->topdomain)
                    {
                        direction = Right;
                        domainfound_p = domaintrav_p->right;
                    }

                } while (NULL != domainfound_p &&
                         topdomain != domainfound_p->topdomain);

                if (NULL == domainfound_p)
                {
                    // No net was found
                    domainfound_p = new domaindata_s;
                    if (!domainfound_p)
                    {
                        TDisplay::GetOutputObject()->
                            ErrorQuit(TDisplay::out_of_memory, 2);
                    }
                    if (Left == direction) domaintrav_p->left = domainfound_p;
                    if (Right == direction) domaintrav_p->right = domainfound_p;

                    domainfound_p->topdomain = topdomain;

                    numdomains ++;
                }
            }

            domainfound_p->count ++;
            domainfound_p->bytes += bytes;
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
            else if (fcompare(toname, perstrav_p->name) > 0)
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
            if (!persfound_p)
            {
                TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory,
                                                       2);
            }
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
        while (fcompare(subject, wstring(L"re:"), 3) == 0)
        {
            if (' ' == subject[3])
            {
#ifdef HAVE_WORKING_WSTRING
                subject = subject.substr(4, subject.length() - 4);
#else
                subject.skip(4);
#endif
            }
            else
            {
#ifdef HAVE_WORKING_WSTRING
                subject = subject.substr(3, subject.length() - 3);
#else
                subject.skip(3);
#endif
            }
        }
    }

    // Locate subject line in database, and update statistics
    subjectdata_s *subtrav_p = subjects_p, *subfound_p = subtrav_p;
    if (NULL == subjects_p)
    {
        subjects_p = new subjectdata_s;
        if (!subjects_p)
        {
            TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory, 2);
        }
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
            else if (fcompare(subject, subtrav_p->subject) > 0)
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
            if (!subfound_p)
            {
                TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory,
                                                       2);
            }
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
    string in_programname;
    string::size_type space1, space2;

    if (newsarea)
    {
        // User-Agent takes precedence over X-Newsreader
        string::size_type where;
        if ((where = controldata.find("X-Newsreader:")) != string::npos)
        {
            int howfar = controldata.find((char) 1, where);
            where += 14;
            while (isspace(controldata[where])) where ++;
            program = controldata.substr(where, howfar - where);
        }
        else if ((where = controldata.find("User-Agent:")) != string::npos)
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
        string::size_type firstspace = program.find(' ');
        string::size_type slash = program.find('/');
        string::size_type paren = program.find('(');
        if (string::npos == space1) firstspace = space1 = program.length();
        if (slash != string::npos && slash < firstspace) space1 = slash;
        if (paren != string::npos && paren < space1) space1 = paren;

        // Special case: "Gnus v#.##/Emacs..."
        if ('v' == program[firstspace + 1] &&
            isdigit(program[firstspace + 2]))
            space1 = firstspace;

        paren = program.find('(', space1 + 1);
        space2 = program.find(' ', space1 + 1);
        if (string::npos == space2)
        {
            if (string::npos == paren)
                space2 = program.length();
            else
                space2 = paren;
        }

        in_programname = program.substr(0, space1);

        // Special case: Pine (only identifies in Message-ID)
        where = controldata.find("\x1""Message-ID:");
        if (in_programname.empty() && where != string::npos)
        {
            where ++;
            string::size_type howfar = controldata.find((char) 1, where);
            if (fcompare(controldata.substr(where + 13, 5), "Pine.") == 0)
            {
                program =
                    controldata.substr(where, howfar - where);
                in_programname = "Pine";
                space1 = 17;
                string::size_type firstperiod = program.find('.', 18);
                if (firstperiod != string::npos) program[firstperiod] = ' ';
                space2 = program.find('.', 24);
            }
        }
    }
    else
    {
        // PID takes precedence over tearline
        string::size_type where;
        if ((where = controldata.find("PID: ")) != string::npos)
        {
            string::size_type howfar = controldata.find((char) 1, where);
            if (string::npos == howfar) howfar = controldata.length();
            program = controldata.substr(where + 5, howfar - where - 5);
        }
        else if ((where = msgbody.rfind("--- ")) != string::npos)
        {
            string::size_type howfar = msgbody.find('\n', where);
            string::size_type nextcr = msgbody.find('\r', where);
            if (string::npos == howfar ||
                (nextcr != string::npos && nextcr < howfar))
                howfar = nextcr;
            if (string::npos == howfar) howfar = msgbody.length();
            program = msgbody.substr(where + 4, howfar - where - 4);
        }

        // Split program name and version
        // It is believed to be divided as such:
        // "Program<space>Version<space>Other"
        // the last part is not counted

        space1 = program.find(' ');
        space2 = program.find(' ', space1 + 1);
        if (string::npos == space1) space1 = space2 = program.length();
        if (string::npos == space2) space2 = program.length();

        if (space1 && '+' == program[space1 - 1])
            in_programname = program.substr(0, space1 - 1);
        else
            in_programname = program.substr(0, space1);
    }

    if (!in_programname.empty())
    {
        wstring programname = decoder_p->Decode(in_programname);
        programdata_s *progtrav_p = programs_p, *progfound_p = progtrav_p;
        if (NULL == programs_p)
        {
            programs_p = new programdata_s;
            if (!programs_p)
            {
                TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory,
                                                       2);
            }
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
                if (!progfound_p)
                {
                    TDisplay::GetOutputObject()->
                        ErrorQuit(TDisplay::out_of_memory, 2);
                }
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
            wstring programvers;

            if (space2 && '+' == program[space2 - 1] && space2 - space1 > 2)
                programvers =
                    decoder_p->Decode(program.substr(space1 + 1,
                                                     space2 - space1 - 2));
            else
                programvers =
                    decoder_p->Decode(program.substr(space1 + 1,
                                                     space2 - space1 - 1));

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
                if (!*vertrav_pp)
                {
                    TDisplay::GetOutputObject()->
                        ErrorQuit(TDisplay::out_of_memory, 2);
                }
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

        // Convert to a struct tm so we can find out weekday and daytime.
        // Since we have a "local" time_t, we use gmtime and just ignore
        // time zone info.
        struct tm *tm_p = gmtime(&timewritten);
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

    delete decoder_p;
}

void StatEngine::AreaDone(void)
{
    numareas ++;
}

string StatEngine::ParseAddress(string controldata, string msgbody) const
{
    // Locate Origin
    // Handles these formats:
    // (address)
    // (network address)
    // (address@network)
    // (network#address)
    string::size_type index = msgbody.rfind(" * Origin: ");
    if (string::npos != index)
    {
        // Locate last '(' and ')' parenthesis on line
        string::size_type endsat = msgbody.find('\n', index);
        string::size_type endsat2 = msgbody.find('\r', index);
        if (endsat2 >= 0 && (endsat2 < endsat || string::npos == endsat))
            endsat = endsat2;
        if (string::npos == endsat) endsat = msgbody.length();
        string::size_type leftparen =
            msgbody.find('(', index), prevleftparen = leftparen;
        while (leftparen != string::npos && leftparen < endsat)
        {
            prevleftparen = leftparen;
            leftparen = msgbody.find('(', leftparen + 1);
        }
        leftparen = prevleftparen; // for clarity
        string::size_type rightparen = msgbody.find(')', leftparen);
        if (string::npos != rightparen && rightparen < endsat)
        {
            // Okay, we've found them, get the address
            string::size_type space  = msgbody.find(' ', leftparen + 1);
            string::size_type atsign = msgbody.find('@', leftparen + 1);
            string::size_type number = msgbody.find('#', leftparen + 1);

            if (space  != string::npos && space  < rightparen)
                leftparen  = space;
            if (atsign != string::npos && atsign < rightparen)
                rightparen = atsign;
            if (number != string::npos && number < rightparen)
                leftparen  = number;

            return msgbody.substr(leftparen + 1, rightparen - leftparen - 1);
        }
    }

    // Locate MSGID
    // Handles these formats:
    // MSGID network#address serial
    // MSGID address@network serial
    // MSGID address serial
    index = controldata.find("MSGID: ");
    if (string::npos != index)
    {
        string::size_type space = controldata.find(' ', index + 1);
        if (string::npos != space)
        {
            string::size_type ends   = controldata.find(' ', space + 1);
            string::size_type atsign = controldata.find('@', space + 1);
            string::size_type number = controldata.find('#', space + 1);

            if (atsign != string::npos && atsign < ends) ends = atsign;
            if (number != string::npos && number < ends) space = number;
            if (string::npos != space)
            {
                return controldata.substr(space + 1, ends - index - 7);
            }
        }
    }

    // Give up
    return string("N/A");
}

wstring StatEngine::DeQP(const string &qp, Decoder *maindecoder_p) const
{
#ifdef HAVE_WORKING_WSTRING
    wstring rc;
#else
    wstring rc(qp.length() + 1);
#endif
    string hex;
    int qpchar;
    string::size_type pos, endpos, current = 0;

    pos = qp.find("=?");
    while (pos != string::npos)
    {
        unsigned qpos = qp.find('?', pos + 3);

        if (string::npos != qpos && '?' == qp[qpos + 2])
        {
            char quotetype = qp[qpos + 1];

            // Copy data up to start of QP/Base64 string
            rc.append(maindecoder_p->Decode(qp.substr(current, pos - current)));

            // Locate end of QP/Base64 string
            endpos = qp.find("?=", qpos + 3);
            if (endpos != string::npos)
            {
                current = endpos + 2;
            } 
            else 
            {
                // Ignore bogus QP
                current = pos;
                break;
            }

            string tmpstr;
            if ('b' == quotetype || 'B' == quotetype)
            {
                // Convert Base64 code to bytestream
                const char base64[] =
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" // 0-25
                    "abcdefghijklmnopqrstuvwxyz" // 26-51
                    "0123456789+/=";             // 52-63 + pad
                int b64buf = 0, b64count = 0, c;
                for (unsigned i = qpos + 3; i < endpos; i ++)
                {
                    char *p = strchr(base64, qp[i]);
                    int b64val = p ? (p - base64) : 0;
                    if (64 == b64val) b64val = 0;
                    b64buf = (b64buf << 6) | b64val;
                    switch (++ b64count)
                    {
                        case 2:
                            c = (b64buf & 0xff0) >> 4;
                            if (c) tmpstr += char(c);
                            b64buf &= 0x0f;
                            break;

                        case 3:
                            c = (b64buf & 0x3fc) >> 2;
                            if (c) tmpstr += char(c);
                            b64buf &= 0x03;
                            break;

                        case 4:
                            if (b64buf) tmpstr += char(b64buf);
                            b64buf = 0;
                            b64count = 0;
                            break;
                    }
                }
            }
            else
            {            
                // Convert QP code to bytestream
                for (unsigned i = qpos + 3; i < endpos; i ++)
                {
                    if ('=' == qp[i])
                    {
                        hex = qp.substr(i + 1, 2);
                        sscanf(hex.c_str(), "%x", &qpchar);
                        tmpstr += char(qpchar);
                        i += 2;
                    }
                    else if ('_' == qp[i])
                        tmpstr += ' ';
                    else
                        tmpstr += qp[i];
                }
            }

            // Convert bytestream to string
            string charset = qp.substr(pos + 2, qpos - pos - 2);
            Decoder *qp_decoder_p = Decoder::GetDecoderByName(charset.c_str());
            rc.append(qp_decoder_p->Decode(tmpstr));
            delete qp_decoder_p;

            // Continue searching
            pos = current;
            pos = qp.find("=?", pos);
        }
        else
        {
            pos = string::npos;
            current = 0;
        }
    }

    rc.append(maindecoder_p->Decode(qp.substr(current)));

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
        if (!persontoplist_p)
        {
            TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory, 2);
        }
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

void StatEngine::DoneTopPeople(void)
{
    if (NULL != persontoplist_p) delete[] persontoplist_p;
    persontoplist_p = NULL;
}

bool StatEngine::GetTopSubjects(bool restart, subjstat_s &result)
{
    if (restart || NULL == subjecttoplist_p)
    {
        // Generate a top list
        // 1. Flatten the binary tree into the array
        if (NULL != subjecttoplist_p) delete[] subjecttoplist_p;

        subjecttoplist_p = new subjstat_s[numsubjects];
        if (!subjecttoplist_p)
        {
            TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory, 2);
        }
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

void StatEngine::DoneTopSubjects(void)
{
    if (NULL != subjecttoplist_p) delete[] subjecttoplist_p;
    subjecttoplist_p = NULL;
}

bool StatEngine::GetTopPrograms(bool restart, progstat_s &result)
{
    if (restart || NULL == programtoplist_p)
    {
        // Generate a top list
        // 1. Flatten the binary tree into the array
        if (NULL != programtoplist_p) delete[] programtoplist_p;

        programtoplist_p = new progstat_s[numprograms];
        if (!programtoplist_p)
        {
            TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory, 2);
        }
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

void StatEngine::DoneTopPrograms(void)
{
    if (NULL != programtoplist_p) delete[] programtoplist_p;
    programtoplist_p = NULL;
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
        if (!nettoplist_p)
        {
            TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory, 2);
        }
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

void StatEngine::DoneTopNets(void)
{
    if (NULL != nettoplist_p) delete[] nettoplist_p;
    nettoplist_p = NULL;
}

bool StatEngine::GetTopDomains(bool restart, domainstat_s &result)
{
    if (restart || NULL == domaintoplist_p)
    {
        // Generate a top list
        // 1. Flatten the binary tree into the array
        if (NULL != domaintoplist_p) delete[] domaintoplist_p;

        domaintoplist_p = new domainstat_s[numdomains];
        if (!domaintoplist_p)
        {
            TDisplay::GetOutputObject()->ErrorQuit(TDisplay::out_of_memory, 2);
        }
        flattenindex = 0;
        FlattenDomains(domaintoplist_p, domains_p);

        // 2. Sort the list according to number of occurances
        qsort(domaintoplist_p, numdomains, sizeof(domainstat_s),
              comparedomains);
        currdomain = 0;
    }
    else
        currdomain ++;

    if (currdomain >= numdomains)
    {
        delete[] domaintoplist_p;
        domaintoplist_p = NULL;
        return false;
    }

    result = domaintoplist_p[currdomain];

    return true;
}

void StatEngine::FlattenDomains(domainstat_s *array, domaindata_s *p)
{
    if (!p) return;

    array[flattenindex].topdomain = p->topdomain;
    array[flattenindex].messages  = p->count;
    array[flattenindex].bytes     = p->bytes;

    flattenindex ++;

    FlattenDomains(array, p->left);
    FlattenDomains(array, p->right);
}

void StatEngine::DoneTopDomains(void)
{
    if (NULL != domaintoplist_p) delete[] domaintoplist_p;
    domaintoplist_p = NULL;
}

// Comparison functions for QSort

int StatEngine::comparenumwritten(const void *p1, const void *p2)
{
    unsigned d1 = (((StatEngine::persstat_s *) p2)->messageswritten);
    unsigned d2 = (((StatEngine::persstat_s *) p1)->messageswritten);

    if (d1 == d2)
        return ((int) (((StatEngine::persstat_s *) p2)->byteswritten)) -
               ((int) (((StatEngine::persstat_s *) p1)->byteswritten));

    if (d1 < d2) return -1;
    return 1;
}

int StatEngine::comparenumreceived(const void *p1, const void *p2)
{
    unsigned d1 = (((StatEngine::persstat_s *) p2)->messagesreceived);
    unsigned d2 = (((StatEngine::persstat_s *) p1)->messagesreceived);

    if (d1 == d2)
        return ((int) (((StatEngine::persstat_s *) p1)->messageswritten)) -
               ((int) (((StatEngine::persstat_s *) p2)->messageswritten));

    if (d1 < d2) return -1;
    return 1;
}

int StatEngine::comparenumquoted(const void *p1, const void *p2)
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

int StatEngine::comparebytepermsg(const void *p1, const void *p2)
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

int StatEngine::compareoriginalpermsg(const void *p1, const void *p2)
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

int StatEngine::comparesubject(const void *p1, const void *p2)
{
    unsigned d1 = (((StatEngine::subjstat_s *) p2)->count);
    unsigned d2 = (((StatEngine::subjstat_s *) p1)->count);
    if (d1 == d2)
        return ((int) (((StatEngine::subjstat_s *) p2)->bytes)) -
               ((int) (((StatEngine::subjstat_s *) p1)->bytes));
    if (d1 < d2) return -1;
    return 1;
}

int StatEngine::compareprogram(const void *p1, const void *p2)
{
    return ((int) (((StatEngine::progstat_s *) p2)->count)) -
           ((int) (((StatEngine::progstat_s *) p1)->count));
}

int StatEngine::comparenets(const void *p1, const void *p2)
{
    return ((int) (((StatEngine::netstat_s *) p2)->messages)) -
           ((int) (((StatEngine::netstat_s *) p1)->messages));
}

int StatEngine::comparedomains(const void *p1, const void *p2)
{
    return ((int) (((StatEngine::domainstat_s *) p2)->messages)) -
           ((int) (((StatEngine::domainstat_s *) p1)->messages));
}
