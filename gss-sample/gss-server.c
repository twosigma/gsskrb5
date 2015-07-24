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
static char *rcsid = "$Header: //tools/src/freeware/gsskrb5/gss-sample/gss-server.c#4 $";
#endif


#include "gss-misc.h"


void usage()
{
     fprintf(stderr, "Usage: gss-server [-port port] [-verbose] [-count number]\n");
     fprintf(stderr, "       [-inetd] [-logfile file] [-xports number] [[(s|u|p):]service_name]\n");
     exit(1);
}

FILE *logfp   = NULL;
FILE *dynfp   = NULL;

int verbose   = 0;
int max_conn  = 0;
int do_xports = 3;

/*
 * Function: server_acquire_creds
 *
 * Purpose: imports a service name and acquires credentials for it
 *
 * Arguments:
 *
 * 	service_name	(r) the ASCII service name
 * 	server_creds	(w) the GSS-API service credentials
 *
 * Returns: 0 on success, -1 on failure
 *
 * Effects:
 *
 * The service name is imported with gss_import_name, and service
 * credentials are acquired with gss_acquire_cred.  If either opertion
 * fails, an error message is displayed and -1 is returned; otherwise,
 * 0 is returned.
 */
int server_acquire_creds(service_name, server_creds)
     char *service_name;
     gss_cred_id_t *server_creds;
{
     gss_buffer_desc name_buf;
     gss_buffer_desc inq_name_buf;
     gss_name_t      server_name;
     gss_OID	     nametype;
     OM_uint32       maj_stat;
     OM_uint32       min_stat;

     if ( service_name!=NULL && *service_name!='\0' ) {
	name_buf.value = service_name;
	nametype       = GSS_C_NT_HOSTBASED_SERVICE;

	if ( service_name[1]==':' ) {
	    if ( service_name[0]=='u' ) {
		name_buf.value = &(service_name[2]);
		nametype       = GSS_C_NT_USER_NAME;
	    } else if ( service_name[0]=='p' ) {
		name_buf.value = &(service_name[2]);
		nametype       = GSS_C_NO_OID;
	    } else if ( service_name[0]=='s' ) {
		name_buf.value = &(service_name[2]);
		nametype       = GSS_C_NT_HOSTBASED_SERVICE;
	    }
	}

	name_buf.length = strlen(name_buf.value);

	maj_stat = gss_import_name(&min_stat, &name_buf, nametype, &server_name);
	if (maj_stat != GSS_S_COMPLETE) {
	    display_status("importing name", maj_stat, min_stat);
	    return -1;
	}
     } else {
	server_name = GSS_C_NO_NAME; /* ask for default credentials */
     }

     maj_stat = gss_acquire_cred(&min_stat, server_name, 0,
				 GSS_C_NULL_OID_SET, GSS_C_ACCEPT,
				 server_creds, NULL, NULL);
     if (maj_stat != GSS_S_COMPLETE) {
	  display_status("acquiring credentials", maj_stat, min_stat);
	  return -1;
     }
     (void) gss_release_name(&min_stat, &server_name);
     maj_stat = gss_inquire_cred(&min_stat, *server_creds, &server_name, NULL, NULL, NULL);
     if (maj_stat != GSS_S_COMPLETE) {
	  display_status("inquiring credentials", maj_stat, min_stat);
	  return -1;
     }

     maj_stat = gss_display_name(&min_stat, server_name, &inq_name_buf, &nametype);
     if (maj_stat != GSS_S_COMPLETE) {
	  display_status("displaying name", maj_stat, min_stat);
	  return -1;
     }
     fprintf(logfp, "Name inquired from Server's credentials = \"%.*s\"\n",
		   (int)inq_name_buf.length, (char *)inq_name_buf.value );

     (void) gss_release_buffer(&min_stat, &inq_name_buf);
     (void) gss_release_name(&min_stat, &server_name);
     return 0;
}

/*
 * Function: server_establish_context
 *
 * Purpose: establishses a GSS-API context as a specified service with
 * an incoming client, and returns the context handle and associated
 * client name
 *
 * Arguments:
 *
 * 	s		(r) an established TCP connection to the client
 * 	service_creds	(r) server credentials, from gss_acquire_cred
 * 	context		(w) the established GSS-API context
 * 	client_name	(w) the client's ASCII name
 *
 * Returns: 0 on success, -1 on failure
 *
 * Effects:
 *
 * Any valid client request is accepted.  If a context is established,
 * its handle is returned in context and the client name is returned
 * in client_name and 0 is returned.  If unsuccessful, an error
 * message is displayed and -1 is returned.
 */
