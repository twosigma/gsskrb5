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

#if !defined(lint) && !defined(__CODECENTER__)
static char *rcsid = "$Header: //tools/src/freeware/gsskrb5/gss-sample/gss-misc.c#3 $";
#endif


#include "gss-misc.h"

#if defined(NEED_WSASTARTUP)
int WSA_usecntr = 0;

int
startup_winsockets( )
{
    if ( WSA_usecntr==0 ) {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 1, 1 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if (err!=0) {
	    perror("WSAStartup Failed!!!!\n");
	    exit(1);
	}

    }

    WSA_usecntr++;

    return(1);

} /* startup_winsockets() */


int
shutdown_winsockets( )
{
    if ( WSA_usecntr>0 ) {
	WSA_usecntr--;
	if ( WSA_usecntr==0 )
	    WSACleanup();
    }

    return(1);

} /* shutdown_winsockets() */

void
display_windows_error( DWORD p_lasterror )
{
    int       i,j;
    char    * errmsg = NULL;
    DWORD     rval;

    rval = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
			  | FORMAT_MESSAGE_FROM_SYSTEM,
			  NULL, (DWORD) p_lasterror,
			  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			  (LPTSTR) &errmsg, 0, NULL );

    if ( rval==0 ) {
	fprintf(display_file, "FormatMessage(0x%08x) failed with 0x%08x!\n",
			      (unsigned int) p_lasterror, (unsigned int) GetLastError() );
    } else {
        /* Life isn't easy ... */
        /* Win32 GetLastError() overlaps and differs from errno-s			 */
        /* When I last tested, FormatMessage() completely ignored Winsocket Errors */
        /* and the printable may contain arbitrary linebreaks ...		         */
        for ( i=0 , j=0 ; j<rval ; j++) {
	    if ( errmsg[j]=='\r' )
	        continue;
	    errmsg[i] = (errmsg[j]=='\n') ? ' ' : errmsg[j];
	    i++;
	}

        errmsg[i] = '\0';

        /* finally, strip trailing spaces */
        while ( i>0 && errmsg[i-1]==' ' ) { i--; errmsg[i]='\0'; }

	fprintf( display_file, " (0x%08x) \"%s\"\n", p_lasterror, errmsg );
    }

    if ( errmsg!=NULL ) {
	LocalFree(errmsg);
	errmsg = NULL;
    }

    return;

} /* display_windows_error() */


#endif /* NEED_WSASTARTUP */


FILE * display_file;
FILE * logfp;

static void display_status_1(char *m, OM_uint32 code, int type);

static int write_all(int fildes, char *buf, unsigned int nbyte)
{
     int ret;
     char *ptr;

     for (ptr = buf; nbyte; ptr += ret, nbyte -= ret) {
	  ret = send(fildes, ptr, nbyte, 0);
	  if (ret < 0) {
	       if (errno == EINTR)
		    continue;
	       return(ret);
	  } else if (ret == 0) {
	       return(ptr-buf);
	  }
     }

     return(ptr-buf);
}

static int read_all(int fildes, char *buf, unsigned int nbyte)
{
     int ret;
     char *ptr;

     for (ptr = buf; nbyte; ptr += ret, nbyte -= ret) {
	  ret = recv(fildes, ptr, nbyte, 0);
	  if (ret < 0) {
	       if (errno == EINTR)
		    continue;
	       return(ret);
	  } else if (ret == 0) {
	       return(ptr-buf);
	  }
     }

     return(ptr-buf);
}

/*
 * Function: send_token
 *
 * Purpose: Writes a token to a file descriptor.
 *
 * Arguments:
 *
 * 	s		(r) an open file descriptor
 * 	tok		(r) the token to write
 *
 * Returns: 0 on success, -1 on failure
 *
 * Effects:
 *
 * send_token writes the token length (as a network long) and then the
 * token data to the file descriptor s.  It returns 0 on success, and
 * -1 if an error occurs or if it could not write all the data.
 */
int send_token(s, tok)
     int s;
     gss_buffer_t tok;
{
     int len, ret;

     len = htonl(tok->length);

     ret = write_all(s, (char *) &len, 4);
     if (ret < 0) {
	  perror("sending token length");
	  return -1;
     } else if (ret != 4) {
	 if (display_file)
	     fprintf(display_file, 
		     "sending token length: %d of %d bytes written\n", 
		     (int)ret, (int)4);
	  return -1;
     }

     ret = write_all(s, tok->value, tok->length);
     if (ret < 0) {
	  perror("sending token data");
	  return -1;
     } else if ( ret != (int)(tok->length) ) {
	 if (display_file)
	     fprintf(display_file, 
		     "sending token data: %d of %d bytes written\n", 
		     (int)ret, (int)tok->length);
	 return -1;
     }
     
     return 0;
}

/*
 * Function: recv_token
 *
 * Purpose: Reads a token from a file descriptor.
 *
 * Arguments:
 *
 * 	s		(r) an open file descriptor
 * 	tok		(w) the read token
 *
 * Returns: 0 on success, -1 on failure
 *
 * Effects:
 * 
 * recv_token reads the token length (as a network long), allocates
 * memory to hold the data, and then reads the token data from the
 * file descriptor s.  It blocks to read the length and data, if
 * necessary.  On a successful return, the token should be freed with
 * gss_release_buffer.  It returns 0 on success, and -1 if an error
 * occurs or if it could not read all the data.
 */
