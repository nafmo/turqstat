// Teal
//
// A small utility that converts text files from one character set to another
// using Turquoise SuperStat conversion routines
// Version 1.0
//
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

#include <config.h>
#include <iostream>
#include <stdio.h>
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
               *inset = "ibm437", *outset = "iso-8859-1";

    // Handle arguments
    int c;
    while (EOF != (c = getopt(argc, argv, "i:o:f:t:")))
    {
        switch (c)
        {
            case 'i': input  = optarg; break;
            case 'o': output = optarg; break;
            case 'f': inset  = optarg; break;
            case 't': outset = optarg; break;
            default:
                cout << "Usage: " << argv[0] <<
                        " [-f from-charset] [-t to-charset]"
                        " [-i infile] [-o outfile]"
                     << endl;
                return 0;
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
    }
    else
    {
        in = stdin;
    }
    if (strcmp(output, "-"))
    {
        out = fopen(output, "w");
    }
    else
    {
        out = stdout;
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
