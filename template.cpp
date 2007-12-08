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
#ifdef HAVE_FSTREAM
# include <fstream>
#else
# include <fstream.h>
#endif

/* static */
Template *Template::Parse(const string &file, bool &error)
{
	bool in_settings = false;

    // Open file
    fstream templatefile(file.c_str(), ios::in);

    if (!(templatefile.is_open()))
    {
        error = true;
        return NULL;
    }

    // Tokenize the file.
    Template *retval_p = NULL;      // The pointer to the whole template.
    Template **last_pp = &retval_p; // Next pointer to fill in.

    while (!templatefile.eof() && !error)
    {
        string line;
        getline(templatefile, line);
		Token *tokenlist = Token::Parse(line, in_settings, error);
		if (tokenlist)
		{
			// Check if a new section found is a settings section
			if (tokenlist->IsSection())
			{
				in_settings =
					static_cast<Section *>(tokenlist)->GetSection()
					== Section::makLocalization;
			}

			// Add line to template
			*last_pp = new Template(tokenlist);
			last_pp = &((*last_pp)->m_next_p);
		}
    }

    // Close the file and exit
    templatefile.close();
    return retval_p;
}
