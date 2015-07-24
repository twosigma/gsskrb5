#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/ws_limit.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/ws_limit.c#2 $
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
 * gn_gss_wrap_size_limit()
 *
 *
 */
OM_uint32
gn_gss_wrap_size_limit(
	OM_uint32	FAR *	pp_min_stat,	  /* minor_status	*/
	gss_ctx_id_t		p_in_context,	  /* context handle	*/
	int			p_in_conf,	  /* conf_req_flag	*/
	gss_qop_t		p_in_qop,	  /* qop_req		*/
	OM_uint32		p_in_req_size,	  /* requested output size*/
	OM_uint32	FAR *	pp_out_max_insize /* maximum input size	*/
     )
{
   char              * this_Call  = "gn_gss_wrap_size_limit";
   gn_context_desc   * ctx        = NULL;
   size_t	       outer_len  = 0;
   OM_uint32           inner_len  = 0;
   size_t	       header_len = 0;
   OM_uint32	       maj_stat   = GSS_S_COMPLETE;
   OM_uint32	       lifetime;

   (*pp_min_stat)   = MINOR_NO_ERROR;

   if ( pp_out_max_insize==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): Missing output parameter pp_out_max_size!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CALL_INACCESSIBLE_WRITE|GSS_S_FAILURE );
   }

   (*pp_out_max_insize) = 0;

   maj_stat = gn_check_context( pp_min_stat, &p_in_context,
				CTX_MSG_OUT, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (gn_context_desc *)(p_in_context);

   maj_stat = gn_context_time( pp_min_stat, ctx, &lifetime );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   if ( (gn_mech[ctx->mech_tag]->use_raw_tokens) ) {

      /* the mechanism does and accounts for framing all by its own */
      inner_len = p_in_req_size;

   } else {

      /* calculate the overhead of the outer gssapi framing    */
      /* the mechanism will only account for its inner framing */

      /* the generic token framing varies in length with the length of  */
      /* the ASN.1 length field of the inner mechanism token            */
      /* we do a 2-step iteration:					     */
      /* 1.) find out the minimum header length (0 bytes inner token)   */
      maj_stat = gn_predict_gss_token_len( pp_min_stat, 0, ctx->mech_tag, &header_len );
      if ( maj_stat!=GSS_S_COMPLETE )
	 return(maj_stat);

      if ( header_len>p_in_req_size ) {
	 (*pp_out_max_insize) = 0;
	 return(GSS_S_COMPLETE);
      }

      /* 2.) subtract the minimum header length from the reqested size and    */
      /*     estimate the header length growth of the ASN.1	  	   */
      /*     (actually, we fail at the border conditions to the safe side)    */
      maj_stat = gn_predict_gss_token_len( pp_min_stat, p_in_req_size - header_len,
					   ctx->mech_tag, &outer_len );
      if ( maj_stat!=GSS_S_COMPLETE )
	 return(maj_stat);

      header_len += (outer_len - p_in_req_size);
      if ( header_len>p_in_req_size ) {
	 (*pp_out_max_insize) = 0;
	 return(GSS_S_COMPLETE);
      }

      inner_len = p_in_req_size - (OM_uint32)header_len;

   }

   /* Finally: have the mechanism account for it's inner token framing */
   maj_stat  = gn_mech[ctx->mech_tag]
	       ->fp_wrap_size_limit( pp_min_stat, ctx->prv_ctx, p_in_conf,
				     p_in_qop, inner_len, pp_out_max_insize );

   return(maj_stat);

} /* gn_gss_wrap_size_limit() */

