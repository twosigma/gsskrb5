#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/krb5/krb5main.c#4 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/krb5/krb5main.c#4 $
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

#define KERBEROS_DLL_NAME	 "SECUR32.DLL"
#define KERBEROS_PROVIDER_NAME	 "Kerberos"

HINSTANCE                krb5_secdll_hInst     = NULL;
SECDLL_INIT            * fp_krb5_secdll_init   = NULL;
SECDLL_INIT_W          * fp_krb5_secdll_init_W = NULL;

char                   * krb5_secdll_name    = KERBEROS_DLL_NAME;

PSecurityFunctionTableA  fp_SSPI_krb5        = NULL; /* from InitSecurityInterfaceA()   */
PSecurityFunctionTableW  fp_SSPI_krb5_W      = NULL; /* from InitSecurityInterfaceW()   */

TRANSLATENAME          * fp_TranslateName    = NULL; /* TranslateName()			*/
GETUSERNAMEEX	       * fp_GetUserNameEx    = NULL; /* GetUserNameEx()			*/

ULONG                    krb5_capabilities   = 0;    /* from QuerySecurityPackageInfo() */
ULONG                    krb5_maxtoken       = 0;    /* from QuerySecurityPackageInfo() */
USHORT                   krb5_version        = 0;    /* from QuerySecurityPackageInfo() */
SEC_CHAR               * krb5_provider       = KERBEROS_PROVIDER_NAME;
DWORD			 krb5_platform_id    = 0;
DWORD		         krb5_platform_minor = 0;
DWORD			 krb5_platform_major = 0;
BOOLEAN                  krb5_realm_to_upper = TRUE;
BOOLEAN			 krb5_have_directory = FALSE; /* Flag whether we have access to */
						      /* an active directory so that    */
						      /* TranslateName() may be used    */
						      /* during name canonicalization   */
BOOLEAN                  krb5_ctx_noexpire   = FALSE; /* SSPI bug workaround: SSPI reports defective lifetimes */

struct gn_mechanism_s
    krb5_mechanism = {
			GSS_C_NO_OID, /* updated by krb5_initialize() -- see below           */
			"GSS-API wrapper for Microsoft's Kerberos SSPI in Windows2000",
			TRUE,         /* indicate that this mech processes raw gssapi tokens */
			krb5_initialize,
			krb5_cleanup,
			krb5_canonicalize_name,
			krb5_display_name,
			krb5_import_expname,
			krb5_inquire_nametypes,
			krb5_acquire_cred,
			krb5_release_cred,
			krb5_inquire_cred,
			krb5_init_sec_context,
			krb5_accept_sec_context,
			krb5_delete_sec_context,
			krb5_release_token,
			krb5_export_sec_context,
			krb5_import_sec_context,
			krb5_context_time,
			krb5_context_name,
			krb5_display_minstat,
			krb5_get_mic,
			krb5_verify_mic,
			krb5_wrap,
			krb5_unwrap,
			krb5_wrap_size_limit

};   


OM_uint32    krb5_minor_msel = 0;

#define REG_MIN(name,desc)  { 0, "KRB5_MINOR_"  #name, desc }

enum krb5_reg_min_vals_e {
   REG_GETVERSIONEX_FAILED   = 0,
   REG_LOADLIBRARY_FAILED,
   REG_INITSECURITY_MISSING,
   REG_INITSECURITY_FAILED,
   REG_QUERYPACKAGE_FAILED,
   REG_TRANSLATENAME_MISSING,
   REG_GETUSERNAMEEX_MISSING,
   REG_INTEGRITY_DENIED,
   REG_CONFIDENTIALITY_DENIED,
   REG_BUGGY_OS_RELEASE,
   REG_UNUSABLE_USERACCOUNT
};

gn_min_stat_desc krb5_reg_min_stat[] = {
   REG_MIN(GETVERSIONEX_FAILED,	   "GetVersionEx() failed" ),
   REG_MIN(LOADLIBRARY_FAILED,     "LoadLibrary() failed (" KERBEROS_DLL_NAME " missing?)" ),
   REG_MIN(INITSECURITY_MISSING,   "InitSecurityInterfaceA() not found in " KERBEROS_DLL_NAME ),
   REG_MIN(INITSECURITY_FAILED,    "InitSecurityInterfaceA() failed" ),
   REG_MIN(QUERYPACKAGE_FAILED,    "QuerySecurityPackageInfo(\"" KERBEROS_PROVIDER_NAME "\") failed" ),
   REG_MIN(TRANSLATENAME_MISSING,  "TranslateNameA() not found in " KERBEROS_DLL_NAME ),
   REG_MIN(GETUSERNAMEEX_MISSING,  "GetUserNameExA() not found in " KERBEROS_DLL_NAME ),
   REG_MIN(INTEGRITY_DENIED,	   KERBEROS_PROVIDER_NAME " missing Integrity protection" ),
   REG_MIN(CONFIDENTIALITY_DENIED, KERBEROS_PROVIDER_NAME " missing Confidentiality protection" ),
   REG_MIN(BUGGY_OS_RELEASE,       "Unsupported OS build (known Kerberos SSPI bugs)" ),
   REG_MIN(UNUSABLE_USERACCOUNT,   "Kerberos SSPI not usable with this User account" ),
   { 0, 0, 0 }
};

#define REG_MINOR(x)   (krb5_reg_min_stat[REG_ ## x].value)


HKEY krb5_basekeys[2] = { (HKEY)HKEY_CURRENT_USER, (HKEY)HKEY_LOCAL_MACHINE };

