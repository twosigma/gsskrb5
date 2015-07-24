#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/acq_cred.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/acq_cred.c#2 $
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
 * gn_gss_acquire_cred()
 *
 *
 */
OM_uint32
gn_gss_acquire_cred(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_name_t		p_in_name,	/* desired_name		*/
	OM_uint32		p_time_req,	/* time_req		*/
	gss_OID_set		p_in_mechs,	/* desired_mechs	*/
	gss_cred_usage_t	p_in_cred_usage,/* cred_usage		*/
	gss_cred_id_t	FAR *	pp_out_cred,	/* output_cred_handle	*/
	gss_OID_set	FAR *	pp_out_mechs,	/* actual_mechs		*/
	OM_uint32	FAR *	pp_out_lifetime	/* time_rec		*/
     )
{
   char             * this_Call = "gn_gss_acquire_cred";
   gn_name_desc     * gn_name;
   gn_name_desc     * gn_can_name;
   gss_OID_desc     * poid;
   gss_OID_set_desc * poid_set;
   gn_cred_desc     * gn_cred;
   gn_cred_desc     * gn_top_cred;
   gn_cred_desc     * tcred;	/* temporary convenience pointer */
   gn_mech_tag_et     mech_tags[GN_MAX_MECH_OIDS+1];
   gn_mech_tag_et     mech_tag;
   Uint               num_mechs, i;
   int                mech_found;
   OM_uint32          maj_stat;
   OM_uint32          maj_stat2, min_stat2;
   time_t             expires_at = 0;
   OM_uint32	      lifetime;

   (*pp_min_stat) = 0;
   maj_stat       = GSS_S_COMPLETE;
   maj_stat2	  = GSS_S_NO_CRED;
   min_stat2	  = MINOR_NO_ERROR;

   num_mechs      = 0;
   gn_name        = NULL;
   gn_cred        = NULL;   /* dynamic return object, release on error  */
   gn_top_cred	  = NULL;   /* reference of top object */
   gn_can_name    = NULL;   /* dynamic local resource, must be released */
   poid_set       = GSS_C_NO_OID_SET; /* dynamic return object, rel on error */

   (*pp_out_cred) = GSS_C_NO_CREDENTIAL;
   if ( pp_out_mechs )    { (*pp_out_mechs)    = GSS_C_NO_OID_SET; }
   if ( pp_out_lifetime ) { (*pp_out_lifetime) = 0;		   }

   switch( p_in_cred_usage ) {
      case GSS_C_INITIATE:
      case GSS_C_ACCEPT:
      case GSS_C_BOTH:
	        break;

      default:  RETURN_MIN_MAJ( MINOR_BAD_CRED_USAGE, GSS_S_FAILURE );

   } /* switch( p_in_cred_usage ) */

   num_mechs = 0;

   if ( p_in_mechs!=GSS_C_NO_OID_SET ) {
      
      /* were there any mechanism OIDs supplied ? */
      if ( p_in_mechs->count==0  ||  p_in_mechs->elements==NULL ) {
	 DEBUG_ERR((tf, "ERROR: %s(): bad in_mechs oid_set\n", this_Call))
	 RETURN_MIN_MAJ( MINOR_INVALID_OID_SET, GSS_S_FAILURE );
      }

      for( i=0 ; i<(p_in_mechs->count) ; i++ ) {
	 /* loop over all supplied mech_oids in (OID_set) p_in_mechs */
	 poid = &(p_in_mechs->elements[i]);
	 maj_stat = gn_oid_to_mech_tag( pp_min_stat, poid, &mech_tag );
	 if ( maj_stat!=GSS_S_COMPLETE )
	    return(maj_stat);

	 if ( mech_tag==MECH_INVALID_TAG ) {
	    RETURN_MIN_MAJ( MINOR_UNKNOWN_MECH_OID, GSS_S_BAD_MECH );
	 }
	 mech_tags[num_mechs] = mech_tag;
	 num_mechs++;
      }

      if ( num_mechs==0 ) {
	  DEBUG_ERR((tf, "ERROR: %s(): no recognized mech_oid in requested oid_set\n", this_Call ));
	  RETURN_MIN_MAJ( MINOR_UNKNOWN_MECH_OID, GSS_S_BAD_MECH );
      }

   } /* p_in_mechs != GSS_C_NO_OID_SET */


   if ( p_in_name!=GSS_C_NO_NAME ) {
      /* explicit name supplied */
      /* check explicit name first (if it is really a valid internal name) */
      maj_stat = gn_check_name( pp_min_stat, &p_in_name, this_Call );
      if (maj_stat!=GSS_S_COMPLETE)
	 return(maj_stat);

      gn_name = (gn_name_desc *) p_in_name;

      if ( gn_name->mech_tag!=MECH_INVALID_TAG ) {
	 /* supplied name is canonical (i.e. mech-specific) */
	 if ( num_mechs>0 ) {
	    mech_found = FALSE;
	    for ( i=0 ; i<num_mechs ; i++ ) {
	       if ( mech_tags[i]==gn_name->mech_tag ) {
		  mech_found = TRUE;
		  break;
	       }
	    }

	    if ( mech_found==FALSE ) {
	       /* There is no intersection of the requested mechanisms      */
	       /* and the mechanism of the explicit supplied mechanism name */ 
	       RETURN_MIN_MAJ( MINOR_MN_DISJUNCT_MECHLIST, GSS_S_FAILURE );
	    }

	 } /* num_mechs>0 */

	 /* hardwire the mechanism-list to the mechanism of the MN */
	 num_mechs    = 1;
	 mech_tags[0] = gn_name->mech_tag;

      } /* gn_name->mech_tag != MECH_INVALID_TAG */

   } /* p_in_name != GSS_C_NO_NAME */


   if ( num_mechs==0 ) {

      /* No mechanism OID was requested for these credentials   */
      /* Create a (multimechanism) credential for ALL available */
      /* mechanism(s)						 */
      for ( i=MECH_FIRST_TAG ; i<=mech_last_tag ; i++ ) {
	 mech_tags[num_mechs] = (gn_mech_tag_et) (i);
	 num_mechs++;
      }

   } /* num_mechs==0 */


   /* Enumerate over all mechanisms in the mech_tags[] array and    */
   /* try to acquire credential elements for them.                  */
   /* We're enumerating backwards and insert fresh credentials at   */
   /* the beginning of the chain, so that finally, the first cred   */
   /* in the list is the first non-expired from the list of         */
   /* requested (or default) mechanisms                             */
   for ( i=0 ; i<num_mechs ; i++ ) {
      
      maj_stat2 = gn_new_cred( &min_stat2, gn_name, p_in_cred_usage,
			       mech_tags[i], &gn_cred, &lifetime );

      if (maj_stat2!=GSS_S_COMPLETE) {

	 /* Treat them all as fatal (for now) */
	 maj_stat       = maj_stat2;
	 (*pp_min_stat) = min_stat2;

      } else { /* elsif (maj_stat2==GSS_S_COMPLETE) */

         /* We will NOT stop here for expired credentials            */
         /* (they need to be dealt with in the other calls anyway)   */
	 /* However we will put expired credentials last in the list */

	 /* the next should always succeed (or its a fatal problem) */
	 maj_stat = gn_chain_cred( pp_min_stat, &gn_top_cred, &gn_cred, TRUE );
	 if ( maj_stat!=GSS_S_COMPLETE )
	    goto error;
	 
      } /* endelse (maj_stat2==GSS_S_COMPLETE) */

   }

   if ( gn_top_cred==NULL ) {
      ERROR_RETURN( min_stat2, maj_stat2 );
   }

   /* Rebuild the list of mechanisms by walking along our new */
   /* credential element list				      */

   num_mechs = 0;
   for ( tcred=gn_top_cred ; tcred!=NULL ; tcred=tcred->next ) {
      mech_tags[num_mechs++] = tcred->mech_tag;
   }

   if ( pp_out_mechs ) {
      /* mechanism list requested, create oid_set from mechanism tag */
      maj_stat = gn_create_mech_oid_set( pp_min_stat, &mech_tags[0],
					 num_mechs, &poid_set );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;
   }

   if ( maj_stat==GSS_S_COMPLETE ) {
      /* SUCCESS -- fill all requested output parameters */

      (*pp_out_cred)        = (gss_cred_id_t) gn_top_cred;

      if ( pp_out_lifetime!=NULL ) {
	 /* out lifetime requested; calculate with return value from (fp_acquire_cred)(...) */
	 (*pp_out_lifetime) = gn_remaining_time( gn_top_cred->expires_at );
      }

      if ( pp_out_mechs!=NULL )
	 /* out mechset requested; return it */
	 (*pp_out_mechs) = poid_set;

   } else {
error:
      /* FAILURE -- release all dynamically allocated objects */
      (void) gn_gss_release_oid_set( &min_stat2, &poid_set );   /* release mechset (if present) */
      (void) gn_release_cred( &min_stat2, &gn_cred );           /* release cred    (if present) */
      (void) gn_gss_release_cred( &min_stat2, &gn_top_cred );   /* release cred    (if present) */

   }

   if ( gn_can_name!=NULL ) {
      maj_stat2   = gn_release_name( &min_stat2, &gn_can_name );
      if ( maj_stat==GSS_S_COMPLETE && maj_stat2!=GSS_S_COMPLETE ) {
	 maj_stat       = maj_stat2;
	 (*pp_min_stat) = min_stat2;
      }
   }

   return(maj_stat);

} /* gn_gss_acquire_cred() */



