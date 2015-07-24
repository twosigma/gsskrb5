#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/del_ctx.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/del_ctx.c#1 $
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
 * gn_gss_delete_sec_context()
 *
 *
 */
OM_uint32
gn_gss_delete_sec_context(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_ctx_id_t	FAR *	pp_io_context,	/* context_handle	*/
	gss_buffer_t		p_out_token	/* output_token		*/
     )
{
   char             * this_Call = "gn_gss_delete_sec_context";
   gn_context_desc  * ctx       = NULL;
   OM_uint32          maj_stat  = GSS_S_COMPLETE;
   OM_uint32          min_stat;

   (*pp_min_stat) = MINOR_NO_ERROR;

   if (p_out_token!=NULL) {
      /* We do not create a delete sec context token! */
      p_out_token->value  = NULL;
      p_out_token->length = 0;
   }

   maj_stat = gn_check_context( pp_min_stat, pp_io_context,
				CTX_DELETE, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (gn_context_desc *) (*pp_io_context);

   ctx->caller_cred   = NULL;  /* READONLY, do not release! */
   ctx->caller_target = NULL;  /* READONLY, do not release! */

   if (ctx->initiator!=NULL)
      (void)gn_gss_release_name( &min_stat, (gss_name_t *)&(ctx->initiator) );

   if (ctx->acceptor!=NULL)
      (void)gn_gss_release_name( &min_stat, (gss_name_t *)&(ctx->acceptor) );

   if (ctx->implicit_cred!=NULL)
      (void)gn_gss_release_cred( &min_stat, (gss_cred_id_t *)&(ctx->implicit_cred) );

   if (ctx->prv_ctx!=NULL
      && ctx->mech_tag!=MECH_INVALID_TAG  &&  (Uint)(ctx->mech_tag)<=mech_last_tag ) {
      (void)(gn_mech[ctx->mech_tag]
	         ->fp_delete_sec_context)( &min_stat, &(ctx->prv_ctx) );
   }

   sy_clear_free( (void **) &ctx, sizeof(*ctx) );

   (*pp_io_context) = ctx;

   return(maj_stat);

} /* gn_gss_delete_sec_context() */
