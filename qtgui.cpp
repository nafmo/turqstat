// Turquoise SuperStat
//
// A statistic collection program for Fidonet and Usenet systems
// Qt version.
// Version 1.5
//
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

#include <time.h>
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qstring.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>

#include "qtgui.h"
#include "qtlist.h"

#include "statengine.h"
#include "arearead.h"
#include "squishread.h"
#include "fdapxread.h"
#include "jamread.h"
#include "mypointread.h"
#include "sdmread.h"
#include "tanstaaflread.h"
#include "newsspoolread.h"
//#include "version.h"
#include "utility.h"

InfoWindow::InfoWindow()
{
    // Construct menu
    menu = new QMenuBar(this);
    QPopupMenu *filemenu = new QPopupMenu(menu);
    filemenu->insertItem(tr("&Open message base"), this, SLOT(open()),
                         CTRL+Key_O);
    filemenu->insertItem(tr("&Save text file"), this, SLOT(save()), CTRL+Key_S);
    filemenu->insertItem(tr("&Exit"), qApp, SLOT(quit()), CTRL+Key_Q);
    menu->insertItem(tr("&File"), filemenu);

    QPopupMenu *showmenu = new QPopupMenu(menu);
    showmenu->insertItem(tr("&Quoter blacklist"), this, SLOT(quotelist()),
                         CTRL+Key_1);
    showmenu->insertItem(tr("&Sender toplist"), this, SLOT(senderlist()),
                         CTRL+Key_2);
    showmenu->insertItem(tr("&Original content toplist"), this,
                         SLOT(contentlist()), CTRL+Key_3);
    showmenu->insertItem(tr("&Fidonet net toplist"), this, SLOT(fidonetlist()),
                         CTRL+Key_4);
    showmenu->insertItem(tr("&Internet topdomain toplist"), this,
                         SLOT(domainlist()), CTRL+Key_5);
    showmenu->insertItem(tr("&Receiver toplist"), this, SLOT(receiverlist()),
                         CTRL+Key_6);
    showmenu->insertItem(tr("&Subject toplist"), this, SLOT(subjectlist()),
                         CTRL+Key_7);
    showmenu->insertItem(tr("Soft&ware toplist"), this, SLOT(softwarelist()),
                         CTRL+Key_8);
    showmenu->insertSeparator();
    showmenu->insertItem(tr("&Posting times"), this, SLOT(timelist()),
                         CTRL+Key_T);
    menu->insertItem(tr("&Show"), showmenu);

    // Add information boxes
    QGrid *grid = new QGrid(2, this);
    grid->setSpacing(5);
    grid->setMargin(5);

    QLabel *labelAreas = new QLabel(tr("&Areas loaded"), grid);
    numareas = new QLineEdit(grid, "numareas");
    numareas->setReadOnly(true);
    numareas->setAlignment(AlignRight);
    numareas->setText("0");
    labelAreas->setBuddy(numareas);

    QLabel *labelTexts = new QLabel(tr("&Texts examined"), grid);
    numtexts = new QLineEdit(grid, "numtexts");
    numtexts->setReadOnly(true);
    numtexts->setAlignment(AlignRight);
    numtexts->setText("0");
    labelTexts->setBuddy(numtexts);

    QLabel *labelBytes = new QLabel(tr("&Bytes written"), grid);
    numbytes = new QLineEdit(grid, "numbytes");
    numbytes->setReadOnly(true);
    numbytes->setAlignment(AlignRight);
    numbytes->setText("0");
    labelBytes->setBuddy(numbytes);

    QLabel *labelLines = new QLabel(tr("&Lines examined"), grid);
    numlines = new QLineEdit(grid, "numlines");
    numlines->setReadOnly(true);
    numlines->setAlignment(AlignRight);
    numlines->setText("0");
    labelLines->setBuddy(numlines);

    QLabel *labelQBytes = new QLabel(tr("Bytes &quoted"), grid);
    numqbytes = new QLineEdit(grid, "numqbytes");
    numqbytes->setReadOnly(true);
    numqbytes->setAlignment(AlignRight);
    numqbytes->setText("0");
    labelQBytes->setBuddy(numqbytes);

    QLabel *labelQLines = new QLabel(tr("L&ines quoted"), grid);
    numqlines = new QLineEdit(grid, "numqlines");
    numqlines->setReadOnly(true);
    numqlines->setAlignment(AlignRight);
    numqlines->setText("0");
    labelQLines->setBuddy(numqlines);

    QLabel *labelPeople = new QLabel(tr("&People identified"), grid);
    numpeople = new QLineEdit(grid, "numpeople");
    numpeople->setReadOnly(true);
    numpeople->setAlignment(AlignRight);
    numpeople->setText("0");
    labelPeople->setBuddy(numpeople);

    QLabel *labelSubjects = new QLabel(tr("S&ubjects found"), grid);
    numsubjects = new QLineEdit(grid, "subjects");
    numsubjects->setReadOnly(true);
    numsubjects->setAlignment(AlignRight);
    numsubjects->setText("0");
    labelSubjects->setBuddy(numsubjects);

    QLabel *labelPrograms = new QLabel(tr("P&rograms used"), grid);
    numprograms = new QLineEdit(grid, "numprograms");
    numprograms->setReadOnly(true);
    numprograms->setAlignment(AlignRight);
    numprograms->setText("0");
    labelPrograms->setBuddy(numprograms);

    QLabel *labelNets = new QLabel(tr("Fidonet &nets represented"), grid);
    numnets = new QLineEdit(grid, "numnets");
    numnets->setReadOnly(true);
    numnets->setAlignment(AlignRight);
    numnets->setText("0");
    labelNets->setBuddy(numnets);

    QLabel *labelDomains = new QLabel(tr("Top &domains represented"), grid);
    numdomains = new QLineEdit(grid, "numdomains");
    numdomains->setReadOnly(true);
    numdomains->setAlignment(AlignRight);
    numdomains->setText("0");
    labelDomains->setBuddy(numdomains);

    QLabel *labelEarliest = new QLabel(tr("&Earliest text written"), grid);
    earliestwritten = new QLineEdit(grid, "earliestwritten");
    earliestwritten->setReadOnly(true);
    earliestwritten->setAlignment(AlignRight);
    earliestwritten->setText("None loaded");
    labelEarliest->setBuddy(earliestwritten);

    QLabel *labelLatest = new QLabel(tr("Latest text &written"), grid);
    latestwritten = new QLineEdit(grid, "latestwritten");
    latestwritten->setReadOnly(true);
    latestwritten->setAlignment(AlignRight);
    latestwritten->setText("None loaded");
    labelLatest->setBuddy(latestwritten);

    setCentralWidget(grid);

    hasnews = false;
    hasany = false;

    connect(this, SIGNAL(newdata()), SLOT(update()));

    // Objects owned
    engine = new StatEngine;
    progress = NULL;
};

