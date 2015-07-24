#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/krb5/krb5msg.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/krb5/krb5msg.c#2 $
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
 * krb5_get_mic()
 *
 *
 */
OM_uint32
krb5_get_mic( OM_uint32   * pp_min_stat,	   /* out  */
	      void	  * p_ctx,		   /* in   */
	      OM_uint32	    p_qop,		   /* in   */
	      Uchar	  * p_message,		   /* in   */
	      size_t	    p_message_len,	   /* in   */
	      Uchar	 ** pp_mic_token,	   /* out  */
	      size_t	  * pp_mic_token_len )	   /* out  */
{
   char		    * this_Call      = "krb5_get_mic";
   krb5_ctx_desc    * ctx            = NULL;
   Uchar	    * mic_token      = NULL;
   size_t	      mic_token_len  = 0;
   SecBufferDesc      SecBufDesc;
   SecBuffer	      SecBuf[2];
   SECURITY_STATUS    SecStatus;
   OM_uint32	      maj_stat       = GSS_S_COMPLETE;

   (*pp_mic_token)     = NULL;
   (*pp_mic_token_len) = 0;

   (*pp_min_stat)      = MINOR_NO_ERROR;

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, &p_ctx,
			      CTX_MSG_OUT, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (krb5_ctx_desc *) p_ctx;

   if ( (ctx->service_rec & GSS_C_INTEG_FLAG)==0 ) {
      DEBUG_ERR((tf, "ERR: %s(): Integrity Service not available!\n", this_Call));
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );
   }

   mic_token_len  = (ctx->sizes.cbMaxSignature);
   mic_token      = sy_calloc( mic_token_len );
   if ( mic_token==0 ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   SecBufDesc.cBuffers  = 2;
   SecBufDesc.pBuffers  = &(SecBuf[0]);
   SecBufDesc.ulVersion = SECBUFFER_VERSION;
   
   /* BUG in Windows2000:  Don't use SECBUFFER_READONLY for the DATA       */
   /* It will create invalid signatures (not compatible with MIT Kerberos) */
   SecBuf[0].BufferType = SECBUFFER_DATA;
   SecBuf[0].cbBuffer   = (ULONG)p_message_len;
   SecBuf[0].pvBuffer   = p_message;

   SecBuf[1].BufferType = SECBUFFER_TOKEN;
   SecBuf[1].cbBuffer   = (ULONG)mic_token_len;
   SecBuf[1].pvBuffer   = mic_token;

   SecStatus = (fp_SSPI_krb5->MakeSignature)(
	 /* PCtxtHandle  phContext  */	 &(ctx->sspi_ctx),
	 /* ULONG        fQOP       */	 (ULONG)p_qop,
	 /* PSecBufferDesc pMessage */	 &SecBufDesc,
	 /* ULONG	 MessageSeqNo */ 0 );

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "ERR: %s(): MakeSignature() failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
      switch( SecStatus ) {
	 case SEC_E_INVALID_HANDLE:	  maj_stat = GSS_S_NO_CONTEXT;		 break;
	 case SEC_E_CONTEXT_EXPIRED:	  maj_stat = GSS_S_CONTEXT_EXPIRED;	 break;
	 case SEC_E_QOP_NOT_SUPPORTED:	  maj_stat = GSS_S_BAD_QOP;		 break;
	 default:		          maj_stat = GSS_S_FAILURE;		 break;
      }

      ERROR_RETURN( KRB5_MINOR_SSPI(MakeSignature,SecStatus), maj_stat );
   }


   if (maj_stat==GSS_S_COMPLETE) {

      (*pp_mic_token)		       = mic_token;
      (*pp_mic_token_len)	       = SecBuf[1].cbBuffer;

   } else {
error:
      if ( mic_token!=NULL && mic_token_len>0 ) {
	 sy_clear_free( (void **) &mic_token, mic_token_len );
      }
   }

   return(maj_stat);

} /* krb5_get_mic() */




