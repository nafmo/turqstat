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
        out_of_memory_area, cannot_open, cannot_read,
        area_out_of_range, illegal_area,
        strange_squish_header, message_base_empty, strange_squish_offset,
        premature_squish_end, illegal_squish_header,
        cannot_open_spool, cannot_open_file,
        illegal_jam_header,
        cannot_read_header,
        illegal_tanstaafl_version, tanstaafl_version_0,
        cannot_open_msg_directory, broken_msg,
        illegal_fdapx_version,
        illegal_mypoint_version, mypoint_area_garbled, mypoint_area_garbled_2,

        // Public warning messages
        abnormal_squish_frame,
        cannot_allocate_control, cannot_allocate_control_file,
        cannot_allocate_body, cannot_allocate_body_file,

        // Internal error messages
        program_halted, cannot_allocate_tdisplay
    };

    // Message methods
    void SetMessagesTotal(int number);
    void UpdateProgress(int messages);
    void ErrorMessage(errormessages_e errormessage, string data = "");
    void ErrorMessage(errormessages_e errormessage, int data);
    void ErrorQuit(errormessages_e errormessage, int returncode);
    void InternalErrorQuit(errormessages_e errormessage, int returncode);
    void WarningMessage(errormessages_e errormessage, string data = "");
    void WarningMessage(errormessages_e errormessage, int data);

    static TDisplay *GetOutputObject();

protected:
    TDisplay    *outputobject;
    int         maximum;
};
#endif