InfoWindow::~InfoWindow()
{
#warning Destroy
}

InfoWindow *InfoWindow::getMainWindow()
{
    static InfoWindow *mainwindow = NULL;

    if (!mainwindow)
    {
        mainwindow = new InfoWindow;
    }
    return mainwindow;
}

// Open a message base
void InfoWindow::open()
{
    QString fileselect =
        QFileDialog::getOpenFileName(QString::null, 
                                     tr("FTSC/Opus SDM (*.msg);;"
                                        "Squish (*.sqd);;"
                                        "FDAPX/w (msgstat.apx);;"
                                        "JAM (*.jdt);;"
                                        "MyPoint (mypoint.*);;"
                                        "Tanstaafl (msgstat.tfl);;"
                                        "News (*)"), this);

    if (fileselect.isEmpty()) return;

    AreaRead *area = NULL;
    const char *path = fileselect.latin1();
    bool isnews = false;

    // Determine what we got based on the file name (yes, that is ugly, but
    // what else can we do?)
    if (fileselect.find(QRegExp("*.msg", false, true)) != -1)
    {
        // "*.msg" - Determine FTSC/Opus style
        if (hasany && hasnews)
        {
            incompatible();
            return;
        }

#warning Determine SDM style
        QMessageBox::information(this, "Turquoise SuperStat",
                                tr("FTSC/Opus style message base"));
    }
    else if (fileselect.find(QRegExp("*.sqd", false, true)) != -1)
    {
        // Squish
        if (hasany && hasnews)
        {
            incompatible();
            return;
        }

        QMessageBox::information(this, "Turquoise SuperStat",
                                tr("Squish message base"));

        area = new SquishRead(path);
    }
    else if (fileselect.find(QRegExp("*.apx", false, true)) != -1)
    {
        // FDAPX/w
        if (hasany && hasnews)
        {
            incompatible();
            return;
        }

        QMessageBox::information(this, "Turquoise SuperStat",
                                tr("FDAPX/w message base"));

#warning Ask for area number
//        area = new FdApxRead(path, 1);
    }
    else if (fileselect.find(QRegExp("*.jdt", false, true)) != -1)
    {
        // JAM
        if (hasany && hasnews)
        {
            incompatible();
            return;
        }

        QMessageBox::information(this, "Turquoise SuperStat",
                                tr("JAM message base"));

        area = new JamRead(path);
    }
    else if (fileselect.find(QRegExp("*.tfl", false, true)) != -1)
    {
        // Taanstafl
        if (hasany && hasnews)
        {
            incompatible();
            return;
        }

        QMessageBox::information(this, "Turquoise SuperStat",
                                tr("Tanstaafl message base"));

#warning Ask for area number
//        area = new TanstaaflRead(path, 1);
    }
    else if (fileselect.find(QRegExp("*mypoint.*", false, true)) != -1)
    {
        // MyPoint
        if (hasany && hasnews)
        {
            incompatible();
            return;
        }

        QMessageBox::information(this, "Turquoise SuperStat",
                                tr("MyPoint message base"));

#warning Ask for area number
//        area = new MyPointRead(path, 1);
    }
    else if (fileselect.find(QRegExp(".*/[0-9]*$", false)) != -1)
    {
        // News spool
        if (hasany && !hasnews)
        {
            incompatible();
            return;
        }

        QMessageBox::information(this, "Turquoise SuperStat",
                                tr("Usenet News Spool"));

        fileselect.truncate(fileselect.findRev('/'));
        path = fileselect.latin1();
        area = new NewsSpoolRead(path);
        isnews = true;
    }
    else
    {
        // Huh?
        QMessageBox::warning(this, "Turquoise SuperStat",
                             tr("I don't understand your selection"));
        return;
    }

    // Transfer data
    if (area)
    {
time_t from = 0;
        progress =
            new QProgressDialog(tr("Reading message base"), 0, 100, this,
                                "progress", true);
        progress->setCaption("Turquoise SuperStat");
        progress->setMinimumDuration(1000);
        area->Transfer(from, *engine);
        engine->AreaDone();
        hasnews = isnews;
        hasany = true;

        delete progress;
        progress = NULL;
    }
    else
    {
        QMessageBox::warning(this, "Turquoise SuperStat",
                             tr("Out of memory allocating area object"));
    }

    // Update information boxes
    emit newdata();
}

