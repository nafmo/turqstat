// Turquoise SuperStat
//
// A statistic collection program for Fidonet and Usenet systems
// Qt version.
// Version 2.2
//
// Copyright (c) 2000-2005 Peter Karlsson
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

#include <time.h>
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#include <stdlib.h>

#include <qglobal.h>
#if QT_VERSION < 230
# error This program is written for Qt version 2.3.0 or later
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
#include <qinputdialog.h>

#include "qtgui.h"
#include "qtlist.h"
#include "qtreport.h"
#include "qtbars.h"
#include "qtprogress.h"

#include "statengine.h"
#include "arearead.h"
#include "squishread.h"
#include "fdapxread.h"
#include "jamread.h"
#include "smbread.h"
#include "mypointread.h"
#include "sdmread.h"
#include "tanstaaflread.h"
#include "newsspoolread.h"
#if defined(HAVE_NNTP)
# include "nntpread.h"
#endif
#include "mytime.h"
#include "utility.h"

#define MENU_FIDOTOPLIST 1000

InfoWindow::InfoWindow()
    : hasnews(false), hasany(false)
{
    // Construct menu
    menu = new QMenuBar(this);
    QPopupMenu *filemenu = new QPopupMenu(menu);
    filemenu->insertItem(tr("&Open message base"), this, SLOT(open()),
                         CTRL+Key_O);
#if defined(HAVE_NNTP)
    filemenu->insertItem(tr("Open &news group"), this, SLOT(opennews()),
                         CTRL+Key_U);
#endif
    filemenu->insertItem(tr("&Clear data"), this, SLOT(clear()), CTRL+Key_L);
    filemenu->insertItem(tr("&Save report"), this, SLOT(report()), CTRL+Key_S);
    filemenu->insertSeparator();
    filemenu->insertItem(tr("&Exit"), qApp, SLOT(quit()), CTRL+Key_Q);
    menu->insertItem(tr("&File"), filemenu);

    QPopupMenu *editmenu = new QPopupMenu(menu);
    editmenu->insertItem(tr("&Set start date"), this, SLOT(startdate()),
                         CTRL+Key_Home);
    menu->insertItem(tr("&Edit"), editmenu);

    QPopupMenu *showmenu = new QPopupMenu(menu);
    showmenu->insertItem(tr("&Quoter blacklist"), this, SLOT(quotelist()),
                         CTRL+Key_1);
    showmenu->insertItem(tr("&Sender toplist"), this, SLOT(senderlist()),
                         CTRL+Key_2);
    showmenu->insertItem(tr("&Original content toplist"), this,
                         SLOT(contentlist()), CTRL+Key_3);
    showmenu->insertItem(tr("&Fidonet net toplist"), this, SLOT(fidonetlist()),
                         CTRL+Key_4, MENU_FIDOTOPLIST);
    showmenu->insertItem(tr("&Internet topdomain toplist"), this,
                         SLOT(domainlist()), CTRL+Key_5);
    showmenu->insertItem(tr("&Receiver toplist"), this, SLOT(receiverlist()),
                         CTRL+Key_6);
    showmenu->insertItem(tr("S&ubject toplist"), this, SLOT(subjectlist()),
                         CTRL+Key_7);
    showmenu->insertItem(tr("Soft&ware toplist"), this, SLOT(softwarelist()),
                         CTRL+Key_8);
    showmenu->insertSeparator();
    showmenu->insertItem(tr("Postings per &hour"), this, SLOT(timelist()),
                         CTRL+Key_T);
    showmenu->insertItem(tr("Postings per &day"), this, SLOT(daylist()),
                         CTRL+Key_D);
    menu->insertItem(tr("&Show"), showmenu);

    // Add information boxes
    QGrid *grid = new QGrid(2, this);
    grid->setSpacing(5);
    grid->setMargin(5);

    QLabel *labelAreas = new QLabel(tr("&Areas loaded"), grid);
    numareas = new QLineEdit(grid, "numareas");
    numareas->setReadOnly(true);
    numareas->setAlignment(AlignRight);
    labelAreas->setBuddy(numareas);

    QLabel *labelTexts = new QLabel(tr("&Texts examined"), grid);
    numtexts = new QLineEdit(grid, "numtexts");
    numtexts->setReadOnly(true);
    numtexts->setAlignment(AlignRight);
    labelTexts->setBuddy(numtexts);

    QLabel *labelBytes = new QLabel(tr("&Bytes written"), grid);
    numbytes = new QLineEdit(grid, "numbytes");
    numbytes->setReadOnly(true);
    numbytes->setAlignment(AlignRight);
    labelBytes->setBuddy(numbytes);

    QLabel *labelLines = new QLabel(tr("&Lines examined"), grid);
    numlines = new QLineEdit(grid, "numlines");
    numlines->setReadOnly(true);
    numlines->setAlignment(AlignRight);
    labelLines->setBuddy(numlines);

    QLabel *labelQBytes = new QLabel(tr("Bytes &quoted"), grid);
    numqbytes = new QLineEdit(grid, "numqbytes");
    numqbytes->setReadOnly(true);
    numqbytes->setAlignment(AlignRight);
    labelQBytes->setBuddy(numqbytes);

    QLabel *labelQLines = new QLabel(tr("L&ines quoted"), grid);
    numqlines = new QLineEdit(grid, "numqlines");
    numqlines->setReadOnly(true);
    numqlines->setAlignment(AlignRight);
    labelQLines->setBuddy(numqlines);

    QLabel *labelPeople = new QLabel(tr("&People identified"), grid);
    numpeople = new QLineEdit(grid, "numpeople");
    numpeople->setReadOnly(true);
    numpeople->setAlignment(AlignRight);
    labelPeople->setBuddy(numpeople);

    QLabel *labelSubjects = new QLabel(tr("S&ubjects found"), grid);
    numsubjects = new QLineEdit(grid, "subjects");
    numsubjects->setReadOnly(true);
    numsubjects->setAlignment(AlignRight);
    labelSubjects->setBuddy(numsubjects);

    QLabel *labelPrograms = new QLabel(tr("P&rograms used"), grid);
    numprograms = new QLineEdit(grid, "numprograms");
    numprograms->setReadOnly(true);
    numprograms->setAlignment(AlignRight);
    labelPrograms->setBuddy(numprograms);

    QLabel *labelNets = new QLabel(tr("Fidonet &nets represented"), grid);
    numnets = new QLineEdit(grid, "numnets");
    numnets->setReadOnly(true);
    numnets->setAlignment(AlignRight);
    labelNets->setBuddy(numnets);

    QLabel *labelDomains = new QLabel(tr("Top &domains represented"), grid);
    numdomains = new QLineEdit(grid, "numdomains");
    numdomains->setReadOnly(true);
    numdomains->setAlignment(AlignRight);
    labelDomains->setBuddy(numdomains);

    QLabel *labelEarliest = new QLabel(tr("&Earliest text written"), grid);
    earliestwritten = new QLineEdit(grid, "earliestwritten");
    earliestwritten->setReadOnly(true);
    earliestwritten->setAlignment(AlignRight);
    labelEarliest->setBuddy(earliestwritten);

    QLabel *labelLatest = new QLabel(tr("Latest text &written"), grid);
    latestwritten = new QLineEdit(grid, "latestwritten");
    latestwritten->setReadOnly(true);
    latestwritten->setAlignment(AlignRight);
    labelLatest->setBuddy(latestwritten);

    // Fill data fields with zeroes
    zeroFill();

    setCentralWidget(grid);

    connect(this, SIGNAL(newdata()), SLOT(update()));

    // Objects owned
    engine = new StatEngine;
    progressdialog = NULL;
    progresstext = NULL;

    // Reset start date
    start = time_t(0);
    end = time_t(DISTANT_FUTURE);
    daysback = 0;
};

