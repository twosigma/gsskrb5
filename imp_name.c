#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/imp_name.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/imp_name.c#1 $
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


/*********************************************************************
 *  gn_gss_import_name()
 *
 *********************************************************************/
OM_uint32
gn_gss_import_name(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_buffer_t		p_in_buf,	/* input_name_buffer	*/
        gss_OID			p_in_oid,	/* input_name_type	*/
        gss_name_t	FAR *	pp_out_name	/* output_name		*/
     )
{
   char           * this_Call = "gn_gss_import_name";
   char             tmpbuf[GN_MAX_NAMELEN];
   gn_name_desc     gn_name;
   OM_uint32        maj_stat;
   gn_nt_tag_et     nt_tag;

   (*pp_min_stat) = 0;              /* init return value */
   (*pp_out_name) = GSS_C_NO_NAME;  /* init return value */

   memset( &gn_name, 0, sizeof(gn_name) ); /* DON'T TOUCH THIS! */
   gn_name.magic_cookie = COOKIE_NAME;
   gn_name.mech_tag     = MECH_INVALID_TAG;

   nt_tag               = NT_INVALID_TAG;

   if ( p_in_oid==GSS_C_NO_OID ) {

      nt_tag		= NT_DEFAULT;

   } else {

      /* explicit nametype OID supplied; convert it into a nt_tag */
      maj_stat = gn_oid_to_nt_tag( pp_min_stat, p_in_oid, &nt_tag );
      if (maj_stat!=GSS_S_COMPLETE)
	 return(maj_stat);

      if ( nt_tag == NT_INVALID_TAG ) {
         /* Supplied Nametype OID is not recognized, return an error */
	 RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_BAD_NAMETYPE );
      }

   } /* p_in_oid != GSS_C_NO_OID */


   if ( nt_tag==NT_ANONYMOUS ) {

      /* create anonymous internal name */
      gn_name.prname     = "#@# Mr. Anonymous #@#";
      gn_name.prname_len = strlen(gn_name.name);

   } else { /* nt_tag!=NT_ANONYMOUS */

      if ( p_in_buf==GSS_C_NO_BUFFER
	   ||  (p_in_buf->value==NULL)  ||  (p_in_buf->length==0) ) {
	 RETURN_MIN_MAJ( MINOR_INVALID_BUFFER, GSS_S_BAD_NAME );
      }

      if ( nt_tag==NT_EXPORTED_NAME ) {

	 void           * exp_name;
	 size_t           exp_name_len;
	 /* this name is canonical by its very nature, so it's directly */
	 /* torn apart and passed to the mechanism                      */
	 maj_stat = gn_parse_expname_token( pp_min_stat, p_in_buf,
					    &(gn_name.mech_tag),
					    &exp_name,
					    &exp_name_len );
	 if ( maj_stat!=GSS_S_COMPLETE ) {
	    return(maj_stat);
	 }

/* Cleanup ALERT:  the following call create a dynamic object for */
/*                 (gn_name.name), which needs to be freed when   */
/*                 there is an error later on!                    */
	 maj_stat = (gn_mech[gn_name.mech_tag]
		           ->fp_import_expname)( pp_min_stat,
						 exp_name, exp_name_len,
						 &(gn_name.name),
						 &(gn_name.name_len),
						 &nt_tag );
	 if ( maj_stat!=GSS_S_COMPLETE ) {
	    return(maj_stat);
	 }

      } else { /* nt_tag != NT_EXPORTED_NAME */

	 /* Check for NUL characters within supplied printable name */
	 if ( memchr( p_in_buf->value, 0, p_in_buf->length )!=NULL ) {
	    RETURN_MIN_MAJ( MINOR_NAME_CONTAINS_NUL, GSS_S_BAD_NAME );
	 }

	 if ( nt_tag==NT_HOSTBASED_SERVICE
	      ||  nt_tag==NT_HOSTBASED_SERVICE_X ) {
	    /* Hostbased Service Names get a special treatment,   */
	    char     * ptr;
	    size_t     len, svclen;

	    len = p_in_buf->length;
	    if ( len+4 > sizeof(tmpbuf) ) {
	       RETURN_MIN_MAJ( MINOR_NAME_TOO_LONG, GSS_S_BAD_NAME );
	    }
	    memcpy( tmpbuf, p_in_buf->value, len );
	    tmpbuf[len]= '\0';

	    ptr = (char *) memchr(tmpbuf, '@', len);

	    if ( ptr!=NULL ) {
	       /* '@' found in supplied name, check hostname */ 
	       svclen = ( ptr - tmpbuf ) + 1;

	       if ( ptr==tmpbuf ) {
		  RETURN_MIN_MAJ( MINOR_SERVICE_MISSING, GSS_S_BAD_NAME );
	       }
	       if ( svclen==len ) {
		  RETURN_MIN_MAJ( MINOR_HOSTNAME_MISSING, GSS_S_BAD_NAME );
	       }
	    } else { /* ptr==NULL    i.e. no '@' found in supplied name */

	       tmpbuf[len++] = '@';
	       tmpbuf[len]   = '\0';
	       maj_stat = sy_get_hostname( pp_min_stat,
					   &(tmpbuf[len]),
					   sizeof(tmpbuf) - len - 3 );

       	    } /* ptr==NULL   i.e. no '@' found in supplied name */

	    if (maj_stat!=GSS_S_COMPLETE)
	       return(maj_stat);

	    gn_name.prname        = (char *) tmpbuf;
	    gn_name.prname_len    = strlen(tmpbuf);

	    /* tag all hostbased service names with NT_HOSTBASED_SERVICE only! */
	    nt_tag = NT_HOSTBASED_SERVICE;

	 } else if ( nt_tag==NT_MACHINE_UID_NAME
		     ||  nt_tag==NT_STRING_UID_NAME ) {

	    maj_stat = sy_uid_2_name( pp_min_stat,
				      p_in_buf->value, p_in_buf->length,
				      (nt_tag==NT_MACHINE_UID_NAME),
				      tmpbuf, sizeof(tmpbuf)-1,
				      &(gn_name.prname_len) );
	    if (maj_stat!=GSS_S_COMPLETE)
	       return(maj_stat);

	    gn_name.prname     = tmpbuf;
	    nt_tag             = NT_USER_NAME;

	 } else { /* nt_tag!=NT_EXPORTED_NAME */

	    /* All other nametypes are lazy evaluated                      */
	    /* just copy the supplied buffer.                              */
	    /* This also applies to name_type = GSS_C_NO_OID               */

	    gn_name.prname     = (char *) p_in_buf->value;
	    gn_name.prname_len =          p_in_buf->length;

	 } /* nt_tag == anything else */

      } /* nt_tag != NT_EXPORTED_NAME */

   } /* nt_tag != NT_ANONYMOUS */

   gn_name.nt_tag       = nt_tag;

   /* Xerox gn_name into a completely dynamically allocated internal name */
   maj_stat = gn_duplicate_name( pp_min_stat,
				 &gn_name, (gn_name_desc **)pp_out_name );

