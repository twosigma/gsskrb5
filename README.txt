
This is the Two Sigma Open Source fork of SAP's GSS->SSPI bridge, which
can be found at:

https://www.kerberos.org/software/samples/gsskrb5/



Release notes for gsskrb5 v1.09

 17-Jun-2008  Martin Rex  <Martin.Rex@sap.com>

 ************************************************************************
 *
 * Copyright (c) 1997-2008  SAP AG.  All rights reserved.
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
 ************************************************************************

1.) Project "Homepage" (Home/Download Location)
===============================================
    The most recent version of *MY* gsskrb5 (remember, it's Open Source)
    is available for download via FTP from the following directory:

    ftp://ftp.sap.com/pub/ietf-work/gssapi/gsskrb5/


2.) Status of this code:
========================
This code has release status.  I have tested only those features that we
at SAP require for our application (which are many but not all of the
GSS-API v2 features), but it seems to work OK for all *our* purposes
(well, except that Microsoft keeps adding bugs to their Kerberos and
 require me to add workarounds to gsskrb5.dll for Microsoft Windows
 Service packs or new OS Releases...)

Since the code was originally written with the whole spec in mind,
there are features that I have implemented but not really tested,
and there are a few shortcuts and ommissions in a few places that
might need to be aligned before some of the untested, unfinished or
missing features may actually work.

See the section "Tested functionality and implementation notes".

I have used Microsoft Visual Studio 98 (aka VC6) to develop this
code and I still use VS98.  The 64-bit version require the
Windows 2003 SDK (sp1 or sp2), which has been available for
free download (with both IA64 and AMD64 cross-Compilers).

I have made a few tweaks so that this code should compile with
Visual Studio 2005, but I **STRONGLY** recommend using VS98,
because VS98 is so much less hazzle and so much more reliable
and you don't need the braindead VCREDIST.EXE libc-variant
of the day, you can just copy the DLL **alone** to every
Win32-Platform (Win9x and NT4 included).


3.) Purpose of of this code:
============================
The gsskrb5 v1.09 codebase can be used to build
two different DLLs implementing two independent GSSAPI mechanisms:

1)  gsskrb5.dll  which implements an RFC-1964 compatible
	         GSS-API v2 mechanism on top of the Kerberos SSP
		 of Microsoft Windows 2000.

		 It supports secure mutual authentication and
		 single-DES message protection as specified in
		 RFC-1964.  It is interoperable with MIT Kerberos 5.
				 
2)  gssntlm.dll  which implements a simple GSS-API v2 mechanism
		 using the NTLM SSP of Windows 9x and Windows NT/2000.
		 This is a "proprietary" gssapi-style framing of NTLM
		 by SAP. I have no formal spec for it.

		 NTLM is a simple target-only challenge-response
		 authentication.  When accepting contexts on Win 9x,
		 an insecure pass-through authentication request to
		 an NT machine is performed.

		 As it is configured, the mechanism supports no
		 message protection (although implemented, the message
		 integrity protection of NTLMv1 is insecure, broken
		 and incompatible with context export/import).

3)  gssboth.dll  (I haven't tried that for quite some time now ...)
		 which is a multi-mechanism containing both
		 the Kerberos 5 and the SAP NTLM SSP gssapi
		 mechanisms in a single DLL.

		 NOTICE: This multimechanism is only very
		 marginally tested.  Some things may not
		 work (or work different than they should).


4)  gss-sample   is the OpenVision donated gss-sample application
                 taken from the MIT Kerberos5 v1.0.5 source tree
		 and modified to work on Win32 platforms with
		 this GSSAPI wrapper for the W2K Kerberos SSP.

	NOTE!	 Getting host-based service names to work
		 on Windows 2000 is a little tricky, and
		 as W2K doesn't support MIT-style keytabs,
		 hostbased service names do *NOT* work with
		 non-Microsoft KDCs.
		 When using a Microsoft KDC (i.e. a W2K-Domain)
                 then hostbased service names are not regular
                 principals, but aliases for regular W2K-Accounts.
                 The Microsoft KDC can cheat when issuing tickets
                 and stuff the hostbased service name alias into
                 tickets to make traditional Kerberos clients happy.

4.) Additional Information for Microsoft W2K Kerberos Interoperability
======================================================================

There's a Step-by-Step Guide to Kerberos 5 (krb5 1.0) Interoperability
available from Microsoft.  Go to http://www.microsoft.com and
search for "ksetup".

KSETUP.EXE along with KTPASS.EXE are two command line tools that are
shipped on W2K CDs in the archive-file \SUPPORT\TOOLS\SUPPORT.CAB

Keep in mind that changes to the Kerberos configuration with
KSETUP.EXE usually require a reboot to take effect.
as in
"Your mouse pointer has moved,
 please reboot for changes to take effect".



5.) Tested functionality
========================

I'm simply lacking the time to list each and every feature here.
I have build a gssapi test program that tests all the functionality
our application needs.  I have planned to make this program
generally available (removing all the SAP-specific tests and
complaints), however I still need some more time.