int
krb5_query_registry_tweak(char * p_reg_param, DWORD * pp_rval)
{
   LONG		   Status;
   HKEY            PRV_key   = 0;              /* temp dynamic */
   int             rc        = KRB5_NO_TWEAK;  /* not found    */
   int             i;

   (*pp_rval) = (DWORD)0;
 
   for ( i=0 ; i<(sizeof(krb5_basekeys)/sizeof(krb5_basekeys[0])) ; i++ ) {

      Status = RegOpenKeyEx( krb5_basekeys[i], "SOFTWARE\\SAP\\gsskrb5",
			     0, KEY_READ, &PRV_key );
      if ( ERROR_SUCCESS==Status ) {
         DWORD dwValue[2] = { 0, 0 };
         DWORD dwValueL  = sizeof(dwValue[0]);
         DWORD dwRegType = REG_DWORD;
         Status = RegQueryValueEx( PRV_key, p_reg_param,
			           NULL, &dwRegType, (LPBYTE)&(dwValue[0]), &dwValueL );
         RegCloseKey( PRV_key );
         PRV_key = 0;
         if ( ERROR_SUCCESS==Status && dwRegType==REG_DWORD ) {
            (*pp_rval) = dwValue[0];
            rc         = KRB5_HAVE_TWEAK;
            break;
         }
      }
   }

   return(rc);

} /* krb5_query_registry_tweak() */



/***********************************************************************
 * krb5_init()
 *
 * Description:
 *   Initialization call for PRIVATE functions and data structures
 *
 *   This function will be called ONCE and INITIALLY when an application
 *   does it's first call into one of the GSS-API functions.
 *
 *   GSS-API v1 and v2 do not have an initialization function, so the
 *   check has to be done within every API-call.
 *
 **********************************************************************/
OM_uint32
krb5_initialize( OM_uint32 * pp_min_stat )
{
   DWORD           dwValue;
   OM_uint32       maj_stat;

   (*pp_min_stat) = 0;

   /**************************************************************************/
   /* (11-Feb-2012) Martin Rex                                               */
   /* Check the Registry whether no there is an explicit ForceRealmToUpper   */
   /* configured, otherwise use the compiled-in default (TRUE)               */
   /* A ProtocolTransition scenario with Citrix has been reported to result  */
   /* in incorrect lowercase realm names showing up on the Acceptor side     */
   /* so we are going to force the realm to uppercase by default, but offer  */
   /* a registry tweak to disable this (for the few Kerberos realms that     */
   /* do use NON-uppercase Realms (Is OSF DCE still alive?)                  */
   /* Strange: If I try RegQueryValue() without RegOpenKeyEx() it fails!     */
   /**************************************************************************/
   if ( KRB5_HAVE_TWEAK==krb5_query_registry_tweak("ForceRealmToUpper", &dwValue) ) {
      krb5_realm_to_upper = (0==dwValue) ? FALSE : TRUE;
   }

   if ( KRB5_HAVE_TWEAK==krb5_query_registry_tweak("ForceCtxNoExpire", &dwValue) ) {
      krb5_ctx_noexpire = (0==dwValue) ? FALSE : TRUE;
   }

   maj_stat = gn_register_min_stat( pp_min_stat, &(krb5_reg_min_stat[0]) );
   if (maj_stat!=GSS_S_COMPLETE)
      goto error;

   maj_stat = krb5_load_dll( pp_min_stat );
   if ( maj_stat!=GSS_S_COMPLETE )
      goto error;

   maj_stat = krb5_init_nt_oid_tags( pp_min_stat );
   if ( maj_stat!=GSS_S_COMPLETE )
      goto error;

   krb5_mechanism.mech_oid = krb5_mech_oid;

   maj_stat = gn_register_mech( pp_min_stat,
				&krb5_mechanism,
				&krb5_mech_tag,
				&krb5_minor_msel );

   if ( maj_stat != GSS_S_COMPLETE )
      goto error;

error:
   return(maj_stat);

} /* krb5_init() */


OM_uint32
krb5_cleanup( void )
{
   return( krb5_unload_dll() );

} /* krb5_cleanup() */



/*
 * krb5_load_dll()
 *
 *
 */
