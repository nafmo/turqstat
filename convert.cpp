// Copyright (c) 2001-2008 Peter Krefting
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

#include <string.h>
#include <stdlib.h>

#include "convert.h"
#include "mappings.h"
#include "utility.h"

/** Structure mapping charset names to mapping tables. */
struct tablemap
{
    const char * const      charset;
    const unsigned short    *inmap;
    const struct reversemap *outmap;
    const unsigned short    outmap_length;
};

/** Lookup table for Fidonet charset identifiers. */
const struct tablemap fidocharsets[] =
{
    // Default charset goes first
    { "IBMPC", cp_437, cp_437_rev, cp_437_rev_len },
    { "LATIN-1", iso_8859_1, iso_8859_1_rev, iso_8859_1_rev_len },
    { "LATIN-2", iso_8859_2, iso_8859_2_rev, iso_8859_2_rev_len },
    { "LATIN-3", iso_8859_3, iso_8859_3_rev, iso_8859_3_rev_len },
    { "LATIN-4", iso_8859_4, iso_8859_4_rev, iso_8859_4_rev_len },
    { "LATIN-5", iso_8859_9, iso_8859_9_rev, iso_8859_9_rev_len },
    { "LATIN-6", iso_8859_10, iso_8859_10_rev, iso_8859_10_rev_len },
    { "LATIN-7", iso_8859_13, iso_8859_13_rev, iso_8859_13_rev_len },
    { "LATIN-8", iso_8859_14, iso_8859_14_rev, iso_8859_14_rev_len },
    { "LATIN-9", iso_8859_15, iso_8859_15_rev, iso_8859_15_rev_len },
    { "LATIN-10", iso_8859_16, iso_8859_16_rev, iso_8859_16_rev_len },
    { "CP437", cp_437, cp_437_rev, cp_437_rev_len },
    { "CP737", cp_737, cp_737_rev, cp_737_rev_len },
    { "CP775", cp_775, cp_775_rev, cp_775_rev_len },
    { "CP850", cp_850, cp_850_rev, cp_850_rev_len },
    { "CP852", cp_852, cp_852_rev, cp_852_rev_len },
    { "CP857", cp_857, cp_857_rev, cp_857_rev_len },
    { "CP860", cp_860, cp_860_rev, cp_860_rev_len },
    { "CP861", cp_861, cp_861_rev, cp_861_rev_len },
    { "CP862", cp_862, cp_862_rev, cp_862_rev_len },
    { "CP863", cp_863, cp_863_rev, cp_863_rev_len },
    { "CP864", cp_864, cp_864_rev, cp_864_rev_len },
    { "CP865", cp_865, cp_865_rev, cp_865_rev_len },
    { "CP866", cp_866, cp_866_rev, cp_866_rev_len },
    { "+7_FIDO", cp_866, cp_866_rev, cp_866_rev_len },
    { "CP869", cp_869, cp_869_rev, cp_869_rev_len },
    { "CP874", cp_874, cp_874_rev, cp_874_rev_len },
    { "CP1250", cp_1250, cp_1250_rev, cp_1250_rev_len },
    { "CP1251", cp_1251, cp_1251_rev, cp_1251_rev_len },
    { "CP1252", cp_1252, cp_1252_rev, cp_1252_rev_len },
    { "CP1253", cp_1253, cp_1253_rev, cp_1253_rev_len },
    { "CP1254", cp_1254, cp_1254_rev, cp_1254_rev_len },
    { "CP1255", cp_1255, cp_1255_rev, cp_1255_rev_len },
    { "CP1256", cp_1256, cp_1256_rev, cp_1256_rev_len },
    { "CP1257", cp_1257, cp_1257_rev, cp_1257_rev_len },
    { "CP1258", cp_1258, cp_1258_rev, cp_1258_rev_len },
    { "CP10000", cp_10000, cp_10000_rev, cp_10000_rev_len },
    { "SWEDISH", iso_ir_11, iso_ir_11_rev, iso_ir_11_rev_len },
    { "FINNISH", iso_ir_11, iso_ir_11_rev, iso_ir_11_rev_len },
    { "ISO-10", iso_ir_11, iso_ir_11_rev, iso_ir_11_rev_len }, // Bug, ISO-10 is not identical
    { "NORWEG", iso_ir_60, iso_ir_60_rev, iso_ir_60_rev_len },
    { "DANISH", iso_ir_60, iso_ir_60_rev, iso_ir_60_rev_len },
    // DUTCH, FRENCH, CANADIAN, GERMAN, ITALIAN, PORTU, SPANISH, SWISS, UK
    // are not supported (defined in FSC-0054)
    { "KOI8-R", koi8r, koi8r_rev, koi8r_rev_len },
    { "KOI8-U", koi8u, koi8u_rev, koi8u_rev_len },
    { "MAC", macroman, macroman_rev, macroman_rev_len },
    // Sentinel
    { NULL, NULL, NULL, 0 }
};

