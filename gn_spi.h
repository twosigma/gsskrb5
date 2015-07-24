/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/gn_spi.h#3 $
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


#ifndef _GN_SPI_H_
#  define _GN_SPI_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>

#include <errno.h>

typedef unsigned char   Uchar;
typedef unsigned short  Ushort;
typedef unsigned int    Uint;
typedef unsigned long   Ulong;

#define BUILDING_DLL    1
#define SHORTCUT_EXPORT 1
#include "gssapi_2.h"

#define GN_MAX_MECHS    10u

#define RETURN_MIN_MAJ(min,maj) { (*pp_min_stat) = (min); return(maj); }

#define ERROR_RETURN(min,maj)	{					\
				  (*pp_min_stat) = (min);		\
				   maj_stat = (maj);       goto error;	\
				}

#define STORE_UINT16_MSB(value,ptr)                                     \
      ( ((Uchar *)ptr)[0] = (Uchar)((((Ushort)value)>>8)&0xff),		\
        ((Uchar *)ptr)[1] = (Uchar)(((Ushort)value) & 0xff) )

#define STORE_UINT32_MSB(value,ptr)                                     \
      ( ((Uchar *)ptr)[0] = (Uchar)((((OM_uint32)value)>>24)&0xff),     \
        ((Uchar *)ptr)[1] = (Uchar)((((OM_uint32)value)>>16)&0xff),     \
        ((Uchar *)ptr)[2] = (Uchar)((((OM_uint32)value)>>8)&0xff),      \
        ((Uchar *)ptr)[3] = (Uchar)   (value & 0xff)                 )

#define LOAD_UINT16_MSB(ptr)                                            \
      ( (Ushort)(((Uchar *)ptr)[0]<<8) + ((Uchar *)ptr)[1] )

#define LOAD_UINT32_MSB(ptr)                                            \
      ( (OM_uint32)(((Uchar *)ptr)[0]<<24) + (((Uchar *)ptr)[1]<<16)    \
                 +  (((Uchar *)ptr)[2]<<8)  +  ((Uchar *)ptr)[3]      )



typedef enum gn_nt_tag_e {
        NT_INVALID_TAG = 0,
	NT_HOSTBASED_SERVICE,
	NT_HOSTBASED_SERVICE_X,
	NT_ANONYMOUS,
	NT_EXPORTED_NAME,
	NT_USER_NAME,
	NT_MACHINE_UID_NAME,
	NT_STRING_UID_NAME,
	NT_PRIVATE_1,
	NT_PRIVATE_2,
	NT_PRIVATE_3,
	NT_PRIVATE_4,
	NT_PRIVATE_5,
	NT_PRIVATE_6,
	NT_PRIVATE_7,
	NT_PRIVATE_8,
	NT_PRIVATE_9,
	NT_DEFAULT = 0x1000
} gn_nt_tag_et;

#define NT_FIRST_TAG NT_HOSTBASED_SERVICE


typedef enum gn_mech_tag_e {
        MECH_INVALID_TAG = 0,
        MECH_PRIVATE_1,
	MECH_PRIVATE_2,
	MECH_PRIVATE_3,
	MECH_PRIVATE_4,
	MECH_PRIVATE_5
} gn_mech_tag_et;

#define MECH_FIRST_TAG  MECH_PRIVATE_1


typedef enum gn_cred_usage_e {
	GN_CRED_INITIATE = GSS_C_INITIATE,
	GN_CRED_ACCEPT   = GSS_C_ACCEPT,
	GN_CRED_BOTH     = GSS_C_BOTH
} gn_cred_usage_et;

/* breaking down the minor_status values into classes
 * at two different levels:  generic and mech-specific
 */
#define MINOR_MSEL_BITS    4
#define MINOR_CLASS_BITS   4

/* the rest is automatically derived */
#define MINOR_MSEL_COUNT   (1<<MINOR_MSEL_BITS)
#define MINOR_MSEL_SHIFT   (32-MINOR_MSEL_BITS)
#define MINOR_MSEL_MASK    ((MINOR_MSEL_COUNT-1ul) << MINOR_MSEL_SHIFT)

#define MINOR_CLASS_COUNT  (1<<MINOR_CLASS_BITS)
#define MINOR_CLASS_SHIFT  (MINOR_MSEL_SHIFT - MINOR_CLASS_BITS)
#define MINOR_CLASS_MASK   ((MINOR_CLASS_COUNT-1ul) << MINOR_CLASS_SHIFT)

