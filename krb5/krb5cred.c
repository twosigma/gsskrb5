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
