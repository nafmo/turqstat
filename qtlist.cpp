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

#include <qlistview.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>

#include "qtlist.h"
#include "statengine.h"

TopListWindow::TopListWindow(QWidget *parent, const char *name, toplist_e list)
    : QDialog(parent, name), toplist(list)
{
    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Create list view
    listview = new QListView(this, "toplistview");
    listview->setSorting(-1);
    layout->addWidget(listview);

    // Add buttons
    QHBoxLayout *buttonlayout = new QHBoxLayout(layout);
    
    QPushButton *ok = new QPushButton(tr("Dismiss"), this);
    buttonlayout->addWidget(ok);
    connect(ok, SIGNAL(clicked()), SLOT(accept()));

    QPushButton *save = new QPushButton(tr("&Save"), this);
    buttonlayout->addWidget(save);
    connect(save, SIGNAL(clicked()), SLOT(saveToFile()));
}

TopListWindow::~TopListWindow()
{
}

void TopListWindow::fillOut(StatEngine *engine)
{
    if (!engine || !listview) return;

    setupHeaders();

    switch (toplist)
    {
        case Quoters:
            setCaption(tr("Quoters"));
            addQuoters(engine);
            break;

        case Senders:
            setCaption(tr("Senders"));
            addSenders(engine);
            break;

        case OrigContent:
            setCaption(tr("Original content per message"));
            addOriginalContent(engine);
            break;

        case FidoNets:
            setCaption(tr("Fidonet nets"));
            addFidoNets(engine);
            break;

        case Domains:
            setCaption(tr("Internet topdomains"));
            addDomains(engine);
            break;

        case Receivers:
            setCaption(tr("Receivers"));
            addReceivers(engine);
            break;

        case Subjects:
            setCaption(tr("Subjects"));
            addSubjects(engine);
            break;

        case Software:
            setCaption(tr("Software"));
            addSoftware(engine);
            break;
    }
}

void TopListWindow::setupHeaders()
{
    listview->addColumn(tr("Place"));

    if (Quoters == toplist || Senders == toplist || OrigContent == toplist ||
        Receivers == toplist)
        listview->addColumn(tr("Name"));

    if (OrigContent == toplist)
        listview->addColumn(tr("Original content"));

    if (FidoNets == toplist)
        listview->addColumn(tr("Zone:Net"));

    if (Domains == toplist)
        listview->addColumn(tr("Domain"));

    if (Subjects == toplist)
        listview->addColumn(tr("Subject"));

    if (Software == toplist)
        listview->addColumn(tr("Program"));

    if (Quoters == toplist || Senders == toplist || OrigContent == toplist ||
        FidoNets == toplist || Domains == toplist || Subjects == toplist ||
        Software == toplist)
        listview->addColumn(tr("Messages"));

    if (Receivers == toplist)
    {
        listview->addColumn(tr("Received"));
        listview->addColumn(tr("Sent"));
    }

    if (Senders == toplist || FidoNets == toplist || Domains == toplist ||
        Subjects == toplist)
        listview->addColumn(tr("Bytes"));

    if (Senders == toplist)
        listview->addColumn(tr("Lines"));

    if (Quoters == toplist || Senders == toplist)
        listview->addColumn(tr("Quote ratio"));

    if (Receivers == toplist)
        listview->addColumn(tr("Ratio"));

    if (OrigContent == toplist)
        listview->addColumn(tr("Per Message"));
}