/** Lookup table for Usenet charset identifiers. */
const struct tablemap usenetcharsets[] =
{
    // Default charset goes first
    { "iso-8859-1", iso_8859_1, iso_8859_1_rev, iso_8859_1_rev_len },
    { "us-ascii", iso_8859_1, iso_8859_1_rev, iso_8859_1_rev_len }, // Well...
    { "iso-8859-2", iso_8859_2, iso_8859_2_rev, iso_8859_2_rev_len },
    { "iso-8859-3", iso_8859_3, iso_8859_3_rev, iso_8859_3_rev_len },
    { "iso-8859-4", iso_8859_4, iso_8859_4_rev, iso_8859_4_rev_len },
    { "iso-8859-5", iso_8859_5, iso_8859_5_rev, iso_8859_5_rev_len },
    { "iso-8859-6", iso_8859_6, iso_8859_6_rev, iso_8859_6_rev_len },
    { "iso-8859-7", iso_8859_7, iso_8859_7_rev, iso_8859_7_rev_len },
    { "iso-8859-8", iso_8859_8, iso_8859_8_rev, iso_8859_8_rev_len },
    { "iso-8859-9", iso_8859_9, iso_8859_9_rev, iso_8859_9_rev_len },
    { "iso-8859-10", iso_8859_10, iso_8859_10_rev, iso_8859_10_rev_len },
    { "iso-8859-11", iso_8859_11, iso_8859_11_rev, iso_8859_11_rev_len },
    { "iso-8859-13", iso_8859_13, iso_8859_13_rev, iso_8859_13_rev_len },
    { "iso-8859-14", iso_8859_14, iso_8859_14_rev, iso_8859_14_rev_len },
    { "iso-8859-15", iso_8859_15, iso_8859_15_rev, iso_8859_15_rev_len },
    { "iso-8859-16", iso_8859_16, iso_8859_16_rev, iso_8859_16_rev_len },
    { "ibm437", cp_437, cp_437_rev, cp_437_rev_len },
    { "ibm737", cp_737, cp_737_rev, cp_737_rev_len },
    { "ibm775", cp_775, cp_775_rev, cp_775_rev_len },
    { "ibm850", cp_850, cp_850_rev, cp_850_rev_len },
    { "ibm852", cp_852, cp_852_rev, cp_852_rev_len },
    { "ibm857", cp_857, cp_857_rev, cp_857_rev_len },
    { "ibm860", cp_860, cp_860_rev, cp_860_rev_len },
    { "ibm861", cp_861, cp_861_rev, cp_861_rev_len },
    { "ibm862", cp_862, cp_862_rev, cp_862_rev_len },
    { "ibm863", cp_863, cp_863_rev, cp_863_rev_len },
    { "ibm864", cp_864, cp_864_rev, cp_864_rev_len },
    { "ibm865", cp_865, cp_865_rev, cp_865_rev_len },
    { "ibm866", cp_866, cp_866_rev, cp_866_rev_len },
    { "ibm869", cp_869, cp_869_rev, cp_869_rev_len },
    { "ibm874", cp_874, cp_874_rev, cp_874_rev_len },
    { "windows-1250", cp_1250, cp_1250_rev, cp_1250_rev_len },
    { "windows-1251", cp_1251, cp_1251_rev, cp_1251_rev_len },
    { "windows-1252", cp_1252, cp_1252_rev, cp_1252_rev_len },
    { "windows-1253", cp_1253, cp_1253_rev, cp_1253_rev_len },
    { "windows-1254", cp_1254, cp_1254_rev, cp_1254_rev_len },
    { "windows-1255", cp_1255, cp_1255_rev, cp_1255_rev_len },
    { "windows-1256", cp_1256, cp_1256_rev, cp_1256_rev_len },
    { "windows-1257", cp_1257, cp_1257_rev, cp_1257_rev_len },
    { "windows-1258", cp_1258, cp_1258_rev, cp_1258_rev_len },
    { "windows-10000", cp_10000, cp_10000_rev, cp_10000_rev_len },
    { "iso-ir-11", iso_ir_11, iso_ir_11_rev, iso_ir_11_rev_len },
    { "iso-ir-60", iso_ir_60, iso_ir_60_rev, iso_ir_60_rev_len },
    { "koi8-r", koi8r, koi8r_rev, koi8r_rev_len },
    { "koi8-u", koi8u, koi8u_rev, koi8u_rev_len },
    { "macintosh", macroman, macroman_rev, macroman_rev_len },
    // Sentinel
    { NULL, NULL, NULL, 0 }
};

