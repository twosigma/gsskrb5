#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/dbg_fn.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/dbg_fn.c#2 $
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


#ifdef NDEBUG

  /* no debug support code for NDEBUG / OPT makes */

#else

#ifdef _WIN32
#  define WIN32_MEAN_AND_LEAN 1
#  include <windows.h>
#  include "BaseTsd.h"
#  define SECURITY_WIN32 1
#  include "security.h"
#  include <winnt.h>
#  define snprintf  _snprintf
#  define ENVVAR_NAME(x)   "%" x "%"
#  define DIR_SEP_CHAR     "\\"
#else
#  define ENVVAR_NAME(x)   "${" x "}"
#  define DIR_SEP_CHAR     "/"
#endif /* _WIN32 */

#include "gssmaini.h"

#include <time.h>


#define MYPARAM  "\t%-15.30s "

FILE FAR     * tf              = NULL;
OM_uint32      dbg_info;
int            dbg_enable = 0;
int            dbg_func[FUNC_MAX];


static char * known_nt_tags[] = {
	"",
	"GSS_C_NT_HOSTBASED_SERVICE",
	"GSS_C_NT_ANONYMOUS",
	"GSS_C_NT_EXPORTED_NAME",
	"GSS_C_NT_USER_NAME",
	"GSS_C_NT_MACHINE_UID_NAME",
	"GSS_C_NT_STRING_UID_NAME",
	"NT_PRIVATE_1",
	"NT_PRIVATE_2",
	"NT_PRIVATE_3",
	"NT_PRIVATE_4",
	"NT_PRIVATE_5",
	"NT_PRIVATE_6",
	"NT_PRIVATE_7",
	"NT_PRIVATE_8",
	"NT_PRIVATE_9"
};

static char * known_mech_tags[] = {
	"",
	"MECH_1",
	"MECH_2",
	"MECH_3",
	"MECH_4",
	"MECH_5"
};


struct dbg_known_func_s {
    int   index;
    char *name;
};

#define FUNCTABLE_ENTRY(fn)   { FUNC_##fn, "gss_" #fn }

static struct dbg_known_func_s dbg_known_func[]= {
	FUNCTABLE_ENTRY(acquire_cred),
	FUNCTABLE_ENTRY(add_cred),
	FUNCTABLE_ENTRY(inquire_cred),
	FUNCTABLE_ENTRY(inquire_cred_by_mech),
	FUNCTABLE_ENTRY(release_cred),
	FUNCTABLE_ENTRY(init_sec_context),
	FUNCTABLE_ENTRY(accept_sec_context),
	FUNCTABLE_ENTRY(delete_sec_context),
	FUNCTABLE_ENTRY(process_context_token),
	FUNCTABLE_ENTRY(context_time),
	FUNCTABLE_ENTRY(inquire_context),
	FUNCTABLE_ENTRY(wrap_size_limit),
	FUNCTABLE_ENTRY(export_sec_context),
	FUNCTABLE_ENTRY(import_sec_context),
	FUNCTABLE_ENTRY(get_mic),
	FUNCTABLE_ENTRY(sign),
	FUNCTABLE_ENTRY(verify_mic),
	FUNCTABLE_ENTRY(verify),
	FUNCTABLE_ENTRY(wrap),
	FUNCTABLE_ENTRY(seal),
	FUNCTABLE_ENTRY(unwrap),
	FUNCTABLE_ENTRY(unseal),
	FUNCTABLE_ENTRY(import_name),
	FUNCTABLE_ENTRY(display_name),
	FUNCTABLE_ENTRY(compare_name),
	FUNCTABLE_ENTRY(release_name),
	FUNCTABLE_ENTRY(inquire_names_for_mech),
	FUNCTABLE_ENTRY(inquire_mechs_for_name),
	FUNCTABLE_ENTRY(canonicalize_name),
	FUNCTABLE_ENTRY(export_name),
	FUNCTABLE_ENTRY(duplicate_name),
	FUNCTABLE_ENTRY(display_status),
	FUNCTABLE_ENTRY(indicate_mechs),
	FUNCTABLE_ENTRY(release_buffer),
	FUNCTABLE_ENTRY(release_oid_set),
	FUNCTABLE_ENTRY(create_empty_oid_set),
	FUNCTABLE_ENTRY(add_oid_set_member),
	FUNCTABLE_ENTRY(test_oid_set_member),
	{ 0, NULL }
};

