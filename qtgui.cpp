// Turquoise SuperStat
//
// A statistic collection program for Fidonet and Usenet systems
// Qt version.
//
// Copyright (c) 2000-2008 Peter Krefting
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
	: m_hasnews(false), m_hasany(false)
{
    // Construct menu
	m_menu_p = new QMenuBar(this);
	QPopupMenu *filemenu = new QPopupMenu(m_menu_p);
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
	m_menu_p->insertItem(tr("&File"), filemenu);

	QPopupMenu *editmenu = new QPopupMenu(m_menu_p);
    editmenu->insertItem(tr("&Set start date"), this, SLOT(startdate()),
                         CTRL+Key_Home);
	m_menu_p->insertItem(tr("&Edit"), editmenu);

	QPopupMenu *showmenu = new QPopupMenu(m_menu_p);
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
	m_menu_p->insertItem(tr("&Show"), showmenu);

    // Add information boxes
    QGrid *grid = new QGrid(2, this);
    grid->setSpacing(5);
    grid->setMargin(5);

    QLabel *labelAreas = new QLabel(tr("&Areas loaded"), grid);
	m_numareas_p = new QLineEdit(grid, "numareas");
	m_numareas_p->setReadOnly(true);
	m_numareas_p->setAlignment(AlignRight);
	labelAreas->setBuddy(m_numareas_p);

    QLabel *labelTexts = new QLabel(tr("&Texts examined"), grid);
	m_numtexts_p = new QLineEdit(grid, "numtexts");
	m_numtexts_p->setReadOnly(true);
	m_numtexts_p->setAlignment(AlignRight);
	labelTexts->setBuddy(m_numtexts_p);

    QLabel *labelBytes = new QLabel(tr("&Bytes written"), grid);
	m_numbytes_p = new QLineEdit(grid, "numbytes");
	m_numbytes_p->setReadOnly(true);
	m_numbytes_p->setAlignment(AlignRight);
	labelBytes->setBuddy(m_numbytes_p);

    QLabel *labelLines = new QLabel(tr("&Lines examined"), grid);
	m_numlines_p = new QLineEdit(grid, "numlines");
	m_numlines_p->setReadOnly(true);
	m_numlines_p->setAlignment(AlignRight);
	labelLines->setBuddy(m_numlines_p);

    QLabel *labelQBytes = new QLabel(tr("Bytes &quoted"), grid);
	m_numqbytes_p = new QLineEdit(grid, "numqbytes");
	m_numqbytes_p->setReadOnly(true);
	m_numqbytes_p->setAlignment(AlignRight);
	labelQBytes->setBuddy(m_numqbytes_p);

    QLabel *labelQLines = new QLabel(tr("L&ines quoted"), grid);
	m_numqlines_p = new QLineEdit(grid, "numqlines");
	m_numqlines_p->setReadOnly(true);
	m_numqlines_p->setAlignment(AlignRight);
	labelQLines->setBuddy(m_numqlines_p);

    QLabel *labelPeople = new QLabel(tr("&People identified"), grid);
	m_numpeople_p = new QLineEdit(grid, "numpeople");
	m_numpeople_p->setReadOnly(true);
	m_numpeople_p->setAlignment(AlignRight);
	labelPeople->setBuddy(m_numpeople_p);

    QLabel *labelSubjects = new QLabel(tr("S&ubjects found"), grid);
	m_numsubjects_p = new QLineEdit(grid, "subjects");
	m_numsubjects_p->setReadOnly(true);
	m_numsubjects_p->setAlignment(AlignRight);
	labelSubjects->setBuddy(m_numsubjects_p);

    QLabel *labelPrograms = new QLabel(tr("P&rograms used"), grid);
	m_numprograms_p = new QLineEdit(grid, "numprograms");
	m_numprograms_p->setReadOnly(true);
	m_numprograms_p->setAlignment(AlignRight);
	labelPrograms->setBuddy(m_numprograms_p);

    QLabel *labelNets = new QLabel(tr("Fidonet &nets represented"), grid);
	m_numnets_p = new QLineEdit(grid, "numnets");
	m_numnets_p->setReadOnly(true);
	m_numnets_p->setAlignment(AlignRight);
	labelNets->setBuddy(m_numnets_p);

    QLabel *labelDomains = new QLabel(tr("Top &domains represented"), grid);
	m_numdomains_p = new QLineEdit(grid, "numdomains");
	m_numdomains_p->setReadOnly(true);
	m_numdomains_p->setAlignment(AlignRight);
	labelDomains->setBuddy(m_numdomains_p);

    QLabel *labelEarliest = new QLabel(tr("&Earliest text written"), grid);
	m_earliestwritten_p = new QLineEdit(grid, "earliestwritten");
	m_earliestwritten_p->setReadOnly(true);
	m_earliestwritten_p->setAlignment(AlignRight);
	labelEarliest->setBuddy(m_earliestwritten_p);

    QLabel *labelLatest = new QLabel(tr("Latest text &written"), grid);
	m_latestwritten_p = new QLineEdit(grid, "latestwritten");
	m_latestwritten_p->setReadOnly(true);
	m_latestwritten_p->setAlignment(AlignRight);
	labelLatest->setBuddy(m_latestwritten_p);

    // Fill data fields with zeroes
    zeroFill();

    setCentralWidget(grid);

    connect(this, SIGNAL(newdata()), SLOT(update()));

    // Objects owned
	m_engine_p = new StatEngine;
	m_progressdialog_p = NULL;
	m_progresstext_p = NULL;

    // Reset start date
	m_start = time_t(0);
	m_end = time_t(DISTANT_FUTURE);
	m_daysback = 0;
};

