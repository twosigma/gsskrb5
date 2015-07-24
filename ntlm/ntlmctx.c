#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/ntlm/ntlmctx.c#8 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/ntlm/ntlmctx.c#8 $
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


#include "ntlmdefs.h"


ULONG	   ntlm_gss_svc_to_ASC_REQ( void		    );
ULONG      ntlm_gss_svc_to_ISC_REQ( OM_uint32  p_svc_flags  );

OM_uint32  ntlm_ASC_RET_to_gss_svc( ULONG      CtxFlagsReq  );
OM_uint32  ntlm_ISC_RET_to_gss_svc( ULONG      CtxAttr	    );




/*
 * ntlm_check_ctx()
 *
 *
 */
OM_uint32
ntlm_check_ctx( OM_uint32       *  pp_min_stat,
	        void            ** pp_ctx,
		int                p_flags,
		char            *  this_Call )
{
   ntlm_ctx_desc   * ctx;

   (*pp_min_stat) = MINOR_NO_ERROR;

   if ( pp_ctx==NULL || *pp_ctx==NULL ) {
      if ( (p_flags&CTX_ESTABLISH)!=0  &&  pp_ctx!=NULL )
	 return(GSS_S_COMPLETE);
      DEBUG_ERR((tf, "Internal ERROR: %s(): NULL priv_ctx handle!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   ctx = (ntlm_ctx_desc *) *pp_ctx;

   if ( ctx->magic_cookie!=NTLM_CTX_COOKIE ) {
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

} /* ntlm_check_ctx() */




/*
 * release_token()
 *
 *
 */
OM_uint32
ntlm_release_token( OM_uint32   * pp_min_stat,
		    Uchar      ** pp_token,
		    size_t      * pp_token_len )
{
   size_t       len = ntlm_maxtoken + NTLM_TOKENDATA_START;

   (*pp_min_stat) = 0;

   if ( *pp_token!=NULL ) {
      sy_clear_free( (void **)pp_token, (*pp_token_len) );
      (*pp_token_len) = 0;
   }

   return(GSS_S_COMPLETE);

} /* release_token() */




/*
 * ntlm_alloc_ctx_token()
 *
 *
 */
OM_uint32
ntlm_alloc_ctx_token( OM_uint32   * pp_min_stat,
		      Uchar	    p_nego_step,
		      Uchar      ** pp_token,
		      size_t      * pp_token_size )
{
   size_t       len = ntlm_maxtoken + NTLM_TOKENDATA_START;

   (*pp_min_stat)   = 0;
   (*pp_token)      = NULL;
   (*pp_token_size) = 0;

   (*pp_token) = sy_calloc( len );
   if (*pp_token==NULL) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   (*pp_token_size)		     = len;
   (*pp_token)[NTLM_TOKEN_TYPE]      = NTLM_CTX_TOKEN;
   (*pp_token)[NTLM_TOKEN_DIRECTION] = p_nego_step;

   return(GSS_S_COMPLETE);

} /* ntlm_alloc_ctx_token() */




/*
 * ntlm_init_sec_context()
 *
 *
 */
OM_uint32
ntlm_init_sec_context(  OM_uint32	 * pp_min_stat,	    /* in */
			void		 * p_cred,	    /* in */
			gss_channel_bindings_t p_ch_bind,   /* in */
			Uchar            * p_target,        /* in */
			size_t             p_target_len,    /* in */
			Uchar            * p_in_token,      /* in */
			size_t             p_in_token_len,  /* in */
			OM_uint32          p_service_req,   /* in */
			void            ** pp_ctx,          /* out */
			Uchar           ** pp_out_token,    /* out */
			size_t           * pp_out_token_len,/* out */
			OM_uint32	 * pp_service_rec,  /* out */
			time_t		 * pp_expires_at )  /* out */
{
   char            * this_Call   = "ntlm_init_sec_context";
   char            * symbol, * desc;
   ntlm_ctx_desc   * ctx         = NULL;
   CtxtHandle      * pCtxt       = NULL;
   SecBufferDesc     OutBufDesc;
   SecBufferDesc     InBufDesc;
   SecBufferDesc   * pInBufDesc  = NULL;
   SecBuffer         OutSecToken;
   SecBuffer	     InSecToken;
   ntlm_cred_desc  * cred        = NULL;
   SECURITY_STATUS   SecStatus;
   ULONG	     CtxFlagsReq = 0;
   OM_uint32         maj_stat    = GSS_S_COMPLETE;

   UNREFERENCED_PARAMETER(p_ch_bind);

   (*pp_min_stat)      = 0;
   (*pp_out_token)     = NULL;
   (*pp_out_token_len) = 0;
   (*pp_service_rec)   = 0;
   (*pp_expires_at)    = 0;

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, pp_ctx,
			      CTX_ESTABLISH, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (ntlm_ctx_desc *) (*pp_ctx);

   /* quick validation of ntlm credential handle */
   maj_stat = ntlm_check_cred( pp_min_stat, &p_cred, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   cred = (ntlm_cred_desc *) p_cred;

   /* GSS-API v1 unconditionally requires a target name ...	*/
   /*							        */
   /* I asked for this requirement to be dropped in GSS-API v2, */
   /* so that a GSS_C_EMPTY_BUFFER would be allowed for a	*/
   /* target-only authentication (which is what NTLM provides)  */
   /* This also changed a requirement in gss_inquire_context()  */
   /* about the availability of the context acceptor's name.    */
   /*								*/
   /* However gssapi mechanism such as Kerberos will still      */
   /* require the target name (so that the ticketing works)     */
   /*                                                           */
   /* MISSING: I need to implemented the ability to go without  */
   /*          the target name.				        */
   if ( p_target==NULL || p_target_len==0 ) {
      DEBUG_ERR((tf, "  E: %s(): missing target name!\n", this_Call))
      RETURN_MIN_MAJ(MINOR_INTERNAL_ERROR, GSS_S_FAILURE);
   }

   /*****************************************************************/
   /* Convert GSS-API context-level service flags into SSPI context */
   /* attribute request flags (ISC_REQ_*) for InitializeSecurity()  */
   /*****************************************************************/

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
      ctx->magic_cookie     = NTLM_CTX_COOKIE;
      ctx->flag_established = FALSE;
      ctx->role             = NTLM_INITIATOR;
      ctx->nego_step        = NTLM_1ST_TOKEN;
      SecInvalidateHandle( &(ctx->sspi_ctx) );
      ctx->sspi_isc_req     = ntlm_gss_svc_to_ISC_REQ( p_service_req );
      ctx->seq_ini2acc	    = 1; /* initiator->acceptor messages have odd numbers  */
      ctx->seq_acc2ini      = 2; /* acceptor->initiator messages have even numbers */
      pCtxt                 = NULL;

      /* NTLM is a simple authentication scheme and does client   */
      /* authentication only.  So the target name will be a       */
      /* purely artificial attribute retained in this wrapper ... */
      /*							  */
      /* NOTICE:  p_target is always a BINARY CANONICAL NAME      */
      /*          in the mechanism-internal format (no framing)   */
      /*          (don't let this code mislead you)               */
      maj_stat = ntlm_copy_name( pp_min_stat,
				 p_target, p_target_len,
				 &(ctx->acceptor), &(ctx->acceptor_len) );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

      /* And then we will also copy our name (the initiator) into  */
      /* our context attribute structure for future reference      */
      maj_stat = ntlm_copy_name( pp_min_stat,
				 cred->name, cred->name_len,
				 &(ctx->initiator), &(ctx->initiator_len) );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

      /* no input token for initial call to InitializeSecurityContext() */
      pInBufDesc          = NULL;

   } else {

      /* There is a prior priv_ctx security context handle ==> use it */
      if (ctx->nego_step!=NTLM_3RD_TOKEN) {
	 /* since NTLM doesn't error as it should, we'll do it here  */
	 /* ... but actually, the generic layer should not call this */
	 /* routine once we flagged "context established"	     */
	 DEBUG_ERR((tf, "  E: %s(): there is no %s nego step!\n",
			this_Call ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

      if ( p_in_token==NULL || p_in_token_len<3 ) {
	 DEBUG_ERR((tf, "  E: %s(): input token missing for continuation call!\n",
			this_Call))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

      if ( p_in_token[NTLM_TOKEN_TYPE]!=NTLM_CTX_TOKEN ) {
	 DEBUG_ERR((tf, "  E: %s(): input is not a context level token!\n",
	                this_Call))
	 RETURN_MIN_MAJ( MINOR_WRONG_TOKEN, GSS_S_DEFECTIVE_TOKEN );
      }

      if ( p_in_token[NTLM_TOKEN_DIRECTION]!=(ctx->nego_step - 1) ) {
	 DEBUG_ERR((tf, "  E: %s(): invalid sequence of context level token!\n",
			this_Call))
	 RETURN_MIN_MAJ( MINOR_WRONG_TOKEN, GSS_S_DEFECTIVE_TOKEN );
      }

      ctx->nego_step = NTLM_3RD_TOKEN;
	     /* a single continuation call to gss_init_sec_context */
	     /* will return the final context establishment token  */
      pCtxt      = &(ctx->sspi_ctx);

      /* There must be an input token for every continuation initial call to InitializeSecurityContext() */
      InSecToken.pvBuffer   = &(p_in_token[NTLM_TOKENDATA_START]);
      InSecToken.cbBuffer   =  (ULONG)(p_in_token_len - NTLM_TOKENDATA_START);
      InSecToken.BufferType = SECBUFFER_TOKEN | SECBUFFER_READONLY;

      InBufDesc.cBuffers    = 1u;
      InBufDesc.pBuffers    = &InSecToken;
      InBufDesc.ulVersion   = SECBUFFER_VERSION;

      pInBufDesc            = &InBufDesc;

   }

   maj_stat = ntlm_alloc_ctx_token( pp_min_stat, (Uchar)ctx->nego_step,
				    pp_out_token, pp_out_token_len );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   OutBufDesc.cBuffers    = 1;
   OutBufDesc.pBuffers    = &OutSecToken;
   OutBufDesc.ulVersion   = SECBUFFER_VERSION;

   OutSecToken.BufferType = SECBUFFER_TOKEN;
   OutSecToken.cbBuffer   = ntlm_maxtoken;
   OutSecToken.pvBuffer   = &((*pp_out_token)[NTLM_TOKENDATA_START]);

   SecStatus = (fp_SSPI_ntlm
	    ->InitializeSecurityContext)(
     /* PCredHandle    phCredential  */   &(cred->sspi_cred),
     /* PCtxtHandle    phContext     */   pCtxt,
     /* SEC_CHAR     * pszTargetName */	  NULL, /* MISSING: target name */
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
      DEBUG_ACTION((tf, "  A: %s(): InitializeSecurityContext #%d()==%s\n",
			this_Call, (int)(ctx->nego_step/2)+1,
			(SecStatus==SEC_E_OK) ? "SEC_E_OK" : "CONTINUE_NEEDED" ));
   }

   if ( SecStatus==SEC_E_OK ) {

      SecStatus = (fp_SSPI_ntlm
	     ->QueryContextAttributes)(
	/* PCtxtHandle   phContext   */    &(ctx->sspi_ctx),
        /* ULONG         ulAttribute */    SECPKG_ATTR_SIZES,
	/* PVOID         pBuffer     */    &(ctx->sizes) );

      if ( SecStatus!=SEC_E_OK ) {
	 ntlm_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "  E: %s(): QueryContextAttributes(SIZES) failed with %s\n(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
	 ERROR_RETURN( NTLM_MINOR_SSPI(QueryContextAttributesSIZES,SecStatus), GSS_S_FAILURE );
      }

      (ctx->flag_established) = TRUE;

#ifndef NTLM_CORRECT_CONTEXT_EXPIRATION
      /* NTLM within Windows NT 4.0 fails to set ptsExpiry correctly!  */
      /* We'll have to fake it ...				       */
      /* The funny thing: They indicate a lifetime of 2 Minutes, which */
      /* is actually their timeout on the challenge-response exchange  */
      /* however they forget to update this lifetime at the final      */
      /* context establishment call.				       */
      ctx->sspi_expiration.LowPart  = 0xffffffffu;
      ctx->sspi_expiration.HighPart = 0x7fffffffu;
#endif

   } else if ( SecStatus==SEC_I_CONTINUE_NEEDED ) {

      maj_stat = GSS_S_CONTINUE_NEEDED;
      if ( OutSecToken.cbBuffer==0 ) {
	 DEBUG_ERR((tf, "  E: %s(): CONTINUE_NEEDED but no token?!\n",
		        this_Call))
	 ERROR_RETURN( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
      
   } else if ( SecStatus!=SEC_E_OK ) {

      ntlm_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "  E: %s(): InitializeSecurityContext() failed with %s\n(Desc=\"%s\")\n",
		     this_Call, symbol, desc));
      if ( NTLM_1ST_TOKEN==ctx->nego_step ) {
	  ERROR_RETURN( NTLM_MINOR_SSPI(InitializeSecurityContext1,SecStatus), GSS_S_FAILURE );
      }
      ERROR_RETURN( NTLM_MINOR_SSPI(InitializeSecurityContextN,SecStatus), GSS_S_FAILURE );
   }


   if ( maj_stat==GSS_S_COMPLETE
        ||  maj_stat==GSS_S_CONTINUE_NEEDED ) {

      (*pp_ctx)           = ctx;
      (ctx->expires_at)   = ntlm_timestamp2time( &(ctx->sspi_expiration) );
      (*pp_expires_at)    = (ctx->expires_at);

      (*pp_out_token_len) = OutSecToken.cbBuffer + NTLM_TOKENDATA_START;
      if (OutSecToken.cbBuffer==0) {
	 OM_uint32  min_stat;

	 (*pp_out_token_len) = ntlm_maxtoken;
	 ntlm_release_token( &min_stat, pp_out_token, pp_out_token_len );
      }

      (ctx->service_rec) = ntlm_ISC_RET_to_gss_svc( ctx->sspi_isc_ret );
      (*pp_service_rec)  = (ctx->service_rec); 
      (ctx->nego_step)  += 2;
   
   } else {
      OM_uint32  min_stat;
error:
      /* Release the output_token if there was one allocated */
      ntlm_release_token( &min_stat, pp_out_token, pp_out_token_len );

      if (ctx->nego_step==NTLM_1ST_TOKEN) {
	 /* This is the first negotiation step -- we have to drop */
	 /* everything after error situations in the initial step */
	 ntlm_delete_sec_context( &min_stat, &ctx );
	 (*pp_ctx) = NULL;
      }
   }
      
   return(maj_stat);

} /* ntlm_init_sec_context() */




/*
 * ntlm_accept_sec_context()
 *
 *
 */
OM_uint32
ntlm_accept_sec_context( OM_uint32		 * pp_min_stat,   /* in  */
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
			 OM_uint32		 * pp_service_rec,/* out */
			 time_t			 * pp_expires_at, /* out */
			 gn_cred_t		 * pp_deleg_cred )/* out */
{
   char               * this_Call   = "ntlm_accept_sec_context";
   ntlm_ctx_desc      * ctx         = NULL; /* convenience pointer */
   ntlm_cred_desc     * cred	    = NULL; /* convenience pointer */
   CtxtHandle         * pCtxt       = NULL; /* returnable dynamic SSPI object */
   char		      * symbol, * desc;
   SecBufferDesc        OutBufDesc;	    /* local scrap */
   SecBufferDesc        InBufDesc;	    /* local scrap */
   SecBuffer            OutSecToken;	    /* local scrap */
   SecBuffer	        InSecToken;	    /* local scrap */
   SECURITY_STATUS      SecStatus;
   SecPkgContext_Names  NamesBuffer;        /* temporary dynamic SSPI object */
   ULONG	        CtxFlagsReq = 0;
   OM_uint32            maj_stat    = GSS_S_COMPLETE;

   UNREFERENCED_PARAMETER(p_chanbind);

   NamesBuffer.sUserName = NULL;
   (*pp_min_stat)        = 0;
   (*pp_out_token)       = NULL;
   (*pp_out_token_len)   = 0;
   (*pp_service_rec)     = 0;
   (*pp_expires_at)      = 0;
   (*pp_srcname)         = NULL;
   (*pp_srcname_len)     = 0;
   (*pp_nt_tag)          = NT_INVALID_TAG;
   if ( pp_deleg_cred )  { (*pp_deleg_cred)      = NULL; }

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, pp_ctx,
			      CTX_ESTABLISH, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (ntlm_ctx_desc *) (*pp_ctx);

   /* quick validation of ntlm credential handle */
   maj_stat = ntlm_check_cred( pp_min_stat, &p_cred, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   cred = (ntlm_cred_desc *) p_cred;

   if ( p_in_token==NULL || p_in_token_len<3 ) {
      DEBUG_ERR((tf, "  E: %s(): input token missing!\n",
		     this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   if ( p_in_token[NTLM_TOKEN_TYPE]!=NTLM_CTX_TOKEN ) {
      DEBUG_ERR((tf, "  E: %s(): input is not a context level token!\n",
		     this_Call))
      RETURN_MIN_MAJ( MINOR_WRONG_TOKEN, GSS_S_DEFECTIVE_TOKEN );
   }

   if (ctx==NULL) {
      /* No prior priv_ctx security context handle ==> create one     */
      /* There is no context handle yet, so this is the first step in */
      /* the security context establishment procedure                 */
      ctx = sy_calloc( sizeof(*ctx) );
      if (ctx==NULL) {
	 RETURN_MIN_MAJ(MINOR_OUT_OF_MEMORY, GSS_S_FAILURE);
      }
      ctx->magic_cookie     = NTLM_CTX_COOKIE;
      ctx->flag_established = FALSE;
      ctx->role             = NTLM_ACCEPTOR;
      ctx->nego_step        = NTLM_2ND_TOKEN;
      SecInvalidateHandle( &(ctx->sspi_ctx) );
      ctx->sspi_asc_req	    = ntlm_gss_svc_to_ASC_REQ(); /* get default ASC_REQs */
      ctx->seq_ini2acc	    = 1; /* initiator->acceptor messages are odd numbered */
      ctx->seq_acc2ini      = 2; /* acceptor->initiator message are even numbered */
      pCtxt                 = NULL;

      /* We have to copy our name (the acceptor) from our creds into   */
      /* our context attribute structure for future reference          */
      /* REMEMBER:  this name is in BINARY CANONICAL format            */
      maj_stat = ntlm_copy_name( pp_min_stat,
				 cred->name, cred->name_len,
				 &(ctx->acceptor), &(ctx->acceptor_len) );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

   } else {

      /* There is a prior priv_ctx security context handle ==> use it */
      if ((ctx->nego_step-1)!=NTLM_3RD_TOKEN) {
	 /* since NTLM doesn't error as it should, we'll do it here  */
	 /* ... but actually, the generic layer should not call this */
	 /* routine once we have flagged "context established"	     */
	 DEBUG_ERR((tf, "  E: %s(): there is no %s nego step!\n",
			this_Call ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

      if ( ctx->role != NTLM_ACCEPTOR ) {
	 DEBUG_ERR((tf, "  E: %s(): context not valid for accepting!\n",
			this_Call))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

      /* a single continuation call to gss_accept_sec_context    */
      /* will establish the security context or fail ultimately  */
      pCtxt      = &(ctx->sspi_ctx);

   }

   if ( p_in_token[NTLM_TOKEN_DIRECTION]!=(ctx->nego_step-1) ) {
      DEBUG_ERR((tf, "  E: %s(): invalid sequence of context level token!\n",
		     this_Call))
      RETURN_MIN_MAJ( MINOR_WRONG_TOKEN, GSS_S_DEFECTIVE_TOKEN );
   }

   /* accept_sec_context() must always be called with an input token.             */
   /* Create the correct buffer objects for the SSPI call AcceptSecurityContext() */
   InSecToken.pvBuffer   = &(p_in_token[NTLM_TOKENDATA_START]);
   InSecToken.cbBuffer   =  (ULONG)(p_in_token_len - NTLM_TOKENDATA_START);
   InSecToken.BufferType = SECBUFFER_TOKEN | SECBUFFER_READONLY;

   InBufDesc.cBuffers    = 1u;
   InBufDesc.pBuffers    = &InSecToken;
   InBufDesc.ulVersion   = 0;

   /* accept_sec_context() will at least for the initial call return a context */
   /* level token, so we have to provide an output buffer for it               */
   maj_stat = ntlm_alloc_ctx_token( pp_min_stat, (Uchar)ctx->nego_step,
				    pp_out_token, pp_out_token_len );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   OutSecToken.pvBuffer   = &((*pp_out_token)[NTLM_TOKENDATA_START]);
   OutSecToken.cbBuffer   = ntlm_maxtoken;
   OutSecToken.BufferType = SECBUFFER_TOKEN;

   OutBufDesc.cBuffers    = 1;
   OutBufDesc.pBuffers    = &OutSecToken;
   OutBufDesc.ulVersion   = SECBUFFER_VERSION;

   SecStatus = (fp_SSPI_ntlm
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
      SecStatus = (fp_SSPI_ntlm
	     ->QueryContextAttributes)(
	/* PCtxtHandle   phContext   */    &(ctx->sspi_ctx),
        /* ULONG         ulAttribute */    SECPKG_ATTR_SIZES,
	/* PVOID         pBuffer     */    &(ctx->sizes) );

      if ( SecStatus!=SEC_E_OK ) {
	 ntlm_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "  E: %s(): QueryContextAttributes(SIZES) failed with %s\n(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
	 ERROR_RETURN( NTLM_MINOR_SSPI(QueryContextAttributesSIZES,SecStatus), GSS_S_FAILURE );
      }

#ifdef CORRECT_QUERY_CONTEXT_NAMES

      /* There is a bug in LSA which will result in an invalid name */
      /* being returned from QueryContextAttributes() if a fallback */
      /* to an un-passworded guest account is performed.            */
      /* Unfortunately we CAN NOT see at the API when this happens! */
      SecStatus = (fp_SSPI_ntlm
	     ->QueryContextAttributes)(
	/* PCtxtHandle   phContext   */    &(ctx->sspi_ctx),
        /* ULONG         ulAttribute */    SECPKG_ATTR_NAMES,
	/* PVOID         pBuffer     */    &NamesBuffer );

      if (SecStatus!=SEC_E_OK) {
	 ntlm_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "  E: %s(): QueryContextAttribute(NAMES) failed with %s\n(Desc=\"%s\")\n",
			 this_Call, symbol, desc));

	 ERROR_RETURN( NTLM_MINOR_SSPI(QueryContextAttributesNAMES,SecStatus), GSS_S_FAILURE );
      }

      maj_stat = ntlm_copy_name( pp_min_stat, NamesBuffer.sUserName,
				 strlen(NamesBuffer.sUserName),
				 &(ctx->initiator), &(ctx->initiator_len) );

#else /* !CORRECT_QUERY_CONTEXT_NAMES */

      /* workaround to find out authorized account -- NTLM is broken (<=NTsp3)  */
      /* unfortunately SSPI confuses authentication with authorization ...      */
      /* and their auto-fallback to a guest account will tickle a caching       */
      /* bug in LSA that causes QueryContextAttributes(Names) to return invalid */
      /* information for that case.					        */
      /* This bug also affects LookupAccountSID() during impersonation!!        */

      if ( ntlm_platform_id!=VER_PLATFORM_WIN32_WINDOWS) {

	 /* Workaround to find out the correct peer/client name on Windows NT  */
	 /* even when a fallback to the guest account happens under the covers */

         HANDLE        cur_th; 
	 HANDLE        hdl_token = NULL;
         UCHAR         buf_info[256]; /* need it larger than the next two buffers ... */
         UCHAR         buf_acc[64];
         UCHAR         buf_dmn[64];
         PTOKEN_USER   pTokenUser = (PTOKEN_USER) buf_info;
         DWORD         dwinf_len;
         DWORD         dwacc_len = sizeof(buf_acc)-1;
         DWORD         dwdmn_len = sizeof(buf_dmn)-1;
	 BOOL          ret;
	 BOOL	       impersonating = FALSE;
	 SID_NAME_USE  snu;

	 buf_acc[0] = buf_dmn[0] = 0;
         SecStatus = (fp_SSPI_ntlm->ImpersonateSecurityContext)(&(ctx->sspi_ctx));
         if ( SecStatus!=SEC_E_OK ) {
	    ntlm_sec_error( SecStatus, &symbol, &desc );
	    DEBUG_ERR((tf, "  E: %s(): ImpersonateSecurityContext() failed with %s\n(Desc=\"%s\")\n",
		           this_Call, symbol, desc));

	    goto fallback_query_context;
         }

	 /* Nota bene:  were impersonating here, we must call RevertSecurityContext() before exit !!! */
	 impersonating = TRUE;

         cur_th = GetCurrentThread(); 
	 ret = OpenThreadToken((HANDLE)cur_th, TOKEN_QUERY, FALSE, &hdl_token);
	 if ( ret ) {
	    ret = GetTokenInformation( hdl_token, TokenUser,
			               buf_info, sizeof(buf_info), &dwinf_len);

	    /* Apparently we need to close this handle  ... would someone    */
	    /* please slap Microsoft for *NOT* documenting this for at least */
	    /* 5 years after shipping Windows NT (it is definitely not       */
	    /* documented up to and including MSDN Visual Studio 6.0)	     */
	    CloseHandle(hdl_token);
	    hdl_token = NULL;

	    if ( ret ) {
	       SecStatus = (fp_SSPI_ntlm->RevertSecurityContext)( &(ctx->sspi_ctx));
	       if ( SecStatus!=SEC_E_OK ) {
		  ntlm_sec_error( SecStatus, &symbol, &desc );
		  DEBUG_ERR((tf, "  E: %s(): RevertSecurityContext() failed with %s\n(Desc=\"%s\")\n",
		                 this_Call, symbol, desc));
		  ERROR_RETURN( NTLM_MINOR_SSPI(RevertSecurityContext,SecStatus), GSS_S_FAILURE );
	       }
	       impersonating = FALSE;
	       /* WARNING!!  LookupAccountSid() may break badly while Impersonating due to  */
	       /* due to a bug in LSA (NT4sp3)...  be sure to RevertSecurityContext() first */
	       /* Curiously, it returns the same garbage as QueryContextAttributes(NAMES)   */
	       ret = LookupAccountSid( NULL, pTokenUser->User.Sid,
		                       buf_acc, &dwacc_len,
				       buf_dmn, &dwdmn_len, &snu);
	    }

	 }

	 if ( !ret ) {
	    (*pp_min_stat) = NTLM_MINOR_LASTERROR( GetLastError() );
	    maj_stat       = GSS_S_FAILURE;
	 }

	 if (impersonating) {
	    /* we already have an error, so blindly do the Revert (let's hope the code is correct ...) */
	    SecStatus = (fp_SSPI_ntlm->RevertSecurityContext)( &(ctx->sspi_ctx));
	    if (SecStatus!=SEC_E_OK && maj_stat==GSS_S_COMPLETE) {
	       ntlm_sec_error( SecStatus, &symbol, &desc );
	       DEBUG_ERR((tf, "  E: %s(): RevertSecurityContext() failed with %s\n(Desc=\"%s\")\n",
		              this_Call, symbol, desc));
	       ERROR_RETURN( NTLM_MINOR_SSPI(RevertSecurityContext,SecStatus), GSS_S_FAILURE );
	    }
	    impersonating = FALSE;
	 }

	 if (maj_stat!=GSS_S_COMPLETE)
	    goto fallback_query_context;

	 buf_acc[sizeof(buf_acc)-1] = '\0';   /* always wear your safety-belt */
	 buf_dmn[sizeof(buf_dmn)-1] = '\0';   /* always wear your safety-belt */
         strcpy (buf_info, buf_dmn);
         strcat (buf_info, "\\");
         strcat (buf_info, buf_acc);

         maj_stat = ntlm_copy_name( pp_min_stat, buf_info,
				    strlen(buf_info),
				    &(ctx->initiator), &(ctx->initiator_len) );

      } else { /* workaround to find out authorized account on Windows NT */

	 /* the Name-is-wrong for guest-fallback DOES affect pass-through */
	 /* authentication of Windows'95, but I DON'T have a workaround   */
fallback_query_context:

         SecStatus = (fp_SSPI_ntlm
	       ->QueryContextAttributes)(
	       /* PCtxtHandle   phContext   */    &(ctx->sspi_ctx),
	       /* ULONG         ulAttribute */    SECPKG_ATTR_NAMES,
	       /* PVOID         pBuffer     */    &NamesBuffer );

         if (SecStatus!=SEC_E_OK) {
	       ntlm_sec_error( SecStatus, &symbol, &desc );
	       DEBUG_ERR((tf, "  E: %s(): QueryContextAttribute(NAMES) failed with %s\n(Desc=\"%s\")\n",
		    this_Call, symbol, desc));

	       ERROR_RETURN( NTLM_MINOR_SSPI(QueryContextAttributesNAMES,SecStatus), GSS_S_FAILURE );
	 }

	 maj_stat = ntlm_copy_name( pp_min_stat, NamesBuffer.sUserName,
				    strlen(NamesBuffer.sUserName),
				    &(ctx->initiator), &(ctx->initiator_len) );

      } /* endif  */

#endif /* !CORRECT_QUERY_CONTEXT_NAMES */

      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

      /* NTLM BUG: it doesn't always canonicalize its names */
      CharUpper(ctx->initiator);

      (*pp_srcname)     = ctx->initiator;
      (*pp_srcname_len) = ctx->initiator_len;
      (*pp_nt_tag)      = ntlm_nt_tag;

      ctx->flag_established = TRUE;

   } else if (SecStatus==SEC_I_CONTINUE_NEEDED) {

      maj_stat = GSS_S_CONTINUE_NEEDED;
      if ( OutSecToken.cbBuffer==0 ) {
	 DEBUG_ERR((tf, "  E: %s #%d(): CONTINUE_NEEDED but no token?!\n",
		        this_Call, (int)(ctx->nego_step/2)+1 ))
	 ERROR_RETURN( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
      
   } else if ( SecStatus!=SEC_E_OK ) {

      char *symbol, *desc;
      
      ntlm_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "  E: %s(): AcceptSecurityContext() failed with %s\n(Desc=\"%s\")\n",
			 this_Call, symbol, desc));

      if ( NTLM_2ND_TOKEN==ctx->nego_step ) {
	  ERROR_RETURN( NTLM_MINOR_SSPI(AcceptSecurityContext1,SecStatus), GSS_S_FAILURE );
      }
      ERROR_RETURN( NTLM_MINOR_SSPI(AcceptSecurityContextN,SecStatus), GSS_S_FAILURE );
   }

   if ( maj_stat==GSS_S_COMPLETE
        ||  maj_stat==GSS_S_CONTINUE_NEEDED ) {

#ifndef NTLM_CORRECT_CONTEXT_EXPIRATION
      if ( maj_stat==GSS_S_COMPLETE
	   &&  ntlm_platform_id==VER_PLATFORM_WIN32_WINDOWS
	   &&  ntlm_platform_minor==0 ) {
	 /* NTLM within Windows 95 fails to set ptsExpiry correctly! */
	 /* We'll have to fake it ...				      */
	 ctx->sspi_expiration.LowPart  = 0xffffffffu;
	 ctx->sspi_expiration.HighPart = 0x7fffffffu;
      }
#endif

      (*pp_ctx)           = ctx;
      (ctx->expires_at)   = ntlm_timestamp2time( &(ctx->sspi_expiration) );
      (*pp_expires_at)    = (ctx->expires_at);

      (*pp_out_token_len) = OutSecToken.cbBuffer + NTLM_TOKENDATA_START;
      if (OutSecToken.cbBuffer==0) {
	 OM_uint32  min_stat;

	 (*pp_out_token_len) = ntlm_maxtoken;
	 ntlm_release_token( &min_stat, pp_out_token, pp_out_token_len );
      }

      (ctx->service_rec) = ntlm_ASC_RET_to_gss_svc( ctx->sspi_asc_ret );
      (*pp_service_rec)  = (ctx->service_rec);
      (ctx->nego_step)  += 2;

   } else {

      OM_uint32   min_stat;
error:
      /* Release the output_token if there was one allocated */
      ntlm_release_token( &min_stat, pp_out_token, pp_out_token_len );

      if (ctx->nego_step==NTLM_2ND_TOKEN) {
	 /* This is the first negotiation step -- we have to drop */
	 /* everything after error situations in the initial step */
	 ntlm_delete_sec_context( &min_stat, &ctx );
	 (*pp_ctx) = NULL;
      }

   }

   if ( NamesBuffer.sUserName!=NULL ) {
      SecStatus = (fp_SSPI_ntlm->FreeContextBuffer)( NamesBuffer.sUserName );
      if (SecStatus!=SEC_E_OK) {
	 ntlm_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "  E: %s(): FreeContextBuffer() failed with %s\n(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
      }
      NamesBuffer.sUserName = NULL;
   }

   return(maj_stat);

} /* ntlm_accept_sec_context() */




/*
 * ntlm_delete_sec_context()
 *
 *
 */
OM_uint32
ntlm_delete_sec_context( OM_uint32   * pp_min_stat,
		         void       ** pp_ctx )
{
   char             * this_Call = "ntlm_delete_context";
   ntlm_ctx_desc    * ctx       = NULL;
   SECURITY_STATUS    SecStatus;
   OM_uint32          maj_stat  = GSS_S_COMPLETE;

   (*pp_min_stat) = 0;

   if ( *pp_ctx==NULL ) 
      return(maj_stat);

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, pp_ctx,
			      CTX_DELETE, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = *pp_ctx;

   if ( IsValidSecurityHandle(&(ctx->sspi_ctx)) ) {

      SecStatus = (fp_SSPI_ntlm->DeleteSecurityContext)( &(ctx->sspi_ctx) );
      if ( SecStatus!=SEC_E_OK ) {
	 char *symbol, *desc;
      
	 ntlm_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "  E: %s(): DeleteSecurityContext() failed with %s\n(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
	 (*pp_min_stat) = NTLM_MINOR_SSPI(DeleteSecurityContext,SecStatus); 
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

} /* ntlm_delete_sec_context() */




/*
 * ntlm_export_sec_context()
 *
 *
 */
OM_uint32
ntlm_export_sec_context( OM_uint32    * pp_min_stat,
			 void	     ** pp_ctx,
			 void	     ** pp_ctx_buffer,
			 size_t	      * pp_ctx_buffer_len )
{
   char               * this_Call      = "ntlm_export_sec_context";
   ntlm_ctx_desc      * ctx            = NULL;  /* convenience pointer */
   ntlm_exp_ctx_desc  * expctx	       = NULL;  
   Uchar	      * buffer         = NULL;  /* returnable dynamic object */
   Uchar	      * ptr	       = NULL;  /* convenience pointer */
   Uchar	      * sspi_token     = NULL;  /* convenience pointer */
   SecBuffer	        SecBuf;
   SECURITY_STATUS      SecStatus;
   size_t	        buffer_len     = 0;
   size_t	        sspi_token_len = 0;
   OM_uint32		min_stat;
   OM_uint32	        maj_stat       = GSS_S_COMPLETE;


   (*pp_min_stat)       = MINOR_NO_ERROR;
   (*pp_ctx_buffer)     = NULL;
   (*pp_ctx_buffer_len) = 0;

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, pp_ctx,
			      CTX_TRANSFER, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (ntlm_ctx_desc *) (*pp_ctx);

   /* NTLM in Windows NT 4.0sp3 and Win9x DO NOT have security context import/export */

   /* We are only going to drop the original SSPI security context  */
   /* for now -- actually the final ntlm_delete_sec_context() will  */
   /* dispose it along with all other data that was associated with */
   /* the security context handle				    */

   if ( ctx->service_rec & (GSS_C_INTEG_FLAG|GSS_C_CONF_FLAG) ) {
      /* we cannot provide CONFIDENTIALITY or INTEGRITY without */
      /* security context export/import facilities              */
      DEBUG_ERR((tf, "  E: %s(): NTLM Mechanism lacking export_context()!\n",
		     this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   SecBuf.BufferType  = SECBUFFER_EMPTY;
   SecBuf.cbBuffer    = 0;
   SecBuf.pvBuffer    = NULL;

   if ( ntlm_emulate_ctx_xfer==FALSE ) {

      SecStatus = (fp_SSPI_ntlm->ExportSecurityContext)(
	 /* PCtxtHandle         phContext      */  &(ctx->sspi_ctx),
	 /* ULONG               fFlags         */   SECPKG_CONTEXT_EXPORT_DELETE_OLD,
	 /* PSecBuffer          pPackedContext */  &SecBuf,
	 /* void             ** pToken         */   NULL );

      if ( SecStatus!=SEC_E_OK ) {
	 char *symbol, *desc;
      
	 ntlm_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "  E: %s(): ExportSecurityContext() failed with %s\n(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
	 ERROR_RETURN( NTLM_MINOR_SSPI(ExportSecurityContext,SecStatus), GSS_S_FAILURE );
      }
      SecInvalidateHandle( &(ctx->sspi_ctx) );

   }

   /* the SSPI security context will be dropped by the final       */
   /* call to ntlm_delete_sec_context()                            */
   /* only our shadowed information will survive context transfers */
   sspi_token_len = SecBuf.cbBuffer;
   sspi_token     = SecBuf.pvBuffer;

   buffer_len = sizeof(*expctx)
	        + ctx->acceptor_len + ctx->initiator_len + sspi_token_len;

   if ( buffer_len>USHRT_MAX ) {
       DEBUG_ERR((tf, "  E: %s(): Exported context token to large (%ld bytes) for this code!\n",
		      this_Call, (unsigned long) buffer_len ));
       ERROR_RETURN( MINOR_INTERNAL_BUFFER_OVERRUN, GSS_S_FAILURE );
   }

   buffer = sy_malloc( buffer_len ); /* we're filling everything anyways */
   if (buffer==NULL) {
      ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   /* clear structure body, avoid uninitalized memory copy warnings */
   /* from purify if the struct contains padding at the end         */
   memset(buffer,0,sizeof(*expctx));

   expctx = (ntlm_exp_ctx_desc *) buffer;

   expctx->magic_cookie    =	     NTLM_CTX_COOKIE;
   expctx->sspi_asc_req    =	     ctx->sspi_asc_req;
   expctx->sspi_asc_ret    =	     ctx->sspi_asc_ret;
   expctx->sspi_isc_req    =	     ctx->sspi_isc_req;
   expctx->sspi_isc_ret    =	     ctx->sspi_isc_ret;
   expctx->seq_ini2acc	   =	     ctx->seq_ini2acc;
   expctx->seq_acc2ini	   =	     ctx->seq_acc2ini;
   expctx->service_rec	   =	     ctx->service_rec;
   expctx->sizes	   =	     ctx->sizes;	    /* struct copy */
   expctx->sspi_expiration =	     ctx->sspi_expiration;  /* struct copy */
   expctx->expires_at      =	     ctx->expires_at;
   expctx->token_len       = (Ushort) buffer_len;
   expctx->initiator_len   = (Ushort) ctx->initiator_len;
   expctx->acceptor_len    = (Ushort) ctx->acceptor_len;
   expctx->sspi_token_len  = (Ushort) sspi_token_len;
   expctx->nego_step       = (Uchar)  ctx->nego_step;
   expctx->role		   = (Uchar)  ctx->role;

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

      (void) ntlm_delete_sec_context( &min_stat, pp_ctx );

      (*pp_ctx_buffer)     = buffer;
      (*pp_ctx_buffer_len) = buffer_len;

   } else {
error:
      if ( sspi_token!=NULL ) {
	 /* try to reimport the security context */	 
	 SecStatus = (fp_SSPI_ntlm->ImportSecurityContext)(
	    /* PSECURITY_STRING     pszPackage     */  ntlm_provider,
	    /* PSecBuffer           pPackedContext */  &SecBuf,
	    /* void SEC_FAR       * Token          */  NULL,
	    /* PCtxtHandle          phContext      */  &(ctx->sspi_ctx) );

	 if ( SecStatus!=SEC_E_OK ) {
	    /* Too bad, we cannot even recover the exported security context !         */
	    /* So we do not just fail, but we also delete the damaged security context */
	    char *symbol, *desc;
      
	    ntlm_sec_error( SecStatus, &symbol, &desc );
	    DEBUG_ERR((tf, "  E: %s(): ImportSecurityContext() failed with %s\n(Desc=\"%s\")\n",
			   this_Call, symbol, desc ))
	    ntlm_delete_sec_context( &min_stat, pp_ctx );

	    (*pp_min_stat) = NTLM_MINOR_SSPI(ImportSecurityContext,SecStatus);
	    maj_stat       = GSS_S_FAILURE;
	 }
      }

      sy_clear_free( (void **)&buffer, buffer_len );
      (*pp_ctx_buffer)     = NULL;
      (*pp_ctx_buffer_len) = 0;
   }

   if ( sspi_token!=NULL ) {
      (fp_SSPI_ntlm->FreeContextBuffer)( sspi_token );
      sspi_token     = NULL;
      sspi_token_len = 0;
   }

   return(maj_stat);

} /* ntlm_export_sec_context() */




/*
 * ntlm_import_sec_context()
 *
 *
 */
OM_uint32
ntlm_import_sec_context( OM_uint32   * pp_min_stat,
			 Uchar	     * p_ctx_buffer,
			 size_t	       p_ctx_buffer_len,
			 void	    ** pp_ctx )
{
   char               * this_Call      = "ntlm_import_sec_context";
   ntlm_ctx_desc      * ctx            = NULL;  /* returnable dynamic object */
   ntlm_exp_ctx_desc    impctx;		        /* convenience struct  */	   
   Uchar	      * ptr	       = NULL;  /* convenience pointer */
   SecBuffer            SecBuf;
   SECURITY_STATUS      SecStatus;
   size_t	        buffer_len     = 0;
   OM_uint32		min_stat;
   OM_uint32	        maj_stat       = GSS_S_COMPLETE;

   (*pp_min_stat) = MINOR_NO_ERROR;
   (*pp_ctx)	  = NULL;

   /* initial sanity checks on supplied interprocess context token */
   if ( p_ctx_buffer==NULL || p_ctx_buffer_len==0 ) {
      DEBUG_ERR((tf, "  E: %s(): missing interprocess token!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }
   
   if ( p_ctx_buffer_len<sizeof(impctx) ) {
      DEBUG_ERR((tf, "  E: %s(): invalid or truncated interprocess token!\n",
		     this_Call ))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );
   }

   /* copy the header into an aligned structure for convenience */
   memcpy( &impctx, p_ctx_buffer, sizeof(impctx) );

   buffer_len = sizeof(impctx) + impctx.initiator_len
		+ impctx.acceptor_len + impctx.sspi_token_len;

   if ( impctx.magic_cookie!=NTLM_CTX_COOKIE
	|| buffer_len != impctx.token_len
	|| buffer_len != p_ctx_buffer_len ) {
      DEBUG_ERR((tf, "  E: %s(): invalid or corrupted token?\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );
   }

   ctx = sy_calloc( sizeof(*ctx) ); /* initialized with zeroes */
   if (ctx==NULL) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   ctx->magic_cookie	   = NTLM_CTX_COOKIE;
   SecInvalidateHandle( &(ctx->sspi_ctx) );
   ctx->sspi_asc_req	   = impctx.sspi_asc_req;
   ctx->sspi_asc_ret	   = impctx.sspi_asc_ret;
   ctx->sspi_isc_req	   = impctx.sspi_isc_req;
   ctx->sspi_isc_ret	   = impctx.sspi_isc_ret;
   ctx->seq_ini2acc	   = impctx.seq_ini2acc;
   ctx->seq_acc2ini	   = impctx.seq_acc2ini;
   ctx->service_rec        = impctx.service_rec;
   ctx->sizes		   = impctx.sizes;	     /* struct copy */
   ctx->sspi_expiration    = impctx.sspi_expiration; /* struct copy */
   ctx->expires_at	   = impctx.expires_at;
   ctx->nego_step	   = impctx.nego_step;
   ctx->role		   = (int)impctx.role;
   ctx->flag_established   = TRUE; /* implicit */

   ptr = &(p_ctx_buffer[sizeof(impctx)]);
   maj_stat = ntlm_copy_name( pp_min_stat,
			      ptr, (size_t)impctx.initiator_len,
			      &(ctx->initiator), &(ctx->initiator_len) );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   ptr += impctx.initiator_len;
   maj_stat = ntlm_copy_name( pp_min_stat,
			      ptr, (size_t)impctx.acceptor_len,
			      &(ctx->acceptor), &(ctx->acceptor_len) );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   ptr += impctx.acceptor_len;

   if ( ntlm_emulate_ctx_xfer==FALSE ) {


      SecBuf.BufferType      = SECBUFFER_TOKEN;
      SecBuf.cbBuffer        = impctx.sspi_token_len;
      SecBuf.pvBuffer        = ptr;

      SecStatus = (fp_SSPI_ntlm->ImportSecurityContext)(
	 /* PSECURITY_STRING     pszPackage     */  ntlm_provider,
	 /* PSecBuffer           pPackedContext */  &SecBuf,
	 /* void SEC_FAR       * Token          */  NULL,
	 /* PCtxtHandle          phContext      */  &(ctx->sspi_ctx) );

      if ( SecStatus!=SEC_E_OK ) {
	 char *symbol, *desc;
      
	 ntlm_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "  E: %s(): ImportSecurityContext() failed with %s\n(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
	 ERROR_RETURN( NTLM_MINOR_SSPI(ImportSecurityContext,SecStatus), GSS_S_FAILURE );
      }

   } else { /* (ntlm_emulate_ctx_xfer==TRUE) */

      /* MISSING: importing of SSPI security context	    */
      /* NTLM in Windows NT 4.0sp3 and Win9x don't have it  */
      SecInvalidateHandle( &(ctx->sspi_ctx) );
   
      if ( impctx.sspi_token_len!=0 ) {
	 DEBUG_ERR((tf, "  E: %s(): Huh? where does this token come from?\n",
		       this_Call))
	 RETURN_MIN_MAJ( NTLM_MINOR(UNEXPECTED_SSPI_CTX), GSS_S_DEFECTIVE_TOKEN );
      }

   }

   if (maj_stat==GSS_S_COMPLETE) {

      (*pp_ctx) = ctx;

   } else {
error:
      if ( ctx!=NULL ) {
	 (void) ntlm_delete_sec_context( &min_stat, (void **)&ctx );
      }
   }

   return(maj_stat);

} /* ntlm_import_sec_context() */




/*
 * ntlm_context_time()
 *
 *
 */
OM_uint32
ntlm_context_time( OM_uint32   * pp_min_stat,
		   void	       * p_ctx,
		   time_t      * pp_expires_at )
{
   char	                   * this_Call  = "ntlm_context_time";
   ntlm_ctx_desc           * ctx        = NULL;
   SecPkgContext_Lifespan    LifeSpan;
   OM_uint32                 maj_stat   = GSS_S_COMPLETE;
   SECURITY_STATUS	     SecStatus;

   (*pp_min_stat) = MINOR_NO_ERROR;
   memset( &LifeSpan, 0, sizeof(LifeSpan) );

   (*pp_expires_at) = 0;

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, &p_ctx,
			      CTX_TIME, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (ntlm_ctx_desc *) p_ctx;

#if !defined(HAVE_QUERY_LIFESPAN)
   /* SSPIs QueryContextAttributes(SECPKG_ATTR_LIFESPAN)      */
   /* is not implemented by NTLM in Windows NT 4.0 !          */
   /* Oh well, let's fake it -- for emulated context transfer */
   /* we will have to fake it as well			      */
      UNREFERENCED_PARAMETER(SecStatus);
      (*pp_expires_at) = ctx->expires_at;

#else

   SecStatus = (fp_SSPI_ntlm->QueryContextAttributes)( &(ctx->sspi_ctx),
						  SECPKG_ATTR_LIFESPAN,
					          &LifeSpan );

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      ntlm_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "  E: %s(): QueryContextAttributes() failed with %s\n(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
      RETURN_MIN_MAJ( NTLM_MINOR_SSPI(QueryContextAttributesLIFESPAN,SecStatus), GSS_S_FAILURE );

   } else {

      ctx->sspi_expiration.LowPart  = LifeSpan.tsExpiry.LowPart;
      ctx->sspi_expiration.HighPart = LifeSpan.tsExpiry.HighPart;
      ctx->expires_at = ntlm_timestamp2time( &(ctx->sspi_expiration) );

      (*pp_expires_at) = ctx->expires_at;

   }
#endif

   return(maj_stat);

} /* ntlm_context_time() */



/*
 * ntlm_context_name()
 *
 *
 */
OM_uint32
ntlm_context_name( OM_uint32    * pp_min_stat,
		   void         * p_ctx,
		   int		  p_usage,
		   gn_nt_tag_et * pp_nt_tag,
		   void        ** pp_cname,
		   size_t	* pp_cname_len )
{
   char           * this_Call   = "ntlm_context_name";
   ntlm_ctx_desc  * ctx         = NULL;
   OM_uint32        maj_stat    = GSS_S_COMPLETE;

   (*pp_min_stat)  = MINOR_NO_ERROR;
   (*pp_nt_tag)    = NT_INVALID_TAG;
   (*pp_cname)     = NULL;
   (*pp_cname_len) = 0;

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, &p_ctx,
			      CTX_INQUIRE, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (ntlm_ctx_desc *) p_ctx;

   if ( ctx->flag_established==FALSE ) {
      if ( (ctx->role==NTLM_INITIATOR && p_usage==GSS_C_INITIATE)
	   ||  (ctx->role==NTLM_ACCEPTOR && p_usage==GSS_C_ACCEPT) ) {
	 ;
      } else {
	 DEBUG_ERR((tf, "  E: %s(): %s not available for proto-context!\n",
	    this_Call, (p_usage==GSS_C_INITIATE) ? "initiator" : "acceptor" ))
	 ERROR_RETURN( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
   }

   /* Actually I'd really like to inquire the name of the acceptor         */
   /* and the initiator from the SSPI context, but:                        */
   /*    (1) without context export it must be faked anyway                */
   /*    (2) SSPI only and just sometimes provides the initiators' name... */
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

   (*pp_nt_tag)    = ntlm_nt_tag;

error:
   return(maj_stat);

} /* ntlm_context_name() */



/****************************************************************************/
/* Conversion routines from GSSAPI service flags to context attributes of   */
/* Microsoft's SSPI.  Unfortunately, SSPI uses different flags for every    */
/* situation -- not only different in name, but also different in values.   */
/* And on top of that, there is even another set of constants for           */
/* QueryPackageInfo().  This makes about 5 sets of differing constants with */
/* similar names to describe equivalent attributes ...			    */
/*									    */
/* For example the ASC_REQ_* and ISC_REQ_* input parameters are not context */
/* attribute requirements, but *requests*.  That means with the exception of*/
/* ALLOCATE_MEMORY, the security context will be established, no matter what*/
/* attributes you ask for or receive.					    */
/*									    */
/* Similarly, when accepting a security context on Win9x, if the initial    */
/* context token carries a request for message protection (integrity or     */
/* confidentiality), then AcceptSecurityContext() on Win9x will return the  */
/* Error "SEC_E_UNSUPPORTED_FUNCTION.  This particular error code is clearly*/
/* inappropriate, and it is returned because with pass-through authentication */
/* on Win9x, there the password is not available to provide message protection*/
/* services.								    */
/*									    */
/* NOTE: don't take the SSPI documentation literally			    */
/* Although it's nice to read, all shipping variants of NTLM (Win9x and NT4)*/
/* differ considerably in multiple and sublte details.			    */
/****************************************************************************/

/*
 * ntlm_gss_svc_to_ASC_REQ()
 *
 * GSS-API doesn't allow to specify flags for gss_accept_sec_context()
 */
ULONG
ntlm_gss_svc_to_ASC_REQ( void )
{
   ULONG   CtxFlagsReq = ASC_REQ_CONNECTION;

#if 0
   /* SSPI doc says that one should request either SEQUENCE_DETECT or REPLAY_DETECT */
   /* to be able to use message protection calls later on (Sign/Verify).            */
   /* This sounds somewhat broken ...						    */
   /* However if we do this, AcceptSecurityContext() on Win95 will unconditionally  */
   /* fail with SEC_E_UNSUPPORTED_FUNCTION -- so we don't.			    */
   /* Other drawbacks:  confidentiality is undocumented in SSPI, Integrity of NTLMv1*/
   /* is horribly insecure and message protection is half duplex only ... forget it */
   CtxFlagsReq |= ASC_REQ_REPLAY_DETECT;
   CtxFlagsReq |= ASC_REQ_CONFIDENTIALITY;
   CtxFlagsReq |= ASC_REQ_INTEGRITY;
#endif

   return(CtxFlagsReq);

} /* ntlm_gss_svc_to_ASC_REQ() */



/*
 * ntlm_ASC_RET_to_gss_svc()
 *
 * convert ASC_RET_* flags into GSS_API service flags
 */
OM_uint32
ntlm_ASC_RET_to_gss_svc( ULONG CtxFlagsRet )
{
   OM_uint32  svc_flags = 0;

   if ( (CtxFlagsRet&ASC_RET_DELEGATE)!=0 )        { svc_flags |= GSS_C_DELEG_FLAG;    }
   if ( (CtxFlagsRet&ASC_RET_MUTUAL_AUTH)!=0 )     { svc_flags |= GSS_C_MUTUAL_FLAG;   }
   if ( (CtxFlagsRet&ASC_RET_REPLAY_DETECT)!=0 )   { svc_flags |= GSS_C_REPLAY_FLAG;   }
   if ( (CtxFlagsRet&ASC_RET_SEQUENCE_DETECT)!=0 ) { svc_flags |= GSS_C_SEQUENCE_FLAG; }

#if 0
   /* Message protection is not available for Win95 Acceptors			    */
   /* Other drawbacks:  confidentiality is undocumented in SSPI, Integrity of NTLMv1*/
   /* is horribly insecure and message protection is half duplex only ... forget it */
   if ( (CtxFlagsRet&ASC_RET_INTEGRITY)!=0 )       { svc_flags |= GSS_C_INTEG_FLAG;    }
   if ( (CtxFlagsRet&ASC_RET_CONFIDENTIALITY)!=0
        && (svc_flags&GSS_C_INTEG_FLAG)!=0 )       { svc_flags |= GSS_C_CONF_FLAG;     }

   if ( (CtxFlagsRet&(ASC_RET_SEQUENCE_DETECT|ASC_RET_REPLAY_DETECT)!=0 ) {
      svc_flags |= GSS_C_INTEG_FLAG;
      svc_flags |= GSS_C_CONF_FLAG;
      svc_flags |= GSS_C_REPLAY_FLAG;  /* I don't know if "REPLAY_DETECT" is true ... */
   }
#endif

   svc_flags |= GSS_C_TRANS_FLAG;

   /* Well, since the NTLM SSPI always maintains it own sequence number for messages */
   /* we will indicate GSS_C_SEQUENCE_FLAG whenever this will happen ...             */
   if ( (CtxFlagsRet & (ASC_RET_REPLAY_DETECT|ASC_RET_SEQUENCE_DETECT|ASC_RET_INTEGRITY|ASC_RET_CONFIDENTIALITY)) != 0 ) {
      svc_flags |= GSS_C_SEQUENCE_FLAG;
   }

   return( svc_flags );

} /* ntlm_ASC_RET_to_gss_svc() */



/*
 * ntlm_gss_svc_to_ISC_REQ()
 *
 * convert GSS-API service flags into ISC_REQ_* flags for InitializeSecurityContext()
 */
ULONG
ntlm_gss_svc_to_ISC_REQ( OM_uint32 p_svc_flags )
{
   ULONG   CtxFlagsReq = ISC_REQ_CONNECTION;

   if ( (p_svc_flags&GSS_C_DELEG_FLAG)!=0 )    { CtxFlagsReq |= ISC_REQ_DELEGATE;        }
   if ( (p_svc_flags&GSS_C_MUTUAL_FLAG)!=0 )   { CtxFlagsReq |= ISC_REQ_MUTUAL_AUTH;     }

#if 0
   /* AcceptSecurityContext() on Windows '95 will fail with SEC_E_UNSUPPORTED_FUNCTION */
   /* as soon as one side requests REPLAY_DETECT, SEQUENCE_DETECT or CONFIDENTIALITY   */
   /* i.e. when accepting security contexts on Windows '95 message integrity and       */
   /* message confidentiality is definitely UNAVAILABLE !!!			       */
   /* Other drawbacks:  confidentiality is undocumented in SSPI, Integrity of NTLMv1   */
   /* is horribly insecure and message protection is half duplex only ... forget it    */

   if ( (p_svc_flags&GSS_C_REPLAY_FLAG)!=0 )   { CtxFlagsReq |= ISC_REQ_REPLAY_DETECT;   }
   if ( (p_svc_flags&GSS_C_SEQUENCE_FLAG)!=0 ) { CtxFlagsReq |= ISC_REQ_SEQUENCE_DETECT; }

   /* SSPI doc says that one should request either SEQUENCE_DETECT or REPLAY_DETECT */
   /* to be able to use message protection calls later on (Sign/Verify).            */
   /* This sounds somewhat broken ...						    */
   CtxFlagsReq |= ISC_REQ_REPLAY_DETECT;

   if ( (p_svc_flags&GSS_C_INTEG_FLAG)!=0 )    { CtxFlagsReq |= ISC_REQ_INTEGRITY;       }
   if ( (p_svc_flags&GSS_C_CONF_FLAG)!=0 )     { CtxFlagsReq |= ISC_REQ_CONFIDENTIALITY; }
#endif

   return( CtxFlagsReq );

} /* ntlm_gss_svc_to_ISC_REQ() */



/*
 * ntlm_ISC_RET_to_gss_svc()
 *
 *
 */
OM_uint32
ntlm_ISC_RET_to_gss_svc( ULONG  CtxAttr )
{
   OM_uint32  svc_flags = 0;

   if ( (CtxAttr&ISC_RET_DELEGATE)!=0 )        { svc_flags |= GSS_C_DELEG_FLAG;    }

#if 0
   // Windows 2000 curiosity:  NTLM will return MUTUAL_FLAG ...
   if ( (CtxAttr&ISC_RET_MUTUAL_AUTH)!=0 )     { svc_flags |= GSS_C_MUTUAL_FLAG;   }
#endif

#if 0
   /* AcceptSecurityContext() on Windows '95 will fail with SEC_E_UNSUPPORTED_FUNCTION */
   /* as soon as one side requests REPLAY_DETECT, SEQUENCE_DETECT or CONFIDENTIALITY   */
   /* i.e. when accepting security contexts on Windows '95 message integrity and       */
   /* message confidentiality is definitely UNAVAILABLE !!!			       */
   if ( (CtxAttr&ISC_RET_REPLAY_DETECT)!=0 )   { svc_flags |= GSS_C_REPLAY_FLAG;   }
   if ( (CtxAttr&ISC_RET_SEQUENCE_DETECT)!=0 ) { svc_flags |= GSS_C_SEQUENCE_FLAG; }

   if ( (CtxAttr&ISC_RET_INTEGRITY)!=0 )       { svc_flags |= GSS_C_INTEG_FLAG;    }
   /* in GSS-API, confidentiality requires integrity services */
   if ( (CtxAttr&ISC_RET_CONFIDENTIALITY)!=0
        && (svc_flags&GSS_C_INTEG_FLAG)!=0 )   { svc_flags |= GSS_C_CONF_FLAG;     }

   /* SSPI flags are severely broken for NTLM in NT4 & Windows '95	   */
   /* attributes like integrity and confidentiality are hardly ever	   */
   /* returned (only when requested).  And when they're requested and	   */
   /* a Win'95 machine is at the other end, it will get upset		   */
   /* (i.e. return SEC_E_FUNCTION_UNSUPPORTED from AcceptSecurityContext() */
   if ( (CtxAttr&ISC_RET_SEQUENCE_DETECT)!=0 ) {
      svc_flags |= GSS_C_INTEG_FLAG;
      svc_flags |= GSS_C_CONF_FLAG;
      svc_flags |= GSS_C_REPLAY_FLAG;  /* I don't know if "REPLAY_DETECT" is true ... */
   }
   /* Furthermore:							      */
   /*  * message protection in NTLMv1 is only half duplex, since both         */
   /*    directions use a single RC4 keystream                                */
   /*  * integrity protection in NTLMv1 via MakeSignature()/VerifySignature() */
   /*    is insecure, because the "signature" is an RC4-obfuscated            */
   /*    CRC32-checksum over known plaintext		                      */

   if ( (CtxAttr&ISC_RET_SEQUENCE_DETECT)!=0 ) {
      svc_flags |= GSS_C_INTEG_FLAG;
      svc_flags |= GSS_C_CONF_FLAG;
      svc_flags |= GSS_C_REPLAY_FLAG;  /* I don't know if "REPLAY_DETECT" is true ... */
   }
#endif

   svc_flags |= GSS_C_TRANS_FLAG; /* indicate that we support security context transfer */

   /* Well, since the NTLM SSPI always maintains it own sequence number for messages */
   /* we will indicate GSS_C_SEQUENCE_FLAG whenever this will happen ...             */
   if ( (CtxAttr & (ISC_RET_REPLAY_DETECT|ISC_RET_SEQUENCE_DETECT|ISC_RET_INTEGRITY|ISC_RET_CONFIDENTIALITY)) != 0 ) {
      svc_flags |= GSS_C_SEQUENCE_FLAG;
   }

   return(svc_flags);

} /* ntlm_ISC_RET_to_gss_svc( ) */
