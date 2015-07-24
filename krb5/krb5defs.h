/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/krb5/krb5defs.h#5 $
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


#ifndef _KRB5DEFS_H_
#  define _KRB5DEFS_H_

#include <windows.h>

#include "BaseTsd.h"
#define SECURITY_WIN32 1
#include "security.h"

#include <time.h>
#include <winnt.h>
#include <winsock.h>

#include <WinError.h>
#include <LMerr.h>

/* include LM.h for NetWkstaUserGetInfo() */
#include <LM.h>
#define MAX_USERNAME_LEN  256

#ifndef SEC_E_WRONG_PRINCIPAL
#  include "issperr.h"
#endif

#include "gn_spi.h"
#include "sy_defs.h"
#include "dbg_defs.h"

#if ISSP_LEVEL == 16 || !defined(MAXULONG_PTR)

#  define IsValidSecurityHandle( x ) \
    ( 0xffFFffFFul!=(x)->dwUpper && 0xffFFffFFul!=(x)->dwLower )

#  define SecInvalidateHandle(x) \
    ( (x)->dwUpper = (x)->dwLower = 0xffFFffFFul )

#else 

#  define IsValidSecurityHandle( x ) \
    ( (ULONG_PTR)((INT_PTR)-1)!=(x)->dwUpper && (ULONG_PTR)((INT_PTR)-1)!=(x)->dwLower )

#endif


#ifndef KERB_WRAP_NO_ENCRYPT
#  define KERB_WRAP_NO_ENCRYPT 0x80000001
#endif

typedef PSecurityFunctionTable  (APIENTRY SECDLL_INIT)(void);
typedef PSecurityFunctionTableW (APIENTRY SECDLL_INIT_W)(void);

typedef BOOLEAN (SEC_ENTRY TRANSLATENAME)(
    LPCSTR lpAccountName,
    EXTENDED_NAME_FORMAT AccountNameFormat,
    EXTENDED_NAME_FORMAT DesiredNameFormat,
    LPSTR lpTranslatedName,
    PULONG nSize
    );

extern TRANSLATENAME * fp_TranslateName;

typedef BOOLEAN
(SEC_ENTRY GETUSERNAMEEX)(
    EXTENDED_NAME_FORMAT  NameFormat,
    LPSTR lpNameBuffer,
    PULONG nSize
);

extern GETUSERNAMEEX * fp_GetUserNameEx;


extern PSecurityFunctionTable   fp_SSPI_krb5;
extern PSecurityFunctionTableW  fp_SSPI_krb5_W;
extern HINSTANCE                krb5_secdll_hInst;
extern SECDLL_INIT            * fp_krb5_secdll_init;
extern SECDLL_INIT_W          * fp_krb5_secdll_init_W;
extern char                   * krb5_secdll_name;
extern char		        alt_domain[];


extern gn_nt_tag_et		krb5_nt_set[];
extern Uint			krb5_nt_count;

extern ULONG                    krb5_capabilities;    /* from QuerySecurityPackageInfo() */
extern ULONG                    krb5_maxtoken;        /* from QuerySecurityPackageInfo() */
extern USHORT                   krb5_version;         /* from QuerySecurityPackageInfo() */
extern SEC_CHAR               * krb5_provider;
extern DWORD		        krb5_platform_id;
extern DWORD		        krb5_platform_minor;
extern DWORD		        krb5_platform_major;
extern BOOLEAN                  krb5_realm_to_upper;  /* Whether to force initiators realm to UpperCase after AcceptSecurityContext() */
extern BOOLEAN			krb5_have_directory;
extern BOOLEAN                  krb5_ctx_noexpire;

OM_uint32  krb5_load_dll( OM_uint32 * pp_min_stat );
OM_uint32  krb5_unload_dll( void );
OM_uint32  krb5_sec_error( SECURITY_STATUS p_sec_status,
			   char   ** pp_err_symbol,   char    ** pp_err_description );


#define KRB5_MAX_NAMELEN     256