int server_establish_context(s, server_creds, context, client_name, ret_flags)
     int s;
     gss_cred_id_t server_creds;
     gss_ctx_id_t *context;
     gss_buffer_t client_name;
     OM_uint32 *ret_flags;
{
     gss_buffer_desc send_tok, recv_tok;
     gss_name_t client;
     gss_OID doid;
     OM_uint32 maj_stat, min_stat;
     gss_buffer_desc	oid_name;

     *context = GSS_C_NO_CONTEXT;
     
     do {
	  if (recv_token(s, &recv_tok) < 0)
	       return -1;

	  if (verbose && logfp) {
	      fprintf(logfp, "Received token (size=%d): \n", (int)recv_tok.length);
	      print_token(&recv_tok);
	  }

	  maj_stat =
	       gss_accept_sec_context(&min_stat,
				      context,
				      server_creds,
				      &recv_tok,
				      GSS_C_NO_CHANNEL_BINDINGS,
				      &client,
				      &doid,
				      &send_tok,
				      ret_flags,
				      NULL, 	/* ignore time_rec */
				      NULL); 	/* ignore del_cred_handle */

	  if (maj_stat!=GSS_S_COMPLETE && maj_stat!=GSS_S_CONTINUE_NEEDED) {
	       display_status("accepting context", maj_stat, min_stat);
	       (void) gss_release_buffer(&min_stat, &recv_tok);
	       return -1;
	  }

	  free_recv_token( &recv_tok );

	  if (send_tok.length != 0) {
	      if (verbose && logfp) {
		  fprintf(logfp,
			  "Sending accept_sec_context token (size=%d):\n",
			  (int)send_tok.length);
		  print_token(&send_tok);
	      }
	       if (send_token(s, &send_tok) < 0) {
		    fprintf(logfp, "failure sending token\n");
		    return -1;
	       }

	       (void) gss_release_buffer(&min_stat, &send_tok);
	  }
	  if (verbose && logfp) {
	      if (maj_stat == GSS_S_CONTINUE_NEEDED)
		  fprintf(logfp, "continue needed...\n");
	      else
		  fprintf(logfp, "\n");
	      fflush(logfp);
	  }
     } while (maj_stat == GSS_S_CONTINUE_NEEDED);

     /* display the flags */
     display_ctx_flags(*ret_flags);

     if (verbose && logfp) {
	 maj_stat = xgss_oid_to_str(&min_stat, doid, &oid_name);
	 if (maj_stat != GSS_S_COMPLETE) {
	     display_status("converting oid->string", maj_stat, min_stat);
	     return -1;
	 }
	 fprintf(logfp, "Accepted connection using mechanism OID %.*s.\n",
		 (int) oid_name.length, (char *) oid_name.value);
	 (void) xgss_release_buffer(&min_stat, &oid_name);
     }

     maj_stat = gss_display_name(&min_stat, client, client_name, &doid);
     if (maj_stat != GSS_S_COMPLETE) {
	  display_status("displaying name", maj_stat, min_stat);
	  return -1;
     }
     maj_stat = gss_release_name(&min_stat, &client);
     if (maj_stat != GSS_S_COMPLETE) {
	  display_status("releasing name", maj_stat, min_stat);
	  return -1;
     }
     return 0;
}

/*
 * Function: create_socket
 *
 * Purpose: Opens a listening TCP socket.
 *
 * Arguments:
 *
 * 	port		(r) the port number on which to listen
 *
 * Returns: the listening socket file descriptor, or -1 on failure
 *
 * Effects:
 *
 * A listening socket on the specified port and created and returned.
 * On error, an error message is displayed and -1 is returned.
 */
int create_socket(port)
     u_short port;
{
     struct sockaddr_in saddr;
     int s;
     int on = 1;
     
     saddr.sin_family = AF_INET;
     saddr.sin_port = htons(port);
     saddr.sin_addr.s_addr = INADDR_ANY;

     if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#ifdef _WIN32
	  fprintf(logfp, "socket() failed with\n");
	  display_windows_error( WSAGetLastError() );
#else
	  fprintf(logfp, "socket() failed with (%d) %s\n", errno, strerror(errno));
#endif
	  return -1;
     }
     /* Let the socket be reused right away */
     (void) setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
#ifdef TCP_NODELAY
     (void) setsockopt(s, SOL_SOCKET, TCP_NODELAY,  (char *)&on, sizeof(on)); 
#endif
     if (bind(s, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
#ifdef _WIN32
	  fprintf(logfp, "bind(%d) failed with\n", port);
	  display_windows_error( WSAGetLastError() );
#else
	  fprintf(logfp, "bind(%d) failed with (%d) %s\n", port, errno, strerror(errno));
#endif
	  (void) closesocket(s);
	  return -1;
     }
     if (listen(s, 5) < 0) {
#ifdef _WIN32
	  fprintf(logfp, "listen(%d) failed with\n",port);
	  display_windows_error( WSAGetLastError() );
#else
	  fprintf(logfp, "listen(%d) failed with (%d) %s\n", port, errno, strerror(errno));
#endif
	  (void) closesocket(s);
	  return -1;
     }
     return s;
}

