#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/ntlm/ntlmoid.c#3 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/ntlm/ntlmoid.c#3 $
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


#include "ntlmdefs.h"


/************************************************************************
 * the following structure contains (static readonly) gss_OID values    *
 ***********************************************************************/
static gss_OID_desc ntlm_mech_oids[] = {
  /* GSSAPI Mechanism OID for SAPNTLM, a gssapi mechanism based on         */
  /* Microsoft's NT Lan Manager authentication				   */
  /* iso(1) org(3) dod(6) internet(1) private(4) enterprise(1)             */
  /*         + SAP(694) security(2) gssapi(1) ntlm(2)                      */
  { 10, (void *)"\053\006\001\004\001\205\066\002\001\002"      }
};

static gss_OID_desc ntlm_nt_oids[] = {
  /* GSSAPI Nametype OID for SAPNTLM to tag the Domain\User syntax that    */
  /* is being used by Microsoft's NT Lan Manager protocol		   */
  /* iso(1) org(3) dod(6) internet(1) private(4) enterprise(1)             */
  /*         + SAP(694) security(2) gssapi(1) ntlm(2) domainuser(1)        */
  { 11, (void *)"\053\006\001\004\001\205\066\002\001\002\001"  }
};

gn_nt_tag_et ntlm_nt_set[10];
Uint         ntlm_nt_count = 0;


gss_OID      ntlm_mech_oid   = &(ntlm_mech_oids[0]);
gss_OID      ntlm_nt_oid     = &(ntlm_nt_oids[0]);


gn_nt_tag_et    ntlm_nt_tag     = NT_INVALID_TAG;
gn_mech_tag_et  ntlm_mech_tag   = MECH_INVALID_TAG;

/*
 * ntlm_init_oid_tags()
 *
 *  Description:
 *    Initialize the tables for translating OIDs into OID_tags
 *
 */

OM_uint32
ntlm_init_nt_oid_tags( OM_uint32 * pp_min_stat )
{
   OM_uint32  maj_stat;

   (*pp_min_stat) = 0;

   /******************************************************/
   /* register ALL+static+readonly private nametype oids */
   /******************************************************/
   maj_stat = gn_add_nt_tag( pp_min_stat,
			     ntlm_nt_oid, &ntlm_nt_tag );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ntlm_nt_set[ntlm_nt_count++] = NT_EXPORTED_NAME;
   ntlm_nt_set[ntlm_nt_count++] = NT_USER_NAME;
   ntlm_nt_set[ntlm_nt_count++] = ntlm_nt_tag;

   return(GSS_S_COMPLETE);

} /* ntlm_init_nt_oid_tags() */

