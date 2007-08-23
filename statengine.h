// Copyright (c) 1998-2001 Peter Karlsson
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

#include "utility.h"
#include "convert.h"

#if !defined(HAVE_IMPLICIT_NAMESPACE)
using namespace std;
#endif

/**
 * The class doing the real work. This is the class that collects (or,
 * rather, is fed) the messages from the message base(s) and collects
 * statistics data from them. It also allows extraction of the collected
 * data in the form of various summary data, as well as various toplist.
 */
class StatEngine
{
public:
    /** Standard constructor. */
    StatEngine(void);
    /** Standard destructor. */
    ~StatEngine(void);

    // Methods to report data to the statistics engine
    /**
     * Add data from a message to the engine. If the message meets the set
     * criterions, data is extracted from it and stored.
     *
     * @param fromname     Name of sender of this message. For Usenet areas,
     *                     this can be retrieved from the control data.
     * @param toname       Name of recipient of this message. If no
     *                     recipient is known or intended (Usenet), leave
     *                     blank.
     * @param subject      Subject of the message. For Usenet areas, this
     *                     can be retrieved from the control data.
     * @param controldata  Control data associated with the message. The
     *                     control data should be one long string where the
     *                     fields are separated by 0x01 characters. Fidonet
     *                     kludges and Usenet headers go here.
     * @param msgbody      Message body associated with the message. The
     *                     paragraphs/lines should be separated with either
     *                     CR, LF or CRLF.
     * @param timewritten  Time (in seconds since epoch) when message was
     *                     written. For Usenet areas, this can be retrieved
     *                     from the control data.
     * @param timereceived Time (in seconds since epoch) when message was
     *                     received. If no such information is available,
     *                     duplicate the timewritten data.
     */
    void AddData(string fromname, string toname, string subject,
                 string controldata, string msgbody,
                 time_t timewritten, time_t timereceived);
    /** Indicate that the processing of the area has been concluded. */
    void AreaDone(void);

    // Structures and methods used to retrieve data from the
    // statistics engine
    /**
     * Structure used to receive person data for a toplist. This is the
     * return type for all toplists describing people.
     */
    struct persstat_s
    {
        wstring     name;               ///< Name of person.
        string      address;            ///< Network address of person.
        unsigned    byteswritten,       ///< Bytes written by person.
                    lineswritten,       ///< Lines/paragraphs written by person.
                    bytesquoted,        ///< Bytes quoted by person.
                    linesquoted,        ///< Lines/paragraphs quoted by person.
                    messageswritten,    ///< Messages written by person.
                    messagesreceived;   ///< Messages where person is recipient.
    };

protected:
    /** Toplist types involving people that can be requested. */
    enum list_t { None, TopReceive, TopWrite, TopQuote, TopBpm, TopOpm };

public:
    /**
     * Retrieve toplist of writers.
     * @param restart Flag indicating whether to restart from position 1 or
     *                continue from previous location.
     * @param result  Reference to structure to retrieve toplist data.
     * @return Flag telling if there is any data left for toplist.
     */
    inline bool GetTopWriters(bool restart, persstat_s &result)
        { return GetTop(restart, result, comparenumwritten, TopWrite); };
    /**
     * Retrieve toplist of receivers.
     * @param restart Flag indicating whether to restart from position 1 or
     *                continue from previous location.
     * @param result  Reference to structure to retrieve toplist data.
     * @return Flag telling if there is any data left for toplist.
     */
    inline bool GetTopReceivers(bool restart, persstat_s &result)
        { return GetTop(restart, result, comparenumreceived, TopReceive); };
    /**
     * Retrieve toplist of quoters.
     * @param restart Flag indicating whether to restart from position 1 or
     *                continue from previous location.
     * @param result  Reference to structure to retrieve toplist data.
     * @return Flag telling if there is any data left for toplist.
     */
    inline bool GetTopQuoters(bool restart, persstat_s &result)
        { return GetTop(restart, result, comparenumquoted, TopQuote); };
    /**
     * Retrieve toplist of bytes per message.
     * @param restart Flag indicating whether to restart from position 1 or
     *                continue from previous location.
     * @param result  Reference to structure to retrieve toplist data.
     * @return Flag telling if there is any data left for toplist.
     */
    inline bool GetTopBytePerMsg(bool restart, persstat_s &result)
        { return GetTop(restart, result, comparebytepermsg, TopBpm); };
    /**
     * Retrieve toplist of original content per message.
     * @param restart Flag indicating whether to restart from position 1 or
     *                continue from previous location.
     * @param result  Reference to structure to retrieve toplist data.
     * @return Flag telling if there is any data left for toplist.
     */
    inline bool GetTopOriginalPerMsg(bool restart, persstat_s &result)
        { return GetTop(restart, result, compareoriginalpermsg, TopOpm); };
    /** Indicate that we are done with the people toplist. */
    void DoneTopPeople(void);

