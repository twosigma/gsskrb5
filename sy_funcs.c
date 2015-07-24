#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/sy_funcs.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/sy_funcs.c#2 $
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


#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

#include <errno.h>
#include <ctype.h>

#include "platform.h"

#if defined(_WINDOWS) || defined(_WIN32)
#  include <sys/timeb.h>
#  include <windows.h>
#endif

#include <time.h>

#if defined HAVE_BSD_SOCKETS
     /* <netdb.h> for gethostbyname() and struct hostent */
#  include <netdb.h>
#endif

#if defined HAVE_WINSOCK
#    /* <winsock.h> for gethostname(), gethostbyname(), etc. */
#    include <winsock.h>
#endif


#ifdef HAVE_UNISTD_H
  /* <unistd.h> for gethostname(), though it doesn't seem to be POSIX ... */ 
#  include <unistd.h>
#endif

#ifdef HAVE_PWD_H
  /* <pwd.h> for getpwuid() */
#  include <pwd.h>
#endif

#include "gssmaini.h"


#  if defined(NEED_WSASTARTUP)
      volatile Uint WSA_usecntr = 0;
#  endif


OM_uint32
sy_init( OM_uint32 * pp_min_stat )
{
   (*pp_min_stat) = 0;

   sy_init_chartable();

   return(GSS_S_COMPLETE);

} /* sy_init() */



void
sy_cleanup( void )
{
   return;

} /* sy_cleanup() */



static char * sy_valid_digits = "0123456789";
static char * sy_valid_alpha  = "abcdefghijklmnopqrstuvwxyz"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char   sy_chartable[256];


void
sy_init_chartable(void)
{
   unsigned char   uch;
   char          * s;

   memset(sy_chartable, 0, sizeof(sy_chartable));

   for ( s = sy_valid_digits ; *s ; s++ ) {
      uch = (unsigned char) *s;
      sy_chartable[uch] = CHECK_DIGIT;
   }

   for ( s = sy_valid_alpha ; *s ; s++ ) {
      uch = (unsigned char) *s;
      sy_chartable[uch] = CHECK_ALPHA;
   }

   return;

} /* sy_init_chartable() */
   


/*
 * sy_check_valid_chars()
 *
 *
 *
 */
int
sy_check_valid_chars( void * p_buffer,			/* in        */
		      int p_len,			/* in        */
		      int p_type,			/* in | 0    */
		      char *p_more )			/* in | NULL */
{
   int i;
   int len;
   int ok = 1;
   unsigned char *puch, *puch2;

   len = (p_len>0) ? p_len : (int)strlen((char *)p_buffer);

   puch = (unsigned char *)p_buffer;

   for ( i=0 ; i<len ; i++, puch++ ) {
      if ( (sy_chartable[*puch] & p_type) == 0  ) {
	 ok = 0;
	 for ( puch2 = (unsigned char *)p_more ; *puch2!='\0' ; puch2++ ) {
	    if ( *puch2 == *puch ) {
	       ok = 1;
	       break;
	    }
	 }
	 if ( ok==0 )
	    break;
      }

      if ( (p_type & CONV_2_UPPER)!=0 ) {
	 *puch = (unsigned char) toupper( *puch );
      } else if ( (p_type & CONV_2_LOWER)!=0 ) {
	 *puch = (unsigned char) tolower( *puch );
      }
   }

   return(ok);

} /* sy_check_valid_chars() */




/*
 * sy_get_time()
 *
 *  Description:
 *    Return the current Universal Coordinated Time (UTC)
 *    in seconds and milliseconds elapsed since January 1st, 1970 GMT
 *
 */
