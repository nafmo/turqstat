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

#include <iostream>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qtgui.h>
#include <qapplication.h>

#include "output.h"

static QString GetMessage(TDisplay::errormessages_e);

TDisplay *TDisplay::GetOutputObject()
{
    static TDisplay *outputobject = NULL;

    if (!outputobject)
    {
        outputobject = new TDisplay();
        if (!outputobject)
        {
            cerr << GetMessage(cannot_allocate_tdisplay) << endl;
            exit(255);
        }
    }

    return outputobject;
}

void TDisplay::SetMessagesTotal(int number)
{
    maximum = number;

    QProgressDialog *p = InfoWindow::getMainWindow()->getProgressDialog();
    if (!p) return;

    if (maximum > 0)
    {
        p->setTotalSteps(number);
    }
    else
    {
        p->setTotalSteps(100);
    }
    p->setProgress(0);
}

void TDisplay::UpdateProgress(int messages)
{
    QProgressDialog *p = InfoWindow::getMainWindow()->getProgressDialog();
    if (!p) return;

    if (maximum <= 0)
    {
        p->setProgress(messages % 100);
    }
    else
    {
        p->setProgress(messages);
    }
}

void TDisplay::ErrorMessage(string errormessage)
{
    QMessageBox::critical(NULL, "Turquoise SuperStat", errormessage.c_str(),
                          QMessageBox::Ok | QMessageBox::Default, 0, 0);
}

void TDisplay::ErrorMessage(string errormessage, int number)
{
    QMessageBox::critical(NULL, "Turquoise SuperStat",
                          QString(errormessage.c_str()) +
                          QString::number(number),
                          QMessageBox::Ok | QMessageBox::Default, 0, 0);
}

static QString GetMessage(TDisplay::errormessages_e errormessage)
    return s;
{
    switch (errormessage)
    {
        case TDisplay::out_of_memory:
            s = qApp->translate("TDisplay", "Out of memory.");
            break;

        case TDisplay::area_not_allocated:
            s = qApp->translate("TDisplay",
                                "Area path was not allocated properly.");
            break;

        case TDisplay::message_base_mismatch:
            s = qApp->translate("TDisplay", "Message base format mismatch.");
            break;

        case TDisplay::out_of_memory_area:
            s = qApp->translate("TDisplay",
                                "Out of memory allocating area object.");
            break;

        case TDisplay::program_halted:
            s = qApp->translate("TDisplay", "Program halted!");
            break;

		case TDisplay::cannot_allocate_tdisplay:
		    s = qApp->translate("TDisplay",
		                        "Unable to allocate memory for output object!");
		    break;
    }
}

void TDisplay::ErrorQuit(errormessages_e errormessage, int returncode)
{
    QMessageBox::critical(NULL, "Turquoise SuperStat",
                          GetMessage(errormessage),
                          QMessageBox::Ok | QMessageBox::Default, 0, 0);
    cerr << GetMessage(program_halted) << endl;
    exit(returncode);
}

void TDisplay::InternalErrorQuit(errormessages_e errormessage,
                                        int returncode)
{
    QMessageBox::critical(NULL, qApp->translate("TDisplay", "Internal error"),
                          GetMessage(errormessage),
                          QMessageBox::Ok | QMessageBox::Default, 0, 0);
    cerr << GetMessage(program_halted) << endl;
    exit(returncode);
}

void TDisplay::WarningMessage(string errormessage)
{
    QMessageBox::warning(NULL, "Turquoise SuperStat", errormessage.c_str(),
                         QMessageBox::Ok | QMessageBox::Default, 0, 0);
}

void TDisplay::WarningMessage(string errormessage, int number)
{
    QMessageBox::warning(NULL, "Turquoise SuperStat",
                         QString(errormessage.c_str()) +
                         QString::number(number),
                         QMessageBox::Ok | QMessageBox::Default, 0, 0);
}
