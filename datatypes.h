#ifndef __DATATYPES_H
#define __DATATYPES_H

# include <limits.h>

  // Define datatypes according to limits.h

# if (UCHAR_MAX == 255)
   typedef unsigned char uint8_t;
   typedef signed char int8_t;
# else
#  error No 8-bit integer type found
# endif

# if (UCHAR_MAX == 65535)
   typedef unsigned char uint16_t;
   typedef signed char int16_t;
# else
#  if (USHRT_MAX == 65535)
    typedef unsigned short uint16_t;
    typedef signed short int16_t;
#  else
#   if (UINT_MAX == 65535)
     typedef unsigned int uint16_t;
     typedef signed int int16_t;
#   else
#    error No 16-bit integer type found
#   endif
#  endif
# endif

# if (UCHAR_MAX == 4294967295U)
   typedef unsigned char uint32_t;
   typedef signed char int32_t;
# else
#  if (USHRT_MAX == 4294967295U)
    typedef unsigned short uint32_t;
    typedef signed short int32_t;
#  else
#   if (UINT_MAX == 4294967295U)
     typedef unsigned int uint32_t;
     typedef signed int int32_t;
#   else
#    if (ULONG_MAX == 4294967295)
      typedef unsigned long uint32_t;
      typedef signed long int32_t;
#    else
#     error No 32-bit integer type found
#    endif
#   endif
#  endif
# endif
#endif