OM_uint32
sy_get_time( sy_timeval_desc * p_time )			/* out | NULL */
{
   static sy_timeval_desc    last;
          sy_timeval_desc    now;
	  sy_timeval_desc    diff;

#if defined(HAVE_GETTIMEOFDAY) /* Unix */
   struct timeval            tv;

   if ( gettimeofday( &tv, NULL )==-1 ) {
      DEBUG_ERR((tf, "ERROR: gettimeofday failed (errno=%d)\n", errno));
      now.sec  = last.sec;
      now.usec = last.usec;
      goto use_time; /* use "last" time and increase it by one microsecond */
   }

   now.sec  = (OM_uint32) tv.tv_sec;
   now.usec = (OM_uint32) tv.tv_usec;

#elif defined(HAVE_FTIME) /* MS-Windows */

   /* The time resolution of _ftime() is in clock-ticks, i.e. about 1/100 sec */
   /* therefore it is a pretty cheap emulation of gettimeofday().             */
   /* However, for delivering GSS-API services this is good enough !          */

   struct _timeb timebuffer;

   _ftime( &timebuffer );
   now.sec  = (OM_uint32) timebuffer.time;
   now.usec = (OM_uint32) timebuffer.millitm;

#else

#  error HOW does one read the current UTC time on this machine \
         (seconds and milliseconds since Jan 1st, 1970 GMT) ?

#endif

   if ( now.sec > last.sec ) {

      goto time_ok; /* good, time is moving forward, memorize "now" */

   } else if ( now.sec == last.sec ) {

      if ( now.usec > last.usec ) {

	 goto time_ok;

      } else if ( now.usec==last.sec ) {

use_time:
	 now.usec ++;
         if ( now.usec >= 1000000ul ) {
	    now.sec++;
	    now.usec = 0;
	 }
time_ok:
	 last.usec = now.usec;
	 last.sec  = now.sec;
	 if ( p_time!=NULL ) {
	    p_time->sec  = now.sec;
	    p_time->usec = now.usec;
	 }

	 return(now.sec);

      }

   }

   sy_diff_time( &now, &last, &diff );  /* calculate backward time shift */

   DEBUG_STRANGE((tf, "STRANGE: Backward Time shift by %lu sec %lu usec!\n",
		  (unsigned long)diff.sec, (unsigned long)diff.usec ))

   goto use_time; /* accept current time and increase by one microsecond */

} /* sy_get_time() */




/*
 * sy_diff_time()
 *
 *
 */
OM_uint32
sy_diff_time( sy_timeval_desc * t1,			/* in  */
	      sy_timeval_desc * t2,			/* in  */
	      sy_timeval_desc * diff )			/* out */
{
   if ( t2->usec >= t1->usec ) {

      diff->usec = t2->usec - t1->usec;
      diff->sec  = t2->sec  - t1->sec;

   } else {

      diff->usec = (t2->usec + 1000000ul) - t1->usec ;
      diff->sec  = (t2->sec - t1->sec) - 1;

   }

   return(diff->sec);

} /* sy_diff_time() */




/* Call to ANSI free(3), return memory buffer to the heap */
/* >>this<< can be called with a NULL argument            */
void
sy_free(void *p_memptr)					/* in/out | NULL */
{
   if (p_memptr!=NULL) {
      free(p_memptr);
   }

} /* sy_free() */


/*********************************************************************
 *  sy_clear_free()
 *
 *  Description:
 *    clear a block of memory, then free() it;
 *    Zero the original pointer to this memory as well
 *********************************************************************/
void
sy_clear_free( void **p_memptr,				/* in/out | NULL */
	       size_t p_clearlen )			/* in | 0	 */
{
   if ( *p_memptr!=NULL ) {
      if ( p_clearlen>0 )
	 memset( *p_memptr, 0, p_clearlen );
      free( *p_memptr );
      *p_memptr = NULL;
   }
} /* sy_clear_free() */

 
/*********************************************************************
 *  sy_malloc()
 *
 *  Description:
 *    Call to ANSI malloc(3), allocate a block of memory (dirty memory)
 *********************************************************************/
void *
sy_malloc(size_t p_mem_size )				/* in  */
{
   void * ptr;

   if ( p_mem_size>UINT_MAX )
      return( (void *)0 );

   ptr = malloc( p_mem_size );

   if (ptr)
      return(ptr);

   DEBUG_ERR((tf, "ERROR: sy_malloc(): Uh-oh! malloc(%lu) failed!\n",
	          (unsigned long)(p_mem_size) ));

   return( (void *)0 );

} /* sy_malloc() */




