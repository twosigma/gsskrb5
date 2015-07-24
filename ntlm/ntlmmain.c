#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/ntlm/ntlmmain.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/ntlm/ntlmmain.c#2 $
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


/* unfortunately it is under-documented how to find out the */
/* domain name on Windows '95 ...			    */
/* We use the workaround proposed by Microsoft support to   */
/* query the Registry (see below under ntlm_load_dll()	    */
char                     alt_domain[128];

HINSTANCE                ntlm_secdll_hInst   = NULL;
SECDLL_INIT            * fp_ntlm_secdll_init = NULL;
char                   * ntlm_secdll_name    = "SECURITY.DLL";

PSecurityFunctionTable   fp_SSPI_ntlm          = NULL; /* from InitSecurityInterface()    */

ULONG                    ntlm_capabilities   = 0;    /* from QuerySecurityPackageInfo() */
ULONG                    ntlm_maxtoken       = 0;    /* from QuerySecurityPackageInfo() */
USHORT                   ntlm_version        = 0;    /* from QuerySecurityPackageInfo() */
SEC_CHAR               * ntlm_provider       = TEXT("NTLM");
DWORD			 ntlm_platform_id    = 0;
DWORD		         ntlm_platform_minor = 0;
DWORD			 ntlm_platform_major = 0;

int			 ntlm_emulate_ctx_xfer = TRUE;


struct gn_mechanism_s
    ntlm_mechanism = {
			GSS_C_NO_OID,
			"GSS-API wrapper for Microsoft's NTLM SSPI in NT4 & Win9x",
			FALSE,         /* indicate that this mech wants gssapi token framing */
			ntlm_initialize,
			ntlm_cleanup,
			ntlm_canonicalize_name,
			ntlm_display_name,
			ntlm_import_expname,
			ntlm_inquire_nametypes,
			ntlm_acquire_cred,
			ntlm_release_cred,
			ntlm_inquire_cred,
			ntlm_init_sec_context,
			ntlm_accept_sec_context,
			ntlm_delete_sec_context,
			ntlm_release_token,
			ntlm_export_sec_context,
			ntlm_import_sec_context,
			ntlm_context_time,
			ntlm_context_name,
			ntlm_display_minstat,

			ntlm_get_mic,
			ntlm_verify_mic,
			ntlm_wrap,
			ntlm_unwrap,

			ntlm_wrap_size_limit

};   


OM_uint32    ntlm_minor_msel = 0;


/***********************************************************************
 * ntlm_init()
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
ntlm_initialize( OM_uint32 * pp_min_stat )
{
   OM_uint32       maj_stat;

   (*pp_min_stat) = 0;

   maj_stat = ntlm_load_dll( pp_min_stat );
   if ( maj_stat!=GSS_S_COMPLETE )
      return(maj_stat);

   maj_stat = ntlm_init_nt_oid_tags( pp_min_stat );
   if ( maj_stat!=GSS_S_COMPLETE )
      return(maj_stat);

   ntlm_mechanism.mech_oid = ntlm_mech_oid;

   maj_stat = gn_register_mech( pp_min_stat,
				&ntlm_mechanism,
				&ntlm_mech_tag,
				&ntlm_minor_msel );

   if ( maj_stat != GSS_S_COMPLETE )
      return(maj_stat);

   return(maj_stat);

} /* ntlm_init() */


OM_uint32
ntlm_cleanup( void )
{
   return( ntlm_unload_dll() );

} /* ntlm_cleanup() */



/*
 * ntlm_load_dll()
 *
 *
 */
