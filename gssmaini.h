/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/gssmaini.h#3 $
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


#ifndef _GN_DEFS_H_
#  define _GN_DEFS_H_

#include "gn_spi.h"
#include "sy_defs.h"
#include "dbg_defs.h"
#include "gss_argm.h"

#define GN_MAX_NAMELEN		256u	     /* characters                     */
#define GN_MAX_NAMETYPE_OIDS	 20u	     /* oids for the tag table (min=6) */
#define GN_MAX_MECH_OIDS	 GN_MAX_MECHS/* oids for the tag table (min=1) */

extern struct gn_mechanism_s * gn_mech[];
extern gn_min_stat_desc	     * registered_min_stat[GN_MAX_MECHS];
extern OM_uint32	       gn_min_stat_counter;

/* INITIALIZATION for calls that need more than just the generic layer */
#define INIT_GSSAPI()							\
	if ( gn_gss_initialized==FALSE ) {				\
		major_status = gn_init( minor_status );			\
		if (major_status!=GSS_S_COMPLETE)			\
			return(major_status);				\
	}

/* INITILIZATION for calls that may be completely handled in the generic layer */
#define TRY_INIT_GSSAPI()						\
	if ( gn_gss_initialized==FALSE ) {				\
		major_status = gn_init( minor_status );			\
	}

      
#define CLEAR_OUT_BUFFER( buffer )			\
	if ( buffer!=GSS_C_NO_BUFFER ) {		\
		buffer->length = 0;			\
		buffer->value  = NULL;			\
	}

#define CLEAR_OUT_OID_SET( poid_set )			\
	if ( poid_set!=(gss_OID_set *)0 ) {		\
		(*poid_set) = GSS_C_NO_OID_SET;		\
	}

#define CLEAR_OUT_OID( poid )				\
	if ( poid!=(gss_OID *)0 ) {			\
		(*poid) = GSS_C_NO_OID;			\
	}

#define CLEAR_OUT_NAME( pname )				\
	if ( pname!=(gss_name_t *)0 ) {			\
		(*pname) = GSS_C_NO_NAME;		\
	}

#define CLEAR_OUT_CRED( pcred )				\
	if ( pcred!=(gss_cred_id_t *)0 ) {		\
		(*pcred) = GSS_C_NO_CREDENTIAL;		\
	}

#define CLEAR_OUT_CTX( pcontext )			\
	if ( pcontext!=(gss_ctx_id_t *)0 ) {		\
		(*pcontext) = GSS_C_NO_CONTEXT;		\
	}


struct gn_print_number_s {
	char digits[32];
        char errnum[32];
};

typedef struct gn_print_number_s PRINT_NUMBER;

#define	COOKIE_CREDENTIAL 0xA55A3331ul
#define	COOKIE_NAME       0xA55A3333ul
#define	COOKIE_CONTEXT    0xA55A3335ul


typedef struct gn_name_s {
	OM_uint32		magic_cookie;
	struct gn_name_s  FAR * first;
	struct gn_name_s  FAR * next;
	Uchar		  FAR *	name;
        char              FAR * prname;
	size_t                  name_len;
        size_t                  prname_len;
        gn_nt_tag_et            nt_tag;
	gn_mech_tag_et          mech_tag;
} gn_name_desc, *gn_name_t;

typedef struct gn_cred_s {
	OM_uint32		magic_cookie;
        struct gn_cred_s  FAR * first;
	struct gn_cred_s  FAR * next;
	gn_name_desc      FAR * subject;        /* cached info           */
        void		  FAR * prv_cred;       /* mech data hook        */
	gn_mech_tag_et          mech_tag;       /* managed               */
	gn_cred_usage_et        usage;          /* managed/cached info   */
        time_t		        expires_at;     /* temp info (credentials may be refreshed) */
} gn_cred_desc, *gn_cred_t;


typedef enum gn_ctx_state_e {
	CTX_STATE_INVALID=0,
	CTX_STATE_INITIATING,
        CTX_STATE_ACCEPTING,
	CTX_STATE_ESTABLISHED,
	CTX_STATE_EXPIRED,
	CTX_STATE_ERROR
} gn_ctx_state_et;

typedef enum gn_ctx_role_e {
	CTX_ROLE_INVALID=0,
	CTX_ROLE_INITIATOR,
	CTX_ROLE_ACCEPTOR
} gn_ctx_role_et;