/*********************************************************************
 *  sy_calloc()
 *
 *  Description:
 *    Call to ANSI calloc(3), allocate a block of Zeroed memory
 *********************************************************************/
void *
sy_calloc( size_t p_mem_size )				/* in  */
{
   void * ptr;

   if ( p_mem_size>UINT_MAX )
      return( (void *)0 );

   ptr = calloc( 1, p_mem_size );

   if (ptr)
      return(ptr);

   DEBUG_ERR((tf, "ERROR: sy_calloc(): Uh-oh! calloc(1, %lu) failed!\n",
	          (unsigned long)(p_mem_size) ));

   return( (void *)0 );

} /* sy_calloc() */


/*
 * sy_strnlen()
 *
 * strlen with a maximum limit
 *
 */
size_t
sy_strnlen( const char *s1, size_t count )
{
   size_t       i;

   for( i=0 ; i<count && s1[i]!=0 ; i++ );

   return(i);

} /* sy_strnlen() */




/*
 * sy_strmaxcpy()
 *
 * copy a string into a limited buffer, possibly truncating it
 * function returns NULL when truncating source string
 *
 */
char *
sy_strmaxcpy( char *dest, const char *src, size_t count )
{
   char  * ptr = NULL;

   if ( count!=0 ) {
      if ( src==NULL || count<=1 ) {
	 *dest = '\0';
	 if ( src!=NULL && *src!='\0' ) {
	    ptr = dest;
	 }

      } else {

	 strncpy( dest, src, count );
	 if ( dest[count-1]=='\0' ) {
	    ptr = dest;
	 } else {
	    /* strcpy didn't put a NUL here => source was longer than count */
	    dest[count-1] = '\0';
	 }
      }
   }

   return(ptr);

} /* sy_strmaxcpy() */




/*
 * sy_strmaxdup()
 *
 * copy a string into dynamic memory when it's less than the max size,
 * and return the pointer to the new memory, or return NULL when the
 * source string exceeds the length limit.
 *
 */
char *
sy_strmaxdup( char * src, size_t  max_len )
{
   char    * ptr = NULL;
   size_t    len;

   for ( len=0 ; src[len]!='\0'  &&  len<max_len ; len++ );
   if ( src[len]=='\0' ) {
      ptr = sy_malloc( len+1 );
      if ( ptr!=NULL )
	 memcpy( ptr, src, len+1 );
   }

   return(ptr);

} /* sy_strmaxdup() */


#if defined(HAVE_SOCKETS)

/*********************************************************************
 *  sy_resolve_hostname()
 *
 *  Description:
 *    Resolve an arbitrary hostname via gethostbyname.
 *
 *    NOTE: this functions allows INPLACE resolving,
 *          i.e. output = input buffer
 *          gn_gss_import_name() currently relies on this feature
 *********************************************************************/
OM_uint32
sy_resolve_hostname( OM_uint32 *pp_min_stat,		/* out */
		     char * p_host,			/* in  */
		     char * p_fqdn,			/* out */
		     size_t p_maxlen )			/* in  */
{
   struct hostent * hent = NULL;
   char           * src;
   char           * dst;
   Uint             i;
   size_t           len;
   unsigned char    uch;

   (*pp_min_stat) = 0;

   if ( p_host==NULL  ||  *p_host=='\0' ) {
      RETURN_MIN_MAJ( MINOR_HOSTNAME_MISSING, GSS_S_FAILURE );
   }

   hent = gethostbyname(p_host);
   if ( hent==NULL
	||  hent->h_name==NULL
	||  hent->h_name[0]=='\0' ) {
      RETURN_MIN_MAJ( MINOR_HOST_LOOKUP_FAILURE, GSS_S_FAILURE );
   }

   len = strlen(hent->h_name);
   if ( len>=p_maxlen ) {
      RETURN_MIN_MAJ( MINOR_HOSTNAME_TOO_LONG, GSS_S_FAILURE );
   }

   if ( sy_check_valid_chars( (char *)(hent->h_name), (int)len,
			       CHECK_ALPHA|CHECK_DIGIT, ".-_" )!=1 ) {
        RETURN_MIN_MAJ(MINOR_INVALID_HOSTNAME, GSS_S_FAILURE);
   }

   src = (char *)hent->h_name;
   dst = p_fqdn;
   memset(p_fqdn, 0, p_maxlen);

   for( i=0 ; *src && i<p_maxlen ; i++, src++, dst++ ) {
     uch = *src;
     *dst = (char) (isupper(uch) ? tolower(uch) : uch );
   }

   *dst = 0;

   while( dst>p_fqdn && dst[-1]=='.' ) {
      /* remove any trailing dots after the FQDN hostname */
      dst--;
      *dst = 0;
   }

   return(GSS_S_COMPLETE); 

} /* sy_resolve_hostname() */



