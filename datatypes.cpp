// Copyright (c) 2001 Peter Krefting
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
#if defined(WORDS_BIGENDIAN)
# include "datatypes.h"

le_uint16_t &le_uint16_t::operator=(uint16_t x)
{
    data[0] =  x       & 0xff;
    data[1] = (x >> 8) & 0xff;
    return *this;
}

le_uint16_t::operator uint16_t()
{
    return (data[1] << 8) | data[0];
}

le_int16_t &le_int16_t::operator=(int16_t x)
{
    data[0] =  uint16_t(x)       & 0xff;
    data[1] = (uint16_t(x) >> 8) & 0xff;
    return *this;
}

le_int16_t::operator int16_t()
{
    return int16_t((data[1] << 8) | data[0]);
}

le_uint32_t &le_uint32_t::operator=(uint32_t x)
{
    data[0] =  x        & 0xff;
    data[1] = (x >>  8) & 0xff;
    data[2] = (x >> 16) & 0xff;
    data[3] = (x >> 24) & 0xff;
    return *this;
}

le_uint32_t::operator uint32_t()
{
    return (data[3] << 24) |
           (data[2] << 16) |
           (data[1] <<  8) |
            data[0];
}

le_int32_t &le_int32_t::operator=(int32_t x)
{
    data[0] =  uint32_t(x)        & 0xff;
    data[1] = (uint32_t(x) >>  8) & 0xff;
    data[2] = (uint32_t(x) >> 16) & 0xff;
    data[3] = (uint32_t(x) >> 24) & 0xff;
    return *this;
}

le_int32_t::operator int32_t()
{
    return int32_t((data[3] << 24) |
                   (data[2] << 16) |
                   (data[1] <<  8) |
                    data[0]         );
}

#endif