typedef struct gn_context_s {
	OM_uint32		magic_cookie;
	gn_cred_desc      FAR * implicit_cred; /* dynamic object */
	gn_cred_desc	  FAR * caller_cred;   /* READONLY reference, DO NOT RELEASE ! */
	gn_name_desc	  FAR * caller_target; /* READONLY reference, DO NOT RELEASE ! */
	gn_cred_desc	  FAR * cred_element;  /* READONLY reference, DO NOT RELEASE ! */
        gn_name_desc      FAR * initiator;     /* dynamic object */
	gn_name_desc      FAR * acceptor;      /* dynamic object */
	void              FAR * prv_ctx;       /* dynamic object */
	time_t			expires_at;
	gss_channel_bindings_t  ch_bind;       /* READONLY reference, DO NO RELEASE ! */
	gn_ctx_role_et		role;
	gn_ctx_state_et         state;
	gn_mech_tag_et          mech_tag;
	OM_uint32               service_req;
	OM_uint32               service_rec;
} gn_context_desc, *gn_context_t;


/* header of an interprocess token                 */
/* for export_sec_context() / import_sec_context() */
typedef struct gn_exp_ctx_s {
	OM_uint32      magic_cookie;
	OM_uint32      service_rec;
	OM_uint32      service_req;
	Ushort	       token_len;
	Ushort         generic_len;
	Ushort	       private_len;
	Ushort	       mech_oid_len;
	Uchar          role;
} gn_exp_ctx_desc, *gn_exp_ctx_t;


#if 0
struct gn_token_id_s {
	unsigned char id[2];
};
typedef struct gn_token_id_s    gn_token_id_desc;
typedef struct gn_token_id_s  * gn_token_id_t;
#endif


/* GSS-API Token:
 *        0x60   1    Byte       Tag for Token
 *               1-n  Byte       encoded token length
 *        0x06   1    Byte       Tag for OID
 *               1-n  Byte       encoded OID length
 *        
 */
	

OM_uint32   gn_mechanism_init( OM_uint32 * pp_min_stat );	


/* Function declarations for gssmaini.c		*/
OM_uint32   gn_init( OM_uint32 * pp_min_stat );
void        gn_cleanup( void );

OM_uint32   gn_init_tag_tables( OM_uint32 * pp_min_stat );

OM_uint32   gn_remaining_time( time_t expires_at );


extern gss_OID_set_desc     gn_avail_mech_oids;
extern Uint                 mech_last_tag;

/* Function declarations from oid_funcs.c */
OM_uint32   gn_create_nt_oid_set( OM_uint32       * pp_min_stat,
				  gn_nt_tag_et    * pp_nt_oids,
				  size_t	    p_num_oids,
				  gss_OID_set     * pp_oid_set );

OM_uint32   gn_create_mech_oid_set( OM_uint32      * pp_min_stat,
				    gn_mech_tag_et * pp_mech_oids,
				    size_t           p_num_oids,
				    gss_OID_set    * pp_oid_set );

OM_uint32   gn_copy_oid_set( OM_uint32 * pp_min_stat,
			     gss_OID_set p_src_oid_set,
			     gss_OID_set * pp_dst_oid_set );


/* Function declarations from          gn_token.c */
OM_uint32   gn_compose_gss_token( OM_uint32        * pp_min_stat,
				  Uchar            * p_itoken,
				  size_t             p_itoken_len,
				  gn_mech_tag_et     p_mech_tag,
				  gss_buffer_desc  * p_obuffer );

OM_uint32   gn_predict_gss_token_len( OM_uint32        * pp_min_stat,
				      size_t             p_itoken_len,
				      gn_mech_tag_et     p_mech_tag,
				      size_t           * pp_out_len );

			  size_t      gn_encode_length( size_t p_length, unsigned char *p_buffer );

OM_uint32   gn_decode_length( OM_uint32     * pp_min_stat,
			      unsigned char * p_buf,
			      size_t          p_buflen,
			      size_t        * pp_nbytes,
			      size_t        * pp_value );

OM_uint32   gn_parse_gss_token( OM_uint32        *  pp_min_stat,
				gss_buffer_desc  *  p_itoken,
				gss_OID_desc     *  p_token_oid,  /* optional */
				gn_mech_tag_et   *  pp_mech_tag,
				void             ** pp_odata,
				size_t           *  pp_olen );

OM_uint32   gn_create_expname_token( OM_uint32          * pp_min_stat,
				     void               * p_name,
				     size_t               p_len,
				     gn_mech_tag_et       p_mech_tag,
				     gss_buffer_desc    * p_obuffer );

OM_uint32   gn_parse_expname_token( OM_uint32         *  pp_min_stat,
				    gss_buffer_desc   *  p_ibuffer,
				    gn_mech_tag_et    *  pp_mech_tag,
				    void              ** pp_oname,
				    size_t            *  pp_olen );


/* Function declarations from other modules */
#define MAJ_STAT_CALLING      1
#define MAJ_STAT_ROUTINE      2
#define MAJ_STAT_INFORMATORY  3

