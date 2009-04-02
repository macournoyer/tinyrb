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

/* TR_BITSINT defines the number of bits in an int. */
#if INT_MAX-20 < 32760
#define TR_BITSINT  16
#elif INT_MAX > 2147483640L
/* int has at least 32 bits */
#define TR_BITSINT  32
#else
#error "you must define TR_BITSINT with number of bits in an integer"
#endif
