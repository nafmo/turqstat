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

#include <fstream.h>
#include "statview.h"

static const char *days[] =
    { "Monday   ",
      "Tuesday  ",
      "Wednesday",
      "Thursday ",
      "Friday   ",
      "Saturday ",
      "Sunday   " };

bool StatView::CreateReport(StatEngine *engine, string filename,
    unsigned maxnumber,
    bool quoters, bool topwritten, bool topreceived, bool topsubjects,
    bool topprograms, bool weekstats, bool daystats)
{
    // Create a report file
    fstream report(filename.c_str(), ios::out);
    if (!(report.is_open())) return false;

    // Include data as given from the statistics engine
    report << "Turqoise * Message area statistics" << endl;
    report << "==================================" << endl;
    report << endl;
    report << "(c) Copyright 1998-1999 Peter Karlsson" << endl;
    report << "This is a BETA version" << endl;
    report << endl;

    if (0 == engine->GetTotalNumber())
    {
        report << "Message area is empty." << endl;
        report.close();
    }


    report.form("This report covers %u messages that were received at this "
                "system between ", engine->GetTotalNumber());

    time_t earliest = engine->GetEarliestReceived();
    struct tm *p1 = localtime(&earliest);

    report.form("%04d-%02d-%02d %02d.%02d.%02d and ",
                p1->tm_year + 1900, p1->tm_mon + 1, p1->tm_mday,
                p1->tm_hour, p1->tm_min, p1->tm_sec);

    time_t latest = engine->GetLastReceived();
    struct tm *p2 = localtime(&latest);

    report.form("%04d-%02d-%02d %02d.%02d.%02d",
                p2->tm_year + 1900, p2->tm_mon + 1, p2->tm_mday,
                p2->tm_hour, p2->tm_min, p2->tm_sec);

    report << " (written between ";

    time_t wearliest = engine->GetEarliestWritten();
    struct tm *p3 = localtime(&wearliest);

    report.form("%04d-%02d-%02d %02d.%02d.%02d and ",
                p3->tm_year + 1900, p3->tm_mon + 1, p3->tm_mday,
                p3->tm_hour, p3->tm_min, p3->tm_sec);

    time_t wlatest = engine->GetLastWritten();
    struct tm *p4 = localtime(&wlatest);

    report.form("%04d-%02d-%02d %02d.%02d.%02d)",
                p4->tm_year + 1900, p4->tm_mon + 1, p4->tm_mday,
                p4->tm_hour, p4->tm_min, p4->tm_sec) << endl;


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
                          "Msgs Address          Ratio" << endl;
            }

            if (data.messageswritten > 2 && data.bytesquoted > 0)
            {
                unsigned percentquotes =
                    data.byteswritten ?
                      ((100000 * (unsigned long long) data.bytesquoted) /
                       (unsigned long long) data.byteswritten)
                      : 0;
                if (percentquotes == oldpercent)
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

                if (data.name != "")
                    tmp = data.name;
                else
                    tmp = "(none)";

                report.form("%-35s%5u %-15s%3u.%02u%%",
                            tmp.c_str(), data.messageswritten,
                            data.address.c_str(), integ, fract);
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
                          "Msgs Address           Bytes Quoted" << endl;
            }

            if (data.messageswritten == oldwritten)
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

            if (data.name != "")
                tmp = data.name;
            else
                tmp = "(none)";

            report.form("%-35s%5u %-15s%8u",
                         tmp.c_str(), data.messageswritten,
                         data.address.c_str(), data.byteswritten);
            if (data.bytesquoted > 0)
            {
                report.form(" %3u.%1u%%",
                             integ, fract);
            }
            report << endl;

            place ++;
            oldwritten = data.messageswritten;
        }

        report << endl;
    }

    if (topreceived)
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

            if (data.messagesreceived == oldrecvd)
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

            report.form("%-35s%5u %5u",
                        tmp.c_str(),
                        data.messagesreceived,
                        data.messageswritten);

            if (data.messageswritten)
            {
                report.form(" %6u%%", (100 * data.messagesreceived) /
                                     data.messageswritten);
            }

            report << endl;

            place ++;
            oldrecvd = data.messagesreceived;
        }

        report << endl;
    }

    if (topreceived || topwritten)
    {
        report << "A total of " << engine->GetTotalPeople()
               << " people were identified (senders and recipients)"
               << endl;
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
                          "                Msgs   Bytes" << endl;
            }

            if (data.count == oldcount)
            {
                report << "      ";
            }
            else
            {
                report.form("%4u. ", place);
            }

            if (data.subject != "")
            {
                if (data.subject.length() > 55)
                    tmp = data.subject.substr(0, 55);
                else
                    tmp = data.subject;
            }
            else
                tmp = "(none)";

            report.form("%-55s%6u %7u",
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

            if (data.count == oldcount)
            {
                report << "      ";
            }
            else
            {
                report.form("%4u. ", place);
            }

            report.form("%-35s%6u", data.program.c_str(), data.count);
            report << endl;

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

        unsigned day[7], max = 0;
        for (int i = 0; i < 7; i ++)
        {
            day[i] = engine->GetDayMsgs(i);
            if (day[i] > max) max = day[i];
        }

        report << "Day        Msgs" << endl;

        for (int i = 0; i < 7; i ++)
        {
            int d = (i + 1) % 7;
            if (day[d])
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

        unsigned hour[24], max =0;
        for (int i = 0; i < 24; i ++)
        {
            hour[i] = engine->GetHourMsgs(i);
            if (hour[i] > max) max = hour[i];
        }

        report << "Hour       Msgs" << endl;

        for (int i = 0; i < 24; i ++)
        {
            if (hour[i])
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
