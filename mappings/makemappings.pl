#!/usr/bin/perl -w
#
# Copyright (c) 1998-2001 Peter Karlsson
#
# $Id$
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 2
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# Create C++ mapping tables from source files obtained from www.unicode.org
# and other sources.

use integer;

# List of files and tables to generate from them.
%arrayname = (
	"8859-1.txt" => "iso_8859_1",
	"8859-2.txt" => "iso_8859_2",
	"8859-3.txt" => "iso_8859_3",
	"8859-4.txt" => "iso_8859_4",
	"8859-5.txt" => "iso_8859_5",
	"8859-6.txt" => "iso_8859_6",
	"8859-7.txt" => "iso_8859_7",
	"8859-8.txt" => "iso_8859_8",
	"8859-9.txt" => "iso_8859_9",
	"8859-10.txt" => "iso_8859_10",
	"8859-13.txt" => "iso_8859_13",
	"8859-14.txt" => "iso_8859_14",
	"8859-15.txt" => "iso_8859_15",
	"cp437.txt" => "cp_437",
	"cp737.txt" => "cp_737",
	"cp775.txt" => "cp_775",
	"cp850.txt" => "cp_850",
	"cp852.txt" => "cp_852",
	"cp857.txt" => "cp_857",
	"cp860.txt" => "cp_860",
	"cp861.txt" => "cp_861",
	"cp862.txt" => "cp_862",
	"cp863.txt" => "cp_863",
	"cp864.txt" => "cp_864",
	"cp865.txt" => "cp_865",
	"cp866.txt" => "cp_866",
	"cp869.txt" => "cp_869",
	"cp874.txt" => "cp_874",
	"cp1250.txt" => "cp_1250",
	"cp1251.txt" => "cp_1251",
	"cp1252.txt" => "cp_1252",
	"cp1253.txt" => "cp_1253",
	"cp1254.txt" => "cp_1254",
	"cp1255.txt" => "cp_1255",
	"cp1256.txt" => "cp_1256",
	"cp1257.txt" => "cp_1257",
	"cp1258.txt" => "cp_1258",
	"cp10000.txt" => "cp_10000",
	"koi8-r.txt" => "koi8r",
	"koi8-u.txt" => "koi8u",
	"roman.txt" => "macroman",
	"iso-ir-11.txt" => "iso_ir_11",
	"iso-ir-60.txt" => "iso_ir_60",
);

# List of character table names (used for display).
%name = (
	"8859-1.txt" => "ISO 8859-1",
	"8859-2.txt" => "ISO 8859-2",
	"8859-3.txt" => "ISO 8859-3",
	"8859-4.txt" => "ISO 8859-4",
	"8859-5.txt" => "ISO 8859-5",
	"8859-6.txt" => "ISO 8859-6",
	"8859-7.txt" => "ISO 8859-7",
	"8859-8.txt" => "ISO 8859-8",
	"8859-9.txt" => "ISO 8859-9",
	"8859-10.txt" => "ISO 8859-10",
	"8859-13.txt" => "ISO 8859-13",
	"8859-14.txt" => "ISO 8859-14",
	"8859-15.txt" => "ISO 8859-15",
	"cp437.txt" => "MS-DOS codepage 437",
	"cp737.txt" => "MS-DOS codepage 737",
	"cp775.txt" => "MS-DOS codepage 775",
	"cp850.txt" => "MS-DOS codepage 850",
	"cp852.txt" => "MS-DOS codepage 852",
	"cp857.txt" => "MS-DOS codepage 857",
	"cp860.txt" => "MS-DOS codepage 860",
	"cp861.txt" => "MS-DOS codepage 861",
	"cp862.txt" => "MS-DOS codepage 862",
	"cp863.txt" => "MS-DOS codepage 863",
	"cp864.txt" => "MS-DOS codepage 864",
	"cp865.txt" => "MS-DOS codepage 865",
	"cp866.txt" => "MS-DOS codepage 866",
	"cp869.txt" => "MS-DOS codepage 869",
	"cp874.txt" => "MS-DOS codepage 874",
	"cp1250.txt" => "Windows codepage 1250",
	"cp1251.txt" => "Windows codepage 1251",
	"cp1252.txt" => "Windows codepage 1252",
	"cp1253.txt" => "Windows codepage 1253",
	"cp1254.txt" => "Windows codepage 1254",
	"cp1255.txt" => "Windows codepage 1255",
	"cp1256.txt" => "Windows codepage 1256",
	"cp1257.txt" => "Windows codepage 1257",
	"cp1258.txt" => "Windows codepage 1258",
	"cp10000.txt" => "Windows codepage 10000 (MacRoman)",
	"koi8-r.txt" => "KOI8-R",
	"koi8-u.txt" => "KOI8-U",
	"roman.txt" => "MacRoman",
	"iso-ir-11.txt" => "ISO-IR-11 (Swedish/Finnish 7-bit)",
	"iso-ir-60.txt" => "ISO-IR-60 (Norwegian/Danish 7-bit)",
);

