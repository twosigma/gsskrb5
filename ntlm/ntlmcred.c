#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/ntlm/ntlmcred.c#3 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/ntlm/ntlmcred.c#3 $
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


/*********************************************************************
 * ntlm_check_cred()
 *
 *********************************************************************/
OM_uint32		    
ntlm_check_cred( OM_uint32        *  pp_min_stat,
		 void            **  pp_cred,
		 char             *  p_call_name
     )
{
    ntlm_cred_desc  * pcred;

    if ( pp_cred == NULL || (*pp_cred)==NULL ) {
       DEBUG_ERR((tf, "Internal ERROR: %s(): NULL priv_cred handle\n",
		  p_call_name ))
       RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
    }

    pcred = (*pp_cred);
    if ( pcred->magic_cookie != NTLM_CRED_COOKIE ) {
       DEBUG_ERR((tf, "Internal ERROR: %s(): invalid priv_cred (bad cookie)\n",
		  p_call_name ))
       RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
    }

    return(GSS_S_COMPLETE);

} /* ntlm_check_cred() */



/*********************************************************************
 * ntlm_acquire_cred()
 *
 *********************************************************************/
OM_uint32
ntlm_acquire_cred( OM_uint32          * pp_min_stat,    /* out  */
		   Uchar              * p_name,         /* in   */
		   size_t               p_name_len,	/* in   */
		   gn_cred_usage_et     p_usage,	/* in   */
		   void              ** pp_cred,   	/* out  */
		   time_t	      * pp_expires_at   /* out  */
     )
{
   char			   * this_Call = "ntlm_acquire_cred";
   Uchar                   * user;
   Uchar                   * domain;
   int			     fallback;
   size_t                    Wuser_len, Wdomain_len;
   char			     Wdomain[NTLM_MAX_NAMELEN];
   char			     Wuser[NTLM_MAX_NAMELEN];
   size_t                    user_len, domain_len;
   SEC_WINNT_AUTH_IDENTITY   AuthIdentity;
   ntlm_cred_desc	   * pcred;
   ULONG	             CredentialUse;
   SECURITY_STATUS	     SecStatus;
   SecPkgCredentials_Names   sspi_cred_name;
   OM_uint32		     maj_stat  = GSS_S_COMPLETE;

   if ( pp_cred==NULL ) {
      DEBUG_ERR((tf, "Internal ERROR: %s(): NULL pp_cred parameter!\n",
		     this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   (*pp_min_stat)	    = 0;
   (*pp_cred)		    = NULL;
   pcred		    = NULL;   /* dynamic return object, release on error */
   sspi_cred_name.sUserName = NULL;

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

   pcred->magic_cookie    = NTLM_CRED_COOKIE;
   pcred->usage           = p_usage;
   SecInvalidateHandle( &(pcred->sspi_cred) );

   /* Severly broken SSPI (implementation(s)):				         */
   /* QueryCredentialsAttributes() is documented but not implemented for NTLM    */
   /* Acquiring credentials for a printable name using the pszPrincipal field    */
   /* of AcquireCredentialsHandle() returns SEC_E_UNKNOWN_CREDENTIALS, because   */
   /* it is not implemented either.  However NTLM support a package-specific     */
   /* extension to specify an Identity via the pAuthData parameter --            */
   /* -- that is the level of portability and abstraction one gets from SSPI ... */

   /* Since NTLM is actually a mediocre challenge-response authentication scheme    */
   /* the client will be able to "acquire" whatever CredentialsHandle he likes.     */
   /* Only the security context acceptor will verify whether these "credentials"    */
   /* are in fact valid or not.  Btw. mutual authentication is IMpossible, but you  */
   /* already knew that, didn't you ...?					    */


   /* Prepare an NTLM package-specific data structure for the pAuthData parameter      */
   /* of AcquireCredentialsHandle() to request the credentials of a specific Identity  */
   /* NOTE:  Although there is a password field in this structure -- because GSS-API   */
   /*        doesn't define the actual "creation" of credentials, we can not fill  it. */
   /*                                                                                  */
   /*        When providing an identity without specifying a password,                 */
   /*        NTLM will automatically try the default/cached password of the the        */
   /*        current logon session.  So when requesting credentials for a different    */
   /*        user and/or domain as the current logon session, the authentication will  */
   /*        succeed only when this other user account is using the same password.     */

   if ( p_name!=NULL && p_name_len!=0 ) {
      /* we were passed a user-supplied (canonical) name                    */
      /* we'll save a copy in the private part of the credentials structure */
      pcred->name = sy_malloc( p_name_len + 1 );
      if (pcred->name==NULL) {
	 ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
      }
      memcpy( pcred->name, p_name, p_name_len );
      pcred->name[p_name_len] = '\0';
      pcred->name_len = p_name_len;

   } else {
      /* no user-supplied name present -- we'll use the default name instead */
      maj_stat = ntlm_default_acceptor( pp_min_stat, &(pcred->name), &(pcred->name_len) );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto error;

   }

   /*************************************************************/
   /* We copy the name here for two braindead SSPI BUGS:        */
   /* 1.)  The members "domain" and "user" in the AuthIdentity  */
   /*      structure must be 0-terminated inspite of the        */
   /*      length fields, otherwise one gets an error.	        */
   /* 2.)  The domain and user parameters must be encoded in    */
   /*      OEM characters inspite of the WIN_IDENTITY_ANSI      */
   /*      flag, or international characters (like umlauts)     */
   /*************************************************************/

   maj_stat = ntlm_split_cname( pp_min_stat, pcred->name, pcred->name_len,
				&user, &user_len, &domain, &domain_len );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   /* NTLM SSP BUG: despite of the explicit length field, Windows NT 4.0sp3 */
   /* checks if the string is zero-terminated.                              */

   fallback=0;

   do { 

      memset( &AuthIdentity, 0, sizeof(AuthIdentity));

      if ( fallback==0 ) {
	 /* NTLM SSP BUG: SEC_WINNT_AUTH_IDENTITY_ANSI is horribly broken, some   */
	 /* implementations treat the characters as OEM chars internally.         */
	 /* We need to convert to Unicode here to work around this bug.           */

	 Wdomain_len = sizeof(Wdomain) / sizeof(WCHAR);
	 Wuser_len   = sizeof(Wuser)   / sizeof(WCHAR);
	 Wdomain_len = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, domain, (int)domain_len,
							    (LPWSTR)Wdomain, (int)Wdomain_len );
	 Wuser_len   = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, user,   (int)user_len,
							    (LPWSTR)Wuser,   (int)Wuser_len   );
	 ((LPWSTR)Wdomain)[Wdomain_len] = 0;
	 ((LPWSTR)Wuser)[Wuser_len]     = 0;

	 AuthIdentity.Flags        = SEC_WINNT_AUTH_IDENTITY_UNICODE;

      } else {
	 /* BUG BUG:  Some early Win95 implementations of the NTLM SSP don't      */
	 /*           understand the UNICODE variant we need this variant as well */
	 Wdomain_len = domain_len;  if ( Wdomain_len >= sizeof(Wdomain) ) { Wdomain_len = sizeof(Wdomain)-1 ; }
	 Wuser_len   = user_len;    if ( Wuser_len   >= sizeof(Wuser)   ) { Wuser_len   = sizeof(Wuser)-1;    }
	 strncpy( Wdomain, domain, Wdomain_len);
	 strncpy( Wuser, user, Wuser_len);
	 Wdomain[Wdomain_len] = 0;
	 Wuser[Wuser_len]     = 0;

	 AuthIdentity.Flags        = SEC_WINNT_AUTH_IDENTITY_ANSI;

	 /* We only get here if the UNICODE variant of this structure is rejected      */
	 /* the other possibility to workaround the OEM/ANSI confusion bug in NTLM SSP */
	 /* is to do the conversion here					       */
	 CharToOem(Wdomain,Wdomain);
	 CharToOem(Wuser,Wuser);

      }

      AuthIdentity.Domain       = Wdomain;
      AuthIdentity.DomainLength = (ULONG)Wdomain_len;

      AuthIdentity.User         = Wuser;
      AuthIdentity.UserLength   = (ULONG)Wuser_len;

      /* Now try to acquire a credentials handle with this User/Domain information */
      
      SecStatus = (fp_SSPI_ntlm->AcquireCredentialsHandle)(
	    /* SEC_CHAR   * pszPrincipal     */	    "",
	    /* SEC_CHAR   * pszPackage       */	    ntlm_provider,
	    /* ULONG        fCredentialUse   */	    CredentialUse,
	    /* PLUID        pvLogonID        */	    NULL,
	    /* PVOID        pAuthData        */	   &AuthIdentity,
	    /* PVOID        pGetKeyFn        */	    NULL,
	    /* PVOID        pvGetKeyArgument */	    NULL,
	    /* PCredHandle  phCredential     */	    &(pcred->sspi_cred),
	    /* PTimeStamp   ptsExpiry        */	    &(pcred->expiration) );

      fallback++;

   } while ( fallback<2 && SecStatus==SEC_E_INVALID_TOKEN );

   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      ntlm_sec_error( SecStatus, &symbol, &desc );
      DEBUG_STRANGE((tf, "AcquireCredentialsHandle() failed with %s\n(Desc=\"%s\")\n",
			 symbol, desc));

      if (SECPKG_CRED_OUTBOUND==CredentialUse) {
	  ERROR_RETURN( NTLM_MINOR_SSPI(AcquireCredentialsHandleOUT,SecStatus), GSS_S_FAILURE );
      }
      ERROR_RETURN( NTLM_MINOR_SSPI(AcquireCredentialsHandleIN,SecStatus), GSS_S_FAILURE );
   }

#ifndef NTLM_CORRECT_CREDENTIAL_EXPIRATION
   if ( ntlm_platform_id==VER_PLATFORM_WIN32_WINDOWS
        &&  ntlm_platform_minor==0 ) {
      /* BUGBUG: NTLM in Windows'95 returns the current time for credentials   */
      /* expiration instead of indefinite or a point in time of the future ... */
      pcred->expiration.LowPart  = 0xffffffffu;
      pcred->expiration.HighPart = 0x7fffffffu;
   }
#endif

   pcred->valid_from      = time(NULL);
   pcred->expires_at      = ntlm_timestamp2time( &(pcred->expiration) );

   if ( pp_expires_at!=NULL ) {
      (*pp_expires_at) = pcred->expires_at;
   }

#if 0
/*
 * Although it was in the specification from the very beginning of SSPI
 * QueryCredentialsAttributes() was never implemented in NTLM for
 * Windows '95 and NT4 (up to at least SP3).
 *
 * i.e. with SSPI you can NOT portably specify whose credentials you
 *      want to use, NOR can you find out whom a credentials handle (especially
 *      the "default" handle) will identify when establishing a security context.
 *
 * On Windows NT 4 it is possible to query the current logon session for
 * user and domain name -- one documented method is to query the access
 * token of the current process/thread.  However this doesn't work on
 * Windows '9x platform because they're lacking the TCB.  Although
 * GetUserName() work on Windows'9x, there is no API to find out the
 * name of the NTLM Workgroup or Domain.  The best workaround that
 * Microsoft support suggested would be to query the Registry key for
 * the Domain name cached for the network provider.
 *
 *   Key "HKLM\\System\\CurrentControlSet\\Services\\MSNP32\\NetworkProvider"
 *   Value "AuthenticatingAgent"
 *
 *

   SecStatus = (fp_SSPI_ntlm->QueryCredentialsAttributes)( &(pcred->sspi_cred),
						     SECPKG_CRED_ATTR_NAMES,
						     &sspi_cred_name );
   if ( SecStatus!=SEC_E_OK ) {
      char *symbol, *desc;
      
      ntlm_sec_error( SecStatus, &symbol, &desc );
      DEBUG_STRANGE((tf, "QueryCredentialsAttributes() failed with %s\n(Desc=\"%s\")\n",
			 symbol, desc));

      ERROR_RETURN( NTLM_SSPI(QueryCredentialsAttributesNAMES,SecStatus), GSS_S_FAILURE );

   } else {
      size_t   len = strlen(sspi_cred_name.sUserName);

      if (len==0 || len>= GN_MAX_NAMELEN) {
	 ERROR_RETURN( MINOR_INVALID_USERNAME, GSS_S_FAILURE );
      }
      pcred->name = sy_malloc( len + 1 );
      if ( pcred->name==NULL ) {
	 ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
      }
      memcpy( pcred->name, sspi_cred_name.sUserName, len );
      pcred->name_len = len;
      pcred->name[len] = '\0';
   }
*/
#endif /* if 0 */

   if (maj_stat!=GSS_S_COMPLETE) {
error:
      if ( pcred!=NULL ) {
         OM_uint32 min_stat;

	 (void) ntlm_release_cred( &min_stat, &pcred );
	 pcred = NULL;
      }
   }

   (*pp_cred) = pcred;

#if 0
/*
 * Recycle the buffer from QueryCredentialsAttributes() ...
 *
   if ( sspi_cred_name.sUserName!=NULL ) {
      (fp_SSPI_ntlm->FreeContextBuffer)( sspi_cred_name.sUserName );
      sspi_cred_name.sUserName = NULL;
   }
*/
#endif /* if 0 */

   return(maj_stat);

} /* ntlm_acquire_cred() */