OM_uint32
ntlm_load_dll( OM_uint32 * pp_min_stat )
{
   OSVERSIONINFO   OsVers;
   SECURITY_STATUS SecStatus;
   PSecPkgInfo     PackageInfo = NULL;

   (*pp_min_stat) = 0;

   if ( ntlm_secdll_hInst==NULL ) {
      /* If the DLL hasn't been loaded yet, load it now */

      /* Figure out which Platform we're on: NT or '95  */
      memset( &OsVers, 0, sizeof(OsVers) );
      OsVers.dwOSVersionInfoSize = sizeof(OsVers);

      if ( GetVersionEx(&OsVers)==FALSE ) {
	 /* GetVersionEx() failed, let's panic here */
         (*pp_min_stat) = MINOR_INIT_FAILED;
	 return(GSS_S_FAILURE);

      }
      
      ntlm_platform_id    = OsVers.dwPlatformId;
      ntlm_platform_minor = OsVers.dwMinorVersion;
      ntlm_platform_major = OsVers.dwMajorVersion;

      if ( ntlm_platform_id==VER_PLATFORM_WIN32_WINDOWS) { /* Windows '95 special */
	 LONG   Status;
	 DWORD  dwlen;
	 DWORD  type;
	 HKEY   PRV_key = (HKEY)0;

	 /* The Windows '95 NTLM Provider is in SECUR32.DLL, not SECURITY.DLL              */
	 /* and you should probably use a later Win'95 OEM release or the Dcom95 update,   */
	 /* otherwise QueryPackageInfo() fails with SEC_E_FUNCTION_UNSUPPORTED (see below) */
	 ntlm_secdll_name    = "SECUR32.DLL";

	 /*************************************************************************************/
	 /* Windows'95 NTLM is severly broken since there is no of inquiring about the name   */
	 /* of the domain that NTLM will send out in the context establishment token ...      */
	 /* and QueryCredentialsAttributes() is not implemented most available NTLM SSPI      */
	 /* implementations including Windows NT ...					      */
	 /* UGLY KLUDGE: Let's retrieve the current logon-domain from the Registry on Win'95  */
	 /*										      */
	 /* Update: Apparently the Win'95 NTLM implementation in the Dcom95 Update implements */
	 /* QueryCredentialsAttributes(NAMES) for the INBOUND credentials.  It returns a name */
	 /* that looks like "<NT-Domain>\<PC-Netbios-Name>", provided that you have           */
	 /* configured pass-through authentication for file share access by "NT-Domain"	      */
	 /*************************************************************************************/
	 Status = RegOpenKeyEx( (HKEY)HKEY_LOCAL_MACHINE,
				  "System\\CurrentControlSet\\Services\\MSNP32\\NetworkProvider",
				  0,
				  KEY_READ,
				 &PRV_key );
	 if ( Status==ERROR_SUCCESS ) {
	    dwlen = sizeof(alt_domain) - 1;
	    type  = REG_SZ;
	    Status = RegQueryValueEx( PRV_key, "AuthenticatingAgent", NULL,
				      &type, (LPBYTE) &alt_domain, &dwlen );
	    if ( Status==ERROR_SUCCESS ) {
	       alt_domain[dwlen]                = '\0';
	       alt_domain[sizeof(alt_domain)-1] = '\0';
	    }
	    RegCloseKey( PRV_key );
	    PRV_key = (HKEY)0;
	 }

	 if ( Status!=ERROR_SUCCESS ) {
	    (*pp_min_stat) = MINOR_LASTERROR( GetLastError() );
	    goto dll_init_error;
	 }

	 if ( alt_domain[0]==0 ) {
	    strcpy(alt_domain, "WORKGROUP");
	 }

      } else if ( ntlm_platform_id==VER_PLATFORM_WIN32_NT ) {

	 if ( ntlm_platform_major<=4 ) {
	    /* On Windows NT 3.5x and 4.0, the library is called */
	    /* "SECURITY.DLL"					 */
	    ntlm_secdll_name = "SECURITY.DLL";
	 } else if ( ntlm_platform_major>=5 ) {
	    /* On Windows 2000, the library is called "SECUR32.DLL" */
	    /* just like on Win95/98				     */
	    ntlm_secdll_name = "SECUR32.DLL";
	 }

      }	/* endif (ntlm_platform_id) */


      ntlm_secdll_hInst = LoadLibrary(ntlm_secdll_name);
      if ( ntlm_secdll_hInst==NULL ) {
	 (*pp_min_stat) = MINOR_SHLIB_NOT_FOUND;
	 goto dll_init_error;
      }

      fp_ntlm_secdll_init = (SECDLL_INIT *)
			    GetProcAddress(ntlm_secdll_hInst, "InitSecurityInterfaceA");
      if ( fp_ntlm_secdll_init==NULL ) {
	 (*pp_min_stat) = MINOR_SHLIB_INVALID;
	 goto dll_init_error;
      }
   
      fp_SSPI_ntlm = (fp_ntlm_secdll_init)();
      if ( fp_SSPI_ntlm==NULL ) {
	 (*pp_min_stat) = MINOR_INIT_FAILED;
	 goto dll_init_error;
      }

      SecStatus = (fp_SSPI_ntlm->QuerySecurityPackageInfo)( ntlm_provider, &PackageInfo );
      if ( SecStatus==SEC_E_OK ) {

	 /* SIGH:  the capabilities would be able to provide useful information		*/
	 /*        but all of the broken implementations that Microsoft shipped so far	*/
	 /*        are forgetting to indicate most of their capabilities	        */
	 /*        or they're indicating capabilities inconsistently across platforms   */
	 /*        or they're indicating capabilities that are not available at the     */
	 /*        documented API e.g. confidentiality ...				*/
         ntlm_capabilities = PackageInfo->fCapabilities;
         ntlm_maxtoken     = PackageInfo->cbMaxToken;

      } else if ( SecStatus==SEC_E_UNSUPPORTED_FUNCTION
		  &&  VER_PLATFORM_WIN32_WINDOWS ) {
	 /* Another UGLY KLUDGE: would you believe that they forgot to implement this    */
	 /* on Windows '95?  You HAVE TO figure out the maximum token size for security  */
	 /* context establishment since they didn't implement ISC_REQ_ALLOCATE_MEMORY    */
	 /* either ...									 */
	 /* Oh well!  For the poor owners of the early Win'95 releases, let's emulate    */
	 /* the values that the bugfixed version of the DCom95-Update returns            */
	 ntlm_maxtoken	   = 768;
	 ntlm_capabilities = 0;   /* about as accurate as Microsoft's implementation */
	 SecStatus         = SEC_E_OK;

      }

      if ( PackageInfo!=NULL ) {
         (fp_SSPI_ntlm->FreeContextBuffer)( PackageInfo );
	 PackageInfo = NULL;
      }

      if ( SecStatus!=SEC_E_OK ) {
         (*pp_min_stat) = MINOR_SHLIB_INVALID;
dll_init_error:
	 ntlm_unload_dll();
 	    
	 return(GSS_S_FAILURE);
      }

   }

   return(GSS_S_COMPLETE);

} /* ntlm_load_dll() */




