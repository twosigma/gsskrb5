#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/ntlm/ntlmmsg.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/ntlm/ntlmmsg.c#2 $
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


/* WARNING:
 *
 * Although this code can be compiled to support NTLM SSP's message protection
 * services, you will not receive any real security at all.  There's a low level
 * of obscurity at best.
 *
 * All sessions in NTLMv1 use the same "session key":
 *    the RC4-encryption key is always created from the first part of the LM-Hash
 *    (used for authentication) padded on the last three bytes with a static value.
 *
 * Because of the pass-through authentication on Windows '95, accepting security
 * contexts on Windows '95 aborts with SEC_E_FUNCTION_UNSUPPORTED if any message
 * protection services are requested by either security context initiator or acceptor.
 *
 * The message integrity check ("signature") is severely flawed in several ways
 * and forging it needs some computer cycles less than creating it ...
 *
 *
 * Message sequencing hasn't been implemented correctly here (btw. NTLM SSP completely
 * ignores the MessageSeqNo parameter).  A more correct implementation would use
 * a 64-bit sequence number and be hashed as well as send within every message and signature
 * token.  The receiver would then be able to check validity of the message independently
 * of sequencing and replay.
 *
 */



/*
 * ntlm_get_mic()
 *
 *
 */
OM_uint32
ntlm_get_mic( OM_uint32   * pp_min_stat,	   /* out  */
	      void	  * p_ctx,		   /* in   */
	      OM_uint32	    p_qop,		   /* in   */
	      Uchar	  * p_message,		   /* in   */
	      size_t	    p_message_len,	   /* in   */
	      Uchar	 ** pp_mic_token,	   /* out  */
	      size_t	  * pp_mic_token_len )	   /* out  */
{
   char		    * this_Call      = "ntlm_get_mic";
   ntlm_ctx_desc    * ctx            = NULL;
   Uchar	    * mic_token      = NULL;
   Uchar	    * ntlm_token     = NULL;
   size_t	      mic_token_len  = 0;
   size_t             ntlm_token_len = 0;
   SecBufferDesc      SecBufDesc;
   SecBuffer	      SecBuf[2];
   ULONG	      seqnum         = 0;
   ULONG	    * pseqnum;
   SECURITY_STATUS    SecStatus;
   OM_uint32	      maj_stat       = GSS_S_COMPLETE;

   (*pp_mic_token)     = NULL;
   (*pp_mic_token_len) = 0;

   if ( ntlm_emulate_ctx_xfer!=FALSE ) {
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );
   }

   (*pp_min_stat)      = MINOR_NO_ERROR;
   pseqnum	       = &seqnum;

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, &p_ctx,
			      CTX_MSG_OUT, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (ntlm_ctx_desc *) p_ctx;

   if ( (ctx->service_rec & GSS_C_INTEG_FLAG)==0 ) {
      DEBUG_ERR((tf, "  %s(): Integrity Service not available!\n", this_Call));
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );
   }

   if ( (ctx->service_rec & GSS_C_SEQUENCE_FLAG)!=0 ) {
      pseqnum = (ctx->role==NTLM_INITIATOR)
		 ? &(ctx->seq_ini2acc) : &(ctx->seq_acc2ini) ;
   }

   ntlm_token_len = (ctx->sizes.cbMaxSignature);
   mic_token_len  = ntlm_token_len + NTLM_TOKENDATA_START;
   mic_token      = sy_calloc( mic_token_len );
   if ( mic_token==0 ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   ntlm_token     = &(mic_token[ NTLM_TOKENDATA_START ]);

   SecBufDesc.cBuffers  = 2u;
   SecBufDesc.pBuffers  = &(SecBuf[0]);
   SecBufDesc.ulVersion = SECBUFFER_VERSION;
   
   SecBuf[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
   SecBuf[0].cbBuffer   = (ULONG)p_message_len;
   SecBuf[0].pvBuffer   = p_message;

   SecBuf[1].BufferType = SECBUFFER_TOKEN;
   SecBuf[1].cbBuffer   = (ULONG)ntlm_token_len;
   SecBuf[1].pvBuffer   = ntlm_token;

   SecStatus = (fp_SSPI_ntlm->MakeSignature)(
	 /* PCtxtHandle  phContext  */	 &(ctx->sspi_ctx),
	 /* ULONG        fQOP       */	 (ULONG)p_qop,
	 /* PSecBufferDesc pMessage */	 &SecBufDesc,
	 /* ULONG	 MessageSeqNo */ *pseqnum );

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      ntlm_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "  E: %s(): MakeSignature() failed with %s\n(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
      ERROR_RETURN( NTLM_MINOR_SSPI(MakeSignature,SecStatus), GSS_S_FAILURE );
   }


   if (maj_stat==GSS_S_COMPLETE) {

      mic_token[NTLM_TOKEN_TYPE]       = NTLM_MIC_TOKEN;
      mic_token[NTLM_TOKEN_DIRECTION]  = NTLM_MSG_OUT(ctx->role);
      (*pp_mic_token)		       = mic_token;
      (*pp_mic_token_len)	       = NTLM_TOKENDATA_START + SecBuf[1].cbBuffer;

      (*pseqnum)		      += 2;  /* advance sequencing counter */

   } else {
error:
      if ( mic_token!=NULL && mic_token_len>0 ) {
	 sy_clear_free( (void **) &mic_token, mic_token_len );
      }
   }

   return(maj_stat);

} /* ntlm_get_mic() */




