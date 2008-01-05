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
#include <stdio.h>

#include "output.h"

#define STRLEN 256

#ifndef HAVE_SNPRINTF
# ifdef HAVE_USNPRINTF
#  define snprintf _snprintf
# else
#  define snprintf(a,b,c,d) sprintf(a,c,d)
# endif
#endif // !HAVE_SNPRINTF

static string GetMessage(TDisplay::errormessages_e);

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
}

void TDisplay::UpdateProgress(int message)
{
    if (maximum <= 0)
    {
        cout << message << " done\r";
    }
    else
    {
        cout << 100 * message / maximum << "% done\r";
    }
}

void TDisplay::ErrorMessage(errormessages_e errormessage, string data)
{
    char msg[STRLEN];
    string msgtemplate = GetMessage(errormessage);
    snprintf(msg, STRLEN, msgtemplate.c_str(), data.c_str());
    cerr << "Error: " << msg << endl;
}

void TDisplay::ErrorMessage(errormessages_e errormessage, int data)
{
    char msg[STRLEN];
    string msgtemplate = GetMessage(errormessage);
    snprintf(msg, STRLEN, msgtemplate.c_str(), data);
    cerr << "Error: " << msg << endl;
}

void TDisplay::ErrorQuit(errormessages_e errormessage, int returncode)
{
    ErrorMessage(errormessage);
    cerr << GetMessage(program_halted) << endl;
    exit(returncode);
}

void TDisplay::InternalErrorQuit(errormessages_e errormessage,
                                 int returncode)
{
    cerr << "Internal error: " << GetMessage(errormessage) << endl;
    cerr << GetMessage(program_halted) << endl;
    exit(returncode);
}

void TDisplay::WarningMessage(errormessages_e errormessage, string data)
{
    char msg[STRLEN];
    string msgtemplate = GetMessage(errormessage);
    snprintf(msg, STRLEN, msgtemplate.c_str(), data.c_str());
    cerr << "Warning: " << msg << endl;
}

void TDisplay::WarningMessage(errormessages_e errormessage, int data)
{
    char msg[STRLEN];
    string msgtemplate = GetMessage(errormessage);
    snprintf(msg, STRLEN, msgtemplate.c_str(), data);
    cerr << "Warning: " << msg << endl;
}

static string GetMessage(TDisplay::errormessages_e errormessage)
{
    string s;

    switch (errormessage)
    {
        case TDisplay::out_of_memory:
            s = "Out of memory.";
            break;

        case TDisplay::area_not_allocated:
            s = "Area path was not allocated properly.";
            break;

        case TDisplay::message_base_mismatch:
            s = "Message base format mismatch.";
            break;

        case TDisplay::out_of_memory_area:
            s = "Out of memory allocating area object.";
            break;

        case TDisplay::program_halted:
            s = "Program halted!";
            break;

        case TDisplay::cannot_allocate_tdisplay:
            s = "Unable to allocate memory for output object!";
            break;

        case TDisplay::cannot_open:
            s = "Cannot open %s";
            break;

        case TDisplay::cannot_read:
            s = "Cannot read from %s";
            break;

        case TDisplay::strange_squish_header:
            s = "Strange Squish header length";
            break;

        case TDisplay::message_base_empty:
            s = "Message base is empty";
            break;

        case TDisplay::strange_squish_offset:
            s = "Strange Squish header offset";
            break;

        case TDisplay::premature_squish_end:
            s = "Premature end of Squish data";
            break;

        case TDisplay::illegal_squish_header:
            s = "Illegal Squish header ID";
            break;

        case TDisplay::abnormal_squish_frame:
            s = "Not normal Squish frame #%d";
            break;

        case TDisplay::cannot_allocate_control:
            s = "Unable to allocate memory for control data #%d";
            break;

        case TDisplay::cannot_allocate_control_file:
            s = "Unable to allocate memory for control data for %s";
            break;

        case TDisplay::cannot_allocate_body:
            s = "Unable to allocate memory for message body #%d";
            break;

        case TDisplay::cannot_allocate_body_file:
            s = "Unable to allocate memory for message body for %s";
            break;

        case TDisplay::cannot_open_spool:
            s = "Unable to open spool directory";
            break;

        case TDisplay::cannot_open_file:
            s = "Cannot open %s";
            break;

        case TDisplay::illegal_jam_header:
            s = "Illegal JAM header";
            break;

        case TDisplay::cannot_read_header:
            s = "Unable to read header #%d";
            break;

        case TDisplay::area_out_of_range:
            s = "Invalid area number chosen (must be between 1-%d)";
            break;

        case TDisplay::illegal_area:
            s = "Invalid area number chosen";
            break;

        case TDisplay::illegal_tanstaafl_version:
            s = "Illegal tanstaafl message base version";
            break;

        case TDisplay::illegal_fdapx_version:
            s = "Illegal FDAPX/w message base version";
            break;

        case TDisplay::illegal_mypoint_version:
            s = "Illegal MyPoint message base version: %d";
            break;

        case TDisplay::tanstaafl_version_0:
            s = "Tanstaafl message base version is 0, assuming 1";
            break;

        case TDisplay::cannot_open_msg_directory:
            s = "Unable to open *.MSG directory";
            break;

        case TDisplay::broken_msg:
            s = "Broken MSG file %s";
            break;

        case TDisplay::mypoint_area_garbled:
            s = "Message area garbled (illegal delimeter)!";
            break;

        case TDisplay::mypoint_area_garbled_2:
            s = "Message area garbled (footer does not match header)!";
            break;

        case TDisplay::nntp_unexpected_result:
            s = "Unexpected NNTP response code: %d";
            break;

        case TDisplay::nntp_communication_problem:
            s = "NNTP communication problem";
            break;

		case TDisplay::template_parse_error:
			s = "Could not parse template file";
			break;
    }

    return s;
}