    /**
     * Structure used to receive subject data for a toplist. This is the
     * return type for the GetTopSubjects method.
     */
    struct subjstat_s
    {
        wstring     subject;    ///< Subject line.
        unsigned    count,      ///< Number of messages with subject.
                    bytes;      ///< Number of bytes under subject.
    };

    /**
     * Retrieve toplist of used subject lines.
     * @param restart Flag indicating whether to restart from position 1 or
     *                continue from previous location.
     * @param result  Reference to structure to retrieve toplist data.
     * @return Flag telling if there is any data left for toplist.
     */
    bool GetTopSubjects(bool restart, subjstat_s &result);
    /** Indicate that we are done with the subject toplist. */
    void DoneTopSubjects(void);

    /**
     * Structure used to receive program data for a toplist. This is the
     * return type for the GetTopPrograms method.
     */
    struct progstat_s
    {
        friend class StatEngine;

        wstring     program;        ///< Name of program.
        unsigned    count;          ///< Number of uses of program.
    private:
        void        *versions;      ///< Pointer to private data.
            // should be struct StatEngine::programversion_s
    };

    /**
     * Retrieve toplist of used programs.
     * @param restart Flag indicating whether to restart from position 1 or
     *                continue from previous location.
     * @param result  Reference to structure to retrieve toplist data.
     * @return Flag telling if there is any data left for toplist.
     */
    bool GetTopPrograms(bool restart, progstat_s &result);
    /** Indicate that we are done with the project toplist. */
    void DoneTopPrograms(void);

    /**
     * Structure used to receive information of versions used for a program.
     * This is the return type of the GetProgramVersions method.
     */
    struct verstat_s
    {
        wstring     version;    ///< Version of program.
        unsigned    count;      ///< Number of uses of version.
    };

    /**
     * Retrieve uses of each version of the current program. This extends
     * the data record retrieved via the GetTopPrograms method.
     * @param restart Flag indicating whether to restart from position 1 or
     *                continue from previous location.
     * @param result  Reference to structure to retrieve version info.
     * @return Flag telling if there is any data left for toplist.
     */
    bool GetProgramVersions(bool restart, verstat_s &result);

    /**
     * Structure used to receive Fidonet network for a toplist. This is the
     * return type for the GetTopNets method.
     */
    struct netstat_s
    {
        unsigned    zone,       ///< Fidonet zone.
                    net;        ///< Fidonet net in zone.
        unsigned    messages,   ///< Number of messages written from net.
                    bytes;      ///< Number of bytes written from net.
    };

    /**
     * Retrieve toplist of Fidonet networks.
     * @param restart Flag indicating whether to restart from position 1 or
     *                continue from previous location.
     * @param result  Reference to structure to retrieve toplist data.
     * @return Flag telling if there is any data left for toplist.
     */
    bool GetTopNets(bool restart, netstat_s &result);
    /** Indicate that we are done with the Fidonet networks toplist. */
    void DoneTopNets();

    /**
     * Structure used to receive Internet topdomains for a toplist. This is the
     * return type for the GetTopNets method.
     */
    struct domainstat_s
    {
        string      topdomain;  ///< Internet topdomain.
        unsigned    messages,   ///< Number of messages written from domain.
                    bytes;      ///< Number of bytes written from domain.
    };

    /**
     * Retrieve toplist of Internet topdomains.
     * @param restart Flag indicating whether to restart from position 1 or
     *                continue from previous location.
     * @param result  Reference to structure to retrieve toplist data.
     * @return Flag telling if there is any data left for toplist.
     */
    bool GetTopDomains(bool restart, domainstat_s &result);
    /** Indicate that we are done with the Internet topdomains toplist. */
    void DoneTopDomains();

    /** Get earliest arrival time recorded. */
    inline time_t GetEarliestReceived(void) { return earliestreceived; };
    /** Get latest arrival time recorded. */
    inline time_t GetLastReceived(void)     { return latestreceived;   };
    /** Get earliest written time recorded. */
    inline time_t GetEarliestWritten(void)  { return earliestwritten;  };
    /** Get latest written time recorded. */
    inline time_t GetLastWritten(void)      { return latestwritten;    };

    /**
     * Get number of messages written on a specific day.
     * @param day Day of week to get data for, 0 = Sunday.
     * @return Number of messages written on the specified day.
     */
    inline unsigned GetDayMsgs(unsigned day){ return daycount[day];    };
    /**
     * Get number of messages written in a specific hour.
     * @param h Hour of day to get data for (0-23).
     * @return Number of messages written in the specified hour.
     */
    inline unsigned GetHourMsgs(unsigned h) { return hourcount[h];     };