/*
 * krb5_verify_mic()
 *
 *
 */
OM_uint32
krb5_verify_mic( OM_uint32  * pp_min_stat,      /* out */
		 void       * p_ctx,
		 Uchar      * p_message,
		 size_t       p_message_len,
		 Uchar      * p_mic_token,
		 size_t       p_mic_token_len,
		 OM_uint32  * pp_qop )
{
   char		    * this_Call      = "krb5_verify_mic";
   krb5_ctx_desc    * ctx            = NULL;
   SecBufferDesc      SecBufDesc;
   SecBuffer	      SecBuf[2];
   Uchar	    * krb5_token     = NULL;
   size_t	      krb5_token_len = 0;
   ULONG	      qop	     = 0;
   SECURITY_STATUS    SecStatus;
   OM_uint32	      maj_stat       = GSS_S_COMPLETE;

   (*pp_min_stat) = MINOR_NO_ERROR;
   (*pp_qop)      = 0;

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, &p_ctx,
			      CTX_MSG_IN, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (krb5_ctx_desc *) p_ctx;

   if ( (ctx->service_rec & GSS_C_INTEG_FLAG)==0 ) {
      DEBUG_ERR((tf, "ERR: %s(): Integrity Service not available!\n", this_Call));
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE, GSS_S_FAILURE );
   }

   if ( p_mic_token_len==0 || p_mic_token==NULL ) {
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );
   }

   SecBufDesc.cBuffers  = 2;
   SecBufDesc.pBuffers  = &(SecBuf[0]);
   SecBufDesc.ulVersion = SECBUFFER_VERSION;
   
   /* BUG in Windows2000:  Don't use SECBUFFER_READONLY for the DATA */
   /* This will fail signatures verification for MIT Kerberos peers  */
   SecBuf[0].BufferType = SECBUFFER_DATA;
   SecBuf[0].cbBuffer   = (ULONG)p_message_len;
   SecBuf[0].pvBuffer   = p_message;

   /* Bogus Windows2000 behaviour: Signature tokens will be decrypted inplace,   */
   /* and the output SecBuffer will actually point into the input SecBuffer      */
   /* -- it is not a new standalone SecBuffer and must therefore not be free()d  */
   /*										 */
   /* Result: one may not use SECBUFFER_READONLY for the input MIC token.	 */
   /* BUG in Windows2000: When using SECBUFFER_READONLY, the Buffer is still modified */
   /*										 */
   /* To conform to GSS-API semantics we must not modify the original MIC token, */
   /* therefore we create a temorary copy which SSP can frob with.		 */
   krb5_token_len = p_mic_token_len;

/* Cleanup Alert: the next call creates a temporary buffer in krb5_token */
   krb5_token     = sy_malloc( krb5_token_len );
   if ( krb5_token==NULL )
      ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );

   memcpy( krb5_token, p_mic_token, krb5_token_len);

   SecBuf[1].BufferType = SECBUFFER_TOKEN;
   SecBuf[1].cbBuffer   = (ULONG)krb5_token_len;
   SecBuf[1].pvBuffer   = krb5_token;

   SecStatus = (fp_SSPI_krb5->VerifySignature)(
	 /* PCtxtHandle  phContext  */	 &(ctx->sspi_ctx),
	 /* PSecBufferDesc pMessage */	 &SecBufDesc,
	 /* ULONG	 MessageSeqNo */ 0,
	 /* ULONG        fQOP       */	 &qop );

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "ERR: %s(): VerifySignature() failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
      switch( SecStatus ) {
	 case SEC_E_INVALID_HANDLE:	  maj_stat = GSS_S_NO_CONTEXT;		 break;
	 case SEC_E_CONTEXT_EXPIRED:	  maj_stat = GSS_S_CONTEXT_EXPIRED;	 break;
	 case SEC_E_QOP_NOT_SUPPORTED:	  maj_stat = GSS_S_BAD_QOP;		 break;
	 case SEC_E_INVALID_TOKEN:	  maj_stat = GSS_S_DEFECTIVE_TOKEN;	 break;
	 case SEC_E_MESSAGE_ALTERED:	  maj_stat = GSS_S_BAD_SIG;		 break;
	 /* unfortunately, we cannot distinguish DUPLICATE, OLD, UNSEQ or GAP tokens thru SSPI */
	 case SEC_E_OUT_OF_SEQUENCE:	  maj_stat = GSS_S_FAILURE;		 break;
	 case SEC_E_INCOMPLETE_MESSAGE:	  maj_stat = GSS_S_DEFECTIVE_TOKEN;	 break;
	 default:		          maj_stat = GSS_S_FAILURE;		 break;
      }

      ERROR_RETURN( KRB5_MINOR_SSPI(VerifySignature,SecStatus), maj_stat );
   }

   if ( maj_stat==GSS_S_COMPLETE) {

      (*pp_qop) = (OM_uint32) qop;

   } else {
error:
      (*pp_qop) = 0;
   }

   sy_clear_free( (void **) &krb5_token, krb5_token_len );

   return(maj_stat);

} /* krb5_verify_mic() */




