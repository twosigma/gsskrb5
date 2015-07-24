#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/sy_shlib.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/sy_shlib.c#1 $
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



#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>
#include <time.h>
#include <ctype.h>

#include "platform.h"

#if defined(_WINDOWS) || defined(_WIN32)
#  include <windows.h>
#endif

 
#if defined HAVE_WINSOCK
#    /* <winsock.h> for gethostname(), gethostbyname(), etc. */
#    include <winsock.h>
#endif

#include "gssmaini.h"





#ifdef _WIN32

  /* Microsoft Windows 32bit DLL entry point */
  BOOL APIENTRY
  DllMain(HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
  {
     switch(ul_reason_being_called) {

         case DLL_PROCESS_ATTACH:
#  if defined(NEED_WSASTARTUP)
	        if ( WSA_usecntr==0 ) {
	            WORD wVersionRequested;
		    WSADATA wsaData;
	            int err;

		    wVersionRequested = MAKEWORD( 1, 1 );
		    err = WSAStartup( wVersionRequested, &wsaData );
		    if (err!=0)
			   return(0);

		}

		WSA_usecntr++;
#  endif /* NEED_WSASTARTUP */
		{
		   /* Although it would be best to do the initialization here,    */
		   /* M$ documentation strongly discourages to load and use other */
		   /* DLLs during DllMain() because the DllMains() of those may   */
		   /* not have been called yet ...                                */

		   /* OM_uint32  minor_status; */

		   /* gn_init( &minor_status ); */

		   /* Failing here would terminate the application   */
		   /* which loaded this library -- let's fail at the */
		   /* gssapi level instead			     */
		}
		break;


         case DLL_PROCESS_DETACH:
		gn_cleanup();
#  if defined(NEED_WSASTARTUP)
	        if ( WSA_usecntr>0 ) {
		    WSA_usecntr--;
		    if ( WSA_usecntr==0 )
			WSACleanup();
		}
#  endif /* NEED_WSASTARTUP */
		break;
			
    
         default:
		break;

     }

     return 1;

     UNREFERENCED_PARAMETER(hInst);
     UNREFERENCED_PARAMETER(lpReserved);

  } /* DllMain() */


#else /* !_WIN32 */

#  if defined(_WINDOWS)

	BOOL CALLBACK
	LibMain (HINSTANCE hInst, WORD wDataSeg, WORD cbHeap, LPSTR CmdLine)
	{
#    if defined(NEED_WSASTARTUP)
           WORD wVersionRequested;
	   WSADATA wsaData;
	   int err;

	   wVersionRequested = MAKEWORD( 1, 1 );
	   err = WSAStartup( wVersionRequested, &wsaData );
	   if (err!=0)
	      return(0);

	   WSA_usecntr = 1;

#    endif /* NEED_WSASTARTUP */
	   UNREFERENCED_PARAMETER(hInst);
	   UNREFERENCED_PARAMETER(wDataSeg);
	   UNREFERENCED_PARAMETER(cbHeap);
	   UNREFERENCED_PARAMETER(CmdLine);

	   {
	       OM_uint32  minor_status;

	       gn_init( &minor_status );
	       /* Failing here would terminate the application   */
	       /* which loaded this library -- let's fail at the */
	       /* gssapi level instead			         */
	   }
	   
	   return 1;

	} /* LibMain() */


	int CALLBACK __export
	WEP(int nParam)
	{
	   gn_cleanup();

#    if defined(NEED_WSASTARTUP)
	   if (WSA_usecntr>0) {
	      WSACleanup();
	      WSA_usecntr = 0;
	   }
#    endif /* NEED_WSASTARTUP */

	   UNREFERENCED_PARAMETER(nParam);

	   return 1;
	} /* WEP() */

#  endif /* _WINDOWS */

#endif /* !_WIN32 */



