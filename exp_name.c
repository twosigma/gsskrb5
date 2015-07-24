#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/exp_name.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/exp_name.c#1 $
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
 * gn_gss_export_name()
 *
 *
 */
OM_uint32
gn_gss_export_name(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_name_t		p_in_name,	/* input_name		*/
	gss_buffer_t		p_out_aclkey	/* output_name_blob	*/
     )
{
   char          * this_Call = " gn_gss_export_name(): ";
   gn_name_desc  * pname;
   OM_uint32       maj_stat;

   (*pp_min_stat)   = 0;

   if ( p_out_aclkey==NULL ) {
      RETURN_MIN_MAJ( MINOR_INVALID_BUFFER,
		      GSS_S_CALL_INACCESSIBLE_WRITE | GSS_S_FAILURE );
   }

   p_out_aclkey->length = 0;
   p_out_aclkey->value  = NULL;

   maj_stat = gn_check_name( pp_min_stat, &p_in_name, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   pname = (gn_name_desc *)p_in_name;

   if ( pname->mech_tag==MECH_INVALID_TAG ) {
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_NAME_NOT_MN );
   }

   maj_stat = gn_create_expname_token( pp_min_stat,
				       pname->name, pname->name_len,
				       pname->mech_tag, p_out_aclkey );

   return(maj_stat);

} /* gn_gss_export_name() */
