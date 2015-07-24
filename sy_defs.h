/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/sy_defs.h#2 $
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


#ifndef _SY_DEFS_H_
#  define _SY_DEFS_H_


#ifndef FALSE
#  define FALSE 0
#endif
#ifndef TRUE
#  define TRUE  1
#endif


#ifndef UNREFERENCED_PARAMETER
#  define UNREFERENCED_PARAMETER(x) ((x)=(x))
#endif

#define ARRAY_ELEMENTS(x)       (sizeof(x)/sizeof(x[0]))


struct sy_timeval_s {
	OM_uint32  sec;
	OM_uint32  usec;
};
typedef struct sy_timeval_s    sy_timeval_desc, *sy_timeval_t;


#  if defined(NEED_WSASTARTUP)
   extern volatile Uint WSA_usecntr;
#  endif

/***********************************************/
/* FUNCTION declarations for syfuncs.c         */
/* Wrappers for OS-specific functions          */
/* that use custom data types or include files */
/***********************************************/

OM_uint32     sy_init( OM_uint32 * pp_min_stat );
void	      sy_cleanup( void );

void	      sy_init_chartable(void);
int           sy_check_valid_chars( void   * p_buffer,   int     len,
				    int      types,      char  * more );

#define CHECK_ALPHA  (1<<0)
#define CHECK_DIGIT  (1<<1)
#define CONV_2_LOWER (1<<8)
#define CONV_2_UPPER (1<<9)

OM_uint32     sy_get_time(  sy_timeval_desc * p_time );
OM_uint32     sy_diff_time( sy_timeval_desc * p_t1,
			    sy_timeval_desc * p_t2, sy_timeval_desc * p_diff );

void          sy_free( void *p_memptr );
void	      sy_clear_free( void **p_memptr, size_t p_clearlen );
void        * sy_malloc( size_t p_mem_size );
void        * sy_calloc( size_t p_mem_size );


OM_uint32     sy_resolve_hostname( OM_uint32 *pp_min_stat,
				   char *p_host,
				   char *p_fqdn, size_t p_maxlen );

OM_uint32     sy_get_hostname( OM_uint32 *pp_min_stat,
			       char *p_fqdn, size_t p_maxlen );

OM_uint32     sy_get_username( OM_uint32 * pp_min_stat,
			       char *p_myname, size_t p_maxlen );

OM_uint32     sy_uid_2_name( OM_uint32 * pp_min_stat,
			     void * p_uid, size_t p_uidlen, int  p_num,
			     char * p_oname, int p_maxolen, size_t * pp_olen );
int           sy_uid_avail(void);

int	      sy_strcasecmp(  const char *s1, const char *s2 );
int	      sy_strncasecmp( const char *s1, const char *s2, size_t count );

size_t	      sy_strnlen( const char *s1, size_t count );
char *	      sy_strmaxcpy( char *dest, const char *src, size_t count );
char *	      sy_strmaxdup( char * src, size_t  max_len );



/* global data from sy_funcs.c */

OM_uint32   sy_lasterror( OM_uint32 * pp_min_stat,
			  OM_uint32 p_error, char *p_buf, size_t p_buf_len );

#if defined(_WIN32) && defined(_WINDOWS_)
LONG        sy_RegQueryValue( HKEY     p_key,    char   * p_subkey_name,
                              char   * p_value,  DWORD    p_regtype,
                              DWORD  * pp_dwret,
                              char   * p_strbuf, size_t   p_strbuf_max );
#endif /* _WIN32 */


#endif /* _SY_DEFS_H_ */