OM_uint32
krb5_load_dll( OM_uint32 * pp_min_stat )
{
   OSVERSIONINFO   OsVers;
   SECURITY_STATUS SecStatus;
   PSecPkgInfo     PackageInfo = NULL;
   OM_uint32       maj_stat    = GSS_S_COMPLETE;

   (*pp_min_stat) = 0;

   if ( krb5_secdll_hInst==NULL ) {
      /* If the DLL hasn't been loaded yet, load it now */

      /* Figure out which Platform we're on: NT or '95  */
      memset( &OsVers, 0, sizeof(OsVers) );
      OsVers.dwOSVersionInfoSize = sizeof(OsVers);

      if ( GetVersionEx(&OsVers)==FALSE ) {

         (*pp_min_stat) = REG_MINOR(GETVERSIONEX_FAILED);
	 return(GSS_S_FAILURE);

      }
      
      krb5_platform_id    = OsVers.dwPlatformId;
      krb5_platform_minor = OsVers.dwMinorVersion;
      krb5_platform_major = OsVers.dwMajorVersion;

      if ( krb5_platform_id==VER_PLATFORM_WIN32_NT ) {
	 if ( krb5_platform_major<=4 ) {
	    krb5_secdll_name = "SECURITY.DLL";
	 } else {
	    krb5_secdll_name = "SECUR32.DLL";
	 }
      }

      krb5_secdll_hInst = LoadLibrary(krb5_secdll_name);
      if ( krb5_secdll_hInst==NULL ) {
	 (*pp_min_stat) = REG_MINOR(LOADLIBRARY_FAILED);
	 goto dll_init_error;
      }

      fp_krb5_secdll_init = (SECDLL_INIT *) GetProcAddress(krb5_secdll_hInst, "InitSecurityInterfaceA");
      if ( fp_krb5_secdll_init==NULL ) {
	 (*pp_min_stat) = REG_MINOR(INITSECURITY_MISSING);
	 goto dll_init_error;
      }
   
      fp_SSPI_krb5 = (fp_krb5_secdll_init)();
      if ( fp_SSPI_krb5==NULL ) {
	 (*pp_min_stat) = REG_MINOR(INITSECURITY_FAILED);
	 goto dll_init_error;
      }

      /* (13-Aug-2004) Martin Rex   Windows SSPI BUG workaround	 */
      /* get the function table for the WideChar variant of SSPI */
      /* we only need this to workaround a severe memory leak    */
      /* in QueryCredentialsAttributeA(NAMES)                    */
      fp_krb5_secdll_init_W = (SECDLL_INIT_W *) GetProcAddress(krb5_secdll_hInst, "InitSecurityInterfaceW");
      if ( NULL!=fp_krb5_secdll_init_W ) {
	 fp_SSPI_krb5_W = (fp_krb5_secdll_init_W)();
      }

      SecStatus = (fp_SSPI_krb5->QuerySecurityPackageInfo)( krb5_provider, &PackageInfo );
      if ( SecStatus!=SEC_E_OK ) {

	 (*pp_min_stat) = REG_MINOR(QUERYPACKAGE_FAILED);
	 goto dll_init_error;

      }

      fp_TranslateName = (TRANSLATENAME *) GetProcAddress(krb5_secdll_hInst, "TranslateNameA");
      if ( fp_TranslateName==NULL ) {
	  (*pp_min_stat) = REG_MINOR(TRANSLATENAME_MISSING);
	  goto dll_init_error;
      }

      fp_GetUserNameEx = (GETUSERNAMEEX *) GetProcAddress(krb5_secdll_hInst, "GetUserNameExA");
      if ( fp_GetUserNameEx==NULL ) {
	  (*pp_min_stat) = REG_MINOR(GETUSERNAMEEX_MISSING);
	  goto dll_init_error;
      }

      /**************************************************************/
      /* Check an memorize whether GetUserNameEx(NameUserPrincipal) */
      /* works -- it only works when an Active Directory is present */
      /**************************************************************/
      {
	  char        buf[KRB5_MAX_NAMELEN*2];
	  ULONG       buf_size;

	  buf_size   = sizeof(buf)-1;

	  if ( (fp_GetUserNameEx)(NameUserPrincipal, &(buf[0]), &buf_size)==FALSE ) {
	      krb5_have_directory = TRUE;
	  }
      }

      krb5_capabilities = PackageInfo->fCapabilities;
      krb5_maxtoken     = PackageInfo->cbMaxToken;

      if ( (krb5_capabilities & SECPKG_FLAG_PRIVACY)==0 ) {
	 (*pp_min_stat) = REG_MINOR(INTEGRITY_DENIED);
	 maj_stat       = GSS_S_FAILURE;
      } else if ( (krb5_capabilities & SECPKG_FLAG_INTEGRITY)==0 ) {
	 (*pp_min_stat) = REG_MINOR(CONFIDENTIALITY_DENIED);
	 maj_stat       = GSS_S_FAILURE;
      }

// BUGBUG:  Windows2000-RC1 (build 2078) contains several fatal bugs.
//	    Security context export will corrupt the stack.
//
// BUGBUG:  Another Bug in SECUR32.DLL prevents ImportSecurityContext() from working
//          when the Security Package has not been addressed by name before
//          This should be fixed in Windows2000-RC2,
//	    so we check for RC1 build here
//
// BUGBUG:  Windows2000-RC2 (build2128) breaks horribly on security context transfer,
//          even the workaround from build 2078 fails -- you need at least build2132
//

      if ( OsVers.dwPlatformId==VER_PLATFORM_WIN32_NT
	   && OsVers.dwMajorVersion==5
	   && OsVers.dwBuildNumber<2132 ) {

	      /* We will abort on OS builds that are known */
	      /* to contain fatal SSPI BUGs		      */
	      (*pp_min_stat) = REG_MINOR(BUGGY_OS_RELEASE);
	      goto dll_init_error;

      }

//
// BEGIN WORKAROUND for SECUR32.DLL bug in build 2078
#if 1
	 {
	    SECURITY_STATUS   SecStatus;
	    CredHandle        SspiCred;

	    SspiCred.dwUpper = 0xffffffff;
	    SspiCred.dwLower = 0xffffffff;

	    SecStatus = (fp_SSPI_krb5->AcquireCredentialsHandle)(
			NULL,
			krb5_provider,
			SECPKG_CRED_OUTBOUND,
			NULL,
			NULL,
			NULL,
			NULL,
			&SspiCred,
			NULL );
	    if ( SspiCred.dwUpper!=0xffffffff
	       ||  SspiCred.dwLower!=0xffffffff ) {
	       SecStatus = (fp_SSPI_krb5->FreeCredentialsHandle)( &SspiCred );
	    }
	 }
#endif
// END WORKAROUND
//

#if 1
      /* Try to retrieve the Kerberos principal name for the current account  */
      /* krb5_get_default_name() will call GetUserNameEx(NameUserPrincipal)   */
      /* or when operating without Active Directory will try to pull the      */
      /* name from the default Kerberos initiating credentials		      */
      /*                                                                      */
      /* If we logged in with an account that is not known to Kerberos,       */
      /* then we can not obtain default (initiating) credentials -- we better */
      /* fail the Kerberos mechanism initialization in this case              */
      {
	 char  user[KRB5_MAX_NAMELEN];
	 char  domain[KRB5_MAX_NAMELEN];

	 maj_stat = krb5_get_default_name( pp_min_stat, domain, sizeof(domain)-1, user, sizeof(user)-1 );
	 if (maj_stat!=GSS_S_COMPLETE) {
	    (*pp_min_stat) = REG_MINOR(UNUSABLE_USERACCOUNT);
	    goto dll_init_error;
	 }
      }
#endif


      if ( PackageInfo!=NULL ) {
         (fp_SSPI_krb5->FreeContextBuffer)( PackageInfo );
	 PackageInfo = NULL;
      }

      if ( maj_stat!=GSS_S_COMPLETE ) {
dll_init_error:
         if ( PackageInfo!=NULL && fp_SSPI_krb5!=NULL && (fp_SSPI_krb5->FreeContextBuffer)!=NULL ) {
            (fp_SSPI_krb5->FreeContextBuffer)( PackageInfo );
	    PackageInfo = NULL;
	 }

	 krb5_unload_dll();

	 maj_stat = GSS_S_FAILURE; /* Here we always have mechanism specific errors */
      }

   }

   return(maj_stat);

} /* krb5_load_dll() */




/*
 * krb5_unload_dll()
 *
 *
 */