The current SAP R/3 interoperability test tool is available
through our public FTP-Server from the following directory:

ftp://ftp.sap.com/pub/ietf-work/gssapi/gsstest/

gsstest-1.26.zip                         381 KB     Sep 03 20:24
gsstest-1.26.zip.asc                     284 bytes  Sep 03 20:24


It is distributed as source, but also includes
a Win32 Binary of gsstest.exe and a gssntlm.dll.




6.) Missing and untested GSS-API features
=========================================

This list is from a quick scan of my memories, and incomplete
and outdated...  :-o


name handling:
--------------

  (hostbased) service names:  implemented -- however WATCHOUT, they're INSECURE!!

	 rfc1964 lists hostbased service names but fails to define how
	 these should be transformed into a Kerberos principal name.

	 There are 4 issues with resolving "service@hostname":

	   (1) how service names are canonicalized/transformed
	   (2) how hostnames are canonicalized/transformed
	   (3) how a Kerberos REALM is determined
	   (4) how a Kerberos principal name is created from above components.


	 This implementation performs the following:
	   (1) service names are copied literally
	   (2) hostnames are canonicalized via INSECURE DNS and lowercased,
	       as it is proposed in rfc2743, section 4.1 and as it is done
		   by MIT's Kerberos 5 v1.0.x and v1.1.x implementations.
	   (3) The realm is determined with a similar strategy as described for
	       the [domain_realm] section of krb5.conf in MIT Kerberos 5 v1.1.
		   However the [domain_realm] information is actually parsed
		   from the registry and not from a file.  The registry key is
	   "HKLM\SYSTEM\CurrentControlSet\Control\Lsa\Kerberos\HostToRealm"
	 	   I don't know whether this has been officially documented
                   yet, but this location and its interpretation was proposed
                   by Microsoft's Kerberos development team during Email
                   disucussion.
		
	   (4) The resulting Kerberos principal name is formed from the
	       components as "service/f.q.d.n@SOME-REALM"

	 Additional Notes for (3):
	   You can supply "domain->realm" mappings as well as "host->realm"
	   mappings; a leading dot "." will indicate that it is a DNS-domain.
	   domain and hostname matching is performed case-insensitive.
	   When searching through the values, the longest match will
	   be used (Remind that registry values under a key are unordered).

	   The realm name will be copied literally, it will not be uppercased!
	   The general Kerberos convention is to use uppercase Realms, however
	   some people are using lower or mixed case realms and therefore this
	   wrapper uses the case as specified in the mapping table (registry).

	   If there is neither a matching domain nor a matching host mapping
	   defined, the *uppercased* DNS domain name of the f.q.d.n will
	   be used as the realm name.


  Unicode User names: missing

     rfc1964 doesn't define how to deal with non-ASCII usernames, therefore
	 I didn't implement them either.  When a spec becomes available how to deal
	 with Unicode Kerberos names, then I will probably update this code.