/*
 * ntlm_verify_mic()
 *
 *
 */
OM_uint32
ntlm_verify_mic( OM_uint32  * pp_min_stat,      /* out */
		 void       * p_ctx,
		 Uchar      * p_message,
		 size_t       p_message_len,
		 Uchar      * p_mic_token,
		 size_t       p_mic_token_len,
		 OM_uint32  * pp_qop )
{
   char		    * this_Call      = "ntlm_verify_mic";
   ntlm_ctx_desc    * ctx            = NULL;
   SecBufferDesc      SecBufDesc;
   SecBuffer	      SecBuf[2];
   Uchar	    * ntlm_token     = NULL;
   size_t	      ntlm_token_len = 0;
   ULONG	      seqnum         = 0;
   ULONG	      qop	     = 0;
   ULONG	    * pseqnum;
   SECURITY_STATUS    SecStatus;
   OM_uint32	      maj_stat       = GSS_S_COMPLETE;

   (*pp_qop)      = 0;

   if ( ntlm_emulate_ctx_xfer!=FALSE ) {
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );
   }

   (*pp_min_stat) = MINOR_NO_ERROR;
   pseqnum        = &seqnum;

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, &p_ctx,
			      CTX_MSG_IN, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (ntlm_ctx_desc *) p_ctx;

   if ( (ctx->service_rec & GSS_C_INTEG_FLAG)==0 ) {
      DEBUG_ERR((tf, "  %s(): Integrity Service not available!\n", this_Call));
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );
   }

   if ( (ctx->service_rec & GSS_C_SEQUENCE_FLAG)!=0 ) {
      pseqnum = (ctx->role==NTLM_INITIATOR)
		 ? &(ctx->seq_acc2ini) : &(ctx->seq_ini2acc) ;
   }

   if ( p_mic_token_len<NTLM_TOKENDATA_START ) {
      DEBUG_ERR((tf, "  E: %s(): Truncated signature token (len = %u)!\n",
		     this_Call, (Uint)p_mic_token_len ));
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );
   }

   if ( p_mic_token[NTLM_TOKEN_TYPE]!=NTLM_MIC_TOKEN ) {
      DEBUG_ERR((tf, "  E: %s(): this is not a signature token!\n", this_Call));
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );
   }

   if ( p_mic_token[NTLM_TOKEN_DIRECTION]!=NTLM_MSG_IN(ctx->role) ) {
      DEBUG_ERR((tf, "  E: %s(): This signature is to be processed by the peer!\n",
		     this_Call));
      RETURN_MIN_MAJ( MINOR_REFLECTED_MIC, GSS_S_FAILURE );
   }

   ntlm_token     = &(p_mic_token[ NTLM_TOKENDATA_START ]);
   ntlm_token_len = p_mic_token_len - NTLM_TOKENDATA_START;

   SecBufDesc.cBuffers  = 2u;
   SecBufDesc.pBuffers  = &(SecBuf[0]);
   SecBufDesc.ulVersion = SECBUFFER_VERSION;
   
   SecBuf[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
   SecBuf[0].cbBuffer   = (ULONG)p_message_len;
   SecBuf[0].pvBuffer   = p_message;

   SecBuf[1].BufferType = SECBUFFER_TOKEN | SECBUFFER_READONLY;
   SecBuf[1].cbBuffer   = (ULONG)ntlm_token_len;
   SecBuf[1].pvBuffer   = ntlm_token;

   SecStatus = (fp_SSPI_ntlm->VerifySignature)(
	 /* PCtxtHandle  phContext  */	 &(ctx->sspi_ctx),
	 /* PSecBufferDesc pMessage */	 &SecBufDesc,
	 /* ULONG	 MessageSeqNo */ *pseqnum,
	 /* ULONG        fQOP       */	 &qop );

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      ntlm_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "  E: %s(): VerifySignature() failed with %s\n(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
      RETURN_MIN_MAJ( NTLM_MINOR_SSPI(VerifySignature,SecStatus), GSS_S_FAILURE );
   }

   if ( maj_stat==GSS_S_COMPLETE) {
      (*pseqnum)		      += 2;  /* advance sequencing counter */
      (*pp_qop) = (OM_uint32) qop;
   }

   return(maj_stat);

} /* ntlm_verify_mic() */





