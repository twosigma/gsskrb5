#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/imp_ctx.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/imp_ctx.c#2 $
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
 * gn_gss_import_sec_context()
 *
 *
 */
OM_uint32
gn_gss_import_sec_context(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_buffer_t		p_in_token,	/* interprocess_token	*/
	gss_ctx_id_t	FAR *	pp_out_context	/* context_handle	*/
     )
{
   char             * this_Call = "gn_gss_import_sec_context";
   gn_context_desc  * ctx             = NULL;  /* convenience pointer   */
   gn_exp_ctx_desc    impctx;		       /* convenience structure */
   gss_OID_desc       mech_oid;                /* convenience structure */
   Uchar            * private_buf     = NULL;  /* convenience pointer */
   Uchar            * buffer          = NULL;  /* convenience pointer */
   size_t             buffer_len      = 0;
   gn_mech_tag_et     mech_tag	      = MECH_INVALID_TAG;
   OM_uint32	      min_stat;
   OM_uint32          maj_stat        = GSS_S_COMPLETE;

   (*pp_min_stat) = MINOR_NO_ERROR;

   if ( pp_out_context==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): missing gss_ctx_id_t handle!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR,
		      GSS_S_CALL_INACCESSIBLE_WRITE | GSS_S_FAILURE );
   }

   (*pp_out_context) = GSS_C_NO_CONTEXT;

   if ( p_in_token==GSS_C_NO_BUFFER
	||  p_in_token->length==0  ||  p_in_token->value==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): missing gss_buffer_t interprocess token!\n",
		     this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR,
		      GSS_S_CALL_INACCESSIBLE_WRITE|GSS_S_FAILURE );
   }

   if ( p_in_token->length<sizeof(impctx) ) {
      DEBUG_ERR((tf, "  E: %s(): truncated interprocess token?\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );
   }

   memcpy( &impctx, p_in_token->value, sizeof(impctx) );
   buffer_len = impctx.mech_oid_len + impctx.private_len + impctx.generic_len;
   if ( impctx.magic_cookie!=COOKIE_CONTEXT
        ||  buffer_len!=p_in_token->length
	||  buffer_len!=impctx.token_len ) {
      DEBUG_ERR((tf, "  E: %s(): invalid or corrupted token?\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );
   }

   buffer            = (Uchar *)   p_in_token->value;
   mech_oid.length   = (OM_uint32) impctx.mech_oid_len;
   mech_oid.elements = (void *)   &(buffer[sizeof(impctx)]);
   private_buf       = (Uchar *)  &(buffer[sizeof(impctx) + impctx.mech_oid_len]);

   maj_stat = gn_oid_to_mech_tag( pp_min_stat, &mech_oid, &mech_tag );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   if (mech_tag==MECH_INVALID_TAG) {
      DEBUG_ERR((tf, "  E: %s() unknown mech tag in exported context\n",
	             this_Call))
      RETURN_MIN_MAJ( MINOR_UNKNOWN_MECH_OID, GSS_S_DEFECTIVE_TOKEN );
   }

/* Cleanup alert!   The next call creates a dynamic object */
   ctx = sy_calloc( sizeof(*ctx) );
   if ( ctx==NULL ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   ctx->magic_cookie = COOKIE_CONTEXT;
   ctx->state	     = CTX_STATE_ESTABLISHED;   /* this is plain guessing !! */
   ctx->mech_tag     = mech_tag;

   ctx->service_rec  =		        impctx.service_rec;
   ctx->service_req  =		        impctx.service_req;
   ctx->role	     = (gn_ctx_role_et) impctx.role;

   maj_stat = (gn_mech[mech_tag]
		 ->fp_import_sec_context)( &min_stat,
					   private_buf,
					   impctx.private_len,
					   &(ctx->prv_ctx) );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   
   if ( maj_stat==GSS_S_COMPLETE ) {

      (*pp_out_context) = ctx;

   } else {
error:
      if ( ctx!=NULL ) {
	 (void) gn_gss_delete_sec_context( &min_stat, (gss_ctx_id_t *)&ctx, NULL );
      }

   }

   return(maj_stat);

} /* gn_gss_import_sec_context() */
