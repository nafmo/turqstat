// Copyright (c) 1998-2000 Peter Karlsson
//
// $Id: statengine.cpp,v 1.27 2000/06/16 21:00:18 peter Exp $
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

ProgressDisplay *ProgressDisplay::GetOutputObject()
{
    static ProgressDisplay *outputobject = NULL;

    if (!outputobject)
    {
        outputobject = new TtyProgressDisplay();
    }

    return outputobject;
}

void ProgressDisplay::SetMessagesTotal(int number)
{
    maximum = number;
}

void TtyProgressDisplay::UpdateProgress(int message)
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

void TtyProgressDisplay::ErrorMessage(string errormessage)
{
    cerr << "Error: " << errormessage << endl;
}

void TtyProgressDisplay::ErrorMessage(string errormessage, int number)
{
    cerr << "Error: " << errormessage << number << endl;
}

void TtyProgressDisplay::WarningMessage(string errormessage)
{
    cerr << "Warning: " << errormessage << endl;
}

void TtyProgressDisplay::WarningMessage(string errormessage, int number)
{
    cerr << "Warning: " << errormessage << number << endl;
}
