// mktime.c (emx+gcc) -- Copyright (c) 1990-1996 by Eberhard Mattes
//
// $Id$
//
// The emx libraries are not distributed under the GPL.  Linking an
// application with the emx libraries does not cause the executable to be
// covered by the GNU General Public License.  You are allowed to change
// and copy the emx library sources if you keep the copyright message
// intact.
//
// Modifications for Turquoise SuperStat copyright (c) 2000-2001 Peter
// Karlsson.

#include <time.h>
#include <limits.h>

#include "mytime.h"
#include "utility.h"

static long day(long, long, long);

// This variation of the mktime function will not do GMT adjustments on the
// output.
time_t my_mktime(struct tm *t)
{
    long x;
    long long r;

    // tm_mon must be in range.  The other members are not restricted.

    if (t->tm_mon < 0)
    {
        // Avoid applying the `/' and `%' operators to negative numbers
        // as the results are implementation-defined for negative
        // numbers.

        t->tm_year -= 1 + ((-t->tm_mon) / 12);
        t->tm_mon = 12 - ((-t->tm_mon) % 12);
    }
    if (t->tm_mon >= 12)
    {
        t->tm_year += (t->tm_mon / 12);
        t->tm_mon %= 12;
    }

    x = day(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    if (-1 == x)
        return (time_t) -1;

    /* 719528 = day(1970, 1, 1) */
    r = (long long) (x - 719528) * 24 * 60 * 60;

    /* This expression is not checked for overflow. */
    x = t->tm_sec + 60 * t->tm_min + 60 * 60 * t->tm_hour;

    r += x;
    if (r < 0 || r > INFINITY || r == (time_t)-1)
        return (time_t) -1;
    return (time_t) r;
}

/* year >= 1582, 1 <= month <= 12, 1 <= day <= 31 */

/** Helper function that returns the day number for a given date. */
static long day(long year, long month, long day)
{
    long result;

    if (year < 1582 || year >= INT_MAX / 365)
        return -1;
    result = 365 * year + day + 31 * (month - 1);
    if (month <= 2)
        -- year;
    else
        result -= (4 * month + 23) / 10;
    result += year / 4 - (3 * (year / 100 + 1)) / 4;
    return result;
}