InfoWindow::~InfoWindow()
{
	delete m_engine_p;
}

QProgressDialog *InfoWindow::getProgressDialog(int maximum)
{
	if (!m_progressdialog_p)
    {
		if (m_progresstext_p)
        {
            return NULL; // Error condition
        }

		m_progressdialog_p =
			new QProgressDialog(m_hasnews ? tr("Reading news group")
			                              : tr("Reading message base"),
                                0, maximum, this, "progress", true);
		m_progressdialog_p->setCaption("Turquoise SuperStat");
		m_progressdialog_p->setMinimumDuration(1000);
    }

	return m_progressdialog_p;
}

ProgressText *InfoWindow::getProgressText()
{
	if (!m_progresstext_p)
    {
		if (m_progressdialog_p)
        {
            return NULL; // Error condition
        }

		m_progresstext_p =
            new ProgressText(this, "progress2",
			                 m_hasnews ? tr("Reading news group")
			                           : tr("Reading message base"));
		m_progresstext_p->setCaption("Turquoise SuperStat");
		m_progresstext_p->show();
    }

	return m_progresstext_p;
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
#define NUMFILTERS 8
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
#define MYPOINT 5
        tr("MyPoint (mypoint.*)"),
#define TANSTAAFL 6
        tr("Tanstaafl (msgstat.tfl)"),
#define USENET 7
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
	if ((m_hasany &&  m_hasnews && USENET != filternum) ||
	    (m_hasany && !m_hasnews && USENET == filternum))
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
	                                       QLineEdit::Normal, m_defaultserver,
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
	m_defaultserver = server;
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
	m_hasnews = isnews;
	m_hasany = true;

	area->Transfer(m_start, m_end, *m_engine_p);
	m_engine_p->AreaDone();

	delete m_progressdialog_p; m_progressdialog_p = NULL;
	delete m_progresstext_p;   m_progresstext_p   = NULL;
}

void InfoWindow::incompatible()
{
    QMessageBox::warning(this, "Turquoise SuperStat",
                         tr("You cannot mix Fidonet and Usenet areas"));
}

void InfoWindow::update()
{
    // Update information in all boxes

	m_numareas_p->setText(QString::number(m_engine_p->GetTotalAreas()));
	m_numtexts_p->setText(QString::number(m_engine_p->GetTotalNumber()));
	m_numbytes_p->setText(QString::number(m_engine_p->GetTotalBytes()));
	m_numlines_p->setText(QString::number(m_engine_p->GetTotalLines()));
	m_numqbytes_p->setText(QString::number(m_engine_p->GetTotalQBytes()));
	m_numqlines_p->setText(QString::number(m_engine_p->GetTotalQLines()));
	m_numpeople_p->setText(QString::number(m_engine_p->GetTotalPeople()));
	m_numsubjects_p->setText(QString::number(m_engine_p->GetTotalSubjects()));
	m_numprograms_p->setText(QString::number(m_engine_p->GetTotalPrograms()));
	if (m_hasnews)
    {
		m_numnets_p->setEnabled(false);
		m_numnets_p->setText(tr("N/A"));
		m_menu_p->setItemEnabled(MENU_FIDOTOPLIST, false);
    }
    else
    {
		m_numnets_p->setText(QString::number(m_engine_p->GetTotalNets()));
    }
	m_numdomains_p->setText(QString::number(m_engine_p->GetTotalDomains()));

    char timebuf[64];
	time_t earliest = m_engine_p->GetEarliestWritten();
    struct tm *p1 = localtime(&earliest);
    strftime(timebuf, 64, "%x", p1);
	m_earliestwritten_p->setText(timebuf);

	time_t latest = m_engine_p->GetLastWritten();
    struct tm *p2 = localtime(&latest);
    strftime(timebuf, 64, "%x", p2);
	m_latestwritten_p->setText(timebuf);
}