/*
 * ntlm_calc_padding()
 *
 *
 */
static Uint
ntlm_calc_padding( ntlm_ctx_desc * p_ctx, size_t p_msg_len )
{
   /* if no blocksize is provided (cbBlockSize==0)		    */
   /* then use fixed padding -- always at least one byte of padding */
   Uint   padding = 1;

   if ( p_ctx->sizes.cbBlockSize>1 ) {

      /* explicit blocksize provided,					   */
      /* calculate padding -- use within [1..cbBlockSize] bytes of padding */
      ldiv_t  ld;

      ld = ldiv( (long) p_msg_len, (long) p_ctx->sizes.cbBlockSize );
      if ( ld.rem==0 ) {
	 padding = (Uint) p_ctx->sizes.cbBlockSize;
      } else {
	 padding = (Uint) ld.rem;
      }
   }

   return(padding);

} /* ntlm_calc_padding() */




/*
 * ntlm_wrap()
 *
 *
 */
OM_uint32
ntlm_wrap( OM_uint32	  * pp_min_stat,	   /* out  */
	   void		  * p_ctx,		   /* in   */
	   OM_uint32	    p_qop,		   /* in   */
	   int		    p_conf_req,		   /* in   */
	   Uchar	  * p_message,		   /* in   */
	   size_t	    p_message_len,	   /* in   */
	   Uchar	 ** pp_wrap_token,	   /* out  */
	   size_t	  * pp_wrap_token_len,	   /* out  */
	   int		  * pp_conf_state )	   /* out  */
{
   char		    * this_Call         = "ntlm_wrap";
   ntlm_ctx_desc    * ctx               = NULL;
   char		    * sspi_call         = NULL;
   Uchar	    * token             = NULL;
   Uchar	    * mic_token         = NULL;
   Uchar	    * seal_token        = NULL;
   Uchar	    * mic_maxlen_field  = NULL;
   Uchar	    * mic_truelen_field = NULL;
   size_t	      token_len         = 0;
   size_t	      mic_token_len     = 0;
   size_t	      seal_token_len    = 0;
   SecBufferDesc      SecBufDesc;
   SecBuffer	      SecBuf[2];
   ULONG	      seqnum            = 0;
   ULONG	    * pseqnum;
   SECURITY_STATUS    SecStatus;
   OM_uint32	      maj_stat          = GSS_S_COMPLETE;
   Uint		      padding	        = 0;
   Uint		      i;
   int		      conf_state        = FALSE;
   Uchar	      token_type        = 0;


   (*pp_wrap_token)     = NULL;
   (*pp_wrap_token_len) = 0;
   (*pp_conf_state)	= 0;

   if ( ntlm_emulate_ctx_xfer!=FALSE ) {
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );
   }

   (*pp_min_stat)       = MINOR_NO_ERROR;
   pseqnum	        = &seqnum;

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, &p_ctx,
			      CTX_MSG_OUT, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (ntlm_ctx_desc *) p_ctx;

   if ( (ctx->service_rec & GSS_C_SEQUENCE_FLAG)!=0 ) {
      pseqnum = (ctx->role==NTLM_INITIATOR)
		 ? &(ctx->seq_ini2acc) : &(ctx->seq_acc2ini) ;
   }

   if ( (ctx->service_rec&GSS_C_CONF_FLAG)!=0  &&  p_conf_req!=0 ) {
      /* confidentiality requested, but not available */
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );

   } else {
      /* NO confidentiality available or requested */

      conf_state       = FALSE;
      mic_token_len    = ctx->sizes.cbMaxSignature;
      token_type       = NTLM_WRAP_TOKEN;

   }

   padding = ntlm_calc_padding( ctx, p_message_len );

   seal_token_len = p_message_len + padding;
   token_len      = NTLM_TOKENDATA_START
		    + 2 /* mic_maxlen_field */ + 2 /* mic_truelen_field */
		    + mic_token_len + seal_token_len;
   token	  = sy_malloc( token_len );
   if ( token==0 ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   mic_maxlen_field  = token + NTLM_TOKENDATA_START;
   STORE_UINT16_MSB( mic_token_len, mic_maxlen_field );
   mic_truelen_field = mic_maxlen_field  + 2;

   mic_token         = mic_truelen_field + 2;

   seal_token        = mic_token + mic_token_len;

   memcpy( seal_token, p_message, p_message_len );
   for ( i=0 ; i<padding ; i++ ) {
      seal_token[p_message_len+i] = (Uchar) padding;
   }

   /* For conf_state==FALSE, we create only a signature token and leave  */
   /* the application data in clear text (however, it is still "padded") */

   SecBufDesc.cBuffers  = 2u;
   SecBufDesc.pBuffers  = &(SecBuf[0]);
   SecBufDesc.ulVersion = SECBUFFER_VERSION;
   
   /* the message in seal_token is already padded, but still plaintext ! */
   SecBuf[0].BufferType = SECBUFFER_DATA;
   SecBuf[0].cbBuffer   = (ULONG)seal_token_len;
   SecBuf[0].pvBuffer   = seal_token;

   SecBuf[1].BufferType = SECBUFFER_TOKEN;
   SecBuf[1].cbBuffer   = (ULONG)mic_token_len;
   SecBuf[1].pvBuffer   = mic_token;

   if ( conf_state==FALSE ) {
      /* NO confidentiality requested (or available)     */
      /* i.e. applying ONLY Message Integrity Protection */

      SecBuf[0].BufferType |= SECBUFFER_READONLY;

      sspi_call = "MakeSignature";
      SecStatus = (fp_SSPI_ntlm->MakeSignature)(
	 /* PCtxtHandle    phContext    */  &(ctx->sspi_ctx),
	 /* ULONG          fQOP         */  (ULONG)p_qop,
	 /* PSecBufferDesc pMessage     */  &SecBufDesc,
	 /* ULONG	   MessageSeqNo */  *pseqnum );

   } else { /* conf_state!=FALSE */

      SecStatus = SEC_E_UNSUPPORTED_FUNCTION;

   } /* conf_state!=FALSE */

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      ntlm_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "  E: %s(): %s() failed with %s\n(Desc=\"%s\")\n",
			this_Call, sspi_call, symbol, desc ))
      ERROR_RETURN( NTLM_MINOR_SSPI(MakeSignature,SecStatus), GSS_S_FAILURE );
   }

   /* put the true length of the signature or trailer token into mic_truelen_field */
   STORE_UINT16_MSB( SecBuf[1].cbBuffer, mic_truelen_field );


   token[NTLM_TOKEN_TYPE]      = token_type;
   token[NTLM_TOKEN_DIRECTION] = NTLM_MSG_OUT(ctx->role);
   (*pp_wrap_token)	       = token;
   (*pp_wrap_token_len)	       = token_len;
   (*pp_conf_state)            = conf_state;

   (*pseqnum)		      += 2;  /* advance sequencing counter */

   if (maj_stat!=GSS_S_COMPLETE) {
error:
      if ( token!=NULL && token_len>0 ) {
	 sy_clear_free( (void **) &token, token_len );
      }
   }

   return(maj_stat);

} /* ntlm_wrap() */




