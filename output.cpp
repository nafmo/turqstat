// Copyright (c) 2000 Peter Karlsson
//
// $Id: output.cpp,v 1.2.2.1 2000/09/19 17:17:56 peter Exp $
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

#include "output.h"

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

void TDisplay::ErrorMessage(string errormessage)
{
    cerr << "Error: " << errormessage << endl;
}

void TDisplay::ErrorMessage(string errormessage, int number)
{
    cerr << "Error: " << errormessage << number << endl;
}

static string GetMessage(TDisplay::errormessages_e errormessage)
    return s;
{
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
    }
}

void TDisplay::ErrorQuit(errormessages_e errormessage, int returncode)
{
    ErrorMessage(GetMessage(errormessage));
    cerr << GetMessage(program_halted) << endl;
    exit(returncode);
}

void TDisplay::InternalErrorQuit(errormessages_e errormessage,
                                        int returncode)
{
    ErrorMessage(string("Internal error: ") + GetMessage(errormessage));
    cerr << GetMessage(program_halted) << endl;
    exit(returncode);
}

void TDisplay::WarningMessage(string errormessage)
{
    cerr << "Warning: " << errormessage << endl;
}

void TDisplay::WarningMessage(string errormessage, int number)
{
    cerr << "Warning: " << errormessage << number << endl;
}
