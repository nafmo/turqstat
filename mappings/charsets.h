// Copyright (c) 2001 Peter Karlsson
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

#ifndef __CHARSETS_H
#define __CHARSETS_H

/** Structure mapping charset names to mapping tables. */
struct tablemap
{
    const char * const      charset;
    const unsigned short    *inmap;
    const struct reversemap *outmap;
};

/** Lookup table for Fidonet charset identifiers. */
extern const struct tablemap fidocharsets[];

/** Lookup table for Usenet charset identifiers. */
extern const struct tablemap usenetcharsets[];

#endif
