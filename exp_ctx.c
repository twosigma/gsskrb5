#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/exp_ctx.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/exp_ctx.c#2 $
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


#include "gssmaini.h"


/*
 * gn_gss_export_sec_context()
 *
 *
 */
OM_uint32
gn_gss_export_sec_context(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_ctx_id_t	FAR *	pp_in_context,	/* context_handle	*/
	gss_buffer_t		p_out_buffer	/* interprocess_token	*/
     )
{
   char             * this_Call       = "gn_gss_export_sec_context";
   gn_context_desc  * ctx             = NULL;  /* convenience pointer */
   gn_exp_ctx_desc  * expctx          = NULL;  /* convenience pointer */
   gss_OID            mech_oid        = GSS_C_NO_OID;
   Uchar            * private_buf     = NULL;  /* temporary dynamic buffer */
   Uchar            * buffer          = NULL;  /* returnable dynamic buffer */
   size_t             private_buf_len = 0;
   size_t             buffer_len      = 0;
   size_t             generic_len     = 0;
   size_t             mech_oid_len    = 0;
   gn_mech_tag_et     mech_tag        = MECH_INVALID_TAG;
   OM_uint32	      min_stat;
   OM_uint32          maj_stat        = GSS_S_COMPLETE;

   (*pp_min_stat) = MINOR_NO_ERROR;

   if ( p_out_buffer==GSS_C_NO_BUFFER ) {
      DEBUG_ERR((tf, "  E: %s(): need gss_buffer_t for interprocess token!\n",
		     this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR,
		      GSS_S_CALL_INACCESSIBLE_WRITE|GSS_S_FAILURE );
   }

   p_out_buffer->length = 0;
   p_out_buffer->value  = NULL;

   maj_stat = gn_check_context( pp_min_stat, pp_in_context,
				CTX_TRANSFER, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (gn_context_desc *)(*pp_in_context);

   mech_tag = ctx->mech_tag;
   mech_oid = gn_mech_tag_to_oid( mech_tag );
   if ( mech_oid==GSS_C_NO_OID ) {
      DEBUG_ERR((tf, "  E: %s(): bad mechanism OID in context?!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   mech_oid_len = mech_oid->length; 
   generic_len  = sizeof(gn_exp_ctx_desc);

   maj_stat = (gn_mech[ctx->mech_tag]
	       ->fp_export_sec_context)( pp_min_stat,
					 &(ctx->prv_ctx),
					 &private_buf,
					 &private_buf_len );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   buffer_len = generic_len + mech_oid_len + private_buf_len;
   if ( buffer_len>USHRT_MAX ) {
       DEBUG_ERR((tf, "  E: %s(): Exported context token to large (%ld bytes) for this code!\n",
		      this_Call, (unsigned long) buffer_len ));
       ERROR_RETURN( MINOR_INTERNAL_BUFFER_OVERRUN, GSS_S_FAILURE );
   }

   buffer     = sy_malloc( buffer_len );
   if (buffer==NULL) {
      ERROR_RETURN(MINOR_OUT_OF_MEMORY, GSS_S_FAILURE);
   }

   expctx = (gn_exp_ctx_desc *) buffer;
   memset( expctx, 0, sizeof(*expctx) );

   expctx->magic_cookie = COOKIE_CONTEXT;
   expctx->service_rec  = ctx->service_rec;
   expctx->service_req  = ctx->service_req;
   expctx->token_len    = (Ushort)buffer_len;
   expctx->generic_len  = (Ushort)generic_len;
   expctx->private_len  = (Ushort)private_buf_len;
   expctx->mech_oid_len = (Ushort)mech_oid_len;
   expctx->role         = (Uchar) ctx->role;

   memcpy( &(buffer[generic_len]), mech_oid->elements, mech_oid_len );
   memcpy( &(buffer[generic_len+mech_oid_len]), private_buf, private_buf_len);

   if (maj_stat==GSS_S_COMPLETE) {

      (void) gn_gss_delete_sec_context( &min_stat, pp_in_context, NULL );

      p_out_buffer->value  = buffer;
      p_out_buffer->length = buffer_len;

   } else {
error:
      if ( ctx->prv_ctx==NULL ) {
	 /* this case is dangerous:  we have already exported the */
	 /* private part of the context, but ran into an error    */
	 /* If we don't manage to recreate the "prv_ctx", then    */
	 /* this security context is lost/invalidated.  :-(       */
	 if ( private_buf!=NULL ) {
	    (void) (gn_mech[mech_tag]
			 ->fp_import_sec_context)( &min_stat,
						   private_buf,
						   private_buf_len,
						   &(ctx->prv_ctx) );
	 }

	 if ( ctx->prv_ctx==NULL ) {
	    /* Too bad -- we have lost the context; now bury the rest */
	    (void) gn_gss_delete_sec_context( &min_stat, pp_in_context, NULL );
	 }
      }

      if ( buffer!=NULL ) {
	 /* release the interpocess token buffer in case of an error */
	 sy_clear_free( &buffer, buffer_len );
      }
   }

   if ( private_buf!=NULL && mech_tag!=MECH_INVALID_TAG ) {
      /* always release this temporary buffer of the prv_ctx data */
      (void) (gn_mech[mech_tag]
	          ->fp_release_token)( &min_stat, &private_buf,
						  &private_buf_len );
   }

   return(maj_stat);

} /* gn_gss_export_sec_context() */