#define KRB5_CLASS_GENERIC	    0
#define KRB5_CLASS_ERRNO	    1
#define KRB5_CLASS_LASTERROR	    2
#define KRB5_CLASS_SSPI		    3

#define KRB5_SSPI_ERROR_BASE	    (SEC_E_INSUFFICIENT_MEMORY-1)

#define KRB5_SSPICALL_BITS  5
#define KRB5_SSPICALL_COUNT (1<<KRB5_SSPICALL_BITS)
#define KRB5_SSPICALL_SHIFT (MINOR_CLASS_SHIFT - KRB5_SSPICALL_BITS)
#define KRB5_SSPICALL_MASK  ((KRB5_SSPICALL_COUNT-1ul) << KRB5_SSPICALL_SHIFT)

#define KRB5_SSPI_VALUE_BITS (MINOR_VALUE_BITS - KRB5_SSPICALL_BITS)
#define KRB5_SSPI_VALUE_MASK ((1<<KRB5_SSPI_VALUE_BITS)-1)

typedef enum k5_sspi_call_e {
	K5SSPI_CALL_NONE			    =  0,
	K5SSPI_CALL_InitSecurityInterface	    =  1,
	K5SSPI_CALL_QuerySecurityPackageInfo	    =  2,
	K5SSPI_CALL_FreeContextBuffer		    =  3,
        K5SSPI_CALL_AcquireCredentialsHandleOUT	    =  4,
        K5SSPI_CALL_AcquireCredentialsHandleIN	    =  5,
	K5SSPI_CALL_AcquireCredentialsHandleBOTH    =  6,
	K5SSPI_CALL_FreeCredentialsHandle	    =  7,
	K5SSPI_CALL_QueryCredentialsAttributesNAMES   =  8,
	K5SSPI_CALL_QueryCredentialsAttributesLIFESPAN=  9,
	K5SSPI_CALL_InitializeSecurityContext1	    = 10,
	K5SSPI_CALL_InitializeSecurityContextN	    = 11,
	K5SSPI_CALL_AcceptSecurityContext1	    = 12,
	K5SSPI_CALL_AcceptSecurityContextN	    = 13,
	K5SSPI_CALL_DeleteSecurityContext	    = 14,
	K5SSPI_CALL_QueryContextAttributesNAMES     = 15,
	K5SSPI_CALL_QueryContextAttributesSIZES	    = 16,
	K5SSPI_CALL_QueryContextAttributesNATNAMES  = 17,
	K5SSPI_CALL_QueryContextAttributesLIFESPAN  = 18,
	K5SSPI_CALL_ImpersonateSecurityContext	    = 19,
	K5SSPI_CALL_RevertSecurityContext	    = 20,
	K5SSPI_CALL_ExportSecurityContext	    = 21,
	K5SSPI_CALL_ImportSecurityContext	    = 22,
	K5SSPI_CALL_MakeSignature		    = 23,
	K5SSPI_CALL_VerifySignature		    = 24,
	K5SSPI_CALL_EncryptMessageMIC		    = 25,
	K5SSPI_CALL_EncryptMessage		    = 26,
	K5SSPI_CALL_DecryptMessage		    = 27,
	K5SSPI_CALL_TranslateNameTOSAMCOMPAT	    = 28,
	K5SSPI_CALL_TranslateNameTOPRINCIPAL	    = 29,
	K5SSPI_CALL_GetUserNameExSAMCOMPAT	    = 30,
	K5SSPI_CALL_GetUserNameExPRINCIPAL	    = 31
} k5_sspi_call_et;

