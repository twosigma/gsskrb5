#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/get_mic.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/get_mic.c#1 $
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
 * gn_gss_get_mic()
 *
 *
 */
OM_uint32
gn_gss_get_mic(
	OM_uint32	 FAR *	pp_min_stat,	/* minor_status		*/
	gss_ctx_id_t		p_in_context,	/* context_handle	*/
	gss_qop_t		p_in_qop,	/* qop_req		*/
	gss_buffer_t		p_in_message,	/* message_buffer	*/
	gss_buffer_t		p_out_token	/* message_token	*/
     )
{
   char              * this_Call = "gn_gss_get_mic";
   gn_context_desc   * ctx       = NULL;
   Uchar	     * mic	 = NULL;
   size_t	       mic_len	 = 0;
   OM_uint32	       maj_stat  = GSS_S_COMPLETE;
   OM_uint32	       min_stat;
   OM_uint32	       lifetime;


   (*pp_min_stat)   = MINOR_NO_ERROR;

   if ( p_out_token==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): Missing MIC_token output gss_buffer_desc !\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CALL_INACCESSIBLE_WRITE|GSS_S_FAILURE );
   }

   p_out_token->value  = NULL;
   p_out_token->length = 0;

   /* NOTICE: We may be processing a ZERO-length message here !! (GSS_C_EMPTY_BUFFER) */
   if ( p_in_message==GSS_C_NO_BUFFER ) {
      DEBUG_ERR((tf, "  E: %s(): Missing input message gss_buffer_desc!\n",
		     this_Call ))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CALL_INACCESSIBLE_READ|GSS_S_FAILURE );
   }

   maj_stat = gn_check_context( pp_min_stat, &p_in_context,
				CTX_MSG_OUT, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (gn_context_desc *)(p_in_context);

   maj_stat = gn_context_time( pp_min_stat, ctx, &lifetime );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

/* Cleanup Alert:  the next call will probably create a */
/*		   dynamically allocated MIC-token 	*/
   maj_stat = (gn_mech[ctx->mech_tag]
	       ->fp_get_mic)( pp_min_stat, ctx->prv_ctx, p_in_qop,
			      p_in_message->value, p_in_message->length,
			      &mic, &mic_len );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   if ( (gn_mech[ctx->mech_tag]->use_raw_tokens) ) {
      /* use raw tokens -- just copy the "inner" token */
      maj_stat = gn_alloc_buffer( pp_min_stat, p_out_token,
			          mic, mic_len, FALSE );
   } else {
      /* add gssapi framing to the inner token */
      maj_stat = gn_compose_gss_token( pp_min_stat, mic, mic_len,
				       ctx->mech_tag, p_out_token );
   }

   if ( maj_stat!=GSS_S_COMPLETE ) {
error:
      if ( p_out_token->value!=NULL && p_out_token->length>0 ) {
	 gn_gss_release_buffer( &min_stat, p_out_token );
      }
   } /* endif (maj_stat!=GSS_S_COMPLETE) */

   if ( mic!=NULL && mic_len!=0 ) {
      maj_stat = (gn_mech[ctx->mech_tag]
	           ->fp_release_token)( &min_stat, &mic, &mic_len );
   }

   return(maj_stat);

} /* gn_gss_get_mic() */