credential management:
----------------------

  credential "verification":

     This wrapper attempts to verify whether credentials can really be acquired.
	 Since AcquireCredentialsHandle() for the Kerberos SSP does not perform
	 any checks, the wrapper will try to establish a security context with
	 itself to find out whether credentials for a given name can really
	 be acquired.
	 BEWARE: this causes an overhead of a few milliseconds and requires
	 the acquisition of a ticket, even for the acceptor.  If you don't
	 like this, you can change krb5/krb5cred.c:318 from "#if 1" to "#if 0"
	 before compiling this code.


  server credentials (server keys)  INCOMPLETE/MISSING

     Accepting security contexts with a User's credentials work fine.
         However, the client must use the Users account name as target name!
         Although it is possible to define a mapping of a service-principal name
         (like "service/f.q.d.n") to an W2K account with KSETUP, this service
         name can only be used as a target name when an Active Directory
	 (i.e. an NT5 Domain) is present.  It will *NOT* work with a
         standalone W2K pro Workstation attached to an MIT-style KDC
	 (even though it can be defined).

     Machine ("host") server credentials (accepting credentials)
         in general don't seem to work with W2K Kerberos.  Although each W2K
	 machine has a "host" service account and host-based credentials for
	 this service, these credentials are only available for processes
	 running as "LocalSystem".  I haven't yet bothered to make this
	 work.
         
     W2K doesn't have keytabs, and the Feb'2000 release of W2K doesn't allow
	 to import keys as they're stored in a MIT-Kerberos style keytab.
	 In contrast to MIT Kerberos, the credentials of an interactive user
	 session can be used to accept security contexts because the long
	 term secret (=password) of the user is cached on Microsoft Windows
	 Platforms.

     Default accepting credentials under W2K differ from MIT Kerberos.
	 With W2K, default accepting credentials are always the credentials
	 of the current thread, usually for the logged-on user.
	 With MIT Kerberos, the default accepting credentials always resolve
	 to "host/f.q.d.n".  However credentials for this identity are normally
	 not accessible by non-root users.
         

  gss_add_cred() is missing. (You shouldn't be using it anyway.)

     This GSS-API v2 call is the only call that I consider broken/mis-spec'd.
	 However it was to late to change the spec when I realized how broken it is.
	 Anyway -- I have not implemented this call, and I really don't feel like
	 ever doing it.


  GSS_C_BOTH type credentials are not supported.

     Only GSS_C_INITIATE and GSS_C_ACCEPT are implemented; an application should
	 always know which kind of credentials it needs for context establishment.


  Credential delegation (TGT forwarding)  acceptor side is UNTESTED!!

	 The initiator side of credential delegation should be working.  However
	 for W2K to permit credential delegation, the target may need a "clearance".
	 Individual service principals in Microsoft's active directory may need to be
	 configured with "trusted for delegation" to make this happen.
	 It is possible to allow delegation to all principals of a REALM
	 by adding "RealmFlags = REG_DWORD 4" to the Registry under the key
	 "HKLM\SYSTEM\CurrentControlSet\Control\LSA\Kerberos\Domains\YOUR-REALM"
	 According to Microsoft, information on this is available in the
	 ResKit for W2K online docs:
	 Start->Programs->Windows 2000 Resource Kit->
	                                     Documentation->Registry Reference

	 I have added some code (bending the internal layer seperation) for the
	 acceptor side of credential delegation which should be able to return a handle
         for a delegated credential.  However I have never used credential delegation
         and not tested this functionality at all -- good luck!


context establishment:
----------------------

  Channel Bindings are not implemented in Microsoft W2K Kerberos, therefore
     they're not available through this wrapper.  You will get an error returned
     when you supply a channel bindings parameter to gss_init_sec_context() or
     gss_accept_sec_context().  (Maybe this feature is added with a later
     W2K service pack...)

  I have never tested target-only authentication (without the GSS_C_MUTUAL_FLAG).
     The underlying W2K Kerberos SSP seems to support it.


  GSS_C_PROT_READY_FLAG: Currently this code doesn't return it at all,
     and I haven't checked whether/how the W2K Kerberos SSP supports it
     (i.e. message protection availability prior to full context
      establishment).


  Sequence/Replay detection:  The sequence and replay detection implemented
     by the W2K Kerberos SSP is fairly simple, it uses a sequence number for
     each direction.  Therefore, if data replay detection is requested,
     data sequencing will be additionally provided/enforced.


  This code does not transfer KERB_ERROR tokens.  Although such tokens are
     allowed by the rfc1964 spec and produced by the W2K Kerberos SSP,
     this code will at most emit two context level tokens for a context
     establishment handshake with mutual authentication.

  When using an invalid target name, W2K Kerberos seems to return a KERB_ERROR
     token from AcceptSecurityContext() while MIT Kerberos will recognize
     the wrong target name immediately and abort with an error like
     "wrong principal in request".  If one would let W2K Kerberos carry on the
     token exchange, it would make two full roundtrips, returning the same
     KERB_ERROR message twice before the initiator aborts with the correct
     error message.  I personally strongly prefer the MIT Kerberos behaviour
     and my wrapper will currently abort when AcceptSecurityContext() returns
     CONTINUE_NEEDED with a minor_status that gss_display_status() visualizes
     as "SSPI talks garbage (maybe wrong target name?)"

  Microsoft's proprietary user2user 3-token security context establishment
     is not currently supported by gsskrb5.dll
     It can be requested with the ISC_REQ_USE_SESSION_KEY context attribute,
     it has been available in Windows2000 Kerberos already and it is
     "enforced" for plain user accounts in Windows 2003 domains when running
     at "windows 2003" domain functional level (instead of "Windows 2000 mixed")

     New error codes&text have been added to specifically identify the
     situation when Microsoft's Kerberos SSP tries to perform a user2user
     token exchange which gsskrb5.dll does not (currently) support.

     If a service principal is defined for a (service) account, then the
     regular 2-token authentication exchange from rfc-1964/rfc-4121
     can be used.  The command line tool "setspn" from the SUPPORT.CAB
     on the OS installation CD can be used to flip the necessary bit
     in the Microsoft Active Directory to re-enable rfc-1964 conforming
     2-token authentication exchanges, e.g.

	    SETSPN -A SAPServiceC11/dontcare  NT4DOMAIN\SAPServiceC11

     to enable it for account "NT4DOMAIN\SAPServiceC11"
     It really doesn't matter what the service principal looks like,
     -- it is not being used by SAP, because it only works for accepting
     security contexts, not for initiating security context--this is
     because Service Principals are not real Kerberos principals in
     Microsoft Kerberos, instead they're only aliases to real accounts.

     Microsoft seems to have shipped a few defective Versions of SETSPN.exe
     that fail to parse the NT4DOMAIN\SAPServiceC11 account syntax,
     these seem to accept only plain account names in the current domain...

