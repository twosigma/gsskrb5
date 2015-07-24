#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/oid_func.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/oid_func.c#2 $
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
 * gn_gss_create_empty_oid_set()
 *
 * Status:  ** done (03-Sep-1996) mrex
 *
 */
OM_uint32
gn_gss_create_empty_oid_set(
        OM_uint32    FAR * p_min_stat,		/* minor_status            */
	gss_OID_set  FAR * p_out_oid_set	/* oid_set                 */
     )
{
   char *this_Call = "gn_gss_create_empty_oid_set";

   (*p_min_stat) = 0;

   (*p_out_oid_set) = sy_malloc( sizeof(gss_OID_set_desc) );

   if ( (*p_out_oid_set)==NULL ) {
      DEBUG_ACTION((tf, "ERROR %s() OUT OF MEMORY\n", this_Call ));
      (*p_min_stat) = MINOR_OUT_OF_MEMORY;
      return(GSS_S_FAILURE);
   }

   (*p_out_oid_set)->count    = 0;
   (*p_out_oid_set)->elements = (gss_OID)0;

   DEBUG_ACTION((tf, "<-- %s() empty gss_OID_set created\n", this_Call ));

   return(GSS_S_COMPLETE);

} /* gn_gss_create_empty_oid_set() */



/*
 * gn_gss_add_oid_set_member()
 *
 * Status:  ** done (05-Sep-1996) mrex
 *
 */
OM_uint32
gn_gss_add_oid_set_member(
        OM_uint32    FAR * pp_min_stat,         /* minor_status            */
	gss_OID            p_in_oid,		/* member_oid              */
        gss_OID_set  FAR * pp_out_oid_set	/* oid_set                 */
     )
{
   char          * this_Call = " gn_gss_add_oid_set_member(): ";
   void          * newelem;
   gss_OID_desc  * oldptr, * newptr;
   size_t          count, newsize;
   OM_uint32	   maj_stat;
   int             found;

   (*pp_min_stat) = 0;

   oldptr  = (*pp_out_oid_set)->elements;
   count   = (*pp_out_oid_set)->count;
   newsize = (count+1)*sizeof(gss_OID_desc);

   if ( p_in_oid==GSS_C_NO_OID ) {
      RETURN_MIN_MAJ( MINOR_INVALID_OID,
		      GSS_S_CALL_INACCESSIBLE_READ | GSS_S_FAILURE );
   }

   if ( pp_out_oid_set==NULL
	|| (*pp_out_oid_set)==GSS_C_NO_OID_SET ) {

      DEBUG_ERR((tf, "ERROR:%scannot add to GSS_C_NO_OID_SET\n",
		 this_Call))

      RETURN_MIN_MAJ( MINOR_INVALID_OID_SET,
		      GSS_S_CALL_INACCESSIBLE_WRITE | GSS_S_FAILURE );
   }

   if ( (*pp_out_oid_set) == &gn_avail_mech_oids ) {

      DEBUG_ERR((tf, "ERROR:%swill not add to my own mech OID list!\n",
		 this_Call ))
      RETURN_MIN_MAJ( MINOR_NO_ERROR,
		      GSS_S_CALL_INACCESSIBLE_WRITE | GSS_S_FAILURE );
   }

   /* check if the member oid is already contained in the oidset */
   maj_stat = gn_gss_test_oid_set_member( pp_min_stat, p_in_oid, *pp_out_oid_set, &found );
   if ( maj_stat!=GSS_S_COMPLETE )
      return(maj_stat);

   /* don't add the oid to the set if it is already there! */
   if ( found!=0 )
      return(GSS_S_COMPLETE);

   /* allocate new memory for a larger gss_OID array */
   newptr = (gss_OID_desc *) sy_malloc( newsize );
   if ( newptr==(void *)0 ) {
      (*pp_min_stat) = MINOR_OUT_OF_MEMORY;
      return(GSS_S_FAILURE);
   }

   /* allocate new memory to copy the elements of the new OID */
   newelem = (void *) sy_malloc( p_in_oid->length );
   if ( newelem==(void *)0 ) {
      sy_free(newptr);
      (*pp_min_stat) = MINOR_OUT_OF_MEMORY;
      return(GSS_S_FAILURE);
   }

   /* copy the OID array into the enlarged array storage */
   memcpy( newptr, oldptr, count*sizeof(gss_OID_desc) );
   sy_clear_free( (void **) &oldptr, count * sizeof(gss_OID_desc) );

   /* copy the elements of the new OID and put it into the new array */
   memcpy( newelem, p_in_oid->elements, p_in_oid->length );
   newptr[count].length   = p_in_oid->length;
   newptr[count].elements = newelem;

   /* adjust the gss_OID_set structure to the new and enlarged array */
   ((*pp_out_oid_set)->count)++;
   (*pp_out_oid_set)->elements = newptr;
   
   return(GSS_S_COMPLETE);

} /* gn_gss_add_oid_set_member() */


/*
 * gn_gss_oid_to_str()
 *
 * Status:
 *
 */
