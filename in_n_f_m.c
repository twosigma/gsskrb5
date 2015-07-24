#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/in_n_f_m.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/in_n_f_m.c#1 $
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
 * gn_gss_inquire_names_for_mech()
 *
 *
 */
OM_uint32
gn_gss_inquire_names_for_mech(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_OID			p_in_mech,	/* mechanism_oid	*/
	gss_OID_set	FAR *	pp_out_nametypes /* name_types		*/
     )
{
   char            * this_Call = " gn_gss_inquire_names_for_mech(): ";
   OM_uint32         maj_stat;
   gn_mech_tag_et    mech_tag;
   gn_nt_tag_et      nt_tags[GN_MAX_NAMETYPE_OIDS + 1];
   int               nt_avail[GN_MAX_NAMETYPE_OIDS + 1];
   int               i, num;

   (*pp_min_stat)      = 0;
   (*pp_out_nametypes) = GSS_C_NO_OID_SET;
   mech_tag            = MECH_INVALID_TAG;
   num                 = 0;
   memset( nt_avail, 0, sizeof(nt_avail) );
   memset( nt_tags, 0, sizeof(nt_tags) );

   maj_stat = gn_oid_to_mech_tag( pp_min_stat, p_in_mech, &mech_tag );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   if ( mech_tag == MECH_INVALID_TAG ) {
      DEBUG_ERR((tf, "ERROR:%sunknown mechanism\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_BAD_MECH );
   }

   maj_stat = (gn_mech[mech_tag]
	              ->fp_inquire_nametypes)( pp_min_stat, nt_avail );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   if ( sy_uid_avail()!=FALSE  &&  nt_avail[NT_USER_NAME]!=FALSE ) {
      nt_avail[NT_MACHINE_UID_NAME] = TRUE;
      nt_avail[NT_STRING_UID_NAME]  = TRUE;
   }

   if ( nt_avail[NT_HOSTBASED_SERVICE]==TRUE ) {
      nt_avail[NT_HOSTBASED_SERVICE_X] = TRUE;
   }

   for ( i=0, num=0; i<ARRAY_ELEMENTS(nt_tags) ; i++ ) {
      if ( nt_avail[i]!=FALSE ) {
	 nt_tags[num] = (gn_nt_tag_et)i;
	 num++;
      }
   }

   maj_stat = gn_create_nt_oid_set( pp_min_stat,
				    nt_tags,
				    num,
				    pp_out_nametypes );

   return(maj_stat);

} /* gn_gss_inquire_names_for_mech() */