/*********************************************************************
 * ntlm_release_cred()
 *
 *********************************************************************/
OM_uint32
ntlm_release_cred( OM_uint32       * pp_min_stat,
		   void           ** pp_cred
     )
{
   char            * this_Call = "gmech_release_cred";
   ntlm_cred_desc  * pcred     = NULL;
   OM_uint32         maj_stat  = GSS_S_COMPLETE;
   SECURITY_STATUS   SecStatus = SEC_E_OK;

   (*pp_min_stat) = 0;

   maj_stat = ntlm_check_cred( pp_min_stat, pp_cred, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   pcred = (ntlm_cred_desc *) (*pp_cred);
   if ( IsValidSecurityHandle(&(pcred->sspi_cred)) ) {
      SecStatus = (fp_SSPI_ntlm->FreeCredentialHandle)( &(pcred->sspi_cred) );

      if ( SecStatus!=SEC_E_OK ) {
	 char *symbol, *desc;
      
	 ntlm_sec_error( SecStatus, &symbol, &desc );
	 DEBUG_STRANGE((tf, "FreeCredentialsHandle() failed with %s\n(Desc=\"%s\")\n",
	 		    symbol, desc));

	 (*pp_min_stat) = NTLM_MINOR_SSPI(FreeCredentialsHandle,SecStatus);
	 maj_stat       = GSS_S_FAILURE;
      }
   }

   sy_clear_free( (void **) &(pcred->name), pcred->name_len );   /* If we kept one around, it must be released */
   sy_clear_free(           pp_cred,        sizeof(*pcred) );    /* clear and release			       */
								 /*   the whole private credentials struct     */
   return(maj_stat);

} /* ntlm_release_cred() */



/*********************************************************************
 *  ntlm_inquire_cred()
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
ntlm_inquire_cred( OM_uint32         * pp_min_stat,
		   void              * p_cred,
		   Uchar            ** pp_name,
		   size_t            * pp_name_len,
		   gn_nt_tag_et      * pp_nt_tag,
		   gn_cred_usage_et  * pp_usage,
		   time_t            * pp_expires_at
     )
{
   char              * this_Call = "ntlm_inquire_cred";
   ntlm_cred_desc    * prvcred;
   OM_uint32           maj_stat;

   (*pp_min_stat) = 0;

   if ( pp_name!=NULL     )  { (*pp_name)     = NULL;           }
   if ( pp_name_len!=NULL )  { (*pp_name_len) = 0;              }
   if ( pp_nt_tag!=NULL   )  { (*pp_nt_tag)   = NT_INVALID_TAG; }

   maj_stat = ntlm_check_cred( pp_min_stat, &p_cred, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   prvcred = p_cred;
   if ( pp_usage!=NULL )      { (*pp_usage)      = prvcred->usage;      }

   if ( pp_expires_at!=NULL ) { (*pp_expires_at) = prvcred->expires_at; }

   if ( pp_name!=NULL && pp_name_len!=NULL && pp_nt_tag!=NULL ) {
      (*pp_name)     = prvcred->name;
      (*pp_name_len) = prvcred->name_len;
      (*pp_nt_tag)   = ntlm_nt_tag;
   } else if ( pp_name!=NULL || pp_name_len!=NULL || pp_nt_tag==NULL ) {
      DEBUG_ERR((tf, "Internal ERROR: %s(): invalid name parameters\n", this_Call))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   return(GSS_S_COMPLETE);  

} /* ntlm_inquire_cred() */

