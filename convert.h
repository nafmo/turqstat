// Copyright (c) 2001-2002 Peter Karlsson
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

#ifndef __CONVERT_H
#define __CONVERT_H

#ifdef HAVE_WORKING_WSTRING
# define __ENABLE_WSTRING
# include <wchar.h>
#else
# include "utility.h"
#endif
#include <string>

#if !defined(HAVE_IMPLICIT_NAMESPACE)
using namespace std;
#endif

/**
 * Class used to decode character data from a legacy character set. It converts
 * it into Unicode for internal use.
 */
class Decoder
{
public:
    /**
     * Identify the character set used in a Fidonet text and return the
     * appropriate Decoder object.
     * @param kludges Kludges for this message.
     * @return Decoder object for this character set. NULL if unknown.
     */
    static Decoder *GetDecoderByKludges(const char *kludges);

    /**
     * Identify the character set used in a Usenet text and return the
     * appropriate Decoder object.
     * @param headers Headers for this message.
     * @return Decoder object for this character set. NULL if unknown.
     */
    static Decoder *GetDecoderByMIMEHeaders(const char *headers);

    /**
     * Get a Decoder object for converting into Unicode for a specified
     * character set. The name can be given using the Fidonet or MIME
     * name of the character set. If in conflict, the Fidonet name takes
     * precedence.
     * @param charset Name of character set.
     * @return Decoder object for this character set.
     */
    static Decoder *GetDecoderByName(const char *charset);

    /**
     * Decode character data.
     * @param input Legacy encoded data to decode.
     * @return Unicode version of data.
     */
    virtual wstring Decode(const string &input) = 0;
};

struct reversemap;

/**
 * Class used to encode character data into a legacy character set. It converts
 * it from Unicode for external use.
 */
class Encoder
{
public:
    /**
     * Get an Encoder object for converting from Unicode to a specified
     * character set. The name can be given using the Fidonet or MIME
     * name of the character set. If in conflict, the Fidonet name takes
     * precedence.
     * @param charset Name of character set.
     * @return Encoder object for this character set.
     */
    static Encoder *GetEncoderByName(const string &charset);

    /**
     * Encode character data.
     * @param input Unicode data to encode.
     * @return Legacy version of data.
     */
    virtual string Encode(const wstring &input) = 0;
};

struct tablemap;

/**
 * Class used to enumerate all available character sets.
 */
class CharsetEnumerator
{
public:
    /**
     * Selection of available sets of character set names.
     */
    enum names_e { Fidonet, Usenet };

    /**
     * Create a character set enumerator object, enumerating the character
     * sets for the selected name space.
     * @param namespace Which character set names to use.
     */
    CharsetEnumerator(names_e);

    /**
     * Get next character set name in the enumeration list.
     * @return NULL when end of list is reached, otherwise name of next
     *         character set.
     */
    const char *Next();

private:
    /** Pointer to character set map we are enumerating. */
    const struct tablemap *map;

    /** Current enumeration index. */
    int current;
};

#endif
