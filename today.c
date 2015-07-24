
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#  define stat _stat
#endif

char * month_names[] = {
   "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


int
main(int argc, char * argv[] )
{
   time_t       now;
   struct tm  * tmptr = NULL;
   struct tm    lcltm;
   struct stat  statb;
   int          month;
   int          day;
   int          year;
   int          rc;
   FILE       * fp    = NULL;
   FILE       * outfp = stdout;
   char       * fname = NULL;

   now   = time(NULL);
   tmptr = localtime(&now);
   lcltm = *tmptr;  /* struct copy */

   day   = lcltm.tm_mday;
   month = lcltm.tm_mon;
   year  = lcltm.tm_year;
   if ( year<1900 ) { year += 1900; }

   if ( argc==2 && NULL!=argv[1] && 0!=argv[1][0] ) {
       fname = argv[1];
       rc = stat(fname, &statb);
       if ( 0==rc ) {
	  tmptr = localtime( &statb.st_mtime );
          if ( tmptr->tm_year==lcltm.tm_year
	       && tmptr->tm_yday==lcltm.tm_yday ) {
	     fprintf(stderr, "File \"%s\" was modified today, leaving as is!\n", fname );
	     rc = 1;
	     goto cleanup;
	  }
       }

       fp = fopen(fname, "w");
       if ( NULL==fp ) {
	   fprintf(stderr, "ERROR: fopen(\"%s\",\"w\") failed: \"%s\"\n",
	           fname, strerror(errno) );
       } else{
	   outfp = fp;
       }
   }

   fprintf(outfp, "#define TODAYS_DATE \"%2d-%.3s-%04d\"\n",
	  day, month_names[month], year);

   if ( NULL!=fp ) {
       fclose(fp);
       fp = NULL;
   }

   rc = 0;

cleanup:
   return(0);

} /* main() */
