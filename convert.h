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

#ifndef __CONVERT_H
#define __CONVERT_H

#ifdef HAVE_WORKING_WSTRING
# define __ENABLE_WSTRING
# include <wchar.h>
#else
# include "utility.h"
#endif
#include <string>

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
    Decoder *GetDecoderByName(const char *charset);

    /**
     * Decode character data.
     * @param input Legacy encoded data to decode.
     * @return Unicode version of data.
     */
    wstring Decode(const string &input);

private:
    /** Private constructor for internal use only. */
    Decoder(const unsigned short *map) : inmap(map) {};

    /** Conversion table. */
    const unsigned short *inmap;
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
    string Encode(const wstring &input);

private:
    /** Private constructor for internal use only. */
    Encoder(const struct reversemap *map) : outmap(map) {};

    /** Conversion table. */
    const struct reversemap *outmap;
};

#endif
