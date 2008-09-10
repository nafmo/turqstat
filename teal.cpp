// Teal - Text Encoder ALgorithm :-)
//
// A small utility that converts text files from one character set to another
// using Turquoise SuperStat conversion routines
// Version 1.0
//
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
#include <iostream>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_GETOPT_IN_UNISTD
# include <unistd.h>
#elif defined(HAVE_GETOPT_IN_GETOPT)
# include <getopt.h>
#else
# include "utility.h"
#endif

#include "convert.h"

int main(int argc, char *argv[])
{
    const char *input = "-", *output = "-",
#if defined(__EMX__) || defined(__CYGWIN__) || defined (__MINGW32__)
               *inset = "iso-8859-1", *outset = "ibm850";
#else
               *inset = "ibm850", *outset = "iso-8859-1";
#endif
    int verbose = 1;
    bool listsets = false;

    // Handle arguments
    int c;
    while (EOF != (c = getopt(argc, argv, "i:o:f:t:ql")))
    {
        switch (c)
        {
            case 'i': input  = optarg; break;
            case 'o': output = optarg; break;
            case 'q': verbose --;      break;
            case 'f': inset  = optarg; break;
            case 't': outset = optarg; break;
            case 'l': listsets = true; break;
            default:
                cout << "Usage: " << argv[0] <<
                        " [-q] [-f from-charset] [-t to-charset]"
                        " [-i infile] [-o outfile]"
                     << endl;
                cout << "       " << argv[0] << " -l" << endl;
                return 0;
        }
    }

    if (listsets)
    {
        cout << "Known Fidonet character set names: " << endl;
        CharsetEnumerator fidonames(CharsetEnumerator::Fidonet);
        const char *name_p;
        while ((name_p = fidonames.Next())) cout << name_p << ' ';
        cout << endl;
        cout << endl;

        cout << "Known MIME character set names: " << endl;
        CharsetEnumerator mimenames(CharsetEnumerator::Usenet);
        while ((name_p = mimenames.Next())) cout << name_p << ' ';
        cout << endl;
        cout << endl;

        return 0;
    }

    if (argc > optind)
    {
        input = argv[optind];
        if (argc > optind + 1)
        {
            output = argv[optind + 1];
        }
    }

    // Create decoder and encoder objects
    Decoder *decoder_p = Decoder::GetDecoderByName(inset);
    Encoder *encoder_p = Encoder::GetEncoderByName(outset);

    // Open files (or connect to stdin/stdout)
    FILE *in, *out;
    if (strcmp(input, "-"))
    {
        in = fopen(input, "r");
        if (!in)
        {
            perror("Cannot open input");
            return 1;
        }
        if (verbose > 0)
        {
            fprintf(stderr, "Converting \"%s\"(%s)", input, inset);
        }
    }
    else
    {
        in = stdin;
        if (verbose > 0)
        {
            fprintf(stderr, "Converting standard input(%s)", inset);
        }
    }
    if (strcmp(output, "-"))
    {
        out = fopen(output, "w");
        if (!out)
        {
            perror("Cannot open output");
            return 1;
        }
        if (verbose > 0)
        {
            fprintf(stderr, " to \"%s\"(%s)\n", output, outset);
        }
    }
    else
    {
        out = stdout;
        if (verbose > 0)
        {
            fprintf(stderr, " to standard output(%s)", outset);
        }
    }

    // Convert
    char buf[1024];
    int len;
    while (0 != (len = fread(buf, 1, 1024, in)))
    {
        fputs(encoder_p->Encode(decoder_p->Decode(string(buf, len))).c_str(),
              out);
    }

    fclose(in);
    fclose(out);
}
