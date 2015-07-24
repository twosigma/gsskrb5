#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/init_ctx.c#3 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/init_ctx.c#3 $
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



/* MISSING:  Here should be some sort of multimechanism negotiation, BUT  */
/*           the first approach will work for single mechanism only ... */

/*
 * gn_gss_init_sec_context()
 *
 *
 */
OM_uint32
gn_gss_init_sec_context(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_cred_id_t		p_in_cred,	/* initiator_cred_handle */
	gss_ctx_id_t	FAR *	pp_io_context,	/* context_handle	*/
	gss_name_t		p_in_target_name, /* target_name	*/
	gss_OID			p_in_mech,	/* mech_type		*/
	OM_uint32		p_in_flags,	/* req_flags		*/
	OM_uint32		p_in_lifetime,	/* time_req		*/
	gss_channel_bindings_t	p_in_chanbind,	/* input_chan_bindings	*/
	gss_buffer_t		p_in_token,	/* input_token		*/
	gss_OID		FAR *	pp_out_mech,	/* actual_mech_type	*/
	gss_buffer_t		p_out_token,	/* output_token		*/
	OM_uint32	FAR *	pp_out_flags,	/* ret_flags		*/
	OM_uint32	FAR *	pp_out_lifetime	/* time_rec		*/
     )
{
   char             * this_Call       = "gn_gss_init_sec_context";
   gn_context_desc  * ctx             = NULL;
   gn_cred_desc     * cred            = NULL;
   gn_name_desc     * target          = NULL;
   gss_buffer_desc    out_token[1];
   Uchar            * inctxtoken      = NULL;
   size_t             inctxtoken_len  = 0;
   Uchar            * outctxtoken     = NULL;   /* dynamic ctx token, always release */
   size_t             outctxtoken_len = 0;
   gn_mech_tag_et     mech_tag        = MECH_INVALID_TAG;
   gn_mech_tag_et     imech           = MECH_INVALID_TAG;
   OM_uint32          maj_stat        = GSS_S_COMPLETE;
   int                initial_call    = FALSE;

   (*pp_min_stat)    = 0;
   out_token->value  = NULL;
   out_token->length = 0;

   if ( pp_out_mech     !=NULL )    { (*pp_out_mech)     = GSS_C_NO_OID; }
   if ( pp_out_flags    !=NULL )    { (*pp_out_flags)    = 0;                }
   if ( pp_out_lifetime !=NULL )    { (*pp_out_lifetime) = 0;                }

   if ( p_out_token==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): Missing out_token handle !\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CALL_INACCESSIBLE_WRITE|GSS_S_FAILURE );
   }
   p_out_token->value  = NULL;
   p_out_token->length = 0;

   /* simple validation of the supplied target name (a required parameter) */
   target = (gn_name_desc *) p_in_target_name;
   if ( target==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): target name missing!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_TARGET, GSS_S_BAD_NAME );
   }

   maj_stat = gn_check_name( pp_min_stat, &p_in_target_name, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);


   maj_stat = gn_proc_ctx_input( pp_min_stat, pp_io_context,
				 CTX_ROLE_INITIATOR, p_in_chanbind,
				 p_in_cred, this_Call, p_in_token,
				 &imech, &inctxtoken, &inctxtoken_len );

   if ( maj_stat!=GSS_S_COMPLETE )
      return(maj_stat);

   /* check whether this is the initial or the continuation call */
   ctx = (gn_context_desc *)(*pp_io_context);

   if ( ctx!=NULL ) {

      /* We were passed a valid but incomplete context, so we will not   */
      /* destroy it upon exit even in case of an error                   */
      /* The caller will have to call delete_sec_context() to release it */
      if ( ctx->caller_target!=target ) {
	 DEBUG_ERR((tf, "  E: %s(): different target name since last call!\n",
			this_Call))
	 RETURN_MIN_MAJ( MINOR_TARGET_CHANGED, GSS_S_FAILURE );
      }

      if ( ctx->service_req!=p_in_flags ) {
	 DEBUG_ERR((tf, "  E: %s(): different service options since last call!\n",
			this_Call))
	 RETURN_MIN_MAJ( MINOR_FLAGS_CHANGED, GSS_S_FAILURE );
      }

      if ( p_in_mech!=GSS_C_NO_OID ) {
         maj_stat = gn_oid_to_mech_tag( pp_min_stat, p_in_mech, &mech_tag );
         if (maj_stat!=GSS_S_COMPLETE)
	    goto error;
	 if ( mech_tag!=ctx->mech_tag ) {
	    DEBUG_ERR((tf, "  E: %s(): different mechanism OID since last call!\n",
			   this_Call))
	    RETURN_MIN_MAJ( MINOR_MECH_CHANGED, GSS_S_FAILURE );
	 }
      }

      if ( imech!=ctx->mech_tag ) {
	 DEBUG_ERR((tf, "  E: %s(): input token doesn't match mechanism of context!\n",
			this_Call))
	 RETURN_MIN_MAJ( MINOR_MECH_CHANGED, GSS_S_FAILURE );
      }

   } else { /* else    ctx==NULL */

      initial_call = TRUE;

      /* First/initial call to gss_init_sec_context() */
      if (p_in_mech!=GSS_C_NO_OID) {
         maj_stat = gn_oid_to_mech_tag( pp_min_stat, p_in_mech, &mech_tag );
         if (maj_stat!=GSS_S_COMPLETE)
	    goto error;
      }
	 
      ctx = sy_calloc(sizeof(*ctx));
      if (ctx==NULL) {
	 RETURN_MIN_MAJ(MINOR_OUT_OF_MEMORY, GSS_S_FAILURE);
      }
      ctx->magic_cookie  = COOKIE_CONTEXT;
      ctx->role          = CTX_ROLE_INITIATOR;
      ctx->state         = CTX_STATE_INITIATING;
      ctx->caller_cred   = (gn_cred_desc *)p_in_cred;
      ctx->service_req   = p_in_flags;
      ctx->mech_tag      = mech_tag;
      ctx->caller_target = target;
      ctx->ch_bind       = p_in_chanbind;

   } /* ctx==NULL */


   /******************************************************************/
   /* To be able to create a security context, we need suitable      */
   /* INITIATING credentials.  These may be explicit (user-supplied) */
   /* or implicit (we acquire them here and keep them around until   */
   /* the context is established or an error occurs                  */
   /******************************************************************/

   maj_stat = gn_proc_ctx_cred( pp_min_stat, ctx, p_in_cred, mech_tag,
				GSS_C_INITIATE, this_Call, &cred );

   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   /********************************************************/
   /* Ok, the credentials look good so far, now let's try  */
   /* to establish the security context                    */
   /********************************************************/

   if ( ctx->acceptor==NULL ) {
      /* actually, we need to canonicalize the target/acceptor name now */
      maj_stat = gn_canonicalize_name( pp_min_stat, target,
				       ctx->mech_tag, &(ctx->acceptor) );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;
   }

   /*****************************************************************************/
   /* Now we are ready to do real work ...                                      */
   /* trying to establish the security context now                              */
   /*****************************************************************************/
   /* Implementation NOTE for multimechanisms:					*/
   /*   The code here always uses the first credential element and ignores      */
   /*   the rest.  This is because the base gssapi spec and therefore most      */
   /*   implementations don't support negotiation. gss_accept_sec_context() may */
   /*   fail in unexpected ways and most applications shut down the connection  */
   /*   upon an error from gss_accept_sec_context() anyway.                     */
   /*   The most sensible behaviour is to fail consistently in the same way!    */
   /*****************************************************************************/

   maj_stat = (gn_mech[ctx->mech_tag]
	         ->fp_init_sec_context)( pp_min_stat, cred->prv_cred,
					 ctx->ch_bind,
					 ctx->acceptor->name,
					 ctx->acceptor->name_len,
					 inctxtoken, inctxtoken_len,
					 ctx->service_req,
					 &(ctx->prv_ctx),
					 &outctxtoken, &outctxtoken_len,
					 &(ctx->service_rec),
					 &(ctx->expires_at) );
   if (maj_stat==GSS_S_COMPLETE) {
      
      ctx->state = CTX_STATE_ESTABLISHED;

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

   if ( pp_out_flags!=NULL )
      (*pp_out_flags) = ctx->service_rec;

   if ( ctx->state==CTX_STATE_INITIATING ) {
      if (out_token->value==NULL || out_token->length==0)  {
	 ERROR_RETURN( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
      maj_stat	     = GSS_S_CONTINUE_NEEDED;
   }

   p_out_token->value  = out_token->value;
   p_out_token->length = out_token->length;

   (*pp_io_context)    = ctx;  /* return context handle */

   if ( maj_stat!=GSS_S_COMPLETE
        &&  maj_stat!=GSS_S_CONTINUE_NEEDED ) {
error:
      if ( initial_call!=FALSE ) {
	 OM_uint32   min_stat;

	 (void)gn_gss_delete_sec_context( &min_stat, &ctx, NULL );
	 (*pp_io_context) = GSS_C_NO_CONTEXT;
      }

      if ( out_token->value!=NULL && out_token->length!=0 ) {
	    sy_clear_free( &(out_token->value), out_token->length );
      }
      if ( p_out_token!=NULL ) {
	 p_out_token->value  = NULL;
	 p_out_token->length = 0;
      }
   }

   if ( outctxtoken!=NULL ) {
      OM_uint32 min_stat2, maj_stat2;

      maj_stat2 = (gn_mech[ctx->mech_tag]
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

} /* gn_gss_init_sec_context() */



/*
 * gn_check_context()
 *
 * This routine does NOT check the context lifetime or STATE_EXPIRED!
 * this must be done in the individual context-level calls where
 * applicable/necessary.
 */
OM_uint32
gn_check_context( OM_uint32     * pp_min_stat,
		  gss_ctx_id_t  * pp_ctx,
		  int             p_flags,
		  char          * this_Call )
{
   gn_context_desc   * ctx;
   OM_uint32           maj_stat = GSS_S_COMPLETE;

   (*pp_min_stat) = MINOR_NO_ERROR;

   if ( pp_ctx==NULL || *pp_ctx==NULL ) {
      if ( (p_flags&CTX_ESTABLISH)!=0  &&  pp_ctx!=NULL )
	 return(maj_stat);
      DEBUG_ERR((tf, "  E: %s(): missing context handle!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CALL_INACCESSIBLE_READ|GSS_S_NO_CONTEXT );
   }

   ctx = (gn_context_desc *) (*pp_ctx);

   if  ( ctx->magic_cookie!=COOKIE_CONTEXT ) {
      DEBUG_ERR((tf, "  E: %s(): bad context handle!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_NO_CONTEXT );
   }

   if ( ctx->role!=CTX_ROLE_INITIATOR && ctx->role!=CTX_ROLE_ACCEPTOR ) {
      DEBUG_ERR((tf, "  E: %s(): invalid role for context!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_NO_CONTEXT );
   }


   if ( (p_flags&CTX_DELETE)==0 ) {

      if ( (ctx->mech_tag)==MECH_INVALID_TAG
	   ||  (Uint)(ctx->mech_tag)>mech_last_tag ) {
	 DEBUG_ERR((tf, "  Internal Error: %s(): invalid mechanism tag!\n",
			this_Call ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

      /* if it's not a DELETE operation, check the context state */

      switch( ctx->state ) {

	 /* Operations permitted with a proto-context (indicated by       */
         /* the state INITIATING or ACCEPTING are(besides delete):        */
	 /*     (1) continuation calls to init/accept_sec_context()       */
	 /*     (2) message protection when GSS_C_PROT_READY is available */
      case CTX_STATE_INITIATING:
      case CTX_STATE_ACCEPTING:
	 if ( (p_flags&(CTX_ESTABLISH|CTX_INQUIRE))!=0
	      ||  ( (p_flags&CTX_MSG_OUT)!=0
	            && ((ctx->service_rec)&GSS_C_PROT_READY_FLAG)!=0 ) ) {
	    break;
	 }
	 DEBUG_ERR((tf, "  E: %s(): invalid with proto-context!\n",
		        this_Call))
	 RETURN_MIN_MAJ( MINOR_INCOMPLETE_CONTEXT, GSS_S_NO_CONTEXT );


	 /* Operations permitted with an established security context are: */
	 /* INQUIRE, MSG_IN, MSG_OUT, TIME, TRANSFER			   */
      case CTX_STATE_ESTABLISHED:
	 if ( (p_flags&(CTX_INQUIRE|CTX_MSG_IN
			|CTX_MSG_OUT|CTX_TIME|CTX_TRANSFER))!=0 ) { 
	    break;
	 }
	 DEBUG_ERR((tf, "  E: %s(): invalid for fully established context!\n",
			this_Call))
	 RETURN_MIN_MAJ( MINOR_ESTABLISHED_CONTEXT, GSS_S_FAILURE );


	 /* Operations permitted with an expired security context are:    */
	 /* INQUIRE, TIME, TRANSFER					  */
      case CTX_STATE_EXPIRED:
	 if ( (p_flags&(CTX_INQUIRE|CTX_TRANSFER))!=0 ) {
	    break;
	 }
	 if ( (p_flags&CTX_TIME)!=0 ) {
	    DEBUG_ERR((tf, "  E: %s(): invalid for an expired context!\n",
			   this_Call))
	 }
	 RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CONTEXT_EXPIRED );


	 /* No operations allowed on a context that failed to established */
	 /* or otherwise turned bad                                       */
      case CTX_STATE_ERROR:
	 DEBUG_ERR((tf, "  E: %s(): context is unusable!\n",
			this_Call))
	 RETURN_MIN_MAJ( MINOR_DEAD_CONTEXT, GSS_S_FAILURE );


	 /* huh, how did you get here? */
      default:
	 DEBUG_ERR((tf, "  E: %s(): invalid context state!\n", this_Call))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_NO_CONTEXT );

      } /* end switch(ctx->state) */

   } /* endif (pflags&CTX_DELETE)!=0 */

   return(maj_stat);

} /* gn_check_context() */



/*
 * gn_proc_ctx_input()
 *
 *
 */
OM_uint32
gn_proc_ctx_input( OM_uint32		   * pp_min_stat,
		   gss_ctx_id_t		   * pp_io_context,
		   gn_ctx_role_et	     p_role,
		   gss_channel_bindings_t    p_in_chanbind,
		   gss_cred_id_t	     p_in_cred,
		   char			   * this_Call,
		   gss_buffer_t		     p_in_token,
		   gn_mech_tag_et	   * pp_mech,
		   Uchar	          ** pp_ctxtoken,
		   size_t		   * pp_ctxtoken_len )
{
   gn_context_desc   * ctx      = NULL;
   OM_uint32           maj_stat = GSS_S_COMPLETE;


   /* simple validation of the security context handle */
   maj_stat = gn_check_context( pp_min_stat, pp_io_context,
			        CTX_ESTABLISH, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ctx = (gn_context_desc *) (*pp_io_context) ;

   if ( p_role==CTX_ROLE_INITIATOR  &&  ctx==NULL ) {

      if ( p_in_token!=NULL && p_in_token->length!=0 ) {
	 DEBUG_STRANGE((tf, "  S: %s(): input token for initial call ??\n",
			    this_Call ))
	 RETURN_MIN_MAJ( MINOR_NO_TOKEN_EXPECTED, GSS_S_FAILURE );
      }
      
   } else {

      /* For gss_accept_sec_context() (=ROLE_ACCEPTOR), an input token */
      /*   is always required.					       */
      /* For gss_init_sec_context() (=ROLE_INITIATOR),  an input token */
      /* is only required for continuation calls.		       */
      if ( p_in_token==NULL ) {
	 DEBUG_ERR((tf, "  E: %s(): Missing in_token parameter\n",
			this_Call))
	 RETURN_MIN_MAJ( MINOR_NO_ERROR,
			 GSS_S_CALL_INACCESSIBLE_READ|GSS_S_FAILURE );
      }

      if ( p_in_token->value==NULL || p_in_token->length==0 ) {
	 DEBUG_ERR((tf, "  E: %s(): Invalid or empty in_token parameter\n",
		        this_Call))
	 RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_TOKEN );
      }

      /* For every call to gss_accept_sec_context() a context level token */
      /* from the initiator must be supplied                              */
      /* NOTICE: Although the GSS-API spec only requires the special      */
      /*         ASN.1-style framing ONLY for the initial context token   */
      /*         we deliberately use it for *all* context tokens          */

      /* pick the token apart and check the mechanism id field in it */
      maj_stat = gn_parse_gss_token( pp_min_stat, p_in_token, NULL, pp_mech,
				     pp_ctxtoken, pp_ctxtoken_len );
      if (maj_stat!=GSS_S_COMPLETE)
	 return(maj_stat);

      if ( (gn_mech[(*pp_mech)]->use_raw_tokens) ) {
	 /* use raw tokens -- don't parse away the gssapi framing at this level */
	 (*pp_ctxtoken)     = p_in_token->value;
	 (*pp_ctxtoken_len) = p_in_token->length;
      }

      if ( ctx!=NULL ) {
	 /* Perform several plausibility checks on the supplied    */
	 /* input arguments to the gss_(init/accept)_sec_context() */
	 /* functions.                                             */

	 if ( (ctx->mech_tag) != (*pp_mech) ) {
	    DEBUG_ERR((tf, "  E: %s(): mechanism id differs from previous token!\n",
			   this_Call ))
	    RETURN_MIN_MAJ( MINOR_MECH_CHANGED, GSS_S_FAILURE );
	 }

	 if ( ctx->ch_bind!=p_in_chanbind ) {
	    /* NOTICE: I personally don't really care that much for channel bindings;     */
	    /*         this check should be more thorough for somebody that does care ... */
	    DEBUG_ERR((tf, "  E: %s(): different channel bindings since last call!\n",
			   this_Call))
	    RETURN_MIN_MAJ( MINOR_CHANBIND_CHANGED, GSS_S_FAILURE );
	 }

	 if ( ctx->caller_cred!=(gn_cred_desc *)p_in_cred ) {
	    DEBUG_ERR((tf, "  E: %s(): different credential since last call!\n",
			   this_Call))
	    RETURN_MIN_MAJ( MINOR_CRED_CHANGED, GSS_S_FAILURE );
	 }

	 if ( (p_role != ctx->role)
	      || ( p_role==CTX_ROLE_ACCEPTOR && ctx->state!=CTX_STATE_ACCEPTING )
	      || ( p_role==CTX_ROLE_INITIATOR && ctx->state!=CTX_STATE_INITIATING ) ) {

	    DEBUG_ERR((tf, "  E: %s(): valid but wrong context handle()!\n",
				    this_Call))
	    RETURN_MIN_MAJ( MINOR_WRONG_CONTEXT, GSS_S_FAILURE );
	 }

      } /* ctx!=NULL */

   }

   return(maj_stat);

} /* gn_proc_ctx_input() */




/*
 * gn_proc_ctx_cred()
 *
 *
 */
OM_uint32
gn_proc_ctx_cred( OM_uint32	    *  pp_min_stat,
		  gn_context_desc   *  p_ctx,
		  gss_cred_id_t        p_in_cred,
		  gn_mech_tag_et       p_mech,
		  gss_cred_usage_t     p_usage,
		  char		    *  this_Call,
		  gn_cred_desc      ** pp_cred )
{
   OM_uint32           maj_stat = GSS_S_COMPLETE;
   gn_cred_desc     ** lpcred   = NULL;   /* dynamic object */
   gn_cred_desc      * tcred    = NULL;   /* temporary convenience pointer */
   gss_OID_set_desc  * pmechs   = NULL;
   gss_OID_set_desc    mech_set;

   (*pp_min_stat) = 0;
   (*pp_cred)     = NULL;

   if ( p_ctx->cred_element==NULL ) {

      /* First time that we are here (i.e. initial context establishment call) */

      if ( p_in_cred==GSS_C_NO_CREDENTIAL ) {
	 
	 /* No caller-supplied credentials, we need to create suitable */
	 /* default credentials here before we can continue            */
	 lpcred = &(p_ctx->implicit_cred);

	 if ( p_mech==MECH_INVALID_TAG ) {
	    /* no mechanism provided - request default credentials */
	    pmechs              = GSS_C_NO_OID_SET;
	 } else {
	    mech_set.count      = 1;
	    mech_set.elements   = gn_mech_tag_to_oid(p_mech);
	    pmechs		= &mech_set;
	 }

	 maj_stat = gn_gss_acquire_cred( pp_min_stat, GSS_C_NO_NAME,
				         GSS_C_INDEFINITE, pmechs,
					 p_usage, lpcred, NULL, NULL );

	 if (maj_stat!=GSS_S_COMPLETE)
	    goto error;

	 if ( p_mech==MECH_INVALID_TAG ) {
	    /* No requirements: pick the first credential element */
	    p_ctx->cred_element = (*lpcred);

	 } else {

	    /* Search for a credential element matching the request */
	    for ( tcred=(*lpcred) ; tcred!=NULL ; tcred=tcred->next ) {
	       if ( p_mech==tcred->mech_tag ) {
		  p_ctx->cred_element = tcred;
		  break;
	       }
	    }

	    if ( p_ctx->cred_element == NULL ) {
	       ERROR_RETURN( MINOR_NO_CRED_FOR_MECH, GSS_S_NO_CRED );
	    }

	 }

      } else {

	 /* Caller provided a credentials handle for us -- validate the handle */
	 maj_stat = gn_check_cred( pp_min_stat, &p_in_cred, this_Call );
	 if ( maj_stat!=GSS_S_COMPLETE )
	    goto error;

	 if ( p_mech==MECH_INVALID_TAG ) {
	    /* No specific mechanism requirement, pick the first cred element */
	    p_ctx->cred_element = p_in_cred;

	 } else {

	    /* Specific mechanism requested, look for matching */
	    /* credentials element			       */
	    for ( tcred=p_in_cred ; tcred!=NULL ; tcred=tcred->next ) {
	       if ( p_mech==tcred->mech_tag ) {
		  (p_ctx->cred_element) = tcred;
		  break;
	       }
	    }

	    if ( p_ctx->cred_element == NULL ) {
	       ERROR_RETURN( MINOR_NO_CRED_FOR_MECH, GSS_S_NO_CRED );
	    }

	 }

      }

   } /* endif ((p_ctx->cred_element)==NULL ) */

   (*pp_cred) = (p_ctx->cred_element);

   /* We need to check usage, at least for the user-supplied credentials */
   if ( (*pp_cred)->usage!=p_usage && (*pp_cred)->usage!=GSS_C_BOTH ) {
      DEBUG_ERR((tf, "  E: %s(): credentials not suitable for %s!\n",
	             this_Call,
		     (p_usage==GSS_C_ACCEPT) ? "accepting" : "initiating"))
      ERROR_RETURN( MINOR_WRONG_CREDENTIAL, GSS_S_NO_CRED );
   }

   /* verify that the mechanism ids match for context and credentials */
   if ( p_ctx->mech_tag==MECH_INVALID_TAG ) {

      p_ctx->mech_tag = (*pp_cred)->mech_tag;

   } else if ( p_ctx->mech_tag!=(*pp_cred)->mech_tag ) {
      DEBUG_ERR((tf, "  E: %s(): credentials don't match mechanism of context!\n",
		     this_Call))
      ERROR_RETURN( MINOR_WRONG_CREDENTIAL, GSS_S_NO_CRED );
   }

#if 0
   /* MISSING: this plausibility check accomodates for the restrictions of */
   /*          the current simplified implementation                       */
   if ( (*pp_cred)->next!=NULL ) {
      DEBUG_ERR((tf, "  E: %s(): support for multimechanism not yet implemented!\n",
	             this_Call))
      ERROR_RETURN( MINOR_NOT_YET_IMPLEMENTED, GSS_S_FAILURE );
   }
#endif

   if ( maj_stat!=GSS_S_COMPLETE ) {
error:  /* Cleanup of credentials via ctx-structure must be done by CALLER !! */
      if  ( lpcred!=NULL ) {
	 OM_uint32   min_stat;

	 gn_gss_release_cred( &min_stat, (gss_cred_id_t *)lpcred );
	 p_ctx->cred_element = NULL;
	 (*pp_cred)          = NULL;
      }

   }

   return(maj_stat);

} /* gn_proc_ctx_cred() */
