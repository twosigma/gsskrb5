#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/ntlm/ntlmname.c#4 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/ntlm/ntlmname.c#4 $
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
 *  These functions parse/provide/define the Domain-User private
 *  nametype for the GSS-API wrapper to Microsofts Lan Manager SSPI.
 *
 *  DomainUsers are represented as   "<domain>\<user>"
 *
 *********************************************************************/

/*********************************************************************
 *  ntlm_default_initiator()
 *
 *********************************************************************/
OM_uint32
ntlm_default_initiator( OM_uint32	*  pp_min_stat,	/* out    */
			Uchar	 	** pp_oname,	/* in/out */
			size_t		*  pp_olen )	/* out    */
{
   OM_uint32   maj_stat;
   char        user[NTLM_MAX_NAMELEN];
   char        domain[NTLM_MAX_NAMELEN];

   (*pp_min_stat) = 0;
   (*pp_oname)    = NULL;
   (*pp_olen)      = 0;

   maj_stat = ntlm_get_default_name( pp_min_stat,
				     domain, sizeof(domain),
				     user, sizeof(user) );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   maj_stat = ntlm_compose_name( pp_min_stat, domain, user, pp_oname, pp_olen );

   return(maj_stat);

} /* gmech_default_initiator() */




/*********************************************************************
 * ntlm_default_acceptor()
 *
 *********************************************************************/
OM_uint32
ntlm_default_acceptor(  OM_uint32	*  pp_min_stat,	/* out    */
			Uchar	 	** pp_oname,	/* in/out */
			size_t		*  pp_olen )	/* out    */
{
   return(ntlm_default_initiator( pp_min_stat, pp_oname, pp_olen ) );

} /* ntlm_default_acceptor() */



/*********************************************************************
 * ntlm_compose_name()
 *
 *********************************************************************/
