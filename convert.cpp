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

#define _GNU_SOURCE
#include <string.h>

#include "convert.h"
#include "mappings.h"
#include "utility.h"

/** Structure mapping charset names to mapping tables. */
struct tablemap
{
    const char * const      charset;
    const unsigned short    *inmap;
    const struct reversemap *outmap;
};

/** Lookup table for Fidonet charset identifiers. */
const struct tablemap fidocharsets[] =
{
    // Default charset goes first
    { "IBMPC", cp_437, cp_437_rev },
    { "LATIN-1", iso_8859_1, iso_8859_1_rev },
    { "LATIN-2", iso_8859_2, iso_8859_2_rev },
    { "LATIN-3", iso_8859_3, iso_8859_3_rev },
    { "LATIN-4", iso_8859_4, iso_8859_4_rev },
    { "LATIN-5", iso_8859_9, iso_8859_9_rev },
    { "LATIN-6", iso_8859_10, iso_8859_10_rev },
    { "LATIN-7", iso_8859_13, iso_8859_13_rev },
    { "LATIN-8", iso_8859_14, iso_8859_14_rev },
    { "LATIN-9", iso_8859_15, iso_8859_15_rev },
    { "CP437", cp_437, cp_437_rev },
    { "CP737", cp_737, cp_737_rev },
    { "CP775", cp_775, cp_775_rev },
    { "CP850", cp_850, cp_850_rev },
    { "CP852", cp_852, cp_852_rev },
    { "CP857", cp_857, cp_857_rev },
    { "CP860", cp_860, cp_860_rev },
    { "CP861", cp_861, cp_861_rev },
    { "CP862", cp_862, cp_862_rev },
    { "CP863", cp_863, cp_863_rev },
    { "CP864", cp_864, cp_864_rev },
    { "CP865", cp_865, cp_865_rev },
    { "CP866", cp_866, cp_866_rev },
    { "+7_FIDO", cp_866, cp_866_rev },
    { "CP869", cp_869, cp_869_rev },
    { "CP874", cp_874, cp_874_rev },
    { "CP1250", cp_1250, cp_1250_rev },
    { "CP1251", cp_1251, cp_1251_rev },
    { "CP1252", cp_1252, cp_1252_rev },
    { "CP1253", cp_1253, cp_1253_rev },
    { "CP1254", cp_1254, cp_1254_rev },
    { "CP1255", cp_1255, cp_1255_rev },
    { "CP1256", cp_1256, cp_1256_rev },
    { "CP1257", cp_1257, cp_1257_rev },
    { "CP1258", cp_1258, cp_1258_rev },
    { "CP10000", cp_10000, cp_10000_rev },
    { "SWEDISH", iso_ir_11, iso_ir_11_rev },
    { "FINNISH", iso_ir_11, iso_ir_11_rev },
    { "ISO-10", iso_ir_11, iso_ir_11_rev }, // Bug, ISO-10 is not identical
    { "NORWEG", iso_ir_60, iso_ir_60_rev },
    { "DANISH", iso_ir_60, iso_ir_60_rev },
    // DUTCH, FRENCH, CANADIAN, GERMAN, ITALIAN, PORTU, SPANISH, SWISS, UK
    // are not supported (defined in FSC-0054)
    { "KOI8-R", koi8r, koi8r_rev },
    { "KOI8-R", koi8u, koi8u_rev },
    { "MAC", macroman, macroman_rev },
    // Sentinel
    { NULL, NULL, NULL }
};