void InfoWindow::incompatible()
{
    QMessageBox::warning(this, "Turquoise SuperStat",
                         tr("You cannot mix Fidonet and Usenet areas"));
}

void InfoWindow::update()
{
    // Update information in all boxes

    numareas->setText(QString::number(engine->GetTotalAreas()));
    numtexts->setText(QString::number(engine->GetTotalNumber()));
    numbytes->setText(QString::number(engine->GetTotalBytes()));
    numlines->setText(QString::number(engine->GetTotalLines()));
    numqbytes->setText(QString::number(engine->GetTotalQBytes()));
    numqlines->setText(QString::number(engine->GetTotalQLines()));
    numpeople->setText(QString::number(engine->GetTotalPeople()));
    numsubjects->setText(QString::number(engine->GetTotalSubjects()));
    numprograms->setText(QString::number(engine->GetTotalPrograms()));
    if (hasnews)
    {
        numnets->setEnabled(false);
        numnets->setText(tr("N/A"));
    }
    else
        numnets->setText(QString::number(engine->GetTotalNets()));
    numdomains->setText(QString::number(engine->GetTotalDomains()));

    char timebuf[64];
    time_t earliest = engine->GetEarliestWritten();
    struct tm *p1 = localtime(&earliest);
    strftime(timebuf, 64, "%x", p1);
    earliestwritten->setText(timebuf);

    time_t latest = engine->GetLastWritten();
    struct tm *p2 = localtime(&latest);
    strftime(timebuf, 64, "%x", p2);
    latestwritten->setText(timebuf);
}

