/* Define if we are using EMX's implementation of findfirst/findnext */
#undef HAS_EMX_FINDFIRST

/* Define if getopt is found in unistd.h */
#undef HAS_GETOPT_IN_UNISTD

/* Define if getopt is found in getopt.h */
#undef HAS_GETOPT_IN_GETOPT

/* Define if no system getopt is available */
#undef USE_OWN_GETOPT

/* Define if struct tm has tm_gmtoff */
#undef HAS_TM_GMTOFF

/* Define if time.h defines daylight variable */
#undef HAVE_DAYLIGHT

/* Define if time.h defines _daylight variable */
#undef HAVE_UDAYLIGHT

/* Define if time.h defines timezone variable */
#undef HAVE_TIMEZONE

/* Define if time.h defines _timezone variable */
#undef HAVE_UTIMEZONE

/* Define if we can remove timezone with putenv and tzset */
#undef FOOL_TZSET

/* Define if platform primarily uses backslash as path separators */
#undef BACKSLASH_PATHS