/**
 * Class used to decode SBCS data. It converts it into wchar_t Unicode for
 * internal use.
 */
class SBCSDecoder : public Decoder
{
public:
    /**
     * Decode character data.
     * @param input Legacy encoded data to decode.
     * @return Unicode version of data.
     */
    virtual wstring Decode(const string &input);

    /** Constructor used only from Decoder class methods. */
    SBCSDecoder(const unsigned short *map) : inmap(map) {};

protected:
    /** Conversion table. */
    const unsigned short *inmap;
};

/**
 * Class used to decode UTF-8 data. It converts it into wchar_t Unicode for
 * internal use.
 */
class UTF8Decoder : public Decoder
{
public:
    /**
     * Decode character data.
     * @param input Legacy encoded data to decode.
     * @return Unicode version of data.
     */
    virtual wstring Decode(const string &input);
};

// Get Decoder object for a character set
Decoder *Decoder::GetDecoderByName(const char *charset)
{
    // Try Fidonet names
    for (int i = 0; fidocharsets[i].charset; i ++)
    {
#if defined(HAVE_STRCASECMP)
        if (0 == strcasecmp(charset, fidocharsets[i].charset))
#elif defined(HAVE_STRICMP)
        if (0 == stricmp(charset, fidocharsets[i].charset))
#else
        if (0 == fcompare(charset, fidocharsets[i].charset))
#endif
        {
            return new SBCSDecoder(fidocharsets[i].inmap);
        }
    }

    // Try MIME names
    for (int i = 0; usenetcharsets[i].charset; i ++)
    {
#if defined(HAVE_STRCASECMP)
        if (0 == strcasecmp(charset, usenetcharsets[i].charset))
#elif defined(HAVE_STRICMP)
        if (0 == stricmp(charset, usenetcharsets[i].charset))
#else
        if (0 == fcompare(charset, usenetcharsets[i].charset))
#endif
        {
            return new SBCSDecoder(usenetcharsets[i].inmap);
        }
    }

    // Check for UTF-8
#if defined(HAVE_STRCASECMP)
    if (0 == strcasecmp(charset, "UTF-8"))
#elif defined(HAVE_STRICMP)
    if (0 == stricmp(charset, "UTF-8"))
#else
    if (0 == fcompare(charset, "UTF-8"))
#endif
    {
        return new UTF8Decoder();
    }

    // Return first in MIME table as default
    return new SBCSDecoder(usenetcharsets[0].inmap);
}