    /** Get total number of messages recorded. */
    inline unsigned GetTotalNumber(void)    { return msgcount;         };
    /** Get total number of bytes recorded. */
    inline unsigned GetTotalBytes(void)     { return totalbytes;       };
    /** Get total number of quoted bytes recorded. */
    inline unsigned GetTotalQBytes(void)    { return totalqbytes;      };
    /** Get total number of lines recorded. */
    inline unsigned GetTotalLines(void)     { return totallines;       };
    /** Get total number of quoted lines recorded. */
    inline unsigned GetTotalQLines(void)    { return totalqlines;      };
    /** Get total number of people recorded. */
    inline unsigned GetTotalPeople(void)    { return numpeople;        };
    /** Get total number of subjects recorded. */
    inline unsigned GetTotalSubjects(void)  { return numsubjects;      };
    /** Get total number of programs recorded. */
    inline unsigned GetTotalPrograms(void)  { return numprograms;      };
    /** Get total number of areas recorded. */
    inline unsigned GetTotalAreas(void)     { return numareas;         };
    /** Get total number of Fidonet networks recorded. */
    inline unsigned GetTotalNets(void)      { return numnets;          };
    /** Get total number of Internet topdomains recorded. */
    inline unsigned GetTotalDomains(void)   { return numdomains;       };

    /** Tell engine that we do not have access to arrival times. */
    inline void NoArrivalTime(void)         { hasarrivaltime = false;  };
    /** Check whether we have recorded arrival times. */
    inline bool HasArrivalTime(void)        { return hasarrivaltime;   };

    /** Tell engine that we are adding data from a news area. */
    inline void NewsArea(void)              { newsarea = true;         };
    /** Check whether we have data for a news area. */
    inline bool IsNewsArea(void)            { return newsarea;         };

protected:
    // Data types used by statistics engine
    /**
     * Information about a person. This structure contains all the
     * information known about the people that are involved with the
     * messages fed to the engine. The data is organized as a binary tree.
     */
    struct persondata_s
    {
        wstring     name;               ///< Name of person.
        string      address;            ///< Network address of person.
        unsigned    byteswritten,       ///< Bytes written by person.
                    lineswritten,       ///< Lines written by person.
                    bytesquoted,        ///< Bytes quoted by person.
                    linesquoted,        ///< Lines quoted by person.
                    messageswritten,    ///< Messages authored by person.
                    messagesreceived;   ///< Messages adressed to person. 

        persondata_s    *left,  ///< Left leaf node. (Lower sorted name).
                        *right; ///< Right leaf node. (Higher sorted name).

        /** Constructor that initializes the structure. */
        persondata_s(void) { left = right = NULL;
                             byteswritten = lineswritten = bytesquoted =
                             linesquoted = messageswritten = messagesreceived =
                             0; };
        /** Destructor that destroys the entire tree rooted here. */
        ~persondata_s(void) { if (left) delete left; if (right) delete (right); };
    };

    /**
     * Information about a program version. This structure contains all the
     * information about the versions that are associated with a piece of
     * software. The data is organized as a linked list, sorted by version
     * number.
     */
    struct programversion_s
    {
        wstring     version;        ///< Name of version.
        unsigned    count;          ///< Number of uses of this version.

        programversion_s    *next;  ///< Next version in linked list.

        /** Constructor that initializes the structure. */
        programversion_s(void) { next = NULL; count = 0; };
        /** Destructor that destroys the entire list rooted here. */
        ~programversion_s(void) { if (next) delete next; };
    };

    /**
     * Information about a piece of software. This structure contains all
     * the information about the programs that has been used in the messages
     * fed to the engine. The data is organized as a binary tree.
     */
    struct programdata_s
    {
        wstring             programname;    ///< Name of program.
        programversion_s    *versions_p;    ///< Pointer to software versions.
        unsigned            count;          ///< Number of uses of this program.

        programdata_s   *left,  ///< Left leaf node. (Lower sorted name).
                        *right; ///< Right leaf node. (Higher sorted name).

        /** Constructor that initializes the structure. */
        programdata_s(void) { left = right = NULL; versions_p = NULL;
                              count = 0; };
        /** Destructor that destroys the entire tree rooted here. */
        ~programdata_s(void) { if (versions_p) delete versions_p;
                               if (left) delete left;
                               if (right) delete right; };
    };

    /**
     * Information about a subject. This structure contains all the
     * information about the subjects that has been used in the messages fed
     * to the engine. The data is organized as a binary tree.
     */
    struct subjectdata_s
    {
        wstring     subject;    ///< Subject line.
        unsigned    count,      ///< Number of messages bearing this subject.
                    bytes;      ///< Number of bytes bearing this subject.

