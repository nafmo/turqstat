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
#ifdef HAVE_WIN32_GETMODULEFILENAME
# include <windows.h>
#endif

#include "qtreport.h"
#include "statview.h"
#include "convert.h"
#include "template.h"
#include "output.h"

bool ReportSelectWindow::g_doquoters = true;
bool ReportSelectWindow::g_dotopwritten = true;
bool ReportSelectWindow::g_dotoporiginal = true;
bool ReportSelectWindow::g_dotopnets = true;
bool ReportSelectWindow::g_dotopdomains = true;
bool ReportSelectWindow::g_dotopreceived = true;
bool ReportSelectWindow::g_dotopsubjects = true;
bool ReportSelectWindow::g_dotopprograms = true;
bool ReportSelectWindow::g_doweekstats = true;
bool ReportSelectWindow::g_dodaystats = true;
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
bool ReportSelectWindow::g_douselocale = true;
#endif
QString ReportSelectWindow::g_docharset = "iso-8859-1";
int ReportSelectWindow::g_defaultmaxnum = 15;
QString ReportSelectWindow::g_templpath;
Template *ReportSelectWindow::g_template = NULL;

ReportSelectWindow::ReportSelectWindow(QWidget *parent, StatEngine *engine_p)
	: QDialog(parent), m_engine_p(engine_p)
{
    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Add checkboxes
	m_quoters_p =
		new QCheckBox(tr("Enable &blacklist of quoters"), this);
	m_topwritten_p =
		new QCheckBox(tr("Enable toplist of &writers"), this);
	m_toporiginal_p =
		new QCheckBox(tr("Enable toplist of &original content"), this);
	m_topnets_p =
		new QCheckBox(tr("Enable toplist of Fidonet &nets"), this);
	m_topdomains_p =
		new QCheckBox(tr("Enable toplist of Internet &domains"), this);
	m_topreceived_p =
		new QCheckBox(tr("Enable toplist of &receivers"), this);
	m_topsubjects_p =
		new QCheckBox(tr("Enable toplist of &subjects"), this);
	m_topprograms_p =
		new QCheckBox(tr("Enable toplist of &programs"), this);
	m_weekstats_p =
		new QCheckBox(tr("&Enable posting by weekday statistics"), this);
	m_daystats_p =
		new QCheckBox(tr("Enable posting by &hour statiscs"), this);
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
	m_uselocale_p =
		new QCheckBox(tr("&Use locale date format"), this);
#endif

    // Check all checkboxes
	m_quoters_p->setChecked(g_doquoters);
	m_topwritten_p->setChecked(g_dotopwritten);
	m_toporiginal_p->setChecked(g_dotoporiginal);
	m_topnets_p->setChecked(g_dotopnets);
	m_topdomains_p->setChecked(g_dotopdomains);
	m_topreceived_p->setChecked(g_dotopreceived);
	m_topsubjects_p->setChecked(g_dotopsubjects);
	m_topprograms_p->setChecked(g_dotopprograms);
	m_weekstats_p->setChecked(g_doweekstats);
	m_daystats_p->setChecked(g_dodaystats);
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
	m_uselocale_p->setChecked(g_douselocale);
#endif

    // Place in dialog
	layout->addWidget(m_quoters_p);
	layout->addWidget(m_topwritten_p);
	layout->addWidget(m_toporiginal_p);
	layout->addWidget(m_topnets_p);
	layout->addWidget(m_topdomains_p);
	layout->addWidget(m_topreceived_p);
	layout->addWidget(m_topsubjects_p);
	layout->addWidget(m_topprograms_p);
	layout->addWidget(m_weekstats_p);
	layout->addWidget(m_daystats_p);
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
	layout->addWidget(m_uselocale_p);
#endif

    // Input boxes
	m_maxnum_p = new QSpinBox(this);
	m_maxnum_p->setMinimum(1);
	m_maxnum_p->setMaximum(INT_MAX);
	m_maxnum_p->setSingleStep(1);
	m_maxnum_p->setValue(g_defaultmaxnum);
	m_maxnum_p->setSuffix(tr(" entries"));
	layout->addWidget(m_maxnum_p);

    QLabel *charsetlabel = new QLabel(tr("&Character set for report"), this);
    layout->addWidget(charsetlabel);

	m_charset_p = new QComboBox(this);
	layout->addWidget(m_charset_p);
	charsetlabel->setBuddy(m_charset_p);

    CharsetEnumerator charsets(CharsetEnumerator::Usenet);
	const char *charsetname_p;
	while (NULL != (charsetname_p = charsets.Next()))
	{
		m_charset_p->addItem(charsetname_p);
		if (g_docharset == charsetname_p)
		{
			m_charset_p->setCurrentIndex(m_charset_p->count() - 1);
		}
	}

	// Template file browser
	if (!g_template)
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
		g_template = Template::Parse(templatefile, is_error);
		if (!is_error)
		{
			g_templpath.fromStdString(templatefile);
		}
	}
	QLabel *templatelabel = new QLabel(tr("Template file to use"), this);
	layout->addWidget(templatelabel);

	QGridLayout *filebrowselayout_p = new QGridLayout(this);

	m_templatefilename_p = new QLineEdit(this);
	m_templatefilename_p->setReadOnly(true);
	m_templatefilename_p->setText(g_templpath);
	filebrowselayout_p->addWidget(m_templatefilename_p, 0, 0);
	m_filebrowsebutton_p = new QPushButton(tr("&Browse"), this);
	filebrowselayout_p->addWidget(m_filebrowsebutton_p, 0, 1);
	connect(m_filebrowsebutton_p, SIGNAL(clicked()), SLOT(browseForTemplate()));

    // Add buttons
    QPushButton *save = new QPushButton(tr("&Save"), this);
	filebrowselayout_p->addWidget(save, 1, 0);
    connect(save, SIGNAL(clicked()), SLOT(saveToFile()));

    QPushButton *ok = new QPushButton(tr("Cancel"), this);
	filebrowselayout_p->addWidget(ok, 1, 1);
    connect(ok, SIGNAL(clicked()), SLOT(accept()));
}

