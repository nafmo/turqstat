/* config.h.in.  Generated from configure.in by autoheader.  */

/* Define if platform primarily uses backslash as path separators */
#define BACKSLASH_PATHS

/* Define if limits.h includes CHAR_BIT */
#define HAVE_CHAR_BIT

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#undef HAVE_DIRENT_H

/* Define if we are using DJGPP's implementation of findfirst/findnext */
#undef HAVE_DJGPP_FINDFIRST

/* Define if we are using EMX's implementation of findfirst/findnext */
#undef HAVE_EMX_FINDFIRST

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H

/* Define to 1 if you have the <fstream> header file. */
#define HAVE_FSTREAM

/* Define to 1 if you have the <fstream.h> header file. */
#undef HAVE_FSTREAM_H

/* Define if getopt is found in getopt.h */
#undef HAVE_GETOPT_IN_GETOPT

/* Define if getopt is found in stdio.h */
#undef HAVE_GETOPT_IN_STDIO

/* Define if getopt is found in unistd.h */
#undef HAVE_GETOPT_IN_UNISTD

/* Define if std namespace always is available */
#undef HAVE_IMPLICIT_NAMESPACE

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the <iostream> header file. */
#define HAVE_IOSTREAM

/* Define to 1 if you have the <iostream.h> header file. */
#undef HAVE_IOSTREAM_H

/* Define to 1 if you have the `crtdll' library (-lcrtdll). */
#define HAVE_LIBCRTDLL

/* Define to 1 if you have the `nsl' library (-lnsl). */
#undef HAVE_LIBNSL

/* Define to 1 if you have the `socket' library (-lsocket). */
#undef HAVE_LIBSOCKET

/* Define to 1 if you have the `w' library (-lw). */
#undef HAVE_LIBW

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#undef HAVE_NDIR_H

/* Define to 1 if you have the <netdb.h> header file. */
#undef HAVE_NETDB_H

/* Define to 1 if you have the <netinet/in.h> header file. */
#undef HAVE_NETINET_IN_H

/* Define if you want NNTP support */
#define HAVE_NNTP

/* Define if you have OS/2 DosQueryCtryInfo API */
#undef HAVE_OS2_COUNTRYINFO

/* Define if C library includes sleep */
#undef HAVE_SLEEP

/* Define if C library includes snprintf */
#undef HAVE_SNPRINTF

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H

/* Define if C library includes strcasecmp */
#undef HAVE_STRCASECMP

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME

/* Define if C library includes stricmp */
#define HAVE_STRICMP

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#undef HAVE_STRING_H

/* Define if C library includes stristr */
#undef HAVE_STRISTR

/* Define if C library includes strlwr */
#undef HAVE_STRLWR

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#undef HAVE_SYS_DIR_H

/* Define to 1 if you have the <sys/errno.h> header file. */
#undef HAVE_SYS_ERRNO_H

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <sys/socket.h> header file. */
#undef HAVE_SYS_SOCKET_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H

/* Define if C library includes timegm */
#undef HAVE_TIMEGM

/* Define if time.h defines timezone variable */
#define HAVE_TIMEZONE

/* Define if struct tm has tm_gmtoff */
#define HAVE_TM_GMTOFF

/* Define if ctype.h defines towupper */
#define HAVE_TOWUPPER_IN_CTYPE_H

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Define if C library includes _snprintf instead of snprint */
#define HAVE_USNPRINTF

/* Define if time.h defines _timezone variable */
#undef HAVE_UTIMEZONE

/* Define if C library includes _sleep instead of sleep */
#define HAVE_U_SLEEP

/* Define to 1 if you have the <wchar.h> header file. */
#define HAVE_WCHAR_H

/* Define to 1 if you have the <wctype.h> header file. */
#define HAVE_WCTYPE_H

/* Define if you have Win32 GetDateFormat API */
#define HAVE_WIN32_LOCALEINFO

/* Define if you have Win32 GetModuleFileName API */
#define HAVE_WIN32_GETMODULEFILENAME

/* Define to 1 if you have the <winsock.h> header file. */
#undef HAVE_WINSOCK_H

/* Define to 1 if you have the <winsock2.h> header file. */
#define HAVE_WINSOCK2_H

/* Define if you have a working wstring implementation */
#define HAVE_WORKING_WSTRING

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* Define to size of time_t, in bytes */
#ifdef _USE_32BIT_TIME_T
#define SIZEOF_TIME_T 4
#else
#define SIZEOF_TIME_T 8
#endif

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS

/* Define if no system getopt is available */
#define USE_OWN_GETOPT

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#undef WORDS_BIGENDIAN

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Additional defines for Visual C++ */
/* Do not deprecate fopen() and friends */
#define _CRT_SECURE_NO_DEPRECATE

/* Do not deprecate strdup() and friends */
#define _CRT_NONSTDC_NO_DEPRECATE