static float timeval_subtract(tv1, tv2)
	struct timeval *tv1, *tv2;
{
	return ((tv1->tv_sec - tv2->tv_sec) +
		((float) (tv1->tv_usec - tv2->tv_usec)) / 1000000);
}

/*
 * Yes, yes, this isn't the best place for doing this test.
 * DO NOT REMOVE THIS UNTIL A BETTER TEST HAS BEEN WRITTEN, THOUGH.
 * 					-TYT
 */
int test_import_export_context(context)
	gss_ctx_id_t *context;
{
	OM_uint32	min_stat, maj_stat;
	gss_buffer_desc context_token, copied_token;
	struct timeval tm1, tm2;

	memset(&tm1, 0, sizeof(tm1));
	memset(&tm2, 0, sizeof(tm2));

	/*
	 * Attempt to save and then restore the context.
	 */
//	gettimeofday(&tm1, (struct timezone *)0);
	maj_stat = gss_export_sec_context(&min_stat, context, &context_token);
	if (maj_stat != GSS_S_COMPLETE) {
		display_status("exporting context", maj_stat, min_stat);
		return 1;
	}
//	gettimeofday(&tm2, (struct timezone *)0);
	if (verbose && logfp) {
		fprintf(logfp, "Exported context: %d bytes, %7.4f seconds\n",
			(int)context_token.length, (float)timeval_subtract(&tm2, &tm1));
     
		fprintf(logfp, "Exported context token:\n");
		print_token(&context_token);
	}
	copied_token.length = context_token.length;
	copied_token.value = malloc(context_token.length);
	if (copied_token.value == 0) {
	    fprintf(logfp, "Couldn't allocate memory to copy context token.\n");
	    return 1;
	}
	memcpy(copied_token.value, context_token.value, copied_token.length);
	maj_stat = gss_import_sec_context(&min_stat, &copied_token, context);
	if (maj_stat != GSS_S_COMPLETE) {
		display_status("importing context", maj_stat, min_stat);
		return 1;
	}
//	gettimeofday(&tm1, (struct timezone *)0);
	if (verbose && logfp)
		fprintf(logfp, "Importing context: %7.4f seconds\n",
			(float)timeval_subtract(&tm1, &tm2));
	(void) gss_release_buffer(&min_stat, &context_token);
	free(copied_token.value);

	return 0;
}

/*
 * Function: sign_server
 *
 * Purpose: Performs the "sign" service.
 *
 * Arguments:
 *
 * 	s		(r) a TCP socket on which a connection has been
 *			accept()ed
 * 	service_name	(r) the ASCII name of the GSS-API service to
 * 			establish a context as
 * 
 * Returns: -1 on error
 *
 * Effects:
 *
 * sign_server establishes a context, and performs a single sign request.
 *
 * A sign request is a single GSS-API sealed token.  The token is
 * unsealed and a signature block, produced with gss_sign, is returned
 * to the sender.  The context is the destroyed and the connection
 * closed.
 *
 * If any error occurs, -1 is returned.
 */
