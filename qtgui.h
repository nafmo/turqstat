// Copyright (c) 2000-2001 Peter Karlsson
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

#ifndef __QTGUI_H
#define __QTGUI_H

#include <qobject.h>
#include <qmainwindow.h>
#include <time.h>

class StatEngine;
class QMenuBar;
class QLineEdit;
class QProgressDialog;

/**
 * Class describing our main window. This class is used in the Qt version to
 * display the main window to the user.
 */
class InfoWindow : public QMainWindow
{
    Q_OBJECT

public:
    /** Standard constructor. */
    InfoWindow();
    /** Standard destructor. */
    ~InfoWindow();

    /**
     * Retrieve the progress dialog associated with the main window.
     * @return Pointer to the QProgressDialog object used with the main
     *         window.
     */
    QProgressDialog *getProgressDialog() { return progress; };

    /**
     * Retrieve global main window object. Use this static method to
     * retrieve the global (and only) instance of this class. The object is
     * instanciated on the first call.
     *
     * @return The global InfoWindow object.
     */
    static InfoWindow *getMainWindow();

private:
    /** Main menubar. */
    QMenuBar *menu;

    // Data displayers
    QLineEdit *numareas;
    QLineEdit *numtexts;
    QLineEdit *numbytes;
    QLineEdit *numlines;
    QLineEdit *numqbytes;
    QLineEdit *numqlines;
    QLineEdit *numpeople;
    QLineEdit *numsubjects;
    QLineEdit *numprograms;
    QLineEdit *numnets;
    QLineEdit *numdomains;
    QLineEdit *earliestwritten;
    QLineEdit *latestwritten;

    /** Progress bar dialog. */
    QProgressDialog *progress;

    // Internal state
    bool hasnews,   ///< State variable indicating if we have seen news areas.
         hasany;    ///< State variable indicating if we have seen any areas.

    // Statistics
    StatEngine *engine; ///< Engine containing all collected data.

    /**
     * Display a message saying that area types are incompatible. Internal.
     */
    void incompatible();

    /**
     * Fill main window with initial values. Internal.
     */
    void zeroFill();

    // Start date
    time_t start;   ///< Earliest time to collect statistics from.
    time_t end;     ///< Last time to collect statistics to.
    int daysback;   ///< Number of days back to collect statistics from.

public slots:
    void open();            ///< Slot handling File|Open message base.
    void clear();           ///< Slot handling File|Clear data.
    void report();          ///< Slot handling File|Save report.
    void quotelist();       ///< Slot handling Show|Quoter blacklist.
    void senderlist();      ///< Slot handling Show|Sender toplist.
    void contentlist();     ///< Slot handling Show|Original content toplist.
    void fidonetlist();     ///< Slot handling Show|Fidonet net toplist.
    void domainlist();      ///< Slot handling Show|Internet topdomain toplist.
    void receiverlist();    ///< Slot handling Show|Receiver toplist.
    void subjectlist();     ///< Slot handling Show|Subject toplist.
    void softwarelist();    ///< Slot handling Show|Software toplist.
    void timelist();        ///< Slot handling Show|Postings per hour.
    void daylist();         ///< Slot handling Show|Postings per day.
    void startdate();       ///< Slot handling Edit|Set start date.

    void update();          ///< Slot handling incoming new statistics data.

signals:
    void newdata();         ///< Signal indicating new statistics data.
};

#endif