OM_uint32
krb5_unload_dll( void )
{
   fp_SSPI_krb5          = NULL;
   fp_SSPI_krb5_W        = NULL;

   fp_TranslateName      = NULL;
   fp_GetUserNameEx      = NULL;
   fp_krb5_secdll_init   = NULL;
   fp_krb5_secdll_init_W = NULL;

   if ( krb5_secdll_hInst != NULL ) {
      FreeLibrary( krb5_secdll_hInst );
      krb5_secdll_hInst = NULL;
   }

   return(GSS_S_COMPLETE);

} /* krb5_unload_dll() */



struct krb5_error_txt_s {
   SECURITY_STATUS    ecode;
   char             * esymbol;
   char             * edescription;
};



/* Microsoft Windows NT
 * SSPI Error codes as defined in the header file ISSPERR.H
 */
#undef SSPIE
#define SSPIE(x,y)   { x, #x, y }

struct krb5_error_txt_s krb5_error_txt[] = {

   SSPIE(NERR_DCNotFound,		  "Domain Controller not found"),
   SSPIE(NERR_NetNotStarted,		  "Network service not started"),
   SSPIE(NERR_WkstaNotStarted,		  "Workstation service not started"),
   SSPIE(NERR_ServerNotStarted,		  "Server services not started"),
   SSPIE(NERR_BrowserNotStarted,	  "Browser service not started"),
   SSPIE(NERR_ServiceNotInstalled,	  "Service not installed"),
   SSPIE(NERR_BadTransactConfig,	  "Bad transaction configuration"),

   SSPIE(ERROR_LOGON_FAILURE,		  "Logon failure"),
   SSPIE(ERROR_ACCESS_DENIED,		  "Access denied"),
   SSPIE(ERROR_NOT_SUPPORTED,		  "Not Supported"),
   SSPIE(ERROR_NO_LOGON_SERVERS,	  "No logon servers"),
   SSPIE(ERROR_NO_SUCH_DOMAIN,		  "No such domain"),
   SSPIE(ERROR_NO_TRUST_LSA_SECRET,	  "No trusting LSA secret"),
   SSPIE(ERROR_NO_TRUST_SAM_ACCOUNT,	  "No trusting SAM account"),
   SSPIE(ERROR_DOMAIN_TRUST_INCONSISTENT, "Inconsistent domain trust configuration"),
   SSPIE(ERROR_BAD_NETPATH,		  "Bad network path"),
   SSPIE(ERROR_FILE_NOT_FOUND,		  "File not found"),

   SSPIE(SEC_E_INSUFFICIENT_MEMORY,	  "Not enough memory available to complete request"),
   SSPIE(SEC_E_INVALID_HANDLE,		  "Specified handle is invalid"),
   SSPIE(SEC_E_UNSUPPORTED_FUNCTION,	  "Requested function is unavailable or unsupported"),
   SSPIE(SEC_E_TARGET_UNKNOWN,		  "Specified target is unknown or unreachable"),
   SSPIE(SEC_E_INTERNAL_ERROR,		  "(internal error) The LSA cannot be contacted"),
   SSPIE(SEC_E_SECPKG_NOT_FOUND,	  "Unrecognized security packages was requested"),
   SSPIE(SEC_E_NOT_OWNER,		  "Caller is not the owner of the requested credentials"),
   SSPIE(SEC_E_CANNOT_INSTALL,		  "Security package failed to initialize"),
   SSPIE(SEC_E_INVALID_TOKEN,		  "Token supplied to this function is invalid"),
   SSPIE(SEC_E_CANNOT_PACK,		  "Logon attempt failed because of strange parsing problems"),
   SSPIE(SEC_E_QOP_NOT_SUPPORTED,	  "Per-message QOP is not supported by the security package"),
   SSPIE(SEC_E_NO_IMPERSONATION,	  "Security context does not allow impersonation of client"),
   SSPIE(SEC_E_LOGON_DENIED,		  "Logon attempt failed"),
   SSPIE(SEC_E_UNKNOWN_CREDENTIALS,	  "Credentials supplied to security package were not recognized"),
   SSPIE(SEC_E_NO_CREDENTIALS,		  "No credentials available in security package"),
   SSPIE(SEC_E_MESSAGE_ALTERED,		  "Integrity check failed for supplied message"),
   SSPIE(SEC_E_OUT_OF_SEQUENCE,		  "Supplied message is out of sequence"),
   SSPIE(SEC_E_NO_AUTHENTICATING_AUTHORITY, "No authority could be contacted for authentication"),
   SSPIE(SEC_I_CONTINUE_NEEDED,		  "CONTINUE_NEEDED: the function needs to be called again"),
   SSPIE(SEC_I_COMPLETE_NEEDED,		  "Function CompleteToken() needs to be called"),
   SSPIE(SEC_I_COMPLETE_AND_CONTINUE,	  "COMPLETE and CONTINUE"),
   SSPIE(SEC_I_LOCAL_LOGON,		  "Logon completed based on locally available information"),
   SSPIE(SEC_E_BAD_PKGID,		  "Requested security package does not exist"),
   SSPIE(SEC_E_CONTEXT_EXPIRED,		  "Referenced security context has expired"),
   SSPIE(SEC_E_INCOMPLETE_MESSAGE,	  "Supplied message is incomplete and could therefore not be verified"),
   SSPIE(SEC_E_INCOMPLETE_CREDENTIALS,	  "Supplied credentials are incomplete; context initialization failed"),
   SSPIE(SEC_E_BUFFER_TOO_SMALL,	  "Supplied output buffer was to small"),
   SSPIE(SEC_I_INCOMPLETE_CREDENTIALS,	  "Supplied credentials are incomplete and could not be verified"),
   SSPIE(SEC_I_RENEGOTIATE,		  "Context data must be renegotiated with the peer"),
   SSPIE(SEC_E_WRONG_PRINCIPAL,		  "Wrong principal in context establishment token"),

#ifdef SEC_I_NO_LSA_CONTEXT
   SSPIE(SEC_I_NO_LSA_CONTEXT,		  "No LSA context"),
#endif

#ifdef SEC_E_TIME_SKEW
   SSPIE(SEC_E_TIME_SKEW,		  "Time skew between communication peers too big"),
#endif

#ifdef SEC_E_UNTRUSTED_ROOT
   SSPIE(SEC_E_UNTRUSTED_ROOT,		  "Untrusted root CA"),
#endif

#ifdef SEC_E_ILLEGAL_MESSAGE
   SSPIE(SEC_E_ILLEGAL_MESSAGE,		  "Illegal message (format?)"),
#endif

#ifdef SEC_E_CERT_UNKNOWN
   SSPIE(SEC_E_CERT_UNKNOWN,		  "Unknown certificate"),
#endif

#ifdef SEC_E_CERT_EXPIRED
   SSPIE(SEC_E_CERT_EXPIRED,		  "Certificate already expired"),
#endif

#ifdef SEC_E_ENCRYPT_FAILURE
   SSPIE(SEC_E_ENCRYPT_FAILURE,		  "unspecified encryption failure"),
#endif

#ifdef SEC_E_DECRYPT_FAILURE
   SSPIE(SEC_E_DECRYPT_FAILURE,		  "unspecified decryption failure"),
#endif

#ifdef SEC_E_ALGORITHM_MISMATCH
   SSPIE(SEC_E_ALGORITHM_MISMATCH,	  "Algorithm mismatch"),
#endif

#ifdef SEC_E_SECURITY_QOS_FAILED
   SSPIE(SEC_E_SECURITY_QOS_FAILED,	  "Security QOS failed"),
#endif

