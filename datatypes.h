#include <config.h>
#ifdef HAVE_STDINT_H
# include <stdint.h>

  // Define datatypes according to stdint.h
  typedef uint8_t UINT8;
  typedef int8_t SINT8;

  typedef uint16_t UINT16;
  typedef int16_t SINT16;

  typedef uint32_t UINT32;
  typedef int32_t SINT32;

#else // no HAVE_STDINT_H
# ifndef HAVE_LIMITS_H
#  error Neither limits.h or stdint.h are present -- cannot determine sizes
# endif
# include <limits.h>

  // Define datatypes according to limits.h

# if (UCHAR_MAX == 255)
   typedef unsigned char UINT8;
   typedef signed char SINT8;
# else
#  error No 8-bit integer type found
# endif

# if (UCHAR_MAX == 65535)
   typedef unsigned char UINT16;
   typedef signed char SINT16;
# else
#  if (USHRT_MAX == 65535)
    typedef unsigned short UINT16;
    typedef signed short SINT16;
#  else
#   if (UINT_MAX == 65535)
     typedef unsigned int UINT16;
     typedef signed int SINT16;
#   else
#    error No 16-bit integer type found
#   endif
#  endif
# endif

# if (UCHAR_MAX == 4294967295U)
   typedef unsigned char UINT32;
   typedef signed char SINT32;
# else
#  if (USHRT_MAX == 4294967295U)
    typedef unsigned short UINT32;
    typedef signed short SINT32;
#  else
#   if (UINT_MAX == 4294967295U)
     typedef unsigned int UINT32;
     typedef signed int SINT32;
#   else
#    if (ULONG_MAX == 4294967295)
      typedef unsigned long UINT32;
      typedef signed long SINT32;
#    else
#     error No 32-bit integer type found
#    endif
#   endif
#  endif
# endif
#endif
