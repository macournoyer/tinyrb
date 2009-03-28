#define TR_VERSION "0.0"

#include <limits.h>

/* Direct threaded code is used to dispatch instructions.
   It's very fast, but GCC specific. */
#if __GNUC__ > 3
#define TR_THREADED_DISPATCH
#endif

#define TR_MAX_FRAMES 255

/* enable CallSite optimization */
#define TR_CALL_SITE

#define MAX_INT (INT_MAX-2)  /* maximum value of an int (-2 for safety) */