OM_uint32
gn_gss_oid_to_str(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_OID			p_oid,		/* oid			*/
	gss_buffer_t		p_buffer	/* oid_str		*/
     )
{
   char        oidbuf[1024], numbuf[32];
   Ulong       num  = 0; 
   OM_uint32   maj_stat;
   int         more;
   Uint        i, curlen, len, maxch;
   Uchar     * bytes;
   Uchar       uch;

   (*pp_min_stat) = 0;
   maxch = (sizeof(oidbuf)/sizeof(oidbuf[0])) - 4;
   memset(oidbuf, 0, sizeof(oidbuf));

   if ( p_buffer==NULL ) {
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CALL_INACCESSIBLE_WRITE );
   }

   p_buffer->length = 0;
   p_buffer->value  = NULL;

   if ( p_oid==NULL  ||  p_oid->length==0  ||  p_oid->elements==NULL ) {
      RETURN_MIN_MAJ( MINOR_INVALID_OID, GSS_S_CALL_BAD_STRUCTURE );
   }

   bytes = ((unsigned char *)p_oid->elements);
   uch = bytes[0];
   sprintf(oidbuf, "{%u %u", (unsigned int)(uch/40), (unsigned int)(uch%40));

   curlen = (Uint)strlen(oidbuf);
   more = FALSE;
   for( i=1 ; i<p_oid->length ; i++ ) {
      uch  = bytes[i];
      num  = (uch & 0x7f) + ( (more) ? (num<<7) : 0 );
      more = ( (uch>127) ? TRUE : FALSE );
      if (more==FALSE) {
	 sprintf(numbuf," %lu", num);
	 len = (Uint)strlen(numbuf);
	 if ( len + 1 + curlen > maxch ) {
	    RETURN_MIN_MAJ(MINOR_INTERNAL_BUFFER_OVERRUN, GSS_S_FAILURE);
	 }
	 memcpy( &(oidbuf[curlen]), numbuf, (size_t)(len+1) );
	 curlen += len;
      }
   }

   if (more) {
      RETURN_MIN_MAJ( MINOR_INVALID_OID, GSS_S_CALL_BAD_STRUCTURE );
   }

   oidbuf[curlen++] = '}';
   oidbuf[curlen]   = '\0';

   maj_stat = gn_alloc_buffer( pp_min_stat, p_buffer,
			       (void *)oidbuf,
			       (size_t)curlen,
			       TRUE );

   return(maj_stat);

} /* gn_gss_oid_to_str() */



/*
 * gn_gss_str_to_oid()
 *
 *
 *
 */
OM_uint32
gn_gss_str_to_oid(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_buffer_t		p_in_oid_string,/* oid_string		*/
	gss_OID		FAR *	pp_out_oid	/* oid			*/
     )
{
   char        * this_Call = "gn_gss_str_to_oid";
   OM_uint32     maj_stat;

   (*pp_min_stat) = MINOR_NOT_YET_IMPLEMENTED;
   maj_stat       = GSS_S_FAILURE;


   return(maj_stat);

} /* gn_gss_str_to_oid() */




/*
 * gn_gss_test_oid_set_member()
 *
 *
 *
 */
OM_uint32
gn_gss_test_oid_set_member(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_OID			p_in_member,	/* member		*/
	gss_OID_set		p_in_set,	/* set			*/
	int		FAR *	p_out_bool	/* present		*/
     )
{
   char        * this_Call = "gn_gss_test_oid_set_member";
   OM_uint32     maj_stat;
   size_t        i;

   (*pp_min_stat) = MINOR_NO_ERROR;
   maj_stat       = GSS_S_COMPLETE;

   (*p_out_bool)  = FALSE;

   for ( i=0 ; i<p_in_set->count ; i++ ) {
      maj_stat = gn_compare_oid( pp_min_stat,
				 &(p_in_set->elements[i]),
				 p_in_member,
				 p_out_bool );
      if ( maj_stat!= GSS_S_COMPLETE  ||  (*p_out_bool)!=FALSE ) {
	 return(maj_stat);
      }
   }

   return(maj_stat);

} /* gn_gss_test_oid_set_member() */




/*
 * gn_create_nt_oid_set()
 *
 *  Description:
 *    Create a (fully dynamic) gss_OID_set of nametypes
 *    from a supplied array of nametype OID tags.
 *
 */ 
