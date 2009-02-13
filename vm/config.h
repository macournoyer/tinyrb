#define TR_VERSION "0.0"

/* Direct threaded code is used to dispatch instructions.
   It's very fast, but GCC specific. */
#if __GNUC__ > 3
#define TR_THREADED_DISPATCH
#endif

#define TR_MAX_FRAMES 255

/* enable CallSite optimization */
#define TR_CALL_SITE

/* enable method inlining as intruction */
#define TR_INLINE_METHOD