#define MINOR_VALUE_BITS   (32 - MINOR_MSEL_BITS - MINOR_CLASS_BITS)
#define MINOR_VALUE_MASK   (1<<MINOR_VALUE_BITS)-1


#define MINOR_CLASS_GENERIC	    0
#define MINOR_CLASS_ERRNO	    1
#define MINOR_CLASS_LASTERROR	    2
#define MINOR_CLASS_REGISTERED	    3

#ifdef _WIN32

#  define MINOR_LASTERROR(err)	       \
	 ( ((err)&MINOR_VALUE_MASK)    \
	   + (MINOR_CLASS_LASTERROR<<MINOR_CLASS_SHIFT) )

#  define MINOR_WSAERROR(err)	       \
	 ( ((err)&MINOR_VALUE_MASK)    \
	   + (MINOR_CLASS_LASTERROR<<MINOR_CLASS_SHIFT) )

#  define MINOR_REGISTERED(err)	       \
	 ( ((err)&MINOR_VALUE_MASK)    \
	   + (MINOR_CLASS_REGISTERED<<MINOR_CLASS_SHIFT) )

#endif /* !_WIN32 */


/* all minor_error_values besides GENERIC (0) enumerate the */
/* mechanisms in the order in which they are initialized    */
#define MINOR_MSEL_GENERIC	    0


typedef struct gn_min_stat_s {
   OM_uint32		   value;
   char			 * label;
   char			 * description;
} gn_min_stat_desc;


struct gn_cred_s;
typedef struct gn_cred_s *gn_cred_t;

/* Defined values for Minor Error codes */
typedef enum gn_min_stat_e {
	MINOR_NO_ERROR		= 0,
	MINOR_OUT_OF_MEMORY     = 1,
	MINOR_BAD_CRED_USAGE,
	MINOR_INVALID_BUFFER,
	MINOR_INVALID_OID,
	MINOR_INVALID_OID_SET,
	MINOR_INVALID_CTX,
	MINOR_INVALID_CRED,
	MINOR_INVALID_NAME,
	MINOR_INCOMPLETE_CTX,
	MINOR_AT_MISSING_IN_SVC_NAME,
	MINOR_BAD_HOSTNAME_IN_SVC_NAME,
	MINOR_SERVICE_MISSING,
	MINOR_GETHOSTNAME_FAILED,
	MINOR_HOSTNAME_MISSING,
	MINOR_NAME_TOO_LONG,
	MINOR_HOST_LOOKUP_FAILURE,
	MINOR_HOSTNAME_TOO_LONG,
	MINOR_INVALID_HOSTNAME,
	MINOR_NOT_YET_IMPLEMENTED,
	MINOR_INTERNAL_BUFFER_OVERRUN,

	MINOR_NAME_CONTAINS_NUL,
	MINOR_INTERNAL_ERROR,
	MINOR_YOU_DONT_EXIST,
	MINOR_INVALID_USERNAME,
	MINOR_BAD_UID,
	MINOR_FEATURE_UNAVAILABLE,
	MINOR_NOT_A_TOKEN,
	MINOR_INVALID_LENGTH_FIELD,	/* invalid ASN.1 DER-encoded length */
	MINOR_MAY_BE_TRUNCATED,		/* (exported name) token too short? */
	MINOR_TRAILING_GARBAGE,         /* token has trailing garbage?      */
	MINOR_UNKNOWN_MECH_OID,		/* unknown mechanism oid in token   */
	MINOR_WRONG_LEN_EXPNAME,

	MINOR_NEED_ONE_MECHNAME,        /* compare_name() needs at least one MechName */
	MINOR_MN_DISJUNCT_MECHLIST,     /* mechanism list doesn't intersect */
	                                /* with mechansim of supplied MN    */

	MINOR_WRONG_CONTEXT,     /* init_sec_ctx(): valid but inadequate context */
	MINOR_CRED_CHANGED,	 /* init_sec_ctx(): cred handle changed between calls */
	MINOR_FLAGS_CHANGED,	 /* init_sec_ctx(): service flags changed between calls */
	MINOR_MECH_CHANGED,	 /* init_sec_ctx(): mech_oid changed between calls */
	MINOR_CHANBIND_CHANGED,  /* init_sec_ctx(): channel bindings changed between calls */
	MINOR_WRONG_CREDENTIAL,  /* init_sec_ctx(): cred not suitable for context initiation */
	MINOR_DEAD_CONTEXT,      /* init_sec_ctx(): ctx handle valid, but context unusable, */
				 /*                 app needs to call delete_sec_context()  */
        MINOR_INCOMPLETE_CONTEXT, /* many context-level calls: security context not fully established */
	MINOR_ESTABLISHED_CONTEXT, /* init/accept_sec_ctx(): security context already established */
        MINOR_TARGET_CHANGED,    /* init_sec_ctx(): target name changed between calls */
	MINOR_NO_TARGET,         /* init_sec_ctx(): target name missing */
	MINOR_INPUT_MISSING,     /* init_sec_ctx(): input token missing */
	MINOR_WRONG_TOKEN,	 /* wrong token received for this call */
	MINOR_MISC_ERROR,	 /* platform-specific errno didn't fit into our minor error code scheme */
	MINOR_REFLECTED_MIC,	 /* reflected signature token   */
	MINOR_REFLECTED_WRAP,	 /* reflected protected message */
	MINOR_SIZE_TOO_SMALL,    /* requested output size to small for wrap_size_limit() */
	MINOR_WRONG_MECHANISM,   /* mech oid prefix from token doesn't match context's mechanism */

	MINOR_INVALID_STATUS,	 /* invalid minor_status value given to gss_display_status() */
	MINOR_INIT_FAILED,	 /* initialization error: misc (unexpected) failure */
	MINOR_SHLIB_NOT_FOUND,	 /* initialization error: SHLIB not found / load error */
	MINOR_SHLIB_INVALID,	 /* initialization error: inappropriate SHLIB */

	MINOR_NO_MECHANISM_AVAIL,/* no mechanism available (not found, mech initialization failed) */
	MINOR_NO_TOKEN_EXPECTED, /* gss_init_sec_context() was handed a token with the initial call */
	MINOR_NO_CRED_FOR_MECH,  /* No credentials available for requested mechanism */

	MINOR_SIMULATED_ERROR

/* IMPORTANT: add new error codes to dsp_stat.c:minor_error[] as well */ 


} gn_min_stat_et;