OM_uint32
gn_create_nt_oid_set( OM_uint32     * pp_min_stat,
		      gn_nt_tag_et  * pp_nt_oids,
		      size_t	      p_num_oids,
		      gss_OID_set   * pp_oid_set )
{
   char              * this_Call = " gn_create_nt_oid_set(): ";
   OM_uint32           maj_stat;
   gss_OID_set_desc    oid_set;
   gss_OID             poid;
   gss_OID_desc        oids[GN_MAX_NAMETYPE_OIDS];
   size_t              i;

   if ( p_num_oids>GN_MAX_NAMETYPE_OIDS ) {
      DEBUG_ERR((tf, "ERROR:%srequested %u num_oids > max (%u)!\n",
		     this_Call,
		     (unsigned int) p_num_oids,
		     GN_MAX_NAMETYPE_OIDS ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   for ( i=0 ; i<p_num_oids ; i++ ) {
      poid = gn_nt_tag_to_oid( pp_nt_oids[i] );
      if (poid==GSS_C_NO_OID) {
	 DEBUG_ERR((tf, "ERROR:%sinvalid nametype tag supplied (%u)\n",
		         this_Call,
		         pp_nt_oids[i] ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
      oids[i].length   = poid->length;
      oids[i].elements = poid->elements;
   }

   oid_set.count    =  p_num_oids;
   oid_set.elements = (p_num_oids==0) ? NULL : &(oids[0]);

   maj_stat = gn_copy_oid_set( pp_min_stat, &(oid_set), pp_oid_set );

   return(maj_stat);

} /* gn_create_nt_oid_set() */




/*
 * gn_create_mech_oid_set()
 *
 *  Description:
 *    Create a (fully dynamic) gss_OID_set of mechanisms
 *    from a supplied array of mechanism OID tags.
 *
 */ 
OM_uint32
gn_create_mech_oid_set( OM_uint32     * pp_min_stat,
			gn_nt_tag_et  * pp_mech_oids,
			size_t	        p_num_oids,
			gss_OID_set   * pp_oid_set )
{
   char              * this_Call = " gn_create_mech_oid_set(): ";
   OM_uint32           maj_stat;
   gss_OID_set_desc    oid_set;
   gss_OID             poid;
   gss_OID_desc        oids[GN_MAX_MECH_OIDS];
   size_t              i;

   if ( p_num_oids>GN_MAX_MECH_OIDS ) {
      DEBUG_ERR((tf, "ERROR:%srequested %u mech_oids > max (%u)!\n",
		     this_Call,
		     (unsigned int) p_num_oids,
		     GN_MAX_MECH_OIDS ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   for ( i=0 ; i<p_num_oids ; i++ ) {
      poid = gn_mech_tag_to_oid( pp_mech_oids[i] );
      if (poid==GSS_C_NO_OID) {
	 DEBUG_ERR((tf, "ERROR:%sinvalid mechanism tag supplied (%u)\n",
		         this_Call,
		         pp_mech_oids[i] ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }
      oids[i].length   = poid->length;
      oids[i].elements = poid->elements;
   }

   oid_set.count    =  p_num_oids;
   oid_set.elements = (p_num_oids==0) ? NULL : &(oids[0]);

   maj_stat = gn_copy_oid_set( pp_min_stat, &(oid_set), pp_oid_set );

   return(maj_stat);

} /* gn_create_mech_oid_set() */




/*
 * gn_copy_oid_set()
 *
 *  Description:
 *    Create a fully dynamic gss_OID_set by cloning.
 *
 */
OM_uint32
gn_copy_oid_set( OM_uint32 * pp_min_stat,
		 gss_OID_set p_src_oid_set,
		 gss_OID_set * pp_dst_oid_set )
{
   OM_uint32            maj_stat;
   gss_OID_set_desc   * poid_set = NULL;
   void               * elem;
   size_t               total    = 0;
   size_t		count, len;
   size_t               i;

   (*pp_min_stat) = 0;
   maj_stat       = GSS_S_COMPLETE;
   count          = 0;

   if ( pp_dst_oid_set==NULL ) {
      RETURN_MIN_MAJ( MINOR_INVALID_OID_SET,
		      GSS_S_CALL_INACCESSIBLE_WRITE | GSS_S_FAILURE );
   }

   poid_set = sy_calloc( sizeof(*poid_set) );
   if ( poid_set==NULL ) {
      goto out_of_memory;
   }

   poid_set->count    = 0;
   poid_set->elements = GSS_C_NO_OID;

   total = p_src_oid_set->count;

   if ( total>0 ) {
      poid_set->elements = sy_malloc( total*sizeof(gss_OID_desc) );
      if ( poid_set->elements==NULL ) {
	 goto out_of_memory;
      }
      for ( count=0 ; count<total; count++ ) {
	 len = (p_src_oid_set->elements[count]).length;
	 elem = sy_malloc( len );
	 if ( elem==NULL ) {
	    goto out_of_memory;
	 }

	 memcpy( elem, (p_src_oid_set->elements[count]).elements, len );
	 (poid_set->elements[count]).length   = (OM_uint32)len;
	 (poid_set->elements[count]).elements = elem;
      }

   }

   poid_set->count   = total;
   (*pp_dst_oid_set) = poid_set;

   return(GSS_S_COMPLETE);	 

out_of_memory:
   if ( poid_set!= NULL ) {
      if ( poid_set->elements!=NULL ) {
	 for ( i=0 ; i<count ; i++ ) {
	    elem = (poid_set->elements[i]).elements;
	    if ( elem!=NULL ) {
	       sy_clear_free( (void **) &((poid_set->elements[i]).elements),
			      (poid_set->elements[i]).length );
	    }
	 }
	 sy_clear_free( (void **) &(poid_set->elements),
			total * sizeof(gss_OID_desc) );
      }
      sy_clear_free( (void **) &(poid_set), sizeof(*poid_set) );
   }

   RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );

} /* gn_copy_oid_set() */

		    