OM_uint32
ntlm_compose_name(  OM_uint32      *  pp_min_stat,
		    char           *  p_domain,
		    char           *  p_user,
		    Uchar	   ** pp_oname,
		    size_t         *  pp_olen )
{
   Uchar      * ptr;
   OM_uint32    maj_stat;
   size_t       len1, len2, len;

   maj_stat    = GSS_S_COMPLETE;
   (*pp_oname) = NULL;
   (*pp_olen)  = 0;

   len1 = strlen(p_domain);
   len2 = strlen(p_user);
   len  = len1 + 1 + len2;
			      
   /* Allocate dynamic buffer for the name  */
   ptr = (*pp_oname) = sy_malloc( len + 1 );
   if ( ptr==NULL ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   /* Since NT4 and Windows '95 usually DON'T canonicalize printable names */
   /* we have to do it here, slightly "guessing" the correct rules that    */
   /* LSA uses to pick a matching account name ...                         */
   strcpy(ptr, p_domain);
   CharUpper(ptr);

   ptr[len1] = '\\';

   strcpy( &(ptr[len1+1]), p_user);
   CharUpper( &(ptr[len1+1]) );

   ptr[len]  = '\0';

   (*pp_olen) = len; /* don't count the trailing NUL character */

   return(maj_stat);

} /* ntlm_compose_name() */



/*********************************************************************
 * ntlm_parse_name()
 *
 *  split a name up into it's components (domain + user),
 *  verify that the name is valid (syntax or existence),
 *  and reassemble the name back into the <domain>\<username> syntax
 *********************************************************************/
OM_uint32
ntlm_parse_name( OM_uint32       *  pp_min_stat,
		  char           *  p_iname,
		  size_t            p_ilen,
		  Uchar	        **  pp_oname,
		  size_t         *  pp_olen )
{
   char      * domain;
   char      * user;
   char        tmpbuf[NTLM_MAX_NAMELEN+3];
   char        defuser[NTLM_MAX_NAMELEN];
   char        defdomain[NTLM_MAX_NAMELEN];
   int         i;
   OM_uint32   maj_stat; 
   size_t      len;

   (*pp_olen)  = 0;
   (*pp_oname) = NULL;

   domain = user = NULL;
   len = (p_ilen>0) ? p_ilen : strlen( p_iname );
   if ( len>sizeof(tmpbuf)-1 ) {
      RETURN_MIN_MAJ( MINOR_NAME_TOO_LONG, GSS_S_BAD_NAME );
   }

   strncpy(tmpbuf, p_iname, len);
   tmpbuf[len] = '\0';
 
   for ( i=0 ; (unsigned)i<len ; i++ ) {
      if ( tmpbuf[i]==0 ) {
	 /* We will not accept ASCII NULs within a name */
	 RETURN_MIN_MAJ( MINOR_NAME_CONTAINS_NUL, GSS_S_BAD_NAME );
      }
      if ( tmpbuf[i]=='/'  ||  tmpbuf[i]=='\\' ) {
	 if (user!=NULL) {
	    /* if there are extra/additional slashes, then the name is invalid */
	    RETURN_MIN_MAJ( NTLM_MINOR(BAD_CHARS), GSS_S_BAD_NAME );
	 }
	 user = &tmpbuf[i+1];
	 tmpbuf[i] = 0; /* convert the seperator into an End-Of-String,    */
		        /* so that we can pass two NUL-terminated strings  */
		        /* into ntlm_compose_name()                        */
      }
   }

   if ( user==NULL ) {
      /* If we didn't find a seperator, then we treat the given name */
      /* as a standalone user name, not a domain name                */
      user   = &(tmpbuf[0]);

      maj_stat = ntlm_get_default_name( pp_min_stat, defdomain, sizeof(defdomain),
					defuser, sizeof(defuser) );
      if (maj_stat!=GSS_S_COMPLETE)
	  return(maj_stat);

      domain = defdomain;

   } else {

      domain = &(tmpbuf[0]);

   }

   if ( user[0]=='\0' ) {
      RETURN_MIN_MAJ( NTLM_MINOR(USER_MISSING), GSS_S_BAD_NAME );
   }

   if ( domain[0]=='\0' ) {
      RETURN_MIN_MAJ( NTLM_MINOR(DOMAIN_MISSING), GSS_S_BAD_NAME );
   }

   /* MISSING !!!:  The given Names for USER and DOMAIN should really
    * be checked at this point for existence via regular Win32 API calls !!!
    *
    * HOWEVER: If one uses unknown names (especially unknown domains),
    *          the the Windows resolution "Technique" is to send a
    *          query via broadcast and wait for an answer (or a timeout)
    *          Resulting network waits are AWFUL...
    */

   maj_stat = ntlm_compose_name( pp_min_stat, domain, user, pp_oname, pp_olen );

   return(maj_stat);

} /* ntlm_parse_name() */





/*********************************************************************
 * ntlm_canonicalize_name()
 *
 *********************************************************************/
OM_uint32
ntlm_canonicalize_name( OM_uint32        *  pp_min_stat,
			Uchar            *  p_iname,
			size_t              p_ilen,
			gn_nt_tag_et        p_nt_itag,
			Uchar		 ** pp_oname,
			size_t            * pp_olen,
			gn_nt_tag_et      * pp_nt_otag )
{
   char          * this_Call = " ntlm_canonicalize_name(): ";
   unsigned char * iname;
   OM_uint32       maj_stat;
   size_t          ilen;

   (*pp_min_stat) = 0;
   maj_stat       = GSS_S_COMPLETE;
   (*pp_oname)    = NULL;
   (*pp_olen)	  = 0;
   (*pp_nt_otag)  = NT_INVALID_TAG;

   if ( p_nt_itag==NT_ANONYMOUS ) {
      (*pp_nt_otag) = NT_ANONYMOUS;
      return(GSS_S_COMPLETE);
   }

   if ( p_iname==NULL  ||  p_ilen==0 ) {
      RETURN_MIN_MAJ( MINOR_INVALID_BUFFER, GSS_S_BAD_NAME );
   }

   if ( p_nt_itag==NT_EXPORTED_NAME ) {

      DEBUG_STRANGE((tf, "  S:%show does an NT_EXPORTED_NAME get here?\n",
		         this_Call ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );

   } else if ( p_nt_itag==NT_USER_NAME  ||  p_nt_itag==ntlm_nt_tag
	       ||  p_nt_itag==NT_DEFAULT ) {

      /* These nametypes don't need extra special pre-processing */
      ilen  = p_ilen;
      iname = p_iname;

   } else {

      /* all other nametypes are not recognized and therefore invalid */ 
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_BAD_NAMETYPE );

   }
     
   maj_stat = ntlm_parse_name( pp_min_stat,
				(char *)iname, ilen,
				(unsigned char **) pp_oname, pp_olen );
   if ( maj_stat!=GSS_S_COMPLETE ) {
      return(maj_stat);
   }

   (*pp_nt_otag) = ntlm_nt_tag;  /* this is ntlm's canonical nametype */
                                 /* (well, except for anonymous names) */

   return(GSS_S_COMPLETE);

} /* ntlm_canonicalize_name() */




/*********************************************************************
 * ntlm_display_name()
 *
 *********************************************************************/
OM_uint32
ntlm_display_name( OM_uint32  * pp_min_stat,   gn_nt_tag_et    p_nt_tag,
		   Uchar      * p_iname,       size_t          p_ilen,
		   char      ** pp_oname,      size_t        * pp_olen )
{
   (*pp_min_stat) = 0;
   (*pp_oname)    = NULL;
   (*pp_olen)     = 0;

   if ( p_nt_tag!=ntlm_nt_tag ) {

      /* Only mech-specific (i.e. canonical) names will ever get in */
      /* here.  We reject everything else                           */

      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_BAD_NAME );
   }

   if ( p_ilen==0  ||  p_iname==NULL ) {
      /* This shouldn't happen either ... */
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_BAD_NAME );
   }

   /* Now this is easy for the SAPNTLM mechanism: the "conversion" from */
   /* our private internal name representation into a printable name    */
   /* is a "memcpy()"   :-)						*/
   (*pp_oname) = sy_malloc( p_ilen + 1);
   if ( (*pp_oname)==NULL ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   memcpy( (*pp_oname), p_iname, p_ilen );
   
   (*pp_oname)[p_ilen] = '\0'; 
   (*pp_olen)          = p_ilen;

   return(GSS_S_COMPLETE);

} /* ntlm_display_name() */