void InfoWindow::quotelist()
{
    TopListWindow *quotedialog =
        new TopListWindow(this, "quotelist", TopListWindow::Quoters);
    if (quotedialog)
    {
        quotedialog->fillOut(engine);
        quotedialog->show();
    }
}

void InfoWindow::senderlist()
{
    TopListWindow *senderdialog =
        new TopListWindow(this, "senderlist", TopListWindow::Senders);
    if (senderdialog)
    {
        senderdialog->fillOut(engine);
        senderdialog->show();
    }
}

void InfoWindow::contentlist()
{
    TopListWindow *contentdialog =
        new TopListWindow(this, "contentlist", TopListWindow::OrigContent);
    if (contentdialog)
    {
        contentdialog->fillOut(engine);
        contentdialog->show();
    }
}

void InfoWindow::fidonetlist()
{
    TopListWindow *fidonetsdialog =
        new TopListWindow(this, "netlist", TopListWindow::FidoNets);
    if (fidonetsdialog)
    {
        fidonetsdialog->fillOut(engine);
        fidonetsdialog->show();
    }
}

void InfoWindow::domainlist()
{
    TopListWindow *domaindialog =
        new TopListWindow(this, "domainlist", TopListWindow::Domains);
    if (domaindialog)
    {
        domaindialog->fillOut(engine);
        domaindialog->show();
    }
}

void InfoWindow::receiverlist()
{
    TopListWindow *receiverdialog =
        new TopListWindow(this, "receiverlist", TopListWindow::Receivers);
    if (receiverdialog)
    {
        receiverdialog->fillOut(engine);
        receiverdialog->show();
    }
}

void InfoWindow::subjectlist()
{
    TopListWindow *subjectdialog =
        new TopListWindow(this, "subjectlist", TopListWindow::Subjects);
    if (subjectdialog)
    {
        subjectdialog->fillOut(engine);
        subjectdialog->show();
    }
}

void InfoWindow::softwarelist()
{
    TopListWindow *softwaredialog =
        new TopListWindow(this, "softwaredialog", TopListWindow::Software);
    if (softwaredialog)
    {
        softwaredialog->fillOut(engine);
        softwaredialog->show();
    }
}

void InfoWindow::timelist()
{
}


// Program entry
int main(int argc, char *argv[])
{
    // Create an application
    QApplication gui(argc, argv);

    // Create a window
    InfoWindow *mainwindow = InfoWindow::getMainWindow();
    mainwindow->setCaption("Turquoise SuperStat");
    mainwindow->show();

    // Run, window, run
    gui.connect(&gui, SIGNAL(lastWindowClosed()), &gui, SLOT(quit()));
    return gui.exec();
}