/* Cleanup the dynamic object (gn_name.name), if one was allocated */ 
   sy_clear_free( (void **) &(gn_name.name), (gn_name.name_len) );

   if (maj_stat!=GSS_S_COMPLETE) {
      return(maj_stat);
   }

   ((gn_name_desc *)*pp_out_name)->first = (*pp_out_name);
   ((gn_name_desc *)*pp_out_name)->next  = NULL;

   return( GSS_S_COMPLETE );

} /* gn_gss_import_name() */





OM_uint32
gn_check_name( OM_uint32    * pp_min_stat,
	       gss_name_t   * p_name,
	       char         * p_call_name )
{
   gn_name_desc  * gn_name;
   OM_uint32       maj_stat  = GSS_S_COMPLETE;

   (*pp_min_stat)  = MINOR_NO_ERROR;

   if ( p_name==NULL ) {
      RETURN_MIN_MAJ( MINOR_INVALID_NAME,
		      GSS_S_CALL_BAD_STRUCTURE | GSS_S_FAILURE );
   }

   gn_name = *p_name;

   if ( gn_name==NULL  ||  gn_name->magic_cookie!=COOKIE_NAME ) {
      RETURN_MIN_MAJ( MINOR_INVALID_NAME, GSS_S_BAD_NAME );
   }

   if ( gn_name->first != gn_name ) {
      DEBUG_ERR((tf, "Internal ERROR:%sthis is not an exposed internal name\n",
		 p_call_name ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   if ( gn_name->mech_tag==MECH_INVALID_TAG ) {

      if ( gn_name->prname==NULL || gn_name->prname_len==0 ) {
	 DEBUG_ERR((tf, "Internal ERROR:%snon-MN missing printable data\n",
		    p_call_name ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

   } else if ( gn_name->name==NULL
	       ||  gn_name->name_len==0 ) {
      DEBUG_ERR((tf, "Internal ERROR:%smech name missing private data\n",
		 p_call_name ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );

   } else if ( gn_name->nt_tag==NT_INVALID_TAG ) {
      DEBUG_ERR((tf, "Internal ERROR:%smech name missing nametype\n",
		 p_call_name ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );

   }      

   return(maj_stat);

} /* gn_check_name() */
 

