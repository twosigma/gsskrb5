#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/gssmaini.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/gssmaini.c#2 $
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

#include <time.h>


char * build_time      = "Compiled on " __DATE__ " at " __TIME__ ;

gss_OID_set_desc     gn_avail_mech_oids;

  /* I ignore the first entry in the nt_oid_tags table (NT_INVALID_TAG==0) */
  /* therefore I have to allocate one more slot (same for mech_oid_tags)   */
static gss_OID_desc  nt_oids[ GN_MAX_NAMETYPE_OIDS + 1 ];
Uint                 nt_last_tag = 0;

static gss_OID_desc  mech_oids[ GN_MAX_MECH_OIDS + 1 ];
Uint                 mech_last_tag = 0;

struct gn_mechanism_s *gn_mech[ GN_MAX_MECH_OIDS + 1 ];

/*********************************************************************
 * gn_init()
 *
 * Description:
 *   Initialization call for GENERIC functions and data structures
 *
 *   This function will be called ONCE and INITIALLY when an application
 *   does it's first call into one of the GSS-API functions.
 *
 *   GSS-API v1 and v2 do not have an initialization function, so the
 *   check has to be done within every API-call.
 *
 *   Initialization functions that would break when called several times
 *   must protect themselves.  If all initalization functions return
 *   success, then the global "gn_gss_initialized" flag will be set
 *   and prevent further calls.
 *
 ********************************************************************/
OM_uint32
gn_init( OM_uint32 * pp_min_stat )
{
   OM_uint32   major_status;

   if ( gn_gss_initialized==TRUE ) {
      (*pp_min_stat) = 0;
      return(GSS_S_COMPLETE);
   }

   /**********************************************************/
   /* First, call all initialization functions of this layer */
   /**********************************************************/

   memset(gn_mech, 0, sizeof(gn_mech) );

   major_status = sy_init( pp_min_stat );  /* Initialize system interface */
   if ( major_status!=GSS_S_COMPLETE )
      return(major_status);

   major_status = gn_init_tag_tables( pp_min_stat ) ;
   if ( major_status!=GSS_S_COMPLETE )
      return(major_status);

   /*********************************************************************/
   /* Then call all initialization functions of the layers further down */
   /*********************************************************************/

   DEBUG_INIT(pp_min_stat, major_status)   /* Initialize Debug Framework */

   major_status = gn_mechanism_init( pp_min_stat );

   if ( major_status==GSS_S_COMPLETE ) {
      gn_gss_initialized = TRUE;
      (*pp_min_stat) = MINOR_NO_ERROR;
   }

   return(major_status);

} /* gn_init() */




/*********************************************************************
 * gn_cleanup()
 *
 *********************************************************************/
void
gn_cleanup( void )
{
   gn_mech_tag_et   mech_tag;

   /* loop over all initialized mechanisms and call their cleanup functions */

   for ( mech_tag=mech_last_tag ; mech_tag>MECH_INVALID_TAG ; mech_tag-- ) {
      if ( gn_mech[mech_tag]!=NULL ) {
	 if ( gn_mech[mech_tag]->fp_cleanup != NULL ) {
	    ((gn_mech[mech_tag])->fp_cleanup)();
	 }
	 gn_mech[mech_tag] = NULL;
         mech_last_tag--;
      }
   }

   sy_cleanup();

   gn_gss_initialized = FALSE;

   return;

} /* gn_cleanup() */




/*********************************************************************
 * gn_register_mech()
 *
 *********************************************************************/
