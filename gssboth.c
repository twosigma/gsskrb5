#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/gssboth.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/gssboth.c#1 $
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


extern OM_uint32 ntlm_initialize( OM_uint32 * );
extern OM_uint32 krb5_initialize( OM_uint32 * );


OM_uint32
gn_mechanism_init( OM_uint32 * pp_min_stat )
{
   OM_uint32  maj_stat1;
   OM_uint32  maj_stat2;
   OM_uint32  min_stat1;
   OM_uint32  min_stat2;

   maj_stat1 = krb5_initialize( &min_stat1 );
   maj_stat2 = ntlm_initialize( &min_stat2 );

   if ( maj_stat1==GSS_S_COMPLETE ) {
      /* If Kerberos 5 SSP was initialized successfully,            */
      /* then return success independent of NTLM SSP's availability */
      (*pp_min_stat) = min_stat1;
      return(maj_stat1);
   }
   
   if ( maj_stat2==GSS_S_COMPLETE ) {
      /* If NTLM SSP was initialized successfully, then return */
      /* success independent of Kerberos 5 SSP's availability  */
      (*pp_min_stat) = min_stat2;
      return(maj_stat2);
   }

   /* When neither Kerberos 5 SSP nor NTLM SSP are available, */
   /* then return the error from NTLM SSP initialization      */
   /* because NTLM SSP should ALWAYS be available!            */
   (*pp_min_stat) = min_stat2;
   return(maj_stat2);

} /* gn_mechanism_init() */
