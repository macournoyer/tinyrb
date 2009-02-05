#define TR_VERSION "0.0.0.0.0.0.0.0.00001.5 kinda beta"

/* Direct threaded code is used to dispatch instructions.
   It's very fast, but GCC specific. */
#if __GNUC__ > 3
#define TR_THREADED_DISPATCH 1
#endif
