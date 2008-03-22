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

#include <stdlib.h>
#include <iostream>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qapplication.h>

#include "output.h"
#include "qtprogress.h"
#include "qtgui.h"

static QString GetMessage(TDisplay::errormessages_e);

TDisplay *TDisplay::GetOutputObject()
{
    static TDisplay *outputobject = NULL;

    if (!outputobject)
    {
        outputobject = new TDisplay();
        if (!outputobject)
        {
			cerr << GetMessage(cannot_allocate_tdisplay).toStdString() << endl;
            exit(255);
        }
    }

    return outputobject;
}

void TDisplay::SetMessagesTotal(int number)
{
    maximum = number;

    if (maximum > 0)
    {
        QProgressDialog *p =
            InfoWindow::getMainWindow()->getProgressDialog(maximum);
		p->setMaximum(number);
		p->setValue(0);
    }
}

void TDisplay::UpdateProgress(int messages)
{
    if (maximum > 0)
    {
        QProgressDialog *p =
            InfoWindow::getMainWindow()->getProgressDialog(maximum);
		p->setValue(messages);
    }
    else
    {
        ProgressText *p =
            InfoWindow::getMainWindow()->getProgressText();
        p->setProgress(messages);
    }
}

void TDisplay::ErrorMessage(errormessages_e errormessage, string data)
{
    QString msg = GetMessage(errormessage);
    if (msg.contains('%'))
    {
        msg = msg.arg(data.c_str());
    }
    QMessageBox::critical(NULL, "Turquoise SuperStat", msg,
                          QMessageBox::Ok | QMessageBox::Default, 0, 0);
}

void TDisplay::ErrorMessage(errormessages_e errormessage, int data)
{
    QString msg = GetMessage(errormessage).arg(data);
    QMessageBox::critical(NULL, "Turquoise SuperStat", msg,
                          QMessageBox::Ok | QMessageBox::Default, 0, 0);
}

void TDisplay::ErrorQuit(errormessages_e errormessage, int returncode)
{
    QMessageBox::critical(NULL, "Turquoise SuperStat",
                          GetMessage(errormessage),
                          QMessageBox::Ok | QMessageBox::Default, 0, 0);
	cerr << GetMessage(program_halted).toStdString() << endl;
    exit(returncode);
}

void TDisplay::InternalErrorQuit(errormessages_e errormessage,
                                        int returncode)
{
    QMessageBox::critical(NULL, qApp->translate("TDisplay", "Internal error"),
                          GetMessage(errormessage),
                          QMessageBox::Ok | QMessageBox::Default, 0, 0);
    cerr << GetMessage(program_halted).toStdString() << endl;
    exit(returncode);
}

void TDisplay::WarningMessage(errormessages_e errormessage, string data)
{
    QString msg = GetMessage(errormessage);
    if (msg.contains('%'))
    {
        msg = msg.arg(data.c_str());
    }
    QMessageBox::warning(NULL, "Turquoise SuperStat", msg,
                         QMessageBox::Ok | QMessageBox::Default, 0, 0);
}

void TDisplay::WarningMessage(errormessages_e errormessage, int data)
{
    QString msg = GetMessage(errormessage).arg(data);
    QMessageBox::warning(NULL, "Turquoise SuperStat", msg,
                        QMessageBox::Ok | QMessageBox::Default, 0, 0);
}

static QString GetMessage(TDisplay::errormessages_e errormessage)
{
    QString s;

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

        case TDisplay::cannot_open:
            s = qApp->translate("TDisplay", "Cannot open %1");
            break;

        case TDisplay::cannot_read:
            s = qApp->translate("TDisplay", "Cannot read from %1");
            break;

        case TDisplay::strange_squish_header:
            s = qApp->translate("TDisplay", "Strange Squish header length");
            break;

        case TDisplay::message_base_empty:
            s = qApp->translate("TDisplay", "Message base is empty");
            break;

        case TDisplay::strange_squish_offset:
            s = qApp->translate("TDisplay", "Strange Squish header offset");
            break;

        case TDisplay::premature_squish_end:
            s = qApp->translate("TDisplay", "Premature end of Squish data");
            break;

        case TDisplay::illegal_squish_header:
            s = qApp->translate("TDisplay", "Illegal Squish header ID");
            break;

        case TDisplay::abnormal_squish_frame:
            s = qApp->translate("TDisplay", "Not normal Squish frame #%1");
            break;

        case TDisplay::cannot_allocate_control:
            s = qApp->translate("TDisplay",
                                "Unable to allocate memory for control "
                                "data #%1");
            break;

        case TDisplay::cannot_allocate_control_file:
            s = qApp->translate("TDisplay",
                                "Unable to allocate memory for control "
                                "data for %1");
            break;

        case TDisplay::cannot_allocate_body:
            s = qApp->translate("TDisplay",
                                "Unable to allocate memory for message "
                                "body #%1");
            break;

        case TDisplay::cannot_allocate_body_file:
            s = qApp->translate("TDisplay",
                                "Unable to allocate memory for message "
                                "body for %1");
            break;

        case TDisplay::cannot_open_spool:
            s = qApp->translate("TDisplay", "Unable to open spool directory");
            break;

        case TDisplay::cannot_open_file:
            s = qApp->translate("TDisplay", "Cannot open %1");
            break;

        case TDisplay::illegal_jam_header:
            s = qApp->translate("TDisplay", "Illegal JAM header");
            break;

        case TDisplay::cannot_read_header:
            s = qApp->translate("TDisplay", "Unable to read header #%1");
            break;

        case TDisplay::area_out_of_range:
            s = qApp->translate("TDisplay",
                                "Invalid area number chosen (must be "
                                "between 1-%1");
            break;

        case TDisplay::illegal_area:
            s = qApp->translate("TDisplay",
                                "Invalid area number chosen ");
            break;

        case TDisplay::illegal_tanstaafl_version:
            s = qApp->translate("TDisplay",
                                "Illegal tanstaafl message base version");
            break;

        case TDisplay::illegal_fdapx_version:
            s = qApp->translate("TDisplay",
                                "Illegal FDAPX/w message base version");
            break;

        case TDisplay::illegal_mypoint_version:
            s = qApp->translate("TDisplay",
                                "Illegal MyPoint message base version: %1");
            break;

        case TDisplay::tanstaafl_version_0:
            s = qApp->translate("TDisplay",
                                "Tanstaafl message base version is 0, "
                                "assuming 1");
            break;

        case TDisplay::cannot_open_msg_directory:
            s = qApp->translate("TDisplay", "Unable to open *.MSG directory");
            break;

        case TDisplay::broken_msg:
            s = qApp->translate("TDisplay", "Broken MSG file %1");
            break;

        case TDisplay::mypoint_area_garbled:
            s = qApp->translate("TDisplay",
                                "Message area garbled (illegal delimeter)!");
            break;

        case TDisplay::mypoint_area_garbled_2:
            s = qApp->translate("TDisplay",
                                "Message area garbled (footer does not "
                                "match header)!");
            break;

        case TDisplay::nntp_unexpected_result:
            s = qApp->translate("TDisplay",
                                "Unexpected NNTP response code: %1");
            break;

        case TDisplay::nntp_communication_problem:
            s = qApp->translate("TDisplay", "NNTP communication problem");
            break;

		case TDisplay::template_parse_error:
			s = qApp->translate("TDisplay", "Could not parse template file");
			break;
    }

    return s;
}
