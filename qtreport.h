// Copyright (c) 2000 Peter Karlsson
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

#ifndef __QTREPORT_H
#define __QTREPORT_H

#include <qdialog.h>

class StatEngine;
class QCheckBox;
class QSpinBox;

class ReportSelectWindow : public QDialog
{
    Q_OBJECT

public:
    ReportSelectWindow(QWidget *parent, const char *name, StatEngine *);
    ~ReportSelectWindow();

protected:
    // Statistics engine
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
#ifdef HAVE_LOCALE_H
    QCheckBox *uselocale;
#endif

    // Input boxes
    QSpinBox *maxnum;

protected slots:
    void saveToFile();

private:
    // Static members to remember values
    static bool doquoters;
    static bool dotopwritten;
    static bool dotoporiginal;
    static bool dotopnets;
    static bool dotopdomains;
    static bool dotopreceived;
    static bool dotopsubjects;
    static bool dotopprograms;
    static bool doweekstats;
    static bool dodaystats;
#ifdef HAVE_LOCALE_H
    static bool douselocale;
#endif

    static int defaultmaxnum;
};

#endif
