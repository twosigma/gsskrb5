#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/acpt_ctx.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/acpt_ctx.c#2 $
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


/* MISSING:  Here should be some sort of multimechanism handshake, BUT  */
/*           the first approach will work for single mechanism only ... */

/*
 * gn_gss_accept_sec_context()
 *
 *
 */
OM_uint32
gn_gss_accept_sec_context(
	OM_uint32	FAR *	pp_min_stat,	  /* minor_status	    */
	gss_ctx_id_t	FAR *	pp_io_context,	  /* context_handle	    */
	gss_cred_id_t		p_in_cred,	  /* acceptor_cred_handle   */
	gss_buffer_t		p_in_token,	  /* input_token_buffer     */
	gss_channel_bindings_t	p_in_chanbind,    /* input_channel_bindings */
	gss_name_t	FAR *	pp_out_name,	  /* src_name		    */
	gss_OID		FAR *	pp_out_mech,	  /* mech_type		    */
	gss_buffer_t		p_out_token,	  /* output_token_buffer    */
	OM_uint32	FAR *	pp_out_flags,	  /* ret_flags		    */
	OM_uint32	FAR *	pp_out_lifetime,  /* time_rec		    */
	gss_cred_id_t	FAR *	pp_out_deleg_cred /* delegated_cred_handle  */
     )
{
   char             * this_Call       = "gn_gss_accept_sec_context";
   gn_context_desc  * ctx             = NULL;
   gn_cred_desc     * cred            = NULL;
   gn_cred_t        * deleg_cred_ptr  = NULL;
   gss_buffer_desc    out_token[1];
   Uchar            * inctxtoken      = NULL;
   size_t             inctxtoken_len  = 0;
   Uchar            * outctxtoken     = NULL;   /* dynamic ctx token, always release */
   size_t             outctxtoken_len = 0;
   Uchar	    * src_name	      = NULL;
   size_t	      src_name_len    = 0;
   gn_nt_tag_et       src_nt_tag      = NT_INVALID_TAG;
   gn_mech_tag_et     imech           = MECH_INVALID_TAG;
   OM_uint32          maj_stat        = GSS_S_COMPLETE;
   int                initial_call    = TRUE;

   (*pp_min_stat)    = 0;
   out_token->value  = NULL;
   out_token->length = 0;

   if ( pp_out_mech      !=NULL )    { (*pp_out_mech)       = GSS_C_NO_OID;  }
   if ( pp_out_flags     !=NULL )    { (*pp_out_flags)      = 0;             }
   if ( pp_out_lifetime  !=NULL )    { (*pp_out_lifetime)   = 0;             }
   if ( pp_out_name      !=NULL )    { (*pp_out_name)       = GSS_C_NO_NAME; }

   if ( pp_out_deleg_cred!=NULL )    {
       (*pp_out_deleg_cred) = NULL;
       deleg_cred_ptr	    = (gn_cred_t *)pp_out_deleg_cred;
   }

   if ( p_out_token==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): Missing out_token gss_buffer_desc !\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CALL_INACCESSIBLE_WRITE|GSS_S_FAILURE );
   }
   p_out_token->value  = NULL;
   p_out_token->length = 0;

   maj_stat = gn_proc_ctx_input( pp_min_stat, pp_io_context,
			         CTX_ROLE_ACCEPTOR, p_in_chanbind,
				 p_in_cred, this_Call, p_in_token,
				 &imech, &inctxtoken, &inctxtoken_len );
   if ( maj_stat!=GSS_S_COMPLETE )
      return(maj_stat);

   ctx = (gn_context_desc *)(*pp_io_context);

   if ( ctx!=NULL ) {

      /* We were passed a valid but incomplete context, so we will not   */
      /* destroy it upon exit even in case of an error                   */
      /* The caller will have to call delete_sec_context() to release it */
      initial_call = FALSE;

   } else {

      /* First/initial call to gss_accept_sec_context() */
      ctx = sy_calloc(sizeof(*ctx));
      if (ctx==NULL) {
	 RETURN_MIN_MAJ(MINOR_OUT_OF_MEMORY, GSS_S_FAILURE);
      }

/* Cleaup Alert: ctx is a dynamic object that will have to be released */
/*               in case of an error during first/initial call         */
      ctx->magic_cookie  = COOKIE_CONTEXT;
      ctx->role          = CTX_ROLE_ACCEPTOR;
      ctx->state         = CTX_STATE_ACCEPTING;
      ctx->caller_cred   = (gn_cred_desc *)p_in_cred;
      ctx->service_req   = 0;
      ctx->mech_tag      = imech;
      ctx->caller_target = GSS_C_NO_NAME;
      ctx->ch_bind       = p_in_chanbind;

   }

   /******************************************************************/
   /* To be able to accept a security context, we need suitable      */
   /* ACCEPTING credentials.  These may be explicit (user-supplied)  */
   /* or implicit (we acquire them here and keep them around until   */
   /* the context is established or an error occurs                  */
   /******************************************************************/

   maj_stat = gn_proc_ctx_cred( pp_min_stat, ctx, p_in_cred, imech,
				GSS_C_ACCEPT, this_Call, &cred );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   /*****************************************************************************/
   /* Now we are ready to do real work ...                                      */
   /* trying to establish the security context now                              */
   /*****************************************************************************/
   maj_stat = (gn_mech[imech]
	         ->fp_accept_sec_context)( pp_min_stat, cred->prv_cred,
					   ctx->ch_bind,
					   inctxtoken, inctxtoken_len,
					   &(ctx->prv_ctx),
					   &src_name,
					   &src_name_len,
					   &src_nt_tag,
					   &outctxtoken, &outctxtoken_len,
					   &(ctx->service_rec),
					   &(ctx->expires_at),
					   deleg_cred_ptr );

   if (maj_stat==GSS_S_COMPLETE) {
      
      ctx->state = CTX_STATE_ESTABLISHED;
      /* security context established, memorize the initiator's name */
      maj_stat = gn_create_name( pp_min_stat, src_nt_tag, imech,
				 src_name, src_name_len,
				 NULL, 0, &(ctx->initiator) );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

      if ( pp_out_name!=NULL ) {
	 /* return a copy of the src_name to the acceptor, if he wants one */
	 /* (he really ougth to want one).                                 */
	 maj_stat = gn_duplicate_name( pp_min_stat, ctx->initiator,
				       (gn_name_desc **) pp_out_name );
	 if (maj_stat!=GSS_S_COMPLETE)
	    goto error;
      }

   } else if (maj_stat==GSS_S_CONTINUE_NEEDED ) {

      if ( outctxtoken==NULL || outctxtoken_len==0 ) {
	 DEBUG_ERR((tf, "  E: %s(): CONTINUE_NEEDED but no token ?!?\n",
			this_Call))
	 ERROR_RETURN(MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

   } else {

      goto error;

   }


   /***************************************************************/
   /* If there was a context token returned from the mechanism    */
   /* then we will wrap it with the generic GSS-API token wrapper */
   /***************************************************************/
   if (outctxtoken!=NULL) {

      if ( (gn_mech[ctx->mech_tag]->use_raw_tokens) ) {
	 /* use raw tokens -- just copy the "inner" token */
	 maj_stat = gn_alloc_buffer( pp_min_stat, out_token,
				     outctxtoken, outctxtoken_len, FALSE );
      } else {
	 /* add gssapi framing to inner mechanism token */
         maj_stat = gn_compose_gss_token( pp_min_stat,
					  outctxtoken, outctxtoken_len,
					  ctx->mech_tag, out_token );
      }

      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;
   }

   /* Finally:  fill in all return parameters and prepare for graceful return */
   if ( pp_out_mech!=NULL )
      (*pp_out_mech) = gn_mech_tag_to_oid( ctx->mech_tag );

   if ( pp_out_lifetime!=NULL )
      (*pp_out_lifetime) = gn_remaining_time( ctx->expires_at );

   /* Flag delegated credential in the GSS-API context attributes only when */
   /*  (a) it is requested from the the application caller (output handle)  */
   /*  (b) we were actually able to create a valid credentials handle       */
   /* otherwise we'll switch off the credential delegation flag here        */
   /* WARNING: this logic will only work with the 1-way and 2-way context   */
   /*   security context establishment as specified by rfc-1964             */
   if ( NULL==deleg_cred_ptr || NULL==(*deleg_cred_ptr) )
      ctx->service_rec &= ~GSS_C_DELEG_FLAG;

   if ( pp_out_flags!=NULL )
      (*pp_out_flags) = ctx->service_rec;

   if ( ctx->state==CTX_STATE_ACCEPTING ) {
      if (out_token->value==NULL || out_token->length==0)  {
	 ERROR_RETURN( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
      maj_stat	     = GSS_S_CONTINUE_NEEDED;
   }

   p_out_token->value  = out_token->value;
   p_out_token->length = out_token->length;

   (*pp_io_context) = ctx; /* return context handle */


   if ( maj_stat!=GSS_S_COMPLETE
        &&  maj_stat!=GSS_S_CONTINUE_NEEDED ) {
error:
      if ( initial_call!=FALSE ) {
	 OM_uint32   min_stat2;

	 (void)gn_gss_delete_sec_context( &min_stat2, &ctx, NULL );
	 (*pp_io_context) = GSS_C_NO_CONTEXT;
      }

      if ( out_token->value!=NULL && out_token->length!=0 ) {
	    sy_clear_free( &(out_token->value), out_token->length );
      }
      if ( p_out_token!=NULL ) {
	 p_out_token->value  = NULL;
	 p_out_token->length = 0;
      }
      if ( pp_out_name!=NULL  &&  *pp_out_name!=GSS_C_NO_NAME ) {
	 OM_uint32   min_stat2;

	 (void) gn_gss_release_name( &min_stat2, pp_out_name );
      }
      if ( deleg_cred_ptr!=NULL && *deleg_cred_ptr!=NULL ) {
	  OM_uint32  min_stat2;
	  gn_release_cred( &min_stat2, deleg_cred_ptr );
      }
      if ( pp_out_flags!=NULL )
          (*pp_out_flags) = 0;
   }

   if ( outctxtoken!=NULL && imech!=MECH_INVALID_TAG ) {
      OM_uint32 min_stat2, maj_stat2;

      maj_stat2 = (gn_mech[imech]
	             ->fp_release_token)(&min_stat2, &outctxtoken, &outctxtoken_len);
      if ( maj_stat2!=GSS_S_COMPLETE
	   && ( maj_stat==GSS_S_COMPLETE || maj_stat==GSS_S_CONTINUE_NEEDED ) ) {
	 maj_stat       = maj_stat2;
	 (*pp_min_stat) = min_stat2;
      }
   }

   if ( (*pp_min_stat)!=0 && pp_out_mech!=NULL ) {
      /* return a valid mechanism OID with every minor_status!=0 */
      if ( imech!=MECH_INVALID_TAG ) {
	 /* There actually was a specific mechanism involved */
	 (*pp_out_mech) = gn_mech_tag_to_oid( imech );
      } else {
	 /* Fall back on the default/first registered mechanism OID */
	 (*pp_out_mech) = gn_mech_tag_to_oid( MECH_PRIVATE_1 );
      }
   }

   return(maj_stat);

} /* gn_gss_accept_sec_context() */
