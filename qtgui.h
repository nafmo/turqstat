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

#ifndef __QTGUI_H
#define __QTGUI_H

#include <qobject.h>
#include <qmainwindow.h>

class StatEngine;
class QMenuBar;
class QLineEdit;
class QProgressDialog;

// This is our main window class
class InfoWindow : public QMainWindow
{
    Q_OBJECT

public:
    InfoWindow();
    ~InfoWindow();

    QProgressDialog *getProgressDialog() { return progress; };

    static InfoWindow *getMainWindow();

private:
    // Main menubar
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

    // Progress bar dialog
    QProgressDialog *progress;

    // Internal state
    bool hasnews, hasany;

    // Statistics
    StatEngine *engine;

    // Internal methods
    void incompatible();
    void zeroFill();
    
public slots:
    void open();
    void clear();
    void report();
    void quotelist();
    void senderlist();
    void contentlist();
    void fidonetlist();
    void domainlist();
    void receiverlist();
    void subjectlist();
    void softwarelist();
    void timelist();
    void daylist();
    void update();

signals:
    void newdata();
};

#endif
