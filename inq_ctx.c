#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/inq_ctx.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/inq_ctx.c#1 $
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


static OM_uint32  gn_update_name( OM_uint32        * pp_min_stat,
				  gn_context_desc  * p_ctx,
				  int                p_usage,
				  gn_name_desc    ** pp_name,
				  gss_name_t       * pp_out_name );

/*
 * gn_gss_inquire_context()
 *
 *
 */
OM_uint32
gn_gss_inquire_context(
	OM_uint32	FAR *	pp_min_stat,	 /* minor_status	*/
	gss_ctx_id_t		p_in_context,	 /* context_handle	*/
	gss_name_t	FAR *	pp_out_ini_name, /* initiator_name	*/
	gss_name_t	FAR *	pp_out_acc_name, /* acceptor_name	*/
	OM_uint32	FAR *	pp_out_lifetime, /* lifetime_rec	*/
	gss_OID		FAR *	pp_out_mech,	 /* mech_type		*/
	OM_uint32	FAR *	pp_out_flags,	 /* ret_flags		*/
	int		FAR *	pp_out_localini, /* locally_initiated	*/
	int		FAR *	pp_out_open	 /* open		*/
     )
{
   char              * this_Call = "gn_gss_inquire_context";
   gn_context_desc   * ctx       = NULL;
   gn_name_desc      * initiator = NULL;
   void		     * cname     = NULL;
   size_t              cname_len = 0;
   gn_nt_tag_et	       nt_tag    = NT_INVALID_TAG;
   gn_name_desc      * acceptor  = NULL;
   int		       open      = FALSE;
   OM_uint32           maj_stat  = GSS_S_COMPLETE;

   (*pp_min_stat) = MINOR_NO_ERROR;

   if ( pp_out_localini!=0 )   { (*pp_out_localini) = 0;             }
   if ( pp_out_open    !=0 )   { (*pp_out_open)     = 0;             }
   if ( pp_out_ini_name!=0 )   { (*pp_out_ini_name) = GSS_C_NO_NAME; }
   if ( pp_out_acc_name!=0 )   { (*pp_out_acc_name) = GSS_C_NO_NAME; }

   maj_stat = gn_check_context( pp_min_stat, &p_in_context,
				CTX_INQUIRE, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   ctx = (gn_context_desc *) p_in_context;

   /* have we originally initiated this security context */
   if ( pp_out_localini
        &&  ctx->role==CTX_ROLE_INITIATOR ) {
      (*pp_out_localini) = 1;
   }

   /* is the security context still pending completion ? */
   if ( ctx->state==CTX_STATE_INITIATING
	     ||  ctx->state==CTX_STATE_ACCEPTING ) {

      open = FALSE;

      if ( pp_out_open!=NULL ) {
	 (*pp_out_open) = open;
      }
      /* we don't return any further information for proto-contexts */
      return(GSS_S_COMPLETE);

   } else {

      open = TRUE;

   }

   /* what mechanism is used for this security context ? */
   if ( pp_out_mech && open!=FALSE ) {
      (*pp_out_mech) = gn_mech_tag_to_oid( ctx->mech_tag );
   }

   /* what flags / context attributes are available */
   if ( pp_out_flags ) {
      (*pp_out_flags) = ctx->service_rec;
   }

   if ( pp_out_ini_name ) {
      maj_stat = gn_update_name( pp_min_stat, ctx, GSS_C_INITIATE,
				 &(ctx->initiator), pp_out_ini_name );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;
   }

   if ( pp_out_acc_name ) {
      maj_stat = gn_update_name( pp_min_stat, ctx, GSS_C_ACCEPT,
				 &(ctx->acceptor), pp_out_acc_name );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;
   }
      
   if ( pp_out_lifetime && open!=FALSE ) {
      maj_stat = gn_context_time( pp_min_stat, ctx, pp_out_lifetime );
      if ( maj_stat==GSS_S_COMPLETE
	   ||  maj_stat==GSS_S_CONTEXT_EXPIRED ) {
	 maj_stat = GSS_S_COMPLETE;
      }
   }

   if ( pp_out_open!=NULL ) { (*pp_out_open) = TRUE; }

   if ( maj_stat!=GSS_S_COMPLETE ) {
      OM_uint32   min_stat;
error:
      if ( pp_out_mech    !=NULL )  { (*pp_out_mech)     = GSS_C_NO_OID; }
      if ( pp_out_lifetime!=NULL )  { (*pp_out_lifetime) = 0;            }
      if ( pp_out_flags   !=NULL )  { (*pp_out_flags)    = 0;            }

      if ( pp_out_ini_name!=NULL  &&  *pp_out_ini_name!=GSS_C_NO_NAME ) {
	 (void) gn_gss_release_name( &min_stat, pp_out_ini_name );
      }

      if ( pp_out_acc_name!=NULL  &&  *pp_out_acc_name!=GSS_C_NO_NAME ) {
	 (void) gn_gss_release_name( &min_stat, pp_out_acc_name );
      }

   } /* endif (maj_stat!=GSS_S_COMPLETE) */

   return(maj_stat);

} /* gn_gss_inquire_context() */



/*
 * gn_update_name()
 *
 *
 */
static OM_uint32
gn_update_name( OM_uint32        * pp_min_stat,
	        gn_context_desc  * p_ctx,
		int                p_usage,
		gn_name_desc    ** pp_name,
		gss_name_t       * pp_out_name )
{
   void          * cname;
   size_t          cname_len;
   OM_uint32       maj_stat = GSS_S_COMPLETE;
   gn_nt_tag_et    nt_tag;

   if ( (*pp_name)==NULL ) {
      maj_stat = gn_mech[p_ctx->mech_tag]
	    ->fp_context_name( pp_min_stat, p_ctx->prv_ctx, p_usage,
			       &nt_tag, &cname, &cname_len );

      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

      maj_stat = gn_create_name( pp_min_stat, nt_tag, p_ctx->mech_tag,
				 cname, cname_len, NULL, 0,
				 pp_name );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

   }
   maj_stat = gn_duplicate_name( pp_min_stat, (*pp_name),
				 (gn_name_desc **) pp_out_name );

error:
   return(maj_stat);

} /* gn_update_name() */