/*********************************************************************
 *  sy_get_hostname()
 *
 *  Description:
 *    Resolve MY own hostname into an FQDN (at least attempt to)
 *********************************************************************/
OM_uint32
sy_get_hostname( OM_uint32 * pp_min_stat,		/* out */
		 char * p_fqdn,				/* out */
		 size_t p_maxlen )			/* in  */
{
   OM_uint32   maj_stat;
   char        myhostname[256];

   (*pp_min_stat) = 0;
   p_fqdn[0] = '\0';

   if ( gethostname( myhostname, sizeof(myhostname)-1 )==-1 ) {
      DEBUG_ERR((tf, "ERROR: gethostname() failed (errno=%d)!\n", errno))

      RETURN_MIN_MAJ( MINOR_GETHOSTNAME_FAILED , GSS_S_FAILURE );
   }

   myhostname[sizeof(myhostname)-1] = '\0';

   maj_stat = sy_resolve_hostname(pp_min_stat, myhostname, p_fqdn, p_maxlen);

   return(maj_stat);

} /* sy_get_hostname() */

#endif /* HAVE_SOCKETS */


#ifdef _WIN32
/*********************************************************************
 *  sy_lasterror()
 *
 *  Description:
 *    Win32-specific hack similar to "strerror()" for the error codes
 *    at the Windows API, which are returned via GetLastError().
 *    Note that the values of GetLastError() overlap with ANSI errno-s
 *    and have a DIFFERENT meaning!
 *    GetLastError() may return Winsocket errors, but FormatMessage()
 *    ignores them completely ...   truely broken!
 *********************************************************************/

struct num_to_text_s {
	OM_uint32      value;
	char         * name;
        char         * desc;
};