   { 0, NULL, NULL }
};


struct k5_sspi_call_name_s {
    k5_sspi_call_et    sspi_call;
    char             * sspi_call_name;
};

#ifdef USE_LONG_NAMES
struct k5_sspi_call_name_s k5_sspi_call_name[] = {
    { K5SSPI_CALL_InitSecurityInterface,	    "InitSecurityInterface()"		    },
    { K5SSPI_CALL_QuerySecurityPackageInfo,	    "QuerySecurityPackageInfo()"	    },
    { K5SSPI_CALL_FreeContextBuffer,		    "FreeContextBuffer()"		    },
    { K5SSPI_CALL_AcquireCredentialsHandleOUT,	    "AcquireCredentialsHandle(INI)"	    },
    { K5SSPI_CALL_AcquireCredentialsHandleIN,	    "AcquireCredentialsHandle(ACC)"	    },
    { K5SSPI_CALL_AcquireCredentialsHandleBOTH,	    "AcquireCredentialsHandle(BOTH)"	    },
    { K5SSPI_CALL_FreeCredentialsHandle,	    "FreeCredentialsHandle()"		    },
    { K5SSPI_CALL_QueryCredentialsAttributesNAMES,  "QueryCredentialsAttributes(NAMES)"	    },
    { K5SSPI_CALL_QueryCredentialsAttributesLIFESPAN, "QueryCredentialsAttributes(LIFESPAN)"  },
    { K5SSPI_CALL_InitializeSecurityContext1,	    "InitializeSecurityContext #1()"	    },
    { K5SSPI_CALL_InitializeSecurityContextN,	    "InitializeSecurityContext #n()"	    },
    { K5SSPI_CALL_AcceptSecurityContext1,	    "AcceptSecurityContext #1()"	    },
    { K5SSPI_CALL_AcceptSecurityContextN,	    "AcceptSecurityContext #n()"	    },
    { K5SSPI_CALL_DeleteSecurityContext,	    "DeleteSecurityContext()"		    },
    { K5SSPI_CALL_QueryContextAttributesNAMES,	    "QueryContextAttributes(NAMES)"	    },
    { K5SSPI_CALL_QueryContextAttributesSIZES,	    "QueryContextAttributes(SIZES)"	    },
    { K5SSPI_CALL_QueryContextAttributesNATNAMES,   "QueryContextAttributes(NATIVE_NAMES)"  },
    { K5SSPI_CALL_QueryContextAttributesLIFESPAN,   "QueryContextAttributes(LIFESPAN)"	    },
    { K5SSPI_CALL_ImpersonateSecurityContext,	    "ImpersonateSecurityContext()"	    },
    { K5SSPI_CALL_RevertSecurityContext,	    "RevertSecurityContext()"		    },
    { K5SSPI_CALL_ExportSecurityContext,	    "ExportSecurityContext()"		    },
    { K5SSPI_CALL_ImportSecurityContext,	    "ImportSecurityContext()"		    },
    { K5SSPI_CALL_MakeSignature,		    "MakeSignature()"			    },
    { K5SSPI_CALL_VerifySignature,		    "VerifySignature()"			    },
    { K5SSPI_CALL_EncryptMessageMIC,		    "EncryptMessageMIC()"		    },
    { K5SSPI_CALL_EncryptMessage,		    "EncryptMessage()"			    },
    { K5SSPI_CALL_DecryptMessage,		    "DecryptMessage()"			    },
    { K5SSPI_CALL_TranslateNameTOSAMCOMPAT,	    "TranslateName(->SamCompatible)"	    },
    { K5SSPI_CALL_TranslateNameTOPRINCIPAL,	    "TranslateName(->UserPrincipal)"	    },
    { K5SSPI_CALL_GetUserNameExSAMCOMPAT,	    "GetUserNameEx(SamCompatible)"	    },
    { K5SSPI_CALL_GetUserNameExPRINCIPAL,	    "GetUserNameEx(UserPrincipal)"	    },
    { K5SSPI_CALL_NONE,				    NULL				    }
};
#else
struct k5_sspi_call_name_s k5_sspi_call_name[] = {
    { K5SSPI_CALL_InitSecurityInterface,	    "IniSecIf()"	    },
    { K5SSPI_CALL_QuerySecurityPackageInfo,	    "QuSecPkgIn()"	    },
    { K5SSPI_CALL_FreeContextBuffer,		    "FrCtxBuf()"	    },
    { K5SSPI_CALL_AcquireCredentialsHandleOUT,	    "AcqCredHdl(INI)"	    },
    { K5SSPI_CALL_AcquireCredentialsHandleIN,	    "AcqCredHdl(ACC)"	    },
    { K5SSPI_CALL_AcquireCredentialsHandleBOTH,	    "AcqCredHdl(BOTH)"	    },
    { K5SSPI_CALL_FreeCredentialsHandle,	    "FrCredHdl()"	    },
    { K5SSPI_CALL_QueryCredentialsAttributesNAMES,  "QuCredAt(NAMES)"	    },
    { K5SSPI_CALL_QueryCredentialsAttributesLIFESPAN, "QuCredAt(LIFESPAN)"    },
    { K5SSPI_CALL_InitializeSecurityContext1,	    "IniSctx#1()"	    },
    { K5SSPI_CALL_InitializeSecurityContextN,	    "IniSctx#n()"	    },
    { K5SSPI_CALL_AcceptSecurityContext1,	    "AccSctx#1()"	    },
    { K5SSPI_CALL_AcceptSecurityContextN,	    "AccSctx#n()"	    },
    { K5SSPI_CALL_DeleteSecurityContext,	    "DelSctx()"		    },
    { K5SSPI_CALL_QueryContextAttributesNAMES,	    "QuCtxAt(NAMES)"	    },
    { K5SSPI_CALL_QueryContextAttributesSIZES,	    "QuCtxAt(SIZES)"	    },
    { K5SSPI_CALL_QueryContextAttributesNATNAMES,   "QuCtxAt(NATNAMES)"     },
    { K5SSPI_CALL_QueryContextAttributesLIFESPAN,   "QuCtxAt(LIFESPAN)"	    },
    { K5SSPI_CALL_ImpersonateSecurityContext,	    "ImpersSctx()"	    },
    { K5SSPI_CALL_RevertSecurityContext,	    "RevertSctx()"	    },
    { K5SSPI_CALL_ExportSecurityContext,	    "ExportSctx()"	    },
    { K5SSPI_CALL_ImportSecurityContext,	    "ImportSctx()"	    },
    { K5SSPI_CALL_MakeSignature,		    "MkSig()"		    },
    { K5SSPI_CALL_VerifySignature,		    "VfySig()"		    },
    { K5SSPI_CALL_EncryptMessageMIC,		    "EncMsg(MIC)"	    },
    { K5SSPI_CALL_EncryptMessage,		    "EncMsg()"		    },
    { K5SSPI_CALL_DecryptMessage,		    "DecMsg()"		    },
    { K5SSPI_CALL_TranslateNameTOSAMCOMPAT,	    "TrlNa(->SamC)"	    },
    { K5SSPI_CALL_TranslateNameTOPRINCIPAL,	    "TrlNa(->UPrin)"	    },
    { K5SSPI_CALL_GetUserNameExSAMCOMPAT,	    "GetUsnEx(SamC)"	    },
    { K5SSPI_CALL_GetUserNameExPRINCIPAL,	    "GetUsnEx(UPrin)"	    },
    { K5SSPI_CALL_NONE,				    NULL		    }
};
#endif





