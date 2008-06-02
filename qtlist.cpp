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

#include <qtablewidget.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include "qtlist.h"
#include "statengine.h"

TopListWindow::TopListWindow(QWidget *parent, toplist_e list)
    : QDialog(parent), toplist(list)
{
    // Create layout
	QGridLayout *layout_p = new QGridLayout(this);

    // Create list view
	m_tableview_p = new QTableWidget(this);
	layout_p->addWidget(m_tableview_p, 0, 0, 1, 2);

    // Add buttons
    QPushButton *ok = new QPushButton(tr("Dismiss"), this);
	ok->setDefault(true);
	layout_p->addWidget(ok, 1, 0);
    connect(ok, SIGNAL(clicked()), SLOT(accept()));

    QPushButton *save = new QPushButton(tr("&Save"), this);
	layout_p->addWidget(save, 1, 1);
    connect(save, SIGNAL(clicked()), SLOT(saveToFile()));
}

TopListWindow::~TopListWindow()
{
}

void TopListWindow::fillOut(StatEngine *engine)
{
	if (!engine || !m_tableview_p) return;

    setupHeaders();

    switch (toplist)
    {
        case Quoters:
			setWindowTitle(tr("Quoters"));
            addQuoters(engine);
            break;

        case Senders:
            setWindowTitle(tr("Senders"));
            addSenders(engine);
            break;

        case OrigContent:
            setWindowTitle(tr("Original content per message"));
            addOriginalContent(engine);
            break;

        case FidoNets:
            setWindowTitle(tr("Fidonet nets"));
            addFidoNets(engine);
            break;

        case Domains:
            setWindowTitle(tr("Internet topdomains"));
            addDomains(engine);
            break;

        case Receivers:
            setWindowTitle(tr("Receivers"));
            addReceivers(engine);
            break;

        case Subjects:
            setWindowTitle(tr("Subjects"));
            addSubjects(engine);
            break;

        case Software:
            setWindowTitle(tr("Software"));
            addSoftware(engine);
            break;
    }
}

void TopListWindow::SetItem(int row, int column, const QString &label)
{
	m_tableview_p->setItem(row, column, new QTableWidgetItem(label));
}

void TopListWindow::setupHeaders()
{
	QStringList headers;

	// Create headers, counting them as we go along
    if (Quoters == toplist || Senders == toplist || OrigContent == toplist ||
        Receivers == toplist)
		headers.append(tr("Name"));

    if (OrigContent == toplist)
		headers.append(tr("Original content"));

    if (FidoNets == toplist)
		headers.append(tr("Zone:Net"));

    if (Domains == toplist)
		headers.append(tr("Domain"));

    if (Subjects == toplist)
		headers.append(tr("Subject"));

    if (Software == toplist)
		headers.append(tr("Program"));

    if (Quoters == toplist || Senders == toplist || OrigContent == toplist ||
        FidoNets == toplist || Domains == toplist || Subjects == toplist ||
        Software == toplist)
		headers.append(tr("Messages"));

    if (Receivers == toplist)
    {
		headers.append(tr("Received"));
		headers.append(tr("Sent"));
    }

    if (Senders == toplist || FidoNets == toplist || Domains == toplist ||
        Subjects == toplist)
		headers.append(tr("Bytes"));

    if (Senders == toplist)
		headers.append(tr("Lines"));

    if (Quoters == toplist || Senders == toplist)
		headers.append(tr("Quote ratio"));

    if (Receivers == toplist)
		headers.append(tr("Ratio"));

    if (OrigContent == toplist)
		headers.append(tr("Per Message"));

	// Set up the tree view, and add the header
	m_tableview_p->setColumnCount(headers.size());
	m_tableview_p->setHorizontalHeaderLabels(headers);
}

