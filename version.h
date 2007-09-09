// Copyright (c) 1998-2007 Peter Karlsson
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

#ifndef __VERSION_H
#define __VERSION_H

#include <string>

#if !defined(HAVE_IMPLICIT_NAMESPACE)
using namespace std;
#endif

/** String describing the current version number. */
const string version = "3.0";

/** String describing the copyright information. */
const string copyright = "(c) Copyright 1998-2007 Peter Karlsson";

/** String describing the release year for this version. */
const string releaseyear = "2007";

#endif