/*
 * gn_check_cred()
 *
 *
 */
OM_uint32
gn_check_cred( OM_uint32 * pp_min_stat, gss_cred_id_t * pp_cred, char * this_Call )
{
   gn_cred_desc  * cred;

   (*pp_min_stat) = 0;

   if ( pp_cred==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): missing credential handle!\n", this_Call ))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CALL_INACCESSIBLE_READ|GSS_S_NO_CRED );
   }

   for ( cred = (gn_cred_desc *) (*pp_cred) ; cred!=NULL ; cred = cred->next ) {

      if ( cred->magic_cookie != COOKIE_CREDENTIAL ) {
	 DEBUG_ERR((tf, "  E: %s(): invalid credential handle!\n", this_Call ))
	 RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CALL_BAD_STRUCTURE|GSS_S_NO_CRED);
      }

      if ( cred->prv_cred!=NULL ) {
	 if ( cred->mech_tag==MECH_INVALID_TAG  ||  (Uint)(cred->mech_tag)>mech_last_tag ) {
	    DEBUG_ERR((tf, "  E: %s(): invalid mech_tag in credential!\n", this_Call ))
	    RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_DEFECTIVE_CREDENTIAL );
	 }
      }

   }

   return(GSS_S_COMPLETE);

} /* gn_check_cred() */