/*
 * ntlm_unload_dll()
 *
 *
 */
OM_uint32
ntlm_unload_dll( void )
{
   fp_ntlm_secdll_init   = NULL;
   fp_SSPI_ntlm		 = NULL;

   if ( ntlm_secdll_hInst != NULL ) {
      FreeLibrary( ntlm_secdll_hInst );
      ntlm_secdll_hInst = NULL;
   }

   return(GSS_S_COMPLETE);

} /* ntlm_unload_dll() */



struct ntlm_error_txt_s {
   SECURITY_STATUS    ecode;
   char             * esymbol;
   char             * edescription;
};



/* Microsoft Win32
 * SSPI Error codes as defined in the header file ISSPERR.H
 */

struct ntlm_error_txt_s ntlm_error_txt[] = {
   { SEC_E_INSUFFICIENT_MEMORY,	 "SEC_E_INSUFFICIENT_MEMORY",
     "Not enough memory available to complete the request"	      },
   { SEC_E_INVALID_HANDLE,	 "SEC_E_INVALID_HANDLE",
     "The specified handle is invalid"				      },
   { SEC_E_UNSUPPORTED_FUNCTION, "SEC_E_UNSUPPORTED_FUNCTION",
     "The requested function is not supported"			      },
   { SEC_E_TARGET_UNKNOWN,	 "SEC_E_TARGET_UNKNOWN",
     "The specified Target is unknown or unreachable"		      },
   { SEC_E_INTERNAL_ERROR,	 "SEC_E_INTERNAL_ERROR",
     "The Local Security Authority cannot be contacted"		      },
   { SEC_E_SECPKG_NOT_FOUND,	 "SEC_E_SECPKG_NOT_FOUND",
     "Unrecognized security package was requested"		      },
   { SEC_E_NOT_OWNER,		 "SEC_E_NOT_OWNER",
     "The caller is not the owner of the requested credentials"	      },
   { SEC_E_CANNOT_INSTALL,	 "SEC_E_CANNOT_INSTALL",
     "The security package failed to initialize"		      },
   { SEC_E_INVALID_TOKEN,	 "SEC_E_INVALID_TOKEN",
     "The token supplied to this function is invalid"		      },
   { SEC_E_CANNOT_PACK,		 "SEC_E_CANNOT_PACK",
     "Logon attempt failed because of strange parsing problems"	      },
   { SEC_E_QOP_NOT_SUPPORTED,	 "SEC_E_QOP_NOT_SUPPORTED",
     "The per-message QOP is not supported by the security package"   },
   { SEC_E_NO_IMPERSONATION,	 "SEC_E_NO_IMPERSONATION",
     "The security context does not allow impersonation of the client"	},
   { SEC_E_LOGON_DENIED,	 "SEC_E_LOGON_DENIED",
     "The logon attempt failed"					      },
   { SEC_E_UNKNOWN_CREDENTIALS,	 "SEC_E_UNKNOWN_CREDENTIALS",
     "The credentials supplied to the package were not recognized"    },
   { SEC_E_NO_CREDENTIALS,	 "SEC_E_NO_CREDENTIALS",
     "No credentials available in the security package"		      },
   { SEC_E_MESSAGE_ALTERED,	 "SEC_E_MESSAGE_ALTERED",
     "The integrity check failed for the supplied message"	      },
   { SEC_E_OUT_OF_SEQUENCE,	 "SEC_E_OUT_OF_SEQUENCE",
     "The supplied message is out of sequence"			      },
   { SEC_E_NO_AUTHENTICATING_AUTHORITY, "SEC_E_NO_AUTHENTICATING_AUTHORITY",
     "No authority could be contacted for authentication"	      },
   { SEC_I_CONTINUE_NEEDED,	 "SEC_I_CONTINUE_NEEDED",
     "CONTINUE_NEEDED: the function needs to be called again"	      },
   { SEC_I_COMPLETE_NEEDED,	 "SEC_I_COMPLETE_NEEDED",
     "COMPLETE_TOKEN: the function CompleteToken() needs to be called" },
   { SEC_I_COMPLETE_AND_CONTINUE, "SEC_I_COMPLETE_AND_CONTINUE",
     "COMPLETE_TOKEN and CONTINUE_NEEDED"			      },
   { SEC_I_LOCAL_LOGON,		 "SEC_I_LOCAL_LOGON",
     "The logon completed based on locally available information"     },
   { SEC_E_BAD_PKGID,		 "SEC_E_BAD_PKGID",
     "The requested security package does not exist"		      },
   { SEC_E_CONTEXT_EXPIRED,	 "SEC_E_CONTEXT_EXPIRED",
     "The referenced security context has expired"		      },
   { SEC_E_INCOMPLETE_MESSAGE,	 "SEC_E_INCOMPLETE_MESSAGE",
     "The supplied message is incomplete and could therefore not be verified" },
   { 0, NULL, NULL }
};