# Create output files.
open MAPPINGS, ">../mappings.cpp"
	or die "Unable to create mappings.cpp: $!\n";

open HEADER, ">../mappings.h"
	or die "Unable to create mappings.h: $!\n";

print MAPPINGS << 'EOM';
// Copyright (c) 2001 Peter Karlsson
//
// This file is auto-generated by makemappings.pl
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

#include "mappings.h"

EOM

print HEADER << 'EOM';
// Copyright (c) 2001 Peter Karlsson
//
// This file is auto-generated by makemappings.pl
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

#ifndef __MAPPINGS_H
#define __MAPPINGS_H

/** Structure used to represent a mapping from Unicode. */
struct reversemap
{
    unsigned short unicode; ///< Unicode code point.
    char legacy;            ///< Value in legacy encoding.
};

EOM

foreach $file (sort keys %arrayname)
{
	# Sanity check.
	die "Error in script: No name defined for $file"
		unless defined $name{$file};

	# Load table.
	open MAPFILE, $file
		or die "Unable to read $file: $!\n";
	my (%inmap, %outmap, %duplicate);
	my $is7bit = 0;
	$is7bit = 1 if $file =~ /iso-ir/;

	# Read file data.
	print "Reading $file ($name{$file})...\n";
	my $codepoints = 0;
	while (<MAPFILE>)
	{
		next if /^#/;
		if (/0x(..)\s*0x(....)/)
		{
			my ($legacy, $unicode) = (hex($1), hex($2));
			if (defined $inmap{$legacy})
			{
				$duplicate{$legacy} = 1;
			}
			else
			{
				$outmap{$unicode} = $legacy;
			}
			$inmap{$legacy} = $unicode;
			$codepoints ++;
		}
	}
	close MAPFILE;

	# Write mapping table for charset to Unicode.
	my $inname = $arrayname{$file};
	print HEADER '/** Incoming conversion table for ', $name{$file}, " */\n";
	print HEADER 'extern const unsigned short ', $inname, "[256];\n";
	print MAPPINGS 'const unsigned short ', $inname, "[256] =\n{";
	foreach $codepoint (0..255)
	{
		if (!defined $inmap{$codepoint} &&
		    ($codepoint < 128 || ($codepoint < 160 && !$is7bit)))
		{
			# Fill in ASCII and control codes if not in mapping file.
			$inmap{$codepoint} = $codepoint;
			$outmap{$codepoint} = $codepoint;
		}

		print MAPPINGS "\n    " if ($codepoint & 0x07) == 0;

		if ($is7bit && $codepoint > 128 && defined $inmap{$codepoint & 0x7F})
		{
			print MAPPINGS $inmap{$codepoint & 0x7F};
		}
		elsif (defined $inmap{$codepoint})
		{
			print MAPPINGS $inmap{$codepoint};
		}
		else
		{
			print MAPPINGS 0;
		}
		print MAPPINGS ',' unless $codepoint == 255;
		print MAPPINGS ' ' unless ($codepoint & 0x07) == 7;
	}
	print MAPPINGS "\n};\n\n";

	# Write mapping table for Unicode to charset.
	my $outname = $arrayname{$file} . "_rev";
	print HEADER '/** Outgoing conversion table for ', $name{$file}, " */\n";
	print HEADER 'extern const struct reversemap ', $outname, "[128];\n\n";
	print MAPPINGS 'const struct reversemap ', $outname, "[128] =\n{\n";
	my $count = 0;
	my @mapped = sort { $a <=> $b } grep { $_ >= 128 } keys %outmap;
	my $prev = 127;

	foreach (0 .. 126 - $#mapped)
	{
		print MAPPINGS "    {     0,   0 }, // Filler\n";
		$count ++;
	}

	foreach $codepoint (@mapped)
	{
		printf MAPPINGS "    { %5d, %3d }",
		       $codepoint, $outmap{$codepoint};
		print MAPPINGS ",\n" if ++ $count < 128;
	}
	print MAPPINGS "\n};\n\n";
}

# Finish up.

print HEADER "#endif\n";
close HEADER;
close MAPPINGS;