/** Lookup table for Usenet charset identifiers. */
const struct tablemap usenetcharsets[] =
{
    // Default charset goes first
    { "iso-8859-1", iso_8859_1, iso_8859_1_rev },
    { "us-ascii", iso_8859_1, iso_8859_1_rev }, // Well...
    { "iso-8859-2", iso_8859_2, iso_8859_2_rev },
    { "iso-8859-3", iso_8859_3, iso_8859_3_rev },
    { "iso-8859-4", iso_8859_4, iso_8859_4_rev },
    { "iso-8859-9", iso_8859_9, iso_8859_9_rev },
    { "iso-8859-10", iso_8859_10, iso_8859_10_rev },
    { "iso-8859-13", iso_8859_13, iso_8859_13_rev },
    { "iso-8859-14", iso_8859_14, iso_8859_14_rev },
    { "iso-8859-15", iso_8859_15, iso_8859_15_rev },
    { "ibm437", cp_437, cp_437_rev },
    { "ibm737", cp_737, cp_737_rev },
    { "ibm775", cp_775, cp_775_rev },
    { "ibm850", cp_850, cp_850_rev },
    { "ibm852", cp_852, cp_852_rev },
    { "ibm857", cp_857, cp_857_rev },
    { "ibm860", cp_860, cp_860_rev },
    { "ibm861", cp_861, cp_861_rev },
    { "ibm862", cp_862, cp_862_rev },
    { "ibm863", cp_863, cp_863_rev },
    { "ibm864", cp_864, cp_864_rev },
    { "ibm865", cp_865, cp_865_rev },
    { "ibm866", cp_866, cp_866_rev },
    { "ibm869", cp_869, cp_869_rev },
    { "ibm874", cp_874, cp_874_rev },
    { "windows-1250", cp_1250, cp_1250_rev },
    { "windows-1251", cp_1251, cp_1251_rev },
    { "windows-1252", cp_1252, cp_1252_rev },
    { "windows-1253", cp_1253, cp_1253_rev },
    { "windows-1254", cp_1254, cp_1254_rev },
    { "windows-1255", cp_1255, cp_1255_rev },
    { "windows-1256", cp_1256, cp_1256_rev },
    { "windows-1257", cp_1257, cp_1257_rev },
    { "windows-1258", cp_1258, cp_1258_rev },
    { "windows-10000", cp_10000, cp_10000_rev },
    { "iso-ir-11", iso_ir_11, iso_ir_11_rev },
    { "iso-ir-60", iso_ir_60, iso_ir_60_rev },
    { "koi8-R", koi8r, koi8r_rev },
    { "koi8-R", koi8u, koi8u_rev },
    { "macintosh", macroman, macroman_rev },
    // Sentinel
    { NULL, NULL, NULL }
};

// Get Decoder object for a character set
Decoder *Decoder::GetDecoderByName(const char *charset)
{
    // Try Fidonet names
    for (int i = 0; fidocharsets[i].charset; i ++)
    {
        if (0 == strcasecmp(charset, fidocharsets[i].charset))
        {
            return new Decoder(fidocharsets[i].inmap);
        }
    }

    // Try MIME names
    for (int i = 0; usenetcharsets[i].charset; i ++)
    {
        if (0 == strcasecmp(charset, usenetcharsets[i].charset))
        {
            return new Decoder(usenetcharsets[i].inmap);
        }
    }

    // Return first in MIME table as default
    return new Decoder(usenetcharsets[0].inmap);
}

// Identify Fidonet character set from kludges
Decoder *Decoder::GetDecoderByKludges(const char *kludges)
{
    // Find CHRS kludge
    char *chrs = kludges ? strcasestr(kludges, "\x01""CHRS: ") : NULL;
    if (!chrs)
    {
        // Return first in Fido table
        return new Decoder(fidocharsets[0].inmap);
    }

    // Isolate kludge
    const char *next = strchr(chrs + 7, ' ');
    if (!next)
    {
        next = strchr(chrs + 7, 1);
    }
    if (!next)
    {
        next = kludges + strlen(kludges);
    }

    string charset((chrs + 7), next - chrs - 7 - 1);

    // Search for it
    for (int i = 0; fidocharsets[i].charset; i ++)
    {
        if (0 == fcompare(charset, fidocharsets[i].charset))
        {
            return new Decoder(fidocharsets[i].inmap);
        }
    }

    // Return first in Fido table
    return new Decoder(fidocharsets[0].inmap);
}