/*
 * krb5_calc_padding()
 *
 *
 */
static Uint
krb5_calc_padding( krb5_ctx_desc * p_ctx, size_t p_msg_len )
{
   /* if no blocksize is provided for EncryptMessage (cbBlockSize==0) */
   /* then use fixed padding -- always at least one byte of padding   */
   Uint   padding = 1;

   if ( p_ctx->sizes.cbBlockSize>1 ) {

      /* explicit blocksize provided for EncryptMessage,                   */
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

} /* krb5_calc_padding() */




/*
 * krb5_wrap()
 *
 *
 */
OM_uint32
krb5_wrap( OM_uint32	  * pp_min_stat,	   /* out  */
	   void		  * p_ctx,		   /* in   */
	   OM_uint32	    p_qop,		   /* in   */
	   int		    p_conf_req,		   /* in   */
	   Uchar	  * p_message,		   /* in   */
	   size_t	    p_message_len,	   /* in   */
	   Uchar	 ** pp_wrap_token,	   /* out  */
	   size_t	  * pp_wrap_token_len,	   /* out  */
	   int		  * pp_conf_state )	   /* out  */
{
   char		    * this_Call         = "krb5_wrap";
   krb5_ctx_desc    * ctx               = NULL;
   char		    * sspi_call         = NULL;
   Uchar	    * token             = NULL;   /* temporary memory */
   Uchar	    * trailer_token     = NULL;   /* temporary memory */
   Uchar	    * data_token        = NULL;   /* temporary memory */
   Uchar            * padding_token     = NULL;   /* temporary memory */
   size_t	      token_len         = 0;
   size_t	      trailer_token_len = 0;
   size_t	      data_token_len    = 0;
   size_t             padding_token_len = 0;
   size_t             len0, len1, len2;
   SecBufferDesc      SecBufDesc;
   SecBuffer	      SecBuf[3];
   unsigned long      fQOP;
   SECURITY_STATUS    SecStatus;
   OM_uint32	      maj_stat          = GSS_S_COMPLETE;
   int		      conf_state        = FALSE;


   (*pp_wrap_token)     = NULL;
   (*pp_wrap_token_len) = 0;
   (*pp_conf_state)	= 0;

   (*pp_min_stat)       = MINOR_NO_ERROR;

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, &p_ctx,
			      CTX_MSG_OUT, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (krb5_ctx_desc *) p_ctx;

   /* First, we need to duplicate the outgoing message,  */
   /* since SSPI is going to encrypt the data inplace !! */

   /* NOTICE: We may be processing a ZERO-length message here !! */
   data_token_len    = p_message_len;
   if ( data_token_len>0 ) {
      data_token        = sy_malloc( data_token_len );
      if (data_token==NULL)
	 ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
      memcpy( data_token, p_message, data_token_len );
   }

   trailer_token_len = ctx->sizes.cbSecurityTrailer;
   if (trailer_token_len>0) {
      trailer_token     = sy_calloc( trailer_token_len );
      if (trailer_token==NULL)
	 ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   padding_token_len = ctx->sizes.cbBlockSize;
   if (padding_token_len>0) {
      padding_token     = sy_calloc( padding_token_len );
      if (padding_token==NULL)
	 ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   SecBufDesc.cBuffers  = 3;
   SecBufDesc.pBuffers  = &(SecBuf[0]);
   SecBufDesc.ulVersion = SECBUFFER_VERSION;
   
   SecBuf[0].BufferType = SECBUFFER_TOKEN;
   SecBuf[0].cbBuffer   = (ULONG)trailer_token_len;
   SecBuf[0].pvBuffer   = trailer_token;

   SecBuf[1].BufferType = SECBUFFER_DATA;
   SecBuf[1].cbBuffer   = (ULONG)data_token_len;
   SecBuf[1].pvBuffer   = data_token;

   SecBuf[2].BufferType = SECBUFFER_PADDING;
   SecBuf[2].cbBuffer   = (ULONG)padding_token_len;
   SecBuf[2].pvBuffer   = padding_token;


   if ( (ctx->service_rec&GSS_C_CONF_FLAG)!=0  &&  p_conf_req!=0 ) {
      /* confidentiality available and requested */

      conf_state       = TRUE;
      fQOP             = 0;			/* According to Microsoft */

   } else {
      /* NO confidentiality available or requested */

      conf_state       = FALSE;
      fQOP             = KERB_WRAP_NO_ENCRYPT;	/* According to Microsoft */

   }

   sspi_call = "EncryptMessage";
   SecStatus = (fp_SSPI_krb5->EncryptMessage)(
      /* PCtxtHandle    phContext    */   &(ctx->sspi_ctx),
      /* ULONG          fQOP         */   fQOP,
      /* PSecBufferDesc pMessage     */   &SecBufDesc,
      /* ULONG		MessageSeqNo */   0 ); /* According to Microsoft */

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "ERR: %s(): %s() failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, sspi_call, symbol, desc ))
      switch( SecStatus ) {
	 case SEC_E_INVALID_HANDLE:	  maj_stat = GSS_S_NO_CONTEXT;		 break;
	 case SEC_E_CONTEXT_EXPIRED:	  maj_stat = GSS_S_CONTEXT_EXPIRED;	 break;
	 case SEC_E_QOP_NOT_SUPPORTED:	  maj_stat = GSS_S_BAD_QOP;		 break;
	 default:		          maj_stat = GSS_S_FAILURE;		 break;
      }

      if ( conf_state ) {
	 ERROR_RETURN( KRB5_MINOR_SSPI(EncryptMessage,SecStatus), maj_stat );
      }
      ERROR_RETURN( KRB5_MINOR_SSPI(EncryptMessageMIC,SecStatus), maj_stat );
   }

   len0      = SecBuf[0].cbBuffer;
   len1      = SecBuf[1].cbBuffer;
   len2      = SecBuf[2].cbBuffer;

   token_len = len0 + len1 + len2;
   token     = sy_malloc(token_len);
   if ( token==NULL )
      ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );

   if ( len0>0 )
      memcpy( &(token[0]),	    SecBuf[0].pvBuffer, len0 );
   if ( len1>0 )  /* We may be processing a ZERO-length message here */
      memcpy( &(token[len0]),	    SecBuf[1].pvBuffer, len1 );
   if ( len2>0 )
      memcpy( &(token[len0+len1]),  SecBuf[2].pvBuffer, len2 );

   (*pp_wrap_token)	       = token;
   token                       = NULL;
   (*pp_wrap_token_len)	       = token_len;
   token_len                   = 0;
   (*pp_conf_state)            = conf_state;

   if (maj_stat!=GSS_S_COMPLETE) {
error:
      (*pp_wrap_token)         = NULL;
      (*pp_wrap_token_len)     = 0;
   }

   sy_clear_free( (void **) &token,         token_len	      );
   sy_clear_free( (void **) &data_token,    data_token_len    );
   sy_clear_free( (void **) &padding_token, padding_token_len );
   sy_clear_free( (void **) &trailer_token, trailer_token_len );

   return(maj_stat);

} /* krb5_wrap() */




/*
 * krb5_unwrap()
 *
 *
 */
OM_uint32
krb5_unwrap( OM_uint32   * pp_min_stat,	   /* krb5msg.c */
	     void	 * p_ctx,
	     Uchar	 * p_wrap_token,
	     size_t	   p_wrap_token_len,
	     Uchar	** pp_message,
	     size_t	 * pp_message_len,
	     int	 * pp_conf_state,
	     OM_uint32   * pp_qop )
{
   char		    * this_Call         = "krb5_unwrap";
   krb5_ctx_desc    * ctx               = NULL;
   char		    * sspi_call         = NULL;
   Uchar            * token             = NULL;	 /* temporary memory    */
   size_t             token_len         = 0;
   SecBufferDesc      SecBufDesc;
   SecBuffer	      SecBuf[2];
   unsigned long      fQop;
   SECURITY_STATUS    SecStatus;
   OM_uint32	      maj_stat          = GSS_S_COMPLETE;
   int		      conf_state        = FALSE;

   (*pp_message)	= NULL;
   (*pp_message_len)	= 0;
   (*pp_conf_state)	= 0;
   (*pp_qop)		= 0;

   (*pp_min_stat)       = MINOR_NO_ERROR;

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, &p_ctx,
			      CTX_MSG_OUT, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (krb5_ctx_desc *) p_ctx;

   if ( p_wrap_token_len==0 )
      RETURN_MIN_MAJ( MINOR_NOT_A_TOKEN, GSS_S_DEFECTIVE_TOKEN );

   /* We need to copy the input token,         */
   /* since SSPI will be decrypting inplace !! */
   token = sy_malloc( p_wrap_token_len );
   if ( token==NULL )
      ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );

   memcpy( token, p_wrap_token, p_wrap_token_len);
   token_len = p_wrap_token_len;

   SecBufDesc.cBuffers  = 2u;
   SecBufDesc.pBuffers  = &(SecBuf[0]);
   SecBufDesc.ulVersion = SECBUFFER_VERSION;
   
   SecBuf[0].BufferType = SECBUFFER_STREAM;
   SecBuf[0].cbBuffer   = (ULONG)token_len;
   SecBuf[0].pvBuffer   = token;

   SecBuf[1].BufferType = SECBUFFER_DATA;
   SecBuf[1].cbBuffer   = 0;
   SecBuf[1].pvBuffer   = NULL;

   fQop = 0;

   sspi_call = "DecryptMessage";
   SecStatus = (fp_SSPI_krb5->DecryptMessage)(
      /* PCtxtHandle    phContext    */  &(ctx->sspi_ctx),
      /* PSecBufferDesc pMessage     */  &SecBufDesc,
      /* ULONG		MessageSeqNo */  0,	/* According to Microsoft */
      /* ULONG          fQOP         */  &fQop );

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "ERR: %s(): %s() failed with %s\n\t(Desc=\"%s\")\n",
			this_Call, sspi_call, symbol, desc ))
      switch( SecStatus ) {
	 case SEC_E_INVALID_HANDLE:	  maj_stat = GSS_S_NO_CONTEXT;		 break;
	 case SEC_E_CONTEXT_EXPIRED:	  maj_stat = GSS_S_CONTEXT_EXPIRED;	 break;
	 case SEC_E_QOP_NOT_SUPPORTED:	  maj_stat = GSS_S_BAD_QOP;		 break;
	 case SEC_E_INVALID_TOKEN:	  maj_stat = GSS_S_DEFECTIVE_TOKEN;	 break;
	 case SEC_E_MESSAGE_ALTERED:	  maj_stat = GSS_S_BAD_SIG;		 break;
	 /* unfortunately, we cannot distinguish DUPLICATE, OLD, UNSEQ or GAP tokens thru SSPI */
	 case SEC_E_OUT_OF_SEQUENCE:	  maj_stat = GSS_S_FAILURE;		 break;
	 case SEC_E_INCOMPLETE_MESSAGE:	  maj_stat = GSS_S_DEFECTIVE_TOKEN;	 break;
	 default:		          maj_stat = GSS_S_FAILURE;		 break;
      }
      ERROR_RETURN( KRB5_MINOR_SSPI(DecryptMessage,SecStatus), maj_stat );
   }

   if ( fQop != KERB_WRAP_NO_ENCRYPT ) {
      conf_state = TRUE;
   }

   /* NOTICE: We may be processing a ZERO-length message here !! */
   if ( SecBuf[1].cbBuffer>0 ) {
      (*pp_message) = sy_malloc( SecBuf[1].cbBuffer );
      if ( (*pp_message)==NULL )
	 ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
      memcpy( (*pp_message), SecBuf[1].pvBuffer, SecBuf[1].cbBuffer );
      (*pp_message_len) = SecBuf[1].cbBuffer;

      if ( (Uchar *)(SecBuf[1].pvBuffer) < token
	   || (Uchar *)(SecBuf[1].pvBuffer) >= token+token_len ){
	 DEBUG_ERR((tf, "ERR: %s(): Returned buffer is not inplace ?!\n", this_Call ))
	 /* DebugBreak(); /* Uh-oh! Do I need to free this memory??     */
	                  /* Normally, SSPI does this operation inplace */
      }
   }

   if ( pp_qop!=NULL        ) { (*pp_qop)	 = (OM_uint32) 0; }
   if ( pp_conf_state!=NULL ) { (*pp_conf_state) = conf_state;    }

   if (maj_stat!=GSS_S_COMPLETE) {
error:
      sy_clear_free( pp_message, (*pp_message_len) );
   }

   sy_clear_free( (void **)&token, token_len );

   return(maj_stat);

} /* krb5_unwrap() */