#define MIN_STRUCT_ENTRY(code,desc)	\
	{  (code), # code , desc }


#define WSA_ENTRY(code,desc)     MIN_STRUCT_ENTRY( WSA ## code , desc )

static struct num_to_text_s wsa_error[] = {
/*    WSA_ENTRY(EACCESS,		 "permission denied for requested operation"	  ), */
    WSA_ENTRY(EFAULT,		 "invalid pointer argument for function call"	  ),
    WSA_ENTRY(EINTR,		 "blocking operation was interrupted"		  ),
    WSA_ENTRY(EINVAL,		 "invalid argument value for function call"	  ),
    WSA_ENTRY(EMFILE,		 "too many open sockets"			  ),
    WSA_ENTRY(EWOULDBLOCK,	 "asynchronous operation: try again later"	  ),
    WSA_ENTRY(EINPROGRESS,	 "a blocking operation is still in progress"	  ),
    WSA_ENTRY(EALREADY,		 "already have an async operation pending on this socket" ),
    WSA_ENTRY(ENOTSOCK,		 "invalid socket referenced"			  ),
    WSA_ENTRY(EDESTADDRREQ,      "destination address required"			  ),
    WSA_ENTRY(EMSGSIZE,          "message too long"				  ),
    WSA_ENTRY(EPROTOTYPE,        "protocol wrong for socket"			  ),
    WSA_ENTRY(ENOPROTOOPT,       "unsupported option/level"			  ),
    WSA_ENTRY(EPROTONOSUPPORT,   "protocol not supported"			  ),
    WSA_ENTRY(ESOCKTNOSUPPORT,   "socket type not supported"			  ),
    WSA_ENTRY(EOPNOTSUPP,        "operation not supported"			  ),
    WSA_ENTRY(EPFNOSUPPORT,      "protocol family not supported"		  ),
    WSA_ENTRY(EAFNOSUPPORT,      "address family incompatible with protocol"	  ),
    WSA_ENTRY(EADDRINUSE,        "port/address already in use"			  ),
    WSA_ENTRY(EADDRNOTAVAIL,     "not a valid port/address"			  ),
    WSA_ENTRY(ENETDOWN,          "network down/unavailable"			  ),
    WSA_ENTRY(ENETUNREACH,       "destination network unreachable"		  ),
    WSA_ENTRY(ENETRESET,         "network dropped connection on reset"		  ),
    WSA_ENTRY(ECONNABORTED,      "connection was locally aborted"		  ),
    WSA_ENTRY(ECONNRESET,        "remote host prematurely disconnected"		  ),
    WSA_ENTRY(ENOBUFS,           "no buffer space available"			  ),
    WSA_ENTRY(EISCONN,           "socket is already connected"			  ),
    WSA_ENTRY(ENOTCONN,          "socket is not connected"			  ),
    WSA_ENTRY(ESHUTDOWN,         "cannot send after shutdown"			  ),
    WSA_ENTRY(ETIMEDOUT,         "operation timed out"				  ),
    WSA_ENTRY(ECONNREFUSED,      "connection refused (nobody listening?)"	  ),
    WSA_ENTRY(ENAMETOOLONG,      "name too long"				  ),
    WSA_ENTRY(EHOSTDOWN,         "destination host is down/unavailable"		  ),
    WSA_ENTRY(EHOSTUNREACH,      "destination host unreachable"			  ),
    WSA_ENTRY(VERNOTSUPPORTED,   "requested winsocket version not supported"	  ),
    WSA_ENTRY(NOTINITIALISED,    "must call WSAStartUp() first"			  ),
    WSA_ENTRY(HOST_NOT_FOUND,    "hostname unknown"				  ),

    WSA_ENTRY(TRY_AGAIN,         "data not yet available, try again"		  ),
    WSA_ENTRY(NO_RECOVERY,       "non-recoverable error"			  ),
    WSA_ENTRY(NO_DATA,           "valid name, no data record of requested type"	  )

};



OM_uint32
sy_lasterror( OM_uint32 * pp_min_stat, OM_uint32 p_error, char *p_buf, size_t p_buf_len )
{
   char			 * this_Call = "sy_lasterror";
   char			 * errmsg    = NULL;
   DWORD		   rval;
   OM_uint32		   maj_stat  = GSS_S_COMPLETE;
   OM_uint32		   errnum    = 0;
   Uint			   i, j;

   p_buf[0] = '\0';

   if ( p_error>=WSABASEERR  &&  p_error<WSABASEERR+3000 ) {
      for ( i=0 ; i<ARRAY_ELEMENTS(wsa_error) ; i++ ) {
	 if ( wsa_error[i].value == p_error ) {
	    char  buf[256];
	    sprintf(buf, /* "%.50s: %.150s", wsa_error[i].name,*/ "%.150s", wsa_error[i].desc);
	    strncpy(p_buf, buf, p_buf_len);
	    p_buf[p_buf_len-1] = '\0';
	    break;
	 }
      }
   }

   /* skip FormatMessage if we already found a description of a WinSocket Error */
   if ( p_buf[0]=='\0' ) {

      rval=FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
			  | FORMAT_MESSAGE_FROM_SYSTEM
                          | FORMAT_MESSAGE_IGNORE_INSERTS,
			  NULL, (DWORD) p_error,
			  MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // en_US
			  (LPTSTR) &errmsg, 0, NULL );
      if ( rval==0 ) {
	 DEBUG_ERR((tf, "  E: %s(): FormatMessage failed with 0x%08lx\n",
			this_Call, (long)GetLastError() ))
	 ERROR_RETURN( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );
      }

      /* Life isn't easy ... */
      /* Win32 GetLastError() overlaps and differs from errno-s			 */
      /* When I last tested, FormatMessage() completely ignored Winsocket Errors */
      /* and the printable may contain arbitrary linebreaks ...		         */
      for ( i=0 , j=0 ; i<(p_buf_len-1) && j<rval ; ) {
	 if ( errmsg[j]=='\r' ) { j++;  continue;  }
	 p_buf[i] = (errmsg[j]=='\n') ? ' ' : errmsg[j];
	 i++;
	 j++;
      }

      p_buf[i] = '\0';

      /* finally, strip trailing spaces */
      while ( i>0 && p_buf[i-1]==' ' ) { i--; p_buf[i]='\0'; }

   }

error:
   if ( errmsg!=NULL ) {
      LocalFree( (LPVOID) errmsg );
   }

   return(maj_stat);

} /* sy_lasterror() */