// Identify Usenet character set from headers
Decoder *Decoder::GetDecoderByMIMEHeaders(const char *headers)
{
    // Find charset identifier
    char *charset = headers ? strcasestr(headers, "charset=") : NULL;
    if (!charset)
    {
        // Return first in MIME table
        return new Decoder(usenetcharsets[0].inmap);
    }

    // Isolate charset parameter
    const char *last = headers + strlen(headers);
    const char *start = NULL;
    const char *next = NULL;
    char *lf = strchr(headers, 10) <? strchr(headers, 13);
    if ('"' == *(charset + 8))
    {
        start = charset + 9;
        next = strchr(start, '"');
    }
    else if ('\'' == *(charset + 8))
    {
        start = charset + 9;
        next = strchr(start, '\'');
    }
    else
    {
        start = charset + 8;
        while (isspace(*start))
        {
            start ++;
        }

        next = strchr(start, ' ') <? strchr(charset, ',');
    }

    if (!next)
    {
        next = lf ? (lf <? last) : last;
    }

    string charsetstring(start, next - start - 1);
    
    // Search for it
    for (int i = 0; usenetcharsets[i].charset; i ++)
    {
        if (0 == fcompare(charsetstring, usenetcharsets[i].charset))
        {
            return new Decoder(usenetcharsets[i].inmap);
        }
    }

    // Return first in MIME table
    return new Decoder(usenetcharsets[0].inmap);
}

// Decode legacy character string
wstring Decoder::Decode(const string &input)
{
#ifdef HAVE_WORKING_WSTRING
    wstring s;
#else
    wstring s(strlen(input.c_str()) + 1);
#endif

    const unsigned char *i = (const unsigned char *) input.c_str();
    while (*i)
    {
        // Get Unicode codepoints
        wchar_t ucs = inmap[*i];

        if (ucs)
        {
            s += ucs;
        }

        i ++;
    }

    return s;
}

// Get encoder object for a character set
Encoder *Encoder::GetEncoderByName(const string &charset)
{
    // Try Fidonet names
    for (int i = 0; fidocharsets[i].charset; i ++)
    {
        if (0 == strcasecmp(charset.c_str(), fidocharsets[i].charset))
        {
            return new Encoder(fidocharsets[i].outmap);
        }
    }

    // Try MIME names
    for (int i = 0; usenetcharsets[i].charset; i ++)
    {
        if (0 == strcasecmp(charset.c_str(), usenetcharsets[i].charset))
        {
            return new Encoder(usenetcharsets[i].outmap);
        }
    }

    // Return first in MIME table as default
    return new Encoder(usenetcharsets[0].outmap);
}

// Encode Unicode string
string Encoder::Encode(const wstring &input)
{
    string s;

    const wchar_t *i = input.c_str();
    while (*i)
    {
        // Get Unicode codepoints
        unsigned short ucs = *i;

        if (ucs < 128)
        {
            // ASCII always map to ASCII, even for 7-bit character sets
            // (this means that a backslash in input will look wrong in
            // output for iso-ir-11, but we can live with that)
            s += char(ucs);
        }
        else
        {
            // Convert to a local codepoint by binary-searching the output
            // map.

            char result = 0;
            int low = 0, high = 127, mid;
            while (!result)
            {
                mid = low + (high - low) / 2;
                if (ucs == outmap[mid].unicode)
                {
                    result = outmap[mid].legacy;
                }
                else if (ucs < outmap[mid].unicode)
                {
                    high = mid - 1;
                }
                else if (ucs > outmap[mid].unicode)
                {
                    low = mid + 1;
                }

                // Check for error condition
                if (low > high)
                {
                    result = '?';
                }
            }
            s += result;
        }

        i ++;
    }

    return s;
}