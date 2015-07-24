#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/dup_name.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/dup_name.c#1 $
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
 * gn_gss_duplicate_name()
 *
 *
 */
OM_uint32
gn_gss_duplicate_name(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_name_t		p_src_name,	/* src_name		*/
	gss_name_t	FAR *	pp_dest_name	/* dest_name		*/
     )
{
   OM_uint32      tmp_min_stat, maj_stat;
   char         * this_Call = " gn_gss_duplicate_name(): ";
   gn_name_desc * src,  *src_first;
   gn_name_desc * dest, *dest_first, *dest_prev;
   gn_name_desc * next;

   (*pp_dest_name)  = GSS_C_NO_NAME;
   (*pp_min_stat)   = 0;

   if ( p_src_name == GSS_C_NO_NAME ) {
      DEBUG_STRANGE((tf, "  S:%sduplicating GSS_C_NO_NAME ?!\n", this_Call));
      return(GSS_S_COMPLETE);
   }

   maj_stat = gn_check_name( pp_min_stat, &p_src_name, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   src_first  = (gn_name_desc *) p_src_name;
   dest_first = dest_prev = NULL;

   for( src = src_first ; src!=NULL ; src = src->next ) {
      maj_stat = gn_duplicate_name( pp_min_stat, src, &dest );
      if ( maj_stat!=GSS_S_COMPLETE ) {
	 /* Oooops, I got an error!  Free all dynamic objects */
	 for ( dest= dest_first ; dest!=NULL ; dest= next ) {
	    next = dest->next;
	    (void)gn_release_name( &tmp_min_stat, &dest );
	 }
	 return(maj_stat);
      }

      if ( dest_first==NULL ) {
	 dest_first      = dest;
      } else {
	 dest_prev->next = dest;
      }
      dest_prev   = dest;
      dest->next  = NULL;
      dest->first = dest_first;
   }

   (*pp_dest_name) = (gss_name_t) dest_first;

   return(GSS_S_COMPLETE);

} /* gn_gss_duplicate_name() */




OM_uint32
gn_duplicate_name( OM_uint32     * pp_min_stat,
		   gn_name_desc  * p_src,
		   gn_name_desc ** pp_dest )
{
   char           * this_Call = "gn_duplicate_name";
   gn_name_desc   * dest      = GSS_C_NO_NAME;

   (*pp_min_stat) = 0;
   (*pp_dest)     = GSS_C_NO_NAME;

   if ( p_src!=GSS_C_NO_NAME ) {

      if ( p_src->magic_cookie!=COOKIE_NAME ) {

         DEBUG_STRANGE((tf, "Internal ERROR: %s(): magic cookie missing!\n",
		         this_Call ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );

      } else {  /* vvv (p_src->magic_cookie == COOKIE_NAME) vvv */

	 /* allocate basic name control structure */ 
	 dest = (gn_name_desc *)sy_malloc( sizeof(*dest) );
	 if ( dest==NULL ) {
	    RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
	 }
	 /* copy original name structure (including pointers -- watch out!) */
	 memcpy( dest, p_src, sizeof(*dest) );

	 /* initialize the linked list of name elements for a single standalone name */
	 dest->first      = dest;
	 dest->next       = NULL;

	 /* clear the reference to the original name image (i.e clear copied pointers) */
	 dest->name       = NULL;
	 dest->name_len   = 0;

	 dest->prname     = NULL;
	 dest->prname_len = 0;

	 /* if there was a name image, then alloc memory and copy */
	 if ( p_src->name!=NULL && p_src->name_len>0 ) {
	    dest->name = (char *) sy_malloc( p_src->name_len );
	    if ( dest->name == NULL ) {
	       sy_clear_free( (void **) &dest, sizeof(*dest) );
	       RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
	    }
	    memcpy( dest->name, p_src->name, p_src->name_len );
	    dest->name_len = p_src->name_len;
	 }

	 /* if there was a printable name image, then alloc memory and copy */
	 if ( p_src->prname!=NULL && p_src->prname_len>0 ) {
	    dest->prname = (char *) sy_malloc( p_src->prname_len + 1 );
	    if ( dest->prname == NULL ) {
	       sy_clear_free( (void **) &(dest->name), dest->name_len );
	       sy_clear_free( (void **) &dest, sizeof(*dest) );
	       RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
	    }
	    memcpy( dest->prname, p_src->prname, p_src->prname_len );
	    dest->prname_len = p_src->prname_len;
	    dest->prname[dest->prname_len] = '\0';
	 }

      } /* ^^^ endif (p_src->magic_cookie == COOKIE_NAME) */

   } /* (p_src != GSS_C_NO_NAME) */

   (*pp_dest) = dest;

   return(GSS_S_COMPLETE);

} /* gn_duplicate_name() */