struct sspi_call_name_s {
    sspi_call_et    sspi_call;
    char          * sspi_call_name;
};

#ifdef USE_LONG_NAMES
struct sspi_call_name_s sspi_call_name[] = {
    { SSPI_CALL_InitSecurityInterface,		    "InitSecurityInterface()"		    },
    { SSPI_CALL_QuerySecurityPackageInfo,	    "QuerySecurityPackageInfo()"	    },
    { SSPI_CALL_FreeContextBuffer,		    "FreeContextBuffer()"		    },
    { SSPI_CALL_AcquireCredentialsHandleOUT,	    "AcquireCredentialsHandle(INI)"	    },
    { SSPI_CALL_AcquireCredentialsHandleIN,	    "AcquireCredentialsHandle(ACC)"	    },
    { SSPI_CALL_AcquireCredentialsHandleBOTH,	    "AcquireCredentialsHandle(BOTH)"	    },
    { SSPI_CALL_FreeCredentialsHandle,		    "FreeCredentialsHandle()"		    },
    { SSPI_CALL_QueryCredentialsAttributesNAMES,    "QueryCredentialsAttributes(NAMES)"	    },
    { SSPI_CALL_QueryCredentialsAttributesLIFESPAN, "QueryCredentialsAttributes(LIFESPAN)"  },
    { SSPI_CALL_InitializeSecurityContext1,	    "InitializeSecurityContext #1()"	    },
    { SSPI_CALL_InitializeSecurityContextN,	    "InitializeSecurityContext #n()"	    },
    { SSPI_CALL_AcceptSecurityContext1,		    "AcceptSecurityContext #1()"	    },
    { SSPI_CALL_AcceptSecurityContextN,		    "AcceptSecurityContext #n()"	    },
    { SSPI_CALL_DeleteSecurityContext,		    "DeleteSecurityContext()"		    },
    { SSPI_CALL_QueryContextAttributesNAMES,	    "QueryContextAttributes(NAMES)"	    },
    { SSPI_CALL_QueryContextAttributesSIZES,	    "QueryContextAttributes(SIZES)"	    },
    { SSPI_CALL_QueryContextAttributesNATNAMES,	    "QueryContextAttributes(NATIVE_NAMES)"  },
    { SSPI_CALL_QueryContextAttributesLIFESPAN,	    "QueryContextAttributes(LIFESPAN)"	    },
    { SSPI_CALL_ImpersonateSecurityContext,	    "ImpersonateSecurityContext()"	    },
    { SSPI_CALL_RevertSecurityContext,		    "RevertSecurityContext()"		    },
    { SSPI_CALL_ExportSecurityContext,		    "ExportSecurityContext()"		    },
    { SSPI_CALL_ImportSecurityContext,		    "ImportSecurityContext()"		    },
    { SSPI_CALL_MakeSignature,			    "MakeSignature()"			    },
    { SSPI_CALL_VerifySignature,		    "VerifySignature()"			    },
    { SSPI_CALL_EncryptMessageMIC,		    "EncryptMessageMIC()"		    },
    { SSPI_CALL_EncryptMessage,			    "EncryptMessage()"			    },
    { SSPI_CALL_DecryptMessage,			    "DecryptMessage()"			    },
    { SSPI_CALL_TranslateNameTOSAMCOMPAT,	    "TranslateName(->SamCompatible)"	    },
    { SSPI_CALL_TranslateNameTOPRINCIPAL,	    "TranslateName(->UserPrincipal)"	    },
    { SSPI_CALL_GetUserNameExSAMCOMPAT,		    "GetUserNameEx(SamCompatible)"	    },
    { SSPI_CALL_GetUserNameExPRINCIPAL,		    "GetUserNameEx(UserPrincipal)"	    },
    { SSPI_CALL_NONE,				    NULL				    }
};
#else
struct sspi_call_name_s sspi_call_name[] = {
    { SSPI_CALL_InitSecurityInterface,		    "IniSecIf()"	    },
    { SSPI_CALL_QuerySecurityPackageInfo,	    "QuSecPkgIn()"	    },
    { SSPI_CALL_FreeContextBuffer,		    "FrCtxBuf()"	    },
    { SSPI_CALL_AcquireCredentialsHandleOUT,	    "AcqCredHdl(INI)"	    },
    { SSPI_CALL_AcquireCredentialsHandleIN,	    "AcqCredHdl(ACC)"	    },
    { SSPI_CALL_AcquireCredentialsHandleBOTH,	    "AcqCredHdl(BOTH)"	    },
    { SSPI_CALL_FreeCredentialsHandle,		    "FrCredHdl()"	    },
    { SSPI_CALL_QueryCredentialsAttributesNAMES,    "QuCredAt(NAMES)"	    },
    { SSPI_CALL_QueryCredentialsAttributesLIFESPAN, "QuCredAt(LIFESPAN)"    },
    { SSPI_CALL_InitializeSecurityContext1,	    "IniSctx#1()"	    },
    { SSPI_CALL_InitializeSecurityContextN,	    "IniSctx#n()"	    },
    { SSPI_CALL_AcceptSecurityContext1,		    "AccSctx#1()"	    },
    { SSPI_CALL_AcceptSecurityContextN,		    "AccSctx#n()"	    },
    { SSPI_CALL_DeleteSecurityContext,		    "DelSctx()"		    },
    { SSPI_CALL_QueryContextAttributesNAMES,	    "QuCtxAt(NAMES)"	    },
    { SSPI_CALL_QueryContextAttributesSIZES,	    "QuCtxAt(SIZES)"	    },
    { SSPI_CALL_QueryContextAttributesNATNAMES,	    "QuCtxAt(NATNAMES)"     },
    { SSPI_CALL_QueryContextAttributesLIFESPAN,	    "QuCtxAt(LIFESPAN)"	    },
    { SSPI_CALL_ImpersonateSecurityContext,	    "ImpersSctx()"	    },
    { SSPI_CALL_RevertSecurityContext,		    "RevertSctx()"	    },
    { SSPI_CALL_ExportSecurityContext,		    "ExportSctx()"	    },
    { SSPI_CALL_ImportSecurityContext,		    "ImportSctx()"	    },
    { SSPI_CALL_MakeSignature,			    "MkSig()"		    },
    { SSPI_CALL_VerifySignature,		    "VfySig()"		    },
    { SSPI_CALL_EncryptMessageMIC,		    "EncMsg(MIC)"	    },
    { SSPI_CALL_EncryptMessage,			    "EncMsg()"		    },
    { SSPI_CALL_DecryptMessage,			    "DecMsg()"		    },
    { SSPI_CALL_TranslateNameTOSAMCOMPAT,	    "TrlNa(->SamC)"	    },
    { SSPI_CALL_TranslateNameTOPRINCIPAL,	    "TrlNa(->UPrin)"	    },
    { SSPI_CALL_GetUserNameExSAMCOMPAT,		    "GetUnaEx(SamC)"	    },
    { SSPI_CALL_GetUserNameExPRINCIPAL,		    "GetUnaEx(UPrin)"	    },
    { SSPI_CALL_NONE,				    NULL		    }
};
#endif




