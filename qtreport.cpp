// Copyright (c) 2000-2008 Peter Karlsson
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

#include <qlayout.h>
#include <qfiledialog.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <limits.h>

#include "qtreport.h"
#include "statview.h"
#include "convert.h"
#include "template.h"
#include "output.h"

bool ReportSelectWindow::doquoters = true;
bool ReportSelectWindow::dotopwritten = true;
bool ReportSelectWindow::dotoporiginal = true;
bool ReportSelectWindow::dotopnets = true;
bool ReportSelectWindow::dotopdomains = true;
bool ReportSelectWindow::dotopreceived = true;
bool ReportSelectWindow::dotopsubjects = true;
bool ReportSelectWindow::dotopprograms = true;
bool ReportSelectWindow::doweekstats = true;
bool ReportSelectWindow::dodaystats = true;
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
bool ReportSelectWindow::douselocale = true;
#endif
QString ReportSelectWindow::docharset = "iso-8859-1";
int ReportSelectWindow::defaultmaxnum = 15;
QString ReportSelectWindow::templfile;
Template *ReportSelectWindow::templ = NULL;

ReportSelectWindow::ReportSelectWindow(QWidget *parent, const char *name,
                                       StatEngine *engine_p)
    : QDialog(parent, name, true), engine(engine_p)
{
    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Add checkboxes
    quoters =
        new QCheckBox(tr("Enable &blacklist of quoters"), this);
    topwritten =
        new QCheckBox(tr("Enable toplist of &writers"), this);
    toporiginal =
        new QCheckBox(tr("Enable toplist of &original content"), this);
    topnets =
        new QCheckBox(tr("Enable toplist of Fidonet &nets"), this);
    topdomains =
        new QCheckBox(tr("Enable toplist of Internet &domains"), this);
    topreceived =
        new QCheckBox(tr("Enable toplist of &receivers"), this);
    topsubjects =
        new QCheckBox(tr("Enable toplist of &subjects"), this);
    topprograms =
        new QCheckBox(tr("Enable toplist of &programs"), this);
    weekstats =
        new QCheckBox(tr("&Enable posting by weekday statistics"), this);
    daystats =
        new QCheckBox(tr("Enable posting by &hour statiscs"), this);
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    uselocale =
        new QCheckBox(tr("&Use locale date format"), this);
#endif

    // Check all checkboxes
    quoters->setChecked(doquoters);
    topwritten->setChecked(dotopwritten);
    toporiginal->setChecked(dotoporiginal);
    topnets->setChecked(dotopnets);
    topdomains->setChecked(dotopdomains);
    topreceived->setChecked(dotopreceived);
    topsubjects->setChecked(dotopsubjects);
    topprograms->setChecked(dotopprograms);
    weekstats->setChecked(doweekstats);
    daystats->setChecked(dodaystats);
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    uselocale->setChecked(douselocale);
#endif

    // Place in dialog
    layout->addWidget(quoters);
    layout->addWidget(topwritten);
    layout->addWidget(toporiginal);
    layout->addWidget(topnets);
    layout->addWidget(topdomains);
    layout->addWidget(topreceived);
    layout->addWidget(topsubjects);
    layout->addWidget(topprograms);
    layout->addWidget(weekstats);
    layout->addWidget(daystats);
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    layout->addWidget(uselocale);
#endif

    // Input boxes
    maxnum = new QSpinBox(1, INT_MAX, 1, this);
    maxnum->setValue(defaultmaxnum);
    maxnum->setSuffix(tr(" entries"));
    layout->addWidget(maxnum);

    QLabel *charsetlabel = new QLabel(tr("&Character set for report"), this);
    layout->addWidget(charsetlabel);

    charset = new QComboBox(false, this);
    layout->addWidget(charset);
    charsetlabel->setBuddy(charset);

    CharsetEnumerator charsets(CharsetEnumerator::Usenet);
    const char *charsetname;
    while (NULL != (charsetname = charsets.Next()))
    {
        charset->insertItem(charsetname);
        if (docharset == charsetname)
        {
            charset->setCurrentItem(charset->count() - 1);
        }
    }

	// Template file browser
	if (!templ)
	{
		// Load default template on first call
		string templatefile;
#if defined(HAVE_WIN32_GETMODULEFILENAME)
		// Windows: Template is in the same directory as the EXE file.
		char defaulttemplate[_MAX_PATH];
		GetModuleFileNameA(NULL, defaulttemplate, _MAX_PATH);
		char *p = strrchr(defaulttemplate, '\\');
		if (p && p < defaulttemplate + _MAX_PATH - 13)
		{
			strcpy(p + 1, "default.tpl");
			templatefile = defaulttemplate;
		}
#else
		// Unix-like environment: Template is in a pre-defined installation
		// directory.
		templatefile = DATA
#ifdef BACKSLASH_PATHS
				"\\"
#else
				"/"
#endif
				"default.tpl";
#endif

		bool is_error = false;
		templ = Template::Parse(templatefile, is_error);
		if (!is_error)
		{
			templfile = templatefile;
		}
	}
	QLabel *templatelabel = new QLabel(tr("Template file to use"), this);
	layout->addWidget(templatelabel);

	QHBoxLayout *filebrowselayout = new QHBoxLayout(layout);

	templatefilename = new QLineEdit(this, "templatefile");
	templatefilename->setReadOnly(true);
	templatefilename->setText(templfile);
	filebrowselayout->addWidget(templatefilename);
	filebrowsebutton = new QPushButton(tr("&Browse"), this);
	filebrowselayout->addWidget(filebrowsebutton);
	connect(filebrowsebutton, SIGNAL(clicked()), SLOT(browseForTemplate()));

    // Add buttons
    QHBoxLayout *buttonlayout = new QHBoxLayout(layout);

    QPushButton *save = new QPushButton(tr("&Save"), this);
    buttonlayout->addWidget(save);
    connect(save, SIGNAL(clicked()), SLOT(saveToFile()));

    QPushButton *ok = new QPushButton(tr("Cancel"), this);
    buttonlayout->addWidget(ok);
    connect(ok, SIGNAL(clicked()), SLOT(accept()));
}