static char          * dbg_func_names[FUNC_MAX];
static unsigned long   dbg_func_call_cntr[FUNC_MAX];

static char dbgfn[256];



int daypermonth[] =
   { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

static char *daynames[] =
   { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

static char *monthnames[] =
   { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static int tz_diff_in_minutes = 0;

/*
 * determine_tz_min_diff()
 *
 */
static int
determine_tz_min_diff( void )
{
   SYSTEMTIME    gst;
   SYSTEMTIME    lst;
   long          gt;
   long          lt;
   long          delta;
   long          tz_min_diff;

   GetSystemTime( &gst );
   GetLocalTime( &lst );
   delta = (gst.wYear - 2);
   gt = (gst.wYear-delta)*365 + daypermonth[gst.wMonth-1] + (gst.wDay-1);
   gt = (gt*24 + gst.wHour)*60 + gst.wMinute;
   lt = (lst.wYear-delta)*365 + daypermonth[lst.wMonth-1] + (lst.wDay-1);
   lt = (lt*24 + lst.wHour)*60 + lst.wMinute;
   tz_min_diff = (lt - gt);
   /* calculate time zone difference in granularity of quarter hours */
   tz_min_diff = (tz_min_diff/15)*15;

   return((int)tz_min_diff);

} /* determine_tz() */



/*
 * dbg_snprint_systemtime()
 *
 */
char *
dbg_snprint_systemtime( char * p_buf, size_t p_buf_max, SYSTEMTIME * p_systime, int p_flags )
{
   SYSTEMTIME          curtime;
   static BOOL         have_tz = FALSE;
   char              * rval;
   char                datebuf[32];
   char                millisbuf[8];
   char                tzbuf[32];
   char                dayname[8];
   
   p_buf[0]   = 0;
   rval       = &(p_buf[0]);

   if ( FALSE==have_tz ) {
      tz_diff_in_minutes = determine_tz_min_diff();
      have_tz = TRUE;
   }

   if ( NULL==p_systime ) {
      GetLocalTime(&curtime);
      p_systime = &curtime;
   }

   millisbuf[0] = 0;
   tzbuf[0]     = 0;
   dayname[0]   = 0;
   datebuf[0]   = 0;
   if ( DBG_TIME_TRACE==(DBG_TIME_TRACE&p_flags) ) {
      snprintf(datebuf,sizeof(datebuf),"%04d%02d%02d--",
               (int)p_systime->wYear,
               (int)p_systime->wMonth,
               (int)p_systime->wDay );
      p_flags |= DBG_TIME_SHOW_MILLISEC;
   } else {
      snprintf(datebuf,sizeof(datebuf),"%02d-%3s-%04d ",
               (int)p_systime->wDay,
               monthnames[p_systime->wMonth-1],
               (int)p_systime->wYear );
   }
   if ( DBG_TIME_SHOW_MILLISEC==(DBG_TIME_SHOW_MILLISEC&p_flags) ) {
      snprintf(millisbuf,sizeof(millisbuf),".%03d",(int)(p_systime->wMilliseconds%1000));
   }
   if ( DBG_TIME_SHOW_TIMEZONE==(DBG_TIME_SHOW_TIMEZONE&p_flags) ) {
      snprintf(tzbuf,sizeof(tzbuf),"  GMT %+03d:%02d", (int)(tz_diff_in_minutes/60),
               (int)((tz_diff_in_minutes>=0) ? (tz_diff_in_minutes%60)
                                                : ((0 - tz_diff_in_minutes)%60)) );
   }
   if ( DBG_TIME_SHOW_DAYNAME==(DBG_TIME_SHOW_DAYNAME&p_flags) ) {
      snprintf(dayname,sizeof(dayname), "%3s, ", daynames[p_systime->wDayOfWeek]);
   }
   datebuf[sizeof(datebuf)-1]     = 0;
   millisbuf[sizeof(millisbuf)-1] = 0;
   tzbuf[sizeof(tzbuf)-1]         = 0;
   dayname[sizeof(dayname)-1]     = 0;

   snprintf(p_buf,p_buf_max, "%s%s%02d:%02d:%02d%s%s",
            dayname, datebuf,
            (int)p_systime->wHour,
            (int)p_systime->wMinute,
            (int)p_systime->wSecond,
            millisbuf,
            tzbuf );

   p_buf[p_buf_max-1] = 0;

   return(rval);

} /* dbg_snprint_systemtime() */



/*
 * dbg_snprint_timestamp()
 *
 */
char *
dbg_snprint_timestamp( char * p_buf, size_t p_buf_max, TimeStamp * p_time, int p_show_tz )
{
   char              * rval;
   FILETIME            fltm;
   SYSTEMTIME          st;

   p_buf[0] = 0;
   rval = &(p_buf[0]);

   if ( NULL!=p_time ) {
      if ( p_time->HighPart>=0x7f000000 ) {
         snprintf(p_buf,p_buf_max, "**No Expiration**  TimeStamp = { 0x%08x, 0x%08x }",
                 (int)(p_time->HighPart), (int)(p_time->LowPart) );
         p_buf[p_buf_max-1] = 0;
      } else {
         fltm.dwLowDateTime  = p_time->LowPart;
         fltm.dwHighDateTime = p_time->HighPart;
         FileTimeToSystemTime( &fltm, &st );
         rval = dbg_snprint_systemtime( p_buf, p_buf_max, &st, p_show_tz );
      }
   }

   return(rval);

} /* dbg_snprint_timestamp() */



/*
 * dbg_snprint_time_t()
 *
 */
char *
dbg_snprint_time_t( char * p_buf, size_t p_buf_max, time_t p_time_t )
{
   char        * rval;
   struct tm   * gtime;

   p_buf[0] = 0;
   rval     = &(p_buf[0]);

   if ( 0==p_time_t ) {
      p_time_t = time(NULL);
   }

   gtime = gmtime( &p_time_t );
   if ( NULL==gtime ) {
      snprintf(p_buf,p_buf_max, "Huh? gmtime(0x%08lx) failed!", (long)p_time_t );
   } else {
      snprintf(p_buf,p_buf_max, "%s, %02d-%3s-%04d  %02d:%02d:%02d  GMT",
              daynames[gtime->tm_wday],
              (int) gtime->tm_mday,
              monthnames[gtime->tm_mon],
	      (int)(1900+gtime->tm_year),
	      (int)gtime->tm_hour,
	      (int)gtime->tm_min,
	      (int)gtime->tm_sec );
   }
   p_buf[p_buf_max-1] = 0;

   return( rval );

} /* dbg_snprint_time_t() */



/*
 * dbg_init()
 *
 * Description:
 *   Initialization call for DEBUG functions and data structures
 *
 *   This function will be called ONCE and INITIALLY when an application
 *   does it's first call into one of the GSS-API functions.
 *
 *   GSS-API v1 and v2 do not have an initialization function, so the
 *   check has to be done within every API-call.
 *
 */
OM_uint32
dbg_init( OM_uint32 * pp_min_stat )
{
   int i;

   (*pp_min_stat) = 0;

#if 1
   /* HARDCODED: switch on credentials and context establishment traces */
   /* and write it to a file with the hardcoded name "gsskrb5-trc.txt"  */
   /* into directory  %TEMP%, %TMP% or "C:\"                            */

   /* ... one day this should be parsed out of     */
   /* a configuration file or from the registry... */

   if ( tf==NULL ) {
        char       * ptr;
        char       * trcfname_where = "hardcoded";
        char         timestr[128];

        ptr = getenv("GSSKRB5_TRACEFILE");
        if ( NULL!=ptr ) {
            trcfname_where = ENVVAR_NAME( "GSSKRB5_TRACEFILE" );
            strncpy(dbgfn,ptr,sizeof(dbgfn)-1);
            dbgfn[sizeof(dbgfn)-1] = 0;
        }

        if ( 0==dbgfn[0] ) {
#ifdef _WIN32
            LONG  lrc;

            lrc = sy_RegQueryValue( HKEY_CURRENT_USER, "Software\\SAP\\gsskrb5", "Tracefile",
                              REG_SZ, NULL, &(dbgfn[0]), sizeof(dbgfn)-1 );
            if ( ERROR_SUCCESS==lrc ) {
               trcfname_where = "HKCU\\Software\\SAP\\gsskrb5\\Tracefile";
            } else {
               lrc = sy_RegQueryValue( HKEY_LOCAL_MACHINE, "SOFTWARE\\SAP\\gsskrb5", "Tracefile",
                              REG_SZ, NULL, &(dbgfn[0]), sizeof(dbgfn)-1 );
               if ( ERROR_SUCCESS!=lrc ) {
                   trcfname_where = "HKLM\\Software\\SAP\\gsskrb5\\Tracefile";
               }

            }
#endif /* _WIN32 */

           if ( 0==dbgfn[0] ) {
	      ptr = getenv("TEMP");
	      if ( NULL!=ptr ) {
                 trcfname_where = ENVVAR_NAME("TEMP");
              } else {
                 ptr = getenv("TMP");
   	         if ( NULL!=ptr ) {
                    trcfname_where = ENVVAR_NAME("TMP");
                 } else {
#ifdef _WIN32
                    ptr = "C:";
#else
                    ptr = "/tmp";
#endif
                    trcfname_where = "hardcoded";
                 }
              }
	      sprintf(dbgfn, "%.128s%sgsskrb5-trc.txt",ptr, DIR_SEP_CHAR);
           }
        }
        dbgfn[sizeof(dbgfn)-1] = 0;

	tf = fopen(dbgfn, "at+");
	if ( NULL==tf ) {
		tf = stderr;
	}
	fprintf(tf, "\n========== NEW TRACE START: %s ==========\n",
                dbg_snprint_systemtime(timestr,sizeof(timestr), NULL, TRUE) );
        fprintf(tf, "Tracefile (from %s) = %s\n\n", trcfname_where, dbgfn );
   }

   for( i=0 ; i<ARRAY_ELEMENTS(dbg_func) ; i++ ) {
      dbg_func[i] = 0;
   }

   dbg_func[FUNC_acquire_cred]       = 1;
   /* dbg_func[FUNC_release_cred]       = 1; */
   /* dbg_func[FUNC_init_sec_context]   = 1; */
   /* dbg_func[FUNC_accept_sec_context] = 1; */
   /* dbg_func[FUNC_delete_sec_context] = 1; */
   /* dbg_func[FUNC_unwrap]	        = 1; */
   /* dbg_func[FUNC_wrap]		= 1; */
   /* dbg_func[FUNC_context_time]	= 1; */
   /* dbg_func[FUNC_inquire_context]    = 1; */

#else

   /* HARDCODED: switch on all traces              */
   /* ... one day this should be parsed out of     */
   /* a configuration file or from the registry... */
   for( i=0 ; i<ARRAY_ELEMENTS(dbg_func) ; i++ ) {
      dbg_func[i] = 1;
   }

#endif

   dbg_info = 0xFFFFFFFF; /* set all info type bits to "1" */
   dbg_info = INFO_ACTION;

   for( i=0 ; dbg_known_func[i].name!=NULL ; i++ ) {
      dbg_func_names[ dbg_known_func[i].index ] = dbg_known_func[i].name;
   }

   return(GSS_S_COMPLETE);

} /* dbg_init() */



/*********************************************************************
 *
 *  dbg_oid():
 *
 *
 *********************************************************************/
void
dbg_oid( OM_uint32 p_dbgmask, char * p_parm_name, gss_OID p_oid )
{
   char                     * symbolic_name;
   gss_buffer_desc            buffer;
   OM_uint32                  min_stat;
   OM_uint32		      maj_stat    = GSS_S_COMPLETE;
   gn_nt_tag_et		      nt_tag;
   gn_mech_tag_et             mech_tag;
   int                        save_dbg_enable;

   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   buffer.value  = NULL;
   buffer.length = (size_t)0;

   symbolic_name = "";

   /* suppress debug information from gn_alloc_buffer as well as */
   /* gn_gss_release_buffer during gn_gss_oid_to_str             */
   save_dbg_enable = dbg_enable; 
   dbg_enable      = 0;

   if ( p_oid!=GSS_C_NO_OID ) {

      maj_stat = gn_oid_to_mech_tag( &min_stat, p_oid, &mech_tag );
      if ( maj_stat==GSS_S_COMPLETE ) {
	 if ( mech_tag!=MECH_INVALID_TAG ) {
	    if ( mech_tag < ARRAY_ELEMENTS(known_mech_tags) ) {
	       symbolic_name = known_mech_tags[mech_tag];
	    }

	 } else {

	    maj_stat = gn_oid_to_nt_tag( &min_stat, p_oid, &nt_tag );
	    if (maj_stat!=GSS_S_COMPLETE)
	       goto bad_oid;

	    if ( nt_tag!=NT_INVALID_TAG ) {
	       if ( nt_tag < ARRAY_ELEMENTS(known_nt_tags) ) {
		  symbolic_name = known_nt_tags[nt_tag];
	       }
	    }
	 }

      } else { /* maj_stat!=GSS_S_COMPLETE */

bad_oid: DEBUG_ERR((tf, "dbg_oid(): gn_oid_to_mech_tag failed! Bad OID ?\n"))

      } /* maj_stat!=GSS_S_COMPLETE */

      maj_stat = gn_gss_oid_to_str( &min_stat, p_oid, &buffer );
      if (maj_stat==GSS_S_COMPLETE ) {
	 DEBUG_VAL((tf, MYPARAM "= %.100s  %.30s\n",
	    p_parm_name, buffer.value!=NULL ? (char *)buffer.value : "", symbolic_name ));

	 dbg_oid_content( INFO_OID_DATA, p_oid );

      } else {

         DEBUG_VAL((tf, MYPARAM "is not a valid OID\n", p_parm_name ));

      }

   } else { /* p_oid==GSS_C_NO_OID */

      DEBUG_VAL((tf, MYPARAM "= NULL         (GSS_C_NO_OID)\n", p_parm_name ));

   }

   if ( buffer.length != 0 ) {
      (void)gn_gss_release_buffer( &min_stat, &buffer );
   }

   /* restore debug output settings to original state */
   dbg_enable      = save_dbg_enable;

} /* dbg_oid() */


void
dbg_oid_content( OM_uint32 p_dbgmask, gss_OID p_oid )
{
   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   if ( p_oid->length>0  &&  p_oid->elements!=NULL ) {

      gss_buffer_desc   oid_buffer;

      oid_buffer.length = p_oid->length;
      oid_buffer.value  = p_oid->elements;

      dbg_buffer_content( 0, &oid_buffer );
   }

   return;

} /* dbg_oid_content() */


/*********************************************************************
 *
 *  dbg_oid_set():
 *
 *
 *********************************************************************/
void
dbg_oid_set( OM_uint32 p_dbgmask,
	     char * p_parm_name , gss_OID_set p_oidset )
{
   gss_OID        oid;
   unsigned int   count, i;
   char           prefix[30];

   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   if ( p_oidset == (gss_OID_set)NULL ) {
      DEBUG_VAL((tf, MYPARAM "= NULL         (GSS_C_NO_OID_SET)\n", p_parm_name ));
      return;
   }

   count = (unsigned int) (p_oidset->count);
   oid   =                 p_oidset->elements;

   DEBUG_VAL((tf, MYPARAM "= { count= %u, elements= %p }\n",
	          p_parm_name, (unsigned int) count, (void *)oid )); 

   if ( count>0 && oid!=NULL ) {
      for ( i=0 ; i<count ; i++ ) {
	 sprintf(prefix, "elements[%2u]", i);
	 dbg_oid( INFO_OID, prefix, &(oid[i]) );
      }
   }

   return;

} /* dbg_oid_set() */



/*********************************************************************
 *
 *  dbg_buffer_head():
 *
 *
 *********************************************************************/
void
dbg_buffer_head( OM_uint32 p_dbgmask,
		 char * p_parm_name , gss_buffer_t p_buffer )
{
   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   if (p_buffer==NULL) {
      DEBUG_VAL((tf, MYPARAM "= NULL         (GSS_C_NO_BUFFER)\n", p_parm_name));

   } else if ( p_buffer->length==0  &&  p_buffer->value==NULL ) {

      DEBUG_VAL((tf, MYPARAM "= { 0, NULL }  (GSS_C_EMPTY_BUFFER)\n",
		 p_parm_name ));

   } else {

      DEBUG_VAL((tf, MYPARAM "= { length= %lu, value= %p }\n", p_parm_name,
		 (unsigned long)(p_buffer->length), (p_buffer->value) ));

   }

   return;

} /* dbg_buffer_head() */





static const char dbg_ascii_table[128] = {
  /* 0x00 */  '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
  /* 0x08 */  '.',  '.',  '.',  '.',  '.',  '.',  '.',  ',',
  /* 0x10 */  '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
  /* 0x18 */  '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.',
  /* 0x20 */  ' ',  '!',  '"',  '#',  '$',  '%',  '&', '\'',
  /* 0x28 */  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
  /* 0x30 */  '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
  /* 0x38 */  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
  /* 0x40 */  '@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',
  /* 0x48 */  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
  /* 0x50 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
  /* 0x58 */  'X',  'Y',  'Z',  '[', '\\',  ']',  '^',  '_',
  /* 0x60 */  '`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',
  /* 0x68 */  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
  /* 0x70 */  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
  /* 0x78 */  'x',  'y',  'z',  '{',  '|',  '}',  '~',  '.'
};



/*********************************************************************
 *
 *  dbg_hexdump():
 *
 *
 *********************************************************************/
void
dbg_hexdump( char *p_base, size_t offset, size_t p_num, size_t p_align )
{
   char           * ptr;
   size_t           xpos, i;
   char             line[200];
   unsigned char    uch;

   /* prefix each line with a hexadecimal offset */
   sprintf(line, "  %06lx  ", (unsigned long)offset );
   xpos = strlen(line);
   ptr  = &(line[xpos]);


   /* convert the bytes in the current line into printable hex */
   for ( i=0 ; i<p_align ; i++ ) {
      if ( i<p_num ) {
	 uch= (unsigned char) p_base[i];
	 sprintf( ptr, "%02x  ", (int) uch );
      } else {
	 strcpy( ptr, "    " );
      }
      ptr = ptr + 3 + ( ((i%8)==7) ? 1 : 0 );
   }

   xpos = strlen(line);
   ptr  = &(line[xpos]);

   /* append a character representation to the dumped line */
   for ( i=0 ; i<p_align ; i++ ) {
      /* insert a space before every block of 4 characters */
      if ( (i%8)==0 ) {
         *(ptr++) = ' ';
      }
      uch = (i<p_num) ? p_base[i] : ' ';
      *(ptr++) = dbg_ascii_table[ ((uch>127) ? 127 : uch) ];
   }

   /* terminate string */
   *(ptr++)= '\0';

   DEBUG_VAL((tf, "%.100s\n", line ));

   return;

} /* dbg_hexdump() */





/*********************************************************************
 *
 *  dbg_buffer_content():
 *
 *
 *********************************************************************/
void
dbg_buffer_content( OM_uint32 p_dbgmask, gss_buffer_t p_buffer )
{
   char    * this_Call = "dbg_buffer_content";
   char    * start_ptr;
   size_t    length, remaining, bytes_done, bytes_to_show;
   size_t    bytes_per_line= 16;

   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   if ( p_buffer==NULL
	||  p_buffer->length==0 ) {
      /* GSS_C_NO_BUFFER oder GSS_C_EMPTY_BUFFER */
      return;
   }

   start_ptr  = p_buffer->value;
   length     = p_buffer->length;
   
   if ( start_ptr==NULL ) {
      /* GSS_C_EMPTY_BUFFER  oder  Fehler ?! */
      DEBUG_ERR((tf, "%s(): Invalid buffer struct { %lu, NULL }\n",
		 this_Call, (unsigned long)length));
      return;
   }

   for( bytes_done=0 ; bytes_done<length ; ) {
      remaining = length - bytes_done;
      bytes_to_show = (remaining>=bytes_per_line )
	              ? bytes_per_line : remaining ;
      dbg_hexdump( &(start_ptr[bytes_done]), bytes_done,
		   bytes_to_show, bytes_per_line );
      bytes_done += bytes_to_show;
   }

   return;

} /* dbg_buffer_content() */


/*
 * dbg_begin()
 *
 */
char *
dbg_begin( dbg_functions_et func, OM_uint32 * pp_min_stat )
{
   char  *func_name;

   dbg_enable = dbg_func[func];
   func_name = dbg_func_names[func];
   if (func_name!=NULL) {
      /* memorize how often a function was called */
      dbg_func_call_cntr[func]++;
   } else {
      DEBUG_ERR((tf, "dbg_begin(): Uh-oh, Function table incomplete!!\n"))
      func_name = "Unknown -- fix the source";
   }

   if ( tf!=NULL  &&  dbg_enable  &&  (dbg_info & INFO_ARG) ) {
      fprintf(tf, "====================\n--> %s (\n"
	          "\t\t&minor_status          = ptr:%p,\n", func_name, pp_min_stat );
      fflush(tf);
   }

   return(func_name);

} /* dbg_begin() */



/*
 * dbg_ret()
 *
 */
void
dbg_ret( char * p_func_name, OM_uint32 *pp_minor, OM_uint32 p_major )
{
   gss_buffer_desc   msg_buf;
   PRINT_NUMBER      d_major;
   char             *routine, *call, *suppi, *desc_maj;
   OM_uint32         maj_stat, min_stat;
   OM_uint32	     context;


   fprintf(tf, "<-- %.50s( ", p_func_name);

   if ( pp_minor!=NULL && *pp_minor!=0 ) {
      msg_buf.length = 0;
      msg_buf.value  = NULL;
      context        = 0;
      maj_stat = gn_gss_display_status( &min_stat, *pp_minor, GSS_C_MECH_CODE,
				        GSS_C_NO_OID, &context, &msg_buf );
      if ( msg_buf.length>0 && msg_buf.value!=NULL ) {
	 fprintf(tf, "minor_status:= %.*s ", (int)msg_buf.length, msg_buf.value );
	 gn_gss_release_buffer( &min_stat, &msg_buf );
      }
   } /* endif   have minor status */

   if ( gn_major_text( p_major, &d_major,
		       &routine, &call, &suppi, &desc_maj )==FALSE ) {

      /* Unknown major error code */
      fprintf(tf, "):= %s %s\n", routine, d_major.digits );

   } else {

      if ( call[0]!='\0' ) {
	 /* Calling error */
	 fprintf(tf, "):= %s\n", call );

      } else { /* call[0]=='\0' */

	 /* Routine status code */
	 fprintf(tf, "):= %s\n", routine);

	 if ( suppi[0]!='\0' ) {
	    /* Supplementary Info */
	    fprintf(tf, "\t\tsupplementary info: %s\n", suppi );
	 }

      } /* endif (call[0]) */

   } /* endif (reconized major status code) */

   fflush(tf);

   return;

} /* dbg_ret() */


/*
 * dbg_integer()
 *
 *
 *
 */
void
dbg_integer( OM_uint32 p_dbgmask, char * p_parm_name,
	     void * p_baseptr, int size )
{
   char          * this_Call = "dbg_integer";
   char            tmpbuf[256];
   unsigned int    ival;
   unsigned long   lval;

   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   if ( p_baseptr==NULL )
      return;

   if ( size==1 ) {
      ival = ((unsigned char *)p_baseptr)[0];
      sprintf(tmpbuf, "(dec) %u / (hex) 0x%02x",    ival, ival );
   } else if ( size==sizeof(unsigned short) ) {
      ival = ((unsigned short *)p_baseptr)[0];
      sprintf(tmpbuf, "(dec) %u / (hex) 0x%04x",    ival, ival );
   } else if ( size==sizeof(unsigned int) ) {
      ival = ((unsigned int *)p_baseptr)[0];
      sprintf(tmpbuf, "(dec) %u / (hex) 0x%08x",    ival, ival );
   } else if ( size==sizeof(unsigned long) ) {
      lval = ((unsigned long *)p_baseptr)[0];
      sprintf(tmpbuf, "(dec) %lu / (hex) 0x%016lx", lval, lval );
   } else {
      DEBUG_STRANGE((tf, "  S: %s(): unable to display %d byte-sized value!\n",
		     this_Call, size ))
   }

   return;

} /* dbg_integer() */


/*
 * dbg_lifetime()
 *
 *
 */
void
dbg_lifetime( OM_uint32 p_dbgmask, char * p_parm_name, OM_uint32 * pp_lifetime )
{
   char	    * this_Call = "dbg_lifetime";
   char	    * comment   = "";
   char	      tmpbuf[256];
   Ulong      hours, mins, secs;

   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   if ( pp_lifetime==NULL )
      return;

   if ( (*pp_lifetime)==GSS_C_INDEFINITE ) {

      sprintf(tmpbuf, "GSS_C_INDEFINITE");

   } else if ( (*pp_lifetime)==GSS_C_EXPIRED ) {

      sprintf(tmpbuf, "GSS_C_EXPIRED");

   } else {

      hours= (*pp_lifetime)/3600;
      mins = ((*pp_lifetime) - hours*3600)/60;
      secs = ((*pp_lifetime) - hours*3600 - mins*60);

      sprintf(tmpbuf, "%02luh %02lum %02lus", hours, mins, secs );

   }

   DEBUG_VAL((tf, MYPARAM "= 0x%08lx   (%s)\n",
	          p_parm_name, (Ulong) *pp_lifetime, tmpbuf ));

   return;

} /* dbg_lifetime() */



/*
 * dbg_usage()
 *
 *
 */
void
dbg_usage( OM_uint32 p_dbgmask, char * p_parm_name, OM_uint32 * pp_usage )
{
   char	    * this_Call = "dbg_usage";
   char     * label;

   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   if ( pp_usage==NULL )
      return;

   if ( (*pp_usage)==GSS_C_BOTH ) {

      label = "GSS_C_BOTH";

   } else if ( (*pp_usage)==GSS_C_INITIATE ) {

      label = "GSS_C_INITIATE";

   } else if ( (*pp_usage)==GSS_C_ACCEPT ) {

      label = "GSS_C_ACCEPT";

   } else {

      label = " **UNKNOWN** ";

   }

   DEBUG_VAL((tf, MYPARAM "= %lu            (%s)\n",
	          p_parm_name, *pp_usage, label ));

   return;

} /* dbg_usage() */


static char * ctx_flag_names[] = {
      "DELEG",
      "MUTUAL",
      "REPLAY",
      "SEQUENCE",
      "CONF",
      "INTEG",
      "ANON",
      "PROT_READY",
      "TRANS",
      (char *)0
};

/*
 * dbg_flags()
 *
 *
 */
void
dbg_flags( OM_uint32 p_dbgmask, char * p_parm_name, OM_uint32 * pp_flags )
{
   char	      * this_Call = "dbg_flags";
   char         tmpbuf[256];
   OM_uint32    flags;
   Uint         i, cnt;


   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   if ( pp_flags==NULL )
      return;

   flags = *pp_flags;
   tmpbuf[0]= '\0';

   for ( cnt=0 , i=0 ; ctx_flag_names[i]!=0 ; i++ ) {
      if ( (flags&0x01)!=0 ) {
	 if (cnt==0) {
	    strcat(tmpbuf, "(");
	 } else {
	    strcat(tmpbuf, "+");
	 }
	 strcat(tmpbuf, ctx_flag_names[i]);
	 cnt++;
      }
      flags /= 2;
   }

   if ( tmpbuf[0]=='\0' ) {
      strcat(tmpbuf,"(no flags)");
   } else {
      strcat(tmpbuf, ")");
   }

   DEBUG_VAL((tf, MYPARAM "= 0x%04x       %s\n",
	          p_parm_name, (Uint)(*pp_flags), tmpbuf ));

   return;

} /* dbg_flags() */


/*
 * dbg_bool()
 *
 *
 */
void
dbg_bool( OM_uint32 p_dbgmask, char * p_parm_name, void * p_bool_val, size_t p_bool_size )
{
   int bval;

   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   if ( p_bool_val==NULL )
      return;

   if ( p_bool_size == sizeof(int) ) {
      bval = (int) *((int *)p_bool_val);
   } else if ( p_bool_size == sizeof(char) ) {
      bval = (int) *((char *)p_bool_val);
   } else if ( p_bool_size == sizeof(long) ) {
      bval = (int) *((long *)p_bool_val);
   } else if ( p_bool_size == sizeof(short) ) {
      bval = (int) *((short *)p_bool_val);
   } else {
      DEBUG_STRANGE((tf, "ERROR: dbg_bool() with unrecognized size argument (%u)!\n",
		     (Uint) p_bool_size ));
      return;
   }

   DEBUG_VAL((tf, MYPARAM "= %d            (%s)\n", p_parm_name, bval, (bval==FALSE) ? "FALSE" : "TRUE" ));

   return;

} /* dbg_bool() */


/*
 * dbg_print_ptr()
 *
 *
 */
void
dbg_print_ptr( OM_uint32   p_dbgmask,  char * p_param_name,  void  ** pp_ptr )
{
   char    * type;
   char    * clean;

   if ( tf==NULL || dbg_enable==0 || (dbg_info&p_dbgmask)!=p_dbgmask )
      return;

   if ( pp_ptr==NULL ) {
      /* optional return parameter -- suppressing output */
      return;
   }

   if ( (p_dbgmask & INFO_NAME)!=0 ) {
      type  = "(gss_name_t)";
      clean = "GSS_C_NO_NAME";
   } else if ( (p_dbgmask & INFO_CTX)!=0 ) {
      type  = "(gss_ctx_id_t)";
      clean = "GSS_C_NO_CONTEXT";
   } else if ( (p_dbgmask & INFO_CRED)!=0 ) {
      type  = "(gss_cred_id_t)";
      clean = "GSS_C_NO_CREDENTIAL";
   } else {
      /* unknown */
      return;
   }

   if ( *pp_ptr==NULL ) {

      DEBUG_VAL((tf, MYPARAM "= %s ptr:%p       \"%s\"\n", p_param_name, type, *pp_ptr, clean ));

   } else {
      
      DEBUG_VAL((tf, MYPARAM "= %s ptr:%p\n", p_param_name, type, *pp_ptr ));

   }

   return;

} /* dbg_print_ptr() */


#endif /* !NDEBUG */

