// Copyright (c) 2000 Peter Karlsson
//
// $Id: output.cpp,v 1.1 2000/06/19 20:01:09 peter Exp $
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
        outputobject = new ProgressDisplay();
    }

    return outputobject;
}

void ProgressDisplay::SetMessagesTotal(int number)
{
    maximum = number;
}

void ProgressDisplay::UpdateProgress(int message)
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

void ProgressDisplay::ErrorMessage(string errormessage)
{
    cerr << "Error: " << errormessage << endl;
}

void ProgressDisplay::ErrorMessage(string errormessage, int number)
{
    cerr << "Error: " << errormessage << number << endl;
}

void ProgressDisplay::WarningMessage(string errormessage)
{
    cerr << "Warning: " << errormessage << endl;
}

void ProgressDisplay::WarningMessage(string errormessage, int number)
{
    cerr << "Warning: " << errormessage << number << endl;
}