/*
 * krb5_sec_error()
 *
 *
 */
OM_uint32
krb5_sec_error( SECURITY_STATUS p_sec_status,
		char ** pp_err_symbol, char ** pp_err_description )
{
   struct krb5_error_txt_s   * p_sec_error;

   if ( pp_err_symbol!=NULL )
      (*pp_err_symbol) = "??";
   if ( pp_err_description!=NULL ) 
      (*pp_err_description) = "??";

   p_sec_error = &(krb5_error_txt[0]);
   for ( ; p_sec_error->esymbol!=NULL ; p_sec_error++ ) {

      if ( p_sec_status == p_sec_error->ecode ) {
	 if ( pp_err_symbol!=NULL )
	    (*pp_err_symbol)      = p_sec_error->esymbol;
	 if ( pp_err_description!=NULL ) 
	    (*pp_err_description) = p_sec_error->edescription;

	 return(GSS_S_COMPLETE);
      }
   }

   return(GSS_S_FAILURE);

} /* krb5_sec_error() */




/*
 * krb5_timestamp2time()
 *
 * Convert a 64-bit TIMESTAMP structure used by Microsoft's SSPI
 * into a Posix/Ansi-C  time_t value (presumably 32-bit).
 *
 * Warning, this suffers from the 2038-BUG.
 * The reuse of the (32-bit) constant GSS_C_INDEFINITE for
 * potentially 64-bit time_t will break in 2038.
 *
 *  Two exceptional return values to express expiration times
 *  of credentials and security contexts:
 *     for TIMESTAMP (SSPI):
 *            {           0,          0 }   "expired"
 *            { 0xffffffffu, 0x7fffffff }   "indefinite"
 *     for time_t/OM_uint32:
 *                                    0     "expired"
 *                           0xfffffffful   "indefinite"  GSS_C_INDEFINTE
 *
 * SSPI's TIMESTAMPs are documented as LocalTime (i.e. local timezone),
 * and NTLMSSP implements them as localtime.
 * the expiration times are 
 * is somewhat fuzzy (e.g. last param of AcquireCredenialsHandle() ... )
 *
 * time_t is ANSI-C / POSIX
 *     timezone = UTC   (Universal Coordinated Time / Greenwich Mean Time)
 *     unit     = seconds, counting from          January 1st, 1970
 * TIMESTAMP is Win32 SSPI
 *     timezone = LocalTime (local time zone)
 *     unit     = 100 nanoseconds, counting from  January 1st, 1601
 */