int recv_token(s, tok)
     int s;
     gss_buffer_t tok;
{
     int ret;

     ret = read_all(s, (char *) &tok->length, 4);
     if (ret < 0) {
	  perror("reading token length");
	  return -1;
     } else if (ret != 4) {
	 if (display_file)
	     fprintf(display_file, 
		     "reading token length: %d of %d bytes read\n", 
		     (int)ret, (int)4);
	 return -1;
     }
	  
     tok->length = ntohl(tok->length);
     tok->value = (char *) malloc(tok->length);
     if (tok->value == NULL) {
	 if (display_file)
	     fprintf(display_file, 
		     "Out of memory allocating token data\n");
	  return -1;
     }

     ret = read_all(s, (char *) tok->value, tok->length);
     if (ret < 0) {
	  perror("reading token data");
	  free(tok->value);
	  return -1;
     } else if ( ret != (int)tok->length ) {
	  fprintf(stderr, "sending token data: %d of %d bytes written\n", 
		  (int)ret, (int)tok->length);
	  free(tok->value);
	  return -1;
     }

     return 0;
}


int free_recv_token( tok )
     gss_buffer_t tok;
{
    if ( tok->value!=NULL ) {
	free(tok->value);
	tok->value  = NULL;
    }
    tok->length = 0;

    return 0;
}

static void display_status_1(m, code, type)
     char *m;
     OM_uint32 code;
     int type;
{
     OM_uint32 maj_stat, min_stat;
     gss_buffer_desc msg;
     OM_uint32 msg_ctx;
     
     msg_ctx = 0;
     while (1) {
	  maj_stat = gss_display_status(&min_stat, code,
				       type, GSS_C_NULL_OID,
				       &msg_ctx, &msg);
	  if (display_file)
	      fprintf(display_file, "GSS-API error %s: %s\n", m,
		      (char *)msg.value); 
	  (void) gss_release_buffer(&min_stat, &msg);
	  
	  if (!msg_ctx)
	       break;
     }
}

/*
 * Function: display_status
 *
 * Purpose: displays GSS-API messages
 *
 * Arguments:
 *
 * 	msg		a string to be displayed with the message
 * 	maj_stat	the GSS-API major status code
 * 	min_stat	the GSS-API minor status code
 *
 * Effects:
 *
 * The GSS-API messages associated with maj_stat and min_stat are
 * displayed on stderr, each preceeded by "GSS-API error <msg>: " and
 * followed by a newline.
 */
void display_status(msg, maj_stat, min_stat)
     char *msg;
     OM_uint32 maj_stat;
     OM_uint32 min_stat;
{
     display_status_1(msg, maj_stat, GSS_C_GSS_CODE);
     display_status_1(msg, min_stat, GSS_C_MECH_CODE);
}

/*
 * Function: display_ctx_flags
 *
 * Purpose: displays the flags returned by context initation in
 *	    a human-readable form
 *
 * Arguments:
 *
 * 	int		ret_flags
 *
 * Effects:
 *
 * Strings corresponding to the context flags are printed on
 * stdout, preceded by "context flag: " and followed by a newline
 */

void display_ctx_flags(flags)
     OM_uint32 flags;
{
     if (flags & GSS_C_DELEG_FLAG)
	  fprintf(display_file, "context flag: GSS_C_DELEG_FLAG\n");
     if (flags & GSS_C_MUTUAL_FLAG)
	  fprintf(display_file, "context flag: GSS_C_MUTUAL_FLAG\n");
     if (flags & GSS_C_REPLAY_FLAG)
	  fprintf(display_file, "context flag: GSS_C_REPLAY_FLAG\n");
     if (flags & GSS_C_SEQUENCE_FLAG)
	  fprintf(display_file, "context flag: GSS_C_SEQUENCE_FLAG\n");
     if (flags & GSS_C_CONF_FLAG )
	  fprintf(display_file, "context flag: GSS_C_CONF_FLAG\n");
     if (flags & GSS_C_INTEG_FLAG )
	  fprintf(display_file, "context flag: GSS_C_INTEG_FLAG\n");
}

void print_token(tok)
     gss_buffer_t tok;
{
    char line[128];
    char syms[32];
    int  offset;
    int  i, j;
    unsigned char *p = tok->value;
    unsigned char uc;

    if (!display_file)
	return;

    for( i=0 ; i<(int)tok->length ; ) {

        line[0]='\0';

        offset = i;

	memset(syms,' ',sizeof(syms));
        for ( j=0 ; j<16 && i<(int)tok->length ; j++,i++ ) {
            uc = p[i];
            sprintf(&(line[j*3+((j>7)?1:0)]), " %02x ", (int)uc);
            uc &= 0x7f;
            syms[j+(j>7?1:0)] = (uc>=' '&&uc<0x7f) ? (char)uc : '.';
        }
        syms[8]=' ';
	syms[17]='\0';

        fprintf( display_file, "0x%04x %-50s %s", (int)offset, line, syms);
        fprintf( display_file, "\n");
        
    }
    fprintf(display_file, "\n");
    fflush(display_file);
}