void InfoWindow::quotelist()
{
    TopListWindow *quotedialog =
        new TopListWindow(this, "quotelist", TopListWindow::Quoters);
    if (quotedialog)
    {
		quotedialog->fillOut(m_engine_p);
        quotedialog->show();
    }
}

void InfoWindow::senderlist()
{
    TopListWindow *senderdialog =
        new TopListWindow(this, "senderlist", TopListWindow::Senders);
    if (senderdialog)
    {
		senderdialog->fillOut(m_engine_p);
        senderdialog->show();
    }
}

void InfoWindow::contentlist()
{
    TopListWindow *contentdialog =
        new TopListWindow(this, "contentlist", TopListWindow::OrigContent);
    if (contentdialog)
    {
		contentdialog->fillOut(m_engine_p);
        contentdialog->show();
    }
}

void InfoWindow::fidonetlist()
{
    TopListWindow *fidonetsdialog =
        new TopListWindow(this, "netlist", TopListWindow::FidoNets);
    if (fidonetsdialog)
    {
		fidonetsdialog->fillOut(m_engine_p);
        fidonetsdialog->show();
    }
}

void InfoWindow::domainlist()
{
    TopListWindow *domaindialog =
        new TopListWindow(this, "domainlist", TopListWindow::Domains);
    if (domaindialog)
    {
		domaindialog->fillOut(m_engine_p);
        domaindialog->show();
    }
}

void InfoWindow::receiverlist()
{
    TopListWindow *receiverdialog =
        new TopListWindow(this, "receiverlist", TopListWindow::Receivers);
    if (receiverdialog)
    {
		receiverdialog->fillOut(m_engine_p);
        receiverdialog->show();
    }
}

void InfoWindow::subjectlist()
{
    TopListWindow *subjectdialog =
        new TopListWindow(this, "subjectlist", TopListWindow::Subjects);
    if (subjectdialog)
    {
		subjectdialog->fillOut(m_engine_p);
        subjectdialog->show();
    }
}

void InfoWindow::softwarelist()
{
    TopListWindow *softwaredialog =
        new TopListWindow(this, "softwaredialog", TopListWindow::Software);
    if (softwaredialog)
    {
		softwaredialog->fillOut(m_engine_p);
        softwaredialog->show();
    }
}

void InfoWindow::timelist()
{
    BarWindow *timedialog =
        new BarWindow(this, "timedialog", BarWindow::Hours);
    if (timedialog)
    {
		timedialog->fillOut(m_engine_p);
        timedialog->show();
    }
}

void InfoWindow::daylist()
{
    BarWindow *daydialog =
        new BarWindow(this, "daydialog", BarWindow::Days);
    if (daydialog)
    {
		daydialog->fillOut(m_engine_p);
        daydialog->show();
    }
}

void InfoWindow::clear()
{
    // Remove the old statistics engine and create a new one
	if (m_engine_p) delete m_engine_p;
	m_engine_p = new StatEngine;

	m_hasnews = false;
	m_hasany = false;

    zeroFill();  
}

void InfoWindow::report()
{
    ReportSelectWindow *reportwindow =
		new ReportSelectWindow(this, "reportselect", m_engine_p);
    reportwindow->exec();
}

void InfoWindow::zeroFill()
{
	m_numareas_p->setText("0");
	m_numtexts_p->setText("0");
	m_numbytes_p->setText("0");
	m_numlines_p->setText("0");
	m_numqbytes_p->setText("0");
	m_numqlines_p->setText("0");
	m_numpeople_p->setText("0");
	m_numsubjects_p->setText("0");
	m_numprograms_p->setText("0");
	m_numnets_p->setText("0");
	m_numdomains_p->setText("0");
	m_earliestwritten_p->setText(tr("None loaded"));
	m_latestwritten_p->setText(tr("None loaded"));

	m_numnets_p->setEnabled(true);
	m_menu_p->setItemEnabled(MENU_FIDOTOPLIST, true);
}

void InfoWindow::startdate()
{
    bool ok = false;
    int days =
        QInputDialog::getInteger(tr("Start date"),
                                 tr("Enter number of days back to take "
                                    "statistics for (0 = no restriction):"),
		                         m_daysback, 0, INT_MAX, 1,
                                 &ok, this, "timeselect");
    if (ok)
    {
        // Compute starting time
        if (0 == days)
			m_start = 0;
        else
			m_start = time(NULL) - days * 86400L;
		struct tm *fromtm = localtime(&m_start);
        fromtm->tm_hour = 0;
        fromtm->tm_min  = 0;
        fromtm->tm_sec  = 0;
		m_start = my_mktime(fromtm);

		m_daysback = days;
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
