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

    // Add Ok button
    QPushButton *ok = new QPushButton(tr("Ok"), this);
    layout->addWidget(ok);
    connect(ok, SIGNAL(clicked()), SLOT(accept()));
}

TopListWindow::~TopListWindow()
{
}

void TopListWindow::fillOut(StatEngine *engine)
{
    if (!engine || !listview) return;

    switch (toplist)
    {
    case Quoters:
        setCaption(tr("Quoters"));
        setupQuotersHeaders();
        addQuoters(engine);
        break;
    }
}

void TopListWindow::setupQuotersHeaders(void)
{
    listview->addColumn(tr("Place"));
    listview->addColumn(tr("Name"));
    listview->addColumn(tr("Messages"));
    listview->addColumn(tr("Ratio"));
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
            float percent =
                (100.0 * data.bytesquoted) / (float) data.byteswritten;
            QString percentstring = QString::number(percent, 'f', 2) + "%";

            QString tmp;
            char *name_p    = charConvertCopy(data.name);
            if (data.name != data.address)
                tmp = QString(name_p) +
                      " <" + data.address.c_str() + ">";
            else
                tmp = data.address.c_str();

            delete[] name_p;

            QListViewItem *item =
                new QListViewItem(listview, previous, QString::number(place ++),
                                  tmp, QString::number(data.messageswritten),
                                  percentstring);
            previous = item;
        }
        restart = false;
    }

    engine->DoneTopPeople();
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
