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

#ifndef __STATENGINE_H
#define __STATENGINE_H

#include <string>
#include <time.h>

int comparenumwritten(const void *, const void *);
int comparenumreceived(const void *, const void *);
int comparenumquoted(const void *, const void *);
int comparebytepermsg(const void *, const void *);
int compareoriginalpermsg(const void *, const void *);
int comparesubject(const void *, const void *);
int compareprogram(const void *, const void *);

class StatEngine
{
public:
    // Constructor and destructor
    StatEngine(void);
    ~StatEngine(void);

    // Methods to report data to the statistics engine
    void AddData(string fromname, string toname, string subject,
                 string controldata, string msgbody,
                 time_t timewritten, time_t timereceived);
    void AreaDone(void);

    // Structures and methods used to retrieve data from the
    // statistics engine
    struct persstat_s
    {
        string      name;
        string      address;
        unsigned    byteswritten, lineswritten, bytesquoted, linesquoted,
                    messageswritten, messagesreceived;
    };

    enum list_t { None, TopReceive, TopWrite, TopQuote, TopBpm, TopOpm };

    inline bool GetTopWriters(bool restart, persstat_s &result)
        { return GetTop(restart, result, comparenumwritten, TopWrite); };
    inline bool GetTopReceivers(bool restart, persstat_s &result)
        { return GetTop(restart, result, comparenumreceived, TopReceive); };
    inline bool GetTopQuoters(bool restart, persstat_s &result)
        { return GetTop(restart, result, comparenumquoted, TopQuote); };
    inline bool GetTopBytePerMsg(bool restart, persstat_s &result)
        { return GetTop(restart, result, comparebytepermsg, TopBpm); };
    inline bool GetTopOriginalPerMsg(bool restart, persstat_s &result)
        { return GetTop(restart, result, compareoriginalpermsg, TopOpm); };

    struct subjstat_s
    {
        string      subject;
        unsigned    count, bytes;
    };

    bool GetTopSubjects(bool restart, subjstat_s &result);

    struct progstat_s
    {
        friend class StatEngine;

        string      program;
        unsigned    count;
    private:
        void        *versions; // should be struct StatEngine::programversion_s
    };

    bool GetTopPrograms(bool restart, progstat_s &result);

    struct verstat_s
    {
        string      version;
        unsigned    count;
    };

    bool GetProgramVersions(bool restart, verstat_s &result);

    inline time_t GetEarliestReceived(void) { return earliestreceived; };
    inline time_t GetLastReceived(void)     { return latestreceived;   };
    inline time_t GetEarliestWritten(void)  { return earliestwritten;  };
    inline time_t GetLastWritten(void)      { return latestwritten;    };

    inline unsigned GetDayMsgs(unsigned day){ return daycount[day];    };
    inline unsigned GetHourMsgs(unsigned h) { return hourcount[h];     };

    inline unsigned GetTotalNumber(void)    { return msgcount;         };
    inline unsigned GetTotalBytes(void)     { return totalbytes;       };
    inline unsigned GetTotalQBytes(void)    { return totalqbytes;      };
    inline unsigned GetTotalLines(void)     { return totallines;       };
    inline unsigned GetTotalQLines(void)    { return totalqlines;      };
    inline unsigned GetTotalPeople(void)    { return numpeople;        };
    inline unsigned GetTotalSubjects(void)  { return numsubjects;      };
    inline unsigned GetTotalPrograms(void)  { return numprograms;      };
    inline unsigned GetTotalAreas(void)     { return numareas;         };

    inline void NoArrivalTime(void)         { hasarrivaltime = false;  };
    inline bool HasArrivalTime(void)        { return hasarrivaltime;   };

    inline void NewsArea(void)              { newsarea = true;         };
    inline bool IsNewsArea(void)            { return newsarea;         };

protected:
    // Data types used by statistics engine
    struct persondata_s
    {
        string      name;
        string      address;
        unsigned    byteswritten, lineswritten, bytesquoted, linesquoted,
                    messageswritten, messagesreceived;

        persondata_s    *left, *right;

        persondata_s(void) { left = right = NULL;
                             byteswritten = lineswritten = bytesquoted =
                             linesquoted = messageswritten = messagesreceived =
                             0; };
        ~persondata_s(void) { if (left) delete left; if (right) delete (right); };
    };

    struct programversion_s
    {
        string      version;
        unsigned    count;

        programversion_s    *next;

        programversion_s(void) { next = NULL; count = 0; };
        ~programversion_s(void) { if (next) delete next; };
    };

    struct programdata_s
    {
        string              programname;
        programversion_s    *versions_p;
        unsigned            count;

        programdata_s   *left, *right;

        programdata_s(void) { left = right = NULL; versions_p = NULL;
                              count = 0; };
        ~programdata_s(void) { if (versions_p) delete versions_p;
                               if (left) delete left;
                               if (right) delete right; };
    };

    struct subjectdata_s
    {
        string      subject;
        unsigned    count, bytes;

        subjectdata_s   *left, *right;

        subjectdata_s(void) { left = right = NULL; count = bytes = 0; };
        ~subjectdata_s(void) { if (left) delete left;
                               if (right) delete right; };
    };

    // Variables used by the statistics engine
    unsigned        msgcount, totallines, totalqlines, totalbytes, totalqbytes;
    unsigned        daycount[7], hourcount[24];
    unsigned        numpeople, numsubjects, numprograms, numareas;
    time_t          earliestwritten, latestwritten;
    time_t          earliestreceived, latestreceived;
    bool            wdatevalid, rdatevalid;
    persondata_s    *people_p;
    programdata_s   *programs_p;
    subjectdata_s   *subjects_p;

    // Variables used by the statistics retreival
    persstat_s          *persontoplist_p;
    subjstat_s          *subjecttoplist_p;
    progstat_s          *programtoplist_p;
    unsigned            currperson, currsubject, currprogram;
    list_t              currpersontype;
    programversion_s    *currversion;
    bool                hasarrivaltime, newsarea;

    // Internal methods
    string ParseAddress(string, string);
    string DeQP(string);
    void FlattenPeople(persstat_s *, persondata_s *);
    void FlattenSubjects(subjstat_s *, subjectdata_s *);
    void FlattenPrograms(progstat_s *, programdata_s *);
    bool GetTop(bool, persstat_s &, int (*)(const void *, const void *), list_t);
};

#endif
