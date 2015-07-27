#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/krb5/krb5name.c#4 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/krb5/krb5name.c#4 $
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


/*********************************************************************
 *  These functions parse/provide/define the two nametypes supported
 *  by the GSS-API wrapper to Microsofts Kerberos SSP.
 *
 *  The Kerberos principal name as defined by RFC1964 is supported,
 *  as well as the SAPNTLM Domain name from our GSS-API over NTLM mechanism.
 *
 *  SAPNTLM DomainUsers are represented as   "<domain>\<user>"
 *
 *********************************************************************/




/*********************************************************************
 * krb5_ntlm_compose_name()
 *
 *********************************************************************/
OM_uint32
krb5_ntlm_compose_name(  OM_uint32      *  pp_min_stat,
		         char           *  p_user,
			 size_t            p_user_len,
			 char           *  p_domain,
			 size_t            p_domain_len,
			 Uchar	       **  pp_oname,
			 size_t         *  pp_olen )
{
   Uchar      * ptr;
   size_t	len;
   OM_uint32    maj_stat = GSS_S_COMPLETE;

   (*pp_min_stat) = 0;
   (*pp_oname)    = NULL;
   (*pp_olen)     = 0;

   len  = p_user_len + 1 + p_domain_len;
			      
   /* Allocate dynamic buffer for the name  */
   ptr = (*pp_oname) = sy_malloc( len + 1 );
   if ( ptr==NULL ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   if ( p_user_len>0 ) {
      memcpy(ptr, p_user, p_user_len);
      ptr += p_user_len;
   }

   *(ptr++) = '\\';

   if ( p_domain_len>0 ) {
      memcpy(ptr, p_domain, p_domain_len);
      ptr += p_domain_len;
   }

   *(ptr++) = '\0';

   /* Since NT4 and Windows '95 usually DON'T canonicalize printable names */
   /* we have to do it here, slightly "guessing" the correct rules that    */
   /* LSA uses to pick a matching account name ...                         */
   CharUpper( (*pp_oname) );

   (*pp_olen) = len; /* don't count the trailing NUL character */

   return(maj_stat);

} /* krb5_ntlm_compose_name() */



/*********************************************************************
 * krb5_ntlm_parse_name()
 *
 *  split a name up into it's components (domain + user),
 *  verify that the name is valid (syntax or existence),
 *  and reassemble it.
 *********************************************************************/
OM_uint32
krb5_ntlm_parse_name( OM_uint32      *  pp_min_stat,
		      char           *  p_iname,
		      size_t            p_ilen,
		      Uchar	    **  pp_oname,
		      size_t         *  pp_olen )
{
   char      * domain;
   char      * user;
   size_t      domain_len;
   size_t      user_len;
   char        defuser[KRB5_MAX_NAMELEN];
   char        defdomain[KRB5_MAX_NAMELEN];
   OM_uint32   maj_stat; 

   (*pp_olen)  = 0;
   (*pp_oname) = NULL;

   if (p_ilen==0) { p_ilen = strlen(p_iname); }

   if ( p_ilen>=KRB5_MAX_NAMELEN ) {
      RETURN_MIN_MAJ( MINOR_NAME_TOO_LONG, GSS_S_BAD_NAME );
   }

   maj_stat = krb5_ntlm_split_name( pp_min_stat, p_iname, p_ilen,
				    &user, &user_len, &domain, &domain_len );

   if ( domain==NULL ) {
      /* apparently there is no domain name present, */
      /* so get the default domain name              */
      maj_stat = krb5_ntlm_get_default_name( pp_min_stat, defdomain, sizeof(defdomain),
					     defuser, sizeof(defuser) );
      if (maj_stat!=GSS_S_COMPLETE)
	  return(maj_stat);
      defdomain[sizeof(defdomain)-1] = '\0';
      domain     = defdomain;
      domain_len = strlen(defdomain);

   }

   if ( user==NULL || user[0]=='\0' ) {
      RETURN_MIN_MAJ( KRB5_MINOR(USER_MISSING), GSS_S_BAD_NAME );
   }

   if ( domain==NULL || domain[0]=='\0' ) {
      RETURN_MIN_MAJ( KRB5_MINOR(DOMAIN_MISSING), GSS_S_BAD_NAME );
   }

   /* MISSING !!!:  The given Names for USER and DOMAIN should really
    * be checked at this point for existence via regular Win32 API calls !!!
    */

   maj_stat = krb5_ntlm_compose_name( pp_min_stat,
				      user, user_len,
				      domain, domain_len,
				      pp_oname, pp_olen );

   return(maj_stat);

} /* krb5_ntlm_parse_name() */




/*********************************************************************
 * krb5_parse_name()
 *
 *********************************************************************/
OM_uint32
krb5_parse_name( OM_uint32           * pp_min_stat,
		 Uchar               * p_iname,
		 size_t                p_ilen,
		 Uchar              ** pp_orealm,
		 size_t		     * pp_orealm_len,
		 Uchar              ** pp_ouser,
		 size_t              * pp_ouser_len )
{
   Uchar      * ptr;
   size_t       i;
   char         our_user[KRB5_MAX_NAMELEN];
   char         our_realm[KRB5_MAX_NAMELEN];
   char         tmpbuf[KRB5_MAX_NAMELEN*2];
   char       * user;
   char       * realm;
   size_t       ulen, rlen, unewlen, rnewlen;
   OM_uint32    maj_stat;

   maj_stat        = GSS_S_COMPLETE;
   (*pp_ouser)      = NULL;
   (*pp_ouser_len)  = 0;
   (*pp_orealm)     = NULL;
   (*pp_orealm_len) = 0;
   ptr              = NULL;

   if ( p_iname!=NULL && p_ilen>0 ) {

      /* We were given a non-emtpy name              */
      /* Try to locate a realm part within this name */

      if ( p_iname[0]=='@' ) { /* prohibit empty user name part */
	 ERROR_RETURN( MINOR_NO_ERROR, GSS_S_BAD_NAME );
      }

      /* When searching for the realm seperator, we have to skip escaped '@' chars */
      for ( i=0 ; i<p_ilen ; i++ ) {
	 if ( p_iname[i]=='\\' ) {
	    /* found escaped character, check whether the string ends here */
	    i++; /* skip escaped character, since it is part of the username */

	    if ( i==p_ilen )
	       ERROR_RETURN( KRB5_MINOR(BAD_ESCAPE_SEQUENCE), GSS_S_BAD_NAME );


	 } else if ( p_iname[i]=='@' ) {
	    /* found (non-escaped) realm seperator -- use this realm */
	    ptr = &(p_iname[i]);
	    i++;
	    if ( i==p_ilen )
	       ERROR_RETURN( KRB5_MINOR(REALM_MISSING), GSS_S_BAD_NAME );

	    break;
	 }

      }

   } /* endif name supplied */


   if ( p_iname==NULL  ||  p_ilen==0  ||  ptr==NULL ) {
      /* If either no name was given or the given name lacks a realm part */
      /* then we fill in the missing parts from our own Username          */

      maj_stat = krb5_get_default_name( pp_min_stat, our_realm, sizeof(our_realm),
				        our_user, sizeof(our_user) );
      if ( maj_stat!=GSS_S_COMPLETE )
	 return(maj_stat);

      if ( p_iname==NULL  ||  p_ilen==0 ) {  /* no user name was supplied            */
	 user  = our_user;
	 ulen  = strlen(our_user);
      } else {		      /* user name without realm was supplied */
	 user  = p_iname;
	 ulen  = p_ilen;
      }
      
      realm = our_realm;
      rlen  = strlen(our_realm);

   } else {

      /* we were given a name with user name and realm part  */

      user  = p_iname;
      ulen  = (ptr - p_iname);

      realm = &(ptr[1]);
      rlen  = p_ilen - ulen - 1;

   }

   /* check realm character encoding according to the rules */
   /* of RFC1964 Section 2.1.1 				    */
   ptr = &(tmpbuf[0]);

   for ( i=0 ; i<rlen ; i++ ) {

      if ( realm[i]=='\\' ) {
	 /* This was an escape sequence introducer */
	 i++;
	 if ( i==rlen )
	    ERROR_RETURN( KRB5_MINOR(BAD_ESCAPE_SEQUENCE), GSS_S_BAD_NAME );

	 switch(realm[i]) {
	    case '/':   /* Forward slash */
	    case ':':	/* colon         */
	    case '\0':	/* NUL           */
			ERROR_RETURN( KRB5_MINOR(BAD_CHAR_IN_REALM), GSS_S_BAD_NAME );

	    case '\n':	/* newline -- convert into real escape sequence */
			*(ptr++) = '\\';
			*(ptr++) = 'n';
			break;

	    case '\t':	/* tab     -- convert into real escape sequence */
			*(ptr++) = '\\';
			*(ptr++) = 't';
			break;

	    case '\b':	/* backspace */
			*(ptr++) = '\\';
			*(ptr++) = 'b';
			break;

	    case '@':	/* At	       */
	    case 'n':	/* Newline     */
	    case 'b':	/* Backspace   */
	    case 't':	/* Tab	       */
	    case '\\':	/* Backslash   */
			*(ptr++) = '\\';
			*(ptr++) = realm[i];
			break;

	    default:	/* all other characters don't need escaping -- remove it */
			*(ptr++) = realm[i];
			break;
	 }

      } else { /* realm[i] != '\\' */

	 switch( realm[i] ) {
	    case '/':	/* Forward slash	  */
	    case ':':	/* colon		  */
	    case '\0':	/* NUL			  */
	    case '@':	/* At   (must be escaped) */
			ERROR_RETURN( KRB5_MINOR(BAD_CHAR_IN_REALM), GSS_S_BAD_NAME );

	    case '\n':	/* newline -- convert into real escape sequence */
			*(ptr++) = '\\';
			*(ptr++) = 'n';
			break;

	    case '\t':	/* tab     -- convert into real escape sequence */
			*(ptr++) = '\\';
			*(ptr++) = 't';
			break;

	    case '\b':	/* backspace */
			*(ptr++) = '\\';
			*(ptr++) = 'b';
			break;

	    default:	*(ptr++) = realm[i];
			break;
	 }

      } /* endif (realm[i]=='\\') */

      if ( ptr - tmpbuf >= KRB5_MAX_NAMELEN ) {
	 ERROR_RETURN( MINOR_NAME_TOO_LONG, GSS_S_BAD_NAME );
      }

   }

   *ptr     = '\0'; /* NUL terminate string */
   rnewlen  = ptr - tmpbuf;

   (*pp_orealm) = sy_malloc( rnewlen + 1 );  /* account for trailing NUL */
   if ( (*pp_orealm)==NULL ) {
      ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   memcpy( (*pp_orealm), tmpbuf, rnewlen + 1 ); /* copy with trailing NUL */
   (*pp_orealm_len) = rnewlen;


   /* check username character encoding according to the rules */
   /* of RFC1964 Section 2.1.1 				       */
   ptr = &(tmpbuf[0]);

   for ( i=0 ; i<ulen ; i++ ) {

      if ( user[i]=='\\' ) {
	 /* This was an escape sequence introducer */
	 i++;
	 if ( i==ulen )
	    ERROR_RETURN( KRB5_MINOR(BAD_ESCAPE_SEQUENCE), GSS_S_BAD_NAME );

	 switch(user[i]) {
	    case '\0':	/* NUL     -- convert into real escape sequence */
			*(ptr++) = '\\';
			*(ptr++) = '0';
			break;

	    case '\n':	/* newline -- convert into real escape sequence */
			*(ptr++) = '\\';
			*(ptr++) = 'n';
			break;

	    case '\t':	/* tab     -- convert into real escape sequence */
			*(ptr++) = '\\';
			*(ptr++) = 't';
			break;

	    case '\b':	/* backspace */
			*(ptr++) = '\\';
			*(ptr++) = 'b';
			break;

	    case '/':   /* Escaped Forward slash */
	    case '@':	/* At	       */
	    case 'n':	/* Newline     */
	    case 'b':	/* Backspace   */
	    case 't':	/* Tab	       */
	    case '0':	/* NUL         */
	    case '\\':	/* Backslash   */
			*(ptr++) = '\\';
			*(ptr++) = user[i];
			break;

	    default:	/* all other characters don't need escaping -- remove it */
			*(ptr++) = user[i];
			break;
	 }

      } else { /* (user[i] != '\\') */

	 switch(user[i]) {
	    case '@':	/* Un-escaped @  -- shouldn't happen here */
			ERROR_RETURN( KRB5_MINOR(BAD_CHARS), GSS_S_BAD_NAME );

	    case '\0':	/* NUL     -- convert into real escape sequence */
			*(ptr++) = '\\';
			*(ptr++) = '0';
			break;

	    case '\n':	/* newline -- convert into real escape sequence */
			*(ptr++) = '\\';
			*(ptr++) = 'n';
			break;

	    case '\t':	/* tab     -- convert into real escape sequence */
			*(ptr++) = '\\';
			*(ptr++) = 't';
			break;

	    case '\b':	/* backspace */
			*(ptr++) = '\\';
			*(ptr++) = 'b';
			break;

	    default:	*(ptr++) = user[i];
			break;

	 }

      } /* endif (user[i] != '\\') */

      if ( ptr - tmpbuf >= KRB5_MAX_NAMELEN ) {
	 ERROR_RETURN( MINOR_NAME_TOO_LONG, GSS_S_BAD_NAME );
      }

   }

   *ptr	   = '\0';  /* NUL terminate string */
   unewlen = ptr - tmpbuf;

   (*pp_ouser) = sy_malloc( unewlen + 1 );  /* account for trailing NUL */
   if ( (*pp_ouser)==NULL ) {
      ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   memcpy( (*pp_ouser), tmpbuf, unewlen+1 ); /* copy with trailing NUL */
   (*pp_ouser_len) = unewlen;


   if ( maj_stat!=GSS_S_COMPLETE ) {
error:
	sy_clear_free( (void **) pp_orealm, (*pp_orealm_len) );
	sy_clear_free( (void **) pp_ouser,  (*pp_ouser_len)  );
	(*pp_orealm_len) = 0;
	(*pp_ouser_len)  = 0;
   }

   return(maj_stat);

} /* krb5_parse_name() */




/*********************************************************************
 * krb5_compose_name()
 *
 *********************************************************************/
OM_uint32
krb5_compose_name(   OM_uint32     *  pp_min_stat,
		     char          *  p_realm,
		     char          *  p_user,
		     Uchar         ** pp_oname,
		     size_t        *  pp_olen )
{
   Uchar      * ptr;
   OM_uint32    maj_stat;
   size_t       ulen, rlen, len;

   maj_stat    = GSS_S_COMPLETE;
   (*pp_oname) = NULL;
   (*pp_olen)  = 0;

   ulen = strlen(p_user);
   rlen = strlen(p_realm);
   len  = ulen + 1 + rlen;
			      
   /* Allocate dynamic buffer for the name  */
   ptr = (*pp_oname) = sy_malloc( len + 1 );
   if ( ptr==NULL ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   strcpy( &((*pp_oname)[0]),      p_user  );
   strcpy( &((*pp_oname)[ulen+1]), p_realm );
   (*pp_oname)[ulen] = '@';
   (*pp_olen)        = len;

   return(maj_stat);

} /* krb5_compose_name() */



/*********************************************************************
 * krb5_canonicalize_name()
 *
 *********************************************************************/
OM_uint32
krb5_canonicalize_name( OM_uint32        *  pp_min_stat,
			Uchar            *  p_iname,
			size_t              p_ilen,
			gn_nt_tag_et        p_nt_itag,
			Uchar		 ** pp_oname,
			size_t            * pp_olen,
			gn_nt_tag_et      * pp_nt_otag )
{
   char          * this_Call	 = "krb5_canonicalize_name";
   char          * user		 = NULL;   /* temporary memory */
   char          * realm	 = NULL;   /* temporary memory */
   char		 * tmp_name	 = NULL;   /* temporary memory */
   char		 * tmp_cname     = NULL;   /* temporary memory */
   Uchar         * ptr		 = NULL;
   Uchar	 * ntlm_name	 = NULL;
   Uchar         * fqdn		 = NULL;
   size_t	   ntlm_namelen	 = 0;
   size_t          user_len	 = 0;
   size_t          realm_len	 = 0;
   size_t          tmp_name_len  = 0;
   ULONG	   tmp_cname_len = 0;
   OM_uint32       maj_stat	 = GSS_S_COMPLETE;

   (*pp_min_stat) = 0;
   (*pp_oname)    = NULL;
   (*pp_olen)	  = 0;
   (*pp_nt_otag)  = NT_INVALID_TAG;

   if ( p_nt_itag==NT_ANONYMOUS )  /* anonymous name -- not supported */
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_BAD_NAMETYPE );

   if ( p_iname==NULL  ||  p_ilen==0 ) /* empty name -- not supported */
      RETURN_MIN_MAJ( MINOR_INVALID_BUFFER, GSS_S_BAD_NAME );


   if ( p_nt_itag==NT_EXPORTED_NAME ) {

      DEBUG_STRANGE((tf, "  S: %s(): how does an NT_EXPORTED_NAME get here?\n",
		         this_Call ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );

   } else if ( p_nt_itag==NT_HOSTBASED_SERVICE ) {
      /* The upper layer is taking care of NT_HOSTBASED_SERVICE_X */

      ptr = (Uchar *)strchr(p_iname, '@');
      if ( ptr==NULL ) {
	 /* this shouldn't happen ... the upper layer must verify this */
	 DEBUG_STRANGE((tf, "  S: %s(): how did a hostbased service name without '@' get here?n",
			    this_Call ))
	 RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
      }

      fqdn = ptr+1;

/* Cleanup Alert: the following function call allocates the temporary name in user */
      user = sy_malloc( p_ilen + 1 );
      if ( NULL==user )
	 ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );

      user_len = p_ilen;
      memcpy( user, p_iname, user_len );

      user[user_len]    = '\0';
      user[ptr-p_iname] = '/';

      *pp_oname = user;
      *pp_olen = user_len;
      user = NULL;
   } else if ( NT_USER_NAME==p_nt_itag  ||  p_nt_itag==krb5_nt_tag
	       ||  NT_DEFAULT==p_nt_itag ) {

#if 0
      /* BUGBUG (13-Nov-2003) Although the SSPI spec suggests that this should         */
      /* work unconditionally, experiments on W2Ksp4 show that *ONLY* the translations */
      /* NameUserPrincipal->NameSamCompatible and NameSamCompatible->NameSamCompatible */
      /* returns results, whereas NameSamCompatible->NameUserPrincipal as well as      */
      /* NameUserPrincipal->NameUserPrincipal constantly fail with Lasterror== 1332    */
      /* (ERROR_NONE_MAPPED) when using a Windows2000 Domain Controllers	       */
      /* The same is true for Domain Controllers at Windows2003 rtm                    */
      /*									       */
      /* Update (10-May-2005) When one is logged on with an account from a             */
      /* Windows 2003sp1 Active Directory, then TranslateName() will be able           */
      /* to translate (at least one's own) SamCompatibleName into the default          */
      /* Kerberos principal name with no explicit mapping present in the directory     */
      /* For Windows 2000 Domains, TranlateName() will always fail when no explicit    */
      /* mapping was added to the AD						       */
      /*                                                                               */
      /* Update (16-06-2008) W2K3 SP2 pushes TranslateName() back to the broken state  */
      /* of failing the SamCompatible->NameUserPrincipal translation                   */


       if ( FALSE!=krb5_have_directory ) {
	  BOOLEAN  res;
/* Cleanup Alert: the following function call allocates the temporary name tmp_name */
	  tmp_name_len  = p_ilen;
	  tmp_name      = sy_malloc( tmp_name_len + 1 );
	  if ( NULL==tmp_name )
	     ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
	  memcpy( tmp_name, p_iname, tmp_name_len );
	  tmp_name[tmp_name_len]    = '\0';   /* NUL-Termination of string			     */

/* Cleanup Alert: the following function call allocates the temporary name tmp_cname */
	  tmp_cname_len = KRB5_MAX_NAMELEN*2u;
	  tmp_cname     = sy_malloc( tmp_cname_len + 1 );
	  if ( NULL==tmp_cname )
	     ERROR_RETURN( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );

	  res = (fp_TranslateName)( tmp_name, NameUserPrincipal, NameUserPrincipal, &(tmp_cname[0]), &tmp_cname_len );
	  if ( res!=FALSE ) {
	      /* translation succeeded */
	      tmp_cname[tmp_cname_len] = 0;
	      /**********************************************************/
	      /* SSPI Bogosity alert:                                   */
	      /* different from AcquireCredentialsAttributes(NAMES)     */
	      /* SSPI's TranslateName(NameUserPrincipal) will return    */
	      /* the lowercase DNS domain name instead of the uppercase */
	      /* Kerberos REALM name -- so we'll have to manually       */
	      /* uppercase those Realms here ...                        */
	      /**********************************************************/
	      ptr = strrchr(tmp_cname, '@');
	      if ( ptr!=NULL && ptr[1]!=0 ) {
		 CharUpper(&(ptr[1]));
	      }

	      DEBUG_ACTION((tf, "  A: %s(): Canonicalized by TranslateName: \"%s\"\n", this_Call, tmp_cname ));
	      p_iname = &(tmp_cname[0]);
	      p_ilen  = tmp_cname_len;

	  } else {
	      DEBUG_STRANGE((tf, "  S: %s(): TranslateName(\"%.500s\") failed: %08lx\n",
			         this_Call, tmp_name, GetLastError() ));
	  }

      }
#endif /* disabled */

/* Cleanup Alert: the following function call allocates	 */
/*		  the two strings "user" and "realm"     */				  
      maj_stat = krb5_parse_name( pp_min_stat, (char *)p_iname, p_ilen,
			          &realm, &realm_len, &user, &user_len );
      if (maj_stat==GSS_S_COMPLETE) {
	 maj_stat = krb5_compose_name( pp_min_stat, realm, user,
				       (unsigned char **) pp_oname, pp_olen );
      }

   } else if ( p_nt_itag==krb5_ntlm_nt_tag ) {

      char     *symbol, *desc;
      BOOLEAN  Success;
      DWORD    lasterror;
      char     tmpname[KRB5_MAX_NAMELEN];
      ULONG    tmpname_len;

      /* parse SAPNTLM nametype "Domain\User" */
      maj_stat = krb5_ntlm_parse_name( pp_min_stat, (char *)p_iname, p_ilen, &ntlm_name, &ntlm_namelen );

      if ( maj_stat==GSS_S_COMPLETE ) {

	 tmpname_len = sizeof(tmpname)-1;

	 /* Translate SAM account name into Kerberos 5 principal name                     */
	 /* BUGBUG (13-Nov-2003) Although the SSPI spec suggests that this should         */
	 /* work unconditionally, experiments show that **ONLY** the translation          */
	 /* NameUserPrincipal->NameSamCompatible and NameSamCompatible->NameSamCompatible */
	 /* returns results, whereas NameSamCompatible->NameUserPrincipal as well as      */
	 /* NameUserPrincipal->NameUserPrincipal constantly fail with Lasterror== 1332    */
	 /* (ERROR_NONE_MAPPED)								  */
	 /*										  */
	 /* Update (10-May-2005) When one is logged on with an account from a             */
	 /* Windows 2003sp1 Active Directory, then TranslateName() will be able           */
         /* to translate (at least one's own) SamCompatibleName into the default          */
         /* Kerberos principal name with no explicit mapping present in the directory     */
         /* For Windows 2000 Domains, TranlateName() will always fail when no explicit    */
         /* mapping was added to the AD						          */
	 Success = (fp_TranslateName)( ntlm_name, NameSamCompatible,
				       NameUserPrincipal, &(tmpname[0]), &tmpname_len );

	 if ( Success==FALSE ) {
	    lasterror = GetLastError();
	    krb5_sec_error( lasterror, &symbol, &desc );
	    DEBUG_ERR((tf, "ERR: %s(): TranslateName(NAMES) failed with %s\n\t(Desc=\"%s\")\n",
			   this_Call, symbol, desc ))
	    ERROR_RETURN( KRB5_MINOR_WINERROR(TranslateNameTOPRINCIPAL,lasterror), GSS_S_BAD_NAME );
	 }

	 /* NOTICE: TranslateName() counts the trailing NUL character in the returned */
	 /*         length count, just like several other Win32 API calls ...         */
	 if ( tmpname_len>0 && tmpname[tmpname_len-1]=='\0' ) {
	    tmpname_len--;
	 }
	 /* NOTICE: TranslateName() returns the realm in lower case characters */
	 /*         in order to obtain the correct Kerberos principal name, we */
	 /*         have to convert the realm part to all-uppercase characters */
	 ptr = strrchr(tmpname,'@');
	 if ( ptr!=NULL ) {
	    CharUpper( &(ptr[1]) );
	 }

	 maj_stat = krb5_copy_name( pp_min_stat, tmpname, tmpname_len, pp_oname, pp_olen );

      }

   } else {

      /* all other nametypes are not recognized and therefore invalid */ 
      ERROR_RETURN( MINOR_NO_ERROR, GSS_S_BAD_NAMETYPE );

   }

   if ( maj_stat!=GSS_S_COMPLETE ) {
error:
      sy_clear_free( (void **) pp_oname, *pp_olen );

   } else {
      if ( p_nt_itag==NT_HOSTBASED_SERVICE )
         *pp_nt_otag = p_nt_itag;
      else
         *pp_nt_otag = krb5_nt_tag;
   }

   sy_clear_free( (void **) &user,      user_len      );
   sy_clear_free( (void **) &realm,	realm_len     );
   sy_clear_free( (void **) &ntlm_name, ntlm_namelen  );
   sy_clear_free( (void **) &tmp_name,  tmp_name_len  );
   sy_clear_free( (void **) &tmp_cname, tmp_cname_len );

   return(maj_stat);

} /* krb5_canonicalize_name() */




/*********************************************************************
 * krb5_display_name()
 *
 *********************************************************************/
OM_uint32
krb5_display_name( OM_uint32  * pp_min_stat,   gn_nt_tag_et    p_nt_tag,
		   Uchar      * p_iname,       size_t          p_ilen,
		   char      ** pp_oname,      size_t        * pp_olen )
{
   (*pp_min_stat) = 0;
   (*pp_oname)    = NULL;
   (*pp_olen)     = 0;

   if ( p_nt_tag!=krb5_nt_tag ) {

      /* Only mech-specific (i.e. canonical) names will ever get in */
      /* here.  We reject everything else                           */

      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_BAD_NAME );
   }

   if ( p_ilen==0  ||  p_iname==NULL ) {
      /* This shouldn't happen either ... */
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_BAD_NAME );
   }

   /* Now this is easy for the demo mechanism: the "conversion" from */
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

} /* krb5_display_name() */




/*********************************************************************
 * krb5_import_expname()
 *
 *********************************************************************/
OM_uint32
krb5_import_expname( OM_uint32      * pp_min_stat,
		     Uchar          * p_iname,        size_t     p_ilen,
		     Uchar         ** pp_oname,       size_t   * pp_olen,
		     gn_nt_tag_et   * pp_nt_tag )
{
   OM_uint32     maj_stat;

   (*pp_min_stat) = 0;
   (*pp_oname)    = NULL;
   (*pp_olen)     = 0;
   (*pp_nt_tag)   = NT_INVALID_TAG;

   /* the exported name image for the demo mechanism is simply the */
   /* "internal representation" of our own private nametype ...    */
   /* Let's validate this name through our regualar parser.        */

   maj_stat = krb5_canonicalize_name( pp_min_stat, p_iname, p_ilen,
				      krb5_nt_tag, pp_oname, pp_olen,
				      pp_nt_tag );
   return(maj_stat);

} /* krb5_import_expname() */




/*********************************************************************
 * krb5_inquire_nametypes()
 *
 *  flag all nametypes as valid which we will accept
 *  in krb5_canonicalize_name()
 *
 *********************************************************************/
OM_uint32
krb5_inquire_nametypes( OM_uint32 * pp_min_stat,
			int       * pp_nt_avail )
{
   Uint i;

   for ( i=0 ; i<krb5_nt_count ; i++ ) {
      pp_nt_avail[ krb5_nt_set[i] ] = TRUE;
   }
#if 0
   /* First, flag all recognized generic nametypes that we accept */
   /* (note: STRING_UID_NAMES and MACHINE_UID_NAMES are           */
   /*        handled completely in the generic layer              */
   pp_nt_avail[ NT_EXPORTED_NAME ]     = TRUE;
   pp_nt_avail[ NT_USER_NAME ]         = TRUE;
   pp_nt_avail[ NT_HOSTBASED_SERVICE]  = TRUE;

   /* Then flag all recognized private nametypes that we accept */
   pp_nt_avail[ krb5_nt_tag ]          = TRUE;
   pp_nt_avail[ krb5_ntlm_nt_tag ]     = TRUE;
#endif

   return(GSS_S_COMPLETE);

} /* krb5_inquire_nametypes() */



/***********************************************************************
 * krb5_get_default_name()
 *
 ***********************************************************************/
OM_uint32
krb5_get_default_name( OM_uint32   * pp_min_stat,
		       char        * p_realm,       Uint  p_realm_len,
		       char        * p_user,        Uint  p_user_len )
{
   char      * ptr;
   char        buf[KRB5_MAX_NAMELEN*2];
   OM_uint32   maj_stat = GSS_S_COMPLETE;
   Uint        buf_size, ulen, rlen;

   p_realm[0] = p_user[0] = '\0';
   buf_size   = (Uint)(sizeof(buf)-1);

   maj_stat = krb5_default_name_from_ini_cred( pp_min_stat, buf, KRB5_MAX_NAMELEN );
   if ( maj_stat!=GSS_S_COMPLETE )
      return(maj_stat);

   buf_size = (Uint)strlen(buf);
   ptr = memchr(buf, '@', buf_size);

   if (ptr==NULL) {
      /* This shouldn't be possible, but one never knows ... */
      RETURN_MIN_MAJ( KRB5_MINOR(DOMAIN_MISSING), GSS_S_FAILURE );
   }

   ulen = (Uint)(ptr - buf);
   rlen = buf_size - ulen - 1;

   if ( ulen==0 ) {
      RETURN_MIN_MAJ( KRB5_MINOR(USER_MISSING), GSS_S_FAILURE );
   }

   if ( rlen==0 ) {
      RETURN_MIN_MAJ( KRB5_MINOR(DOMAIN_MISSING), GSS_S_FAILURE );
   }

   if ( ulen>p_user_len  ||  rlen>p_realm_len ) {
      RETURN_MIN_MAJ( MINOR_NAME_TOO_LONG, GSS_S_FAILURE );
   }

   strncpy(p_user, buf, ulen);
   p_user[ulen] = '\0';

   strncpy(p_realm, &(ptr[1]), rlen);
   p_realm[rlen] = '\0';

   return(maj_stat);

} /* krb5_get_default_name() */



/*
 * krb5_ntlm_get_default_name():
 *
 * Hey, this is windows2000 SSPI, we have GetUserNameEx() !
 * No more fuzzing around with Windows 95 shortcomings.
 *
 */

OM_uint32
krb5_ntlm_get_default_name( OM_uint32 * pp_min_stat,
			    char * p_domain,  Uint p_domain_len,
			    char * p_user,    Uint p_user_len )
{
   char		  buf[KRB5_MAX_NAMELEN*2];
   char		* user	    = NULL;
   char	        * domain    = NULL;
   size_t         userlen   = 0;
   size_t	  domainlen = 0;
   ULONG	  buf_size;
   OM_uint32	  maj_stat = GSS_S_COMPLETE;
 
   (*pp_min_stat) = 0;

   p_user[0] = p_domain[0] = '\0';

   buf_size = sizeof(buf)-1;
   /**************************************************/
   /* GetUserNameEx(NameSamCompatible) seems to work */
   /* without Active Directory -- phew!              */
   /**************************************************/
   if ( (fp_GetUserNameEx)(NameSamCompatible, &(buf[0]), &buf_size)==FALSE ) {
      DWORD    lasterror = GetLastError();

      RETURN_MIN_MAJ( KRB5_MINOR_WINERROR( GetUserNameExSAMCOMPAT,lasterror ), GSS_S_FAILURE );
   }

   buf[buf_size] = '\0'; /* Don't trust WinAPI to include the NUL-char */

   maj_stat = krb5_ntlm_split_name( pp_min_stat, buf, buf_size,
				    &user, &userlen, &domain, &domainlen );
   if ( maj_stat==GSS_S_COMPLETE ) {

      if ( userlen >= p_user_len  ||  domainlen >= p_domain_len ) {
	 RETURN_MIN_MAJ(MINOR_NAME_TOO_LONG, GSS_S_FAILURE);
      }
 
      if ( user!=NULL && userlen>0 ) {
	 strncpy(p_user, user, userlen);
	 p_user[userlen] = '\0';
      }

      if ( domain!=NULL && domainlen>0 ) {
	 strncpy(p_domain, domain, domainlen);
	 p_domain[domainlen] = '\0';
      }
   }

   /* I don't think that the Uppercasing of the names is still necessary */
   /* But it shouldn't hurt either ...                                   */
   if ( maj_stat==GSS_S_COMPLETE ) {
      CharUpper( p_user );
      CharUpper( p_domain );
   }

   return(maj_stat);

} /* krb5_ntlm_get_default_name() */




/*
 * krb5_ntlm_split_name()
 *
 *
 */
OM_uint32
krb5_ntlm_split_name( OM_uint32    * pp_min_stat,
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
   user = domain = NULL;

   if ( p_iname!=NULL && p_iname_len>0 ) {

      ptr = memchr( p_iname, '\\', p_iname_len );
      if (ptr==NULL) {
	 /* allow the forward slash as an alternate seperator char */
	 ptr = memchr( p_iname, '/', p_iname_len );
      }
      if ( ptr==NULL ) {

	 user     = p_iname;
	 user_len = p_iname_len;

      } else {

	 domain     = p_iname;
	 domain_len = (ptr - p_iname);
	 user       = &(ptr[1]);
	 user_len   = p_iname_len - 1 - domain_len;

      }
   }

   if ( user_len>0 ) {
      if ( memchr( user, '/', user_len)!=NULL
	   ||  memchr( user, '\\', user_len)!=NULL ) {
	 RETURN_MIN_MAJ( KRB5_MINOR(BAD_CHARS), GSS_S_BAD_NAME );
      }
   }

   if (pp_user      !=NULL)   { (*pp_user)       = user;       }
   if (pp_user_len  !=NULL)   { (*pp_user_len)   = user_len;   }
   if (pp_domain    !=NULL)   { (*pp_domain)     = domain;     }
   if (pp_domain_len!=NULL)   { (*pp_domain_len) = domain_len; }

   return(maj_stat);
   
} /* krb5_ntlm_split_name() */