/*
 * ntlm_unwrap()
 *
 *
 */
OM_uint32
ntlm_unwrap( OM_uint32   * pp_min_stat,
	     void	 * p_ctx,
	     Uchar	 * p_wrap_token,
	     size_t	   p_wrap_token_len,
	     Uchar	** pp_message,
	     size_t	 * pp_message_len,
	     int	 * pp_conf_state,
	     OM_uint32   * pp_qop )
{
   char		    * this_Call         = "ntlm_unwrap";
   ntlm_ctx_desc    * ctx               = NULL;
   char		    * sspi_call         = NULL;
   Uchar	    * message           = NULL;
   Uchar	    * mic_token         = NULL;
   Uchar	    * seal_token        = NULL;
   Uchar	    * mic_maxlen_field  = NULL;
   Uchar	    * mic_truelen_field = NULL;
   size_t	      message_len       = 0;
   size_t	      message_true_len  = 0;
   size_t	      mic_maxlen        = 0;
   size_t	      mic_truelen	= 0;
   size_t	      seal_token_len    = 0;
   SecBufferDesc      SecBufDesc;
   SecBuffer	      SecBuf[2];
   ULONG	      seqnum            = 0;
   ULONG	    * pseqnum;
   ULONG	      qop	        = 0;
   SECURITY_STATUS    SecStatus;
   OM_uint32	      maj_stat          = GSS_S_COMPLETE;
   Uint		      i;
   Uint		      padding           = 0;
   Uint	 	      expected_padding  = 0;
   int		      conf_state        = FALSE;
   Uchar	      token_type        = 0;


   (*pp_message)	= NULL;
   (*pp_message_len)	= 0;
   (*pp_conf_state)	= 0;
   (*pp_qop)		= 0;

   if ( ntlm_emulate_ctx_xfer!=FALSE ) {
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );
   }

   (*pp_min_stat)       = MINOR_NO_ERROR;
   pseqnum	        = &seqnum;

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, &p_ctx,
			      CTX_MSG_OUT, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (ntlm_ctx_desc *) p_ctx;

   if ( (ctx->service_rec & GSS_C_SEQUENCE_FLAG)!=0 ) {
      pseqnum = (ctx->role==NTLM_INITIATOR)
		 ? &(ctx->seq_acc2ini) : &(ctx->seq_ini2acc) ;
   }

   if ( p_wrap_token_len< (NTLM_TOKENDATA_START + 2 + 2) ) {
      DEBUG_ERR((tf, "  E: %s(): wrap token truncated (len = %lu)!\n",
		     this_Call, (Ulong) p_wrap_token_len ))
      RETURN_MIN_MAJ( MINOR_MAY_BE_TRUNCATED, GSS_S_DEFECTIVE_TOKEN );
   }

   token_type        = p_wrap_token[NTLM_TOKEN_TYPE];

   if ( token_type==NTLM_WRAP_TOKEN ) {

      conf_state = FALSE;
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );

   } else if ( token_type==NTLM_WRAPCONF_TOKEN ) {

      conf_state = TRUE;

   } else {

      DEBUG_ERR((tf, "  E: %s(): this is not a wrap token!\n", this_Call));
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );

   }


   if ( p_wrap_token[NTLM_TOKEN_DIRECTION]!=NTLM_MSG_IN(ctx->role) ) {
      DEBUG_ERR((tf, "  E: %s(): This wrap token is to be processed by the peer!\n",
		     this_Call));
      RETURN_MIN_MAJ( MINOR_REFLECTED_WRAP, GSS_S_FAILURE );
   }

   mic_maxlen_field  = p_wrap_token      + NTLM_TOKENDATA_START;
   mic_truelen_field = mic_maxlen_field  + 2;
   mic_token	     = mic_truelen_field + 2;

   mic_maxlen     = LOAD_UINT16_MSB( mic_maxlen_field  );
   mic_truelen    = LOAD_UINT16_MSB( mic_truelen_field );

   seal_token     = mic_token + mic_maxlen;
   seal_token_len = p_wrap_token_len - ( seal_token - p_wrap_token );

   if ( p_wrap_token_len< (NTLM_TOKENDATA_START + 2 + 2 + mic_maxlen) ) {
      DEBUG_ERR((tf, "  E: %s(): wrap token truncated (len = %lu)!\n",
		     this_Call, (Ulong) p_wrap_token_len ))
      RETURN_MIN_MAJ( MINOR_MAY_BE_TRUNCATED, GSS_S_DEFECTIVE_TOKEN );
   }

