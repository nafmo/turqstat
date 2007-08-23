// Copyright (c) 2000-2001 Peter Karlsson
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

#ifndef __QTREPORT_H
#define __QTREPORT_H

#include <qdialog.h>

class StatEngine;
class QCheckBox;
class QSpinBox;
class QComboBox;

/**
 * Class describing a window asking what to save to file. This class is used
 * in the Qt version to ask the user which toplists to save to the compound
 * report file (the same file that is created by the command line version).
 */
class ReportSelectWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * Standard constructor. Creates a window ready to fill in with toplist
     * data.
     * @param parent   Parent widget owning the window.
     * @param name     Name given to this instance.
     * @param engine_p Pointer to engine containing the statistics.
     */
    ReportSelectWindow(QWidget *parent, const char *name, StatEngine *);
    /** Standard destructor. */
    ~ReportSelectWindow();

protected:
    /** Statistics engine. */
    StatEngine *engine;

    // Checkboxes
    QCheckBox *quoters;
    QCheckBox *topwritten;
    QCheckBox *toporiginal;
    QCheckBox *topnets;
    QCheckBox *topdomains;
    QCheckBox *topreceived;
    QCheckBox *topsubjects;
    QCheckBox *topprograms;
    QCheckBox *weekstats;
    QCheckBox *daystats;
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    QCheckBox *uselocale;
#endif

    // Input boxes
    QSpinBox *maxnum;
    QComboBox *charset;

protected slots:
    /**
     * Slot for actually creating the report file. Called when the save
     * button is pressed.
     */
    void saveToFile();

private:
    // Static members to remember values
    static bool doquoters;      ///< Remember if we want quoter toplist.
    static bool dotopwritten;   ///< Remember if we want sender toplist.
    static bool dotoporiginal;  ///< Remember if we want original content
                                ///< toplist.
    static bool dotopnets;      ///< Remember if we want Fidonet toplist.
    static bool dotopdomains;   ///< Remember if we want topdomain toplist.
    static bool dotopreceived;  ///< Remember if we want receiver toplist.
    static bool dotopsubjects;  ///< Remember if we want subjects toplist.
    static bool dotopprograms;  ///< Remember if we want programs toplist.
    static bool doweekstats;    ///< Remember if we want weekday statistics.
    static bool dodaystats;     ///< Remember if we want hour statistics.
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    static bool douselocale;    ///< Remember if we want local date formats.
#endif

    static int defaultmaxnum;   ///< Remember number of entries in toplists.
    static QString docharset;   ///< Remember character set to use.
};

#endif