#define KRB5_MINOR(min_stat)				 \
	( (krb5_minor_msel<<MINOR_MSEL_SHIFT)		 \
	  + (KRB5_MINOR_ ## min_stat & MINOR_VALUE_MASK)	 \
	  + (KRB5_CLASS_GENERIC<<MINOR_CLASS_SHIFT) )

#define KRB5_MINOR_ERRNO(xerr)				 \
	( (krb5_minor_msel<<MINOR_MSEL_SHIFT)		 \
	   + (xerr & MINOR_VALUE_MASK)			 \
	   + (KRB5_CLASS_ERRNO<<MINOR_CLASS_SHIFT) )

#define KRB5_MINOR_LASTERROR(lasterr)			 \
	( (krb5_minor_msel<<MINOR_MSEL_SHIFT)		 \
          + (lasterr & MINOR_VALUE_MASK)		 \
	  + (KRB5_CLASS_LASTERROR<<MINOR_CLASS_SHIFT) )

#define KRB5_MINOR_WINSOCK(wsa_error)			 \
	( (krb5_minor_msel<<MINOR_MSEL_SHIFT)		 \
	  + (wsa_error & MINOR_VALUE_MASK)		 \
	  + (KRB5_CLASS_LASTERROR<<MINOR_CLASS_SHIFT) )

#define KRB5_MINOR_SSPI(sspi_call,sspi_error)			    \
	( (krb5_minor_msel<<MINOR_MSEL_SHIFT)			    \
	  + ((K5SSPI_CALL_ ## sspi_call <<KRB5_SSPICALL_SHIFT)&KRB5_SSPICALL_MASK) \
	  + ((sspi_error - KRB5_SSPI_ERROR_BASE)&KRB5_SSPI_VALUE_MASK)  \
	  + (KRB5_CLASS_SSPI<<MINOR_CLASS_SHIFT) )

#define KRB5_MINOR_2_SSPI(min_stat)			 \
	( (min_stat&KRB5_SSPI_VALUE_MASK) + KRB5_SSPI_ERROR_BASE )

#define KRB5_MINOR_2_SSPICALL(min_stat)			 \
        ( (min_stat&KRB5_SSPICALL_MASK)>>KRB5_SSPICALL_SHIFT)



#define KRB5_MINOR_WINERROR(sspi_call,lasterr)			 \
        ( (HRESULT_FACILITY(lasterr)==9 ? KRB5_MINOR_SSPI(sspi_call,lasterr)   \
	                                : ( (lasterr>=10000 && lasterr<=11000) ? KRB5_MINOR_WINSOCK(lasterr)   \
	                                                                       : KRB5_MINOR_LASTERROR(lasterr) ) ) )

typedef enum krb5_minor_e {
        KRB5_MINOR_NO_ERROR = 0,
	KRB5_MINOR_DOMAIN_MISSING,        /* Domain name missing */
	KRB5_MINOR_USER_MISSING,          /* User name missing */
	KRB5_MINOR_BAD_CHARS,             /* Bad characters in string/name/hostname */
	KRB5_MINOR_CRED_ACCESS_DENIED,    /* Access to the requested credentials denied */
	KRB5_MINOR_SSPI_WRONG_MECH,       /* SSPI returned wrong mechanism token from AcceptSecurityContext() */
	KRB5_MINOR_SSPI_TALKS_GARBAGE,    /* SSPI returned a garbage token from AcceptSecurityContext() */
	KRB5_MINOR_REALM_MISSING,         /* Realm name missing */
	KRB5_MINOR_BAD_ESCAPE_SEQUENCE,	  /* Bad escape sequence in name */
	KRB5_MINOR_BAD_CHAR_IN_REALM,	  /* Bad character in realm name */
	KRB5_MINOR_SSPI_BAD_CRED_NAME,	  /* SSPI didn't return Kerberos credentials for this name */
	KRB5_MINOR_SSPI_CLIENT_ONLY,	  /* SSPI indicates "CLIENT_ONLY" in the security package capabilities */
	KRB5_MINOR_NO_CHBINDINGS_IN_SSPI, /* The W2K Kerberos SSP doesn't support GSSAPI style channel bindings */
	KRB5_MINOR_W2K3_U2U_OWN,	  /* W2K3 Kerberos emits User2User tokens, need to define Service Principal for account */
	KRB5_MINOR_W2K3_U2U_TARGET,	  /*  -"- for target name */
	KRB5_MINOR_SSPI_BAD_CRED,	  /* Something is wrong about the SSPI credentials, security context establishment does */
					  /* not produce rfc-1964 context token.						*/
	KRB5_MINOR_NO_FQDN_HOSTNAME	  /* no FQDN hostname available */
} krb5_minor_et;


extern gss_OID  krb5_mech_oid;
extern gss_OID  krb5_w2k3_u2u_oid;
extern gss_OID  krb5_nt_oid;
extern gss_OID  krb5_ntlm_nt_oid;

extern gn_nt_tag_et    krb5_nt_tag;
extern gn_nt_tag_et    krb5_ntlm_nt_tag;
extern gn_mech_tag_et  krb5_mech_tag;

extern OM_uint32  krb5_minor_msel;

#define KRB5_CRED_COOKIE    0x13467905u
#define KRB5_CTX_COOKIE     0x13467915u

typedef struct krb5_cred_s {
	OM_uint32	    magic_cookie;
	Uchar		  * name;
	size_t		    name_len;
	gss_cred_usage_t    usage;
        time_t              valid_from;
	time_t		    expires_at;
	CredHandle          sspi_cred;
	TimeStamp	    expiration;
} krb5_cred_desc;


typedef struct krb5_ctx_s {
	OM_uint32	          magic_cookie;
	CtxtHandle	          sspi_ctx;
	Uchar                   * initiator;
	size_t                    initiator_len;
	Uchar		        * acceptor;
	size_t		          acceptor_len;
	TimeStamp	          sspi_expiration;
	ULONG		          sspi_asc_req;  /* requested ctx attributes of acceptor  */
	ULONG		          sspi_asc_ret;  /* returned  ctx attributes of acceptor  */
	ULONG		          sspi_isc_req;  /* requested ctx attributes of initiator */
	ULONG		          sspi_isc_ret;  /* returned  ctx attributes of initiator */
	ULONG		          seq_ini2acc;
	ULONG			  seq_acc2ini;
	SecPkgContext_Sizes	  sizes;
	time_t			  expires_at;
	OM_uint32	          service_rec;   /* translated received flags (gssapi)    */
	int		          flag_established;
	int                       nego_step;
	int		          role;
} krb5_ctx_desc;


typedef struct krb5_exp_ctx_s {
        OM_uint32		  magic_cookie;
	ULONG			  sspi_asc_req;
	ULONG			  sspi_asc_ret;
	ULONG			  sspi_isc_req;
	ULONG			  sspi_isc_ret;
	ULONG			  seq_ini2acc;
	ULONG		          seq_acc2ini;
	SecPkgContext_Sizes	  sizes;
	TimeStamp		  sspi_expiration;
	time_t			  expires_at;
	OM_uint32		  service_rec;
	Ushort			  token_len;
	Ushort			  initiator_len;
	Ushort			  acceptor_len;
	Ushort			  sspi_token_len;
	Uchar			  nego_step;
	Uchar			  role;
} krb5_exp_ctx_desc;


/********************************************************************/
/* Inner GSS-API token format for the GSS-API over KRB5 wrapper:    */
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

#define KRB5_TOKEN_TYPE        0
#define KRB5_TOKEN_DIRECTION   1
#define KRB5_TOKENDATA_START   2   /* start of inner token */

#define KRB5_MSG_OUT(role)   (((role)==KRB5_INITIATOR) ? 1 : 2 )
#define KRB5_MSG_IN(role)    (((role)==KRB5_INITIATOR) ? 2 : 1 )

typedef enum krb5_tokentype_e {
        KRB5_CTX_TOKEN	    = 0x01,
	KRB5_MIC_TOKEN	    = 0x02,
	KRB5_WRAP_TOKEN	    = 0x03,
	KRB5_WRAPCONF_TOKEN = 0x04
} krb5_tokentype_et;

#define KRB5_1ST_TOKEN      1
#define KRB5_2ND_TOKEN      2
#define KRB5_3RD_TOKEN      3

#define KRB5_INITIATOR      GSS_C_INITIATE
#define KRB5_ACCEPTOR       GSS_C_ACCEPT



/*******************************************************/
/* import from generic layer for credential delegation */
/*******************************************************/

OM_uint32 gn_new_cred(   OM_uint32        * pp_min_stat, /* acq_cred.c */
		  	 void		  * not_used,
		  	 gss_cred_usage_t   p_cred_usage,
		  	 gn_mech_tag_et     p_mechtag,
		  	 gn_cred_t        * pp_cred,
		  	 OM_uint32        * pp_lifetime );

OM_uint32 gn_release_cred( OM_uint32	  * pp_min_stat, /* rel_cred.c */
			   gn_cred_t      * pp_cred );



/*******************************************/
/* private (mech-specific layer) functions */
/*******************************************/

#define KRB5_NO_TWEAK   0
#define KRB5_HAVE_TWEAK 1

int krb5_query_registry_tweak(char * p_reg_param, DWORD * pp_rval);


OM_uint32  krb5_check_cred( OM_uint32        *  pp_min_stat, /* krb5cred.c */
			    void            **  pp_cred,
			    char             *  p_call_name );

#define CTX_DELETE    (1<<0)
#define CTX_ESTABLISH (1<<1)
#define CTX_INQUIRE   (1<<2)
#define CTX_MSG_IN    (1<<3)
#define CTX_MSG_OUT   (1<<4)
#define CTX_TIME      (1<<5)
#define CTX_TRANSFER  (1<<6)

OM_uint32  krb5_check_ctx(  OM_uint32        *  pp_min_stat, /* krb5ctx.c */
			    void            **  pp_ctx,
			    int                 p_flags,
			    char             *  this_Call );

SECURITY_STATUS krb5_free_sspi_buffer( char	  * p_buffer_name,   /* krb5ctx.c */
				       void	 ** pp_sspi_buffer );

OM_uint32  krb5_init_nt_oid_tags(      OM_uint32  * pp_min_stat ); /* krb5oids.c */

OM_uint32  krb5_ntlm_get_default_name( OM_uint32  * pp_min_stat,  /* krb5name.c */
				       char       * p_domain,
				       Uint         p_domain_len,
		    		       char       * p_user,
				       Uint         p_user_len );

OM_uint32  krb5_ntlm_compose_name( OM_uint32      *  pp_min_stat,   /* krb5name.c */
				   char           *  p_user,
				   size_t	     p_user_len,
				   char           *  p_domain,
				   size_t	     p_domain_len,
				   unsigned char  ** pp_oname,
				   size_t         *  pp_olen );

OM_uint32  krb5_ntlm_parse_name(  OM_uint32      *  pp_min_stat,  /* krb5name.c */
				  char           *  p_iname,
				  size_t            p_ilen,
				  unsigned char  ** pp_oname,
				  size_t         *  pp_olen );

OM_uint32  krb5_ntlm_split_name( OM_uint32    * pp_min_stat,    /* krb5name.c */
				 Uchar        * p_iname,
				 size_t         p_iname_len,
				 Uchar       ** pp_user,
				 size_t       * pp_user_len,
				 Uchar       ** pp_domain,
				 size_t       * pp_domain_len );

OM_uint32  krb5_get_default_name( OM_uint32   * pp_min_stat, /* krb5name.c */
				  char        * p_realm,
				  Uint          p_realm_len,
			          char        * p_user,
				  Uint	        p_user_len  );

OM_uint32  krb5_parse_name(  OM_uint32    * pp_min_stat,    /* krb5name.c */
			     Uchar        * p_iname,
			     size_t         p_ilen,
			     Uchar       ** pp_orealm,
			     size_t	  * pp_orealm_len,
			     Uchar       ** pp_ouser,
			     size_t       * pp_ouser_len    );

OM_uint32 krb5_compose_name( OM_uint32    * pp_min_stat,    /* krb5name.c */
			     char         * p_realm,
			     char         * p_user,
			     Uchar       ** pp_oname,
			     size_t       * pp_olen	    );

time_t     krb5_timestamp2time( TimeStamp   * p_timestamp   ); /* in  */

void       krb5_time2timestamp( time_t        p_time,          /* in  */
			        TimeStamp   * p_timestamp   ); /* out */


OM_uint32  krb5_cred_expiration( OM_uint32         * pp_min_stat,
				 gn_cred_usage_et    p_usage,
				 time_t            * pp_valid_from,
				 time_t            * pp_expires_at );

OM_uint32  krb5_copy_name( OM_uint32   * pp_min_stat, /* krb5main.c */
			   Uchar       * p_src,
			   size_t        p_src_len,
			   Uchar      ** p_dst,
			   size_t      * p_dst_len );

OM_uint32  krb5_display_minstat( OM_uint32   * pp_min_stat, /* krb5main.c */
				 OM_uint32     minor_status,
				 char       ** pp_msg,
				 size_t	     * pp_msglen,
				 int	     * pp_need_release );

/*****************************************************/
/* public functions; linkage to generic gssapi layer */
/*****************************************************/

OM_uint32  krb5_initialize( OM_uint32 * pp_min_stat );
OM_uint32  krb5_cleanup( void );

OM_uint32  krb5_canonicalize_name( OM_uint32        *  pp_min_stat,
				   Uchar            *  p_iname,
				   size_t              p_ilen,
				   gn_nt_tag_et        p_nt_itag,
				   Uchar            ** pp_oname,
				   size_t           *  pp_olen,
				   gn_nt_tag_et     *  pp_nt_otag );

OM_uint32  krb5_display_name( OM_uint32	         *  pp_min_stat,
			      gn_nt_tag_et	    p_nt_tag,
			      Uchar		 *  p_iname,
			      size_t		    p_ilen,
			      char		 ** pp_oname,
			      size_t		 *  pp_olen );

OM_uint32  krb5_import_expname( OM_uint32	 *  pp_min_stat,
				Uchar		 *  p_iname,
				size_t		    p_ilen,
				Uchar		 ** pp_oname,
				size_t		 *  pp_olen,
				gn_nt_tag_et	 *  pp_nt_tag );

OM_uint32  krb5_inquire_nametypes( OM_uint32 * pp_min_stat,
				   int       * pp_nt_avail );

OM_uint32  krb5_acquire_cred( OM_uint32          * pp_min_stat,     /* krb5cred.c */
			      Uchar              * p_name,
			      size_t               p_name_len,
			      gn_cred_usage_et     p_usage,
			      void              ** pp_cred,
			      time_t		 * pp_expires_at );

OM_uint32  krb5_release_cred( OM_uint32       * pp_min_stat,	   /* krb5cred.c */
			      void           ** pp_cred );

OM_uint32  krb5_inquire_cred( OM_uint32         * pp_min_stat,	   /* krb5cred.c */
			      void              * p_cred,
			      Uchar            ** pp_name,
			      size_t            * pp_name_len,
			      gn_nt_tag_et      * pp_nt_tag,
			      gn_cred_usage_et  * pp_usage,
			      time_t            * pp_expires_at );

OM_uint32  krb5_default_name_from_ini_cred( OM_uint32  * pp_min_stat, /* krb5cred.c */
					    char       * buf,
					    size_t       buf_max );

OM_uint32  krb5_verify_ini_cred( OM_uint32	* pp_min_stat,	  /* krb5cred.c */
			         krb5_cred_desc * p_cred,
				 int	        * pp_cred_ok );

OM_uint32  krb5_verify_acc_cred( OM_uint32	* pp_min_stat,	  /* krb5cred.c */
				 krb5_cred_desc * p_cred,
				 int	        * pp_cred_ok );



OM_uint32  krb5_init_sec_context( OM_uint32	 * pp_min_stat,	   /* krb5ctx.c */
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

OM_uint32  krb5_accept_sec_context( OM_uint32		         * pp_min_stat,   /* in  */
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
				    OM_uint32		         * pp_service_rec,/* out */
				    time_t		         * pp_expires_at, /* out */
				    gn_cred_t			 * pp_deleg_cred );/* out */

OM_uint32 krb5_release_token( OM_uint32   * pp_min_stat,	   /* krb5ctx.c */
			      Uchar      ** pp_token,
			      size_t      * pp_token_len );

OM_uint32  krb5_delete_sec_context( OM_uint32   * pp_min_stat,     /* krb5ctx.c */
				    void       ** pp_ctx );

OM_uint32  krb5_export_sec_context( OM_uint32    * pp_min_stat,    /* krb5ctx.c */
				    void	** pp_ctx,
				    void	** pp_ctx_buffer,
				    size_t	 * pp_ctx_buffer_len );

OM_uint32  krb5_import_sec_context( OM_uint32    * pp_min_stat,    /* krb5ctx.c */
				    Uchar	 * p_ctx_buffer,
				    size_t	   p_ctx_buffer_len,
				    void	** pp_ctx );

OM_uint32  krb5_context_time( OM_uint32   * pp_min_stat,    /* krb5ctx.c */
			      void	  * p_ctx,
			      time_t      * pp_expires_at );

OM_uint32  krb5_context_name( OM_uint32    * pp_min_stat,   /* krb5ctx.c */
			      void         * p_ctx,
			      int	     p_usage,
			      gn_nt_tag_et * pp_nt_tag,
			      void         * pp_cname,
			      size_t	   * pp_cname_len );

ULONG	   krb5_gss_svc_to_ASC_REQ( void		  ); /* krb5ctx.c */
OM_uint32  krb5_ASC_RET_to_gss_svc( ULONG     CtxFlagsReq ); /* krb5ctx.c */
ULONG      krb5_gss_svc_to_ISC_REQ( OM_uint32 p_svc_flags ); /* krb5ctx.c */
OM_uint32  krb5_ISC_RET_to_gss_svc( ULONG     CtxAttr	  ); /* krb5ctx.c */


OM_uint32  krb5_get_mic( OM_uint32   * pp_min_stat,	   /* krb5msg.c */
			 void	     * p_ctx,
			 OM_uint32     p_qop,
			 Uchar	     * p_message,
			 size_t	       p_message_len,
			 Uchar	    ** pp_mic_token,
			 size_t	     * pp_mic_token_len );

OM_uint32  krb5_verify_mic( OM_uint32  * pp_min_stat,      /* krb5msg.c */
			    void       * p_ctx,
			    Uchar      * p_message,
			    size_t       p_message_len,
			    Uchar      * p_mic_token,
			    size_t       p_mic_token_len,
			    OM_uint32  * pp_qop );

OM_uint32  krb5_wrap( OM_uint32   * pp_min_stat,	   /* krb5msg.c */
		      void	  * p_ctx,
		      OM_uint32     p_qop,
		      int	    p_conf_req,
		      Uchar	  * p_message,
		      size_t	    p_message_len,
		      Uchar	 ** pp_wrap_token,
		      size_t	  * pp_wrap_token_len,
		      int	  * pp_conf_state );

OM_uint32  krb5_unwrap( OM_uint32   * pp_min_stat,	   /* krb5msg.c */
			void	    * p_ctx,
			Uchar	    * p_wrap_token,
			size_t	      p_wrap_token_len,
			Uchar	   ** pp_message,
			size_t	    * pp_message_len,
			int	    * pp_conf_state,
			OM_uint32   * pp_qop );

OM_uint32  krb5_wrap_size_limit( OM_uint32   * pp_min_stat, /* krb5msg.c */
			         void	     * p_ctx,
				 int	       p_conf_req,
				 OM_uint32     p_qop,	       
				 OM_uint32     p_req_out_size,
				 OM_uint32   * pp_max_input );


#endif /* _KRB5DEFS_H_ */
