// Written in 2000-2005 by Peter Krefting
//
// This file is not copyrighted.

#ifndef __MYTIME_H
#define __MYTIME_H

#ifdef HAVE_TIMEGM
inline time_t my_mktime(struct tm *p)
{ return timegm(p); }
#else
/**
 * Convert a tm structure to time_t, disregarding timezone.
 * This function behaves just like the standard C library function mktime,
 * except that it does not do adjustments for the local timezone when
 * converting. This is needed since most Fidonet message base formats assume
 * that the date stored in the messages are in local time, and we can thus
 * not do any reliable timezone adjustments on the values we receive.
 */
time_t my_mktime(struct tm *);
#endif

#endif
