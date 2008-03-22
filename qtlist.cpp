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

#include <qtreewidget.h>
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
	m_treeview_p = new QTreeWidget(this);
	layout_p->addWidget(m_treeview_p, 0, 0, 0, 2);

    // Add buttons
    QPushButton *ok = new QPushButton(tr("Dismiss"), this);
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
	if (!engine || !m_treeview_p) return;

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

void TopListWindow::setupHeaders()
{
	int column = 0;
	QTreeWidgetItem *header_p = new QTreeWidgetItem(m_treeview_p);
	if (!header_p)
		return;

	// Create headers, counting them as we go along
    if (Quoters == toplist || Senders == toplist || OrigContent == toplist ||
        Receivers == toplist)
		header_p->setText(column ++, tr("Name"));

    if (OrigContent == toplist)
		header_p->setText(column ++, tr("Original content"));

    if (FidoNets == toplist)
		header_p->setText(column ++, tr("Zone:Net"));

    if (Domains == toplist)
		header_p->setText(column ++, tr("Domain"));

    if (Subjects == toplist)
		header_p->setText(column ++, tr("Subject"));

    if (Software == toplist)
		header_p->setText(column ++, tr("Program"));

    if (Quoters == toplist || Senders == toplist || OrigContent == toplist ||
        FidoNets == toplist || Domains == toplist || Subjects == toplist ||
        Software == toplist)
		header_p->setText(column ++, tr("Messages"));

    if (Receivers == toplist)
    {
		header_p->setText(column ++, tr("Received"));
		header_p->setText(column ++, tr("Sent"));
    }

    if (Senders == toplist || FidoNets == toplist || Domains == toplist ||
        Subjects == toplist)
		header_p->setText(column ++, tr("Bytes"));

    if (Senders == toplist)
		header_p->setText(column ++, tr("Lines"));

    if (Quoters == toplist || Senders == toplist)
		header_p->setText(column ++, tr("Quote ratio"));

    if (Receivers == toplist)
		header_p->setText(column ++, tr("Ratio"));

    if (OrigContent == toplist)
		header_p->setText(column ++, tr("Per Message"));

	// Set up the tree view, and add the header
	m_treeview_p->setColumnCount(column);
	m_treeview_p->setHeaderItem(header_p);
}

