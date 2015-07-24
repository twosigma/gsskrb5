#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/rel_cred.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/rel_cred.c#1 $
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
 * gn_gss_release_cred()
 *
 *
 */
OM_uint32
gn_gss_release_cred(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_cred_id_t	FAR *	pp_io_cred	/* cred_handle		*/
     )
{
   char          * this_Call = "gn_gss_release_cred";
   gn_cred_desc  * cred, * ncred;
   OM_uint32       maj_stat;
   OM_uint32       maj_stat2 = GSS_S_COMPLETE;
   OM_uint32       min_stat2 = MINOR_NO_ERROR;

   (*pp_min_stat) = MINOR_NO_ERROR;
   maj_stat       = GSS_S_COMPLETE;

   if ( pp_io_cred==NULL || (*pp_io_cred)==NULL ) {
      DEBUG_ACTION((tf, "  A: %s(): ignoring GSS_C_NO_CREDENTIAL!\n",
		        this_Call ))
      return(maj_stat);
   }

   maj_stat = gn_check_cred( pp_min_stat, pp_io_cred, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   /* Loop over the linked list of all credential elements/components */
   /* Individual credential elements must differ from each other by   */
   /* at least one of:  usage (INITIATE, ACCEPT) and mech_tag         */
   for ( cred = (gn_cred_desc *)(*pp_io_cred) ; cred!=NULL ; cred = ncred ) {

      ncred = cred->next;
      maj_stat2 = gn_release_cred( &min_stat2, &cred );
      if ( maj_stat==GSS_S_COMPLETE ) {
	 maj_stat       = maj_stat2;
	 (*pp_min_stat) = min_stat2;
      }

   } /* for ( cred!=NULL ) */

   (*pp_io_cred) = GSS_C_NO_CREDENTIAL;

   return(maj_stat);

} /* gn_gss_release_cred() */



/*
 * gn_release_cred()
 *
 *
 */
OM_uint32
gn_release_cred( OM_uint32  * pp_min_stat,   gn_cred_desc ** pp_cred )
{
   char        * this_Call = "gn_release_cred";
   OM_uint32     maj_stat  = GSS_S_COMPLETE;
   OM_uint32     maj_stat2 = GSS_S_COMPLETE;
   OM_uint32     min_stat2 = MINOR_NO_ERROR;
   
   (*pp_min_stat) = MINOR_NO_ERROR;

   if ( (*pp_cred)!=NULL ) {
      if ( (*pp_cred)->magic_cookie != COOKIE_CREDENTIAL ) {

         DEBUG_STRANGE((tf, "Internal ERROR: %s(): magic cookie missing!\n",
		         this_Call ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );

      } else {

	 if ( (*pp_cred)->prv_cred!=NULL ) {
	    if ( (*pp_cred)->mech_tag==MECH_INVALID_TAG
	         ||  (Uint)((*pp_cred)->mech_tag)>mech_last_tag ) {
	       DEBUG_ERR((tf, "  E: %s(): invalid mech_tag in credential!\n", this_Call ))
	       (*pp_min_stat) = MINOR_INTERNAL_ERROR;
	       maj_stat       = GSS_S_FAILURE;
	    } else {
	       maj_stat = (gn_mech[(*pp_cred)->mech_tag]
			      ->fp_release_cred)( pp_min_stat,
					          &((*pp_cred)->prv_cred) );
	    }
	 }
	 if ( (*pp_cred)->subject!=NULL ) {
	    maj_stat2 = gn_gss_release_name( &min_stat2,
					     &((*pp_cred)->subject) );
	 }
	 sy_clear_free( (void **) pp_cred, sizeof(**pp_cred) );

	 if ( maj_stat==GSS_S_COMPLETE ) {
	    /* We like to see every error, but we can only return one */
	    maj_stat	   = maj_stat2;
	    (*pp_min_stat) = min_stat2;
	 }
      }
   }

   return(maj_stat);

} /* gn_release_cred() */