#define MINOR_USE_ERRNO(min_stat, xerrno)		 \
	{						 \
	    OM_uint32  errnum = (xerrno);		 \
	    min_stat = ( (errnum > MINOR_ERRNO_MASK)	 \
			? MINOR_MISC_ERROR : errnum );	 \
	}




#define SPI_FUNC_PREFIX

#define TDEF_GN_SPI(name) typedef OM_uint32 (SPI_FUNC_PREFIX GSP_ ## name)



#define A1_GSP_INITIALIZE	OM_uint32        *	/* pp_min_stat   */

#define A0_GSP_CLEANUP		void			/*               */

#define A7_GSP_CANONICALIZE_NAME OM_uint32	*,	/* pp_min_stat   */ \
				Uchar		*,	/* p_iname       */ \
				size_t		 ,	/* p_ilen        */ \
				gn_nt_tag_et     ,	/* p_nt_itag     */ \
				Uchar		**,	/* pp_oname      */ \
				size_t		*,	/* pp_olen	 */ \
				gn_nt_tag_et	*	/* pp_nt_otag	 */

#define A6_GSP_DISPLAY_NAME	OM_uint32	*,	/* pp_min_stat   */ \
				gn_nt_tag_et     ,	/* p_nt_tag      */ \
				Uchar		*,	/* p_iname	 */ \
				size_t		 ,      /* p_ilen	 */ \
				char	       **,	/* pp_oname	 */ \
				size_t		*	/* pp_olen	 */

#define A6_GSP_IMPORT_EXPNAME	OM_uint32	*,	/* pp_min_stat	 */ \
				Uchar		*,	/* p_iname	 */ \
				size_t		 ,	/* p_ilen	 */ \
				Uchar	       **,	/* pp_oname	 */ \
				size_t		*,	/* pp_olen	 */ \
				gn_nt_tag_et	*	/* pp_nt_tag	 */

#define A2_GSP_INQUIRE_NAMETYPES					    \
				OM_uint32	*,	/* pp_min_stat	 */ \
				int             *	/* pp_nt_avail   */
#if 0
#define A5_GSP_ADD_CRED							    \
				OM_uint32       *,      /* pp_min_stat   */ \
				Uchar           *,      /* cname         */ \
				size_t           ,	/* cname_len     */ \
				gss_cred_usage_t ,      /* cred_usage    */ \
				void           **       /* prv_cred      */
#endif

