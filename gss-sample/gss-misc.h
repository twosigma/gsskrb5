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

/*
 * $Id: //tools/src/freeware/gsskrb5/gss-sample/gss-misc.h#3 $
 */

#ifndef _GSSMISC_H_
#define _GSSMISC_H_


#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "platform.h"

#ifdef HAVE_WINSOCK
#  include <io.h>
#  include <windows.h>
#  include <winsock.h>
#  include <fcntl.h>
#  define stat _stat
#  define fstat _fstat
#  define open  _open
#  define close _close
#  define read  _read
#  define O_RDONLY _O_RDONLY
#endif

#ifdef NEED_WSASTARTUP
int   startup_winsockets(void);
int   shutdown_winsockets(void);
void  display_windows_error( DWORD p_lasterror );

#endif

#ifdef HAVE_UNISTD_H
#  define  closesocket close
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <netinet/in.h>
#  include <unistd.h>
#  include <fcntl.h>
#endif

#include "gssapi_2.h"
#include "oid-func.h"


extern FILE *display_file;

int send_token(int s, gss_buffer_t tok);
int recv_token(int s, gss_buffer_t tok);
int free_recv_token(gss_buffer_t tok);
void display_status(char *msg, OM_uint32 maj_stat, OM_uint32 min_stat);
void display_ctx_flags(OM_uint32 flags);
void print_token(gss_buffer_t tok);

#endif