#else /* !_WIN32 */

/*********************************************************************
 * sy_lasterror()
 * currently undefined for other platforms
 *********************************************************************/
OM_uint32
sy_lasterror( OM_uint32 * pp_min_stat, OM_uint32 p_error, char *p_buf, size_t p_buf_len )
{
   RETURN_MIN_MAJ( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );
} /* sy_lasterror() */

#endif  /* !_WIN32 */


/*********************************************************************
 *  sy_get_username()
 *
 *  Description:
 *    Return the name for the current user running this process.
 *
 *    This is very platform specific, and it may not work on platforms
 *    that do not have the notion of a user (e.g. Dos/Win3/Win95/Mac/OS-2)
 *********************************************************************/
#if defined(HAVE_PWD_H)

OM_uint32
sy_get_username( OM_uint32 * pp_min_stat,		/* out  */
		 char *p_myname,			/* out  */
		 size_t p_maxlen )			/* in   */
{
   uid_t             my_uid;
   struct passwd   * pwent;

   (*pp_min_stat) = 0;
   p_myname[0]= '\0';

   my_uid = geteuid();
   pwent  = getpwuid( my_uid );

   if ( pwent==NULL ) {
      DEBUG_ERR((tf, "ERROR: getpwuid(geteuid()) failed!\n"))
      RETURN_MIN_MAJ( MINOR_YOU_DONT_EXIST, GSS_S_FAILURE );
   }

   if ( (strlen(pwent->pw_name) + 1) < p_maxlen ) {
      RETURN_MIN_MAJ( MINOR_NAME_TOO_LONG, GSS_S_FAILURE );
   }

   /* we know that it will fit, so strncpy will add the trailing '\0' */
   strncpy( p_myname, pwent->pw_name, p_maxlen );

   return(GSS_S_COMPLETE);

} /* sy_get_username() */

#else /* !HAVE_PWD_H */

#  if defined(_WIN32)
   
/*********************************************************************
 *  sy_get_username()
 *
 *  Description:
 *    Get the name of the current user on M$-Win32 platforms
 *
 *    Does this work on Windows '95??
 *********************************************************************/
OM_uint32
sy_get_username( OM_uint32 * pp_min_stat,		/* out */
		 char *p_myname,			/* out */
		 size_t p_maxlen )			/* in  */
{
   DWORD   buflen = (DWORD)(p_maxlen-1);

   (*pp_min_stat) = 0;
   p_myname[0] = '\0';

   if ( GetUserName(p_myname, &buflen)==0 ) {
      DEBUG_ERR((tf, "ERROR: GetUserName() failed!\n"))
      RETURN_MIN_MAJ( MINOR_YOU_DONT_EXIST, GSS_S_FAILURE );
   }
   p_myname[p_maxlen-1] = '\0';

   return(GSS_S_COMPLETE);

} /* sy_get_username() */

#  else /* !_WIN32 */
/*********************************************************************
 *  sy_get_username()
 *
 *  Description:
 *    Get the name of the current user on OSes that do not
 *    know and/or do not distinguish users   "emulation".
 *********************************************************************/

OM_uint32
sy_get_username( OM_uint32 * pp_min_stat,		/* out */
		 char *p_myname,			/* out */
		 size_t p_maxlen )			/* in  */
{
   (*pp_min_stat) = 0;

   strncpy( p_myname, "John.Doe", p_maxlen-1 );
   p_myname[p_maxlen-1] = '\0';

   return(GSS_S_COMPLETE);

} /* sy_get_username() */

#  endif /* !_WIN32 */

#endif /* !HAVE_PWD_H */




   
#if defined(HAVE_PWD_H) && defined(HAVE_UID_T)

/*********************************************************************
 *  sy_uid_2_name()
 *
 *  Description:
 *    Convert a POSIX uid_t into a username.
 *
 *    This routine accepts a "binary" uid_t and a "decimal string" uid_t
 *    to support the GSS-API nametypes GSS_C_NT_MACHINE_UID
 *    and GSS_C_NT_STRING_UID
 *********************************************************************/
OM_uint32
sy_uid_2_name( OM_uint32 * pp_min_stat, void * p_uid, size_t p_uidlen,
	       int  p_num, char * p_oname, int p_maxolen, size_t * pp_olen )
{
   uid_t             my_uid;
   struct passwd   * pwent;

   (*pp_min_stat) = 0;
   p_oname[0]     = '\0';
   (*pp_olen)     = 0;

   if ( p_uidlen!=sizeof(uid_t) ) {
      RETURN_MIN_MAJ( MINOR_BAD_UID, GSS_S_BAD_NAME );
   }

   if ( p_num ) {

      /* the UID is passed down numerically */
      my_uid = ((uid_t *) p_uid)[0];

   } else {

      /* the UID is passed down as a decimal number string */
      errno = 0;
      my_uid  = (uid_t) strtoul( (char *)p_uid, NULL, 10 );
      if ( errno!=0 ) {
	 RETURN_MIN_MAJ( MINOR_BAD_UID, GSS_S_BAD_NAME );
      }

   }

   pwent  = getpwuid( my_uid );

   if ( pwent==NULL ) {
      DEBUG_ERR((tf, "ERROR: getpwuid(%lu) failed!\n", (unsigned long) my_uid))
      RETURN_MIN_MAJ( MINOR_YOU_DONT_EXIST, GSS_S_FAILURE );
   }

   if ( (strlen(pwent->pw_name) + 1) < p_maxolen ) {
      RETURN_MIN_MAJ( MINOR_NAME_TOO_LONG, GSS_S_FAILURE );
   }

   /* we know that it will fit, so strncpy will add the trailing '\0' */
   strncpy( p_oname, pwent->pw_name, p_maxolen );

   return(GSS_S_COMPLETE);

} /* sy_uid_2_name() */


int
sy_uid_avail(void)
{
   return(TRUE);
}

#else /* !HAVE_PWD_H  &&  !HAVE_UID_T */

/*********************************************************************
 *  sy_uid_2_name()
 *
 *  Description:
 *    non-POSIX environments don't have (uid_t)s
 *********************************************************************/

OM_uint32
sy_uid_2_name( OM_uint32 * pp_min_stat, void * p_uid, size_t p_uidlen,
	       int  p_num, char * p_oname, int p_maxolen, size_t * pp_olen )
{
   UNREFERENCED_PARAMETER(p_uid);
   UNREFERENCED_PARAMETER(p_uidlen);
   UNREFERENCED_PARAMETER(p_num);
   UNREFERENCED_PARAMETER(p_oname);
   UNREFERENCED_PARAMETER(p_maxolen);
   UNREFERENCED_PARAMETER(pp_olen);

   RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_BAD_NAMETYPE );

} /* sy_uid_2_name() */

int sy_uid_avail(void)
{
   return(FALSE);
}
#endif /* !HAVE_PWD_H  &&  !HAVE_UID_T */



/*
 * sy_strcasecmp()
 *
 * Case independent string compare
 *
 */