void TopListWindow::addQuoters(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QListViewItem *previous = NULL;

    StatEngine::persstat_s data;
    while (engine->GetTopQuoters(restart, data))
    {
        if (data.bytesquoted && data.byteswritten)
        {
            QString tmp;
            char *name_p = charConvertCopy(data.name);
            if (data.name != data.address)
                tmp = QString(name_p) +
                      " <" + data.address.c_str() + ">";
            else
                tmp = data.address.c_str();

            delete[] name_p;

            QListViewItem *item =
                new QListViewItem(listview, previous, QString::number(place ++),
                                  tmp, QString::number(data.messageswritten),
                                  percentString(data.bytesquoted,
                                                data.byteswritten));
            previous = item;
        }
        restart = false;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addSenders(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QListViewItem *previous = NULL;

    StatEngine::persstat_s data;
    while (engine->GetTopWriters(restart, data))
    {
        if (!data.messageswritten) break;

        QString tmp;
        char *name_p = charConvertCopy(data.name);
        if (data.name != data.address)
            tmp = QString(name_p) +
                  " <" + data.address.c_str() + ">";
        else
            tmp = data.address.c_str();

        delete[] name_p;


        QListViewItem *item =
            new QListViewItem(listview, previous, QString::number(place ++),
                              tmp, QString::number(data.messageswritten),
                              QString::number(data.byteswritten),
                              QString::number(data.lineswritten),
                              (data.bytesquoted > 0
                               ? percentString(data.bytesquoted,
                                               data.byteswritten)
                               : tr("N/A")));

        previous = item;
        restart = false;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addOriginalContent(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QListViewItem *previous = NULL;

    StatEngine::persstat_s data;
    while (engine->GetTopOriginalPerMsg(restart, data))
    {
        restart = false;
        if (0 == data.messageswritten)
        {
            continue;
        }

        QString tmp;
        char *name_p = charConvertCopy(data.name);
        if (data.name != data.address)
            tmp = QString(name_p) +
                  " <" + data.address.c_str() + ">";
        else
            tmp = data.address.c_str();

        delete[] name_p;

        unsigned originalpermsg =
            (data.byteswritten - data.bytesquoted) / data.messageswritten;


        QListViewItem *item =
            new QListViewItem(listview, previous, QString::number(place ++),
                              tmp, QString::number(data.byteswritten -
                                                   data.bytesquoted),
                              QString::number(data.messageswritten),
                              QString::number(originalpermsg));
        previous = item;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addFidoNets(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QListViewItem *previous = NULL;

    StatEngine::netstat_s data;
    while (engine->GetTopNets(restart, data))
    {
        QListViewItem *item =
            new QListViewItem(listview, previous, QString::number(place ++),
                              QString::number(data.zone) + ":" +
                              QString::number(data.net),
                              QString::number(data.messages),
                              QString::number(data.bytes));
        previous = item;
        restart = false;
    }

    engine->DoneTopNets();
}

void TopListWindow::addDomains(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QListViewItem *previous = NULL;

    StatEngine::domainstat_s data;
    while (engine->GetTopDomains(restart, data))
    {
        QListViewItem *item =
            new QListViewItem(listview, previous, QString::number(place ++),
                              data.topdomain.c_str(),
                              QString::number(data.messages),
                              QString::number(data.bytes));

        previous = item;
        restart = false;
    }

    engine->DoneTopDomains();
}

void TopListWindow::addReceivers(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QListViewItem *previous = NULL;

    StatEngine::persstat_s data;
    while (engine->GetTopReceivers(restart, data))
    {
        if (!data.messagesreceived) break;

        QString tmp;
        char *name_p = charConvertCopy(data.name);
        if (name_p && *name_p)
            tmp = QString(name_p);
        else
            tmp = tr("(none)");
        delete[] name_p;

        QListViewItem *item =
            new QListViewItem(listview, previous, QString::number(place ++),
                              tmp, QString::number(data.messagesreceived),
                              QString::number(data.messageswritten),
                              (data.messageswritten
                               ? QString::number((100 * data.messagesreceived)
                                                 / data.messageswritten) + "%"
                               : tr("N/A")));

        previous = item;
        restart = false;
    }

    engine->DoneTopPeople();
}

void TopListWindow::addSubjects(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    StatEngine::subjstat_s data;
    QListViewItem *previous = NULL;

    while (engine->GetTopSubjects(restart, data))
    {
        QString tmp;
        char *subj_p = charConvertCopy(data.subject);
        if (subj_p && *subj_p)
            tmp = QString(subj_p);
        else
            tmp = tr("(none)");
        delete[] subj_p;

        QListViewItem *item =
            new QListViewItem(listview, previous, QString::number(place ++),
                              tmp, QString::number(data.count),
                              QString::number(data.bytes));

        previous = item;
        restart = false;
    }

    engine->DoneTopSubjects();
}

void TopListWindow::addSoftware(StatEngine *engine)
{
    unsigned place = 1;
    bool restart = true;
    QListViewItem *previous = NULL;

    StatEngine::progstat_s data;
    while (engine->GetTopPrograms(restart, data))
    {
        QString tmp;
        char *prog_p = charConvertCopy(data.program);
        if (prog_p && *prog_p)
            tmp = QString(prog_p);
        else
            tmp = tr("(none)");
        delete[] prog_p;

        QListViewItem *item =
            new QListViewItem(listview, previous, QString::number(place ++),
                              tmp, QString::number(data.count));


        previous = item;
        restart = false;
    }

    engine->DoneTopPrograms();
}

char *TopListWindow::charConvertCopy(string &inputstring)
{
    // Convert any characters in the range 128-159 as if they were in the
    // IBM codepage 865 character set (Fidonet is mostly PC8 based).
    // -*-TODO-*-This should of course be done in the engine itself
    // (CHRS support with configurable fallback), but until that is
    // implemented, this will have to do.
    static const char convert[] =
        "Çüéâäàå©êëèïîìÄÅÉæÆôöòûùÿÖÜø£ØPf";

    unsigned char *output= (unsigned char *) new char[inputstring.length() + 1];
    unsigned char *input = (unsigned char *) inputstring.c_str();
    char *rc = (char *) output;

    while (*input)
    {
        if (*input >= 128 && *input <= 160)
        {
            *(output ++) = convert[*(input ++) & 0x7f];
        }
        else
        {
            *(output ++) = *(input ++);
        }
    }

    // Terminate what we got
    *output = 0;

    return rc;
}

QString TopListWindow::percentString(int numerator, int denumerator)
    return s;
{
    float percent =
        (100.0 * numerator) / (float) denumerator;
    s = QString::number(percent, 'f', 2) + "%";
}

void TopListWindow::saveToFile()
{
    int numentries = listview->childCount();
    int numcolumns = listview->columns();
    if (0 == numentries || 0 == numcolumns)
    {
        QMessageBox::warning(this, "Turquoise SuperStat",
                             tr("This list is empty"));
        return;
    }

#define NUMFILTERS 3
    const QString filter[NUMFILTERS] =
    {
#define PLAINTEXT 0
        tr("Plain text (*.txt)"),
#define COMMA 1
        tr("Comma-separated file (*.csf)"),
#define SEMICOLON 2
        tr("Semi-colon separated file (*.csf)")
    };

    QFileDialog filedialog(QString::null,
                           filter[0] + ";;" + filter[1] + ";;" + filter[2],
                           this, "savefile", true);
    filedialog.setMode(QFileDialog::AnyFile);
    filedialog.setCaption(tr("Save toplist"));
    filedialog.setShowHiddenFiles(false);
    if (filedialog.exec() != QDialog::Accepted)
    {
        return;
    }

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
        return;
    }

    // Create the file
    QFile output(filedialog.selectedFile());
    if (!output.open(IO_WriteOnly | IO_Translate))
    {
        QMessageBox::warning(this, filedialog.selectedFile(),
                             tr("Unable to create the selected file"));
        return;
    }

    // Select output format
    QString format;
    unsigned defaultwidth = 1;
    if (PLAINTEXT == filternum)
    {
        // Default: same width for all entries
        defaultwidth = 80 / numcolumns;
        if (!defaultwidth)
        {
            defaultwidth = 1;
        }
cout << numcolumns << endl;

        // TODO: Give the user some option to edit this...
//      columnwidth = new unsigned[numcolumns];
//      unsigned *columnwidth = NULL;
//      for (i = 0; i < numcolumns; i ++)
//      {
//          columnwidth[i] = defaultwidth;
//      }
    }

    QTextStream out(&output);

    // Print the titles
    for (int i = 0; i < numcolumns; i ++)
    {
        QString s = listview->columnText(i);
        if (i < numcolumns - 1)
        {
            switch (filternum)
            {
                case PLAINTEXT:
                    s = s.leftJustify(defaultwidth, ' ', true);
                    break;

                case COMMA:
                    s += ',';
                    break;

                case SEMICOLON:
                    s += ';';
                    break;
            }
        }
        out << s;
    }
    out << endl;

    QListViewItem *current = listview->firstChild();
    // Print the entries
    for (int i = 0; i < numentries; i ++)
    {
        for (int j = 0; j < numcolumns; j ++)
        {
            QString s = current->text(j);
    
            if (j < numcolumns - 1)
            {
                switch (filternum)
                {
                    case PLAINTEXT:
                        s = s.leftJustify(defaultwidth, ' ', true);
                        break;

                    case COMMA:
                        s.replace(QRegExp(","), ";");
                        s += ',';
                        break;

                    case SEMICOLON:
                        s.replace(QRegExp(";"), ",");
                        s += ';';
                        break;
                }
            }
            out << s;
        }
        out << endl;

        // Go to next item
        current = current->nextSibling();
    }

    // Done
    output.close();
}
