#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/krb5/krb5ctx.c#7 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/krb5/krb5ctx.c#7 $
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


#include "krb5defs.h"



/*
 * krb5_check_ctx()
 *
 *
 */
OM_uint32
krb5_check_ctx( OM_uint32       *  pp_min_stat,
	        void            ** pp_ctx,
		int                p_flags,
		char            *  this_Call )
{
   krb5_ctx_desc   * ctx;

   (*pp_min_stat) = MINOR_NO_ERROR;

   if ( pp_ctx==NULL || *pp_ctx==NULL ) {
      if ( (p_flags&CTX_ESTABLISH)!=0  &&  pp_ctx!=NULL )
	 return(GSS_S_COMPLETE);
      DEBUG_ERR((tf, "Internal ERROR: %s(): NULL priv_ctx handle!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   ctx = (krb5_ctx_desc *) *pp_ctx;

   if ( ctx->magic_cookie!=KRB5_CTX_COOKIE ) {
      DEBUG_ERR((tf, "Internal ERROR: %s(): invalid priv_ctx (bad cookie)!\n",
	         this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   if ( ctx->flag_established==FALSE ) {
      if ( (p_flags&(CTX_DELETE|CTX_ESTABLISH|CTX_INQUIRE))==0 ) {
	 DEBUG_ERR((tf, "Internal ERROR: %s(): incomplete priv_ctx!\n", this_Call))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
   } else { /* else   ctx->flag_established!=FALSE */
      if ( (p_flags&CTX_ESTABLISH)!=0 ) {
	 DEBUG_ERR((tf, "Internal ERROR: %s(): priv_ctx already established!\n", this_Call))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
   } /* endif   ctx->flag_established!=FALSE */

   return(GSS_S_COMPLETE);

} /* krb5_check_ctx() */




/*
 * krb5_release_token()
 *
 *
 */
OM_uint32
krb5_release_token( OM_uint32   * pp_min_stat,
		    Uchar      ** pp_token,
		    size_t      * pp_token_len )
{
   (*pp_min_stat) = 0;

   if ( *pp_token!=NULL ) {
      sy_clear_free( (void **)pp_token, (*pp_token_len) );
      (*pp_token_len) = 0;
   }

   return(GSS_S_COMPLETE);

} /* krb5_release_token() */



#if 0
/* no longer needed, we use ISC/ASC_REQ_ALLOC_MEMORY */

/*
 * krb5_alloc_ctx_token()
 *
 *
 */
OM_uint32
krb5_alloc_ctx_token( OM_uint32   * pp_min_stat,
		      Uchar      ** pp_token,
		      size_t      * pp_token_size )
{
   (*pp_min_stat)   = 0;
   (*pp_token)      = NULL;
   (*pp_token_size) = 0;

   (*pp_token) = sy_calloc( krb5_maxtoken );
   if (*pp_token==NULL) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   (*pp_token_size)		     = krb5_maxtoken;

   return(GSS_S_COMPLETE);

} /* krb5_alloc_ctx_token() */
#endif



/*
 * krb5_init_sec_context()
 *
 *
 */
OM_uint32
krb5_init_sec_context(  OM_uint32		 * pp_min_stat,	    /* in */
			void			 * p_cred,	    /* in */
			gss_channel_bindings_t	   p_channel_bindings,  /* in */
			Uchar			 * p_target,        /* in */
			size_t			   p_target_len,    /* in */
			Uchar			 * p_in_token,      /* in */
			size_t			   p_in_token_len,  /* in */
			OM_uint32		   p_service_req,   /* in */
			void			** pp_ctx,          /* out */
			Uchar			** pp_out_token,    /* out */
			size_t			 * pp_out_token_len,/* out */
			OM_uint32		 * pp_service_rec,  /* out */
			time_t			 * pp_expires_at )  /* out */
{
   char            * this_Call   = "krb5_init_sec_context";
   char            * symbol, * desc;
   krb5_ctx_desc   * ctx         = NULL;
   CtxtHandle      * pCtxt       = NULL;
   SecBufferDesc     OutBufDesc;
   SecBufferDesc     InBufDesc;
   SecBufferDesc   * pInBufDesc  = NULL;
   SecBuffer         OutSecToken;
   SecBuffer	     InSecToken;
   krb5_cred_desc  * cred        = NULL;
   SECURITY_STATUS   SecStatus;
   ULONG	     CtxFlagsReq = 0;
   OM_uint32         maj_stat    = GSS_S_COMPLETE;

   (*pp_min_stat)      = 0;
   (*pp_out_token)     = NULL;
   (*pp_out_token_len) = 0;
   (*pp_service_rec)   = 0;
   (*pp_expires_at)    = 0;

   OutSecToken.BufferType = SECBUFFER_TOKEN;
   OutSecToken.cbBuffer   = 0u;
   OutSecToken.pvBuffer   = NULL;

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, pp_ctx,
			      CTX_ESTABLISH, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (krb5_ctx_desc *) (*pp_ctx);

   /* quick validation of krb5 credential handle */
   maj_stat = krb5_check_cred( pp_min_stat, &p_cred, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   cred = (krb5_cred_desc *) p_cred;

   /* Kerberos currently still requires a target name ... */
   if ( p_target==NULL || p_target_len==0 ) {
      DEBUG_ERR((tf, "ERR: %s(): missing target name!\n", this_Call))
      RETURN_MIN_MAJ(MINOR_INTERNAL_ERROR, GSS_S_FAILURE);
   }

   /* The W2K Kerberos SSP doesn't implement GSS-API style channel bindings */
   if ( p_channel_bindings!=GSS_C_NO_CHANNEL_BINDINGS ) {
      RETURN_MIN_MAJ( KRB5_MINOR(NO_CHBINDINGS_IN_SSPI), GSS_S_FAILURE );
   }

   if (ctx==NULL) {
      /* No prior priv_ctx security context handle ==> create one */

      if ( p_in_token!=NULL && p_in_token_len!=0 ) {
	 DEBUG_ERR((tf, "Internal ERROR: %s(): Extra token to initial call!\n",
		        this_Call))
	 RETURN_MIN_MAJ(MINOR_INTERNAL_ERROR, GSS_S_FAILURE);
      }
      /* There is no context handle yet, so this is the first step in */
      /* the security context establishment procedure                 */
      ctx = sy_calloc( sizeof(*ctx) );
      if (ctx==NULL) {
	 RETURN_MIN_MAJ(MINOR_OUT_OF_MEMORY, GSS_S_FAILURE);
      }
      ctx->magic_cookie     = KRB5_CTX_COOKIE;
      ctx->flag_established = FALSE;
      ctx->role             = KRB5_INITIATOR;
      ctx->nego_step        = KRB5_1ST_TOKEN;
      SecInvalidateHandle( &(ctx->sspi_ctx) );
      ctx->sspi_isc_req     = krb5_gss_svc_to_ISC_REQ( p_service_req );
      ctx->seq_ini2acc	    = 1; /* initiator->acceptor messages are odd numbered */
      ctx->seq_acc2ini      = 2; /* acceptor->initiator message are even numbered */
      pCtxt                 = NULL;

      /* NOTICE:  p_target is always a BINARY CANONICAL NAME      */
      /*          (don't let this code mislead you)               */
      maj_stat = krb5_copy_name( pp_min_stat,
				 p_target, p_target_len,
				 &(ctx->acceptor), &(ctx->acceptor_len) );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

      /* And then we will also copy our name (the initiator) into  */
      /* our context attribute structure for future reference      */
      maj_stat = krb5_copy_name( pp_min_stat,
				 cred->name, cred->name_len,
				 &(ctx->initiator), &(ctx->initiator_len) );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

      /* no input token for initial call to InitializeSecurityContext() */
      pInBufDesc          = NULL;

   } else {

      /* There is a prior priv_ctx security context handle ==> use it */
      if (ctx->nego_step!=KRB5_3RD_TOKEN) {
	 /* since KRB5 doesn't error as it should, we'll do it here  */
	 /* ... but actually, the generic layer should dispatch here */
	 /* any more once we flag "context established"              */
	 DEBUG_ERR((tf, "ERR: %s(): there is no %d nego step!\n",
			this_Call, ctx->nego_step ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

      if ( p_in_token==NULL || p_in_token_len<3 ) {
	 DEBUG_ERR((tf, "ERR: %s(): input token missing for continuation call!\n",
			this_Call))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

      ctx->nego_step = KRB5_3RD_TOKEN;
	     /* a single continuation call to gss_init_sec_context */
	     /* will return the final context establishment token  */
      pCtxt      = &(ctx->sspi_ctx);

      /* There must be an input token for every continuation initial call to InitializeSecurityContext() */
      InSecToken.pvBuffer   = p_in_token;
      InSecToken.cbBuffer   = (ULONG)p_in_token_len;
      InSecToken.BufferType = SECBUFFER_TOKEN | SECBUFFER_READONLY;

      InBufDesc.cBuffers    = 1u;
      InBufDesc.pBuffers    = &InSecToken;
      InBufDesc.ulVersion   = SECBUFFER_VERSION;

      pInBufDesc            = &InBufDesc;

   }

   OutBufDesc.cBuffers    = 1u;
   OutBufDesc.pBuffers    = &OutSecToken;
   OutBufDesc.ulVersion   = SECBUFFER_VERSION;

   SecStatus = (fp_SSPI_krb5
	    ->InitializeSecurityContext)(
     /* PCredHandle    phCredential  */   &(cred->sspi_cred),
     /* PCtxtHandle    phContext     */   pCtxt,
     /* SEC_CHAR     * pszTargetName */	  (ctx->acceptor),
     /* ULONG          fContextReq   */	  ctx->sspi_isc_req,
     /* ULONG          Reserved1     */   0,
     /* ULONG	       TargetDataRep */	  SECURITY_NATIVE_DREP,
     /* PSecBufferDesc pInput        */	  pInBufDesc,
     /* ULONG	       Reserved2     */   0,
     /* PCtxtHandle    phNewContext  */	  &(ctx->sspi_ctx),
     /* PSecBufferDesc pOutput       */	  &OutBufDesc,
     /* PULONG         pfContextAttr */	  &(ctx->sspi_isc_ret),
     /* PTimeStamp     ptsExpiry     */	  &(ctx->sspi_expiration) );

   if ( SecStatus==SEC_I_CONTINUE_NEEDED || SecStatus==SEC_E_OK ) {
      DEBUG_ACTION((tf, "  A: %s(): InitializeSecurityContext(target=\"%.256s\")==%s\n",
		        this_Call, ctx->acceptor,
			SecStatus==SEC_E_OK ? "SEC_E_OK" : "CONTINUE_NEEDED" ));
   }

   if ( SecStatus==SEC_E_OK ) {

      SecStatus = (fp_SSPI_krb5
	     ->QueryContextAttributes)(
	/* PCtxtHandle   phContext   */    &(ctx->sspi_ctx),
        /* ULONG         ulAttribute */    SECPKG_ATTR_SIZES,
	/* PVOID         pBuffer     */    &(ctx->sizes) );

      if ( SecStatus!=SEC_E_OK ) {
	 krb5_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "ERR: %s(): QueryContextAttributes(SIZES) failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
	 ERROR_RETURN( KRB5_MINOR_SSPI(QueryContextAttributesSIZES,SecStatus), GSS_S_FAILURE );
      }

      (ctx->flag_established) = TRUE;

   } else if ( SecStatus==SEC_I_CONTINUE_NEEDED ) {

      maj_stat = GSS_S_CONTINUE_NEEDED;
      if ( OutSecToken.cbBuffer==0 ) {
	 DEBUG_ERR((tf, "ERR: %s(): CONTINUE_NEEDED but no token?!\n",
		        this_Call))
	 ERROR_RETURN( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
      
   } else if ( SecStatus!=SEC_E_OK ) {

      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "ERR: %s(): InitializeSecurityContext(target=\"%.256s\") failed with %s\n\t(Desc=\"%s\")\n",
		     this_Call, ctx->acceptor,
		     symbol, desc));
      if ( KRB5_1ST_TOKEN==ctx->nego_step ) {
	  ERROR_RETURN( KRB5_MINOR_SSPI(InitializeSecurityContext1,SecStatus), GSS_S_FAILURE );
      }
      ERROR_RETURN( KRB5_MINOR_SSPI(InitializeSecurityContextN,SecStatus), GSS_S_FAILURE );
   }


   if ( maj_stat==GSS_S_COMPLETE
        ||  maj_stat==GSS_S_CONTINUE_NEEDED ) {

      (*pp_ctx)           = ctx;
      (ctx->expires_at)   = krb5_timestamp2time( &(ctx->sspi_expiration) );
      (*pp_expires_at)    = (ctx->expires_at);

      if ( OutSecToken.cbBuffer > 0 ) {

	 OM_uint32          maj_stat2;
	 OM_uint32          min_stat2;
	 gn_mech_tag_et     mech_tag;
	 gss_buffer_desc    out_token;
	 gss_OID_desc       token_oid;
	 void             * itoken;
	 size_t             itoken_len;

	 (*pp_out_token) = sy_malloc( OutSecToken.cbBuffer );
	 if ( (*pp_out_token)==NULL )
	    ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );

	 memcpy( (*pp_out_token), OutSecToken.pvBuffer, OutSecToken.cbBuffer );
	 (*pp_out_token_len) = OutSecToken.cbBuffer;

	 out_token.value  = (*pp_out_token);
	 out_token.length = (*pp_out_token_len);

	 maj_stat2 = gn_parse_gss_token( &min_stat2, &out_token, &token_oid, &mech_tag, &itoken, &itoken_len );
	 if (maj_stat2!=GSS_S_COMPLETE) {
	    if ( maj_stat2==GSS_S_BAD_MECH 
		&& token_oid.length==krb5_w2k3_u2u_oid->length
		&& 0==memcmp(token_oid.elements, krb5_w2k3_u2u_oid->elements, token_oid.length) ) {
		DEBUG_ERR((tf, "ERR: %s(): InitializeSecurityContext(target=\"%.256s\") returned U2U-token!?\n",
			       this_Call, ctx->acceptor ));
		ERROR_RETURN( KRB5_MINOR(W2K3_U2U_TARGET), GSS_S_FAILURE );
	    }
	    DEBUG_ERR((tf, "ERR: %s(): InitializeSecurityContext(target=\"%.256s\") returned non-GSSAPI-token!?\n",
	                   this_Call, ctx->acceptor ));
	    ERROR_RETURN( KRB5_MINOR(SSPI_TALKS_GARBAGE), GSS_S_FAILURE );
	 }
	 if (mech_tag!=krb5_mech_tag) {
	    DEBUG_ERR((tf, "ERR: %s(): InitializeSecurityContext(target=\"%.256s\") returned non-Kerberos5-token!?\n",
	                   this_Call, ctx->acceptor ));
	    ERROR_RETURN( KRB5_MINOR(SSPI_WRONG_MECH), GSS_S_FAILURE );
	 }
      }

      (ctx->service_rec) = krb5_ISC_RET_to_gss_svc( ctx->sspi_isc_ret );
      (*pp_service_rec)  = (ctx->service_rec); 
      (ctx->nego_step)  += 2;
   
   } else {
      OM_uint32  min_stat2;
error:
      /* Release the output_token if there was one allocated */
      (void)krb5_release_token( &min_stat2, pp_out_token, pp_out_token_len );

      if (ctx->nego_step==KRB5_1ST_TOKEN) {
	 /* This is the first negotiation step -- we have to drop */
	 /* everything after error situations in the initial step */
	 (void)krb5_delete_sec_context( &min_stat2, &ctx );
	 (*pp_ctx) = NULL;
      }
   }
      
   if ( OutSecToken.pvBuffer!=NULL && OutSecToken.cbBuffer>0 ) {
      (fp_SSPI_krb5->FreeContextBuffer)( OutSecToken.pvBuffer );
      OutSecToken.pvBuffer = NULL;
      OutSecToken.cbBuffer = 0;
   }

   return(maj_stat);

} /* krb5_init_sec_context() */




/*
 * krb5_accept_sec_context()
 *
 *
 */
OM_uint32
krb5_accept_sec_context( OM_uint32		 * pp_min_stat,   /* in  */
			 void			 * p_cred,	  /* in  */
			 gss_channel_bindings_t	   p_channel_bindings,	/* in  */
			 Uchar			 * p_in_token,    /* in  */
			 size_t			   p_in_token_len,/* in  */
			 void			** pp_ctx,	  /* out */
			 Uchar			** pp_srcname,	  /* out */
			 size_t			 * pp_srcname_len,/* out */
			 gn_nt_tag_et		 * pp_nt_tag,	  /* out */
			 Uchar			** pp_out_token,  /* out */
			 size_t			 * pp_out_token_len,/* out */
			 OM_uint32		 * pp_service_rec,/* out */
			 time_t			 * pp_expires_at, /* out */
			 gn_cred_t		 * pp_deleg_cred )/* out */
{
   char                       * this_Call   = "krb5_accept_sec_context";
   krb5_ctx_desc	      * ctx         = NULL; /* convenience pointer */
   krb5_cred_desc	      * cred	    = NULL; /* convenience pointer */
   CtxtHandle		      * pCtxt       = NULL; /* returnable dynamic SSPI object */
   char			      * symbol, * desc;
   char			      * user        = NULL; /* temporary memory */
   char			      * realm       = NULL; /* temporary memory */
   SecBufferDesc		OutBufDesc;	    /* local scrap */
   SecBufferDesc	        InBufDesc;	    /* local scrap */
   SecBuffer		        OutSecToken;	    /* temporary dynamic SSPI object */
   SecBuffer			InSecToken;	    /* local scrap */
   SECURITY_STATUS	        SecStatus;
   SECURITY_STATUS	        SecStatus1  = SEC_E_OK;
   SECURITY_STATUS	        SecStatus2  = SEC_E_OK;
   SECURITY_STATUS	        SecStatus3  = SEC_E_OK;
   SECURITY_STATUS	        SecStatus4  = SEC_E_OK;
   SecPkgContext_Names		NamesBuffer;        /* temporary dynamic SSPI object */
   SecPkgContext_NativeNames    NativeNamesBuffer;  /* temporary dynamic SSPI object */
   ULONG			CtxFlagsReq = 0;
   OM_uint32			maj_stat    = GSS_S_COMPLETE;

   NamesBuffer.sUserName	 = NULL;
   NativeNamesBuffer.sClientName = NULL;
   NativeNamesBuffer.sServerName = NULL;

   (*pp_min_stat)	= 0;
   (*pp_out_token)	= NULL;
   (*pp_out_token_len)	= 0;
   (*pp_service_rec)	= 0;
   (*pp_expires_at)	= 0;
   (*pp_srcname)	= NULL;
   (*pp_srcname_len)	= 0;
   (*pp_nt_tag)		= NT_INVALID_TAG;

   if ( pp_deleg_cred )  { (*pp_deleg_cred)      = NULL; }

   OutSecToken.pvBuffer   = NULL;
   OutSecToken.cbBuffer   = 0;
   OutSecToken.BufferType = SECBUFFER_TOKEN;

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, pp_ctx,
			      CTX_ESTABLISH, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (krb5_ctx_desc *) (*pp_ctx);

   /* quick validation of krb5 credential handle */
   maj_stat = krb5_check_cred( pp_min_stat, &p_cred, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   cred = (krb5_cred_desc *) p_cred;

   if ( p_in_token==NULL || p_in_token_len<3 ) {
      DEBUG_ERR((tf, "ERR: %s(): input token missing!\n",
		     this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   /* The W2K Kerberos SSP doesn't implement GSS-API style channel bindings */
   if ( p_channel_bindings!=GSS_C_NO_CHANNEL_BINDINGS ) {
      RETURN_MIN_MAJ( KRB5_MINOR(NO_CHBINDINGS_IN_SSPI), GSS_S_FAILURE );
   }

   if (ctx==NULL) {
      /* No prior priv_ctx security context handle ==> create one     */
      /* There is no context handle yet, so this is the first step in */
      /* the security context establishment procedure                 */
      ctx = sy_calloc( sizeof(*ctx) );
      if (ctx==NULL) {
	 RETURN_MIN_MAJ(MINOR_OUT_OF_MEMORY, GSS_S_FAILURE);
      }
      ctx->magic_cookie     = KRB5_CTX_COOKIE;
      ctx->flag_established = FALSE;
      ctx->role             = KRB5_ACCEPTOR;
      ctx->nego_step        = KRB5_2ND_TOKEN;
      SecInvalidateHandle( &(ctx->sspi_ctx) );
      ctx->sspi_asc_req	    = krb5_gss_svc_to_ASC_REQ(); /* get default ASC_REQs */
      ctx->seq_ini2acc	    = 1; /* initiator->acceptor messages are odd numbered */
      ctx->seq_acc2ini      = 2; /* acceptor->initiator message are even numbered */
      pCtxt                 = NULL;

      /* We have to copy our name (the acceptor) from our creds into   */
      /* our context attribute structure for future reference          */
      /* REMEMBER:  this name is in BINARY CANONICAL format            */
      maj_stat = krb5_copy_name( pp_min_stat,
				 cred->name, cred->name_len,
				 &(ctx->acceptor), &(ctx->acceptor_len) );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

   } else {

      /* Kerberos security context establishment (rfc1964) is only 1-way or 2-way */
      /* if we get another context-level token, then it must be SSPI garbage      */
      RETURN_MIN_MAJ( KRB5_MINOR(SSPI_TALKS_GARBAGE), GSS_S_FAILURE );

   }

   /* accept_sec_context() must always be called with an input token.             */
   /* Create the correct buffer objects for the SSPI call AcceptSecurityContext() */
   InSecToken.pvBuffer   = p_in_token;
   InSecToken.cbBuffer   = (ULONG)p_in_token_len;
   InSecToken.BufferType = SECBUFFER_TOKEN | SECBUFFER_READONLY;

   InBufDesc.cBuffers    = 1u;
   InBufDesc.pBuffers    = &InSecToken;
   InBufDesc.ulVersion   = SECBUFFER_VERSION;

   OutBufDesc.cBuffers    = 1u;
   OutBufDesc.pBuffers    = &OutSecToken;
   OutBufDesc.ulVersion   = SECBUFFER_VERSION;

   SecStatus = (fp_SSPI_krb5
	    ->AcceptSecurityContext)(
     /* PCredHandle    phCredential  */  &(cred->sspi_cred),
     /* PCtxtHandle    phContext     */  pCtxt,
     /* PSecBufferDesc pInput        */  &InBufDesc,
     /* ULONG          fContextReq   */  ctx->sspi_asc_req,
     /* ULONG          TargetDataRep */  SECURITY_NATIVE_DREP,
     /* PCtxtHandle    phNewCount    */  &(ctx->sspi_ctx),
     /* PSecBufferDesc pOutput       */  &OutBufDesc,
     /* PULONG         pfContextAttr */  &(ctx->sspi_asc_ret),
     /* PTimeStamp     ptsExpiry     */  &(ctx->sspi_expiration) );

   if ( SecStatus==SEC_I_CONTINUE_NEEDED || SecStatus==SEC_E_OK ) {
      DEBUG_ACTION((tf, "  A: %s(): AcceptSecurityContext #%d()==%s\n",
			this_Call, (int)(ctx->nego_step/2)+1,
			(SecStatus==SEC_E_OK) ? "SEC_E_OK" : "CONTINUE_NEEDED" ));
   }

   if (SecStatus==SEC_E_OK) {
      SecStatus = (fp_SSPI_krb5
	     ->QueryContextAttributes)(
	/* PCtxtHandle   phContext   */    &(ctx->sspi_ctx),
        /* ULONG         ulAttribute */    SECPKG_ATTR_SIZES,
	/* PVOID         pBuffer     */    &(ctx->sizes) );

      if ( SecStatus!=SEC_E_OK ) {
	 krb5_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "ERR: %s(): QueryContextAttributes(SIZES) failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
	 ERROR_RETURN( KRB5_MINOR_SSPI(QueryContextAttributesSIZES,SecStatus), GSS_S_FAILURE );
      }

      /* Query the security context initiator from the underlying Kerberos5 */
      /* name-based authentication through the new functionality of         */
      /* the QueryContextAttributes call in Windows 2000	            */
      SecStatus = (fp_SSPI_krb5->QueryContextAttributes)(
	/* PCtxtHandle   phContext   */		     &(ctx->sspi_ctx),
        /* ULONG         ulAttribute */		     SECPKG_ATTR_NATIVE_NAMES,
	/* PVOID         pBuffer     */		     &(NativeNamesBuffer) );
	    
      if ( SecStatus!=SEC_E_OK ) {
	 krb5_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "ERR: %s(): QueryContextAttributes(NATIVE_NAMES) failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, symbol, desc ));
	 ERROR_RETURN( KRB5_MINOR_SSPI(QueryContextAttributesNATNAMES,SecStatus), GSS_S_FAILURE );
      }

      maj_stat = krb5_copy_name( pp_min_stat,
			         NativeNamesBuffer.sClientName, strlen(NativeNamesBuffer.sClientName),
				 &(ctx->initiator), &(ctx->initiator_len) );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

      /*********************************************************************/
      /* (13-Feb-2012) Martin Rex                                          */
      /* BUGBUG: Workaround for S4U2Proxy misbehaviour of forging Kerberos */
      /*         tickets for incorrect principals, the username part of    */
      /*         Samcompatible Name will more likely contain the correct   */
      /*         spelling upon authentication                              */
      /* In case the username part differs _only_ in case, we'll use the   */
      /* username part from NAMES rather that from NATIVE_NAMES            */
      /*********************************************************************/
      SecStatus = (fp_SSPI_krb5->QueryContextAttributes)(
	/* PCtxtHandle   phContext   */		     &(ctx->sspi_ctx),
        /* ULONG         ulAttribute */		     SECPKG_ATTR_NAMES,
	/* PVOID         pBuffer     */		     &(NamesBuffer) );
	    
      if ( SecStatus!=SEC_E_OK ) {
	 krb5_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "ERR: %s(): QueryContextAttributes(NAMES) failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, symbol, desc ));
         /* ignore this error */
         
	 /* ERROR_RETURN( KRB5_MINOR_SSPI(QueryContextAttributesNAMES,SecStatus), GSS_S_FAILURE ); */
      } else if ( NULL!=ctx->initiator && NULL!=NamesBuffer.sUserName ) {
         char    * ptrNN = strchr(ctx->initiator, '@');
         char    * ptrN  = strchr(NamesBuffer.sUserName, '\\');
         size_t    NN_len;
         size_t    N_len;

         if ( NULL!=ptrN && NULL!=ptrNN ) {
            NN_len = (size_t)(ptrNN - ctx->initiator);  /* username@SOME.REALM */
            N_len  =         strlen(&(ptrN[1]));        /* NT4DOMAIN\username */
            if ( NN_len==N_len && 0==_strnicmp(ctx->initiator,&(ptrN[1]),N_len) ) {
               memcpy(ctx->initiator, &(ptrN[1]), N_len);
            }
         }
      }

      if ( FALSE!=krb5_realm_to_upper && NULL!=ctx->initiator ) {
         char * ptr = strchr(ctx->initiator, '@');
         if ( NULL!=ptr ) {
            /* Force Kerberos Realm name to uppercase                       */
            /* It seems that in some scenarios using Microsofts proprietary */
            /* "Protocol Transition with Constrained Delegation", Microsoft */
            /* might forge Kerberos tickets with incorrect Realm names      */
            /* (lowercase) -- which breaks name-based authentication of     */
            /* gss-api acceptors                                            */
            CharUpper(ptr);
         }
      }

      (*pp_srcname)     = ctx->initiator;
      (*pp_srcname_len) = ctx->initiator_len;
      (*pp_nt_tag)      = krb5_nt_tag;

      ctx->flag_established = TRUE;

      if ( pp_deleg_cred!=NULL ) {
	  OM_uint32	    maj_stat2, min_stat2;
	  OM_uint32	    lifetime;
	  SECURITY_STATUS   SecStatus2;
	  DEBUG_ACTION((tf, "  A: %s(): Checking for delegated credentials\n", this_Call));
	  /* caller asks for delegated creds, try to get them */
          SecStatus2 = (fp_SSPI_krb5->ImpersonateSecurityContext)(&(ctx->sspi_ctx));
          if ( SecStatus2==SEC_E_OK ) {
	      maj_stat2 = gn_new_cred( &min_stat2, NULL, GSS_C_INITIATE,
					krb5_mech_tag, pp_deleg_cred, &lifetime );
	      DEBUG_ACTION((tf, "  A: %s(): Acquisition of delegated credential %s\n",
			        this_Call,
				(maj_stat2==GSS_S_COMPLETE) ? "successful" : "failed" ));
	  } else {
	      char *symbol, *desc;
      
	      krb5_sec_error( SecStatus, &symbol, &desc );
	      DEBUG_ERR((tf, "ERR: %s(): ImpersonateSecurityContext() failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, symbol, desc ));
	  }

	  (fp_SSPI_krb5->RevertSecurityContext)( &(ctx->sspi_ctx));
      }

   } else if (SecStatus==SEC_I_CONTINUE_NEEDED) {

      maj_stat = GSS_S_CONTINUE_NEEDED;
      if ( OutSecToken.cbBuffer==0 ) {
	 DEBUG_ERR((tf, "ERR: %s(): CONTINUE_NEEDED but no token?!\n",
		        this_Call))
	 ERROR_RETURN( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

      /* Kerberos security context establishment (rfc1964) is only 1-way or 2-way */
      /* if we get SEC_I_CONTINUE_NEEDED, then it must be SSPI garbage            */
      DEBUG_ERR((tf, "ERR: %s(): AcquireSecurityContext() returned CONTINUE_NEEDED,"
	             "which is not permitted for rfc-1964 Kerberos!\n", this_Call));
      ERROR_RETURN( KRB5_MINOR(SSPI_TALKS_GARBAGE), GSS_S_FAILURE );
      
   } else if ( SecStatus!=SEC_E_OK ) {

      char *symbol, *desc;
      
      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "ERR: %s(): AcceptSecurityContext() failed with %s\n\t(Desc=\"%s\")\n",
			 this_Call, symbol, desc));
      if ( KRB5_2ND_TOKEN==ctx->nego_step ) {
          ERROR_RETURN( KRB5_MINOR_SSPI(AcceptSecurityContext1,SecStatus), GSS_S_FAILURE );
      }
      ERROR_RETURN( KRB5_MINOR_SSPI(AcceptSecurityContextN,SecStatus), GSS_S_FAILURE );
   }

   if ( maj_stat==GSS_S_COMPLETE
        ||  maj_stat==GSS_S_CONTINUE_NEEDED ) {

      (*pp_ctx)           = ctx;
      (ctx->expires_at)   = krb5_timestamp2time( &(ctx->sspi_expiration) );
      (*pp_expires_at)    = (ctx->expires_at);

      if ( OutSecToken.cbBuffer > 0 ) {

	 OM_uint32          maj_stat2;
	 OM_uint32          min_stat2;
	 gn_mech_tag_et     mech_tag;
	 gss_buffer_desc    out_token;
	 void             * itoken;
	 size_t             itoken_len;

	 (*pp_out_token) = sy_malloc( OutSecToken.cbBuffer );
	 if ( (*pp_out_token)==NULL )
	    ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );

	 memcpy( (*pp_out_token), OutSecToken.pvBuffer, OutSecToken.cbBuffer );
	 (*pp_out_token_len) = OutSecToken.cbBuffer;

	 out_token.value  = (*pp_out_token);
	 out_token.length = (*pp_out_token_len);

	 maj_stat2 = gn_parse_gss_token( &min_stat2, &out_token, NULL, &mech_tag, &itoken, &itoken_len );
	 if (maj_stat2!=GSS_S_COMPLETE)
	    ERROR_RETURN( KRB5_MINOR(SSPI_TALKS_GARBAGE), GSS_S_FAILURE );
	 if (mech_tag!=krb5_mech_tag)
	    ERROR_RETURN( KRB5_MINOR(SSPI_WRONG_MECH), GSS_S_FAILURE );

      }

      (ctx->service_rec) = krb5_ASC_RET_to_gss_svc( ctx->sspi_asc_ret );
      (*pp_service_rec)  = (ctx->service_rec);
      (ctx->nego_step)  += 2;

   } else {

      OM_uint32   min_stat2;
error:
      /* Release the output_token if there was one allocated */
      (void)krb5_release_token( &min_stat2, pp_out_token, pp_out_token_len );

      if (ctx->nego_step==KRB5_2ND_TOKEN) {
	 /* This is the first negotiation step -- we have to drop */
	 /* everything after error situations in the initial step */
	 krb5_delete_sec_context( &min_stat2, &ctx );
	 (*pp_ctx) = NULL;
      }

      if ( pp_deleg_cred!=NULL && *pp_deleg_cred!=NULL ) {
	  gn_release_cred( &min_stat2, pp_deleg_cred );
      }
   }

   SecStatus1 = krb5_free_sspi_buffer( "NamesBuffer.sUserName",
				       &(NamesBuffer.sUserName) );
   SecStatus2 = krb5_free_sspi_buffer( "NativeNamesBuffer.sClientName",
				       &(NativeNamesBuffer.sClientName) );
   SecStatus3 = krb5_free_sspi_buffer( "NativeNamesBuffer.sServerName",
				       &(NativeNamesBuffer.sServerName) );
   if ( OutSecToken.cbBuffer>0 ) {
      SecStatus4 = krb5_free_sspi_buffer( "OutSecToken.pvBuffer",
					  &(OutSecToken.pvBuffer) );
      OutSecToken.cbBuffer = 0;
   }

   SecStatus = (SecStatus1!=SEC_E_OK) ? SecStatus1
	          : ( (SecStatus2!=SEC_E_OK) ? SecStatus2
		      : ( (SecStatus3!=SEC_E_OK) ? SecStatus3 : SecStatus4 ) );

   if ( SecStatus!=SEC_E_OK ) {
      (*pp_min_stat) = KRB5_MINOR_WINERROR(FreeContextBuffer,SecStatus);
      maj_stat       = GSS_S_FAILURE;
   }

   return(maj_stat);

} /* krb5_accept_sec_context() */




/*
 * krb5_delete_sec_context()
 *
 *
 */
OM_uint32
krb5_delete_sec_context( OM_uint32   * pp_min_stat,
		         void       ** pp_ctx )
{
   char             * this_Call = "krb5_delete_context";
   krb5_ctx_desc    * ctx       = NULL;
   SECURITY_STATUS    SecStatus;
   OM_uint32          maj_stat  = GSS_S_COMPLETE;

   (*pp_min_stat) = 0;

   if ( *pp_ctx==NULL ) 
      return(maj_stat);

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, pp_ctx,
			      CTX_DELETE, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = *pp_ctx;

   if ( IsValidSecurityHandle( &(ctx->sspi_ctx) ) ) {

      SecStatus = (fp_SSPI_krb5->DeleteSecurityContext)( &(ctx->sspi_ctx) );
      if ( SecStatus!=SEC_E_OK ) {
	 char *symbol, *desc;
      
	 krb5_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "ERR: %s(): DeleteSecurityContext() failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
	 (*pp_min_stat) = KRB5_MINOR_SSPI(DeleteSecurityContext,SecStatus); 
	 maj_stat       = GSS_S_FAILURE;
      }
      SecInvalidateHandle( &(ctx->sspi_ctx) );
   }

   if ( ctx->initiator!=NULL ) {
      sy_clear_free( &(ctx->initiator), ctx->initiator_len );
   }

   if ( ctx->acceptor!=NULL ) {
      sy_clear_free( &(ctx->acceptor), ctx->acceptor_len );
   }

   sy_clear_free( (void **)&ctx, sizeof(*ctx) );
   (*pp_ctx) = NULL;

   return(maj_stat);

} /* krb5_delete_sec_context() */




/*
 * krb5_export_sec_context()
 *
 *
 */
OM_uint32
krb5_export_sec_context( OM_uint32    * pp_min_stat,
			 void	     ** pp_ctx,
			 void	     ** pp_ctx_buffer,
			 size_t	      * pp_ctx_buffer_len )
{
   char               * this_Call      = "krb5_export_sec_context";
   krb5_ctx_desc      * ctx            = NULL;  /* convenience pointer */
   krb5_exp_ctx_desc  * expctx	       = NULL;  
   Uchar	      * buffer         = NULL;  /* returnable dynamic object */
   Uchar	      * ptr	       = NULL;  /* convenience pointer */
   Uchar	      * sspi_token     = NULL;  /* SSPI temporary memory */
   SecBuffer	        SecBuf;
   SECURITY_STATUS      SecStatus;
   size_t	        buffer_len     = 0;
   size_t	        sspi_token_len = 0;
   OM_uint32		min_stat;
   OM_uint32	        maj_stat       = GSS_S_COMPLETE;


   (*pp_min_stat)       = MINOR_NO_ERROR;
   (*pp_ctx_buffer)     = NULL;
   (*pp_ctx_buffer_len) = 0;

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, pp_ctx,
			      CTX_TRANSFER, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (krb5_ctx_desc *) (*pp_ctx);

   SecBuf.BufferType  = SECBUFFER_EMPTY;
   SecBuf.cbBuffer    = 0;
   SecBuf.pvBuffer    = NULL;

   SecStatus = (fp_SSPI_krb5->ExportSecurityContext)(
      /* PCtxtHandle         phContext      */	 &(ctx->sspi_ctx),
      /* ULONG               fFlags         */	 SECPKG_CONTEXT_EXPORT_DELETE_OLD,
      /* PSecBuffer          pPackedContext */	 &SecBuf,
      /* void             ** pToken         */   NULL );

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "ERR: %s(): ExportSecurityContext() failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
      ERROR_RETURN( KRB5_MINOR_SSPI(ExportSecurityContext,SecStatus), GSS_S_FAILURE );
   } else {
      DEBUG_ACTION((tf, "  A: %s(): ExportSecurityContext()==SEC_E_OK, token_len = %lu\n",
			this_Call, (unsigned long)SecBuf.cbBuffer ));
   }

   SecInvalidateHandle( &(ctx->sspi_ctx) );

   /* the SSPI security context will be dropped by the final       */
   /* call to krb5_delete_sec_context()                            */
   /* only our shadowed information will survive context transfers */
   sspi_token_len = SecBuf.cbBuffer;
   sspi_token     = SecBuf.pvBuffer;

   buffer_len = sizeof(*expctx)
	        + ctx->initiator_len + ctx->acceptor_len + sspi_token_len;

   if ( buffer_len>USHRT_MAX ) {
       DEBUG_ERR((tf, "ERR: %s(): Exported context token to large (%ld bytes) for this code!\n",
		      this_Call, (unsigned long) buffer_len ));
       ERROR_RETURN( MINOR_INTERNAL_BUFFER_OVERRUN, GSS_S_FAILURE );
   }

   buffer = sy_malloc( buffer_len ); /* we're filling everything anyways */
   if (buffer==NULL) {
      ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   expctx = (krb5_exp_ctx_desc *) buffer;

   /* clear structure body, avoid uninitalized memory copy warnings */
   /* from purify if the struct contains padding at the end         */
   memset(buffer,0,sizeof(*expctx));

   expctx->magic_cookie     =	       KRB5_CTX_COOKIE;
   expctx->sspi_asc_req     =	       ctx->sspi_asc_req;
   expctx->sspi_asc_ret     =	       ctx->sspi_asc_ret;
   expctx->sspi_isc_req     =	       ctx->sspi_isc_req;
   expctx->sspi_isc_ret     =	       ctx->sspi_isc_ret;
   expctx->seq_ini2acc	    =          ctx->seq_ini2acc;
   expctx->seq_acc2ini	    =          ctx->seq_acc2ini;
   expctx->service_rec      =          ctx->service_rec;
   expctx->sizes	    =	       ctx->sizes;            /* struct copy */
   expctx->sspi_expiration  =          ctx->sspi_expiration;  /* struct copy */
   expctx->expires_at       =	       ctx->expires_at;
   expctx->token_len        = (Ushort) buffer_len;
   expctx->initiator_len    = (Ushort) ctx->initiator_len;
   expctx->acceptor_len     = (Ushort) ctx->acceptor_len;
   expctx->sspi_token_len   = (Ushort) sspi_token_len;
   expctx->nego_step        = (Uchar)  ctx->nego_step;
   expctx->role		    = (Uchar)  ctx->role;

   ptr = &(buffer[sizeof(*expctx)]);
   if ( ctx->initiator_len>0 ) {
      memcpy(ptr, ctx->initiator, ctx->initiator_len);
      ptr += ctx->initiator_len;
   }
   if ( ctx->acceptor_len>0 ) {
      memcpy(ptr, ctx->acceptor, ctx->acceptor_len);
      ptr += ctx->acceptor_len;
   }
   if ( sspi_token_len>0 ) {
      memcpy(ptr, sspi_token, sspi_token_len );
      ptr += sspi_token_len;
   }

   if (maj_stat==GSS_S_COMPLETE) {

      (void) krb5_delete_sec_context( &min_stat, pp_ctx );

      (*pp_ctx_buffer)     = buffer;
      (*pp_ctx_buffer_len) = buffer_len;

   } else {
error:
      if ( sspi_token!=NULL ) {
	 /* try to reimport the security context */	 
	 SecStatus = (fp_SSPI_krb5->ImportSecurityContext)(
	    /* PSECURITY_STRING     pszPackage     */  krb5_provider,
	    /* PSecBuffer           pPackedContext */  &SecBuf,
	    /* void SEC_FAR       * Token          */  NULL,
	    /* PCtxtHandle          phContext      */  &(ctx->sspi_ctx) );

	 if ( SecStatus!=SEC_E_OK ) {
	    /* Too bad, we cannot even recover the exported security context !         */
	    /* So we do not just fail, but we also delete the damaged security context */
	    char *symbol, *desc;
      
	    krb5_sec_error( SecStatus, &symbol, &desc );
	    DEBUG_ERR((tf, "ERR: %s(): ImportSecurityContext() failed with %s\n\t(Desc=\"%s\")\n",
			   this_Call, symbol, desc ))
	    krb5_delete_sec_context( &min_stat, pp_ctx );

	    (*pp_min_stat) = KRB5_MINOR_SSPI(ImportSecurityContext,SecStatus);
	    maj_stat       = GSS_S_FAILURE;
	 }
      }

      sy_clear_free( (void **)&buffer, buffer_len );
      (*pp_ctx_buffer)     = NULL;
      (*pp_ctx_buffer_len) = 0;
   }

   if ( sspi_token!=NULL ) {
      (fp_SSPI_krb5->FreeContextBuffer)( sspi_token );
      sspi_token     = NULL;
      sspi_token_len = 0;
   }

   return(maj_stat);

} /* krb5_export_sec_context() */




/*
 * krb5_import_sec_context()
 *
 *
 */
OM_uint32
krb5_import_sec_context( OM_uint32   * pp_min_stat,
			 Uchar	     * p_ctx_buffer,
			 size_t	       p_ctx_buffer_len,
			 void	    ** pp_ctx )
{
   char               * this_Call      = "krb5_import_sec_context";
   krb5_ctx_desc      * ctx            = NULL;  /* returnable dynamic object */
   krb5_exp_ctx_desc    impctx;		        /* convenience struct  */	   
   Uchar	      * ptr	       = NULL;  /* convenience pointer */
   Uchar	      * alignedbuf     = NULL;  /* temporary dynamic memory, free() on exit */
   size_t               alignedbuf_len = 0;
   SecBuffer            SecBuf;
   SECURITY_STATUS      SecStatus;
   size_t	        buffer_len     = 0;
   static int		cred_bug_flag  = 0;
   OM_uint32		min_stat;
   OM_uint32	        maj_stat       = GSS_S_COMPLETE;

   (*pp_min_stat) = MINOR_NO_ERROR;
   (*pp_ctx)	  = NULL;

   /****************************************************************/
   /* Workaround for yet another Win2KSP2 and Win64 breakage       */
   /* There is a stupid initialization problem within Kerberos.dll */
   /* which causes problems if a new process tries to import a     */
   /* security context without ever having previously acquired     */
   /* Kerberos SSP credentials.  I reported this bug originally for*/
   /* W2K beta3 and it is definitely fixed in W2K final, but       */
   /* it seems that the same or an error to the same effect was    */
   /* reintroduced in Win64 and one of the Win2K service packs.    */
   /* The workaround that seems to ensure proper kerberos.dll      */
   /* initialization is to call AcquireCredentialsHandle() once    */
   /* in a processes lifetime before the first call to             */
   /* ImportSecurityContext() -- which is what I do here           */
   /****************************************************************/
   if ( cred_bug_flag==0 ) {
      void       * tmpcred = NULL;
      time_t       expires_at;
      OM_uint32    min_stat;

      DEBUG_ACTION((tf, "  A: %s(): Calling AcquireCred() to workaround an SSPI initialization Bug\n",
			this_Call));
      krb5_acquire_cred( &min_stat, NULL, 0, GN_CRED_INITIATE,
		         &tmpcred, &expires_at );
      cred_bug_flag = 1;

      if (tmpcred!=NULL) {
	  krb5_release_cred( &min_stat, &tmpcred );
      }

   } /* endif (cred_bug_flag==0) */

   /* initial sanity checks on supplied interprocess context token */
   if ( p_ctx_buffer==NULL || p_ctx_buffer_len==0 ) {
      DEBUG_ERR((tf, "ERR: %s(): missing interprocess token!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }
   
   if ( p_ctx_buffer_len<sizeof(impctx) ) {
      DEBUG_ERR((tf, "ERR: %s(): invalid or truncated interprocess token!\n",
		     this_Call ))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );
   }

   /* copy the header into an aligned structure for convenience */
   memcpy( &impctx, p_ctx_buffer, sizeof(impctx) );

   buffer_len = sizeof(impctx) + impctx.initiator_len
		+ impctx.acceptor_len + impctx.sspi_token_len;

   if ( impctx.magic_cookie!=KRB5_CTX_COOKIE
	|| buffer_len != impctx.token_len
	|| buffer_len != p_ctx_buffer_len ) {
      DEBUG_ERR((tf, "ERR: %s(): invalid or corrupted token?\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );
   }

   ctx = sy_calloc( sizeof(*ctx) ); /* initialized with zeroes */
   if (ctx==NULL) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   ctx->magic_cookie	   = KRB5_CTX_COOKIE;
   SecInvalidateHandle( &(ctx->sspi_ctx) );
   ctx->sspi_asc_req	   = impctx.sspi_asc_req;
   ctx->sspi_asc_ret	   = impctx.sspi_asc_ret;
   ctx->sspi_isc_req	   = impctx.sspi_isc_req;
   ctx->sspi_isc_ret	   = impctx.sspi_isc_ret;
   ctx->seq_ini2acc	   = impctx.seq_ini2acc;
   ctx->seq_acc2ini	   = impctx.seq_acc2ini;
   ctx->service_rec        = impctx.service_rec;
   ctx->sizes		   = impctx.sizes;           /* struct copy */
   ctx->sspi_expiration    = impctx.sspi_expiration; /* struct copy */
   ctx->expires_at	   = impctx.expires_at;
   ctx->nego_step	   = (int)impctx.nego_step;
   ctx->role		   = (int)impctx.role;
   ctx->flag_established   = TRUE; /* implicit */

   ptr = &(p_ctx_buffer[sizeof(impctx)]);
   maj_stat = krb5_copy_name( pp_min_stat,
			      ptr, (size_t)impctx.initiator_len,
			      &(ctx->initiator), &(ctx->initiator_len) );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   ptr += impctx.initiator_len;
   maj_stat = krb5_copy_name( pp_min_stat,
			      ptr, (size_t)impctx.acceptor_len,
			      &(ctx->acceptor), &(ctx->acceptor_len) );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   ptr += impctx.acceptor_len;

   /******************************************************************/
   /* Workaround for yet another W2K/Win64 Kerberos SSP BUG:         */
   /* If a mis-aligned context token is handed to the SSPI Function  */
   /* ImportSecurityContext() of Kerberos SSP on Win64 it results    */
   /* in a call for DrWatson within Kerberos.dll due to an unaligned */
   /* access violation ...                                           */
   /******************************************************************/
   alignedbuf_len = impctx.sspi_token_len;
   alignedbuf     = sy_malloc( alignedbuf_len );
   if ( alignedbuf==NULL ) {
       ERROR_RETURN(MINOR_OUT_OF_MEMORY, GSS_S_FAILURE);
   }
   memcpy(alignedbuf, ptr, alignedbuf_len);

   SecBuf.BufferType      = SECBUFFER_TOKEN;
   SecBuf.cbBuffer        = (ULONG)alignedbuf_len;
   SecBuf.pvBuffer        = alignedbuf;

   SecStatus = (fp_SSPI_krb5->ImportSecurityContext)(
      /* PSECURITY_STRING     pszPackage     */  krb5_provider,
      /* PSecBuffer           pPackedContext */  &SecBuf,
      /* void SEC_FAR       * Token          */  NULL,
      /* PCtxtHandle          phContext      */  &(ctx->sspi_ctx) );

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "ERR: %s(): ImportSecurityContext() failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
      ERROR_RETURN( KRB5_MINOR_SSPI(ImportSecurityContext,SecStatus), GSS_S_FAILURE );
   } else {
      DEBUG_ACTION((tf, "  A: %s(): ImportSecurityContext(token_len=%lu)==SEC_E_OK\n",
			this_Call, (unsigned long)alignedbuf_len ));
   }

   if (maj_stat==GSS_S_COMPLETE) {

      (*pp_ctx) = ctx;

   } else {
error:
      if ( ctx!=NULL ) {
	 (void) krb5_delete_sec_context( &min_stat, (void **)&ctx );
      }
   }

   sy_clear_free( &alignedbuf, alignedbuf_len );

   return(maj_stat);

} /* krb5_import_sec_context() */




/*
 * krb5_context_time()
 *
 *
 */
OM_uint32
krb5_context_time( OM_uint32   * pp_min_stat,
		   void	       * p_ctx,
		   time_t      * pp_expires_at )
{
   char	                   * this_Call  = "krb5_context_time";
   krb5_ctx_desc           * ctx        = NULL;
   SecPkgContext_Lifespan    LifeSpan;
   OM_uint32                 maj_stat   = GSS_S_COMPLETE;
   SECURITY_STATUS	     SecStatus;

   (*pp_min_stat) = MINOR_NO_ERROR;
   memset( &LifeSpan, 0, sizeof(LifeSpan) );

   (*pp_expires_at) = 0;

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, &p_ctx,
			      CTX_TIME, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (krb5_ctx_desc *) p_ctx;

   if ( FALSE!=krb5_ctx_noexpire ) {

	  /* ForceCtxNoExpire registry tweak active */
	  ctx->expires_at  = GSS_C_INDEFINITE;
	  (*pp_expires_at) = GSS_C_INDEFINITE;

   } else {

	  SecStatus = (fp_SSPI_krb5->QueryContextAttributes)( &(ctx->sspi_ctx),
							  SECPKG_ATTR_LIFESPAN,
								  &LifeSpan );
	  if ( SEC_E_OK!=SecStatus ) {
		  char *symbol, *desc;

		  krb5_sec_error( SecStatus, &symbol, &desc );
		  DEBUG_ERR((tf, "ERR: %s(): QueryContextAttributes() failed with %s\n\t(Desc=\"%s\")\n",
				this_Call, symbol, desc ))
		  RETURN_MIN_MAJ( KRB5_MINOR_SSPI(QueryContextAttributesLIFESPAN,SecStatus), GSS_S_FAILURE );

	  } else {

		  DEBUG_ACTION((tf, "  A: %s(): QueryContextAttributes(LIFESPAN) == (0x%08lx,0x%08lx)\n",
				this_Call,
				(unsigned long)LifeSpan.tsExpiry.HighPart,
				(unsigned long)LifeSpan.tsExpiry.LowPart ));
		  ctx->sspi_expiration.LowPart  = LifeSpan.tsExpiry.LowPart;
		  ctx->sspi_expiration.HighPart = LifeSpan.tsExpiry.HighPart;
		  ctx->expires_at = krb5_timestamp2time( &(ctx->sspi_expiration) );

		  (*pp_expires_at) = ctx->expires_at;

	  }

   }

   return(maj_stat);

} /* krb5_context_time() */



/*
 * krb5_context_name()
 *
 *
 */
OM_uint32
krb5_context_name( OM_uint32    * pp_min_stat,
		   void         * p_ctx,
		   int		  p_usage,
		   gn_nt_tag_et * pp_nt_tag,
		   void        ** pp_cname,
		   size_t	* pp_cname_len )
{
   char           * this_Call   = "krb5_context_name";
   krb5_ctx_desc  * ctx         = NULL;
   OM_uint32        maj_stat    = GSS_S_COMPLETE;

   (*pp_min_stat)  = MINOR_NO_ERROR;
   (*pp_nt_tag)    = NT_INVALID_TAG;
   (*pp_cname)     = NULL;
   (*pp_cname_len) = 0;

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, &p_ctx,
			      CTX_INQUIRE, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (krb5_ctx_desc *) p_ctx;

   if ( ctx->flag_established==FALSE ) {
      if ( (ctx->role==KRB5_INITIATOR && p_usage==GSS_C_INITIATE)
	   ||  (ctx->role==KRB5_ACCEPTOR && p_usage==GSS_C_ACCEPT) ) {
	 ;
      } else {
	 DEBUG_ERR((tf, "ERR: %s(): %s not available for proto-context!\n",
	    this_Call, (p_usage==GSS_C_INITIATE) ? "initiator" : "acceptor" ))
	 ERROR_RETURN( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
   }

   /* Actually I'd really like to inquire the name of the acceptor  */
   /* and the initiator from the SSPI context, but                  */
   /* SSPI doesn't store them consistently on both ends ...         */
   switch( p_usage ) {

      case GSS_C_ACCEPT:
	       (*pp_cname)     = ctx->acceptor;
	       (*pp_cname_len) = ctx->acceptor_len;
	       break;

      case GSS_C_INITIATE:
	       (*pp_cname)     = ctx->initiator;
	       (*pp_cname_len) = ctx->initiator_len;
	       break;

      default:
	       ERROR_RETURN( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );

   }

   (*pp_nt_tag)    = krb5_nt_tag;

error:
   return(maj_stat);

} /* krb5_context_name() */



/******************************************************************************/
/* Conversion routines from GSSAPI service flags to context attributes of     */
/* Microsoft's SSPI.  Unfortunately, SSPI uses different flags for every      */
/* situation -- not only different in name, but also different in values.     */
/* And on top of that, there is even another set of constants for             */
/* QueryPackageInfo().  This makes about 5 sets of differing constants with   */
/* similar names to describe equivalent attributes ...			      */
/*									      */
/* NOTE: don't take the SSPI documentation literally			      */
/* Although it's nice to read, all of the various NTLM SSP implementations    */
/* in the various ServicePacks of Windows 95 and across available versions    */
/* and service packs of NT differ considerably in non-compliance to this spec */
/* as well as missing of mandatory functionality ...                          */
/*									      */
/* For example the ASC_REQ_* and ISC_REQ_* input parameters are not context   */
/* attribute requirements, but *requests*.  That means with the exception of  */
/* ALLOCATE_MEMORY, the security context will be established, no matter what  */
/* attributes you ask for or receive.					      */
/******************************************************************************/

/*
 * krb5_gss_svc_to_ASC_REQ()
 *
 * GSS-API doesn't allow to specify flags for gss_accept_sec_context()
 */
ULONG
krb5_gss_svc_to_ASC_REQ( void )
{
   ULONG   CtxFlagsReq = 0;

   /* For GSS-API, the message protection services are always requested */
   CtxFlagsReq |= ASC_REQ_CONFIDENTIALITY;
   CtxFlagsReq |= ASC_REQ_INTEGRITY;
   CtxFlagsReq |= ASC_REQ_DELEGATE;

#if 0
   /* SSPI doc says that one should request either SEQUENCE_DETECT or REPLAY_DETECT */
   /* to be able to use message protection calls later on (Sign/Verify).            */
   /* This sounds somewhat broken ...						    */
   CtxFlagsReq |= ASC_REQ_REPLAY_DETECT;
#endif

   CtxFlagsReq |= ASC_REQ_ALLOCATE_MEMORY; /* this affects the context tokens from AcceptSecurityContext() */

   /* CtxFlagsReq |= ASC_REQ_MUTUAL_AUTH;  Let the client decide whether to do mutual authentication */
   /* CtxFlagsReq |= ASC_REQ_IDENTIFY;     Let the client decide whether this is to be permitted */

   return(CtxFlagsReq);

} /* krb5_gss_svc_to_ASC_REQ() */



/*
 * krb5_ASC_RET_to_gss_svc()
 *
 * convert ASC_RET_* flags into GSS_API service flags
 */
OM_uint32
krb5_ASC_RET_to_gss_svc( ULONG CtxFlagsRet )
{
   OM_uint32  svc_flags = 0;

   if ( (CtxFlagsRet&ASC_RET_DELEGATE)!=0 )        { svc_flags |= GSS_C_DELEG_FLAG;    }
   if ( (CtxFlagsRet&ASC_RET_MUTUAL_AUTH)!=0 )     { svc_flags |= GSS_C_MUTUAL_FLAG;   }
   if ( (CtxFlagsRet&ASC_RET_REPLAY_DETECT)!=0 )   { svc_flags |= GSS_C_REPLAY_FLAG;   }
   if ( (CtxFlagsRet&ASC_RET_SEQUENCE_DETECT)!=0 ) { svc_flags |= GSS_C_SEQUENCE_FLAG; }

   if ( (CtxFlagsRet&ASC_RET_INTEGRITY)!=0 )       { svc_flags |= GSS_C_INTEG_FLAG;    }
   /* in GSS-API, confidentiality requires integrity services */
   if ( (CtxFlagsRet&ASC_RET_CONFIDENTIALITY)!=0
        && (svc_flags&GSS_C_INTEG_FLAG)!=0 )       { svc_flags |= GSS_C_CONF_FLAG;     }

#if 0 /* disabled for now */
   svc_flags |= GSS_C_PROT_READY_FLAG;
#endif

   svc_flags |= GSS_C_TRANS_FLAG;  /* indicate that export/import of security contexts is supported */

   return( svc_flags );

} /* krb5_ASC_RET_to_gss_svc() */



/*
 * krb5_gss_svc_to_ISC_REQ()
 *
 * convert GSS-API service flags into ISC_REQ_* flags for InitializeSecurityContext()
 */
ULONG
krb5_gss_svc_to_ISC_REQ( OM_uint32 p_svc_flags )
{
   ULONG   CtxFlagsReq = 0;

   if ( (p_svc_flags&GSS_C_DELEG_FLAG)!=0 )    { CtxFlagsReq |= ISC_REQ_DELEGATE;        }
   if ( (p_svc_flags&GSS_C_MUTUAL_FLAG)!=0 )   { CtxFlagsReq |= ISC_REQ_MUTUAL_AUTH;     }
   if ( (p_svc_flags&GSS_C_REPLAY_FLAG)!=0 )   { CtxFlagsReq |= ISC_REQ_REPLAY_DETECT;   }
   if ( (p_svc_flags&GSS_C_SEQUENCE_FLAG)!=0 ) { CtxFlagsReq |= ISC_REQ_SEQUENCE_DETECT; }

   /* For GSS-API, the message protection services are always requested */
   CtxFlagsReq |= ISC_REQ_INTEGRITY;
   CtxFlagsReq |= ISC_REQ_CONFIDENTIALITY;

#if 0
   if ( (p_svc_flags&GSS_C_DELEG_FLAG)==0 ) {
      CtxFlagsReq |= ISC_REQ_IDENTIFY; /* According to the GSS-API spec, we must perform     */
                                       /* delegation ONLY when it was explicitly requested   */
				       /* SSPI Impersonation level is a delegation, because  */
				       /* it may create a significantly more powerful access */
				       /* token than that of the original server process     */
   }
#endif

   CtxFlagsReq |= ISC_REQ_ALLOCATE_MEMORY;  /* This affects context tokens from InitializeSecurityContext() */

   return( CtxFlagsReq );

} /* krb5_gss_svc_to_ISC_REQ() */



/*
 * krb5_ISC_RET_to_gss_svc()
 *
 *
 */
OM_uint32
krb5_ISC_RET_to_gss_svc( ULONG  CtxAttr )
{
   OM_uint32  svc_flags = 0;

   if ( (CtxAttr&ISC_RET_DELEGATE)!=0 )        { svc_flags |= GSS_C_DELEG_FLAG;    }
   if ( (CtxAttr&ISC_RET_MUTUAL_AUTH)!=0 )     { svc_flags |= GSS_C_MUTUAL_FLAG;   }
   if ( (CtxAttr&ISC_RET_REPLAY_DETECT)!=0 )   { svc_flags |= GSS_C_REPLAY_FLAG;   }
   if ( (CtxAttr&ISC_RET_SEQUENCE_DETECT)!=0 ) { svc_flags |= GSS_C_SEQUENCE_FLAG; }

   if ( (CtxAttr&ISC_RET_INTEGRITY)!=0 )       { svc_flags |= GSS_C_INTEG_FLAG;    }
   /* in GSS-API, confidentiality requires integrity services */
   if ( (CtxAttr&ISC_RET_CONFIDENTIALITY)!=0
        && (svc_flags&GSS_C_INTEG_FLAG)!=0 )   { svc_flags |= GSS_C_CONF_FLAG;     }

#if 0  /* disabled for now */
   svc_flags |= GSS_C_PROT_READY_FLAG;
#endif

   svc_flags |= GSS_C_TRANS_FLAG;  /* indicate that export/import of security contexts is supported */

   return(svc_flags);

} /* krb5_ISC_RET_to_gss_svc( ) */



/*
 * krb5_free_sspi_buffer()
 *
 *
 */
SECURITY_STATUS
krb5_free_sspi_buffer( char * p_buffer_name, void ** pp_sspi_buffer )
{
   SECURITY_STATUS   SecStatus = SEC_E_OK;
   char              * symbol;
   char              * desc;

   if ( (*pp_sspi_buffer)!=NULL ) {
      SecStatus = (fp_SSPI_krb5->FreeContextBuffer)( (*pp_sspi_buffer) );
      if ( SecStatus!=SEC_E_OK) {
	 krb5_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "ERR: FreeContextBuffer(%s) failed with %s\n\t(Desc=\"%s\")\n",
			p_buffer_name, symbol, desc ))
      }
      (*pp_sspi_buffer) = NULL;
   }

   return(SecStatus);

} /* krb5_free_sspi_buffer() */