time_t
krb5_timestamp2time( TimeStamp * p_timestamp )
{
   LARGE_INTEGER  lt1;
   LARGE_INTEGER  lt2;
   time_t         xtime;

   if ( FALSE!=krb5_ctx_noexpire ) {
      return(GSS_C_INDEFINITE);
   }

#if 0
   DEBUG_ONLY_CMD( SYSTEMTIME  mysystime;   )
   DEBUG_ONLY_CMD( SYSTEMTIME  mylocaltime; )
   DEBUG_ONLY_CMD( time_t      now = time(NULL);  )
   DEBUG_ONLY_CMD( GetSystemTime( &mysystime );   )
   DEBUG_ONLY_CMD( GetLocalTime(  &mylocaltime ); )
   DEBUG_ACTION((tf, "  A: TimeStamp   = %s\n", dbg_sprint_timestamp(p_timestamp, TRUE) ));
   DEBUG_ACTION((tf, "  A: LocalTime   = %s\n", dbg_sprint_systemtime( &mylocaltime, TRUE) ));
   DEBUG_ACTION((tf, "  A: SystemTime  = %s\n", dbg_sprint_systemtime( &mysystime, FALSE) ));
   DEBUG_ACTION((tf, "  A: gmtime(now) = %s\n", dbg_sprint_time_t(now) ));
#endif

   if ( p_timestamp->LowPart==0
        &&  p_timestamp->HighPart==0 )
      return(0);

   if ( p_timestamp->LowPart==0xffffffffu
        &&  p_timestamp->HighPart==0x7fffffff )
      return(GSS_C_INDEFINITE);

   /* Indefinite comes in different flavours with Microsoft		 */
   /* -- it is some very large value, but they haven't decided which ... */
   if ( p_timestamp->HighPart>0x7f000000 )
      return(GSS_C_INDEFINITE);

   lt2.LowPart  = p_timestamp->LowPart;
   lt2.HighPart = p_timestamp->HighPart;

   /* DANGER! this pointer casting is permissible ONLY when            */
   /*         the structures LARGE_INTEGER and FILETIME are compatible */
   /*         (endianess).  LARGE_INTEGER has a stricter alignment, so */
   /*         casting them down to FILETIME should be ok ...           */
   LocalFileTimeToFileTime( (FILETIME *) &lt2, (FILETIME *) &lt1 );

   /* Adjust 1601 A.D. --> 1970 A.D. */
   lt1.QuadPart = lt1.QuadPart - (__int64)116444736000000000 ;

   if ( lt1.HighPart<0 ) {
      /* This date precedes the Posix time_t era ... */
      return(0);
   }

   if ( lt1.HighPart>=10000000 ) {
      /* this date exceeds the (unsigned 32-bit) Posix time_t era ... */
      return(GSS_C_INDEFINITE);
   }

   /* Since I don't know how to divide a full 64-bit number by a 32-bit number
    * into a 32-bit result, I'm using this workaround ...
    * 2038-BUG!   (VC98 doesn't have 64-bit math)
    */

   /* 32<<32 / 32-bit -> 32-bit division for the HighPart */
   xtime  = (OM_uint32) MulDiv( lt1.HighPart*4, 0x40000000, 10000000 );

   /* add the result from the LowPart division */
   xtime += lt1.LowPart/10000000;

   return( xtime );

} /* krb5_timestamp2time() */



/*
 * krb5_time2timestamp()
 *
 * Convert a Posix/Ansi-C  time_t value (presumably 32-bit on Win32)
 * into a 64-bit TIMESTAMP structure used by Microsoft's SSPI
 * 2038-BUG!
 *
 *
 * time_t is ANSI-C / POSIX
 *     timezone = UTC   (Universal Coordinated Time / Greenwich Mean Time)
 *     unit     = seconds, counting from          January 1st, 1970
 * TIMESTAMP is Win32 SSPI
 *     timezone = UTC   (Universal Coordinated Time / Greenwich Mean Time)
 *     unit     = 100 nanoseconds, counting from  January 1st, 1601
 */
void
krb5_time2timestamp( time_t p_time, TimeStamp * p_timestamp )
{
   LARGE_INTEGER  lt1;
   LARGE_INTEGER  lt2;

   if ( p_time == GSS_C_INDEFINITE ) {
      /* Create a TIMESTAMP with value "INDEFINITE" */
      p_timestamp->LowPart  = 0xffffffff;
      p_timestamp->HighPart = 0x7fffffff;
      return;
   } else if ( p_time == 0 ) {
      /* Create a TIMESTAMP with value "Expired" (I'm not sure this really exists) */
      p_timestamp->LowPart  = 0;
      p_timestamp->HighPart = 0;
      return;
   }

   lt1.QuadPart = Int32x32To64( (OM_uint32)p_time, 10000000 ) + (__int64)116444736000000000;

   FileTimeToLocalFileTime( (FILETIME *)&lt1, (FILETIME *)&lt2 );

   p_timestamp->LowPart  = lt2.LowPart;
   p_timestamp->HighPart = lt2.HighPart;

   return;

} /* krb5_time2timestamp() */




/*
 * krb5_copy_name()
 *
 *
 */
OM_uint32
krb5_copy_name( OM_uint32   * pp_min_stat,
		Uchar       * p_src,
		size_t        p_src_len,
		Uchar      ** p_dst,
		size_t      * p_dst_len )
{
   (*pp_min_stat) = 0;

   (*p_dst_len) = 0;

   if ( p_src_len==0 ) {

      (*p_dst) = NULL;

   } else {

      (*p_dst) = sy_malloc( p_src_len + 1 );
      if ( (*p_dst)==NULL ) {
	 RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
      }
      memcpy( (*p_dst), p_src, p_src_len );
      (*p_dst)[p_src_len] = '\0';
      (*p_dst_len)        = p_src_len;

   }

   return(GSS_S_COMPLETE);

} /* krb5_copy_name() */




/*************************************************************************/
/*************************************************************************/
/****   converting  Minor Status Codes  into printable text   ************/
/*************************************************************************/
/*************************************************************************/

struct num_to_text_s {
	OM_uint32      code;
	char         * name;
        char         * desc;
};

