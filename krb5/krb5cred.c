#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/krb5/krb5cred.c#5 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/krb5/krb5cred.c#5 $
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


#include "krb5defs.h"

OM_uint32 krb5_determine_cred_owner( OM_uint32   *  pp_min_stat,
				     CredHandle  *  p_Cred,
				     char        ** pp_buf,
				     size_t      *  pp_buf_len );

OM_uint32 krb5_fallback_def_owner(   OM_uint32   *  pp_min_stat,
                                     char        ** pp_buf,
                                     size_t      *  pp_buf_len );


/*********************************************************************
 * krb5_check_cred()
 *
 *********************************************************************/
OM_uint32		    
krb5_check_cred( OM_uint32        *  pp_min_stat,
		 void            **  pp_cred,
		 char             *  p_call_name
     )
{
    krb5_cred_desc  * pcred;

    if ( pp_cred == NULL || (*pp_cred)==NULL ) {
       DEBUG_ERR((tf, "Internal ERROR: %s(): NULL priv_cred handle\n",
		  p_call_name ))
       RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
    }

    pcred = (*pp_cred);
    if ( pcred->magic_cookie != KRB5_CRED_COOKIE ) {
       DEBUG_ERR((tf, "Internal ERROR: %s(): invalid priv_cred (bad cookie)\n",
		  p_call_name ))
       RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
    }

    return(GSS_S_COMPLETE);

} /* krb5_check_cred() */



/*********************************************************************
 * krb5_acquire_cred()
 *
 * Description:
 *   Acquire a Credentials Handle from SSPI for the supplied name
 *   (or for the default identity if no name is specified).
 *
 * BUGBUG: SSPI credentials differ considerably from GSS-API
 *         credentials:
 *
 *   NTLM:
 *      Acceptor ("INBOUND"):   The supplied pAuthData structure is ignored.
 *                              (NTLM is a challenge-response authentication,
 *                               the acceptor can NOT be authenticated to
 *                               the initiator, i.e. mutual auth is not supported)
 *
 *      Initiator ("OUTBOUND"): The supplied pAuthData is used, but can not be
 *                              locally verified.
 *                              (The supplied pAuthData (Username,Domain,Password)
 *                               will be used for the authentication to the acceptor
 *                               during context establishment)   
 *                               
 *
 *   Kerberos:
 *	Acceptor ("INBOUND"):   The supplied pszPrincipal name is ignored
 *   W2K build 2031 behaviour:  pAuthIdentity can be used to access specific
 *                              credentials.  The realm can be supplied via DomainName,
 *                              the rest of the name goes into UserName.  If DomainName
 *                              is given with NULL (not ""), then the realm name
 *                              is filled in from the loacal realm name
 *
 *      Initiator("OUTBOUND"):  The supplied pszPrincipal name is ignored
 *   W2K build 2031 behaviour:  pAuthIdentity can be used to access specific
 *                              credentials.  The realm can be supplied via DomainName,
 *                              the rest of the name goes into UserName.  If DomainName
 *                              is given with NULL (not ""), then the realm name
 *                              is filled in from the loacal realm name
 *     
 *********************************************************************/