#define A6_GSP_ACQUIRE_CRED	OM_uint32	 *,	/* minor_status  */ \
				Uchar		 *,	/* name          */ \
				size_t		  ,	/* name_len      */ \
				gn_cred_usage_et  ,	/* usage         */ \
				void		**,	/* priv_cred     */ \
				time_t		 *	/* expires_at    */

#define A2_GSP_RELEASE_CRED	OM_uint32	 *,	/* minor_status  */ \
				void		**	/* priv_cred	 */

#define A7_GSP_INQUIRE_CRED	OM_uint32	 *,	/* minor_status  */ \
				void		 *,     /* priv_cred     */ \
				Uchar           **,     /* name		 */ \
				size_t		 *,     /* name_len	 */ \
				gn_nt_tag_et	 *,	/* nt_tag	 */ \
				gn_cred_usage_et *,	/* usage	 */ \
				time_t		 *	/* expires_at    */



#define A13_GSP_INIT_SEC_CTX    OM_uint32	 *,	/* o:  minor_status  */ \
				void		 *,	/* i:  priv_cred     */ \
				gss_channel_bindings_t ,/* i:  chan_bind     */ \
				Uchar            *,     /* i:  target_name   */ \
				size_t            ,     /* i:  target_name_len */ \
				Uchar            *,     /* i:  in_token      */ \
				size_t            ,     /* i:  in_token_len  */ \
				OM_uint32         ,     /* i:  service_req   */ \
				void            **,     /* o:  priv_ctx      */ \
				Uchar           **,	/* o:  out_token     */ \
				size_t           *,     /* o:  out_token_len */ \
				OM_uint32	 *,     /* o:  service_rec   */ \
				time_t		 *      /* o:  expires_at    */

#define A14_GSP_ACCEPT_SEC_CTX  OM_uint32	 *,     /* o:  minor_status  */ \
				void		 *,	/* i:  priv_cred     */ \
				gss_channel_bindings_t, /* i:  chan_bind     */ \
				Uchar		 *,	/* i:  in_token      */ \
				size_t		  ,	/* i:  in_token_len  */ \
				void		**,	/* o:  priv_ctx      */ \
				Uchar		**,	/* o:  src_name      */ \
				size_t		 *,	/* o:  src_name_len  */ \
				gn_nt_tag_et	 *,	/* o:  src_nt_tag    */ \
				Uchar		**,	/* o:  out_token     */ \
				size_t		 *,	/* o:  out_token_len */ \
				OM_uint32	 *,	/* o:  service_rec   */ \
				time_t		 *,	/* o:  expires_at    */ \
				gn_cred_t	 *	/* o:  priv_deleg_cred */

				

#define A3_GSP_RELEASE_TOKEN	OM_uint32	 *,     /* o:  minor_status  */ \
				Uchar		**,     /* i:  ctx_token     */ \
				size_t		 *      /* i:  ctx_token_len */


#define A2_GSP_DELETE_SEC_CTX	OM_uint32	 *,	/* o:  minor_status   */ \
				void		**	/* io: context handle */

#define A4_GSP_EXPORT_SEC_CTX   OM_uint32	 *,     /* o:  minor_status  */ \
				void		**,	/* io: priv_ctx      */ \
				void		**,	/* o:  prv_ctxbuf    */ \
				size_t		 *	/* o:  prv_ctxbuf_len*/

#define A4_GSP_IMPORT_SEC_CTX	OM_uint32	 *,	/* o:  minor_status  */ \
				void		 *,	/* i:  prv_ctxbuf    */ \
				size_t		  ,	/* i:  prv_ctxbuf_len*/ \
				void		**	/* o:  priv_ctx      */

#define A3_GSP_CONTEXT_TIME     OM_uint32	 *,     /* o:  minor_status  */ \
				void		 *,	/* i:  priv_ctx      */ \
				time_t		 *	/* o:  expires_at    */


#define A6_GSP_CONTEXT_NAME	OM_uint32	 *,     /* o: minor_status   */ \
				void		 *,     /* i: priv_ctx	     */ \
				int		  ,	/* i: usage	     */ \
				gn_nt_tag_et	 *,	/* o: nt_tag	     */ \
				void		**,	/* o: cname	     */ \
				size_t		 *	/* o: cname_len	     */

#define A5_GSP_DISPLAY_MINSTAT	OM_uint32	 *,	/* o: minor_status   */ \
				OM_uint32	  ,	/* i: status_value   */ \
				char		**,     /* o: message_ptr    */ \
				size_t		 *,	/* o: message_len    */ \
				int		 *	/* o: need_release   */