#define MIN_STRUCT_ENTRY(code,desc)	\
	{  (code), # code , desc }


#define MIN_ENTRY(code,desc)     MIN_STRUCT_ENTRY( KRB5_MINOR_ ## code , desc )

static struct num_to_text_s krb5_errors[] = {
    MIN_ENTRY(NO_ERROR,		    ""					   ),
    MIN_ENTRY(DOMAIN_MISSING,	    "Domainname missing in name"	   ),
    MIN_ENTRY(USER_MISSING,	    "Username missing in name"		   ),
    MIN_ENTRY(BAD_CHARS,	    "Invalid characters in name"	   ),
    MIN_ENTRY(CRED_ACCESS_DENIED,   "Access to SSPI credentials denied"	   ),
    MIN_ENTRY(SSPI_WRONG_MECH,      "SSPI returned token with invalid mechanism" ),
    MIN_ENTRY(SSPI_TALKS_GARBAGE,   "SSPI returns garbage (maybe wrong target name?)" ),
    MIN_ENTRY(REALM_MISSING,	    "Realm name missing after '@'" ),
    MIN_ENTRY(BAD_ESCAPE_SEQUENCE,  "Bad escape sequence in name"	   ),
    MIN_ENTRY(BAD_CHAR_IN_REALM,    "Bad characters in realm name"	   ),
    MIN_ENTRY(SSPI_BAD_CRED_NAME,   "No Kerberos SSPI credentials available for requested name" ),
    MIN_ENTRY(SSPI_CLIENT_ONLY,	    "SSPI indicates \"client only\" functionality" ),
    MIN_ENTRY(NO_CHBINDINGS_IN_SSPI,"The W2K Kerberos SSP doesn't support GSS-API channel bindings" ),

    MIN_ENTRY(W2K3_U2U_OWN,	    "SSPI u2u-problem: please add Service principal for own account" ),
    MIN_ENTRY(W2K3_U2U_TARGET,	    "SSPI u2u-problem: please add Service principal for target account" ),
    MIN_ENTRY(SSPI_BAD_CRED,        "SSPI-Problem: Credentials not usable for rfc-1964 Kerberos" ),
    MIN_ENTRY(NO_FQDN_HOSTNAME,     "Could not find FQDN hostname (DNS lookup failure?)" ),
};


#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))

/*
 * krb5_display_minstat()
 *
 *
 */
OM_uint32
krb5_display_minstat( OM_uint32   * pp_min_stat,
		      OM_uint32     minor_status,
		      char       ** pp_msg,
		      size_t	  * pp_msglen,
		      int	  * pp_need_release )
{
   char            * this_Call = "krb5_display_minstat";
   char              tmpbuf[256];
   char              winerr_buf[256];
   char		   * esymbol   = NULL;
   char            * edescr    = NULL;
   char            * ecall_name= "SSPI";
   k5_sspi_call_et   ecall     = K5SSPI_CALL_NONE;
   SECURITY_STATUS   SecStatus;
   OM_uint32	     maj_stat  = GSS_S_COMPLETE;
   OM_uint32	     min_val;
   OM_uint32	     min_class;
   Uint              i;

   (*pp_msg)          = NULL;
   (*pp_msglen)       = 0;
   (*pp_need_release) = 0;
   tmpbuf[0]          = '\0';

   min_class = (minor_status&MINOR_CLASS_MASK)>>MINOR_CLASS_SHIFT;
   min_val   = (minor_status&MINOR_VALUE_MASK);

   switch ( min_class ) {

      case KRB5_CLASS_GENERIC:
            for ( i=0 ; i<ARRAY_SIZE(krb5_errors) ; i++ ) {
	       if ( krb5_errors[i].code==min_val ) {
		  sprintf( tmpbuf, /* "%.50s: %.150s", krb5_errors[i].name, */
			   "%.150s", krb5_errors[i].desc );
		  break;
	       }
	    }

	    if ( tmpbuf[0]=='\0' ) {
	       DEBUG_ERR((tf, "ERR: %s(): Unknown KRB5_MINOR error 0x%08lx\n",
			      this_Call, (long) min_val ))
	       RETURN_MIN_MAJ( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );
	    }

	    break;


      case KRB5_CLASS_ERRNO:
	    sprintf(tmpbuf, "%.150s", strerror(min_val) );
	    if ( !sy_strcasecmp(tmpbuf, "no error") ) {
	       RETURN_MIN_MAJ( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );
	    }
	    break;


      case KRB5_CLASS_LASTERROR:
	    maj_stat = sy_lasterror( pp_min_stat, min_val,
				     tmpbuf, sizeof(tmpbuf)-1 );
	    if (maj_stat!=GSS_S_COMPLETE)
	       goto error;

	    break;


      case KRB5_CLASS_SSPI:
	    SecStatus = KRB5_MINOR_2_SSPI(min_val);
	    for ( i=0 ; krb5_error_txt[i].ecode!=0 ; i++ ) {
	       if (krb5_error_txt[i].ecode==SecStatus) {
		  esymbol = krb5_error_txt[i].esymbol;
		  edescr  = krb5_error_txt[i].edescription;
		  break;
	       }
	    }
	    ecall = KRB5_MINOR_2_SSPICALL(min_val);
	    for ( i=0 ; k5_sspi_call_name[i].sspi_call_name!=NULL ; i++ ) {
		if ( k5_sspi_call_name[i].sspi_call==ecall ) {
		    ecall_name = k5_sspi_call_name[i].sspi_call_name;
		    break;
		}
	    }

	    if ( NULL==edescr ) {
                OM_uint32 ms2;
                if ( GSS_S_COMPLETE==sy_lasterror(&ms2, SecStatus, winerr_buf, sizeof(winerr_buf)-2) ) {
                   winerr_buf[sizeof(winerr_buf)-1] = 0;
                }
                edescr = winerr_buf;
            }
            if ( NULL==edescr ) {
	        sprintf(tmpbuf, "SSPI::%.40s==Unknown SSPI error 0x%08lx",
			      ecall_name, (long)SecStatus);
	    } else {
	        sprintf(tmpbuf, /* "%.50s: %.150s", ntlm_error_txt[i].esymbol, */
				"SSPI::%.40s==%.150s", ecall_name, edescr );
	    }
	    break;

	    
      default:
	       DEBUG_ERR((tf, "ERR: %s(): Unknown minor status 0x%08lx\n",
			      this_Call, (long) min_val ))
	       RETURN_MIN_MAJ( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );

   }

   (*pp_msglen) = strlen(tmpbuf);

   if ( (*pp_msglen)>0 ) {
      (*pp_msg)    = sy_malloc( (*pp_msglen)+1 );
      if ( (*pp_msg)==NULL ) {
	 RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
      }

      (*pp_need_release) = 1;

      strcpy((*pp_msg),tmpbuf);
   }

error:

   return(maj_stat);

} /* krb5_display_minstat() */


