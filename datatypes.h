#ifndef __DATATYPES_H
#define __DATATYPES_H

#include <config.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#else // not HAVE_STDINT_H

# if defined(__CYGWIN__) && defined(__BIT_TYPES_DEFINED__)

 typedef u_int8_t  uint8_t;
 typedef u_int16_t uint16_t;
 typedef u_int32_t uint32_t;

# else // not _CYGWIN__ && __BIT_TYPES_DEFINED__

#  include <limits.h>

  // Define datatypes according to limits.h

#  if (UCHAR_MAX == 255)
   typedef unsigned char uint8_t;
   typedef signed char int8_t;
#  else
#   error No 8-bit integer type found
#  endif

#  if (UCHAR_MAX == 65535)
   typedef unsigned char uint16_t;
   typedef signed char int16_t;
#  else
#   if (USHRT_MAX == 65535)
    typedef unsigned short uint16_t;
    typedef signed short int16_t;
#   else
#    if (UINT_MAX == 65535)
     typedef unsigned int uint16_t;
     typedef signed int int16_t;
#    else
#     error No 16-bit integer type found
#    endif
#   endif
#  endif

#  if (UCHAR_MAX == 4294967295U)
   typedef unsigned char uint32_t;
   typedef signed char int32_t;
#  else
#   if (USHRT_MAX == 4294967295U)
    typedef unsigned short uint32_t;
    typedef signed short int32_t;
#   else
#    if (UINT_MAX == 4294967295U)
     typedef unsigned int uint32_t;
     typedef signed int int32_t;
#    else
#     if (ULONG_MAX == 4294967295)
      typedef unsigned long uint32_t;
      typedef signed long int32_t;
#     else
#      error No 32-bit integer type found
#     endif
#    endif
#   endif
#  endif
# endif  // else not _CYGWIN__ && __BIT_TYPES_DEFINED__
#endif // else not HAVE_STDINT_H

// Define little-endian datatypes

#if defined(WORDS_BIGENDIAN)
# include <string.h>
# if defined(__GNUC__) || defined(__EMX__)
#  pragma pack(1)
# endif

// Little-endian emulation layer

/** Class that emulates a 16-bit unsigned little-endian value */
class le_uint16_t
{
public:
    /** Assign a 16-bit unsigned value to the variable. */
    le_uint16_t &operator=(uint16_t);
    /** Convert variable to corresponding machine-endian value. */
    operator uint16_t();
private:
    uint8_t data[2]; ///< 16 bits in a byte field
};

/** Class that emulates a 16-bit signed little-endian value */
class le_int16_t
{
public:
    /** Assign a 16-bit signed value to the variable. */
    le_int16_t &operator=(int16_t);
    /** Convert variable to corresponding machine-endian value. */
    operator int16_t();
private:
    uint8_t data[2]; ///< 16 bits in a byte field
};

/** Class that emulates a 32-bit unsigned little-endian value */
class le_uint32_t
{
public:
    /** Assign a 32-bit unsigned value to the variable. */
    le_uint32_t &operator=(uint32_t);
    /** Convert variable to corresponding machine-endian value. */
    operator uint32_t();
private:
    uint8_t data[4]; ///< 32 bits in a byte field
};

/** Class that emulates a 32-bit signed little-endian value */
class le_int32_t
{
public:
    /** Assign a 32-bit signed value to the variable. */
    le_int32_t &operator=(int32_t);
    /** Convert variable to corresponding machine-endian value. */
    operator int32_t();
private:
    uint8_t data[4]; ///< 32 bits in a byte field
};
# if defined(__GNUC__) || defined(__EMX__)
#  pragma pack(pop)
# endif
#else
# define le_uint16_t uint16_t
# define le_int16_t  int16_t
# define le_uint32_t uint32_t
# define le_int32_t  int32_t
#endif

#endif
