#define TR_VERSION "0.0"

/* Direct threaded code is used to dispatch instructions.
   It's very fast, but GCC specific. */
#if __GNUC__ > 3
#define TR_THREADED_DISPATCH 1
#endif

#define TR_MAX_FRAMES 255