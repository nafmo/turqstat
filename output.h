// Copyright (c) 2000-2001 Peter Karlsson
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

using namespace std;

class ostream;

/**
 * Class used to display messages.
 * This class is used to send output to the user running the program. It is
 * re-implemented for each user interface type the program is implemented
 * for, but the interface to the class remains the same no matter if the
 * output is destined for a text console or a GUI window.
 */
class TDisplay
{
public:
    /** Standard constructor. */
    TDisplay() : maximum(-1) {};

    /**
     * Numeric constants describing all possible error messages that can be
     * delivered to the user.
     */
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

    /**
     * Set the number of messages to count to. This value is used to
     * determine what to count to, when displaying how much of the area has
     * been read.
     * @param number Number of messages in area, or -1 if it is unknown in
     *               advance.
     */
    void SetMessagesTotal(int number);

    /**
     * Update progress indicator. Indicate to the user how much of the area
     * that has been read, and perhaps an estimate of how much time is left.
     * @param messages Number of messages read.
     */
    void UpdateProgress(int messages);

    /**
     * Display an error message to the user.
     * @param errormessage Number of error message to display.
     * @param data         Possible extra data to display to the user.
     */
    void ErrorMessage(errormessages_e errormessage, string data = "");

    /**
     * Display an error message to the user.
     * @param errormessage Number of error message to display.
     * @param data         Possible extra data to display to the user.
     */
    void ErrorMessage(errormessages_e errormessage, int data);

    /**
     * Display an error message to the user and abort execution. This is
     * used for fatal errors that cannot be recovered.
     * @param errormessage Number of error message to display.
     * @param data         Possible extra data to display to the user.
     */
    void ErrorQuit(errormessages_e errormessage, int returncode);

    /**
     * Display an error message to the user and abort execution. This is
     * used for fatal internal errors that cannot be recovered.
     * @param errormessage Number of error message to display.
     * @param data         Possible extra data to display to the user.
     */
    void InternalErrorQuit(errormessages_e errormessage, int returncode);

    /**
     * Display a warning message to the user.
     * @param errormessage Number of warning message to display.
     * @param data         Possible extra data to display to the user.
     */
    void WarningMessage(errormessages_e errormessage, string data = "");

    /**
     * Display a warning message to the user.
     * @param errormessage Number of warning message to display.
     * @param data         Possible extra data to display to the user.
     */
    void WarningMessage(errormessages_e errormessage, int data);

    /**
     * Get the global instance of the output object. Use this static method
     * to retrieve the global (and only) instance of this class, to use for
     * displaying messages to the user. The object is instanciated on the
     * first call.
     * @return The global TDisplay object.
     */
    static TDisplay *GetOutputObject();

protected:
    /** The number of messages in the current area. */
    int         maximum;
};
#endif