// Identify Fidonet character set from kludges
Decoder *Decoder::GetDecoderByKludges(const char *kludges)
{
    // Find CHRS kludge
    const char *chrs = kludges ? strstr(kludges, "\x01""CHRS: ") : NULL;
    if (!chrs)
    {
        // Return first in Fido table
        return new SBCSDecoder(fidocharsets[0].inmap);
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

    string charset((chrs + 7), next - chrs - 7);

    // Test for special case: IBMPC may have CODEPAGE specifier (FDAPX/w)
    if (0 == fcompare(charset, "IBMPC"))
    {
        const char *codepage = kludges ? strstr(kludges, "\x01""CODEPAGE: ") : NULL;
        if (codepage)
        {
            // Isolate codepage number
            const char *next = strchr(codepage + 11, 1);
            if (!next)
            {
                next = kludges + strlen(kludges);
            }

            // MS-DOS codepage number is always three-digit
            if (next - codepage != 3)
            {
                string cpnum((codepage + 11), next - codepage - 11);
                charset = "CP" + cpnum;
            }
        }
    }

    // Search for it
    for (int i = 0; fidocharsets[i].charset; i ++)
    {
        if (0 == fcompare(charset, fidocharsets[i].charset))
        {
            return new SBCSDecoder(fidocharsets[i].inmap);
        }
    }

    // Check for UTF-8
    if (0 == fcompare(charset, "UTF-8"))
    {
        return new UTF8Decoder();
    }

    // Return first in Fido table
    return new SBCSDecoder(fidocharsets[0].inmap);
}

// Identify Usenet character set from headers
Decoder *Decoder::GetDecoderByMIMEHeaders(const char *headers)
{
    // Find charset identifier
#if defined(HAVE_STRISTR)
    char *charset = headers ? stristr(headers, "charset=") : NULL;
#else
    const char *charset = NULL;
    if (headers)
    {
#if defined(HAVE_STRLWR)
        char *lowercaseheaders = strlwr(strdup(headers));
#else
        char *lowercaseheaders = strdup(headers);
        for (char *p = lowercaseheaders; *p; p ++)
        {
            *p = tolower(*p);
        }
#endif
        char *find = strstr(lowercaseheaders, "charset=");
        if (find)
            charset = headers + (find - lowercaseheaders);
        free(lowercaseheaders);
    }
#endif

    if (!charset)
    {
        // Return first in MIME table
        return new SBCSDecoder(usenetcharsets[0].inmap);
    }

    // Isolate charset parameter
    const char *start = NULL;
    const char *next = headers + strlen(headers);
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
    }

    const char *p;
    if (NULL != (p = strchr(start, ' ')))
    {
        if (p < next)
            next = p;
    }
    if (NULL != (p = strchr(start, 1)))
    {
        if (p < next)
            next = p;
    }
    if (NULL != (p = strchr(start, ',')))
    {
        if (p < next)
            next = p;
    }

    string charsetstring(start, next - start);

    // Search for it
    for (int i = 0; usenetcharsets[i].charset; i ++)
    {
        if (0 == fcompare(charsetstring, usenetcharsets[i].charset))
        {
            return new SBCSDecoder(usenetcharsets[i].inmap);
        }
    }

    // Check for UTF-8
    if (0 == fcompare(charset, "UTF-8"))
    {
        return new UTF8Decoder();
    }

    // Return first in MIME table
    return new SBCSDecoder(usenetcharsets[0].inmap);
}

// Decode legacy character string
wstring SBCSDecoder::Decode(const string &input)
{
#ifdef HAVE_WORKING_WSTRING
    wstring s;
#else
    wstring s(input.length() + 1);
#endif

    const unsigned char *i = (const unsigned char *) input.c_str();
    while (*i)
    {
        // Get Unicode codepoints
        wchar_t ucs = inmap[*i];

        if (ucs)
        {
#ifdef HAVE_WORKING_WSTRING
            s += ucs;
#else
            s.append(ucs);
#endif
        }

        i ++;
    }

    return s;
}

// Decode UTF-8 character string
wstring UTF8Decoder::Decode(const string &input)
{
#ifdef HAVE_WORKING_WSTRING
    wstring s;
#else
    wstring s(input.length() + 1);
#endif

    const unsigned char *i = (const unsigned char *) input.c_str();
    while (*i)
    {
        wchar_t ucs = 0;
        if (0 == (*i & 0x80))
        {
            // ASCII
            ucs = wchar_t(*i);
        }
        else if (0xC0 == (*i & 0xE0))
        {
            // U+0080 - U+07FF
            ucs  = (*(i ++) & 0x1F) <<  6;
            ucs |=  *i      & 0x3F;
            if (ucs < 0x80)
            {
                ucs = '?';
            }
        }
        else if (0xE0 == (*i & 0xF0))
        {
            // U+0800 - U+FFFF
            ucs  = (*(i ++) & 0x0F) << 12;
            ucs |= (*(i ++) & 0x3F) <<  6;
            ucs |=  *i      & 0x3F;
            if (ucs < 0x800 || (ucs >= 0xD800 && ucs <= 0xDFFF))
            {
                ucs = '?';
            }
        }
        else if (0xF0 == (*i & 0xF8))
        {
            // U+10000 - U+1FFFFF
            ucs  = (*(i ++) & 0x07) << 18;
            ucs |= (*(i ++) & 0x3F) << 12;
            ucs |= (*(i ++) & 0x3F) <<  6;
            ucs |=  *i      & 0x3F;
            if (ucs < 0x10000)
            {
                ucs = '?';
            }
        }
        else if (0xF8 == (*i & 0xFC))
        {
            // U+200000 - U+3FFFFFF
            ucs  = (*(i ++) & 0x03) << 24;
            ucs |= (*(i ++) & 0x3F) << 18;
            ucs |= (*(i ++) & 0x3F) << 12;
            ucs |= (*(i ++) & 0x3F) <<  6;
            ucs |=  *i      & 0x3F;
            if (ucs < 0x200000)
            {
                ucs = '?';
            }
        }
        else if (0xFC == (*i & 0xFE))
        {
            // U+4000000 - U+7FFFFFFF
            ucs  = (*(i ++) & 0x01) << 30;
            ucs |= (*(i ++) & 0x3F) << 24;
            ucs |= (*(i ++) & 0x3F) << 18;
            ucs |= (*(i ++) & 0x3F) << 12;
            ucs |= (*(i ++) & 0x3F) <<  6;
            ucs |=  *i      & 0x3F;
            if (ucs < 0x4000000)
            {
                ucs = '?';
            }
        }

#ifdef HAVE_WORKING_WSTRING
        if (ucs) s += ucs;
#else
        if (ucs) s.append(ucs);
#endif

        i ++;
    }

    return s;
}

