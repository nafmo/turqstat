// Copyright (c) 2000-2002 Peter Karlsson
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

class AreaRead;
class StatEngine;
class QMenuBar;
class QLineEdit;
class QProgressDialog;
class ProgressText;

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
     * Retrieve the progress bar dialog associated with the main window.
     *
     * @param maximum Total number of steps to display.
     * @return Pointer to the QProgressDialog object used with the main
     *         window.
     */
    QProgressDialog *getProgressDialog(int maximum);

    /**
     * Retrieve the progress text dialog associated with the main window.
     *
     * @return Pointer to the ProgressText object used with the main window.
     */
     ProgressText *getProgressText();

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
    QProgressDialog *progressdialog;
    /** Progress text dialog. */
    ProgressText *progresstext;

    // Internal state
    bool hasnews,   ///< State variable indicating if we have seen news areas.
         hasany;    ///< State variable indicating if we have seen any areas.
    QString defaultserver; ///< Last selected news server.

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

    /**
     * Transfer data from an area object to the engine. Internal.
     */
    void transfer(AreaRead *, bool);

    // Start date
    time_t start;   ///< Earliest time to collect statistics from.
    time_t end;     ///< Last time to collect statistics to.
    int daysback;   ///< Number of days back to collect statistics from.

public slots:
    void open();            ///< Slot handling File|Open message base.
    void opennews();        ///< Slot handling File|Open news group.
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
