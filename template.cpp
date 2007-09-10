// Copyright (c) 2007 Peter Karlsson
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

#include "template.h"
#include "lexer.h"

/* static */
Template *Template::Parse(const string &file, bool &error)
{
    // open file

    if (/* could not open file */)
    {
        error = true;
        return NULL;
    }

    // Tokenize the file.
    Template *retval_p = NULL;      // The pointer to the whole template.
    Template **last_pp = &retval_p; // Next pointer to fill in.

    while (/* !eof */ && !error)
    {
        string line = /* read from file */
        if (line != "")
        {
            Token *tokenlist = Token::Parse(line, error);
            if (tokenlist)
            {
                *last_pp = new Template(tokenlist);
                last_pp = &((*last_pp)->m_next_p);
            }
        }
    }

    // close file

    return retval_p;
}