int         gn_maj_stat_text( OM_uint32   maj_stat,     /* dsp_stat.c */
			      int         errtype,
			      int       * pp_counter,
			      char      * buffer,
			      char     ** pp_label,
			      char     ** pp_desc );

int	    gn_major_text( OM_uint32 p_major,		/* dsp_stat.c */
			   PRINT_NUMBER * p_text_num,
			   char ** p_text_routine,
			   char ** p_text_call,
			   char ** p_text_suppi,
			   char ** p_text_desc );


/* Function declarations from other modules */ 

OM_uint32 gn_alloc_buffer( OM_uint32    * pp_min_stat,   /* rel_buff.c */
			   gss_buffer_t   p_buffer,
			   void         * p_source,
			   size_t         p_source_len,
			   int            p_add_nul );

OM_uint32 gn_duplicate_name( OM_uint32     * pp_min_stat,
			     gn_name_desc  * p_src,
			     gn_name_desc ** pp_dest );

OM_uint32 gn_release_name( OM_uint32	   * pp_min_stat,
			   gn_name_desc   ** pp_name );


OM_uint32 gn_check_name( OM_uint32      * pp_min_stat,   /* imp_name.c */
			 gss_name_t   * p_name,
			 char           * p_call_name );

OM_uint32 gn_canonicalize_name(	OM_uint32	* pp_min_stat,  /* can_name.c */
				gn_name_desc	* p_in_name,
				gn_mech_tag_et    p_mech_tag,
				gn_name_desc   ** pp_out_name );

OM_uint32 gn_create_name( OM_uint32       * pp_min_stat,  /* can_name.c */
			  gn_nt_tag_et      nt_tag,
			  gn_mech_tag_et    mech_tag,
			  Uchar           * name,
			  size_t            name_len,
			  Uchar           * prname,
			  size_t            prname_len,
			  gn_name_desc   ** pp_name );


OM_uint32 gn_check_cred( OM_uint32	  * pp_min_stat, /* acq_cred.c */
			 gss_cred_id_t	  * pp_cred,
			 char		  * this_Call );

OM_uint32 gn_new_cred(   OM_uint32        * pp_min_stat, /* acq_cred.c */
		  	 gn_name_desc	  * p_name,
		  	 gss_cred_usage_t   p_cred_usage,
		  	 gn_mech_tag_et     p_mechtag,
		  	 gn_cred_desc    ** pp_cred,
		  	 OM_uint32        * pp_lifetime );

OM_uint32 gn_chain_cred( OM_uint32        * pp_min_stat, /* acq_cred.c */
			 gn_cred_desc    ** pp_top_cred,
			 gn_cred_desc    ** pp_cred,
			 int		    p_chain_last );

OM_uint32 gn_release_cred( OM_uint32	  * pp_min_stat, /* rel_cred.c */
			   gn_cred_desc  ** pp_cred );


#define CTX_DELETE    (1<<0)
#define CTX_ESTABLISH (1<<1)
#define CTX_INQUIRE   (1<<2)
#define CTX_MSG_IN    (1<<3)
#define CTX_MSG_OUT   (1<<4)
#define CTX_TIME      (1<<5)
#define CTX_TRANSFER  (1<<6)

OM_uint32 gn_check_context( OM_uint32     * pp_min_stat,  /* init_ctx.c */
			    gss_ctx_id_t  * pp_ctx,
			    int             p_flags,
			    char          * this_Call );

OM_uint32  gn_proc_ctx_input( OM_uint32		     * pp_min_stat,  /* init_ctx.c */
			      gss_ctx_id_t	     * pp_io_context,
			      gn_ctx_role_et	       p_role,
			      gss_channel_bindings_t   p_in_chanbind,
			      gss_cred_id_t            p_in_cred,
			      char		     * this_Call,
			      gss_buffer_t	       p_in_token,
			      gn_mech_tag_et	     * pp_mech,
			      Uchar	            ** pp_ctxtoken,
			      size_t		     * pp_ctxtoken_len );

OM_uint32  gn_proc_ctx_cred( OM_uint32	       *  pp_min_stat, /* init_ctx.c */
			     gn_context_desc   *  p_ctx,
			     gss_cred_id_t        p_in_cred,
			     gn_mech_tag_et       p_mech,
			     gss_cred_usage_t     p_usage,
			     char	       *  this_Call,
			     gn_cred_desc      ** pp_cred );

OM_uint32  gn_context_time( OM_uint32         * pp_min_stat,   /* ctx_time.c */
			    gn_context_desc   * p_ctx,
			    OM_uint32	      * pp_lifetime );


/* global data from gssmain.c */
extern gss_OID_desc	gn_gss_oids[];
extern int		gn_gss_oids_num;
extern int		gn_gss_initialized;