/*
 * ntlm_sec_error()
 *
 *
 */
OM_uint32
ntlm_sec_error( SECURITY_STATUS p_sec_status,
		char ** pp_err_symbol, char ** pp_err_description )
{
   struct ntlm_error_txt_s   * p_sec_error;

   if ( pp_err_symbol!=NULL )
      (*pp_err_symbol) = "??";
   if ( pp_err_description!=NULL ) 
      (*pp_err_description) = "??";

   p_sec_error = &(ntlm_error_txt[0]);
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

} /* ntlm_sec_error() */




/*
 * ntlm_timestamp2time()
 *
 * Convert a 64-bit TIMESTAMP structure used by Microsoft's SSPI
 * into a Posix/Ansi-C  time_t value (presumably 32-bit).
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
ntlm_timestamp2time( TimeStamp * p_timestamp )
{
   LARGE_INTEGER  lt1;
   LARGE_INTEGER  lt2;
   time_t         xtime;

   if ( p_timestamp->LowPart==0
        &&  p_timestamp->HighPart==0 )
      return(0);

   if ( p_timestamp->LowPart==0xffffffffu
        &&  p_timestamp->HighPart==0x7fffffff )
      return(GSS_C_INDEFINITE);

   lt1.LowPart  = p_timestamp->LowPart;
   lt1.HighPart = p_timestamp->HighPart;

   /* Adjust 1601 A.D. --> 1970 A.D. */
   lt2.QuadPart = lt1.QuadPart - 116444736000000000 ;

   /* DANGER! this pointer casting is permissible ONLY when            */
   /*         the structures LARGE_INTEGER and FILETIME are compatible */
   /*         (endianess).  LARGE_INTEGER has a stricter alignment, so */
   /*         casting them down to FILETIME should be ok ...           */
   LocalFileTimeToFileTime( (FILETIME *) &lt2, (FILETIME *) &lt1 );

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
    */

   /* 32<<32 / 32-bit -> 32-bit division for the HighPart */
   xtime  = (time_t) MulDiv( lt1.HighPart*4, 0x40000000, 10000000 );

   /* add the result from the LowPart division */
   xtime += lt1.LowPart/10000000;

   return( xtime );

} /* ntlm_timestamp2time() */



