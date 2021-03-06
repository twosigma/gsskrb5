#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/ctx_time.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/ctx_time.c#2 $
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
 * gn_gss_context_time()
 *
 *
 *
 */
OM_uint32
gn_gss_context_time(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_ctx_id_t		p_in_context,	/* context_handle	*/
	OM_uint32	FAR *	pp_lifetime	/* time_rec		*/
     )
{
   char		      * this_Call = "gn_gss_context_time";
   gn_context_desc    * ctx;		  /* convenience pointer */
   OM_uint32		maj_stat  = GSS_S_COMPLETE;


   (*pp_min_stat)    = MINOR_NO_ERROR;

   /* sanity check on return parameter */
   if ( pp_lifetime==NULL ) {
      RETURN_MIN_MAJ( MINOR_NO_ERROR,
		      GSS_S_CALL_INACCESSIBLE_WRITE | GSS_S_FAILURE );
   }

   /* default value for return parameter ("expired") */
   (*pp_lifetime)     = 0;


   /* validate the context handle */
   maj_stat = gn_check_context( pp_min_stat, &p_in_context,
				CTX_TIME, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (gn_context_desc *) p_in_context;

   /* always retrieve the expiration time from the mechanism */ 
   maj_stat = gn_context_time( pp_min_stat, ctx, pp_lifetime );

   return( maj_stat );

} /* gn_gss_context_time() */




/*
 * gn_context_time()
 *
 * provide a consistent security context lifetime processing
 */
OM_uint32
gn_context_time( OM_uint32         * pp_min_stat,
		 gn_context_desc   * p_ctx,
		 OM_uint32	   * pp_lifetime )
{
   OM_uint32    maj_stat = GSS_S_CONTEXT_EXPIRED;

   (*pp_min_stat) = 0;
   (*pp_lifetime) = 0;

   if ( p_ctx->state!=CTX_STATE_EXPIRED ) {    /* once it's expired, it's expired */
      
      maj_stat = (gn_mech[p_ctx->mech_tag]
		  ->fp_context_time)( pp_min_stat,
				      p_ctx->prv_ctx, &(p_ctx->expires_at) );

      if (maj_stat==GSS_S_COMPLETE) {
	 (*pp_lifetime) = gn_remaining_time( p_ctx->expires_at );
	 if ( (*pp_lifetime)==0 ) {
	    maj_stat     = GSS_S_CONTEXT_EXPIRED;
	    p_ctx->state = CTX_STATE_EXPIRED;   /* remember this event */
	 }
      }

   } /* ctx->state!=STATE_EXPIRED */

   return(maj_stat);

} /* gn_context_time() */

