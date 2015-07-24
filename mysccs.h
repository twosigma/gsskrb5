#ifdef __GNUC__
#  define GNU_UNUSED __attribute__ ((unused))
#else
#  define GNU_UNUSED
#endif

#define SCCSID(arg)  static char *sccsid GNU_UNUSED = arg; 