/* Cleanup Alert:  the next call dynamically allocates a message buffer */
/* it needs to be released when this call returns with an error, or     */
/* when the output cleartext message has a length of 0 bytes !          */
   message_len = seal_token_len;
   message     = sy_malloc( message_len );
   if ( message==NULL ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }


   memcpy( message, seal_token, message_len );

   /* For conf_state==FALSE, we create only a signature token and leave  */
   /* the application data in clear text (however, it is still "padded") */

   SecBufDesc.cBuffers  = 2u;
   SecBufDesc.pBuffers  = &(SecBuf[0]);
   SecBufDesc.ulVersion = SECBUFFER_VERSION;
   
   /* the message in seal_token is already padded, but still plaintext ! */
   SecBuf[0].BufferType = SECBUFFER_DATA;
   SecBuf[0].cbBuffer   = (ULONG)message_len;
   SecBuf[0].pvBuffer   = message;

   SecBuf[1].BufferType = SECBUFFER_TOKEN | SECBUFFER_READONLY;
   SecBuf[1].cbBuffer   = (ULONG)mic_truelen;
   SecBuf[1].pvBuffer   = mic_token;


   if ( conf_state==FALSE ) {

      SecBuf[0].BufferType |= SECBUFFER_READONLY;

      sspi_call = "VerifySignature";
      SecStatus = (fp_SSPI_ntlm->VerifySignature)(
	 /* PCtxtHandle    phContext    */  &(ctx->sspi_ctx),
	 /* PSecBufferDesc pMessage     */  &SecBufDesc,
	 /* ULONG	   MessageSeqNo */  *pseqnum,
	 /* ULONG          fQOP         */  &qop );

   } else { /* conf_state!=FALSE */

      SecStatus = SEC_E_UNSUPPORTED_FUNCTION;

   }

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      ntlm_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "  E: %s(): %s() failed with %s\n(Desc=\"%s\")\n",
			this_Call, sspi_call, symbol, desc ))
      ERROR_RETURN( NTLM_MINOR_SSPI(VerifySignature,SecStatus), GSS_S_FAILURE );
   }

   padding = message[message_len-1]; /* the last byte must always be a padding byte */
   if ( padding > message_len
	||  padding > ctx->sizes.cbBlockSize ) {
padding_error:
      DEBUG_ERR((tf, "  E: %s(): invalid padding for message (%u bytes)!\n",
		     (Uint) padding))
      ERROR_RETURN( NTLM_MINOR(INVALID_WRAP_PADDING), GSS_S_DEFECTIVE_TOKEN );
   }

   message_true_len  = message_len - padding;

   for ( i=0 ; i<padding ; i++ ) {
      if ( message[message_true_len + i] != (Uchar) padding ) {
	 goto padding_error;
      }
   }

   expected_padding = ntlm_calc_padding( ctx, message_true_len );
   if ( expected_padding != padding ) {
      goto padding_error;
   }

   if ( message_true_len> 0 ) {
      (*pp_message)     = message;
      (*pp_message_len) = message_true_len;
   } else {
      (*pp_message)     = NULL;
      (*pp_message_len) = 0;
      sy_clear_free( (void **) &message, message_len );
   }

   (*pp_qop)	    = (OM_uint32) qop;
   (*pp_conf_state) = conf_state;

   (*pseqnum)	   += 2;  /* advance sequencing counter */


   if (maj_stat!=GSS_S_COMPLETE) {
error:
      if ( message!=NULL  &&  message_len>0 ) {
	 sy_clear_free( (void **) &message, message_len );
      }
   }

   return(maj_stat);

} /* ntlm_unwrap() */





