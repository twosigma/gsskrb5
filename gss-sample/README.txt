The following source files

      gss-client.c
      gss-server.c
      gss-misc.c
      gss-misc.h

have been copied from the MIT Kerberos v1.0.5 distribution.
These were originally donated to MIT by OpenVision Technologies, Inc.,
bearing the following copyright and licensing statement -- which
still applies to the sources here.  The sources required *significant*
bugfixing and minor modifications to be usable on Microsoft Win32
platforms and usable with the W2K Kerberos SSP wrapper.

Hostbased service names are somewhat unusual on Microsoft W2K Kerberos,
however everyone's credentials work for both, initiating and accepting
credentials so I have added code to accept not just "hostbased service
names" but also regular Kerberos principal names.

And as a final goodie -- since every users credentials are usable
for accepting credentials it is possible to run gss-server without
giving any service-name (just a -port <num>) and have gss-server
use its default accepting credentials and display the inquired
name from these credentials.

/*
 * Copyright 1994 by OpenVision Technologies, Inc.
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of OpenVision not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. OpenVision makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 * 
 * OPENVISION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL OPENVISION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */


