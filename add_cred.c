#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/add_cred.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/add_cred.c#1 $
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
 * gn_gss_add_cred()
 *
 *
 */
OM_uint32
gn_gss_add_cred(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_cred_id_t		p_in_cred,	/* input_cred_handle	*/
	gss_name_t		p_in_name,	/* desired_name		*/
	gss_OID			p_in_mech,	/* desired_mech		*/
	gss_cred_usage_t	p_in_usage,	/* cred_usage		*/
	OM_uint32		p_in_ini_lifet,	/* initiator_time_req	*/
	OM_uint32		p_in_acc_lifet, /* acceptor_time_req	*/
	gss_cred_id_t	FAR *	p_out_cred,	/* output_cred_handle	*/
	gss_OID_set	FAR *	p_out_mechs,	/* actual_mechs		*/
	OM_uint32	FAR *	p_out_ini_lifet,/* initiator_time_rec	*/
	OM_uint32	FAR *	p_out_acc_lifet	/* acceptor_time_rec	*/
     )
{
   char        * this_Call = "gn_gss_add_cred";
   OM_uint32     maj_stat;

   (*pp_min_stat) = MINOR_NOT_YET_IMPLEMENTED;
   maj_stat       = GSS_S_FAILURE;


   return(maj_stat);

} /* gn_gss_add_cred() */