void TopListWindow::addQuoters(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
	QTreeWidgetItem *previous_p = NULL;

    StatEngine::persstat_s data;
    while (engine->GetTopQuoters(restart, data))
    {
        if (data.bytesquoted && data.byteswritten)
        {
            QString tmp;
			QString name = QString::fromStdWString(data.name);
            if (name != QString(data.address.c_str()))
                tmp = name + " <" + data.address.c_str() + ">";
            else
                tmp = data.address.c_str();

			QTreeWidgetItem *item_p = new QTreeWidgetItem(m_treeview_p, previous_p);
			item_p->setText(0, QString::number(place ++));
			item_p->setText(1, tmp);
			item_p->setText(2, QString::number(data.messageswritten));
			item_p->setText(3, percentString(data.bytesquoted, data.byteswritten));

            previous_p = item_p;
        }
        restart = false;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addSenders(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
	QTreeWidgetItem *previous_p = NULL;

    StatEngine::persstat_s data;
    while (engine->GetTopWriters(restart, data))
    {
        if (!data.messageswritten) break;

        QString tmp;
		QString name = QString::fromStdWString(data.name);
        if (name != QString(data.address.c_str()))
            tmp = name + " <" + data.address.c_str() + ">";
        else
            tmp = data.address.c_str();

		QTreeWidgetItem *item_p = new QTreeWidgetItem(m_treeview_p, previous_p);
		item_p->setText(0, QString::number(place ++));
		item_p->setText(1, tmp);
		item_p->setText(2, QString::number(data.messageswritten));
		item_p->setText(3, QString::number(data.byteswritten));
		item_p->setText(4, QString::number(data.lineswritten));
		item_p->setText(5, (data.bytesquoted > 0
								? percentString(data.bytesquoted,
								                data.byteswritten)
								: tr("N/A")));

		previous_p = item_p;
        restart = false;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addOriginalContent(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QTreeWidgetItem *previous_p = NULL;

    StatEngine::persstat_s data;
    while (engine->GetTopOriginalPerMsg(restart, data))
    {
        restart = false;
        if (0 == data.messageswritten)
        {
            continue;
        }

        QString tmp;
		QString name = QString::fromStdWString(data.name);
        if (name != QString(data.address.c_str()))
            tmp = name + " <" + data.address.c_str() + ">";
        else
            tmp = data.address.c_str();

        unsigned originalpermsg =
            (data.byteswritten - data.bytesquoted) / data.messageswritten;

		QTreeWidgetItem *item_p = new QTreeWidgetItem(m_treeview_p, previous_p);
		item_p->setText(0, QString::number(place ++));
		item_p->setText(1, tmp);
		item_p->setText(2, QString::number(data.byteswritten - data.bytesquoted));
		item_p->setText(3, QString::number(data.messageswritten));
		item_p->setText(4, QString::number(originalpermsg));

		previous_p = item_p;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addFidoNets(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QTreeWidgetItem *previous_p = NULL;

    StatEngine::netstat_s data;
    while (engine->GetTopNets(restart, data))
    {
		QTreeWidgetItem *item_p = new QTreeWidgetItem(m_treeview_p, previous_p);
		item_p->setText(0, QString::number(place ++));
		item_p->setText(1, QString::number(data.zone) + ":" + QString::number(data.net));
		item_p->setText(2, QString::number(data.messages));
		item_p->setText(3, QString::number(data.bytes));

		previous_p = item_p;
        restart = false;
    }

    engine->DoneTopNets();
}

void TopListWindow::addDomains(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QTreeWidgetItem *previous_p = NULL;

    StatEngine::domainstat_s data;
    while (engine->GetTopDomains(restart, data))
    {
		QTreeWidgetItem *item_p = new QTreeWidgetItem(m_treeview_p, previous_p);
		item_p->setText(0, QString::number(place ++));
		item_p->setText(1, data.topdomain.c_str());
		item_p->setText(2, QString::number(data.messages));
		item_p->setText(3, QString::number(data.bytes));

		previous_p = item_p;
        restart = false;
    }

    engine->DoneTopDomains();
}

void TopListWindow::addReceivers(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QTreeWidgetItem *previous_p = NULL;

    StatEngine::persstat_s data;
    while (engine->GetTopReceivers(restart, data))
    {
        if (!data.messagesreceived) break;

        QString tmp;
        if (data.name.length())
			tmp = QString::fromStdWString(data.name);
        else
            tmp = tr("(none)");

		QTreeWidgetItem *item_p = new QTreeWidgetItem(m_treeview_p, previous_p);
		item_p->setText(0, QString::number(place ++));
		item_p->setText(1, tmp);
		item_p->setText(2, QString::number(data.messagesreceived));
		item_p->setText(3, QString::number(data.messageswritten));
		item_p->setText(4, (data.messageswritten
								? QString::number((100 * data.messagesreceived)
								                  / data.messageswritten) + "%"
								: tr("N/A")));

		previous_p = item_p;
        restart = false;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addSubjects(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    StatEngine::subjstat_s data;
    QTreeWidgetItem *previous_p = NULL;

    while (engine->GetTopSubjects(restart, data))
    {
        QString tmp;
        if (data.subject.length())
			tmp = QString::fromStdWString(data.subject);
        else
            tmp = tr("(none)");

		QTreeWidgetItem *item_p = new QTreeWidgetItem(m_treeview_p, previous_p);
		item_p->setText(0, QString::number(place ++));
		item_p->setText(1, tmp);
		item_p->setText(2, QString::number(data.count));
		item_p->setText(3, QString::number(data.bytes));

		previous_p = item_p;
        restart = false;
    }

    engine->DoneTopSubjects();
}

void TopListWindow::addSoftware(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QTreeWidgetItem *previous_p = NULL;

    StatEngine::progstat_s data;
    while (engine->GetTopPrograms(restart, data))
    {
        QString tmp;
        if (data.program.length())
			tmp = QString::fromStdWString(data.program);
        else
            tmp = tr("(none)");

		QTreeWidgetItem *item_p = new QTreeWidgetItem(m_treeview_p, previous_p);
		item_p->setText(0, QString::number(place ++));
		item_p->setText(1, tmp);
		item_p->setText(2, QString::number(data.count));

		previous_p = item_p;
        restart = false;
    }

    engine->DoneTopPrograms();
}

QString TopListWindow::percentString(int numerator, int denumerator)
{
    float percent =
        (100.0 * numerator) / (float) denumerator;
    return QString::number(percent, 'f', 2) + "%";
}

void TopListWindow::saveToFile()
{
	int numentries = m_treeview_p->topLevelItemCount();
	int numcolumns = m_treeview_p->columnCount();
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
			QTreeWidgetItem *current_p = m_treeview_p->topLevelItem(i);
			if (current_p)
			{
				for (int j = 0; j < numcolumns - 1; j ++)
				{
					if (current_p->text(j).length() >= columnwidth_p[j])
					{
						columnwidth_p[j] = current_p->text(j).length() + 1;
					}
				}
			}
        }
    }

    QTextStream out(&output);

    // Print the titles
	QTreeWidgetItem *header_p = m_treeview_p->headerItem();
    for (int i = 0; i < numcolumns; i ++)
    {
		QString s = header_p->text(i);
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
		QTreeWidgetItem *current_p = m_treeview_p->topLevelItem(i);
		if (!current_p)
			continue;

        for (int j = 0; j < numcolumns; j ++)
        {
			QString s = current_p->text(j);

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