/*
 * krb5_wrap_size_limit()
 *
 *
 */
OM_uint32
krb5_wrap_size_limit( OM_uint32	 * pp_min_stat,	   /* out */
		      void	 * p_ctx,	   /* in  */
		      int	   p_conf_req,	   /* in  */
		      OM_uint32	   p_qop,	   /* in  */
		      OM_uint32	   p_req_out_size, /* in  */
		      OM_uint32	 * pp_max_input )  /* out */
{
   char		    * this_Call      = "krb5_wrap";
   krb5_ctx_desc    * ctx            = NULL;
   size_t	      len	     = 0;
   size_t	      mic_max_len    = 0;
   size_t	      fix_len	     = 0;
   OM_uint32	      maj_stat	     = GSS_S_COMPLETE;

   (*pp_min_stat)  = MINOR_NO_ERROR;
   (*pp_max_input) = 0;

   /* quick validation of krb5 context handle */
   maj_stat = krb5_check_ctx( pp_min_stat, &p_ctx,
			      CTX_MSG_OUT, this_Call );

   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (krb5_ctx_desc *) p_ctx;

   /* MISSING: we should check the QOP value for validity ...        */
   
   if ( (ctx->service_rec&(GSS_C_CONF_FLAG|GSS_C_INTEG_FLAG))==0 ) {
      RETURN_MIN_MAJ( MINOR_FEATURE_UNAVAILABLE , GSS_S_FAILURE );
   }

   len = ctx->sizes.cbSecurityTrailer + ctx->sizes.cbBlockSize;
   if ( len < p_req_out_size ) {
      (*pp_max_input) = (OM_uint32)(p_req_out_size - len);
   } else {
      (*pp_max_input) = 0;
   }

   return(GSS_S_COMPLETE);

} /* krb5_wrap_size_limit() */
