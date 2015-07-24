/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/ntlm/ntlmdefs.h#4 $
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


#ifndef _NTLMDEFS_H_
#  define _NTLMDEFS_H_

#include <windows.h>

#include "BaseTsd.h"
#define SECURITY_WIN32 1
#include "security.h"
#include "winnls.h"

#include <time.h>
#include <winnt.h>
#include <winsock.h>

#include <WinError.h>
#include <LMerr.h>
#ifndef SEC_E_WRONG_PRINCIPAL
#  include "issperr.h"
#endif

#include "gn_spi.h"
#include "sy_defs.h"
#include "dbg_defs.h"


#if ISSP_LEVEL == 16  || !defined(MAXULONG_PTR)

#  define IsValidSecurityHandle( x ) \
    ( 0xffFFffFFul!=(x)->dwUpper && 0xffFFffFFul!=(x)->dwLower )

#  define SecInvalidateHandle(x) \
    ( (x)->dwUpper = (x)->dwLower = 0xffFFffFFul )

#else 

#  define IsValidSecurityHandle( x ) \
    ( (ULONG_PTR)((INT_PTR)-1)!=(x)->dwUpper && (ULONG_PTR)((INT_PTR)-1)!=(x)->dwLower )

#endif

typedef PSecurityFunctionTable (APIENTRY SECDLL_INIT)(void);

extern PSecurityFunctionTable   fp_SSPI_ntlm;
extern HINSTANCE                ntlm_secdll_hInst;
extern SECDLL_INIT            * fp_ntlm_secdll_init;
extern char                   * ntlm_secdll_name;
extern char		        alt_domain[];


extern ULONG                    ntlm_capabilities;    /* from QuerySecurityPackageInfo() */
extern ULONG                    ntlm_maxtoken;        /* from QuerySecurityPackageInfo() */
extern USHORT                   ntlm_version;         /* from QuerySecurityPackageInfo() */
extern SEC_CHAR               * ntlm_provider;
extern DWORD		        ntlm_platform_id;
extern DWORD		        ntlm_platform_minor;
extern DWORD			ntlm_platform_major;

extern int			ntlm_emulate_ctx_xfer;


OM_uint32  ntlm_load_dll( OM_uint32 * pp_min_stat );
OM_uint32  ntlm_unload_dll( void );
OM_uint32  ntlm_sec_error( SECURITY_STATUS p_sec_status,
			   char   ** pp_err_symbol,   char    ** pp_err_description );


#define NTLM_MAX_NAMELEN     128

#define NTLM_CLASS_GENERIC	    0
#define NTLM_CLASS_ERRNO	    1
#define NTLM_CLASS_LASTERROR	    2
#define NTLM_CLASS_SSPI		    3

#define NTLM_SSPI_ERROR_BASE	    (SEC_E_INSUFFICIENT_MEMORY-1)

#define NTLM_SSPICALL_BITS  5
#define NTLM_SSPICALL_COUNT (1<<NTLM_SSPICALL_BITS)
#define NTLM_SSPICALL_SHIFT (MINOR_CLASS_SHIFT - NTLM_SSPICALL_BITS)
#define NTLM_SSPICALL_MASK  ((NTLM_SSPICALL_COUNT-1ul) << NTLM_SSPICALL_SHIFT)

#define NTLM_SSPI_VALUE_BITS (MINOR_VALUE_BITS - NTLM_SSPICALL_BITS)
#define NTLM_SSPI_VALUE_MASK ((1<<NTLM_SSPI_VALUE_BITS)-1)