/*
 * ntlm_wrap_size_limit()
 *
 *
 */
OM_uint32
ntlm_wrap_size_limit( OM_uint32	 * pp_min_stat,	   /* out */
		      void	 * p_ctx,	   /* in  */
		      int	   p_conf_req,	   /* in  */
		      OM_uint32	   p_qop,	   /* in  */
		      OM_uint32	   p_req_out_size, /* in  */
		      OM_uint32	 * pp_max_input )  /* out */
{
   char		    * this_Call      = "ntlm_wrap_size_limit";
   ntlm_ctx_desc    * ctx            = NULL;
   size_t	      len	     = 0;
   size_t	      mic_max_len    = 0;
   size_t	      fix_len	     = 0;
   OM_uint32	      maj_stat	     = GSS_S_COMPLETE;

   (*pp_max_input) = 0;

   if ( ntlm_emulate_ctx_xfer!=FALSE ) {
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );
   }

   (*pp_min_stat)  = MINOR_NO_ERROR;

   /* quick validation of ntlm context handle */
   maj_stat = ntlm_check_ctx( pp_min_stat, &p_ctx,
			      CTX_MSG_OUT, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (ntlm_ctx_desc *) p_ctx;

   /* MISSING: we should check the QOP value for validity ...     */
   
   if ( (ctx->service_rec&(GSS_C_CONF_FLAG|GSS_C_INTEG_FLAG))==0 ) {
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE , GSS_S_FAILURE );
   }

   /* the following must match the algorithm implemented in ntlm_wrap ! */
   if ( (ctx->service_rec&GSS_C_CONF_FLAG)!=FALSE  &&  p_conf_req!=FALSE ) {
      mic_max_len = ctx->sizes.cbSecurityTrailer;
   } else {
      mic_max_len = ctx->sizes.cbMaxSignature;
   }

   /* first, calculate the minimum size of a wrap token ... */
   fix_len = NTLM_TOKENDATA_START
	     + 2 /* mic_maxlen_field */ + 2 /* mic_truelen_field */ + mic_max_len;

   /* the following MUST match the padding algorithm in ntlm_calc_padding ! */
   len = fix_len + ( (ctx->sizes.cbBlockSize>0) ? ctx->sizes.cbBlockSize : 1 );

   if ( p_req_out_size<len ) {
      RETURN_MIN_MAJ( MINOR_SIZE_TOO_SMALL, GSS_S_FAILURE );
   }

   /* then calculate the maximum possible size to fit in the */
   /* requested output buffer size			     */
   (*pp_max_input) = (OM_uint32)(p_req_out_size - fix_len - 1);
   
   return(GSS_S_COMPLETE);

} /* ntlm_wrap_size_limit() */



