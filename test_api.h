/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/test_api.h#1 $
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

#include "platform.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN 1
#  include <windows.h>
#endif

#include "gssapi_2.h"

typedef unsigned short Ushort;
typedef unsigned char  Uchar;
typedef unsigned long  Ulong;
typedef unsigned int   Uint;

#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

extern	 FILE * fh;

#define ARRAY_ELEMENTS(x)   (sizeof(x)/sizeof(x[0]))

#define NUM_SAMPLES  16


int       drop_buffer( gss_buffer_desc * buf );
int       drop_name( gss_name_t * name );
int       drop_oid_set( gss_OID_set * oid_set );
int       drop_cred( gss_cred_id_t * cred );
int       drop_context( gss_ctx_id_t  * ctx );

int	  display_name( char * type_of_name , gss_name_t name );

int
create_context( gss_name_t      p_target,         gss_OID          p_mech,
	        OM_uint32	p_recflags,	  OM_uint32	   p_lifetime,
	        gss_cred_id_t   p_ini_cred,       gss_cred_id_t    p_acc_cred,
	        gss_name_t    * pp_source,
		gss_ctx_id_t  * pp_ini_ctx,       gss_ctx_id_t   * pp_acc_ctx,
		gss_OID	      * pp_ini_mech,      gss_OID        * pp_acc_mech,
	        OM_uint32     *	pp_ini_retflags,  OM_uint32	 * pp_acc_retflags,
		OM_uint32     * pp_ini_lifetime,  OM_uint32	 * pp_acc_lifetime );

int  exp_imp_context( Uint count,
		      char * type_of_context, gss_OID mech, gss_ctx_id_t * ctx );

int  check_context_lifetime( char * type_of_context, gss_OID mech, gss_ctx_id_t ctx );

void      print_lifetime(char * prefix, OM_uint32 p_lifetime);

void	  print_status( char       * call_name,
			gss_OID      mech_oid,
			OM_uint32    major_status,
			OM_uint32    minor_status );


int  transfer_signed_message( int  count,
			      char * src_name, gss_ctx_id_t src_ctx,
			      char * dst_name, gss_ctx_id_t dst_ctx );

int  transfer_wrapped_message( int  count,  int do_conf,
			       char * src_name, gss_ctx_id_t src_ctx,
			       char * dst_name, gss_ctx_id_t dst_ctx );


int  signed_message_speed( size_t  msg_size,
			   gss_ctx_id_t ini_ctx, gss_ctx_id_t acc_ctx );


void	      init_timer(  void );
void	      start_timer( void );
unsigned long read_timer(  void );
char *        sprint_timer( unsigned long timer_val );
void	      show_timer_resolution( void );

Ulong	      sample_avg( Ulong * p_sample, size_t p_nelements );