/*
 * ntlm_time2timestamp()
 *
 * Convert a Posix/Ansi-C  time_t value (presumably 32-bit on Win32)
 * into a 64-bit TIMESTAMP structure used by Microsoft's SSPI
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
ntlm_time2timestamp( time_t p_time, TimeStamp * p_timestamp )
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

   lt1.QuadPart = Int32x32To64( (OM_uint32)p_time, 10000000 ) + 116444736000000000;

   FileTimeToLocalFileTime( (FILETIME *)&lt1, (FILETIME *)&lt2 );

   p_timestamp->LowPart  = lt2.LowPart;
   p_timestamp->HighPart = lt2.HighPart;

   return;

} /* ntlm_time2timestamp() */




/*
 * ntlm_copy_name()
 *
 *
 */
OM_uint32
ntlm_copy_name( OM_uint32   * pp_min_stat,
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

} /* ntlm_copy_name() */




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


#define MIN_ENTRY(code,desc)     MIN_STRUCT_ENTRY( NTLM_MINOR_ ## code , desc )

static struct num_to_text_s ntlm_errors[] = {
    MIN_ENTRY(NO_ERROR,		    ""					     ),
    MIN_ENTRY(DOMAIN_MISSING,	    "Domainname missing in name"	     ),
    MIN_ENTRY(USER_MISSING,	    "Username missing in name"		     ),
    MIN_ENTRY(BAD_CHARS,	    "Invalid characters in name"	     ),
    MIN_ENTRY(INVALID_WRAP_PADDING, "Invalid message padding in wrap token"  ),
    MIN_ENTRY(UNEXPECTED_SSPI_CTX,  "Unexpected SSPI exported context token" )
};


