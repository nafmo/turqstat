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

#ifndef __QTLIST_H
#define __QTLIST_H

#include <qdialog.h>
#include <string>

#if !defined(HAVE_IMPLICIT_NAMESPACE)
using namespace std;
#endif

class QListView;
class StatEngine;
#if !defined(HAVE_WORKING_WSTRING)
class wstring;
#endif

/**
 * Class describing a window showing a toplist. This class is used in the Qt
 * version to display the various toplists.
 */
class TopListWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * Numeric constants describing all available toplists that can be
     * displayed by this class.
     */
    enum toplist_e
    {
        Quoters,        ///< Blacklist of quoters.
        Senders,        ///< Toplist of senders.
        OrigContent,    ///< Toplist of original contents.
        FidoNets,       ///< Toplist of Fidonet networks.
        Domains,        ///< Toplist of Internet topdomains.
        Receivers,      ///< Toplist of receivers.
        Subjects,       ///< Toplist of subject lines.
        Software        ///< Toplist of posting software.
    };

    /**
     * Standard constructor. Creates a window ready to fill in with toplist
     * data.
     * @param parent Parent widget owning the window.
     * @param name   Name given to this instance.
     * @param list   Type of list to display in the window.
     */
    TopListWindow(QWidget *parent, const char *name, toplist_e list);
    /** Standard destructor. */
    ~TopListWindow();

    /**
     * Fill the toplist with statistics from loaded message bases.
     * @param engine Pointer to the engine containing the statistics.
     */
    void fillOut(StatEngine *engine);

protected:
    /** Internal method used to setup the column headings. */
    void setupHeaders();
    /** Internal method used for creating a quoter blacklist. */
    void addQuoters(StatEngine *);
    /** Internal method used for creating a senders toplist. */
    void addSenders(StatEngine *);
    /** Internal method used for creating a original content toplist. */
    void addOriginalContent(StatEngine *);
    /** Internal method used for creating a Fidonet network toplist. */
    void addFidoNets(StatEngine *);
    /** Internal method used for creating a Internet topdomain toplist. */
    void addDomains(StatEngine *);
    /** Internal method used for creating a receiver toplist. */
    void addReceivers(StatEngine *);
    /** Internal method used for creating a subject toplist. */
    void addSubjects(StatEngine *);
    /** Internal method used for creating a software toplist. */
    void addSoftware(StatEngine *);

    /**
     * Convert a wide character string into a QString.
     * @param inputstring String to copy.
     * @return A QString representation of the string.
     */
    static QString charConvert(wstring &);
    /** Create a percent string to display in a toplist. */
    QString percentString(int, int);

    /** List view used to display the actual toplist. */
    QListView   *listview;

    /** The kind of toplist associated with this object. */
    enum toplist_e toplist;

protected slots:
    /**
     * Slot for saving the contents of the current window to file. Called
     * when the save button is pressed.
     */
    void saveToFile();
};

#endif