/*********************************************************************
 * ntlm_import_expname()
 *
 *********************************************************************/
OM_uint32
ntlm_import_expname( OM_uint32      * pp_min_stat,
		     Uchar          * p_iname,        size_t     p_ilen,
		     Uchar         ** pp_oname,       size_t   * pp_olen,
		     gn_nt_tag_et   * pp_nt_tag )
{
   OM_uint32     maj_stat;

   (*pp_min_stat) = 0;
   (*pp_oname)    = NULL;
   (*pp_olen)     = 0;
   (*pp_nt_tag)   = NT_INVALID_TAG;

   /* the exported name image for the SAPNTLM mechanism is simply   */
   /* the "internal representation" of our own private nametype ... */
   /* Let's validate this name through our regualar parser.         */

   maj_stat = ntlm_canonicalize_name( pp_min_stat, p_iname, p_ilen,
				      ntlm_nt_tag, pp_oname, pp_olen,
				      pp_nt_tag );
   return(maj_stat);

} /* ntlm_import_expname() */




/*********************************************************************
 * ntlm_inquire_nametypes()
 *
 *  flag all nametypes as valid which we will accept
 *  in ntlm_canonicalize_name()
 *
 *********************************************************************/
OM_uint32
ntlm_inquire_nametypes( OM_uint32 * pp_min_stat,
			int       * pp_nt_avail )
{
   /* First, flag all recognized generic nametypes that we accept */
   /* (note: STRING_UID_NAMES and MACHINE_UID_NAMES are           */
   /*        handled completely in the generic layer              */
   pp_nt_avail[ NT_EXPORTED_NAME ]     = TRUE;
   pp_nt_avail[ NT_USER_NAME ]         = TRUE;

   /* Then flag all recognized private nametypes that we accept */
   pp_nt_avail[ ntlm_nt_tag ]          = TRUE;

   return(GSS_S_COMPLETE);

} /* ntlm_inquire_nametypes() */




/*
 * ntlm_get_default_name():
 *
 * Find out the Name and Domain of the Current User
 * Microsoft Knowledgebase, PSS ID Number: Q111544,  ShowUserDomain()
 *
 * Unfortunately this only works on Windows NT.
 *
 * Win '95 provides a GetUserName() but lacks a call to return
 * the Domain name (This is so stupid).  We use the Microsoft-proposed
 * workaround and use the domain name from the registry.
 * (see ntlmmain.c:ntlm_load_dll(), where alt_domain is filled)
 * 
 * (21-Oct-2001) mrex
 * Apparently Q111544 was buggy (missing CloseHandle()) and doesn't
 * work in all situations -- prefer OpenThreadToken() to OpenProcessToken()
 * but keep the fallback
 *
 */

#include <winbase.h>