int sign_server(s, server_creds)
     int s;
     gss_cred_id_t server_creds;
{
     gss_buffer_desc client_name, recv_buf, xmit_buf, msg_buf;
     gss_ctx_id_t context;
     OM_uint32 maj_stat, min_stat;
     int i, conf_state, ret_flags;
     char	*cp;
     int         rc = -1;
     
     recv_buf.value  = xmit_buf.value  = msg_buf.value  = NULL;
     recv_buf.length = xmit_buf.length = msg_buf.length = 0;

     /* Establish a context with the client */
     if (server_establish_context(s, server_creds, &context,
				  &client_name, &ret_flags) < 0)
     goto error;
	  
     fprintf(logfp,"Accepted connection: \"%.*s\"\n",
	    (int) client_name.length, (char *) client_name.value);
     (void) gss_release_buffer(&min_stat, &client_name);

     for (i=0; i < do_xports; i++)
	     if (test_import_export_context(&context))
		     goto error;

     /* Receive the sealed message token */
     if (recv_token(s, &recv_buf) < 0)
	 goto error;
	  
     if (verbose && logfp) {
	fprintf(logfp, "Sealed message token:\n");
	print_token(&recv_buf);
     }

     maj_stat = gss_unwrap(&min_stat, context, &recv_buf, &msg_buf,
			   &conf_state, (gss_qop_t *) NULL);
     if ( do_xports>0 && verbose) {
	 /* one more time to get a hexdump of the exported context token */
         test_import_export_context(&context);
     }

     if (maj_stat != GSS_S_COMPLETE) {
	display_status("unsealing message", maj_stat, min_stat);
	goto error;
     } else if (! conf_state) {
	fprintf(stderr, "Warning!  Message not encrypted.\n");
     }

     fprintf(logfp, "Received message: ");
     cp = msg_buf.value;
     if (isprint(cp[0]) && isprint(cp[1]))
	fprintf(logfp, "\"%s\"\n", cp);
     else {
	fprintf(logfp,"\n");
	print_token(&msg_buf);
     }
	  
     /* Produce a signature block for the message */
     maj_stat = gss_get_mic(&min_stat, context, GSS_C_QOP_DEFAULT,
			    &msg_buf, &xmit_buf);
     if (maj_stat != GSS_S_COMPLETE) {
	display_status("signing message", maj_stat, min_stat);
	goto error;
     }

     /* Send the signature block to the client */
     if (send_token(s, &xmit_buf) < 0)
	 goto error;

     rc = 0;

error:
     if ( recv_buf.value!=NULL )
         (void) free_recv_token(&recv_buf);

     if ( msg_buf.value!=NULL )
         (void) gss_release_buffer(&min_stat, &msg_buf);

     if ( xmit_buf.value!=NULL )
	(void) gss_release_buffer(&min_stat, &xmit_buf);

     /* Delete context */
     if ( context!=GSS_C_NO_CONTEXT ) {
         maj_stat = gss_delete_sec_context(&min_stat, &context, NULL);
         if (maj_stat != GSS_S_COMPLETE) {
	    display_status("deleting context", maj_stat, min_stat);
	    rc = -1;
	 }
     }

     fflush(logfp);

     return(rc);
}

int
main(argc, argv)
     int argc;
     char **argv;
{
     char *service_name;
     gss_cred_id_t server_creds;
     OM_uint32 min_stat;
     u_short port = 4444;
     int s;
     int once = 0;
     int do_inetd = 0;
     int port_given = 0;
     int conn_count = 0;

     logfp = stdout;
     display_file = stdout;
     argc--; argv++;
     while (argc) {
	  if (strcmp(*argv, "-port") == 0) {
	       argc--; argv++;
	       if (!argc) usage();
	       port = atoi(*argv);
	       port_given = 1;
	  } else if (strcmp(*argv, "-verbose") == 0) {
	      verbose = 1;
	  } else if (strcmp(*argv, "-once") == 0) {
	      once = 1;
	  } else if (strcmp(*argv, "-inetd") == 0) {
	      do_inetd = 1;
	  } else if (strcmp(*argv, "-xports") == 0) {
	      argc--; argv++;
	      if (!argc) usage();
	      do_xports = atoi(*argv);
	  } else if (strcmp(*argv, "-count") == 0 ) {
	      argc--; argv++;
	      if (!argc) usage();
	      max_conn = atoi(*argv);
	  } else if (strcmp(*argv, "-logfile") == 0) {
	      argc--; argv++;
	      if (!argc) usage();
	      dynfp = fopen(*argv, "a");
	      if (!dynfp) {
		  perror(*argv);
		  exit(1);
	      }
	      display_file = logfp = dynfp;
	  } else
	       break;
	  argc--; argv++;
     }
     if (argc > 1)
	  usage();

     if (argv[0]!=NULL && (*argv)[0] == '-')
	  usage();

     service_name = *argv;

#ifdef NEED_WSASTARTUP
     startup_winsockets();
#endif

     if (server_acquire_creds(service_name, &server_creds) < 0)
	 return -1;
     
     if (do_inetd) {
	 close(1);
	 close(2);

	 sign_server(0, server_creds);
	 close(0);
     } else {
	 int stmp;

	 if ((stmp = create_socket(port))) {
	     do {
		 /* Accept a TCP connection */
		 if ((s = accept(stmp, NULL, 0)) < 0) {
		     perror("accepting connection");
		 } else {
		     conn_count++;
		     /* this return value is not checked, because there's
			not really anything to do if it fails */
		     sign_server(s, server_creds);
		     (void) closesocket(s);
		 }
		 if ( max_conn>0 && conn_count>=max_conn )
		     break;
	     } while (!once);
	 }

	 closesocket(stmp);
     }

     (void) gss_release_cred(&min_stat, &server_creds);


#ifdef NEED_WSASTARTUP
     shutdown_winsockets();
#endif
     if (dynfp!=NULL) {
	 fclose(dynfp);
	 dynfp = NULL;
	 display_file = logfp = stdout;
     }

     return 0;
}
