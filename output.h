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

class TDisplay
{
public:
    // Constructor and destructor
    TDisplay() : maximum(-1) {};

    // Data types
    enum errormessages_e
    {
        // Public error messages
        out_of_memory, area_not_allocated, message_base_mismatch,
        out_of_memory_area,

        // Internal error messages
        program_halted, cannot_allocate_tdisplay
    };

    // Message methods
    void SetMessagesTotal(int number);
    void UpdateProgress(int messages);
    void ErrorMessage(string errormessage);
    void ErrorMessage(string errormessage, int number);
    void ErrorQuit(errormessages_e errormessage, int returncode);
    void InternalErrorQuit(errormessages_e errormessage, int returncode);
    void WarningMessage(string errormessage);
    void WarningMessage(string errormessage, int number);

    static TDisplay *GetOutputObject();

protected:
    TDisplay    *outputobject;
    int         maximum;
};
#endif
