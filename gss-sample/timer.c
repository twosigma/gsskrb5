/************************************************************************/
/* $Id: //tools/src/freeware/gsskrb5/gss-sample/timer.c#1 $
 ************************************************************************
 *
 * Copyright (c) 1998-2000  SAP AG.  All rights reserved.
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



#include "gss-misc.h"


#include <time.h>


unsigned long    hrtimer_resolution = 0;
char           * hrtimer_name       = "";

/**********************************************************************************/
/**********************************************************************************/
/**										 **/
/**   LOW Resolution Timers (platform specific)					 **/
/**										 **/
/**   the return value (unsigned long) is in milliseconds units.		 **/
/**   the true resolution can be significantly less, around 10ms or 20ms ...	 **/
/**										 **/
/**********************************************************************************/
/**********************************************************************************/

#if defined(LRTIMER_GETTIMEOFDAY)
#  include <sys/time.h>

static struct timeval start_lrtime = { 0, 0 };

void
start_lrtimer( void )
{
   gettimeofday( &start_lrtime, NULL );

   return;

} /* init_lrtimer() */

unsigned long
read_lrtimer( void )
{
   struct timeval  now_lrtime;
   unsigned long   millisec;

   gettimeofday( &now_lrtime, NULL );

   millisec = (now_lrtime.tv_sec - start_lrtime.tv_sec) * 1000;
   millisec = millisec + ((1000000 + now_lrtime.tv_usec) - start_lrtime.tv_usec)/1000;
   millisec -= 1000;

   return(millisec);

} /* init_lrtimer() */

#elif defined(LRTIMER_TIMES)

#  include <sys/times.h>
#  include <unistd.h>

static  time_t   start_lrtime;
static  time_t   clock_ticks;

void
start_lrtimer( void )
{
   struct tms  dummy;

   clock_ticks  = sysconf(_CLK_TCK);
   if ( clock_ticks==0 ) { clock_ticks = 1; /* prevent division by zero ... */ }
   start_lrtime = times( &dummy );

   return;

} /* start_lrtimer() */

unsigned long
read_lrtimer( void )
{
   struct tms     dummy;
   unsigned long  millisec;
   time_t         now_lrtime;

   now_lrtime = times( &dummy );

   if ( now_lrtime>=start_lrtime ) {
      millisec = ((now_lrtime - start_lrtime) * 1000 )/ clock_ticks;
   } else {
      millisec = (((((time_t)-1) - start_lrtime) + 1 + now_lrtime) * 1000 ) / clock_ticks;
   }

   return(millisec);

} /* read_lrtimer() */

#elif defined(LRTIMER_WIN_FTIME)

#  include <sys/types.h>
#  include <sys/timeb.h>

static struct _timeb  start_lrtime;

void
start_lrtimer( void )
{
   _ftime( &start_lrtime );

   return;

} /* start_lrtimer() */


unsigned long
read_lrtimer( void )
{
   struct _timeb  now_lrtime;
   unsigned long  millisec;

   _ftime( &now_lrtime );

   millisec = ( now_lrtime.time - start_lrtime.time ) * 1000 ;
   millisec = (millisec + now_lrtime.millitm) - start_lrtime.millitm;

   return(millisec);

} /* read_lrtimer() */


#else

#  error  MISSING: LRTIMER_ not defined for this platform

#endif /* LRTIMER_* */




/**********************************************************************************/
/**********************************************************************************/
/**										 **/
/**	 High Resolution Timers (VERY platform specific)                         **/
/**										 **/
/**      the return value (unsigned long) is in microseconds units.              **/
/**      The true resolution may be significantly less, but we'll		 **/
/**      try to do our best for those individual platforms                       **/
/**      We will need at least 100us to get useful results ...                   **/
/**										 **/
/**********************************************************************************/
/**********************************************************************************/

#ifdef HRTIMER_GETTIMEOFDAY

#  define TIMER_TYPE  "gettimeofday()"

 /**************************************************************/
 /**   Unix with gettimeofday()                               **/
 /**							      **/
 /** Although most Unix have it, the actual resolution varies **/
 /** considerably.  For our purposes a hi-res hardware        **/
 /** counter would be sufficient and more appropriate,        **/
 /** but it is also very platform specific.                   **/
 /** You're free to add your own code.                        **/
 /**************************************************************/

#  include <sys/time.h>


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

#elif defined(HRTIMER_OSFALPHA_PCC)

#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/time.h>


#  define TIMER_TYPE "read_alpha_pcc"

static unsigned long  start_pcc        = 0;
static struct timeval start_time;
static unsigned long  clock_ticks      = 0;
static unsigned long  frequency        = 1;
static unsigned long  max_resolution   = 0;
int                   alpha_cpu_speed  = 0;

/* The Processor Clock Counter (PCC) counts the CPU clocks  */
/* and therefore increases (and wraps) very fast!           */
/* We sample/average the frequency of the PCC clock counter */


extern read_alpha_pcc( void );