OM_uint32
gn_gss_acquire_cred( ARG8_ACQUIRE_CRED );		/* acq_cred.c */
OM_uint32
gn_gss_release_cred( ARG2_RELEASE_CRED );		/* rel_cred.c */
OM_uint32
gn_gss_init_sec_context( ARG13_INIT_SEC_CONTEXT );	/* init_ctx.c */
OM_uint32
gn_gss_accept_sec_context( ARG11_ACCEPT_SEC_CONTEXT );	/* acpt_ctx.c */
OM_uint32
gn_gss_process_context_token( ARG3_PROCESS_CONTEXT_TOKEN ); /* proc_tok.c */
OM_uint32
gn_gss_delete_sec_context( ARG3_DELETE_SEC_CONTEXT );	/* del_ctx.c */
OM_uint32
gn_gss_context_time( ARG3_CONTEXT_TIME );		/* ctx_time.c */
OM_uint32
gn_gss_get_mic( ARG5_GET_MIC );				/* get_mic.c */
OM_uint32
gn_gss_verify_mic( ARG5_VERIFY_MIC );			/* vrfy_mic.c */
OM_uint32
gn_gss_wrap( ARG7_WRAP );				/* wrap.c */
OM_uint32
gn_gss_unwrap( ARG6_UNWRAP );				/* unwrap.c */
OM_uint32
gn_gss_display_status( ARG6_DISPLAY_STATUS );		/* dsp_stat.c */
OM_uint32
gn_gss_indicate_mechs( ARG2_INDICATE_MECHS );		/* ind_mech.c */
OM_uint32
gn_gss_compare_name( ARG4_COMPARE_NAME );		/* cmp_name.c */
OM_uint32
gn_gss_display_name( ARG4_DISPLAY_NAME );		/* dsp_name.c */
OM_uint32
gn_gss_import_name( ARG4_IMPORT_NAME );			/* imp_name.c */
OM_uint32
gn_gss_release_name( ARG2_RELEASE_NAME );		/* rel_name.c */
OM_uint32
gn_gss_release_buffer( ARG2_RELEASE_BUFFER );		/* rel_buff.c */
OM_uint32
gn_gss_release_oid_set( ARG2_RELEASE_OID_SET );		/* rel_oids.c */
OM_uint32
gn_gss_inquire_cred( ARG6_INQUIRE_CRED );		/* inq_cred.c */
OM_uint32
gn_gss_add_cred( ARG11_ADD_CRED );			/* add_cred.c */
OM_uint32
gn_gss_inquire_cred_by_mech( ARG7_INQUIRE_CRED_BY_MECH ); /* in_c_b_m.c */
OM_uint32
gn_gss_inquire_context( ARG9_INQUIRE_CONTEXT );		/* inq_ctx.c */
OM_uint32
gn_gss_wrap_size_limit( ARG6_WRAP_SIZE_LIMIT );		/* ws_limit.c */
OM_uint32
gn_gss_export_sec_context( ARG3_EXPORT_SEC_CONTEXT );	/* exp_ctx.c */
OM_uint32
gn_gss_import_sec_context( ARG3_IMPORT_SEC_CONTEXT );	/* imp_ctx.c */
OM_uint32
gn_gss_release_oid( ARG2_RELEASE_OID );			/* oid_func.c */
OM_uint32
gn_gss_create_empty_oid_set( ARG2_CREATE_EMPTY_OID_SET ); /* oid_func.c */
OM_uint32
gn_gss_add_oid_set_member( ARG3_ADD_OID_SET_MEMBER );	/* oid_func.c */
OM_uint32
gn_gss_test_oid_set_member( ARG4_TEST_OID_SET_MEMBER );	/* oid_func.c */
OM_uint32
gn_gss_oid_to_str( ARG3_OID_TO_STR );			/* oid_func.c */
OM_uint32
gn_gss_str_to_oid( ARG3_STR_TO_OID );			/* oid_func.c */
OM_uint32
gn_gss_inquire_names_for_mech( ARG3_INQUIRE_NAMES_FOR_MECH ); /* in_n_f_m.c */
OM_uint32
gn_gss_inquire_mechs_for_name( ARG3_INQUIRE_MECHS_FOR_NAME ); /* in_m_f_n.c */
OM_uint32
gn_gss_canonicalize_name( ARG4_CANONICALIZE_NAME );	/* can_name.c */
OM_uint32
gn_gss_export_name( ARG3_EXPORT_NAME );			/* exp_name.c */
OM_uint32
gn_gss_duplicate_name( ARG3_DUPLICATE_NAME );		/* dup_name.c */


#endif /* _GN_DEFS_H_ */