        subjectdata_s   *left,  ///< Left leaf node. (Lower sorted subject).
                        *right; ///< Right leaf node. (Higher sorted subject).

        /** Constructor that initializes the structure. */
        subjectdata_s(void) { left = right = NULL; count = bytes = 0; };
        /** Destructor that destroys the entire tree rooted here. */
        ~subjectdata_s(void) { if (left) delete left;
                               if (right) delete right; };
    };

    /**
     * Information about a Fidonet network. This structure contains all the
     * information about the Fidonet networks in the messages fed to the
     * engine. The data is organized as a binary tree.
     */
    struct netdata_s
    {
        unsigned    zone,   ///< Fidonet zone.
                    net;    ///< Fidonet net in zone.
        unsigned    count,  ///< Number of messages written from net.
                    bytes;  ///< Number of bytes written from net.

        netdata_s   *left,  ///< Left leaf node. (Numerically lower net).
                    *right; ///< Right leaf node. (Numerically higher net).

        /** Constructor that initializes the structure. */
        netdata_s(void) { left = right = NULL; count = bytes = 0; };
        /** Destructor that destroys the entire tree rooted here. */
        ~netdata_s(void) { if (left) delete left;
                           if (right) delete right; };
    };

    /**
     * Information about an Internet topdomain. This structure contains all
     * the information about the Internet topdomains in the messages fed to
     * the engine. The data is organized as a binary tree.
     */
    struct domaindata_s
    {
        string      topdomain;  ///< Internet topdomain.
        unsigned    count,      ///< Number of messages written from domain.
                    bytes;      ///< Number of bytes written from domain.

        domaindata_s    *left,  ///< Left leaf node. (Lower topdomain).
                        *right; ///< Right leaf node. (Higher topdomain).

        /** Constructor that initializes the structure. */
        domaindata_s(void) { left = right = NULL; count = bytes = 0; };
        /** Destructor that destroys the entire tree rooted here. */
        ~domaindata_s(void) { if (left) delete left;
                              if (right) delete right; };
    };

    // Variables used by the statistics engine
    unsigned        msgcount, totallines, totalqlines, totalbytes, totalqbytes;
    unsigned        daycount[7], hourcount[24];
    unsigned        numpeople, numsubjects, numprograms, numareas, numnets,
                    numdomains;
    time_t          earliestwritten, latestwritten;
    time_t          earliestreceived, latestreceived;
    bool            wdatevalid, rdatevalid;
    persondata_s    *people_p;
    programdata_s   *programs_p;
    subjectdata_s   *subjects_p;
    netdata_s       *nets_p;
    domaindata_s    *domains_p;

    // Variables used by the statistics retreival
    persstat_s          *persontoplist_p;
    subjstat_s          *subjecttoplist_p;
    progstat_s          *programtoplist_p;
    netstat_s           *nettoplist_p;
    domainstat_s        *domaintoplist_p;
    unsigned            currperson, currsubject, currprogram, currnet,
                        currdomain;
    list_t              currpersontype;
    programversion_s    *currversion;
    bool                hasarrivaltime, newsarea;

    // Internal methods
    /**
     * Locate a Fidonet address in a message.
     * @param controldata Control data of message.
     * @param msgbody     Body text of message.
     * @return Fidonet address, or "N/A" if none was found.
     */
    string ParseAddress(string, string) const;
    /** Decode a Quoted-Printable encoded string. */
    wstring DeQP(const string &, Decoder *) const;
    /** Internal helper for the people toplists. */
    void FlattenPeople(persstat_s *, persondata_s *);
    /** Internal helper for the subjects toplist. */
    void FlattenSubjects(subjstat_s *, subjectdata_s *);
    /** Internal helper for the program toplist. */
    void FlattenPrograms(progstat_s *, programdata_s *);
    /** Internal helper for the Fidonet networks toplist. */
    void FlattenNets(netstat_s *, netdata_s *);
    /** Internal helper for the Internet topdomain toplist. */
    void FlattenDomains(domainstat_s *, domaindata_s *);
    /** Internal helper for the people toplists. */
    bool GetTop(bool, persstat_s &, int (*)(const void *, const void *), list_t);

private:
    // State variables
    unsigned int flattenindex;

    // Comparison functions for QSort
    static int comparenumwritten(const void *, const void *);
    static int comparenumreceived(const void *, const void *);
    static int comparenumquoted(const void *, const void *);
    static int comparebytepermsg(const void *, const void *);
    static int compareoriginalpermsg(const void *, const void *);
    static int comparesubject(const void *, const void *);
    static int compareprogram(const void *, const void *);
    static int comparenets(const void *, const void *);
    static int comparedomains(const void *, const void *);
};

#endif