void TopListWindow::addQuoters(StatEngine *engine)
{
    int row = 0;
    bool restart = true;

    StatEngine::persstat_s data;
    while (engine->GetTopQuoters(restart, data))
    {
        if (data.bytesquoted && data.byteswritten)
        {
            QString tmp;
            QString name = charConvert(data.name);
            if (name != QString(data.address.c_str()))
                tmp = name + " <" + data.address.c_str() + ">";
            else
                tmp = data.address.c_str();

			m_tableview_p->setRowCount(row + 1);
			SetItem(row, 0, tmp);
			SetItem(row, 1, QString::number(data.messageswritten));
			SetItem(row, 2, percentString(data.bytesquoted, data.byteswritten));

            ++ row;
        }
        restart = false;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addSenders(StatEngine *engine)
{
    int row = 0;
    bool restart = true;

    StatEngine::persstat_s data;
    while (engine->GetTopWriters(restart, data))
    {
        if (!data.messageswritten) break;

        QString tmp;
        QString name = charConvert(data.name);
        if (name != QString(data.address.c_str()))
            tmp = name + " <" + data.address.c_str() + ">";
        else
            tmp = data.address.c_str();

		m_tableview_p->setRowCount(row + 1);
		SetItem(row, 0, tmp);
		SetItem(row, 1, QString::number(data.messageswritten));
		SetItem(row, 2, QString::number(data.byteswritten));
		SetItem(row, 3, QString::number(data.lineswritten));
		SetItem(row, 4, (data.bytesquoted > 0
							? percentString(data.bytesquoted,
							                data.byteswritten)
							: tr("N/A")));

		++ row;
        restart = false;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addOriginalContent(StatEngine *engine)
{
    int row = 0;
    bool restart = true;

    StatEngine::persstat_s data;
    while (engine->GetTopOriginalPerMsg(restart, data))
    {
        restart = false;
        if (0 == data.messageswritten)
        {
            continue;
        }

        QString tmp;
        QString name = charConvert(data.name);
        if (name != QString(data.address.c_str()))
            tmp = name + " <" + data.address.c_str() + ">";
        else
            tmp = data.address.c_str();

        unsigned originalpermsg =
            (data.byteswritten - data.bytesquoted) / data.messageswritten;

		m_tableview_p->setRowCount(row + 1);
		SetItem(row, 0, tmp);
		SetItem(row, 1, QString::number(data.byteswritten - data.bytesquoted));
		SetItem(row, 2, QString::number(data.messageswritten));
		SetItem(row, 3, QString::number(originalpermsg));

		++ row;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addFidoNets(StatEngine *engine)
{
    int row = 0;
    bool restart = true;

    StatEngine::netstat_s data;
    while (engine->GetTopNets(restart, data))
    {
		m_tableview_p->setRowCount(row + 1);
		SetItem(row, 0, QString::number(data.zone) + ":" + QString::number(data.net));
		SetItem(row, 1, QString::number(data.messages));
		SetItem(row, 2, QString::number(data.bytes));

		++ row;
        restart = false;
    }

    engine->DoneTopNets();
}

void TopListWindow::addDomains(StatEngine *engine)
{
    int row = 0;
    bool restart = true;

    StatEngine::domainstat_s data;
    while (engine->GetTopDomains(restart, data))
    {
		m_tableview_p->setRowCount(row + 1);
		SetItem(row, 0, data.topdomain.c_str());
		SetItem(row, 1, QString::number(data.messages));
		SetItem(row, 2, QString::number(data.bytes));

		++ row;
        restart = false;
    }

    engine->DoneTopDomains();
}

void TopListWindow::addReceivers(StatEngine *engine)
{
    int row = 0;
    bool restart = true;

    StatEngine::persstat_s data;
    while (engine->GetTopReceivers(restart, data))
    {
        if (!data.messagesreceived) break;

        QString tmp;
        if (data.name.length())
            tmp = charConvert(data.name);
        else
            tmp = tr("(none)");

		m_tableview_p->setRowCount(row + 1);
		SetItem(row, 0, tmp);
		SetItem(row, 1, QString::number(data.messagesreceived));
		SetItem(row, 2, QString::number(data.messageswritten));
		SetItem(row, 3, (data.messageswritten
						 ? QString::number((100 * data.messagesreceived)
						                   / data.messageswritten) + "%"
						 : tr("N/A")));

		++ row;
        restart = false;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addSubjects(StatEngine *engine)
{
    int row = 0;
    bool restart = true;
    StatEngine::subjstat_s data;

    while (engine->GetTopSubjects(restart, data))
    {
        QString tmp;
        if (data.subject.length())
            tmp = charConvert(data.subject);
        else
            tmp = tr("(none)");

		m_tableview_p->setRowCount(row + 1);
		SetItem(row, 0, tmp);
		SetItem(row, 1, QString::number(data.count));
		SetItem(row, 2, QString::number(data.bytes));

		++ row;
        restart = false;
    }

    engine->DoneTopSubjects();
}

void TopListWindow::addSoftware(StatEngine *engine)
{
    int row = 0;
    bool restart = true;

    StatEngine::progstat_s data;
    while (engine->GetTopPrograms(restart, data))
    {
        QString tmp;
        if (data.program.length())
            tmp = charConvert(data.program);
        else
            tmp = tr("(none)");

		m_tableview_p->setRowCount(row + 1);
		SetItem(row, 0, tmp);
		SetItem(row, 1, QString::number(data.count));

		++ row;
        restart = false;
    }

    engine->DoneTopPrograms();
}

QString TopListWindow::charConvert(wstring &inputstring)
{
    int length = inputstring.length();
    QChar *tmp = new QChar[length];
    for (int i = 0; i < length; i ++)
    {
        tmp[i] = inputstring[i];
    }

    QString rc = QString(tmp, length);
    delete[] tmp;

    return rc;
}

QString TopListWindow::percentString(int numerator, int denumerator)
{
    float percent =
        (100.0 * numerator) / (float) denumerator;
    return QString::number(percent, 'f', 2) + "%";
}

void TopListWindow::saveToFile()
{
	int numentries = m_tableview_p->rowCount();
	int numcolumns = m_tableview_p->columnCount();
    if (0 == numentries || 0 == numcolumns)
    {
        QMessageBox::warning(this, "Turquoise SuperStat",
                             tr("This list is empty"));
        return;
    }

#define NUMFILTERS 4
    const QString filter[NUMFILTERS] =
    {
#define PLAINTEXT 0
        tr("Plain text (*.txt)"),
#define COMMA 1
        tr("Comma-separated file (*.csv)"),
#define SEMICOLON 2
        tr("Semi-colon separated file (*.csv)"),
#define TAB 3
        tr("Tab separated file (*.tsv)")
    };

	QFileDialog filedialog(this, tr("Save toplist"), QString(),
	                       filter[0] + ";;" + filter[1] + ";;" + filter[2] +
	                       ";;" + filter[3]);
    filedialog.setFileMode(QFileDialog::AnyFile);
    if (filedialog.exec() != QDialog::Accepted)
    {
        return;
    }

    int filternum = PLAINTEXT;
    for (int i = 0; i < NUMFILTERS; i ++)
    {
        if (filedialog.selectedFilter() == filter[i])
        {
            filternum = i;
            break;
        }
    }

    // Create the file
	QFile output(filedialog.selectedFiles().at(0));
	if (!output.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, filedialog.selectedFiles().at(0),
                             tr("Unable to create the selected file"));
        return;
    }

    // Select output format
    QString format;
	int *columnwidth_p = NULL;
    if (PLAINTEXT == filternum)
    {
        // TODO: Give the user some option to edit this...
		columnwidth_p = new int[numcolumns - 1];
        for (int i = 0; i < numcolumns; i ++)
        {
			columnwidth_p[i] = 1;
        }
        // Traverse the list and find the widest element of each column
        for (int i = 0; i < numentries; i ++)
        {
			for (int j = 0; j < numcolumns - 1; j ++)
			{
				QTableWidgetItem *current_p = m_tableview_p->itemAt(i, j);
				if (current_p)
				{
					if (current_p->text().length() >= columnwidth_p[j])
					{
						columnwidth_p[j] = current_p->text().length() + 1;
					}
				}
			}
        }
    }

    QTextStream out(&output);

    // Print the titles
    for (int i = 0; i < numcolumns; i ++)
    {
		QTableWidgetItem *current_p = m_tableview_p->horizontalHeaderItem(i);
		if (!current_p)
			continue;

		QString s = current_p->text();
        if (i < numcolumns - 1)
        {
            switch (filternum)
            {
                case PLAINTEXT:
					if (s.length() > columnwidth_p[i])
                    {
						s.truncate(columnwidth_p[i] - 1);
                        s += '.';
                    }
                    else
                    {
						s = s.leftJustified(columnwidth_p[i], ' ', true);
                    }
                    break;

                case COMMA:
                    s += ',';
                    break;

                case SEMICOLON:
                    s += ';';
                    break;

                case TAB:
                    s += '\t';
                    break;
            }
        }
        out << s;
    }
    out << endl;

    // Print the entries
    for (int i = 0; i < numentries; i ++)
    {
        for (int j = 0; j < numcolumns; j ++)
        {
			QTableWidgetItem *current_p = m_tableview_p->itemAt(i, j);
			if (!current_p)
				continue;

			QString s = current_p->text();

            if (j < numcolumns - 1)
            {
                switch (filternum)
                {
                    case PLAINTEXT:
						s = s.leftJustified(columnwidth_p[j], ' ', true);
                        break;

                    case COMMA:
                        s.replace(QRegExp(","), ";");
                        s += ',';
                        break;

                    case SEMICOLON:
                        s.replace(QRegExp(";"), ",");
                        s += ';';
                        break;

                    case TAB:
                        s.replace(QRegExp("\t"), " ");
                        s += '\t';
                        break;
                }
            }
            out << s;
        }
        out << endl;
    }

    // Done
    output.close();
}
