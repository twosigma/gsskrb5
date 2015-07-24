/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/platform.h#1 $
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


/* try to recognize platforms ... */

#if defined(_WINDOWS) || defined(_WIN32)

#  define LRTIMER_WIN_FTIME	 1
#  define HRTIMER_WIN32_QueryPerformanceCounter  1
#  define HAVE_FTIME
#  define HAVE_SOCKETS
#  define HAVE_WINSOCK
#  define NEED_WSASTARTUP
#  define SHLIB_EXTENSION ".dll"
#  define sleep(x)   Sleep( (x)*1000 );

#elif defined(_AIX)

   /* AIX 3.2.x or 4.x.x */
#  define LRTIMER_GETTIMEOFDAY	 1
#  define HRTIMER_GETTIMEOFDAY	 1
#  define UNIX
#  define HAVE_DYNLINK_AIX_LOAD
#  define SHLIB_EXTENSION ".o"

#elif defined(__osf__) && defined(__alpha)

   /* Digital UNIX (aka DEC OSF/1) on Alpha */
#  define LRTIMER_GETTIMEOFDAY	 1
#  define HRTIMER_OSFALPHA_PCC   1
#  define UNIX
#  define HAVE_DYNLINK_DLOPEN
#  define SHLIB_EXTENSION ".so"

#elif defined(__sun) && defined(__unix) && defined(__SVR4)

   /* SUN Solaris 2.x */
#  define LRTIMER_GETTIMEOFDAY	 1
#  define HRTIMER_GETTIMEOFDAY	 1
#  define UNIX
#  define HAVE_DYNLINK_DLOPEN
#  define SHLIB_EXTENSION ".so"

#elif defined(linux)

   /* Linux  any.any */
#  define LRTIMER_GETTIMEOFDAY	 1
#  define HRTIMER_GETTIMEOFDAY	 1
#  define UNIX
#  define HAVE_DYNLINK_DLOPEN
#  define SHLIB_EXTENSION ".so"

#elif (defined(unix) || defined(__SNI_TARG_UNIX)) && defined(sinix)

   /* Siemens-Nixdorf SVR4  5.43 */
#  define UNIX
#  define HAVE_DYNLINK_DLOPEN
#  define SHLIB_EXTENSION ".so"

#elif defined(__hpux) || defined(hpux)

  /* HP-UX 9.x and 10.x					        */
  /* should work for PA-Risc   9000/700 and 9000/800  machines  */
#  define LRTIMER_GETTIMEOFDAY	 1
#  define HRTIMER_GETTIMEOFDAY	 1
#  define UNIX
#  define HAVE_DYNLINK_HPUX_SHLOAD
#  define SHLIB_EXTENSION ".sl"

#else

#  error This Platform is not recognized (yet).  Please insert your definitions into this tree

#endif



/* common Unix definitions ... */
#if defined(unix) || defined(UNIX)

#  define HAVE_GETTIMEOFDAY
#  define HAVE_SOCKETS
#  define HAVE_BSD_SOCKETS
#  define HAVE_UNISTD_H
#  define HAVE_PWD_H
#  define HAVE_UID_T

#endif /* !UNIX  &&  !unix */