void
init_timer( void )
{
   struct tms      tmsb;
   time_t          time_1;
   time_t          time_2;
   time_t          diff_time;
   unsigned long   next_pcc;
   unsigned long   diff_pcc;
   Ulong           vals[16];
   int             i;

   clock_ticks = sysconf(_SC_CLK_TCK);

   for ( i=0 ; i<ARRAY_ELEMENTS(vals); i++ ) {

      time_1 = times(&tmsb);
      while ( time_1==times(&tmsb) );
      start_pcc = read_alpha_pcc( );
      time_1 = times(&tmsb);

      while ( time_1==times(&tmsb) );
      next_pcc = read_alpha_pcc( );
      time_2 = times(&tmsb);

      diff_time = (time_2 - time_1);
   
      if ( next_pcc<start_pcc ) {
         diff_pcc = (0xffffffff - start_pcc) + 1 + next_pcc;
      } else {
         diff_pcc = next_pcc - start_pcc;
      }
      diff_pcc = diff_pcc * clock_ticks / diff_time;

      vals[i] = ((diff_pcc + 500000) / 1000000) * 1000000;

   }

   frequency = sample_avg( vals, ARRAY_ELEMENTS(vals) );

   /* max_resolution: 3/4 of the 32-bit counterspace expressed in usec        */
   /*									      */
   /* If gettimeofday() indicates that a time  exceeding 3/4 PCC counterspace */
   /* has passed since start_timer(), then we fall back on the output and     */
   /* resolution of gettimeofday()                                            */
   max_resolution = (0xc0000000 / frequency)*1000000;

   alpha_cpu_speed = (int)(frequency / 1000000);

   return;

} /* init_timer() */

/* ---------- */

void
start_timer( void )
{
   gettimeofday( &start_time, NULL );
   start_pcc   = read_alpha_pcc( );

   return;

} /* start_timer() */

/* ---------- */

/* Since the PCC clock counter is only 32bits wide and counts    */
/* CPU cycles, it will wrap within a matter of several seconds   */
/* (~10 seconds for a 466 MHz CPU).  We fall back on the output  */
/* of gettimeofday() as soon as it indicates that a time         */
/* exceeding 3/4 PCC counter space has already passed since      */
/* start_timer() was called.  On OSF1, this should happen        */
/* within the magnitude of a second for a 1GHz CPU, while the    */
/* resolution of gettimeofday() seems to be ~1ms                 */

unsigned long
read_timer( void )
{
   struct timeval  now;
   unsigned long   diff_time;
   unsigned long   current_pcc;
   unsigned long   diff_pcc;
   unsigned long   usecs;

   current_pcc = read_alpha_pcc( );
   gettimeofday( &now, NULL );

   if ( start_time.tv_sec == 0 )
      return (0);
   
   if ( current_pcc < start_pcc ) {
      diff_pcc = (0xffffffff - start_pcc) + 1 + current_pcc;
   } else {
      diff_pcc = current_pcc - start_pcc;
   }

   diff_time = 0;
   usecs = (diff_pcc * 1000000) / frequency;

   if ( now.tv_sec >= start_time.tv_sec ) {

      diff_time = now.tv_sec - start_time.tv_sec;
      diff_time *= 1000000;
      diff_time = diff_time + now.tv_usec - start_time.tv_usec;
      if ( diff_time > max_resolution ) {
	  usecs = diff_time;
      }
   }

   return(usecs);

} /* read_timer() */



#elif defined(HRTIMER_WIN32_QueryPerformanceCounter) /* !HAVE_GETTIMEOFDAY */


#  define TIMER_TYPE "QueryPerformanceCounter()"

  /**********************************************************************/
  /**  Win32 with QueryPerformanceCounter()			       **/
  /**								       **/
  /** This call is documented to offer EXTREMELY hardware specific     **/
  /** timer resolution across different Win32 Platforms.               **/
  /** Windows NT on Pentiums and Pentium Pros looks like 8-15 usec     **/
  /** BEWARE: when using older MS Win32 OSes (like Win95) on new CPUs, **/
  /** then QueryPerformanceCounter() uses 10 millisec resolution       **/
  /**********************************************************************/

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
   float	  diff;
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

#else

#  error  MISSING: HRTIMER_ not defined for this platform

#endif /* HRTIMER_*  platform specific high resolution timer */




/*******************************************************************************/
/*******************************************************************************/
/**									      **/
/**	    Platform independent Timing support functions		      **/
/**									      **/
/*******************************************************************************/
/*******************************************************************************/

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

   sprintf(timespan, "%6lu.%03lu millisec", msec, usec);

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
   Uint    start = p_nelements/4; /* 25% */
   Uint    stop  = start * 2;	  /* 50% */
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
   unsigned long now;
   unsigned long val, newval;
   unsigned long i;
   Ulong         vals[16];

   init_timer();

   /* await next increase of lowres-timer */
   start_lrtimer();
   while ( read_lrtimer() == 0 );

   start_timer();
   val = read_timer();

   for ( i=0 ; i<ARRAY_ELEMENTS(vals) ; i++ ) {
      do {
	 newval = read_timer();
      } while ( val == newval );
      vals[i] = (newval - val);
      val = newval;
   }

   hrtimer_resolution = sample_avg(vals, ARRAY_ELEMENTS(vals));
   hrtimer_name       = TIMER_TYPE;

   fprintf(logfp, "Timer resolution of %s is (at least) %s\n",
	       hrtimer_name, sprint_timer(hrtimer_resolution) );

   /* await next increase of lowres-timer */
   start_lrtimer();
   while ( read_lrtimer() == 0 );

   /* reset both timers */
   start_lrtimer();
   start_timer();
   
   /* loop for about one second */
   do {
      now = read_lrtimer();
   } while ( now <= 1000 );

   val = read_timer();
   

   val = (val * 1000) / now;

   if ( val<900000 || val>1100000 ) {
      fprintf(logfp, "\nWhoa!  timer resolution significantly wrong !\n");
   } else if ( val<950000 || val>1050000 ) {
      fprintf(logfp, "\nThe timer resolution looks somewhat inaccurate!\n");
   }
   fprintf(logfp, "1 second passed in %s. \n\n", sprint_timer(val) );

   return;

} /* show_timer_resolution() */




