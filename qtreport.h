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

#ifndef QTREPORT_H
#define QTREPORT_H

#include <qdialog.h>

class StatEngine;
class QCheckBox;
class QSpinBox;
class QComboBox;
class QLineEdit;
class QPushButton;
class Template;

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
    StatEngine *m_engine_p;

    // Checkboxes
	QCheckBox *m_quoters_p;
	QCheckBox *m_topwritten_p;
	QCheckBox *m_toporiginal_p;
	QCheckBox *m_topnets_p;
	QCheckBox *m_topdomains_p;
	QCheckBox *m_topreceived_p;
	QCheckBox *m_topsubjects_p;
	QCheckBox *m_topprograms_p;
	QCheckBox *m_weekstats_p;
	QCheckBox *m_daystats_p;
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
	QCheckBox *m_uselocale_p;
#endif

    // Input boxes
	QSpinBox *m_maxnum_p;
    QComboBox *m_charset_p;
	QLineEdit *m_templatefilename_p;
	QPushButton *m_filebrowsebutton_p;

protected slots:
    /**
     * Slot for actually creating the report file. Called when the save
     * button is pressed.
     */
    void saveToFile();
	/**
	 * Slot for browsing for a template file.
	 */
	void browseForTemplate();

private:
    // Static members to remember values
	static bool g_doquoters;	///< Remember if we want quoter toplist.
	static bool g_dotopwritten;	///< Remember if we want sender toplist.
	static bool g_dotoporiginal;///< Remember if we want original content
                                ///< toplist.
	static bool g_dotopnets;	///< Remember if we want Fidonet toplist.
	static bool g_dotopdomains;	///< Remember if we want topdomain toplist.
	static bool g_dotopreceived;///< Remember if we want receiver toplist.
	static bool g_dotopsubjects;///< Remember if we want subjects toplist.
	static bool g_dotopprograms;///< Remember if we want programs toplist.
	static bool g_doweekstats;	///< Remember if we want weekday statistics.
	static bool g_dodaystats;	///< Remember if we want hour statistics.
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
	static bool g_douselocale;	///< Remember if we want local date formats.
#endif

	static int g_defaultmaxnum;	///< Remember number of entries in toplists.
	static QString g_docharset;	///< Remember character set to use.
	static QString g_templpath;	///< Remember path to template file.
	static Template *g_template;///< Remember parsed template file.
};

#endif