#define A7_GSP_GET_MIC	        OM_uint32	 *,     /* o: minor_status   */ \
				void		 *,     /* i: priv_ctx	     */ \
				OM_uint32	  ,	/* i: mic_qop	     */ \
				Uchar		 *,	/* i: message_buffer */ \
				size_t		  ,	/* i: message_len    */ \
				Uchar		**,     /* o: mic_token      */ \
				size_t		 *	/* o: mic_token_len  */

#define A7_GSP_VERIFY_MIC       OM_uint32        *,     /* o: minor_status   */ \
				void		 *,	/* i: priv_ctx       */ \
				Uchar		 *,	/* i: message_ptr    */ \
				size_t		  ,	/* i: message_len    */ \
				Uchar            *,     /* i: mic_token      */ \
				size_t            ,     /* i: mic_token_len  */ \
				OM_uint32	 *	/* o: mic_qop	     */

#define A9_GSP_WRAP	        OM_uint32	 *,     /* o: minor_status   */ \
				void		 *,     /* i: priv_ctx	     */ \
				OM_uint32	  ,	/* i: wrap_qop	     */ \
				int		  ,	/* i: conf_req	     */ \
				Uchar		 *,	/* i: message_buffer */ \
				size_t		  ,	/* i: message_len    */ \
				Uchar		**,     /* o: wrap_token     */ \
				size_t		 *,	/* o: wrap_token_len */ \
				int		 *	/* o: conf_state     */


#define A7_GSP_UNWRAP	        OM_uint32	 *,     /* o: minor_status   */ \
				void		 *,     /* i: priv_ctx	     */ \
				Uchar		 *,	/* i: wrap_token     */ \
				size_t		  ,	/* i: wrap_token_len */ \
				Uchar		**,     /* o: message	     */ \
				size_t		 *,	/* o: message_len    */ \
				int		 *,	/* o: conf_state     */ \
				OM_uint32	 *	/* o: wrap_qop	     */

#define A6_GSP_WRAP_SIZE_LIMIT  OM_uint32	 *,	/* o: minor_status   */ \
				void		 *,	/* i: priv_ctx	     */ \
				int		  ,	/* i: conf_req	     */	\
				OM_uint32	  ,	/* i: wrap_qop	     */ \
				OM_uint32	  ,	/* i: req_output_size*/ \
				OM_uint32	 *	/* o: max_input_size */


TDEF_GN_SPI(INITIALIZE)			( A1_GSP_INITIALIZE		);
TDEF_GN_SPI(CLEANUP)			( A0_GSP_CLEANUP		);
TDEF_GN_SPI(CANONICALIZE_NAME)		( A7_GSP_CANONICALIZE_NAME	);
TDEF_GN_SPI(DISPLAY_NAME)		( A6_GSP_DISPLAY_NAME		);
TDEF_GN_SPI(IMPORT_EXPNAME)		( A6_GSP_IMPORT_EXPNAME		);
TDEF_GN_SPI(INQUIRE_NAMETYPES)		( A2_GSP_INQUIRE_NAMETYPES	);
TDEF_GN_SPI(ACQUIRE_CRED)		( A6_GSP_ACQUIRE_CRED		);
TDEF_GN_SPI(RELEASE_CRED)		( A2_GSP_RELEASE_CRED		);
TDEF_GN_SPI(INQUIRE_CRED)	        ( A7_GSP_INQUIRE_CRED		);
TDEF_GN_SPI(INIT_SEC_CTX)	        ( A13_GSP_INIT_SEC_CTX		);
TDEF_GN_SPI(ACCEPT_SEC_CTX)	        ( A14_GSP_ACCEPT_SEC_CTX	);
TDEF_GN_SPI(DELETE_SEC_CTX)		( A2_GSP_DELETE_SEC_CTX		);
TDEF_GN_SPI(RELEASE_TOKEN)	        ( A3_GSP_RELEASE_TOKEN          );
TDEF_GN_SPI(EXPORT_SEC_CTX)	        ( A4_GSP_EXPORT_SEC_CTX		);
TDEF_GN_SPI(IMPORT_SEC_CTX)	        ( A4_GSP_IMPORT_SEC_CTX		);
TDEF_GN_SPI(CONTEXT_TIME)	        ( A3_GSP_CONTEXT_TIME		);
TDEF_GN_SPI(CONTEXT_NAME)	        ( A6_GSP_CONTEXT_NAME		);
TDEF_GN_SPI(DISPLAY_MINSTAT)	        ( A5_GSP_DISPLAY_MINSTAT	);
TDEF_GN_SPI(GET_MIC)		        ( A7_GSP_GET_MIC		);
TDEF_GN_SPI(VERIFY_MIC)		        ( A7_GSP_VERIFY_MIC	        );
TDEF_GN_SPI(WRAP)		        ( A9_GSP_WRAP			);
TDEF_GN_SPI(UNWRAP)		        ( A7_GSP_UNWRAP			);
TDEF_GN_SPI(WRAP_SIZE_LIMIT)	        ( A6_GSP_WRAP_SIZE_LIMIT	);


