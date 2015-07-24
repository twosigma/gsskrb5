/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/version.h#7 $
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

#define STRING(x)  # x
#define XSTRING(x) STRING(x)

#define LIBRARY_COPYRIGHT     "Copyright © 1997-2013, SAP AG"

#define KRB5_VERS_MAJOR     1
#define KRB5_VERS_MINOR     0
#define KRB5_VERS_PATCH    12

#define KRB5_LIBRARY_NAME     "Kerberos 5 GSS-API for Microsoft W2K+ Kerberos SSP"
#define KRB5_LIBRARY_VERSION  "Version " XSTRING(KRB5_VERS_MAJOR) "." XSTRING(KRB5_VERS_MINOR) "." XSTRING(KRB5_VERS_PATCH)



#define NTLM_VERS_MAJOR    1
#define NTLM_VERS_MINOR    0
#define NTLM_VERS_PATCH    8

#define NTLM_LIBRARY_NAME     "SAP NTLM GSS-API mechanism for Microsoft Win32 NTLM SSP"
#define NTLM_LIBRARY_VERSION  "Version " XSTRING(NTLM_VERS_MAJOR) "." XSTRING(NTLM_VERS_MINOR) "." XSTRING(NTLM_VERS_PATCH)

extern char * library_name;
extern char * library_version;
extern char * library_copyright;