/**
 * Class used to encode character data into a legacy character set. It converts
 * it from Unicode for external use.
 */
class SBCSEncoder : public Encoder
{
public:
    /**
     * Encode character data.
     * @param input Unicode data to encode.
     * @return Legacy version of data.
     */
    virtual string Encode(const wstring &input);

    /** Constructor used only from Encoder class methods. */
    SBCSEncoder(const struct reversemap *map, unsigned short len)
        : outmap(map), maplength(len) {};

protected:
    /** Conversion table. */
    const struct reversemap *outmap;

    /** Length of conversion table. */
    unsigned short maplength;
};

// Get encoder object for a character set
Encoder *Encoder::GetEncoderByName(const string &charset)
{
    // Try Fidonet names
    for (int i = 0; fidocharsets[i].charset; i ++)
    {
#if defined(HAVE_STRCASECMP)
        if (0 == strcasecmp(charset.c_str(), fidocharsets[i].charset))
#elif defined(HAVE_STRICMP)
        if (0 == stricmp(charset.c_str(), fidocharsets[i].charset))
#else
        if (0 == fcompare(charset, fidocharsets[i].charset))
#endif
        {
            return new SBCSEncoder(fidocharsets[i].outmap,
                                  fidocharsets[i].outmap_length);
        }
    }

    // Try MIME names
    for (int i = 0; usenetcharsets[i].charset; i ++)
    {
#if defined(HAVE_STRCASECMP)
        if (0 == strcasecmp(charset.c_str(), usenetcharsets[i].charset))
#elif defined(HAVE_STRICMP)
        if (0 == stricmp(charset.c_str(), usenetcharsets[i].charset))
#else
        if (0 == fcompare(charset, usenetcharsets[i].charset))
#endif
        {
            return new SBCSEncoder(usenetcharsets[i].outmap,
                                   usenetcharsets[i].outmap_length);
        }
    }

    // Return first in MIME table as default
    return new SBCSEncoder(usenetcharsets[0].outmap,
                           usenetcharsets[0].outmap_length);
}

// Encode Unicode string
string SBCSEncoder::Encode(const wstring &input)
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
            s += static_cast<char>(ucs);
        }
        else
        {
            // Convert to a local codepoint by binary-searching the output
            // map.

            char result = 0;
            int low = 0, high = maplength - 1, mid;
            while (!result && low <= high)
            {
                mid = low + (high - low) / 2;
                if (ucs == outmap[mid].unicode)
                {
                    result = static_cast<char>(outmap[mid].legacy);
                }
                else if (ucs < outmap[mid].unicode)
                {
                    high = mid - 1;
                }
                else if (ucs > outmap[mid].unicode)
                {
                    low = mid + 1;
                }
            }

            if (result)
            {
                s += result;
            }
            else if (!result)
            {
                // Use fallback if character was not found
                low = 0;
                high = fallbackmap_len -1;
                const char *result = NULL;

                while (!result && low <= high)
                {
                    mid = low + (high - low) / 2;
                    if (ucs == fallback[mid].unicode)
                    {
                        result = fallback[mid].fallback;
                    }
                    else if (ucs < fallback[mid].unicode)
                    {
                        high = mid - 1;
                    }
                    else if (ucs > fallback[mid].unicode)
                    {
                        low = mid + 1;
                    }
                }

                if (result)
                {
                    s += result;
                }
                else
                {
                    // Give up and use a question mark
                    s += '?';
                }
            }
        }

        i ++;
    }

    return s;
}

CharsetEnumerator::CharsetEnumerator(names_e namelist)
    : current(0)
{
    switch (namelist)
    {
        case Fidonet:
            map = fidocharsets;
            break;

        case Usenet:
        default: // Just in case
            map = usenetcharsets;
            break;
    }
}

const char *CharsetEnumerator::Next()
{
    if (!map[current].charset)
    {
        // We're at the end
        return NULL;
    }

    return map[current ++].charset;
}
