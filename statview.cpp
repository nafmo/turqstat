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

#include <config.h>
#include <fstream.h>
#include <limits.h>
#include <time.h>

#include "statview.h"
#include "statengine.h"
#include "version.h"

static const char *days[] =
    { "Monday   ",
      "Tuesday  ",
      "Wednesday",
      "Thursday ",
      "Friday   ",
      "Saturday ",
      "Sunday   " };

StatView::StatView()
{
    // Initialize all values
    quoters = false;
    topwritten = false;
    topreceived = false;
    toporiginal = false;
    topnets = false;
    topsubjects = false;
    topprograms = false;
    topdomains = false;
    weekstats = false;
    daystats = false;

    showversions = false;
    showallnums = false;
    maxnumber = 15;
}

bool StatView::CreateReport(StatEngine *engine, string filename)
{
    // If we give 0 as maximum entries, we want unlimited (=UINT_MAX...)
    if (0 == maxnumber) maxnumber = UINT_MAX;

    // Select date format
    const char *dateformat;
#ifdef HAVE_LOCALE_H
    if (uselocale) dateformat = "%x %X";
    else
#endif
    dateformat = "%Y-%m-%d %H:%M:%S";

    // Create a report file
    fstream report(filename.c_str(), ios::out);
    if (!(report.is_open())) return false;

    // Include data as given from the statistics engine
    bool news = engine->IsNewsArea();

    report << "Turquoise SuperStat " << version << " * Message area statistics" << endl;
    report << "=================================================" << endl;
    report << endl;
    report << copyright << endl;
    report << endl;

    if (0 == engine->GetTotalNumber())
    {
        report << "Message area is empty." << endl;
        report.close();
    }

    report << "This report covers " << engine->GetTotalNumber()
           <<" messages ";

    if (engine->GetTotalAreas() > 1)
    {
        report << "in " << engine->GetTotalAreas();
        if (news)
            report << " groups, ";
        else
            report << " areas, ";
    }

    if (engine->HasArrivalTime())
    {
        report << "that were received at this system ";
    }
    else
    {
        report << "written ";
    }

    time_t earliest = engine->GetEarliestReceived();
    struct tm *p1 = localtime(&earliest);
    char date[64];
    strftime(date, 64, dateformat, p1);

    report << "between " << date << " and ";

    time_t latest = engine->GetLastReceived();
    struct tm *p2 = localtime(&latest);
    strftime(date, 64, dateformat, p2);

    report << date;

    if (engine->HasArrivalTime())
    {
        report << " (written between ";

        time_t wearliest = engine->GetEarliestWritten();
        struct tm *p3 = localtime(&wearliest);
        strftime(date, 64, dateformat, p3);

        report << date << " and ";

        time_t wlatest = engine->GetLastWritten();
        struct tm *p4 = localtime(&wlatest);
        strftime(date, 64, dateformat, p4);

        report << date << ")";
    }

    report << endl;
    report << endl;

    string tmp;

    if (quoters)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Blacklist of quoters (of people who have written at least "
                  "three messages)" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::persstat_s data;
        unsigned oldpercent = 10100;
        while (place <= maxnumber && engine->GetTopQuoters(restart, data))
        {
            if (restart)
            {
                restart = false;
                report << "Place Name                                "
                          "                       Msgs   Ratio" << endl;
            }

            if (data.messageswritten > 2 && data.bytesquoted > 0)
            {
                unsigned percentquotes =
                    data.byteswritten ?
                      ((100000 * (unsigned long long) data.bytesquoted) /
                       (unsigned long long) data.byteswritten)
                      : 0;
                if (!showallnums && percentquotes == oldpercent)
                {
                    report << "      ";
                }
                else
                {
                    report.form("%4u. ", place);
                }

                unsigned integ = percentquotes / 1000;
                unsigned fract = (percentquotes - integ * 1000 + 5) / 10;
                if (100 == fract)
                {
                    fract = 0;
                    integ ++;
                }


                if (data.name != data.address)
                    tmp = string(data.name + " <") +
                          string(data.address + ">");
                else
                    tmp = data.name;

                report.form("%-58.58s%5u %3u.%02u%%",
                            tmp.c_str(), data.messageswritten,
                            integ, fract);

                report << endl;

                place ++;
                oldpercent = percentquotes;
            }
        }

        report << endl;

        unsigned long long totalbytes = engine->GetTotalBytes();
        if (totalbytes)
        {
            unsigned long long totalqbytes = engine->GetTotalQBytes();

            report << "A total of " << totalbytes << " bytes were written "
                      "(message bodies only), of which " << totalqbytes
                   << ", or ";

            unsigned percentquotes =
                ((100000 * totalqbytes) / totalbytes);
            unsigned integ = percentquotes / 1000;
            unsigned fract = (percentquotes - integ * 1000 + 5) / 10;
            if (100 == fract)
            {
                fract = 0;
                integ ++;
            }
            report.form("%u.%02u%%,", integ, fract)
                   << " were quotes." << endl;

            report << endl;
        }
    }

    if (topwritten)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of writers" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::persstat_s data;
        unsigned oldwritten = 0;
        while (place <= maxnumber && engine->GetTopWriters(restart, data))
        {
            if (!data.messageswritten) break;

            if (restart)
            {
                restart = false;
                report << "Place Name                                "
                          "               Msgs    Bytes Quoted"  << endl;
            }

            if (!showallnums && data.messageswritten == oldwritten)
            {
                report << "      ";
            }
            else
            {
                report.form("%4u. ", place);
            }

            unsigned long long percentquotes =
                data.byteswritten ? ((10000 * data.bytesquoted) /
                                     data.byteswritten)
                             : 0;
            unsigned integ = percentquotes / 100;
            unsigned fract = (percentquotes - integ * 100 + 5) / 10;
            if (10 == fract)
            {
                fract = 0;
                integ ++;
            }

            if (data.name != data.address)
                tmp = string(data.name + " <") +
                      string(data.address + ">");
            else
                tmp = data.name;

            report.form("%-50.50s%5u %8u",
                        tmp.c_str(), data.messageswritten,
                        data.byteswritten);

            if (data.bytesquoted > 0)
            {
                report.form(" %3u.%1u%%",
                             integ, fract);
            }
            else if (showallnums)
            {
                report << "    N/A";
            }
            report << endl;

            place ++;
            oldwritten = data.messageswritten;
        }

        report << endl;

        if (news || !topreceived)
        {
            report << "A total of " << engine->GetTotalPeople()
                   << " people were identified";
            if (!news)
                report << " (senders and recipients)";
            report << '.' << endl;
            report << endl;
        }
    }

    if (toporiginal)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of original content per message" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::persstat_s data;
        unsigned oldoriginalpermsg = 0;
        while (place <= maxnumber &&
               engine->GetTopOriginalPerMsg(restart, data))
        {
            if (0 == data.messageswritten)
            {
                restart = false;
                continue;
            }

            if (restart)
            {
                restart = false;
                report << "Place Name                        "
                          "                Orig. / Msgs = PrMsg Quoted"
                       << endl;
            }

            unsigned originalpermsg =
                (data.byteswritten - data.bytesquoted) / data.messageswritten;

            if (!showallnums && originalpermsg == oldoriginalpermsg)
            {
                report << "      ";
            }
            else
            {
                report.form("%4u. ", place);
            }

            unsigned long long percentquotes =
                data.byteswritten ? ((10000 * data.bytesquoted) /
                                     data.byteswritten)
                             : 0;
            unsigned integ = percentquotes / 100;
            unsigned fract = (percentquotes - integ * 100 + 5) / 10;
            if (10 == fract)
            {
                fract = 0;
                integ ++;
            }

            if (data.name != data.address)
                tmp = string(data.name + " <") +
                      string(data.address + ">");
            else
                tmp = data.name;

            report.form("%-43.43s%6u /%5u = %5u",
                        tmp.c_str(),
                        data.byteswritten - data.bytesquoted,
                        data.messageswritten, originalpermsg);

            if (data.bytesquoted > 0)
            {
                report.form(" %3u.%1u%%",
                             integ, fract);
            }
            else if (showallnums)
            {
                report << "    N/A";
            }
            report << endl;

            place ++;
            oldoriginalpermsg = originalpermsg;
        }

        report << endl;
    }

    if (topnets && !news)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of nets" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::netstat_s data;
        unsigned oldcount = 0;
        while (place <= maxnumber && engine->GetTopNets(restart, data))
        {
            if (restart)
            {
                restart = false;

                report << "Place  Zone:Net   Messages    Bytes" << endl;
            }

            if (!showallnums && data.messages == oldcount)
            {
                report << "      ";
            }
            else
            {
                report.form("%4u. ", place);
            }

            report.form("%5u:%-5u %8u %8u",
                        data.zone, data.net, data.messages, data.bytes);

            report << endl;

            place ++;
            oldcount = data.messages;
        }

        report << endl;

        report << "Messages from " << engine->GetTotalNets()
               << " different nets were found." << endl;
        report << endl;
    }

    if (topdomains && (news || engine->GetTotalDomains()))
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of topdomains" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::domainstat_s data;
        unsigned oldcount = 0;
        while (place <= maxnumber && engine->GetTopDomains(restart, data))
        {
            if (restart)
            {
                restart = false;

                report << "Place Domain Messages    Bytes" << endl;
            }

            if (!showallnums && data.messages == oldcount)
            {
                report << "      ";
            }
            else
            {
                report.form("%4u. ", place);
            }

            report.form("%-7.7s%8u %8u",
                        data.topdomain.c_str(), data.messages, data.bytes);

            report << endl;

            place ++;
            oldcount = data.messages;
        }

        report << endl;

        report << "Messages from " << engine->GetTotalDomains()
               << " different top domains were found." << endl;
        report << endl;
    }

    if (topreceived && !news)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of receivers" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::persstat_s data;
        unsigned oldrecvd = 0;
        while (place <= maxnumber && engine->GetTopReceivers(restart, data))
        {
            if (!data.messagesreceived) break;

            if (restart)
            {
                restart = false;
                report << "Place Name                                "
                          "Rcvd  Sent   Ratio" << endl;
            }

            if (!showallnums && data.messagesreceived == oldrecvd)
            {
                report << "      ";
            }
            else
            {
                report.form("%4u. ", place);
            }

            if (data.name != "")
                tmp = data.name;
            else
                tmp = "(none)";

            report.form("%-35.35s%5u %5u",
                        tmp.c_str(),
                        data.messagesreceived,
                        data.messageswritten);

            if (data.messageswritten)
            {
                report.form(" %6u%%", (100 * data.messagesreceived) /
                                     data.messageswritten);
            }
            else if (showallnums)
            {
                report << "     N/A";
            }

            report << endl;

            place ++;
            oldrecvd = data.messagesreceived;
        }

        report << endl;
    }

    if (!news && topreceived)
    {
        report << "A total of " << engine->GetTotalPeople()
               << " people were identified (senders and recipients)." << endl;
        report << endl;
    }

    if (topsubjects)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of subjects" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::subjstat_s data;
        unsigned oldcount = 0;
        while (place <= maxnumber && engine->GetTopSubjects(restart, data))
        {
            if (restart)
            {
                restart = false;
                report << "Place Subject                                  "
                          "                  Msgs   Bytes" << endl;
            }

            if (!showallnums && data.count == oldcount)
            {
                report << "      ";
            }
            else
            {
                report.form("%4u. ", place);
            }

            if (data.subject != "")
                tmp = data.subject;
            else
                tmp = "(none)";

            report.form("%-57.57s%6u %7u",
                        tmp.c_str(), data.count, data.bytes);

            report << endl;

            place ++;
            oldcount = data.count;
        }

        report << endl;
        report << "A total of " << engine->GetTotalSubjects()
               << " subjects were identified." << endl;
        report << endl;
    }

    if (topprograms)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of programs" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::progstat_s data;
        unsigned oldcount = 0;
        while (place <= maxnumber && engine->GetTopPrograms(restart, data))
        {
            if (restart)
            {
                restart = false;
                report << "Place Program                              Msgs"
                       << endl;
            }

            if (!showallnums && data.count == oldcount)
            {
                report << "      ";
            }
            else
            {
                report.form("%4u. ", place);
            }

            report.form("%-35.35s%6u", data.program.c_str(), data.count);
            report << endl;

            if (showversions)
            {
                bool restartv = true;
                StatEngine::verstat_s vdata;
                while (engine->GetProgramVersions(restartv, vdata))
                {
                    if (restartv)
                    {
                        restartv = false;
                        report << "      ";
                    }
                    report << ' ' << vdata.version << ':' << vdata.count;
                }
                if (!restartv) report << endl;
            }

            place ++;
            oldcount = data.count;
        }

        report << endl;
        report << "A total of " << engine->GetTotalPrograms()
               << " different programs (not counting different versions) "
                  "were identified." << endl;
        report << endl;
    }

    if (weekstats)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Postings per weekday" << endl;
        report << endl;

        unsigned day[7], max = 1;
        for (int i = 0; i < 7; i ++)
        {
            day[i] = engine->GetDayMsgs(i);
            if (day[i] > max) max = day[i];
        }

        report << "Day        Msgs" << endl;

        for (int i = 0; i < 7; i ++)
        {
            int d = (i + 1) % 7;
            if (day[d] || showallnums)
            {
                report << days[i];
                report.form("%6u ", day[d]);
                int len = (60 * day[d]) / max;
                for (int j = 0; j < len; j ++) report << '*';
                report << endl;
            }
        }

        report << endl;
    }

    if (daystats)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Postings per hour" << endl;
        report << endl;

        unsigned hour[24], max = 1;
        for (int i = 0; i < 24; i ++)
        {
            hour[i] = engine->GetHourMsgs(i);
            if (hour[i] > max) max = hour[i];
        }

        report << "Hour       Msgs" << endl;

        for (int i = 0; i < 24; i ++)
        {
            if (hour[i] || showallnums)
            {
                report.form("%02d00-%02d59%6u ", i, i, hour[i]);
                int len = (60 * hour[i]) / max;
                for (int j = 0; j < len; j ++) report << '*';
                report << endl;
            }
        }

        report << endl;
    }

    report << "-------------------------------------------"
              "----------------------------------" << endl;

    report.close();
}