struct gn_mechanism_s {
   gss_OID_desc			* mech_oid;
   char				* mech_name;
   int                            use_raw_tokens;
   GSP_INITIALIZE		* fp_initialize;
   GSP_CLEANUP			* fp_cleanup;
   GSP_CANONICALIZE_NAME	* fp_canonicalize_name;
   GSP_DISPLAY_NAME		* fp_display_name;
   GSP_IMPORT_EXPNAME		* fp_import_expname;
   GSP_INQUIRE_NAMETYPES	* fp_inquire_nametypes;
/*   GSP_ADD_CRED                 * fp_add_cred; */
   GSP_ACQUIRE_CRED		* fp_acquire_cred;
   GSP_RELEASE_CRED		* fp_release_cred;
   GSP_INQUIRE_CRED		* fp_inquire_cred;
   GSP_INIT_SEC_CTX	        * fp_init_sec_context;
   GSP_ACCEPT_SEC_CTX	        * fp_accept_sec_context;
   GSP_DELETE_SEC_CTX		* fp_delete_sec_context;
   GSP_RELEASE_TOKEN		* fp_release_token;
   GSP_EXPORT_SEC_CTX	        * fp_export_sec_context;
   GSP_IMPORT_SEC_CTX		* fp_import_sec_context;
   GSP_CONTEXT_TIME		* fp_context_time;
   GSP_CONTEXT_NAME		* fp_context_name;
   GSP_DISPLAY_MINSTAT		* fp_display_minstat;
   GSP_GET_MIC			* fp_get_mic;
   GSP_VERIFY_MIC	        * fp_verify_mic;
   GSP_WRAP			* fp_wrap;
   GSP_UNWRAP		        * fp_unwrap;
   GSP_WRAP_SIZE_LIMIT		* fp_wrap_size_limit;
};





/* Support functions of the generic layer */

OM_uint32   gn_compare_oid( OM_uint32	* pp_min_stat,
			    gss_OID	  p_oid1,
			    gss_OID	  p_oid2,
			    int		* pp_compare );

OM_uint32   gn_add_nt_tag( OM_uint32		* pp_min_stat,
			   gss_OID		  p_oid,
			   gn_nt_tag_et		* pp_nt_tag );

OM_uint32   gn_add_mech_tag( OM_uint32		* pp_min_stat,
			     gss_OID		  p_oid,
			     gn_mech_tag_et	* pp_mech_tag );


OM_uint32   gn_oid_to_nt_tag( OM_uint32		* pp_min_stat,
			      gss_OID		  p_oid,
			      gn_nt_tag_et	* pp_nt_tag );

gss_OID     gn_nt_tag_to_oid( gn_nt_tag_et	  p_nt_tag );

OM_uint32   gn_oid_to_mech_tag( OM_uint32	* pp_min_stat,
				gss_OID		  p_oid,
				gn_mech_tag_et	* pp_mech_tag );

gss_OID     gn_mech_tag_to_oid( gn_mech_tag_et	  p_mech_tag );


OM_uint32   gn_register_mech( OM_uint32                * pp_min_stat,
			      struct gn_mechanism_s    * p_mech,
			      gn_mech_tag_et           * pp_mech_tag,
			      OM_uint32                * pp_minor_msel );

OM_uint32   gn_register_min_stat( OM_uint32	       * pp_min_stat,
				  gn_min_stat_desc     * p_min_stat_list );

OM_uint32   gn_parse_gss_token( OM_uint32        *  pp_min_stat,
				gss_buffer_desc  *  p_itoken,
				gss_OID_desc	 *  p_token_oid,  /* optional */
				gn_mech_tag_et   *  pp_mech_tag,
				void             ** pp_odata,
				size_t           *  pp_olen );




#endif /* _GN_SPI_H_ */