OM_uint32
ntlm_get_default_name( OM_uint32 * pp_min_stat,
		       char * p_domain,  Uint p_domain_len,
		       char * p_user,    Uint p_user_len )
{
    HANDLE        hProcess	      = NULL; /* read-only reference */
    HANDLE        hThread             = NULL; /* read-only reference */
    HANDLE	  hAccessToken	      = NULL; /* dynamic temporary handle, needs CloseHandle() */
    UCHAR         InfoBuffer[1000];
    PTOKEN_USER   pTokenUser = (PTOKEN_USER)InfoBuffer;
    DWORD         dwInfoBufferSize;
    DWORD         dwAccountSize       = p_user_len;
    DWORD         dwDomainSize        = p_domain_len;
    SID_NAME_USE  snu;
    BOOL          rval;
    OM_uint32     maj_stat = GSS_S_COMPLETE;
 
    p_user[0] = p_domain[0] = '\0';

    if ( ntlm_platform_id==VER_PLATFORM_WIN32_WINDOWS ) {
       /* Hack for Windows '95 ... */
       if ( strlen(alt_domain)>= p_domain_len ) {
	  RETURN_MIN_MAJ( MINOR_NAME_TOO_LONG, GSS_S_FAILURE );
       }
       strcpy(p_domain, alt_domain);
       if ( GetUserName( &(p_user[0]), &dwAccountSize )!=0 ) {

	  p_user[dwAccountSize] = '\0';

       } else {

	  p_user[0]      = p_domain[0] = '\0';
	  (*pp_min_stat) = NTLM_MINOR_LASTERROR( GetLastError() );
	  maj_stat       = GSS_S_FAILURE;

       }

    } else {  /* Assume that we're on Windows NT ! */

       hThread  = GetCurrentThread(); /* readonly-reference */
       rval     = OpenThreadToken( hThread, TOKEN_QUERY, TRUE, &hAccessToken );

       if ( rval==0 ) {
	   /* when OpenThreadToken() fails we fallback on OpenProcessToken() */
	   hProcess = GetCurrentProcess(); /* readonly-reference */
	   rval     = OpenProcessToken( hProcess, TOKEN_QUERY, &hAccessToken );
       }

       if ( rval!=0) {
           rval = GetTokenInformation( hAccessToken,TokenUser,InfoBuffer,
				       sizeof(InfoBuffer), &dwInfoBufferSize );
	   if ( rval!=0 && dwInfoBufferSize>0 ) {
	       rval = LookupAccountSid( NULL, pTokenUser->User.Sid,
				        p_user, &dwAccountSize,
				        p_domain,  &dwDomainSize, &snu);
	   }
	   /* Apparently we need to close this handle  ... would someone    */
	   /* please slap Microsoft for *NOT* documenting this for at least */
	   /* 5 years after shipping Windows NT (it is definitely not       */
	   /* documented up to and including MSDN Visual Studio 6.0)	    */
	   CloseHandle(hAccessToken);
	   hAccessToken = NULL;
       }

       if ( rval==0 ) {
	  p_user[0]      = p_domain[0] = '\0';
	  (*pp_min_stat) = NTLM_MINOR_LASTERROR( GetLastError() );
	  maj_stat       = GSS_S_FAILURE;
       }

    }

    if ( maj_stat==GSS_S_COMPLETE ) {
       CharUpper( p_user );
       CharUpper( p_domain );
    }

    return(maj_stat);

} /* ntlm_get_default_name() */




/*
 * ntlm_split_cname()
 *
 *
 */
OM_uint32
ntlm_split_cname( OM_uint32    * pp_min_stat,
		  Uchar        * p_iname,
		  size_t         p_iname_len,
		  Uchar       ** pp_user,
		  size_t       * pp_user_len,
		  Uchar       ** pp_domain,
		  size_t       * pp_domain_len )
{
   Uchar       * user;
   Uchar       * domain;
   Uchar       * ptr;
   size_t        user_len   = 0;
   size_t        domain_len = 0;
   OM_uint32     maj_stat   = GSS_S_COMPLETE;

   (*pp_min_stat) = 0;
   user = domain = "";

   if ( p_iname!=NULL && p_iname_len>0 ) {
      if ( p_iname[0]!='\\' ) {
	 user     = p_iname;
	 user_len = p_iname_len;
      }

      for ( ptr = p_iname ; p_iname_len>0 ; ptr++ , p_iname_len-- ) {
	 if ( *ptr=='\\' && p_iname_len>1 ) {
	    domain     = p_iname;
	    domain_len = ptr - user;
	    user       = &(ptr[1]);
	    user_len   = p_iname_len - 1;
	    break;
	 }
      }
   }

   if (pp_user      !=NULL)   { *pp_user       = user;       }
   if (pp_user_len  !=NULL)   { *pp_user_len   = user_len;   }
   if (pp_domain    !=NULL)   { *pp_domain     = domain;     }
   if (pp_domain_len!=NULL)   { *pp_domain_len = domain_len; }

   return(maj_stat);
   
} /* ntlm_split_cname() */