/*
 * gn_chain_cred()
 *
 */
OM_uint32
gn_chain_cred(
	OM_uint32        * pp_min_stat,
	gn_cred_desc    ** pp_top_cred,
	gn_cred_desc    ** pp_cred,
	int                p_chain_last
    )
{
   char            * this_Call = "gn_chain_cred";
   gn_cred_desc    * cred;
   gn_cred_desc    * top_cred;
   OM_uint32	     maj_stat = GSS_S_COMPLETE;

   if ( pp_cred==NULL || (*pp_cred)==NULL ) {
      DEBUG_ERR((tf, "Internal ERROR: %s(): NULL pp_cred handle\n", this_Call));
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }
   if ( (*pp_cred)->next!=NULL ) {
      DEBUG_ERR((tf, "Internal ERROR: %s(): this is not a NEW pp_cred handle\n", this_Call));
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   /* first credential element in the chain (i.e. linked list) */
   top_cred = (*pp_top_cred)!=NULL ? (*pp_top_cred) : (*pp_cred);

   
   if ( p_chain_last!=0 ) {

      /* The new element is to be the last in the list/chain */
      if ( top_cred != (*pp_cred) ) {
	 /* multiple credential elements in the chain, walk to the end */
	 for( cred=top_cred ; cred->next!=NULL ; cred = cred->next );
	 cred->next = (*pp_cred);
      }

   } else {

      /* The new element is to be the first in the list/chain */
      if ( top_cred!=(*pp_cred) ) {
	 /* multiple credentials elements in the chain, insert */
	 (*pp_cred)->next = top_cred;
	 top_cred         = (*pp_cred);
      }

   }

   for ( cred = top_cred ; cred!=NULL ; cred = cred->next ) {
      cred->first = top_cred;
   }

   /* mark the top of the list in the new credential element */
   (*pp_top_cred) = top_cred;

   /* Prevent caller from freeing this element twice,      */
   /* Once within the pp_top_cred list and once standalone */
   (*pp_cred)     = NULL;

   return(maj_stat);

} /* gn_chain_cred() */



/*
 * gn_new_cred()
 *
 *
 */
OM_uint32
gn_new_cred(
        OM_uint32          * pp_min_stat,
	gn_name_desc	   * p_name,
	gss_cred_usage_t     p_cred_usage,
	gn_mech_tag_et       p_mechtag,
	gn_cred_desc      ** pp_cred,
	OM_uint32          * pp_lifetime
      )
{
   char		      * this_Call    = "gn_new_cred()";
   gn_name_desc	      * gn_can_name  = NULL;
   void		      * cname        = NULL;  /* temporary buffer           */
   size_t		cname_len    = 0;     /* length of temporary buffer */
   time_t		expires_at   = 0;
   OM_uint32		maj_stat     = GSS_S_COMPLETE;
   OM_uint32		maj_stat2;
   OM_uint32	        min_stat2;

   (*pp_min_stat) = 0;
   (*pp_lifetime) = 0; /* initialize with 0 */

   (*pp_cred) = sy_calloc( sizeof(gn_cred_desc) );
   if ((*pp_cred)==NULL) {
      ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   (*pp_cred)->magic_cookie = COOKIE_CREDENTIAL;
   (*pp_cred)->first        = (gn_cred_desc *) NULL;
   (*pp_cred)->next         = (gn_cred_desc *) NULL;
   (*pp_cred)->subject      = (gn_name_desc *) NULL;

   (*pp_cred)->usage        = p_cred_usage;
   (*pp_cred)->prv_cred     = NULL;
   (*pp_cred)->mech_tag     = p_mechtag;

   if ( p_name==NULL ) {
      /* no name supplied, request default credentials from mechanism */
      cname      = NULL;
      cname_len  = 0;

   } else {

      /* We were given an explicit user name and will use that */
      if ( p_name->mech_tag==p_mechtag ) {
	 /* The supplied name is already canonical for the */
	 /* requested mechanism, so we can use it directly */
	 cname     = p_name->name;
	 cname_len = p_name->name_len;

      } else {

	 /* we need to canonicalize this name first ... */

/* Cleanup ALERT:  the following call creates a dynamic object for */
/*                 (gn_can_name), which needs to be freed when     */
/*                 there is an error later on!                     */

	 maj_stat    = gn_canonicalize_name( pp_min_stat, p_name,
					     p_mechtag, &gn_can_name );
	 if (maj_stat!=GSS_S_COMPLETE)
	    goto error;

	 cname     = gn_can_name->name;
	 cname_len = gn_can_name->name_len;

      }

   }

   maj_stat = (gn_mech[p_mechtag]
	       ->fp_acquire_cred)( pp_min_stat, cname, cname_len,
			           p_cred_usage,
				   &((*pp_cred)->prv_cred), &expires_at );

   if ( maj_stat==GSS_S_COMPLETE ) {
      /* SUCCESS */
      (*pp_cred)->expires_at = expires_at;
      (*pp_lifetime)         = gn_remaining_time(expires_at);

   } else {
error:
      gn_release_cred( &min_stat2, pp_cred );

   }

   if ( gn_can_name!=NULL ) {
      /* We have to release a temporary name here */
      maj_stat2 = gn_release_name( &min_stat2, &gn_can_name );
      if ( maj_stat==GSS_S_COMPLETE  &&  maj_stat2!=GSS_S_COMPLETE ) {
	 maj_stat       = maj_stat2;
	 (*pp_min_stat) = min_stat2;
      }
   }

   return(maj_stat);

} /* gn_new_cred() */
