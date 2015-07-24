#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/misc_sup.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/misc_sup.c#2 $
 ************************************************************************
 *
 * Copyright (c) 1997-2000  SAP AG.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by SAP AG"
 *
 * 4. The name "SAP AG" must not be used to endorse or promote products
 *    derived from this software without prior written permission.
 *    For written permission, please contact www.press@sap.com
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by SAP AG"
 *
 * THIS SOFTWARE IS PROVIDED BY SAP AG ``AS IS'' AND ANY EXPRESSED
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. SAP AG SHALL BE LIABLE FOR ANY DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE ONLY IF CAUSED BY SAP AG'S
 * INTENT OR GROSS NEGLIGENCE. IN CASE SAP AG IS LIABLE UNDER THIS
 * AGREEMENT FOR DAMAGES CAUSED BY SAP AG'S GROSS NEGLIGENCE SAP AG
 * FURTHER SHALL NOT BE LIABLE FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT, AND SHALL NOT BE LIABLE IN EXCESS OF THE AMOUNT OF
 * DAMAGES TYPICALLY FORESEEABLE FOR SAP AG, WHICH SHALL IN NO EVENT
 * EXCEED US$ 500.000.- 
 *
 ************************************************************************/


/* misc support functions (possibly platform dependent) */

#include "test_api.h"

#include <time.h>



#ifdef HAVE_GETTIMEOFDAY

#include <sys/time.h>

#define TIMER_TYPE  "gettimeofday()"

static struct timeval start_time = { 0, 0 };

void
init_timer( void )
{
   return;

} /* init_timer() */


void
start_timer( void )
{
   gettimeofday( &start_time, NULL );

   return;

} /* start_timer() */


unsigned long
read_timer( void )
{
   unsigned long  diff;
   struct timeval now;

   gettimeofday( &now , NULL );

   if ( start_time.tv_sec == 0 )
      return (0);

   diff  = (now.tv_sec  - start_time.tv_sec );
   if ( diff> ULONG_MAX/1000000 ) {
      return(ULONG_MAX);
   }

   diff *= 1000000;
   diff += (now.tv_usec - start_time.tv_usec);

   return(diff);

} /* read_timer() */


#else /* !HAVE_GETTIMEOFDAY */


#  ifdef _WIN32

#define TIMER_TYPE "QueryPerformanceCounter()"

static LARGE_INTEGER  PerfFrequency;
static LARGE_INTEGER  StartCount;

void
init_timer( void )
{
   QueryPerformanceFrequency( &PerfFrequency );

   QueryPerformanceCounter( &StartCount );

   return;

} /* init_timer() */

void
start_timer( void )
{
   QueryPerformanceCounter( &StartCount );

   return;

} /* start_timer() */



unsigned long
read_timer( void )
{
   LARGE_INTEGER  now;
   float          diff;
   float	  xdiff;

   QueryPerformanceCounter( &now );
   if (PerfFrequency.QuadPart == 0)
      return(0);

   xdiff = ((float) (now.QuadPart - StartCount.QuadPart)) * 1000000;
   diff = xdiff / (float)PerfFrequency.QuadPart;
   if ( diff > (float)UINT_MAX )
      return( UINT_MAX );

   return((unsigned long)diff);
   
} /* read_timer() */

#  endif /* !_WIN32 */

#endif /* !HAVE_GETTIMEOFDAY */




char *
sprint_timer( unsigned long timer_val )
{
   unsigned long   msec, usec;
   static char     timespan[128];

   timespan[0] = '\0';

   if ( timer_val==0 ) {

      return("unknown timespan");

   }

   usec = timer_val % 1000;
   msec = (timer_val - usec) / 1000;

   sprintf(timespan, "%6lu.%03lu msec", msec, usec);

   return(timespan);

} /* print_timer() */


static int
sample_compare( const void * s1, const void * s2 )
{

   return( (*( (Ulong *)s1)) - (*( (Ulong *)s2)) );

} /* sample_compare() */


/*
 * sample_avg()
 *
 * sort the sample[] array of elapsed time numerically
 * and calculate an average time over sample[],
 * ignoring
 *     the worst 50% of the results because of context switches
 *     the best  25% because of insufficient timer resolution on a few platforms
 */
Ulong
sample_avg( Ulong * p_sample, size_t p_nelements )
{
   Uint    start = (Uint)p_nelements/4; /* 25% */
   Uint    stop  = start * 2;		     /* 50% */
   Uint    i;
   Ulong   sum, avg, count;

   qsort( (void *)p_sample, p_nelements, sizeof(p_sample[0]), sample_compare );

   sum = count = 0;

   for ( count=0, i=start ; i<stop ; i++ ) {
      sum += p_sample[i];
      count++;
   }

   avg = (sum + count/2) / count;

   return(avg);

} /* sample_avg() */




void
show_timer_resolution( void )
{
   time_t        now, start;
   unsigned long val, newval, avg;
   unsigned long i;
   Ulong         vals[16];

   init_timer();

   now = time(NULL);
   while ( now==time(NULL) );

   start_timer();
   val = read_timer();

   for ( i=0 ; i<ARRAY_ELEMENTS(vals) ; i++ ) {
      do {
	 newval = read_timer();
      } while ( val == newval );
      vals[i] = (newval - val);
      val = newval;
   }

   avg = sample_avg(vals, ARRAY_ELEMENTS(vals));

   fprintf(fh, "Timer resolution of %s is (at least) %s\n",
	       TIMER_TYPE, sprint_timer(avg) );

   start = time(NULL);
   do {
      now = time(NULL);
   } while ( start == now );
   
   start_timer();
   while ( time(NULL)==now );

   val = read_timer();

   if ( val<950000 || val>1050000 ) {
      fprintf(fh, "Whoa!  timer resolution wrong??\n");
      fprintf(fh, "1 second passed in %s ?!\n\n", sprint_timer(val) );
   } else {
      fprintf(fh, "1 second passed in %s. \n\n", sprint_timer(val) );
   }

   return;

} /* show_timer_resolution() */