int
sy_strcasecmp( const char *s1, const char *s2 )
{
   const unsigned char  * us1 = (unsigned char *)s1;
   const unsigned char  * us2 = (unsigned char *)s2;
   int                    lc1 = 0;
   int                    lc2 = 0;

   if ( us1==us2 )
      return(0);

   /* make this call safe against calling with NULL ptrs ... */
   if ( us1==0 ) return(-1);
   if ( us2==0 ) return(1);

   for ( ; lc1==lc2 ; us1++, us2++ ) {
      lc1 = tolower(*us1);
      lc2 = tolower(*us2);

      /* stop the comparison when either there is a difference */
      /* or we hit the end of one (or both) of the strings     */
      if ( lc1 != lc2 || lc1==0 || lc2==0 )
         break;
   }

   return( lc1 - lc2 );

} /* sy_strcasecmp() */



/*
 * sy_strncasecmp()
 *
 * Case independent string compare with a length limit
 *
 */
int
sy_strncasecmp( const char *s1, const char *s2, size_t count )
{
   const unsigned char   * a = (const unsigned char *) s1;
   const unsigned char   * b = (const unsigned char *) s2;
   int                     c1 = 0;
   int                     c2 = 0;
   size_t		   i;

   if ( a==b )
      return(0);

   for( i=0 ; i<count  &&  c1==c2 ; a++, b++, i++ ) {
      c1 = isupper(*a) ? tolower(*a) : *a ;
      c2 = isupper(*b) ? tolower(*b) : *b ;
      if ( c1==0 || c2==0 )
	 break;
   }
   
   return( c2 - c1 );

} /* sy_strncasecmp() */


#ifdef _WIN32
LONG
sy_RegQueryValue(
      HKEY       p_key,
      char     * p_subkey_name,
      char     * p_value,
      DWORD      p_regtype,
      DWORD    * pp_dwret,
      char     * p_strbuf,
      size_t     p_strbuf_max )
{
   char     * this_Call = "sy_getregval";
   HKEY       dyn_key   = (HKEY)0;  /* dynamic -- call RegCloseKey() */
   LONG       lrc       = ERROR_SUCCESS;
   DWORD      valueL;
   DWORD      regtype;
   LPBYTE     valueptr;

   
   if ( 0!=pp_dwret )       { (*pp_dwret) = (DWORD)0; }
   if ( 0!=p_strbuf
        && p_strbuf_max>0 ) { p_strbuf[0] = 0;        }

   if ( REG_DWORD==p_regtype ) {
      if ( 0==pp_dwret ) {
         DEBUG_ERR((tf, "  E: %s(): Invalid FuncArgs: REG_DWORD and NULL==pp_dwret\n",
                        this_Call ));
         goto error;
      }
      valueptr   = (LPBYTE) pp_dwret;
      valueL     = sizeof(DWORD);
   } else if ( REG_SZ==p_regtype ) {
      if ( 0==p_strbuf || p_strbuf_max<2 ) {
         DEBUG_ERR((tf, "  E: %s(): Invalid FuncArgs: REG_SZ and p_strbuf=%p, p_strbuf_max=%lu\n",
                        this_Call, p_strbuf, (unsigned long)p_strbuf_max ));
         goto error;
      }
      valueptr  = (LPBYTE) p_strbuf;
      valueL    = p_strbuf_max - 1;
   } else {
      DEBUG_ERR((tf, "  E: %s(): Invalid FuncArgs: unrecognized p_regtype %d\n",
                     this_Call, (int)p_regtype ));
      goto error;
   }

   lrc = RegOpenKeyEx( p_key, p_subkey_name, 0, KEY_READ, &dyn_key );
   if ( ERROR_SUCCESS==lrc ) {
      regtype = p_regtype;
      lrc = RegQueryValueEx( dyn_key, p_value, NULL,
                             &regtype, valueptr, &valueL );
      if ( REG_SZ==p_regtype ) {
         if ( ERROR_SUCCESS==lrc && REG_SZ==regtype ) {
            p_strbuf[valueL] = 0;
         } else{
            p_strbuf[0] = 0;
         }
      }

      RegCloseKey( dyn_key );
      dyn_key = (HKEY)0;
   }

error:
   return(lrc);

} /* sy_getregval() */


#endif /* _WIN32 */