OM_uint32
gn_register_mech( OM_uint32                * pp_min_stat,
		  struct gn_mechanism_s    * p_mech,
		  gn_mech_tag_et           * pp_mech_tag,
		  OM_uint32                * pp_minor_msel )
{
   char           * this_Call = " gn_register_mech(): ";
   OM_uint32       maj_stat;
   gn_mech_tag_et  mech_tag;

   maj_stat = gn_oid_to_mech_tag( pp_min_stat, p_mech->mech_oid, &mech_tag);
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   if ( mech_tag!=MECH_INVALID_TAG ) {
      DEBUG_ERR((tf, "ERROR:%scannot register mechanism more than once\n",
		 this_Call ));
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   maj_stat = gn_add_mech_tag( pp_min_stat, p_mech->mech_oid,
			       &mech_tag );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   (*pp_mech_tag)   = mech_tag;
   (*pp_minor_msel) = mech_tag;

   gn_mech[mech_last_tag] = p_mech;

   return(GSS_S_COMPLETE);

} /* gn_register_mech() */




/*********************************************************************
 * gn_register_min_stat()
 *
 * Register minor_status codes that can be resolved even
 * when mechanism initialization fails
 *
 *********************************************************************/
OM_uint32
gn_register_min_stat( OM_uint32		  * pp_min_stat,
		      gn_min_stat_desc    * p_min_stat_list )
{
   gn_min_stat_desc * min_list;
   int i;

   (*pp_min_stat) = 0;

   for (i=0 ; i<ARRAY_ELEMENTS(registered_min_stat) ; i++ ) {
      if ( registered_min_stat[i]==p_min_stat_list ) {
	 /* already registered ... */
	 return(GSS_S_COMPLETE);
      }
      if ( registered_min_stat[i]==0 ) {
	 registered_min_stat[i] = p_min_stat_list;
	 for ( min_list = p_min_stat_list ; min_list->label!=NULL ; min_list++ ) {
	    min_list->value = MINOR_REGISTERED(gn_min_stat_counter);
	    gn_min_stat_counter++;
	 }
	 return(GSS_S_COMPLETE);
      }
   }

   (*pp_min_stat) = MINOR_INTERNAL_BUFFER_OVERRUN;

   return(GSS_S_FAILURE);

} /* gn_register_min_stat() */






/*********************************************************************
 * gn_init_tag_tables()
 *
 *  Clear and initialize the tag tables with default nametypes
 *********************************************************************/
OM_uint32
gn_init_tag_tables( OM_uint32 * pp_min_stat )
{
   OM_uint32   maj_stat;

   (*pp_min_stat) = 0;
   maj_stat       = GSS_S_COMPLETE;

   /* Clear tag table of available/known mechanism OIDs */
   memset( mech_oids, 0, sizeof(mech_oids) );
   mech_last_tag = 0;

   /* Empty the list of available mechanism OIDs which can be */
   /* obtained via gss_indicate_mechs()                       */
   gn_avail_mech_oids.count    = 0;
   gn_avail_mech_oids.elements = (gss_OID)NULL;


   /* Clear the tag table of available/known nametype OIDs */
   memset( nt_oids, 0, sizeof(nt_oids) );

   /* Fill in the known/standardized/link-exported nametype OIDs      */
   /* You HAVE to keep their order synchronized with the enumerator!! */
   nt_oids[ NT_HOSTBASED_SERVICE   ] = *GSS_C_NT_HOSTBASED_SERVICE;
   nt_oids[ NT_HOSTBASED_SERVICE_X ] = *GSS_C_NT_HOSTBASED_SERVICE_X;
   nt_oids[ NT_ANONYMOUS           ] = *GSS_C_NT_ANONYMOUS;
   nt_oids[ NT_EXPORTED_NAME       ] = *GSS_C_NT_EXPORT_NAME;
   nt_oids[ NT_USER_NAME           ] = *GSS_C_NT_USER_NAME;
   nt_oids[ NT_MACHINE_UID_NAME    ] = *GSS_C_NT_MACHINE_UID_NAME;
   nt_oids[ NT_STRING_UID_NAME     ] = *GSS_C_NT_STRING_UID_NAME;

   nt_last_tag = NT_STRING_UID_NAME;

   return(maj_stat);

} /* gn_init_tag_tables() */



/*********************************************************************
 * gn_add_nt_tag()
 *
 *  Register nametype tag in nt_tag table
 *********************************************************************/
OM_uint32
gn_add_nt_tag( OM_uint32     * pp_min_stat,		/* out */
	       gss_OID         p_oid,			/* in  */
	       gn_nt_tag_et  * pp_nt_tag )		/* out */
{
   char           * this_Call = "gn_add_nt_tag";
   OM_uint32	    maj_stat;
   gn_nt_tag_et     nt_tag;

   (*pp_min_stat) = 0;
   (*pp_nt_tag)   = NT_INVALID_TAG;

   if ( nt_last_tag >= GN_MAX_NAMETYPE_OIDS ) {
      DEBUG_ERR((tf, "ERROR: %s(): too many nametypes (max=%d)\n",
		 this_Call, (int)GN_MAX_NAMETYPE_OIDS ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   /* check if we already have this nametype registered in our table */
   maj_stat = gn_oid_to_nt_tag( pp_min_stat, p_oid, &nt_tag );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   if ( nt_tag!=NT_INVALID_TAG ) {
      /* we already know this nametype, so return its tag */ 
      (*pp_nt_tag) = nt_tag;
      return(GSS_S_COMPLETE);
   }

   /* We haven't seen this nametype before, so put it into the list */
   nt_last_tag++;

   nt_oids[nt_last_tag].length   = p_oid->length;
   nt_oids[nt_last_tag].elements = p_oid->elements;

   (*pp_nt_tag) = (gn_nt_tag_et) nt_last_tag;

   return(GSS_S_COMPLETE);

} /* gn_add_nt_tag() */




/*********************************************************************
 * gn_add_mech_tag()
 *
 *  Register mechanism tag in mech_tag table
 *********************************************************************/
OM_uint32
gn_add_mech_tag( OM_uint32        * pp_min_stat,	/* out */
		 gss_OID            p_oid,		/* in  */
		 gn_mech_tag_et   * pp_mech_tag )	/* out */
{
   char          * this_Call = "gn_add_mech_tag";
   OM_uint32       maj_stat  = GSS_S_COMPLETE;
/*   gn_mech_tag_et  mech_tag; */

   (*pp_min_stat)      = 0;
   (*pp_mech_tag)      = MECH_INVALID_TAG;

   if ( mech_last_tag >= GN_MAX_MECH_OIDS ) {
      DEBUG_ERR((tf, "ERROR: %s(): too many mechanisms (max=%d)\n",
		 this_Call, (int)GN_MAX_MECH_OIDS ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   mech_last_tag++;

   /* update the list of available mechanisms,                        */
   /* i.e. the OID_set that can be obtained via gss_indicate_mechs()  */
   gn_avail_mech_oids.count    = mech_last_tag;
   gn_avail_mech_oids.elements = &(mech_oids[MECH_FIRST_TAG]);

   mech_oids[mech_last_tag].length   = p_oid->length;
   mech_oids[mech_last_tag].elements = p_oid->elements;

   (*pp_mech_tag)   = (gn_mech_tag_et) mech_last_tag;

   return(maj_stat);

} /* gn_add_mech_tag() */





/*********************************************************************
 * gn_oid_to_nt_tag()
 *
 *  Convert nametype OID into an integer
 *********************************************************************/
OM_uint32
gn_oid_to_nt_tag( OM_uint32      * pp_min_stat,		/* out  */
		   gss_OID         p_oid,		/* in   */
		   gn_nt_tag_et  * pp_nt_tag )		/* out  */
{
   OM_uint32        maj_stat;
   Uint             i, compare;

   (*pp_min_stat) = 0;
   (*pp_nt_tag)   = NT_INVALID_TAG;

   for ( i=NT_FIRST_TAG ; i<=nt_last_tag ; i++ ) {
      if ( nt_oids[i].length>0 ) { /* skip empty table entries */
	 maj_stat = gn_compare_oid( pp_min_stat,
				    p_oid, &(nt_oids[i]), &compare );
	 if (maj_stat!=GSS_S_COMPLETE)
	    return(maj_stat);

	 if (compare==0) {
	    *pp_nt_tag = i;
	    break;
	 }
      }
   }

   return(GSS_S_COMPLETE);

} /* gn_oid_to_nt_tag () */




/*********************************************************************
 * gn_nt_tag_to_oid()
 *
 *  Return the static OID for a given nametype tag
 *********************************************************************/
gss_OID
gn_nt_tag_to_oid( gn_nt_tag_et p_nt_tag )
{
   if ( p_nt_tag>=NT_FIRST_TAG && p_nt_tag<=(gn_mech_tag_et)nt_last_tag ) {
      return( &(nt_oids[p_nt_tag]) );
   }

   return(GSS_C_NO_OID);

} /* gn_nt_tag_to_oid() */



/*********************************************************************
 * gn_mech_tag_to_oid()
 *
 *  Return the static OID for a given mech tag
 *********************************************************************/
gss_OID
gn_mech_tag_to_oid( gn_mech_tag_et p_mech_tag )
{
   if ( p_mech_tag>=MECH_FIRST_TAG && p_mech_tag<=(gn_mech_tag_et)mech_last_tag ) {
      return( &(mech_oids[p_mech_tag]) );
   }

   return(GSS_C_NO_OID);

} /* gn_mech_tag_to_oid() */




/*********************************************************************
 * gn_oid_to_mech_tag()
 *
 *  Convert a known mech OID into an integer tag
 *********************************************************************/
OM_uint32
gn_oid_to_mech_tag( OM_uint32        * pp_min_stat,
		    gss_OID            p_oid,
		    gn_mech_tag_et   * pp_mech_tag )
{
   OM_uint32        maj_stat;
   Uint             i, compare;

   (*pp_min_stat) = 0;
   (*pp_mech_tag) = MECH_INVALID_TAG;

   for ( i=(Uint)MECH_FIRST_TAG ; i<=mech_last_tag ; i++ ) {
      if ( mech_oids[i].length > 0 ) { /* skip empty table entries */
         maj_stat = gn_compare_oid( pp_min_stat,
				    p_oid, &(mech_oids[i]), &compare );
	 if (maj_stat!=GSS_S_COMPLETE)
	    return(maj_stat);

	 if (compare==0) {
	    *pp_mech_tag = i;
	    break;
	 }
      }
   }

   return(GSS_S_COMPLETE);

} /* gn_oid_to_mech_tag() */




/***************************************************************
 * gn_compare_oid()
 *
 * Compares two gss_OID objects.
 *   p_compare = 0    when p_oid1 == p_oid2
 *   p_compare != 0   other times
 *
 ***************************************************************/
OM_uint32
gn_compare_oid( OM_uint32    * pp_min_stat,
		gss_OID        p_oid1,
		gss_OID        p_oid2,
		int          * pp_compare )
{
   unsigned char *p1, *p2;
   int            l1, l2;

   (*pp_min_stat) = 0;
   (*pp_compare)  = -1;

   if ( p_oid1 == p_oid2 ) {
      (*pp_compare) = 0;
      return(GSS_S_COMPLETE);
   }

   if ( p_oid1==NULL ) {
      (*pp_compare) = -1;
      return(GSS_S_COMPLETE);
   }

   if ( p_oid2==NULL ) {
      (*pp_compare) = 1;
      return(GSS_S_COMPLETE);
   }

   l1 = (int)p_oid1->length;
   l2 = (int)p_oid2->length;
   p1 = (unsigned char *) p_oid1->elements;
   p2 = (unsigned char *) p_oid2->elements;

   if ( p1==NULL || p2==NULL || l1==0 || l2==0 ) {
      RETURN_MIN_MAJ( MINOR_INVALID_OID, GSS_S_CALL_BAD_STRUCTURE );
   }

   if ( l1 != l2 ) {
      (*pp_compare) = ( l1 - l2 );
      return(GSS_S_COMPLETE);
   }

   (*pp_compare) = memcmp( p1, p2, l1 );
   return(GSS_S_COMPLETE);

} /* gn_compare_oid() */




/************************************************************************
 * gn_gss_release_oid()							*
 *									*
 *									*
 *									*
 ***********************************************************************/
OM_uint32
gn_gss_release_oid(
	OM_uint32    FAR * p_min_stat,
	gss_OID      FAR * p_in_oid
     )
{
   char      * this_Call = " gn_gss_release_oid(): ";

   (*p_min_stat) = 0;

   if ( (*p_in_oid) == GSS_C_NO_OID ) {
      DEBUG_ACTION((tf, "  A:%signoring NULL handle\n", this_Call ))
      return(GSS_S_COMPLETE);
   }

   /* Compare p_in_oid with static (internal) standardized object ids */
   /* This catches references to the static data objects exported at  */
   /* the link level                                                  */
   if ( (*p_in_oid) >= &(gn_gss_oids[0])
	&&  (*p_in_oid) <= &(gn_gss_oids[gn_gss_oids_num-1]) ) {

      DEBUG_ACTION((tf, "  A:%signoring statically exported nametype OID\n",
		        this_Call ))
ignore:
      (*p_in_oid) = GSS_C_NO_OID;
      return(GSS_S_COMPLETE);
   }

   /* Compare p_in_oid to the table of registered mechanism OIDs */
   if ( (*p_in_oid)>= &(mech_oids[0])
	&&  (*p_in_oid)<=&(mech_oids[mech_last_tag]) ) {

      DEBUG_ACTION((tf, "  A:%signoring static mechanism OID\n", this_Call ))
      goto ignore;
   }

   /* Compare p_in_oid to the table of registered nametype OIDs */
   if ( (*p_in_oid)>= &(nt_oids[0])
	&&  (*p_in_oid)<=&(nt_oids[nt_last_tag]) ) {

      DEBUG_ACTION((tf, "  A:%signoring static nametype OID\n", this_Call ))
      goto ignore;
   }

   /* The OID was not recognized as a gssapi static object  */
   /* therefore we try to free() the components, as it may  */
   /* have been dynamically created by gss_str_to_oid() ... */

   DEBUG_ACTION((tf, "  A:%sfree()ing a dynamically allocated OID\n",
		     this_Call ))

   /* sy_free()/sy_clear_free() can handle NULL */
   /* clear the elements in gss_OID and the elements pointer */
   sy_clear_free( &((*p_in_oid)->elements), (*p_in_oid)->length );

   /* clear the gss_OID struct and handle */
   sy_clear_free( (void **)p_in_oid, sizeof(**p_in_oid) );

   return(GSS_S_COMPLETE);

} /* gn_gss_release_oid() */




/************************************************************************
 * gn_gss_indicate_mechs()						*
 *									*
 *									*
 *									*
 ***********************************************************************/
OM_uint32
gn_gss_indicate_mechs( OM_uint32 FAR    * pp_min_stat,
		       gss_OID_set  FAR * pp_out_oid_set )
{
   char      * this_Call = " gn_gss_indicate_mechs(): ";
   OM_uint32   maj_stat;

   (*pp_min_stat = 0);

   if ( gn_avail_mech_oids.count==0 ) 
      RETURN_MIN_MAJ( MINOR_NO_MECHANISM_AVAIL, GSS_S_FAILURE );

   maj_stat = gn_copy_oid_set( pp_min_stat, &gn_avail_mech_oids,
			       pp_out_oid_set );

   if ( maj_stat==GSS_S_COMPLETE ) {
      DEBUG_ACTION((tf, "  A:%sreturning OID_set of available mechanisms\n",
		        this_Call ))
   }

   return(maj_stat);

} /* gn_gss_indicate_mechs() */




/************************************************************************
 * gn_gss_release_oid_set()						*
 *									*
 *									*
 *									*
 ***********************************************************************/
OM_uint32
gn_gss_release_oid_set(
        OM_uint32      *pp_min_stat,          /* minor_status	*/
        gss_OID_set    *p_in_oids            /* set		*/
     )
{
   char             * this_Call = " gn_gss_release_oid_set(): ";
   OM_uint32          maj_stat;           
   gss_OID_desc     * poid;
   Uint               i; 

   (*pp_min_stat) = 0;
   maj_stat       = GSS_S_COMPLETE;

   if ( p_in_oids==NULL ) {
      RETURN_MIN_MAJ(MINOR_NO_ERROR, GSS_S_CALL_BAD_STRUCTURE|GSS_S_FAILURE );
   }

   if ( (*p_in_oids)==GSS_C_NO_OID_SET ) {
      DEBUG_ACTION((tf, "  A:%signoring GSS_C_NO_OID_SET\n", this_Call))
      return(maj_stat);
   }

#if 0
   /* Providing OID_set pointers into static memory is no longer  */
   /* supported, so we don't do it and don't have to check for it */ 
   if ( (*p_in_oids)==&gn_avail_mech_oids ) {
      DEBUG_ACTION((tf, "  A:%snot freeing my own static mech_list\n",
		        this_Call))
      (*p_in_oids)= GSS_C_NO_OID_SET;
      return(maj_stat);
   }
#endif

   if ( (*p_in_oids)->elements==NULL && (*p_in_oids)->count==0 ) {
      DEBUG_ACTION((tf, "  A:%sfreeing empty OID_set\n", this_Call ))
      sy_free( *p_in_oids );
      (*p_in_oids) = GSS_C_NO_OID_SET;
      return(maj_stat);
   }

   if ( (*p_in_oids)->elements==NULL || (*p_in_oids)->count==0 ) {
      DEBUG_ERR((tf, "ERROR:%sinvalid OID_set parameter\n", this_Call))
      RETURN_MIN_MAJ( MINOR_INVALID_OID_SET, GSS_S_FAILURE );
   }


   DEBUG_ACTION((tf, "  A:%sfree()ing all elements of OID_set\n",
		 this_Call ))

   poid = &((*p_in_oids)->elements[0]);

   for( i=0 ; i<(*p_in_oids)->count ; i++, poid++ ) {
      if ( poid->length==0 || poid->elements==NULL ) {
	 DEBUG_ERR((tf, "ERROR:%soid #%d of OID_set is invalid!\n",
		        this_Call, i ))
      } else {
	 sy_clear_free( (void **)&(poid->elements), poid->length );
	 poid->length = 0;
      }
   }

   poid = &((*p_in_oids)->elements[0]);

   /* clear and free the gss_OID array */
   sy_clear_free( (void **)&poid, (*p_in_oids)->count * sizeof(*poid) );

   /* clear and free the top-level gss_OID_set structure and the handle */
   sy_clear_free( (void **)p_in_oids, sizeof(**p_in_oids) );
      
   return(maj_stat);

} /* gn_gss_release_oid_set() */







/************************************************************************
 * gn_gss_inquire_mechs_for_name()					*
 *									*
 *									*
 *									*
 ***********************************************************************/
OM_uint32
gn_gss_inquire_mechs_for_name(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_name_t		p_in_name,	/* input_name		*/
	gss_OID_set	FAR *	pp_out_mechs	/* mechanism_oids	*/
     )
{
   char            * this_Call = " gn_gss_inquire_mechs_for_name(): ";
   gn_name_desc    * pname;
   Uchar           * tmp_name      = NULL;
   size_t            tmp_name_len  = 0;
   OM_uint32         maj_stat;
   gn_mech_tag_et    mech_tags[GN_MAX_MECH_OIDS+1];
   int               num_tags      = 0;
   gn_mech_tag_et    mech_tag;
   gn_nt_tag_et      tmp_nt_tag;

   (*pp_min_stat)  = MINOR_NO_ERROR;
   (*pp_out_mechs) = GSS_C_NO_OID_SET;

   maj_stat = gn_check_name( pp_min_stat, &p_in_name, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   pname = (gn_name_desc *) p_in_name;

   if ( pname->mech_tag!=MECH_INVALID_TAG ) {

      mech_tags[0] = pname->mech_tag;
      num_tags     = 1;

   } else { /* pname->mech_tag == MECH_INVALID_TAG */
      
      num_tags = 0;
      for( mech_tag=MECH_FIRST_TAG ; mech_tag<=(gn_mech_tag_et)mech_last_tag ; mech_tag++ ) {
	 maj_stat = (gn_mech[mech_tag]
		            ->fp_canonicalize_name)( pp_min_stat,
						     pname->prname,
						     pname->prname_len,
						     pname->nt_tag,
						     &tmp_name,
						     &tmp_name_len,
						     &tmp_nt_tag );
	 if ( maj_stat==GSS_S_COMPLETE ) {
	    mech_tags[num_tags] = mech_tag;
	    num_tags++;
	 }
	 sy_clear_free( &tmp_name, tmp_name_len );
      }

   } /* pname->mech_tag == MECH_INVALID_TAG */

   if ( num_tags==0 ) {
      /* None of the mechanisms accepted the name */ 
      return(GSS_S_BAD_NAME);
   }

   maj_stat = gn_create_mech_oid_set( pp_min_stat,
				      mech_tags, num_tags, pp_out_mechs );
   return(maj_stat);

} /* gn_gss_inquire_mechs_for_name() */



/*
 * gn_remaining_time()
 *
 *
 */
OM_uint32
gn_remaining_time( time_t expires_at )
{
   time_t  xtime;

   if ( expires_at==0 || expires_at==GSS_C_INDEFINITE )
      return( (OM_uint32)expires_at );

   xtime = time(NULL);
   if ( expires_at>xtime )
      return( (OM_uint32)(expires_at - xtime) );

   return( 0 );  /* expired */

} /* gn_remaining_time() */
