/* Direct threaded code is used to dispatch instructions.
   It's very fast, but GCC specific. */
#if __GNUC__ > 3
#define MIN_THREADED_DISPATCH 1
#endif

/* max number of stack frame in the VM */
#define MIN_MAX_FRAME 256
