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

#ifndef __OUTPUT_H
#define __OUTPUT_H

#include <string>

class ostream;

class ProgressDisplay
{
public:
    // Constructor and destructor
    ProgressDisplay() : maximum(-1) {};

    void SetMessagesTotal(int number);
    void UpdateProgress(int messages);
    void ErrorMessage(string errormessage);
    void ErrorMessage(string errormessage, int number);
    void WarningMessage(string errormessage);
    void WarningMessage(string errormessage, int number);

    static ProgressDisplay *GetOutputObject();

protected:
    ProgressDisplay *outputobject;
    int             maximum;
};
#endif
