// Copyright (c) 1998-1999 Peter Karlsson
//
// $Id$
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
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

#include "statengine.h"
#include "utility.h"

StatEngine::StatEngine(void)
{
    // Initialize count variables
    msgcount = 0;
    for (int i = 0; i < 7; i ++) daycount[i] = 0;
    for (int i = 0; i < 24; i ++) hourcount[i] = 0;
    numpeople = numsubjects = numprograms = 0;
    totallines = totalqlines = totalbytes = totalqbytes = 0;
    people_p = NULL;
    programs_p = NULL;
    subjects_p = NULL;
    persontoplist_p = NULL;
    subjecttoplist_p = NULL;
    programtoplist_p = NULL;
    currversion = NULL;
    wdatevalid = false;
    rdatevalid = false;
    earliestwritten = latestwritten = earliestreceived = latestreceived = 0;
    currpersontype = None;
}

StatEngine::~StatEngine(void)
{
    // Deallocate any leftover data structures
    if (people_p) delete people_p;
    if (programs_p) delete programs_p;
    if (subjects_p) delete subjects_p;

    if (persontoplist_p) delete[] persontoplist_p;
    if (subjecttoplist_p) delete[] subjecttoplist_p;
    if (programtoplist_p) delete[] programtoplist_p;
}

void StatEngine::AddData(string fromname, string toname, string subject,
                         string controldata, string msgbody,
                         time_t timewritten, time_t timereceived)
{
    enum { None, Left, Right } direction;

    // Locate sender's name in database, and update statistics
    persondata_s *perstrav_p = people_p, *persfound_p = perstrav_p;

    if (NULL == perstrav_p)
    {
        people_p = new persondata_s;
        people_p->name = fromname;
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
            if (Left == direction) perstrav_p->left = persfound_p;
            if (Right == direction) perstrav_p->right = persfound_p;

            persfound_p->name = fromname;
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
        if (nextcr2 < nextcr || -1 == nextcr) nextcr = nextcr2;
        if (-1 == nextcr) nextcr = msgbody.length();
        string thisline = msgbody.substr((int) currindex, nextcr - currindex);

        if ("--- " == thisline.substr(0, 4) ||
            thisline.length() == 3 && "---" == thisline)
        {
            foundtear = true;
        }
        else if (" * Origin: " == thisline.substr(0, 11))
        {
            break;
        }
        else
            foundtear = false;

        int gt = thisline.find('>'), lt = thisline.find('<');
        if (gt >= 0 && gt < 6 && (-1 == lt || gt < lt))
        {
            isquoted = true;
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

    // Locate recipient's name in database, and update statistics
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
        if (Left == direction) perstrav_p->left = persfound_p;
        if (Right == direction) perstrav_p->right = persfound_p;

        persfound_p->name = toname;
        persfound_p->address = "N/A";

        numpeople ++;
    }

    persfound_p->messagesreceived ++;

    if (subject.length() > 3)
    {
        if (fcompare(subject.substr(0, 3), "re:") == 0)
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
    // PID takes precedence over tearline
    int where;
    if ((where = controldata.find("PID: ")) != -1)
    {
        int howfar = controldata.find((char) 1, where);
        if (-1 == howfar) howfar = controldata.length();
        program = controldata.substr(where + 5, howfar - where - 5);
    }
    else if ((where = msgbody.find("--- ")) != -1)
    {
        int howfar = msgbody.find('\n', where);
        if (-1 == howfar) howfar = msgbody.find('\r', where);
        if (-1 == howfar) howfar = msgbody.length();
        program = msgbody.substr(where + 4, howfar - where - 4);
    }

    // Split program name and version
    // It is believed to be divided as such: "Program<space>Version<space>Other"
    // the last part is not counted

    int space1 = program.find(' ');
    int space2 = program.find(' ', space1 + 1);
    if (-1 == space1) space1 = space2 = program.length();
    if (-1 == space2) space2 = program.length();

    string programname;
    if (space1 && '+' == program[space1 - 1])
        programname = program.substr(0, space1 - 1);
    else
        programname = program.substr(0, space1);

    if (programname != "")
    {
        programdata_s *progtrav_p = programs_p, *progfound_p = progtrav_p;
        if (NULL == programs_p)
        {
            programs_p = new programdata_s;
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
                   fcompare((*vertrav_pp)->version, programvers) != 0)
            {
                vertrav_pp = &((*vertrav_pp)->next);
            }

            if (NULL == *vertrav_pp)
            {
                *vertrav_pp = new programversion_s;
                (*vertrav_pp)->version = programvers;
            }
            (*vertrav_pp)->count ++;
        }
    }

    // Check writing and receiption date and add to statistics
    if (timewritten != 0) // 0 date indicates error
    {
        if (!wdatevalid || timewritten  < earliestwritten)
            earliestwritten  = timewritten;
        if (!wdatevalid || timewritten  > latestwritten)
            latestwritten    = timewritten;
        wdatevalid = true;
    }
    if (timereceived != 0)
    {
        if (!rdatevalid || timereceived < earliestreceived)
            earliestreceived = timereceived;
        if (!rdatevalid || timereceived > latestreceived)
            latestreceived   = timereceived;
        rdatevalid = true;
    }

    struct tm *tm_p = localtime(&timewritten);
    daycount[tm_p->tm_wday] ++;
    hourcount[tm_p->tm_hour] ++;
}

string StatEngine::ParseAddress(string controldata, string msgbody)
{
    // Locate Origin
    int index = msgbody.find(" * Origin: ");
    if (-1 != index)
    {
        // Locate last '(' and ')' parenthesis on line
        int endsat = msgbody.find('\n', index);
        int endsat2 = msgbody.find('\r', index);
        if (endsat2 < endsat || -1 == endsat) endsat = endsat2;
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
            return msgbody.substr(leftparen + 1, rightparen - leftparen - 1);
        }
    }

    // Locate MSGID
    // Handles these formats:
    // MSGID Network#address serial
    // MSGID address@network serial
    // MSGID address serial
    index = controldata.find("MSGID: ");
    if (-1 != index)
    {
        int index2 = controldata.find(' ', index + 1);
        if (-1 != index2)
        {
            int index3 = controldata.find(' ', index2 + 1);
            int index4 = controldata.find('@', index2 + 1);
            int index5 = controldata.find('#', index2 + 1);
            if (index4 != -1 && index4 < index3) index3 = index4;
            if (index5 != -1 && index5 < index3) index2 = index5;
            if (-1 != index3)
            {
                return controldata.substr(index2 + 1, index3 - index - 7);
            }
        }
    }

    // Give up
    return string("N/A");
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
        return false;

    result = persontoplist_p[currperson];

    return true;
}

