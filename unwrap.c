#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/unwrap.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/unwrap.c#2 $
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
 * gn_gss_unwrap()
 *
 *
 */
OM_uint32
gn_gss_unwrap(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_ctx_id_t		p_in_context,	/* context_handle	*/
	gss_buffer_t		p_in_token,	/* input_message_buffer */
	gss_buffer_t		p_out_message,	/* output_message_buffer*/
	int		FAR *	pp_conf_state,	/* conf_state		*/
	gss_qop_t	FAR *	pp_out_qop	/* qop_state		*/
     )
{
   char		     * this_Call        = "gn_gss_unwrap";
   gn_context_desc   * ctx		= NULL;
   Uchar	     * wrap_token	= NULL;
   Uchar	     * message	        = NULL;
   size_t	       wrap_token_len	= 0;
   size_t	       message_len      = 0;
   OM_uint32	       maj_stat		= GSS_S_COMPLETE;
   OM_uint32	       min_stat;
   OM_uint32	       lifetime;
   OM_uint32	       qop	        = 0;
   gn_mech_tag_et      mech_tag	        = MECH_INVALID_TAG;
   int		       conf_state       = FALSE;


   (*pp_min_stat)   = MINOR_NO_ERROR;

   if ( pp_conf_state!=NULL ) { (*pp_conf_state) = FALSE; }
   if ( pp_out_qop   !=NULL ) { (*pp_out_qop)    = FALSE; }

   if ( p_out_message==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): Missing output message gss_buffer_desc !\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR,
		      GSS_S_CALL_INACCESSIBLE_WRITE|GSS_S_FAILURE );
   }

   p_out_message->value  = NULL;
   p_out_message->length = 0;

   if ( p_in_token==GSS_C_NO_BUFFER ) {
      DEBUG_ERR((tf, "  E: %s(): Missing input token gss_buffer_desc!\n",
		     this_Call ))
      RETURN_MIN_MAJ( MINOR_NO_ERROR,
		      GSS_S_CALL_INACCESSIBLE_READ|GSS_S_DEFECTIVE_TOKEN );
   }

   maj_stat = gn_check_context( pp_min_stat, &p_in_context,
				CTX_MSG_IN, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (gn_context_desc *)(p_in_context);

   maj_stat = gn_context_time( pp_min_stat, ctx, &lifetime );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   if ( (gn_mech[ctx->mech_tag]->use_raw_tokens) ) {
      /* this mechanism wants raw tokens, so we don't look at the framing */
      wrap_token     = p_in_token->value;
      wrap_token_len = p_in_token->length;

   } else {
      /* parse the outer gssapi framing to reveal the inner token */

      /* pick the token apart and check the mechanism id field in it */
      maj_stat = gn_parse_gss_token( pp_min_stat, p_in_token, NULL, &mech_tag,
				    &wrap_token, &wrap_token_len );
      if ( maj_stat!=GSS_S_COMPLETE)
	 return(maj_stat);

      if (mech_tag!=ctx->mech_tag) {
	 DEBUG_ERR((tf, "  E: %s(): Wrong mechanism in token header!\n",
			this_Call ));
	 RETURN_MIN_MAJ( MINOR_WRONG_MECHANISM, GSS_S_DEFECTIVE_TOKEN );
      }

      if (maj_stat!=GSS_S_COMPLETE)
	 return(maj_stat);
   }

/* Cleanup Alert:  the next call will probably create a */
/*		   dynamically allocated message buffer	*/
   maj_stat = (gn_mech[ctx->mech_tag]
	       ->fp_unwrap)( pp_min_stat, ctx->prv_ctx, wrap_token, wrap_token_len,
			     &message, &message_len, &conf_state, &qop );

   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   maj_stat = gn_alloc_buffer( pp_min_stat, p_out_message, message, message_len, TRUE );

   if (maj_stat==GSS_S_COMPLETE) {

      if ( pp_out_qop   !=NULL ) { (*pp_out_qop)    = qop;	  }
      if ( pp_conf_state!=NULL ) { (*pp_conf_state) = conf_state; }

   } else {
error:
      if ( p_out_message->value!=NULL && p_out_message->length>0 ) {
	 gn_gss_release_buffer( &min_stat, p_out_message );
      }
   } /* endif (maj_stat!=GSS_S_COMPLETE) */

   if ( message!=NULL && message_len!=0 ) {
      maj_stat = (gn_mech[ctx->mech_tag]
	     ->fp_release_token)( &min_stat, &message, &message_len );
   }

   return(maj_stat);

} /* gn_gss_unwrap() */