ReportSelectWindow::~ReportSelectWindow()
{
}

void ReportSelectWindow::saveToFile()
{
	if (!g_template)
	{
		TDisplay *display = TDisplay::GetOutputObject();
		display->ErrorMessage(TDisplay::template_parse_error);
	}

    // Browse for filename
	QString filename =
		QFileDialog::getSaveFileName(this, tr("Save report"),
		                             tr("report.txt"),
		                             tr("Report files (*.txt)"));
    if (filename.isEmpty()) return; // Cancel

    // Create view object
    StatView view;

    // Save data
	g_doquoters = m_quoters_p->isChecked();
	g_dotopwritten = m_topwritten_p->isChecked();
	g_dotoporiginal = m_toporiginal_p->isChecked();
	g_dotopnets = m_topnets_p->isChecked();
	g_dotopdomains = m_topdomains_p->isChecked();
	g_dotopreceived = m_topreceived_p->isChecked();
	g_dotopsubjects = m_topsubjects_p->isChecked();
	g_dotopprograms = m_topprograms_p->isChecked();
	g_doweekstats = m_weekstats_p->isChecked();
	g_dodaystats = m_daystats_p->isChecked();
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
	g_douselocale = m_uselocale_p->isChecked();
#endif
	g_defaultmaxnum = m_maxnum_p->value();
	g_docharset = m_charset_p->currentText();

    // Enable toplists we want
	view.EnableQuoters(g_doquoters);
	view.EnableTopWritten(g_dotopwritten);
	view.EnableTopOriginal(g_dotoporiginal);
	view.EnableTopNets(g_dotopnets);
	view.EnableTopDomains(g_dotopdomains);
	view.EnableTopReceived(g_dotopreceived);
	view.EnableTopSubjects(g_dotopsubjects);
	view.EnableTopPrograms(g_dotopprograms);
	view.EnableWeekStats(g_doweekstats);
	view.EnableDayStats(g_dodaystats);
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
	view.UseLocale(g_douselocale);
#endif

    // Set toplist parameters
    view.ShowVersions(true);
    view.ShowAllNums(false);
	view.SetMaxEntries(g_defaultmaxnum);
	view.SetCharset(g_docharset.toLatin1());

    // Write output
	view.SetTemplate(g_template);
	view.CreateReport(m_engine_p, string(filename.toLocal8Bit()));

    // Close
    accept();
}

void ReportSelectWindow::browseForTemplate()
{
	// Browse for filename
	QString filename =
		QFileDialog::getOpenFileName(this, tr("Browse for template"),
		                             g_templpath,
		                             tr("Template files (*.tpl)"));
	if (filename.isEmpty()) return; // Cancel

	// Load template if possible
	bool error = false;
	Template *new_template =
		Template::Parse(string(filename.toLocal8Bit()), error);
	if (!new_template || error)
	{
		TDisplay *display = TDisplay::GetOutputObject();
		display->ErrorMessage(TDisplay::template_parse_error);

		delete new_template;
	}
	else
	{
		// Remember template
		g_template = new_template;
		g_templpath = filename;
		m_templatefilename_p->setText(g_templpath);
	}
}