void StatEngine::FlattenPeople(persstat_s *array, persondata_s *p)
{
    static unsigned index;

    if (!p) return;
    if (p == people_p) index = 0;

    array[index].name             = p->name;
    array[index].address          = p->address;
    array[index].byteswritten     = p->byteswritten;
    array[index].lineswritten     = p->lineswritten;
    array[index].bytesquoted      = p->bytesquoted;
    array[index].linesquoted      = p->linesquoted;
    array[index].messageswritten  = p->messageswritten;
    array[index].messagesreceived = p->messagesreceived;

    index ++;

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
        FlattenSubjects(subjecttoplist_p, subjects_p);

        // 2. Sort the list according to number of written texts
        qsort(subjecttoplist_p, numsubjects, sizeof(subjstat_s),
              comparesubject);
        currsubject = 0;
    }
    else
        currsubject ++;

    if (currsubject >= numsubjects)
        return false;

    result = subjecttoplist_p[currsubject];

    return true;
}

void StatEngine::FlattenSubjects(subjstat_s *array, subjectdata_s *p)
{
    static unsigned index;

    if (!p) return;
    if (p == subjects_p) index = 0;

    array[index].subject = p->subject;
    array[index].bytes   = p->bytes;
    array[index].count   = p->count;

    index ++;

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
        FlattenPrograms(programtoplist_p, programs_p);

        // 2. Sort the list according to number of written texts
        qsort(programtoplist_p, numprograms, sizeof(progstat_s),
              compareprogram);
        currprogram = 0;
    }
    else
        currprogram ++;

    if (currprogram >= numprograms)
        return false;

    result = programtoplist_p[currprogram];

    return true;
}

void StatEngine::FlattenPrograms(progstat_s *array, programdata_s *p)
{
    static unsigned index;

    if (!p) return;
    if (p == programs_p) index = 0;

    array[index].program  = p->programname;
    array[index].versions = p->versions_p;
    array[index].count    = p->count;

    index ++;

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