OM_uint32
krb5_acquire_cred( OM_uint32          * pp_min_stat,    /* out  */
		   Uchar              * p_name,         /* in   */
		   size_t               p_name_len,	/* in   */
		   gn_cred_usage_et     p_usage,	/* in   */
		   void              ** pp_cred,   	/* out  */
		   time_t	      * pp_expires_at   /* out  */
     )
{
   char			   * this_Call    = "krb5_acquire_cred";
   Uchar                   * principal    = NULL;   /* temporary */
   krb5_cred_desc	   * pcred;
   char                    * user         = NULL;   /* temporary */
   char                    * realm        = NULL;   /* temporary */
   char                    * pszPrincipal = NULL;
   size_t                    user_len     = 0;
   size_t                    realm_len    = 0;
   size_t                    principal_len;
   int			     cred_ok;
   ULONG	             CredentialUse;
   SECURITY_STATUS	     SecStatus;
   SEC_WINNT_AUTH_IDENTITY   AuthIdentity;
   SEC_WINNT_AUTH_IDENTITY * pAuthIdentity;
   OM_uint32		     maj_stat  = GSS_S_COMPLETE;

   if ( pp_cred==NULL ) {
      DEBUG_ERR((tf, "Internal ERROR: %s(): NULL pp_cred parameter!\n",
		     this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   (*pp_min_stat)	      = 0;
   (*pp_cred)		      = NULL;
   pcred		      = NULL;   /* dynamic return object, release on error */
   principal                  = NULL;
   principal_len              = 0;
   pAuthIdentity              = NULL;

   if ( pp_expires_at!=NULL )
      (*pp_expires_at) = 0;

   switch( p_usage ) {
      case GN_CRED_ACCEPT:    CredentialUse = SECPKG_CRED_INBOUND;    break;
      case GN_CRED_INITIATE:  CredentialUse = SECPKG_CRED_OUTBOUND;   break;
      case GN_CRED_BOTH:      CredentialUse = SECPKG_CRED_BOTH;       break;
      default:
         DEBUG_ERR((tf, "Internal ERROR: %s(): invalid usage (%u)"
	                " for privat cred!\n",
		        this_Call, (unsigned int)p_usage ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

/* Cleanup ALERT:  the following call creates a dynamic object for */
/*                 (pcred), which needs to be freed when there is  */
/*                 an error later on!                              */

   pcred = sy_calloc( sizeof(*pcred) );
   if (pcred==NULL) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   pcred->magic_cookie    = KRB5_CRED_COOKIE;
   pcred->usage           = p_usage;
   SecInvalidateHandle( &(pcred->sspi_cred) );

   if ( p_name==NULL || p_name_len==0 ) {

      /* SSPI docs specify to use a Pointer to a NUL-terminated string here */
      pszPrincipal = "";

   } else {

      maj_stat = krb5_parse_name( pp_min_stat, p_name, p_name_len,
				  &realm, &realm_len, &user, &user_len );
      if (maj_stat!=GSS_S_COMPLETE)
	  goto error;

      maj_stat = krb5_compose_name( pp_min_stat, realm, user, &principal, &principal_len );
      if (maj_stat!=GSS_S_COMPLETE)
	  goto error;

      /* SSPI BUG in Windows2000 Beta 3 (build 2031):               */
      /* If we do this for OUTBOUND credentials,                    */
      /* the default SSPI initiating credentials will be corrupted  */
      /* and to get them "fixed", one must log out and log in again */

      memset( &AuthIdentity, 0, sizeof(AuthIdentity) );
      AuthIdentity.Domain       = realm;
      AuthIdentity.DomainLength = (ULONG)realm_len;
      AuthIdentity.User         = user;
      AuthIdentity.UserLength   = (ULONG)user_len;

      AuthIdentity.Flags        = SEC_WINNT_AUTH_IDENTITY_ANSI;
      pAuthIdentity             = &AuthIdentity;

      /* It should be ok to pass the full Kerberos principal name  */
      /* but up to windows2000 rc1 the SSPI credentials management */
      /* was broken and got confused by this parameter             */
      /* (23-Oct-2003) The pszPrincipal parameter still has no     */
      /* apparent effect on Kerberos SSP up to and including W2K3  */
      if ( principal_len>0 && *principal!=0 ) {
         pszPrincipal              = principal;
      }

      /* What is the meaning of this pszPrincipal parameter anyway? */
      /* The SSPI documentation and the reality are worlds apart... */
   }

   SecStatus = (fp_SSPI_krb5->AcquireCredentialsHandle)(
	 /* SEC_CHAR   * pszPrincipal     */	    pszPrincipal,
	 /* SEC_CHAR   * pszPackage       */	    krb5_provider,
	 /* ULONG        fCredentialUse   */	    CredentialUse,
	 /* PLUID        pvLogonID        */	    NULL,
	 /* PVOID        pAuthData        */	    pAuthIdentity,
	 /* PVOID        pGetKeyFn        */	    NULL,
	 /* PVOID        pvGetKeyArgument */	    NULL,
	 /* PCredHandle  phCredential     */	    &(pcred->sspi_cred),
	 /* PTimeStamp   ptsExpiry        */	    &(pcred->expiration) );


   if ( SecStatus!=SEC_E_OK ) {
      char             *symbol, *desc;

      if ( pAuthIdentity!=NULL ) {
	  pszPrincipal = "";
	  /* (07-Sep-2004) Martin Rex                                           */
	  /* Severe BUG in WindowsXP ServicePack2: after a screen-lock/unlock   */
	  /* explicitly named credentials always receive SSPI-error 0x80009030E */
	  /* and when using a W2K-level domain AcceptSecurityContext() fails    */
	  /* always with 0x80009030E (NO_CREDENTIALS).  Luckily this workaround */
	  /* here will cover the initiating-credentials case (simple client)    */
	  /*                                                                    */
	  /* (23-Oct-2003) Martin Rex                                           */
	  /* Braindamage in Kerberos SSP: when using SmartCard-based preauth    */
	  /* with the Kerberos SSP then AcquireCredentialsHandle() with an      */
	  /* explicit pAuthIdentity containing domain and user will not be      */
	  /* correctly matched with the default credentials                     */
	  /* QueryCredentialsAttributes(NAMES) will work correctly, however,    */
	  /* so we'll catch a mismatch with the existing code later on          */
	  SecStatus = (fp_SSPI_krb5->AcquireCredentialsHandle)(
	      /* SEC_CHAR   * pszPrincipal     */	    pszPrincipal,
	      /* SEC_CHAR   * pszPackage       */	    krb5_provider,
	      /* ULONG        fCredentialUse   */	    CredentialUse,
	      /* PLUID        pvLogonID        */	    NULL,
	      /* PVOID        pAuthData        */	    NULL,
	      /* PVOID        pGetKeyFn        */	    NULL,
	      /* PVOID        pvGetKeyArgument */	    NULL,
	      /* PCredHandle  phCredential     */	    &(pcred->sspi_cred),
	      /* PTimeStamp   ptsExpiry        */	    &(pcred->expiration) );
      }

      if ( SecStatus!=SEC_E_OK ) {
	  krb5_sec_error( SecStatus, &symbol, &desc );


	  DEBUG_STRANGE((tf, "AcquireCredentialsHandle(\"%s\") failed with %s\n(Desc=\"%s\")\n",
	                     pszPrincipal, symbol, desc));
	  if ( SECPKG_CRED_OUTBOUND==CredentialUse ) {
	      ERROR_RETURN( KRB5_MINOR_SSPI(AcquireCredentialsHandleOUT,SecStatus), GSS_S_FAILURE );
	  }
	  ERROR_RETURN( KRB5_MINOR_SSPI(AcquireCredentialsHandleIN,SecStatus), GSS_S_FAILURE );
      }
   }

   /* BUGBUG: Kerberos SSP returns a near-infinite value instead of INFINITE */
   /* According to Microsoft, Kerberos credentials in Windows 2000 do not    */
   /* expire, i.e. they cache the password and request new TGTs whenever     */
   /* that is necessary							     */
   pcred->expiration.LowPart    = 0xffffffffu;
   pcred->expiration.HighPart   = 0x7fffffffu;

   pcred->valid_from      = time(NULL);
   pcred->expires_at      = krb5_timestamp2time( &(pcred->expiration) );

   if ( pp_expires_at!=NULL ) {
      (*pp_expires_at) = pcred->expires_at;
   }

   maj_stat = krb5_determine_cred_owner( pp_min_stat, &(pcred->sspi_cred),
					 (Uchar **) &(pcred->name), &(pcred->name_len) );
   if ( GSS_S_COMPLETE!=maj_stat )
       goto error;

   if ( principal!=NULL ) {
      /* credentials were requested for an explicit name    */
      /* verify if the credentials are those that we expect */
      /* -- never trust SSPI!                               */
      if ( pcred->name_len!=principal_len  ||  strcmp(pcred->name, principal) ) {
         sy_clear_free( (void **) &pcred->name, pcred->name_len );
	 pcred->name_len = 0;
         ERROR_RETURN( KRB5_MINOR(SSPI_BAD_CRED_NAME), GSS_S_NO_CRED );
      }
   }

   /* Do a *REAL* verification of the credential that SSPI returned     */
   /* by performing a context establishment with ourselves              */
   /* This has a noticable performance penalty of a few milliseconds,   */
   /* however I prefer determining the true validity of the credentials */
   /* right here and not just during context establishment		*/
   /*                                                                   */
   if ( CredentialUse==SECPKG_CRED_INBOUND ) {
      if ( FALSE!=krb5_acred_nocheck ) {
         maj_stat = krb5_verify_acc_cred( pp_min_stat, pcred, &cred_ok );
         if ( GSS_S_COMPLETE!=maj_stat || FALSE==cred_ok ) {
            DWORD dwValue;
            if ( KRB5_HAVE_TWEAK==krb5_query_registry_tweak("ForceAccCredOK", &dwValue) ) {
               if ( dwValue==1 ) {
                  maj_stat = GSS_S_COMPLETE;
                  cred_ok  = TRUE;
               }
            }
         }
      }
   } else {
      if ( FALSE!=krb5_icred_nocheck ) {
         maj_stat = krb5_verify_ini_cred( pp_min_stat, pcred, &cred_ok );
         if ( GSS_S_COMPLETE!=maj_stat || FALSE==cred_ok ) {
            DWORD dwValue;
            if ( KRB5_HAVE_TWEAK==krb5_query_registry_tweak("ForceIniCredOK", &dwValue) ) {
               if ( dwValue==1 ) {
                  maj_stat = GSS_S_COMPLETE;
                  cred_ok  = TRUE;
               }
            }
         }
      }
   }

   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   if ( cred_ok==FALSE )
      ERROR_RETURN( KRB5_MINOR(SSPI_BAD_CRED_NAME), GSS_S_NO_CRED );

   if (maj_stat!=GSS_S_COMPLETE) {
error:
      if ( pcred!=NULL ) {
         OM_uint32  min_stat;

	 (void) krb5_release_cred( &min_stat, &pcred );
	 pcred = NULL;
      }
   }

   (*pp_cred) = pcred;

   /* release the temporary NUL-terminated copies of the name */
   sy_clear_free((void **) &principal, principal_len );
   sy_clear_free((void **) &user,      user_len      );
   sy_clear_free((void **) &realm,     realm_len     );

   return(maj_stat);

} /* krb5_acquire_cred() */




/*********************************************************************
 * krb5_release_cred()
 *
 *********************************************************************/
OM_uint32
krb5_release_cred( OM_uint32       * pp_min_stat,
		   void           ** pp_cred
     )
{
   char            * this_Call = "krb5_release_cred";
   krb5_cred_desc  * pcred     = NULL;
   OM_uint32         maj_stat  = GSS_S_COMPLETE;
   SECURITY_STATUS   SecStatus = SEC_E_OK;

   (*pp_min_stat) = 0;

   maj_stat = krb5_check_cred( pp_min_stat, pp_cred, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   pcred = (krb5_cred_desc *) (*pp_cred);
   if ( IsValidSecurityHandle( &(pcred->sspi_cred) ) ) {
      SecStatus = (fp_SSPI_krb5->FreeCredentialHandle)( &(pcred->sspi_cred) );

      if ( SecStatus!=SEC_E_OK ) {
	 char *symbol, *desc;
      
	 krb5_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_STRANGE((tf, "FreeCredentialsHandle() failed with %s\n(Desc=\"%s\")\n",
	 		    symbol, desc));

	 (*pp_min_stat) = KRB5_MINOR_SSPI(FreeCredentialsHandle,SecStatus);
	 maj_stat       = GSS_S_FAILURE;
      }
   }

   sy_clear_free( (void **) &(pcred->name), pcred->name_len );   /* If we kept one around, it must be released */
   sy_clear_free(           pp_cred,        sizeof(*pcred) );    /* clear and release			       */
								 /*   the whole private credentials struct     */
   return(maj_stat);

} /* krb5_release_cred() */



/*********************************************************************
 *  krb5_inquire_cred()
 *
 *  Description:
 *    Query/inquire attributes of the private credentials.
 *
 *  Parameters:
 *    pp_min_stat    OUT      Minor status code
 *    p_cred         IN       private credential handle
 *       --all following parameters are optional
 *       --the next 3 Parameters are interdependent and must be
 *       --all present or all absent
 *    pp_name        OUT      Pointer (ro) to (internal) name of owner
 *    pp_name_len    OUT      length of (internal) name of owner
 *    pp_nt_tag      OUT      nt_tag for name
 *    pp_usage       OUT      usage of the credentials
 *    pp_expires_at  OUT      expiration time of the credentials
 *********************************************************************/
OM_uint32
krb5_inquire_cred( OM_uint32         * pp_min_stat,
		   void              * p_cred,
		   Uchar            ** pp_name,
		   size_t            * pp_name_len,
		   gn_nt_tag_et      * pp_nt_tag,
		   gn_cred_usage_et  * pp_usage,
		   time_t            * pp_expires_at
     )
{
   char              * this_Call = "krb5_inquire_cred";
   krb5_cred_desc    * prvcred;
   OM_uint32           maj_stat;

   (*pp_min_stat) = 0;

   if ( pp_name!=NULL     )  { (*pp_name)     = NULL;           }
   if ( pp_name_len!=NULL )  { (*pp_name_len) = 0;              }
   if ( pp_nt_tag!=NULL   )  { (*pp_nt_tag)   = NT_INVALID_TAG; }

   maj_stat = krb5_check_cred( pp_min_stat, &p_cred, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   prvcred = p_cred;
   if ( pp_usage!=NULL )      { (*pp_usage)      = prvcred->usage;      }

   /* Kerberos credentials from Kerberos SSP don't actually expire ... */
   if ( pp_expires_at!=NULL ) { (*pp_expires_at) = prvcred->expires_at; }

   if ( pp_name!=NULL && pp_name_len!=NULL && pp_nt_tag!=NULL ) {
      (*pp_name)     = prvcred->name;
      (*pp_name_len) = prvcred->name_len;
      (*pp_nt_tag)   = krb5_nt_tag;
   } else if ( pp_name!=NULL || pp_name_len!=NULL || pp_nt_tag==NULL ) {
      DEBUG_ERR((tf, "Internal ERROR: %s(): invalid name parameters\n", this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   return(GSS_S_COMPLETE);  

} /* krb5_inquire_cred() */



/*
 * krb5_default_name_from_ini_cred()
 *
 */
OM_uint32
krb5_default_name_from_ini_cred( OM_uint32  * pp_min_stat,
			         char       * buf,
			         size_t       buf_max )
{
   SECURITY_STATUS          SecStatus;
   CredHandle		    Cred;
   TimeStamp                Expiry;
   char                   * tmpbuf     = NULL;   /* dynamic, use sy_free() */
   size_t                   tmpbuf_len = 0;
   OM_uint32		    maj_stat = GSS_S_COMPLETE;

   SecInvalidateHandle(&Cred);

   buf[0] = 0;

   SecStatus = (fp_SSPI_krb5->AcquireCredentialsHandle)(
	 /* SEC_CHAR   * pszPrincipal     */	    "",
	 /* SEC_CHAR   * pszPackage       */	    krb5_provider,
	 /* ULONG        fCredentialUse   */	    SECPKG_CRED_OUTBOUND,
	 /* PLUID        pvLogonID        */	    NULL,
	 /* PVOID        pAuthData        */	    NULL,
	 /* PVOID        pGetKeyFn        */	    NULL,
	 /* PVOID        pvGetKeyArgument */	    NULL,
	 /* PCredHandle  phCredential     */	   &Cred,
	 /* PTimeStamp   ptsExpiry        */	   &Expiry );

   if (SecStatus!=SEC_E_OK) {

      char *symbol, *desc;

      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_STRANGE((tf, "AcquireCredentialsHandle(OUTBOUND,default) failed with %s\n(Desc=\"%s\")\n",
			      symbol, desc));

      ERROR_RETURN( KRB5_MINOR_SSPI(AcquireCredentialsHandleOUT,SecStatus), GSS_S_FAILURE );

   } else {

      maj_stat = krb5_determine_cred_owner( pp_min_stat, &Cred, 
					    &tmpbuf, &tmpbuf_len );
      if ( GSS_S_COMPLETE!=maj_stat )
	  goto error;

      if ( tmpbuf_len>=buf_max )
	  ERROR_RETURN( MINOR_NAME_TOO_LONG, GSS_S_FAILURE );

      if ( tmpbuf_len>0 )
	  memcpy(buf,tmpbuf,tmpbuf_len);

      buf[tmpbuf_len] = 0;

   }

error:
   if ( IsValidSecurityHandle( &Cred ) ) {
      SecStatus = (fp_SSPI_krb5->FreeCredentialHandle)( &Cred );
      SecInvalidateHandle(&Cred);
   }

   sy_clear_free( &tmpbuf, tmpbuf_len );

   return(maj_stat);

} /* krb5_default_name_from_ini_cred() */



/*********************************************************************
 * krb5_verify_ini_cred()
 *
 * Try to verify that an SSPI OUTBOUND credential is valid for
 * initiating a security context.  We'll try to establish a security
 * context to ourselves.  That should always be possible with
 * Microsoft's Kerberos SSPI.
 *
 * The call to InitializeSecurityContext() may be slightly expensive,
 * but I don't know any other reliable check so far.  Looking into
 * the ticket cache is not reliable, since SSPI Kerberos will
 * automatically acquire a TGT when we try context establishment.
 *
 * Since the ticket to ourselves ends up in the ticket cache,
 * the overhead for successive calls should be much less than
 * for the initial call.
 *********************************************************************/
OM_uint32
krb5_verify_ini_cred( OM_uint32	       * pp_min_stat,
		      krb5_cred_desc   * p_cred,
		      int	       * pp_cred_ok
   )
{
   char               * this_Call = "krb5_verify_ini_cred";
   SECURITY_STATUS      SecStatus;
   CtxtHandle	      * pNoCtxt     = NULL;
   CtxtHandle		IniCtxt;		  /* temporary dynamic object, must release */
   gss_buffer_desc      ctx_token;
   SecBufferDesc	OutBufDesc;
   SecBuffer		OutSecToken;	  /* temporary dynamic object, must release */
   ULONG		CtxFlagsReq = 0;
   ULONG		CtxFlagsRec = 0;
   TimeStamp		Expiry;
   OM_uint32		maj_stat  = GSS_S_COMPLETE;

   (*pp_min_stat) = MINOR_NO_ERROR;
   (*pp_cred_ok)  = FALSE;

   if ( (krb5_capabilities&SECPKG_FLAG_CLIENT_ONLY)!=0 ) {
      /* Don't know how to verify the cred if we can *NOT* talk to ourselves */
      (*pp_cred_ok) = TRUE;
      return(maj_stat);
   }

   CtxFlagsReq = krb5_gss_svc_to_ISC_REQ(GSS_C_MUTUAL_FLAG|GSS_C_REPLAY_FLAG);
   /* Windows BUG workaround (W2K3 AD event log ID 673)                   */
   /* the flag ISC_REQ_USE_SESSION_KEY will request Kerberos User-2-User  */
   /* authentication, and it appears to work on everything from W2K-gold. */
   /* On W2K3 machines User2User is automatically done for all targets    */
   /* that do not have a service principal name defined in the active     */
   /* directory (when the AD is running in Windows 2003 native mode)      */
   CtxFlagsReq |= ISC_REQ_USE_SESSION_KEY;
   SecInvalidateHandle( &IniCtxt );

   OutSecToken.BufferType = SECBUFFER_TOKEN;
   OutSecToken.cbBuffer   = 0;
   OutSecToken.pvBuffer   = NULL;

   OutBufDesc.cBuffers    = 1;
   OutBufDesc.pBuffers    = &OutSecToken;
   OutBufDesc.ulVersion   = SECBUFFER_VERSION;

   SecStatus = (fp_SSPI_krb5
	    ->InitializeSecurityContext)(
     /* PCredHandle    phCredential  */   &(p_cred->sspi_cred),
     /* PCtxtHandle    phContext     */   pNoCtxt,
     /* SEC_CHAR     * pszTargetName */	  (p_cred->name),
     /* ULONG          fContextReq   */	  CtxFlagsReq,
     /* ULONG          Reserved1     */   0,
     /* ULONG	       TargetDataRep */	  SECURITY_NATIVE_DREP,
     /* PSecBufferDesc pInput        */	  NULL,
     /* ULONG	       Reserved2     */   0,
     /* PCtxtHandle    phNewContext  */	  &IniCtxt,
     /* PSecBufferDesc pOutput       */	  &OutBufDesc,
     /* PULONG         pfContextAttr */	  &CtxFlagsRec,
     /* PTimeStamp     ptsExpiry     */	  &Expiry );

   if ( SecStatus==SEC_I_CONTINUE_NEEDED || SecStatus==SEC_E_OK ) {
      if ( OutSecToken.pvBuffer!=NULL && OutSecToken.cbBuffer>0 ) {
	 gss_OID_desc      token_oid;
	 void		 * inner_token;
	 size_t		   inner_token_len;
	 gn_mech_tag_et	   mech_tag;

	 ctx_token.value  = OutSecToken.pvBuffer;
	 ctx_token.length = OutSecToken.cbBuffer;
	 maj_stat = gn_parse_gss_token( pp_min_stat, &ctx_token, &token_oid,
				        &mech_tag, &inner_token, &inner_token_len );
	 if ( maj_stat!=GSS_S_COMPLETE ) {

	    if ( maj_stat==GSS_S_BAD_MECH
		&& token_oid.length==krb5_w2k3_u2u_oid->length
		&& 0==memcmp(token_oid.elements, krb5_w2k3_u2u_oid->elements, token_oid.length) ) {

		/* For client credentials we do not require a W2K service principal (HERE) */
		/* we'll catch problems during gss_init_sec_context() then                 */
		(*pp_cred_ok) = TRUE;
		maj_stat      = GSS_S_COMPLETE;

		DEBUG_ACTION((tf, "  A: %s(): InitializeSecurityContext(target=\"%s\") returned W2K3 U2U-token\n",
				  this_Call,
				  (p_cred->name==NULL) ? "(NULL)" : p_cred->name ));

	    } else {

		DEBUG_ERR((tf, "  E: %s(): InitializeSecurityContext(target=\"%.256s\") returned a non-Kerberos context!?\n",
				this_Call,
				(p_cred->name==NULL) ? "(NULL)" : p_cred->name ));
		ERROR_RETURN(KRB5_MINOR(SSPI_BAD_CRED), GSS_S_NO_CRED);

	    }

         } else {

	     if ( mech_tag==krb5_mech_tag ) {
		/* We should only be able to get here, if SSPI managed to acquire */
		/* a service ticket to ourselves using the initiating credentials */
		/* that we wanted to verify                                       */
		(*pp_cred_ok) = TRUE;

	     } else {

		DEBUG_STRANGE((tf, "  S: %s(): how did we get here?", this_Call));

		ERROR_RETURN(KRB5_MINOR(SSPI_BAD_CRED), GSS_S_NO_CRED);

	     }
	 }
      }

   } else {
      char *symbol, *desc;
      
      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "  E: %s(): InitializeSecurityContext(target=\"%.256s\") failed with %s\n(Desc=\"%s\")\n",
			this_Call,
			(p_cred->name==NULL) ? "(NULL)" : p_cred->name,
			symbol, desc ));
      ERROR_RETURN(KRB5_MINOR_SSPI(InitializeSecurityContext1,SecStatus), GSS_S_NO_CRED);
   }

error:
   if ( OutSecToken.pvBuffer!=NULL && OutSecToken.cbBuffer>0 ) {
      SecStatus = (fp_SSPI_krb5->FreeContextBuffer)( OutSecToken.pvBuffer );
      OutSecToken.pvBuffer = NULL;
      OutSecToken.cbBuffer = 0;
   }

   if ( IsValidSecurityHandle( &IniCtxt ) ) {
      SecStatus = (fp_SSPI_krb5->DeleteSecurityContext)( &IniCtxt );
      SecInvalidateHandle( &IniCtxt );
   }

   return(maj_stat);

} /* krb5_verify_ini_cred() */




/*********************************************************************
 * krb5_verify_acc_cred()
 *
 * Try to verify that an SSPI INBOUND credential is valid for
 * accepting a security context.  We'll try to establish a security
 * context to ourselves.  That should always be possible with
 * Microsoft's Kerberos SSPI.
 *
 * The call to InitializeSecurityContext() & AcceptSecurityContext()
 * may be slightly expensive, but I don't know any other reliable
 * check so far.
 *
 * Since the ticket to ourselves ends up in the ticket cache,
 * the overhead for successive calls should be much less than
 * for the initial call.
 *********************************************************************/
OM_uint32
krb5_verify_acc_cred( OM_uint32	       * pp_min_stat,
		      krb5_cred_desc   * p_cred,
		      int	       * pp_cred_ok
   )
{
   char               * this_Call = "krb5_verify_acc_cred";
   SECURITY_STATUS      SecStatus;
   CtxtHandle	      * pNoCtxt     = NULL;
   CtxtHandle		IniCtxt;	  /* temporary dynamic object, must release */
   CtxtHandle		AccCtxt;	  /* temporary dynamic object, must release */
   CredHandle		IniCred;	  /* temporary dynamic object, must release */
   gss_buffer_desc      ctx_token;
   SecBufferDesc	IniOutBufDesc;
   SecBuffer		IniOutSecToken;	  /* temporary dynamic object, must release */
   SecBufferDesc	AccOutBufDesc;
   SecBuffer		AccOutSecToken;	  /* temporary dynamic object, must release */
   ULONG		IniCtxFlagsReq = 0;
   ULONG		IniCtxFlagsRec = 0;
   ULONG		AccCtxFlagsReq = 0;
   ULONG		AccCtxFlagsRec = 0;
   TimeStamp		IniExpiry;
   TimeStamp		AccExpiry;
   OM_uint32		maj_stat  = GSS_S_COMPLETE;

   (*pp_min_stat) = MINOR_NO_ERROR;
   (*pp_cred_ok)  = FALSE;

   if ( (krb5_capabilities&SECPKG_FLAG_CLIENT_ONLY)!=0 ) {
      /* Don't know how to verify the cred if we can *NOT* talk to ourselves */
      (*pp_cred_ok) = TRUE;
      return(maj_stat);
   }

   IniCtxFlagsReq = krb5_gss_svc_to_ISC_REQ(GSS_C_REPLAY_FLAG); /* Target-Only authentication !! */
   AccCtxFlagsReq = krb5_gss_svc_to_ASC_REQ();
   SecInvalidateHandle( &IniCtxt );
   SecInvalidateHandle( &AccCtxt );
   SecInvalidateHandle( &IniCred );

   IniOutSecToken.BufferType  = SECBUFFER_TOKEN;
   IniOutSecToken.cbBuffer    = 0;
   IniOutSecToken.pvBuffer    = NULL;

   AccOutSecToken.BufferType  = SECBUFFER_TOKEN;
   AccOutSecToken.cbBuffer    = 0;
   AccOutSecToken.pvBuffer    = NULL;

   IniOutBufDesc.cBuffers     = 1;
   IniOutBufDesc.pBuffers     = &IniOutSecToken;
   IniOutBufDesc.ulVersion    = SECBUFFER_VERSION;

   AccOutBufDesc.cBuffers     = 1;
   AccOutBufDesc.pBuffers     = &AccOutSecToken;
   AccOutBufDesc.ulVersion    = SECBUFFER_VERSION;

   SecStatus = (fp_SSPI_krb5->AcquireCredentialsHandle)(
      /* SEC_CHAR   * pszPrincipal     */     "",	  /* default cred */
      /* SEC_CHAR   * pszPackage       */     krb5_provider,
      /* ULONG        fCredentialUse   */     SECPKG_CRED_OUTBOUND,
      /* PLUID        pvLogonID        */     NULL,
      /* PVOID        pAuthData        */     NULL, /* default cred */
      /* PVOID        pGetKeyFn        */     NULL,
      /* PVOID        pvGetKeyArgument */     NULL,
      /* PCredHandle  phCredential     */    &IniCred,
      /* PTimeStamp   ptsExpiry        */    &IniExpiry );

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      krb5_sec_error( SecStatus, &symbol, &desc );
      DEBUG_ERR((tf, "  E: %s(): AcquireCredentialsHandle(OUTBOUND,default) failed with %s\n(Desc=\"%s\")\n",
			this_Call, symbol, desc ))
      ERROR_RETURN(KRB5_MINOR_SSPI(AcquireCredentialsHandleOUT,SecStatus), GSS_S_NO_CRED);

   } else {

      SecStatus = (fp_SSPI_krb5
	       ->InitializeSecurityContext)(
	 /* PCredHandle    phCredential  */   &IniCred,
	 /* PCtxtHandle    phContext     */    pNoCtxt,
	 /* SEC_CHAR     * pszTargetName */    (p_cred->name),
	 /* ULONG          fContextReq   */    IniCtxFlagsReq,
	 /* ULONG          Reserved1     */    0,
	 /* ULONG	   TargetDataRep */    SECURITY_NATIVE_DREP,
	 /* PSecBufferDesc pInput        */    NULL,
	 /* ULONG	   Reserved2     */    0,
	 /* PCtxtHandle    phNewContext  */   &IniCtxt,
	 /* PSecBufferDesc pOutput       */   &IniOutBufDesc,
	 /* PULONG         pfContextAttr */   &IniCtxFlagsRec,
	 /* PTimeStamp     ptsExpiry     */   &IniExpiry );

      if ( SecStatus==SEC_I_CONTINUE_NEEDED || SecStatus==SEC_E_OK ) {

	 if ( IniOutSecToken.pvBuffer!=NULL && IniOutSecToken.cbBuffer>0 ) {
	    gss_OID_desc	   token_oid;
	    void		 * inner_token;
	    size_t		   inner_token_len;
	    gn_mech_tag_et	   mech_tag;

	    ctx_token.value  = IniOutSecToken.pvBuffer;
	    ctx_token.length = IniOutSecToken.cbBuffer;
	    maj_stat = gn_parse_gss_token( pp_min_stat, &ctx_token, &token_oid, &mech_tag,
					   &inner_token, &inner_token_len );

	    if ( maj_stat!=GSS_S_COMPLETE ) {

		if ( maj_stat==GSS_S_BAD_MECH
		    && token_oid.length==krb5_w2k3_u2u_oid->length
		    && 0==memcmp(token_oid.elements, krb5_w2k3_u2u_oid->elements, token_oid.length) ) {

		    DEBUG_ERR((tf, "  E: %s(): InitializeSecurityContext(target=\"%.256s\") returned W2K3 U2U-token!\n",
				    this_Call,
				    (p_cred->name==NULL) ? "(NULL)" : p_cred->name ));
		    ERROR_RETURN(KRB5_MINOR(W2K3_U2U_OWN), GSS_S_NO_CRED);
		}

		DEBUG_ERR((tf, "  E: %s(): InitializeSecurityContext(target=\"%.256s\") returned a non-Kerberos Context token!?\n",
				this_Call,
				(p_cred->name==NULL) ? "(NULL)" : p_cred->name ));
		ERROR_RETURN(KRB5_MINOR(SSPI_BAD_CRED), GSS_S_NO_CRED);

	    } else {

		if ( mech_tag==krb5_mech_tag ) {

		   SecStatus = (fp_SSPI_krb5
		      ->AcceptSecurityContext)(
		   /* PCredHandle    phCredential  */   &(p_cred->sspi_cred),
		   /* PCtxtHandle    phContext     */    pNoCtxt,
		   /* PSecBufferDesc pInput        */   &IniOutBufDesc,
		   /* ULONG          fContextReq   */    AccCtxFlagsReq,
		   /* ULONG          TargetDataRep */    SECURITY_NATIVE_DREP,
		   /* PCtxtHandle    phNewCount    */   &AccCtxt,
		   /* PSecBufferDesc pOutput       */   &AccOutBufDesc,
		   /* PULONG         pfContextAttr */   &AccCtxFlagsRec,
		   /* PTimeStamp     ptsExpiry     */   &AccExpiry );

		   if ( SecStatus==SEC_E_OK ) {
		       /* We requested a target-only authentication, so we can stop here */
		       (*pp_cred_ok) = TRUE;
		   } else {
		       char *symbol, *desc;
      
		       krb5_sec_error( SecStatus, &symbol, &desc );
		       DEBUG_ERR((tf, "  E: %s(): AcceptSecurityContext() failed with %s\n(Desc=\"%s\")\n",
			           	this_Call,
					symbol, desc ))

		       ERROR_RETURN(KRB5_MINOR_SSPI(AcceptSecurityContext1,SecStatus), GSS_S_NO_CRED);
		   }


		} else {

		    DEBUG_STRANGE((tf, "  S: %s(): How did we get here!?\n", this_Call));
		    ERROR_RETURN(KRB5_MINOR(SSPI_BAD_CRED), GSS_S_NO_CRED);
		}

	    }

	 }


      } else {
	 char *symbol, *desc;
      
         krb5_sec_error( SecStatus, &symbol, &desc );
         DEBUG_ERR((tf, "  E: %s(): InitializeSecurityContext(target=\"%.256s\") failed with %s\n(Desc=\"%s\")\n",
			this_Call,
			(p_cred->name==NULL) ? "(NULL)" : p_cred->name,
			symbol, desc ))
	 ERROR_RETURN(KRB5_MINOR_SSPI(InitializeSecurityContext1,SecStatus), GSS_S_NO_CRED);
      }

   }

error:
   if ( IniOutSecToken.pvBuffer!=NULL && IniOutSecToken.cbBuffer>0 ) {
      SecStatus = (fp_SSPI_krb5->FreeContextBuffer)( IniOutSecToken.pvBuffer );
      IniOutSecToken.pvBuffer = NULL;
      IniOutSecToken.cbBuffer = 0;
   }

   if ( AccOutSecToken.pvBuffer!=NULL && AccOutSecToken.cbBuffer>0 ) {
      SecStatus = (fp_SSPI_krb5->FreeContextBuffer)( AccOutSecToken.pvBuffer );
      AccOutSecToken.pvBuffer = NULL;
      AccOutSecToken.cbBuffer = 0;
   }

   if ( IsValidSecurityHandle( &IniCtxt ) ) {
      SecStatus = (fp_SSPI_krb5->DeleteSecurityContext)( &IniCtxt );
      SecInvalidateHandle( &IniCtxt );
   }

   if ( IsValidSecurityHandle( &AccCtxt ) ) {
      SecStatus = (fp_SSPI_krb5->DeleteSecurityContext)( &AccCtxt );
      SecInvalidateHandle( &AccCtxt );
   }

   if ( IsValidSecurityHandle( &IniCred ) ) {
      SecStatus = (fp_SSPI_krb5->FreeCredentialHandle)( &IniCred );
      SecInvalidateHandle( &IniCred );
   }

   return(maj_stat);

} /* krb5_verify_acc_cred() */



/*
 * krb5_determine_cred_owner()
 *
 * 
 *
 */
OM_uint32
krb5_determine_cred_owner( OM_uint32    * pp_min_stat,
			   CredHandle   * p_Cred,
			   char        ** pp_buf,
			   size_t       * pp_buf_len )
{
   SECURITY_STATUS          SecStatus;
   SecPkgCredentials_Names  CredName;
   SecPkgCredentials_NamesW CredName_W;
   size_t		    len = 0;
   char                   * ptr;
   char                     namebuf[256];
   DWORD                    namebuf_len;
   OM_uint32		    maj_stat = GSS_S_COMPLETE;

   CredName.sUserName   = NULL;
   CredName_W.sUserName = NULL;

   (*pp_buf)     = NULL;
   (*pp_buf_len) = 0;

/*
 * Now query the actual credentials owner from the SSPI credentials
 * via QueryCredentialsAttributes(SECPKG_CRED_ATTR_NAMES)
 * 
 * BUGBUG Alert: (16-Aug-2004) Martin Rex
 *  QueryCredentialsAttributesA(NAMES) leaks memory on Win2Ksp4 and WinXPsp1
 *  the leak size is around 4 KByte virtual and 40 Bytes non-paged pool memory
 *
 * Microsoft will issue a patch, but there's a workaround: we call
 * QueryCredentialsAttributesW(NAMES) and convert to ANSI ourselves.
 */

   if ( NULL!=fp_SSPI_krb5_W ) {

      SecStatus = (fp_SSPI_krb5_W->QueryCredentialsAttributesW)(
					     p_Cred,
					     SECPKG_CRED_ATTR_NAMES,
					    &CredName_W );
      if ( SecStatus!=SEC_E_OK )
         goto sspi_error;

      len = WideCharToMultiByte(CP_ACP,0,CredName_W.sUserName,-1,NULL, 0, NULL, NULL);
      if ( 0==len || len >= KRB5_MAX_NAMELEN) {
	 ERROR_RETURN( MINOR_INVALID_USERNAME, GSS_S_FAILURE );
      }
      (*pp_buf) = sy_malloc( len + 1 );
      if ( NULL==(*pp_buf) ) {
	 ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
      }
      len = (size_t)WideCharToMultiByte(CP_ACP,0,CredName_W.sUserName,-1, (*pp_buf), (int)len, NULL, NULL);
      if (len==0) {
	 ERROR_RETURN( MINOR_INVALID_USERNAME, GSS_S_FAILURE );
      }
      len--; /* WideCharToMultiByte() counts the terminating NUL char */

   } else {

      SecStatus = (fp_SSPI_krb5->QueryCredentialsAttributes)(
					     p_Cred,
					     SECPKG_CRED_ATTR_NAMES,
					    &CredName );
      if ( SecStatus!=SEC_E_OK ) {
         char *symbol, *desc;
sspi_error:      
         if ( SEC_E_INVALID_HANDLE==SecStatus && NULL!=p_Cred) 
         {
            /* In some environments under yet undetermined conditions  */
            /* but involving Passthru Authentication (aka Delegation)  */
            /* QueryCredentialsAttributes(NAMES) fails with 0x80090301 */
            /* = SEC_E_INVALID_HANDLE = "Specified handle is invalid"  */
            /* which is undoubtedly a bug in SSPI.                     */
            /* As a workaround, we will make a bold assumption here    */
            /* the the Kerberos ticket identifies the current user     */
            /* and concoct a principal name from other Win32 calls     */
            DWORD                 wkstu_len;
            NET_API_STATUS        nStatus;
            LPWKSTA_USER_INFO_0   pBuf = NULL;
            char                  wkstu_buf[MAX_USERNAME_LEN];
            char                  getu_buf[MAX_USERNAME_LEN];
            char                * realm = NULL; /* convenience ptr */
            size_t                getu_len  = 0;
            size_t                realm_len = 0;
         

            len          = 0;
            namebuf_len  = sizeof(namebuf)-1;
            wkstu_len    = 0;
            wkstu_buf[0] = 0;
            /* in theory, NetWkstaUserGetInfo(level0) returns the canonical */
            /* representation of the account name MUCH more reliably than   */
            /* the broken GetUserName().  But it FAILS under LocalSystem    */
            /* and I don't know whether it works correctly during impersonation */
            /* so we need another emergency fallback to the broken GetUserName()*/
            nStatus = NetWkstaUserGetInfo(NULL, 0, (LPBYTE*) &pBuf);
            if ( NERR_Success==nStatus ) {
               wkstu_len = WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)(pBuf->wkui0_username), -1,
                                                    &(wkstu_buf[0]), sizeof(wkstu_buf)-1, 0, 0 );
               if ( wkstu_len>0 ) {
                  wkstu_buf[wkstu_len]       = 0;
               }
            }
            if ( NULL!=pBuf ) {
               NetApiBufferFree( pBuf );
               pBuf = NULL;
            }
            if ( FALSE!=(fp_GetUserNameEx)( NameUserPrincipal, namebuf, &namebuf_len ) ) {
               namebuf[namebuf_len] = 0;
               realm = strchr(namebuf, '@');
               if ( NULL!=realm ) {
                  DWORD lenpar = sizeof(getu_buf)-1;
                  CharUpper(realm);  /* case-correct Kerberos Realm name */
                  realm_len = strlen(realm);

                  if ( FALSE!=GetUserName(getu_buf, &lenpar ) ) {
                     getu_buf[lenpar] = 0;
                     getu_len = strlen(getu_buf); /* recount! GetUserName() may include trailing 0) */
                     len = getu_len + realm_len;
                     if ( len>=KRB5_MAX_NAMELEN )
	                ERROR_RETURN( MINOR_NAME_TOO_LONG, GSS_S_FAILURE );
                     (*pp_buf) = sy_malloc( len+2 ); /* account for trailing 0 */
                     if ( NULL==(*pp_buf) )
	                ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
                     if ( getu_len>0 ) {
                        /* warning: wkstu_len from above includes a trailing 0 in its count      */
                        /* whereas getu_len was recomputed with strlen() and does not include it */
                        if ( wkstu_len>0 && 0==_stricmp(wkstu_buf, getu_buf) ) {
                           memcpy((*pp_buf), wkstu_buf, wkstu_len);
                        } else {
                           memcpy((*pp_buf), getu_buf, getu_len);
                        }
                     }
                     memcpy((*pp_buf)+getu_len, realm, realm_len+1); /* copy trailing 0 inclusive */
                     goto continue_after_workaround;
                  }
               }
            }

         } /* end of inserted block */

         krb5_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_ERR((tf, "QueryCredentialsAttributes(NAMES) failed with %s\n(Desc=\"%s\")\n",
			symbol, desc));

	 ERROR_RETURN( KRB5_MINOR_SSPI(QueryCredentialsAttributesNAMES,SecStatus), GSS_S_FAILURE );
      }

      len = strlen(CredName.sUserName);
      if ( len>=KRB5_MAX_NAMELEN )
	 ERROR_RETURN( MINOR_NAME_TOO_LONG, GSS_S_FAILURE );

      (*pp_buf) = sy_malloc( len + 1 );
      if ( NULL==(*pp_buf) ) {
	 ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
      }

      if ( len>0 )
	memcpy( (*pp_buf), CredName.sUserName, len );

   }

continue_after_workaround:
   (*pp_buf)[len] = 0;

   /*********************************************************************/
   /* (31-May-08) mrex							*/
   /* WORKAROUND for BUG in at least WinXP sp2/sp3 and W2K3 sp0/sp1/sp2 */
   /* When a process running under LocalSystem queries the name of his  */
   /* own Kerberos credentials, the username parts (hostname followed   */
   /* by a dollar sign) is spelled incorrectly (wrong case, not as it   */
   /* is registered in the Active Directory, and not how it will be     */
   /* authenticated to peers.)                                          */
   /* So if we find a dollar '$' in front of the '@', we'll retrieve    */
   /* the hostname part via GetUserNameEx(NameUserPrincipal), which     */
   /* appears to be correct in this situation.                          */
   /*									*/
   /* Other possible workarounds: retrieve the authenticated name	*/
   /* (native names) when "verifying" the credential.                   */
   /* Querying the LSA ticket cache could work, but it is possible that */
   /* the cache is empty or flushed (this is somewhat quite unlikely    */
   /* localsystem, but there are unresolved caching bugs in LSA...)     */
   /*********************************************************************/
   namebuf_len = sizeof(namebuf)-1;
   ptr = strchr((*pp_buf), '@');

   if ( NULL!=ptr && ptr>(*pp_buf) && '$'==ptr[-1] && NULL!=fp_GetUserNameEx ) {
       if ( FALSE==(fp_GetUserNameEx)( NameUserPrincipal, namebuf, &namebuf_len ) ) {
	    DWORD lasterror = GetLastError();
	    char  *symbol, *desc;
      
	    krb5_sec_error( lasterror, &symbol, &desc );
	    DEBUG_ERR((tf, "GetUserNameEx(NameUserPrincipal() failed with %s\n(Desc=\"%s\")\n",
	 		   symbol, desc));

	   RETURN_MIN_MAJ( KRB5_MINOR_WINERROR( GetUserNameExPRINCIPAL,lasterror ), GSS_S_FAILURE );
       };

      namebuf[namebuf_len] = 0;
      /* BUG: VS.NET 2003 Studio Documentation claims GetUserNameEx() does NOT  */
      /* include the terminating zero in the returned length count when copying */
      /* which is definitely not what WinXPsp2 is doing (it counts the 0)       */
      /* Safety Belts on:  determine the length ourselves...                    */
      namebuf_len = (DWORD)strlen(namebuf);

      if ( (size_t)namebuf_len==len
	   && 0==_strnicmp(namebuf, (*pp_buf), len) ) {
	  /* QueryCredentialsAttributes(NAMES) and GetUserNameEx(NameUserPrincipal)  */
	  /* returned the same name (differing only in case, if at all)              */
	  /*                                                                         */
	  /* Kerberos Principal names are case-sensitive and GetUserNameEx() usually */
	  /* returns the canonical representation of the name as defined in the AD   */
	  /* whereas QueryCredentialsAttributes() returns whatever spelling was      */
	  /* for logon or supplied to AcquireCredentials() through the AuthIdentity  */
	  /* structure                                                               */
	  /* ... therefore we copy the username part from GetUserNameEx()            */
	  memcpy((*pp_buf), namebuf, ptr-(*pp_buf));
	  if ( *(ptr+1)!=0 )
	    /* ensure the entire realm part is all uppercase                         */
	    CharUpper( ptr+1 );
      }
   }

error:

   if ( CredName.sUserName!=NULL ) {
      (fp_SSPI_krb5->FreeContextBuffer)( CredName.sUserName );
      CredName.sUserName = NULL;
   }

   if ( CredName_W.sUserName!=NULL ) {
      (fp_SSPI_krb5->FreeContextBuffer)( CredName_W.sUserName );
      CredName_W.sUserName = NULL;
   }

   (*pp_buf_len) = (NULL==(*pp_buf)) ? 0 : len;

   return(maj_stat);

} /* krb5_determine_cred_owner()*/

  
  
/*
 * krb5_fallback_def_owner()
 *
 * Synthesize the name of the Windows user principal name
 * through alternative means: NetWkstaUserGetInfo + GetUserNameEx
 *
 * In some environments under yet undetermined conditions
 * but involving Passthru Authentication (aka Delegation)
 * QueryCredentialsAttributes(NAMES) fails with 0x80090301
 * = SEC_E_INVALID_HANDLE = "Specified handle is invalid"
 * which is undoubtedly a bug in SSPI.
 * As a workaround, we will make a bold assumption here
 * the the Kerberos ticket identifies the current user
 * and concoct a principal name from other Win32 calls
 *
 * Another buggy scenario: SmartCard preauthentication may
 * return an incorrect name form QueryCredentialsAttribute(NAMES)
 */
OM_uint32
krb5_fallback_def_owner( OM_uint32      * pp_min_stat,
                         char          ** pp_buf,
                         size_t         * pp_buf_len )
{
   DWORD                 wkstu_len;
   NET_API_STATUS        nStatus;
   LPWKSTA_USER_INFO_0   pBuf = NULL;
   char                  wkstu_buf[MAX_USERNAME_LEN];
   char                  getu_buf[MAX_USERNAME_LEN];
   char                * realm = NULL; /* convenience ptr */
   char                  namebuf[256];
   DWORD                 namebuf_len;
   size_t                len       = 0;
   size_t                getu_len  = 0;
   size_t                realm_len = 0;
   OM_uint32             maj_stat  = GSS_S_FAILURE;
         
   (*pp_buf)      = NULL;
   (*pp_buf_len)  = 0;
   (*pp_min_stat) = 0;

   len            = 0;
   namebuf_len    = sizeof(namebuf)-1;
   wkstu_len      = 0;
   wkstu_buf[0]   = 0;

   /* in theory, NetWkstaUserGetInfo(level0) returns the canonical */
   /* representation of the account name MUCH more reliably than   */
   /* the broken GetUserName().  But it FAILS under LocalSystem    */
   /* and I don't know whether it works correctly during impersonation */
   /* so we need another emergency fallback to the broken GetUserName()*/
   nStatus = NetWkstaUserGetInfo(NULL, 0, (LPBYTE*) &pBuf);
   if ( NERR_Success==nStatus ) {
      wkstu_len = WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)(pBuf->wkui0_username), -1,
                                           &(wkstu_buf[0]), sizeof(wkstu_buf)-1, 0, 0 );
      if ( wkstu_len>0 ) {
         wkstu_buf[wkstu_len]       = 0;
      }
   }
   if ( NULL!=pBuf ) {
      NetApiBufferFree( pBuf );
      pBuf = NULL;
   }
   if ( FALSE!=(fp_GetUserNameEx)( NameUserPrincipal, namebuf, &namebuf_len ) ) {
      namebuf[namebuf_len] = 0;
      realm = strchr(namebuf, '@');
      if ( NULL!=realm ) {
         DWORD lenpar = sizeof(getu_buf)-1;
         CharUpper(realm);  /* case-correct Kerberos Realm name */
         realm_len = strlen(realm);

         if ( FALSE!=GetUserName(getu_buf, &lenpar ) ) {
            getu_buf[lenpar] = 0;
            getu_len = strlen(getu_buf); /* recount! GetUserName() may include trailing 0) */
            len = getu_len + realm_len;
            if ( len>=KRB5_MAX_NAMELEN )
	       ERROR_RETURN( MINOR_NAME_TOO_LONG, GSS_S_FAILURE );
            (*pp_buf) = sy_malloc( len+2 ); /* account for trailing 0 */
            if ( NULL==(*pp_buf) )
	       ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
            if ( getu_len>0 ) {
               /* warning: wkstu_len from above includes a trailing 0 in its count      */
               /* whereas getu_len was recomputed with strlen() and does not include it */
               if ( wkstu_len>0 && 0==_stricmp(wkstu_buf, getu_buf) ) {
                  memcpy((*pp_buf), wkstu_buf, wkstu_len);
               } else {
                  memcpy((*pp_buf), getu_buf, getu_len);
               }
            }
            memcpy((*pp_buf)+getu_len, realm, realm_len+1); /* copy trailing 0 inclusive */
            (*pp_buf)[len] = 0;
            (*pp_buf_len)  = len;
            maj_stat = GSS_S_COMPLETE;
         }
      }
   }

error:
   return(maj_stat);

} /* krb5_fallback_def_owner() */
