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

#include "output.h"

ProgressDisplay *ProgressDisplay::GetOutputObject()
{
    static ProgressDisplay *outputobject = NULL;

    if (!outputobject)
    {
        outputobject = new ProgressDisplay();
    }

    return outputobject;
}

void ProgressDisplay::SetMessagesTotal(int number)
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

void ProgressDisplay::UpdateProgress(int messages)
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

void ProgressDisplay::ErrorMessage(string errormessage)
{
    QMessageBox::critical(NULL, "Turquoise SuperStat", errormessage.c_str(),
                          QMessageBox::Ok | QMessageBox::Default, 0, 0);
}

void ProgressDisplay::ErrorMessage(string errormessage, int number)
{
    QMessageBox::critical(NULL, "Turquoise SuperStat",
                          QString(errormessage.c_str()) +
                          QString::number(number),
                          QMessageBox::Ok | QMessageBox::Default, 0, 0);
}

void ProgressDisplay::WarningMessage(string errormessage)
{
    QMessageBox::warning(NULL, "Turquoise SuperStat", errormessage.c_str(),
                         QMessageBox::Ok | QMessageBox::Default, 0, 0);
}

void ProgressDisplay::WarningMessage(string errormessage, int number)
{
    QMessageBox::warning(NULL, "Turquoise SuperStat",
                         QString(errormessage.c_str()) +
                         QString::number(number),
                         QMessageBox::Ok | QMessageBox::Default, 0, 0);
}