typedef enum sspi_call_e {
	SSPI_CALL_NONE				    =  0,
	SSPI_CALL_InitSecurityInterface		    =  1,
	SSPI_CALL_QuerySecurityPackageInfo	    =  2,
	SSPI_CALL_FreeContextBuffer		    =  3,
        SSPI_CALL_AcquireCredentialsHandleOUT	    =  4,
        SSPI_CALL_AcquireCredentialsHandleIN	    =  5,
	SSPI_CALL_AcquireCredentialsHandleBOTH	    =  6,
	SSPI_CALL_FreeCredentialsHandle		    =  7,
	SSPI_CALL_QueryCredentialsAttributesNAMES   =  8,
	SSPI_CALL_QueryCredentialsAttributesLIFESPAN=  9,
	SSPI_CALL_InitializeSecurityContext1	    = 10,
	SSPI_CALL_InitializeSecurityContextN	    = 11,
	SSPI_CALL_AcceptSecurityContext1	    = 12,
	SSPI_CALL_AcceptSecurityContextN	    = 13,
	SSPI_CALL_DeleteSecurityContext		    = 14,
	SSPI_CALL_QueryContextAttributesNAMES  	    = 15,
	SSPI_CALL_QueryContextAttributesSIZES	    = 16,
	SSPI_CALL_QueryContextAttributesNATNAMES    = 17,
	SSPI_CALL_QueryContextAttributesLIFESPAN    = 18,
	SSPI_CALL_ImpersonateSecurityContext	    = 19,
	SSPI_CALL_RevertSecurityContext		    = 20,
	SSPI_CALL_ExportSecurityContext		    = 21,
	SSPI_CALL_ImportSecurityContext		    = 22,
	SSPI_CALL_MakeSignature			    = 23,
	SSPI_CALL_VerifySignature		    = 24,
	SSPI_CALL_EncryptMessageMIC		    = 25,
	SSPI_CALL_EncryptMessage		    = 26,
	SSPI_CALL_DecryptMessage		    = 27,
	SSPI_CALL_TranslateNameTOSAMCOMPAT	    = 28,
	SSPI_CALL_TranslateNameTOPRINCIPAL	    = 29,
	SSPI_CALL_GetUserNameExSAMCOMPAT	    = 30,
	SSPI_CALL_GetUserNameExPRINCIPAL	    = 31
} sspi_call_et;