#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))

/*
 * ntlm_display_minstat()
 *
 *
 */
OM_uint32
ntlm_display_minstat( OM_uint32   * pp_min_stat,
		      OM_uint32     minor_status,
		      char       ** pp_msg,
		      size_t	  * pp_msglen,
		      int	  * pp_need_release )
{
   char            * this_Call  = "ntlm_display_minstat";
   char              tmpbuf[256];
   char		   * esymbol    = NULL;
   char		   * edescr     = NULL;
   char            * ecall_name = "SSPI";
   sspi_call_et      ecall      = SSPI_CALL_NONE;
   SECURITY_STATUS   SecStatus;
   OM_uint32	     maj_stat   = GSS_S_COMPLETE;
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

      case NTLM_CLASS_GENERIC:
            for ( i=0 ; i<ARRAY_SIZE(ntlm_errors) ; i++ ) {
	       if ( ntlm_errors[i].code==min_val ) {
		  sprintf( tmpbuf, /* "%.50s: %.150s", ntlm_errors[i].name, */
			   "%.150s", ntlm_errors[i].desc );
		  break;
	       }
	    }

	    if ( tmpbuf[0]=='\0' ) {
	       DEBUG_ERR((tf, "  E: %s(): Unknown NTLM_MINOR error 0x%08lx\n",
			      this_Call, (long) min_val ))
	       RETURN_MIN_MAJ( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );
	    }

	    break;


      case NTLM_CLASS_ERRNO:
	    sprintf(tmpbuf, "%.150s", strerror(min_val) );
	    if ( !sy_strcasecmp(tmpbuf, "no error") ) {
	       RETURN_MIN_MAJ( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );
	    }
	    break;


      case NTLM_CLASS_LASTERROR:
	    maj_stat = sy_lasterror( pp_min_stat, min_val,
				     tmpbuf, sizeof(tmpbuf)-1 );
	    if (maj_stat!=GSS_S_COMPLETE)
	       goto error;

	    break;


      case NTLM_CLASS_SSPI:
	    SecStatus = NTLM_MINOR_2_SSPI(min_val);
	    for ( i=0 ; ntlm_error_txt[i].ecode!=0 ; i++ ) {
	       if (ntlm_error_txt[i].ecode==SecStatus) {
		  esymbol = ntlm_error_txt[i].esymbol;
		  edescr  = ntlm_error_txt[i].edescription;
		  break;
	       }
	    }
	    ecall = (sspi_call_et) NTLM_MINOR_2_SSPICALL(min_val);
	    for ( i=0 ; sspi_call_name[i].sspi_call_name!=NULL ; i++ ) {
		if ( sspi_call_name[i].sspi_call==ecall ) {
		    ecall_name = sspi_call_name[i].sspi_call_name;
		    break;
		}
	    }

	    if ( esymbol==NULL || edescr==NULL ) {
	        sprintf(tmpbuf, "SSPI::%.40s==Unknown SSPI error 0x%08lx",
			      ecall_name, (long)SecStatus);
	    } else {
	        sprintf(tmpbuf, /* "%.50s: %.150s", ntlm_error_txt[i].esymbol, */
				"SSPI::%.40s==%.150s", ecall_name, edescr );
	    }
	    break;

	    
      default:
	       DEBUG_ERR((tf, "  E: %s(): Unknown minor status 0x%08lx\n",
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

} /* ntlm_display_minstat() */


