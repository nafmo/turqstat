// Copyright (c) 1998-1999 Peter Karlsson
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

#include <string>
#include <ctype.h>

#include "utility.h"

// Compare two strings case in-sensitively
// Why isn't this functionality available in ANSI C++? *sigh*
int fcompare(const string &s1, const string &s2)
{
    int ls1 = s1.length(), ls2 = s2.length();

    for (int i = 0; i < ls1 && i < ls2; i ++)
    {
        if (toupper(s1[i]) != toupper(s2[i]))
        {
            return toupper(s1[i]) - toupper(s2[i]);
        }
    }

    return ls1 - ls2;
}