#define NTLM_MINOR(min_stat)				 \
	( (ntlm_minor_msel<<MINOR_MSEL_SHIFT)		 \
	  + (NTLM_MINOR_ ## min_stat & MINOR_VALUE_MASK)	 \
	  + (NTLM_CLASS_GENERIC<<MINOR_CLASS_SHIFT) )

#define NTLM_MINOR_ERRNO(xerr)				 \
	( (ntlm_minor_msel<<MINOR_MSEL_SHIFT)		 \
	   + (xerr & MINOR_VALUE_MASK)			 \
	   + (NTLM_CLASS_ERRNO<<MINOR_CLASS_SHIFT) )

#define NTLM_MINOR_LASTERROR(lasterr)			 \
	( (ntlm_minor_msel<<MINOR_MSEL_SHIFT)		 \
          + (lasterr & MINOR_VALUE_MASK)		 \
	  + (NTLM_CLASS_LASTERROR<<MINOR_CLASS_SHIFT) )

#define NTLM_MINOR_WINSOCK(wsa_error)			 \
	( (ntlm_minor_msel<<MINOR_MSEL_SHIFT)		 \
	  + (wsa_error & MINOR_VALUE_MASK)		 \
	  + (NTLM_CLASS_LASTERROR<<MINOR_CLASS_SHIFT) )

#define NTLM_MINOR_SSPI(sspi_call,sspi_error)		 \
	( (ntlm_minor_msel<<MINOR_MSEL_SHIFT)		 \
	  + ((SSPI_CALL_ ## sspi_call <<NTLM_SSPICALL_SHIFT)&NTLM_SSPICALL_MASK)	\
	  + ((sspi_error - NTLM_SSPI_ERROR_BASE)&NTLM_SSPI_VALUE_MASK)  \
	  + (NTLM_CLASS_SSPI<<MINOR_CLASS_SHIFT) )

#define NTLM_MINOR_2_SSPI(min_stat)			 \
	( (min_stat&NTLM_SSPI_VALUE_MASK) + NTLM_SSPI_ERROR_BASE )

#define NTLM_MINOR_2_SSPICALL(min_stat)			 \
        ( (min_stat&NTLM_SSPICALL_MASK)>>NTLM_SSPICALL_SHIFT)


	    
typedef enum ntlm_minor_e {
        NTLM_MINOR_NO_ERROR = 0,
	NTLM_MINOR_DOMAIN_MISSING,       /* Domain name missing			    */
	NTLM_MINOR_USER_MISSING,         /* User name missing			    */
	NTLM_MINOR_BAD_CHARS,            /* Bad characters in string/name/hostname  */
	NTLM_MINOR_INVALID_WRAP_PADDING, /* Invalid padding withing wrap token      */
	NTLM_MINOR_UNEXPECTED_SSPI_CTX   /* Unexpected SSPI context token during gss_import_sec_context() */
} ntlm_minor_et;

extern gss_OID  ntlm_mech_oid;
extern gss_OID  ntlm_nt_oid;

extern gn_nt_tag_et    ntlm_nt_tag;
extern gn_mech_tag_et  ntlm_mech_tag;

extern OM_uint32  ntlm_minor_msel;

#define NTLM_CRED_COOKIE    0x13467902u
#define NTLM_CTX_COOKIE     0x13467912u

typedef struct ntlm_cred_s {
	OM_uint32	    magic_cookie;
	Uchar		  * name;
	size_t		    name_len;
	gss_cred_usage_t    usage;
        time_t              valid_from;
	time_t    	    expires_at;
	CredHandle          sspi_cred;
	TimeStamp	    expiration;
} ntlm_cred_desc;


typedef struct ntlm_ctx_s {
	OM_uint32	    magic_cookie;
	CtxtHandle	    sspi_ctx;
	Uchar             * initiator;
	size_t              initiator_len;
	Uchar		  * acceptor;
	size_t		    acceptor_len;
	TimeStamp	    sspi_expiration;
	ULONG		    sspi_asc_req;  /* requested ctx attributes of acceptor  */
	ULONG		    sspi_asc_ret;  /* returned  ctx attributes of acceptor  */
	ULONG		    sspi_isc_req;  /* requested ctx attributes of initiator */
	ULONG		    sspi_isc_ret;  /* returned  ctx attributes of initiator */
	ULONG		    seq_ini2acc;
	ULONG		    seq_acc2ini;
	SecPkgContext_Sizes sizes;
	OM_uint32	    service_rec;   /* translated received flags (gssapi)    */
	time_t              expires_at;
	int		    flag_established;
	int                 nego_step;
	int		    role;
} ntlm_ctx_desc;


typedef struct ntlm_exp_ctx_s {
        OM_uint32	    magic_cookie;
	ULONG		    sspi_asc_req;
	ULONG		    sspi_asc_ret;
	ULONG		    sspi_isc_req;
	ULONG		    sspi_isc_ret;
	ULONG		    seq_ini2acc;
	ULONG		    seq_acc2ini;
	SecPkgContext_Sizes sizes;
	TimeStamp	    sspi_expiration;
	time_t   	    expires_at;
	OM_uint32	    service_rec;
	Ushort		    token_len;
	Ushort		    initiator_len;
	Ushort		    acceptor_len;
	Ushort		    sspi_token_len;
	Uchar		    nego_step;
	Uchar		    role;
} ntlm_exp_ctx_desc;


/********************************************************************/
/* Inner GSS-API token format for the GSS-API over NTLM wrapper:    */
/* -------------------------------------------------------------    */
/* Tthis mechanism-glue uses a 2-Byte prefix in the inner token     */
/* to distinguish different tokens (context level and message       */
/* protection) as well as the negotiation step of the context       */
/* level tokens.						    */
/*								    */
/* The first Byte identifies the type of the token:                 */
/*       01    context level token				    */
/*       02    MIC token                                            */
/*       03    wrap token                                           */
/*								    */
/* The second Byte identifies the sequence of context level tokens  */
/* or the direction of the message protection tokens                */
/********************************************************************/

#define NTLM_TOKEN_TYPE        0
#define NTLM_TOKEN_DIRECTION   1
#define NTLM_TOKENDATA_START   2   /* start of inner token */

#define NTLM_MSG_OUT(role)   (((role)==NTLM_INITIATOR) ? 1 : 2 )
#define NTLM_MSG_IN(role)    (((role)==NTLM_INITIATOR) ? 2 : 1 )

typedef enum ntlm_tokentype_e {
        NTLM_CTX_TOKEN	    = 0x01,
	NTLM_MIC_TOKEN	    = 0x02,
	NTLM_WRAP_TOKEN	    = 0x03,
	NTLM_WRAPCONF_TOKEN = 0x04
} ntlm_tokentype_et;

#define NTLM_1ST_TOKEN      1
#define NTLM_2ND_TOKEN      2
#define NTLM_3RD_TOKEN      3

#define NTLM_INITIATOR      GSS_C_INITIATE
#define NTLM_ACCEPTOR       GSS_C_ACCEPT



/*******************************************/
/* private (mech-specific layer) functions */
/*******************************************/

OM_uint32  ntlm_check_cred( OM_uint32        *  pp_min_stat, /* ntlmcred.c */
			    void            **  pp_cred,
			    char             *  p_call_name );

#define CTX_DELETE    (1<<0)
#define CTX_ESTABLISH (1<<1)
#define CTX_INQUIRE   (1<<2)
#define CTX_MSG_IN    (1<<3)
#define CTX_MSG_OUT   (1<<4)
#define CTX_TIME      (1<<5)
#define CTX_TRANSFER  (1<<6)

OM_uint32  ntlm_check_ctx(  OM_uint32        *  pp_min_stat, /* ntlmctx.c */
			    void            **  pp_ctx,
			    int                 p_flags,
			    char             *  this_Call );

OM_uint32  ntlm_init_nt_oid_tags( OM_uint32 * pp_min_stat ); /* ntlmoids.c */

OM_uint32  ntlm_get_default_name( OM_uint32  * pp_min_stat,  /* ntlmname.c */
		                  char       * p_domain,
			          Uint         p_domain_len,
		    		  char       * p_user,
			          Uint         p_user_len );

OM_uint32  ntlm_default_initiator( OM_uint32	  *  pp_min_stat, /* ntlmname.c */
				   unsigned char  ** pp_oname,
				   size_t	  *  pp_olen );

OM_uint32  ntlm_default_acceptor( OM_uint32	  *  pp_min_stat, /* ntlmname.c */
				  unsigned char   ** pp_oname,
				  size_t	  *  pp_olen );

OM_uint32  ntlm_compose_name( OM_uint32      *  pp_min_stat,   /* ntlmname.c */
			      char           *  p_domain,
			      char           *  p_user,
			      unsigned char  ** pp_oname,
			      size_t         *  pp_olen );

OM_uint32  ntlm_parse_name( OM_uint32      *  pp_min_stat,  /* ntlmname.c */
			     char           *  p_iname,
			     size_t            p_ilen,
			     unsigned char  ** pp_oname,
			     size_t         *  pp_olen );

OM_uint32  ntlm_split_cname( OM_uint32    * pp_min_stat,    /* ntlmname.c */
			     Uchar        * p_iname,
			     size_t         p_iname_len,
			     Uchar       ** pp_user,
			     size_t       * pp_user_len,
			     Uchar       ** pp_domain,
			     size_t       * pp_domain_len );


time_t     ntlm_timestamp2time( TimeStamp   * p_timestamp ); /* in  */

void       ntlm_time2timestamp( time_t        p_time,        /* in  */
			        TimeStamp   * p_timestamp ); /* out */


OM_uint32  ntlm_cred_expiration( OM_uint32         * pp_min_stat,
				 gn_cred_usage_et    p_usage,
				 time_t            * pp_valid_from,
				 time_t            * pp_expires_at );

OM_uint32  ntlm_copy_name( OM_uint32   * pp_min_stat, /* ntlmmain.c */
			   Uchar       * p_src,
			   size_t        p_src_len,
			   Uchar      ** p_dst,
			   size_t      * p_dst_len );

OM_uint32  ntlm_display_minstat( OM_uint32   * pp_min_stat, /* ntlmmain.c */
				 OM_uint32     minor_status,
				 char       ** pp_msg,
				 size_t	     * pp_msglen,
				 int	     * pp_need_release );

/*****************************************************/
/* public functions; linkage to generic gssapi layer */
/*****************************************************/

OM_uint32  ntlm_initialize( OM_uint32 * pp_min_stat );
OM_uint32  ntlm_cleanup( void );

OM_uint32  ntlm_canonicalize_name( OM_uint32        *  pp_min_stat,
				   Uchar            *  p_iname,
				   size_t              p_ilen,
				   gn_nt_tag_et        p_nt_itag,
				   Uchar            ** pp_oname,
				   size_t           *  pp_olen,
				   gn_nt_tag_et     *  pp_nt_otag );

OM_uint32  ntlm_display_name( OM_uint32	         *  pp_min_stat,
			      gn_nt_tag_et	    p_nt_tag,
			      Uchar		 *  p_iname,
			      size_t		    p_ilen,
			      char		 ** pp_oname,
			      size_t		 *  pp_olen );

OM_uint32  ntlm_import_expname( OM_uint32	 *  pp_min_stat,
				Uchar		 *  p_iname,
				size_t		    p_ilen,
				Uchar		 ** pp_oname,
				size_t		 *  pp_olen,
				gn_nt_tag_et	 *  pp_nt_tag );

OM_uint32  ntlm_inquire_nametypes( OM_uint32 * pp_min_stat,
				   int       * pp_nt_avail );

OM_uint32  ntlm_acquire_cred( OM_uint32          * pp_min_stat,     /* ntlmcred.c */
			      Uchar              * p_name,
			      size_t               p_name_len,
			      gn_cred_usage_et     p_usage,
			      void              ** pp_cred,
			      time_t		 * pp_expires_at );

OM_uint32  ntlm_release_cred( OM_uint32       * pp_min_stat,	   /* ntlmcred.c */
			      void           ** pp_cred );

OM_uint32  ntlm_inquire_cred( OM_uint32         * pp_min_stat,	   /* ntlmcred.c */
			      void              * p_cred,
			      Uchar            ** pp_name,
			      size_t            * pp_name_len,
			      gn_nt_tag_et      * pp_nt_tag,
			      gn_cred_usage_et  * pp_usage,
			      time_t            * pp_expires_at );

OM_uint32  ntlm_init_sec_context( OM_uint32	 * pp_min_stat,	   /* ntlmctx.c */
				  void		 * p_cred,
				  gss_channel_bindings_t p_ch_bind,
				  Uchar          * p_target,
				  size_t           p_target_len,
				  Uchar          * p_in_token,
				  size_t           p_in_token_len,
				  OM_uint32        p_serivce_req,
				  void          ** pp_ctx,
				  Uchar         ** p_out_token,
				  size_t         * p_out_token_len,
				  OM_uint32	 * service_rec,
				  time_t	 * expires_at );

OM_uint32  ntlm_accept_sec_context( OM_uint32		         * pp_min_stat,   /* in  */
				    void			 * p_cred,	  /* in  */
				    gss_channel_bindings_t	   p_chanbind,	  /* in  */
				    Uchar			 * p_in_token,    /* in  */
				    size_t			   p_in_token_len,/* in  */
				    void			** pp_ctx,	  /* out */
				    Uchar			** pp_srcname,	  /* out */
				    size_t			 * pp_srcname_len,/* out */
				    gn_nt_tag_et		 * pp_nt_tag,	  /* out */
				    Uchar			** pp_out_token,  /* out */
				    size_t			 * pp_out_token_len,/* out */
				    OM_uint32		         * pp_service_rec, /* out */
				    time_t		         * pp_expires_at,  /* out */
				    gn_cred_t			 * pp_deleg_cred );/* out */

OM_uint32 ntlm_release_token( OM_uint32   * pp_min_stat,	   /* ntlmctx.c */
			      Uchar      ** pp_token,
			      size_t      * pp_token_len );

OM_uint32  ntlm_delete_sec_context( OM_uint32   * pp_min_stat,     /* ntlmctx.c */
				    void       ** pp_ctx );

OM_uint32  ntlm_export_sec_context( OM_uint32    * pp_min_stat,    /* ntlmctx.c */
				    void	** pp_ctx,
				    void	** pp_ctx_buffer,
				    size_t	 * pp_ctx_buffer_len );

OM_uint32  ntlm_import_sec_context( OM_uint32    * pp_min_stat,    /* ntlmctx.c */
				    Uchar	 * p_ctx_buffer,
				    size_t	   p_ctx_buffer_len,
				    void	** pp_ctx );

OM_uint32  ntlm_context_time( OM_uint32   * pp_min_stat,    /* ntlmctx.c */
			      void	  * p_ctx,
			      time_t      * pp_expires_at );

OM_uint32  ntlm_context_name( OM_uint32    * pp_min_stat,   /* ntlmctx.c */
			      void         * p_ctx,
			      int	     p_usage,
			      gn_nt_tag_et * pp_nt_tag,
			      void         * pp_cname,
			      size_t	   * pp_cname_len );

OM_uint32  ntlm_get_mic( OM_uint32   * pp_min_stat,	   /* ntlmmsg.c */
			 void	     * p_ctx,
			 OM_uint32     p_qop,
			 Uchar	     * p_message,
			 size_t	       p_message_len,
			 Uchar	    ** pp_mic_token,
			 size_t	     * pp_mic_token_len );

OM_uint32  ntlm_verify_mic( OM_uint32  * pp_min_stat,      /* ntlmmsg.c */
			    void       * p_ctx,
			    Uchar      * p_message,
			    size_t       p_message_len,
			    Uchar      * p_mic_token,
			    size_t       p_mic_token_len,
			    OM_uint32  * pp_qop );

OM_uint32  ntlm_wrap( OM_uint32   * pp_min_stat,	   /* ntlmmsg.c */
		      void	  * p_ctx,
		      OM_uint32     p_qop,
		      int	    p_conf_req,
		      Uchar	  * p_message,
		      size_t	    p_message_len,
		      Uchar	 ** pp_wrap_token,
		      size_t	  * pp_wrap_token_len,
		      int	  * pp_conf_state );

OM_uint32  ntlm_unwrap( OM_uint32   * pp_min_stat,	   /* ntlmmsg.c */
			void	    * p_ctx,
			Uchar	    * p_wrap_token,
			size_t	      p_wrap_token_len,
			Uchar	   ** pp_message,
			size_t	    * pp_message_len,
			int	    * pp_conf_state,
			OM_uint32   * pp_qop );

OM_uint32  ntlm_wrap_size_limit( OM_uint32   * pp_min_stat, /* ntlmmsg.c */
			         void	     * p_ctx,
				 int	       p_conf_req,
				 OM_uint32     p_qop,	       
				 OM_uint32     p_req_out_size,
				 OM_uint32   * pp_max_input );


#endif /* _NTLMDEFS_H_ */
