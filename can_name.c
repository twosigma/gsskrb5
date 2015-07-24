#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/can_name.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/can_name.c#1 $
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
 * gn_gss_canonicalize_name()
 *
 *
 */
OM_uint32
gn_gss_canonicalize_name(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_name_t		p_in_name,	/* input_name		*/
	gss_OID			p_in_mech,	/* mechanism_type	*/
	gss_name_t	FAR *	pp_out_name	/* output_name		*/
     )
{
   char            * this_Call = " gn_gss_canonicalize_name(): ";
   OM_uint32         maj_stat;
   gn_mech_tag_et    mech_tag;
   char            * tmp_prname     = NULL;   /* needs cleanup ! */
   size_t            tmp_prname_len = 0;
   void            * tmp_name       = NULL;   /* needs cleanup ! */
   size_t	     tmp_name_len   = 0;
   gn_name_desc    * pname;
   gn_name_desc    * outname;
   gn_name_desc      gn_name;

   (*pp_min_stat)  = 0;
   (*pp_out_name)  = GSS_C_NO_NAME;
   memset( &gn_name, 0, sizeof(gn_name) );

   maj_stat = gn_check_name( pp_min_stat, &p_in_name, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   pname           = (gn_name_desc *)p_in_name;

   maj_stat = gn_oid_to_mech_tag( pp_min_stat, p_in_mech, &mech_tag );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   if ( mech_tag==MECH_INVALID_TAG ) {
      RETURN_MIN_MAJ( MINOR_UNKNOWN_MECH_OID, GSS_S_BAD_MECH );
   }

   maj_stat = gn_canonicalize_name( pp_min_stat, pname, mech_tag, &outname );
   (*pp_out_name) = outname;

   return(maj_stat);

} /* gn_gss_canonicalize_name() */




/*
 * gn_canonicalize_name():
 *
 */
OM_uint32
gn_canonicalize_name(
	OM_uint32		* pp_min_stat,
	gn_name_desc            * p_in_name,
	gn_mech_tag_et            p_mech_tag,
	gn_name_desc           ** pp_out_name
     )
{
   char            * this_Call = " gn_canonicalize_name(): ";
   OM_uint32         maj_stat;
   char            * tmp_prname     = NULL;   /* needs cleanup ! */
   size_t            tmp_prname_len = 0;
   Uchar           * tmp_name       = NULL;   /* needs cleanup ! */
   size_t	     tmp_name_len   = 0;
   gn_name_desc    * outname;
   gn_name_desc      gn_name;

   (*pp_min_stat)  = 0;
   (*pp_out_name)  = NULL;
   memset( &gn_name, 0, sizeof(gn_name) );

   memcpy( &gn_name, p_in_name, sizeof(gn_name) );
   gn_name.mech_tag     = p_mech_tag;
   gn_name.first        = &gn_name;
   gn_name.next         = NULL;

   if ( p_mech_tag==p_in_name->mech_tag ) {
      /* Wow! the input name is already a canonical name for the */
      /* requested mechanism!  So let's just duplicate it.       */

   } else {

      /* Hard work: canonicalize a name from a printable representation */
      gn_name.name      = NULL;
      gn_name.name_len  = 0;
      gn_name.nt_tag    = NT_INVALID_TAG;

      if ( gn_name.prname==NULL ) {
	 /* Oh, there isn't a printable name -- we'll have to make one */
	 if ( p_in_name->mech_tag==MECH_INVALID_TAG ) {
	    DEBUG_ERR((tf, "Internal ERROR:%sbad name,"
		              " missing printable data\n",
		          this_Call ))
	    RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
	 }
/* Cleanup ALERT:  the following call create a dynamic object for */
/*                 (tmp_prname), which needs to be freed when     */
/*                 there is an error later on!                    */
	 maj_stat = (gn_mech[p_in_name->mech_tag]
		            ->fp_display_name)( pp_min_stat,
						p_in_name->nt_tag,
						p_in_name->name,
						p_in_name->name_len,
						&tmp_prname,
						&tmp_prname_len );
	 if (maj_stat!=GSS_S_COMPLETE)
	    goto cleanup;

	 gn_name.prname     = tmp_prname;
	 gn_name.prname_len = tmp_prname_len;

      } /* gn_name.prname==NULL */

/* Cleanup ALERT:  the following call create a dynamic object for */
/*                 (tmp_name), which needs to be freed when       */
/*                 there is an error later on!                    */
      maj_stat = (gn_mech[p_mech_tag]
	                 ->fp_canonicalize_name)( pp_min_stat,
						 gn_name.prname,
						 gn_name.prname_len,
						 p_in_name->nt_tag,
						 &tmp_name,
						 &tmp_name_len,
						 &(gn_name.nt_tag) );

      if (maj_stat!=GSS_S_COMPLETE)
	 goto cleanup;

      gn_name.name       = tmp_name;
      gn_name.name_len   = tmp_name_len;

      /* When the nametype changes then we MUST not retain the previous	*/
      /* printable representation					*/
      /* display_name() is going to create a new one when it is needed. */
      gn_name.prname     = NULL;
      gn_name.prname_len = 0;

   }

   /* Now Xerox the temporary name into a fully dynamic internal name */
   maj_stat = gn_duplicate_name( pp_min_stat, &gn_name, &outname );

   if (maj_stat!=GSS_S_COMPLETE)
      goto cleanup;

   outname->first = outname;
   outname->next  = NULL;

   (*pp_out_name) = outname;

cleanup:
   if ( tmp_prname!=NULL ) {
      sy_clear_free( (void **)&tmp_prname, tmp_prname_len );
   }
   if ( tmp_name!=NULL ) {
      sy_clear_free( (void **)&tmp_name, tmp_name_len );
   }

   return(maj_stat);

} /* gn_canonicalize_name() */



	
/*
 * gn_create_name()
 *
 *
 */
OM_uint32
gn_create_name( OM_uint32       * pp_min_stat,
		gn_nt_tag_et      nt_tag,
		gn_mech_tag_et    mech_tag,
		Uchar           * name,
		size_t            name_len,
		Uchar           * prname,
		size_t            prname_len,
		gn_name_desc   ** pp_name )
{
   gn_name_desc  tmp_name;
   OM_uint32     maj_stat = GSS_S_COMPLETE;

   (*pp_name) = NULL;
   memset( &tmp_name, 0, sizeof(tmp_name) );

   tmp_name.magic_cookie = COOKIE_NAME;
   tmp_name.mech_tag     = mech_tag;
   tmp_name.nt_tag       = nt_tag;
   if ( nt_tag==MECH_INVALID_TAG && ( name!=NULL || name_len!=0 ) ) {
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   tmp_name.name       = name;
   tmp_name.name_len   = name_len;
   tmp_name.prname     = prname;
   tmp_name.prname_len = prname_len;
   tmp_name.first      = &tmp_name;
   tmp_name.next       = NULL;

   maj_stat = gn_duplicate_name( pp_min_stat, &tmp_name, pp_name );

   return(maj_stat);

} /* gn_create_name() */
