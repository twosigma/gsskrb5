#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/inq_cred.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/inq_cred.c#2 $
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
 * gn_gss_inquire_cred()
 *
 *
 */
OM_uint32
gn_gss_inquire_cred(
	OM_uint32	 FAR *	pp_min_stat,	/* minor_status		*/
	gss_cred_id_t		p_in_cred,	/* cred_handle		*/
	gss_name_t	 FAR *	pp_out_name,	/* name			*/
	OM_uint32	 FAR *	pp_out_lifetime,/* lifetime		*/
	gss_cred_usage_t FAR *	pp_out_usage,	/* cred_usage		*/
	gss_OID_set	 FAR *	pp_out_mechset	/* mechanisms		*/
     )
{
   char              * this_Call  = "gn_gss_inquire_cred";
   gn_cred_desc      * cred       = NULL;
   gn_cred_desc      * tmp_cred   = NULL;  /* dynamic object, release on exit */
   gn_name_desc      * tmp_name   = NULL;  /* dynamic object, release on error */
   gss_OID_set_desc  * mech_set   = GSS_C_NO_OID_SET; /* dynamic object,  */
						      /* release on error */
   Uchar             * iname      = NULL;
   size_t              iname_len  = 0;
   gn_nt_tag_et        nt_tag     = NT_INVALID_TAG;
   gn_cred_usage_et    usage;
   gn_mech_tag_et      mech_tags[GN_MAX_MECH_OIDS+1];
   time_t              expires_at = 0;
   OM_uint32           min_stat;
   OM_uint32           maj_stat   = GSS_S_COMPLETE;
   int                 num_mechs  = 0;

   (*pp_min_stat) = MINOR_NO_ERROR;

   if ( pp_out_name     != NULL )   { (*pp_out_name)     = GSS_C_NO_NAME;     }
   if ( pp_out_lifetime != NULL )   { (*pp_out_lifetime) = 0;                 }
   if ( pp_out_mechset  != NULL )   { (*pp_out_mechset)  = GSS_C_NO_OID_SET;  }

   if ( p_in_cred!=GSS_C_NO_CREDENTIAL ) {

      maj_stat = gn_check_cred( pp_min_stat, &p_in_cred, this_Call );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

      cred = (gn_cred_desc *) p_in_cred;

   } else {
      /* Inquiring attributes of default credential                    */
      /* Since we don't have a credential to query, we need to create  */
      /* a temporary credential first ...                              */
      maj_stat = gn_gss_acquire_cred( pp_min_stat, GSS_C_NO_NAME,
				      GSS_C_INDEFINITE, GSS_C_NO_OID_SET,
			              GSS_C_INITIATE, &tmp_cred, NULL, NULL );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

      cred = tmp_cred;

   }

   maj_stat = (gn_mech[cred->mech_tag]
	           ->fp_inquire_cred)( pp_min_stat, cred->prv_cred,
				       &iname, &iname_len, &nt_tag,
				       &usage, &expires_at );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   if ( pp_out_lifetime!=NULL )
      (*pp_out_lifetime) = gn_remaining_time( expires_at );

   if ( pp_out_usage!=NULL )
      (*pp_out_usage) = usage;

   if ( pp_out_name!=NULL ) {
      maj_stat = gn_create_name( pp_min_stat, nt_tag, cred->mech_tag,
				 iname, iname_len, NULL, 0, &tmp_name );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;
   }

   if ( pp_out_mechset!=NULL ) {
      for ( num_mechs=0 ; cred!=NULL ; cred = cred->next, num_mechs++ ) {
	 mech_tags[num_mechs] = cred->mech_tag;
      }

      maj_stat = gn_create_mech_oid_set( pp_min_stat, mech_tags,
					 num_mechs, &mech_set );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;
   }

error:
   if ( tmp_cred!=NULL )   (void) gn_gss_release_cred( &min_stat, &tmp_cred );
   if ( maj_stat!=GSS_S_COMPLETE ) {

      if (tmp_name!=NULL)  (void) gn_gss_release_name( &min_stat, &tmp_name );
      if (mech_set!=NULL)  (void) gn_gss_release_oid_set( &min_stat, &mech_set );
   
   } else {

      if ( pp_out_name!=NULL )      { (*pp_out_name)    = (gss_name_t)tmp_name; }
      if ( pp_out_mechset!=NULL )   { (*pp_out_mechset) = mech_set;             }

   }

   return(maj_stat);

} /* gn_gss_inquire_cred() */