ReportSelectWindow::~ReportSelectWindow()
{
}

void ReportSelectWindow::saveToFile()
{
	if (!templ)
	{
		TDisplay *display = TDisplay::GetOutputObject();
		display->ErrorMessage(TDisplay::template_parse_error);
	}

    // Browse for filename
    QString filename =
        QFileDialog::getSaveFileName(tr("report.txt"),
                                     tr("Report files (*.txt)"),
                                     this, "savereport", tr("Save report"));
    if (filename.isEmpty()) return; // Cancel

    // Create view object
    StatView view;

    // Save data
    doquoters = quoters->isChecked();
    dotopwritten = topwritten->isChecked();
    dotoporiginal = toporiginal->isChecked();
    dotopnets = topnets->isChecked();
    dotopdomains = topdomains->isChecked();
    dotopreceived = topreceived->isChecked();
    dotopsubjects = topsubjects->isChecked();
    dotopprograms = topprograms->isChecked();
    doweekstats = weekstats->isChecked();
    dodaystats = daystats->isChecked();
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    douselocale = uselocale->isChecked();
#endif
    defaultmaxnum = maxnum->value();
    docharset = charset->currentText();

    // Enable toplists we want
    view.EnableQuoters(doquoters);
    view.EnableTopWritten(dotopwritten);
    view.EnableTopOriginal(dotoporiginal);
    view.EnableTopNets(dotopnets);
    view.EnableTopDomains(dotopdomains);
    view.EnableTopReceived(dotopreceived);
    view.EnableTopSubjects(dotopsubjects);
    view.EnableTopPrograms(dotopprograms);
    view.EnableWeekStats(doweekstats);
    view.EnableDayStats(dodaystats);
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
    view.UseLocale(douselocale);
#endif

    // Set toplist parameters
    view.ShowVersions(true);
    view.ShowAllNums(false);
    view.SetMaxEntries(defaultmaxnum);
    view.SetCharset(docharset.latin1());

    // Write output
	view.SetTemplate(templ);
    view.CreateReport(engine, string(filename.local8Bit()));

    // Close
    accept();
}

void ReportSelectWindow::browseForTemplate()
{
	// Browse for filename
	QString filename =
		QFileDialog::getSaveFileName(tr("default.tpl"),
		                             tr("Template files (*.tpl)"),
		                             this, "browsefortemplate", tr("Browse for template"));
	if (filename.isEmpty()) return; // Cancel

	// Load template if possible
	bool error = false;
	Template *new_template =
		Template::Parse(string(filename.local8Bit()), error);
	if (!new_template || error)
	{
		TDisplay *display = TDisplay::GetOutputObject();
		display->ErrorMessage(TDisplay::template_parse_error);

		delete new_template;
	}
	else
	{
		// Remember template
		templ = new_template;
		templfile = filename;
		templatefilename->setText(templfile);
	}
}