InfoWindow::~InfoWindow()
{
    delete engine;
}

QProgressDialog *InfoWindow::getProgressDialog(int maximum)
{
    if (!progressdialog)
    {
        if (progresstext)
        {
            return NULL; // Error condition
        }

        progressdialog =
            new QProgressDialog(hasnews ? tr("Reading news group")
                                        : tr("Reading message base"),
                                0, maximum, this, "progress", true);
        progressdialog->setCaption("Turquoise SuperStat");
        progressdialog->setMinimumDuration(1000);
    }

    return progressdialog;
}

ProgressText *InfoWindow::getProgressText()
{
    if (!progresstext)
    {
        if (progressdialog)
        {
            return NULL; // Error condition
        }

        progresstext =
            new ProgressText(this, "progress2",
                             hasnews ? tr("Reading news group")
                                     : tr("Reading message base"));
        progresstext->setCaption("Turquoise SuperStat");
        progresstext->show();
    }

    return progresstext;
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
#define NUMFILTERS 9
    const QString filter[NUMFILTERS] =
    {
#define FTSCSDM 0
        tr("FTSC SDM (*.msg)"),
#define OPUSSDM 1
        tr("Opus SDM (*.msg)"),
#define SQUISH 2
        tr("Squish (*.sqd)"),
#define FDAPXW 3
        tr("FDAPX/w (msgstat.apx)"),
#define JAM 4
        tr("JAM (*.jdt)"),
#define SMB 5
        tr("SMB (*.shd)"),
#define MYPOINT 6
        tr("MyPoint (mypoint.*)"),
#define TANSTAAFL 7
        tr("Tanstaafl (msgstat.tfl)"),
#define USENET 8
        tr("News (.overview)")
    };

    QFileDialog filedialog(QString::null,
                           filter[0] + ";;" + filter[1] + ";;" +
                           filter[2] + ";;" + filter[3] + ";;" +
                           filter[4] + ";;" + filter[5] + ";;" +
                           filter[6] + ";;" + filter[7],
                           this, "fileselect", true);
    filedialog.setMode(QFileDialog::ExistingFile);
    filedialog.setCaption(tr("Open message base"));
    filedialog.setShowHiddenFiles(true);
    if (filedialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QString fileselect = filedialog.selectedFile();
    if (fileselect.isEmpty()) return;

    // Determine what type we selected
    int filternum = -1;
    for (int i = 0; i < NUMFILTERS; i ++)
    {
        if (filedialog.selectedFilter() == filter[i])
        {
            filternum = i;
        }
    }
    if (-1 == filternum)
    {
        QMessageBox::warning(this, "Turquoise SuperStat",
                             tr("I don't understand your selection"));
        return;
    }

    // Make sure we do not try to mix incompatible area types
    if ((hasany &&  hasnews && USENET != filternum) ||
        (hasany && !hasnews && USENET == filternum))
    {
        incompatible();
        return;
    }

    // Setup
    AreaRead *area = NULL;
    // Remove file name if we only want the path
    if (FTSCSDM == filternum || OPUSSDM == filternum || FDAPXW == filternum ||
        MYPOINT == filternum || TANSTAAFL == filternum || USENET == filternum)
    {
#ifdef BACKSLASH_PATHS
        fileselect.truncate(fileselect.findRev(QRegExp("[/\\]"));
#else
        fileselect.truncate(fileselect.findRev('/'));
#endif
    }
    const char *path = fileselect.latin1();

    // Select area number
    int areanum = 0;
    if (FDAPXW == filternum || MYPOINT == filternum || TANSTAAFL == filternum)
    {
        bool ok;
        areanum = QInputDialog::getInteger(path,
                                           tr("Select area number"),
                                           0, 0, INT_MAX, 1, &ok, this,
                                           "areaselect");
        if (!ok)
        {
            // User pressed cancel
            return;
        }
    }

    bool isnews = false;

    // Do the magic selection
    switch (filternum)
    {
        case FTSCSDM:
        case OPUSSDM:
            area = new SdmRead(path, OPUSSDM == filternum);
            break;

        case SQUISH:
            area = new SquishRead(path);
            break;

        case FDAPXW:
            area = new FdApxRead(path, areanum);
            break;

        case JAM:
            area = new JamRead(path);
            break;

        case SMB:
            area = new SMBRead(path);
            break;

        case SMB:
            area = new SMBRead(path);
            break;

        case MYPOINT:
            area = new MyPointRead(path, areanum);
            break;

        case TANSTAAFL:
            area = new TanstaaflRead(path, areanum);
            break;

        case USENET:
            area = new NewsSpoolRead(path);
            isnews = true;
            break;
    }

    // Transfer data
    if (area)
    {
        transfer(area, isnews);
        delete area;
    }
    else
    {
        QMessageBox::warning(this, "Turquoise SuperStat",
                             tr("Out of memory allocating area object"));
    }

    // Update information boxes
    emit newdata();
}

void InfoWindow::opennews()
{
#if defined(HAVE_NNTP) // moc bug (cannot ifdef out a slot)
    // Select news server
    bool ok;
    QString server = QInputDialog::getText("Turquoise SuperStat",
                                           tr("Please enter the news (NNTP) server you want to use:"),
                                           QLineEdit::Normal, defaultserver,
                                           &ok, this, "serverselect");
    if (!ok)
    {
        // User pressed cancel
        return;
    }

    // Select group name
    QString group = QInputDialog::getText(server,
                                          tr("Please enter the name of the news group you want to read:"),
                                          QLineEdit::Normal, "",
                                          &ok, this, "groupselect");
    if (!ok)
    {
        // User pressed cancel
        return;
    }

    // Transfer data
    defaultserver = server;
    NntpRead *area = new NntpRead(server, group);
    if (area)
    {
        transfer(area, true);
        delete area;
    }
    else
    {
        QMessageBox::warning(this, "Turquoise SuperStat",
                             tr("Out of memory allocating area object"));
    }

    // Update information boxes
    emit newdata();
#endif
}

void InfoWindow::transfer(AreaRead *area, bool isnews)
{
    hasnews = isnews;
    hasany = true;

    area->Transfer(start, end, *engine);
    engine->AreaDone();

    delete progressdialog; progressdialog = NULL;
    delete progresstext;   progresstext   = NULL;
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
        menu->setItemEnabled(MENU_FIDOTOPLIST, false);
    }
    else
    {
        numnets->setText(QString::number(engine->GetTotalNets()));
    }
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
    BarWindow *timedialog =
        new BarWindow(this, "timedialog", BarWindow::Hours);
    if (timedialog)
    {
        timedialog->fillOut(engine);
        timedialog->show();
    }
}

void InfoWindow::daylist()
{
    BarWindow *daydialog =
        new BarWindow(this, "daydialog", BarWindow::Days);
    if (daydialog)
    {
        daydialog->fillOut(engine);
        daydialog->show();
    }
}

void InfoWindow::clear()
{
    // Remove the old statistics engine and create a new one
    if (engine) delete engine;
    engine = new StatEngine;

    hasnews = false;
    hasany = false;

    zeroFill();  
}

void InfoWindow::report()
{
    ReportSelectWindow *reportwindow =
        new ReportSelectWindow(this, "reportselect", engine);
    reportwindow->exec();
}

void InfoWindow::zeroFill()
{
    numareas->setText("0");
    numtexts->setText("0");
    numbytes->setText("0");
    numlines->setText("0");
    numqbytes->setText("0");
    numqlines->setText("0");
    numpeople->setText("0");
    numsubjects->setText("0");
    numprograms->setText("0");
    numnets->setText("0");
    numdomains->setText("0");
    earliestwritten->setText(tr("None loaded"));
    latestwritten->setText(tr("None loaded"));

    numnets->setEnabled(true);
    menu->setItemEnabled(MENU_FIDOTOPLIST, true);
}

void InfoWindow::startdate()
{
    bool ok = false;
    int days =
        QInputDialog::getInteger(tr("Start date"),
                                 tr("Enter number of days back to take "
                                    "statistics for (0 = no restriction):"),
                                 daysback, 0, INT_MAX, 1,
                                 &ok, this, "timeselect");
    if (ok)
    {
        // Compute starting time
        if (0 == days)
            start = 0;
        else
            start = time(NULL) - days * 86400L;
        struct tm *fromtm = localtime(&start);
        fromtm->tm_hour = 0;
        fromtm->tm_min  = 0;
        fromtm->tm_sec  = 0;
        start = my_mktime(fromtm);

        daysback = days;
    }
}

// Program entry
int main(int argc, char *argv[])
{
    // Create an application
    QApplication gui(argc, argv);

    // Support translations
    QString language;
    if (getenv("LANG"))
    {
        language = getenv("LANG");
    }
    else if (getenv("LC_MESSAGES"))
    {
        language = getenv("LC_MESSAGES");
    }
    else if (getenv("LC_ALL"))
    {
        language = getenv("LC_ALL");
    }

    QTranslator translator(NULL);
    if (!language.isEmpty())
    {
        translator.load(language, "/usr/share/xturqstat/locale");
        gui.installTranslator(&translator);
    }

    // Create a window
    InfoWindow *mainwindow = InfoWindow::getMainWindow();
    mainwindow->setCaption("Turquoise SuperStat");
    mainwindow->show();

    // Run, window, run
    gui.connect(&gui, SIGNAL(lastWindowClosed()), &gui, SLOT(quit()));
    return gui.exec();
}
