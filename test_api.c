#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/test_api.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/test_api.c#2 $
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

#include "test_api.h"

#include <time.h>

FILE * fh = NULL;

#if defined(SHLIB_EXPORTS_OID_DATA)

#  define gss_C_NT_HOSTBASED_SERVICE_V2	  GSS_C_NT_HOSTBASED_SERVICE_V2
#  define gss_C_NT_ANONYMOUS		  GSS_C_NT_ANONYMOUS
#  define gss_C_NT_EXPORT_NAME		  GSS_C_NT_EXPORT_NAME
#  define gss_C_NT_USER_NAME		  GSS_C_NT_USER_NAME
#  define gss_C_NT_MACHINE_UID_NAME	  GSS_C_NT_MACHINE_UID_NAME
#  define gss_C_NT_STRING_UID_NAME	  GSS_C_NT_STRING_UID_NAME
#  define gss_C_NT_HOSTBASED_SERVICE	  GSS_C_NT_HOSTBASED_SERVICE

#else /* !SHLIB_EXPORTS_OID_DATA) */

   gss_OID_desc gn_gss_oids[] = {
      {  6, (void *)"\x2b\x06\x01\x05\x06\x02"			}, /* IANA: hostbased service  */
      {  6, (void *)"\x2b\x06\x01\x05\x06\x03"			}, /* IANA: anonymous name     */
      {  6, (void *)"\x2b\x06\x01\x05\x06\x04"			}, /* IANA: exported name      */
      { 10, (void *)"\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x01"	}, /* MIT: Printable User name */
      { 10, (void *)"\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x02"	}, /* MIT: Machine UID name    */
      { 10, (void *)"\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x03"	}, /* MIT: String UID name     */
      { 10, (void *)"\052\206\110\206\367\022\001\002\001\004"	}, /* MIT: Hostbased Service   */
      {  0, NULL						}
   };

   int gn_gss_oids_num = ARRAY_ELEMENTS(gn_gss_oids);

   /***********************************************************/
   /* Official & standardized GSS-API Name Types	      */
   /* first letters are lowercased to prevent collisions with */
   /* the __dllexport() declarations in the header gssapi_2.h */
   /***********************************************************/

   gss_OID gss_C_NT_HOSTBASED_SERVICE_V2	= &(gn_gss_oids[0]);
   gss_OID gss_C_NT_ANONYMOUS			= &(gn_gss_oids[1]);
   gss_OID gss_C_NT_EXPORT_NAME			= &(gn_gss_oids[2]);
   gss_OID gss_C_NT_USER_NAME			= &(gn_gss_oids[3]);
   gss_OID gss_C_NT_MACHINE_UID_NAME		= &(gn_gss_oids[4]);
   gss_OID gss_C_NT_STRING_UID_NAME		= &(gn_gss_oids[5]);
   gss_OID gss_C_NT_HOSTBASED_SERVICE		= &(gn_gss_oids[6]);

#endif /* !SHLIB_EXPORTS_OID_DATA */



int force_export = 0;





int
main( int argc, char **argv )
{
   int                rc = 0;
   OM_uint32          min_stat, maj_stat;
   gss_buffer_desc    in_pr_name;
   char		    **xargv;
   char		     *arg_initiator = NULL;
   char		     *arg_acceptor  = NULL; 
   char               username[256];
   char		      hostname[256];
   gss_buffer_desc    dsp_name;
   gss_buffer_desc    exp_name;
   gss_name_t         out_name     = GSS_C_NO_NAME;
   gss_name_t         can_name     = GSS_C_NO_NAME;
   gss_name_t	      cred_name    = GSS_C_NO_NAME;
   gss_name_t	      src_name	   = GSS_C_NO_NAME;
   gss_name_t	      ini_name	   = GSS_C_NO_NAME;
   gss_name_t	      acc_name	   = GSS_C_NO_NAME;
   gss_name_t	      dup_name     = GSS_C_NO_NAME;
   gss_OID            nametype     = GSS_C_NO_OID;
   gss_OID            oid          = GSS_C_NO_OID;
   gss_OID            mech_oid     = GSS_C_NO_OID;
   gss_OID            mech1_oid    = GSS_C_NO_OID;
   gss_OID	      ini_mech	   = GSS_C_NO_OID;
   gss_OID	      acc_mech	   = GSS_C_NO_OID;
   gss_OID_set        mech_oid_set = GSS_C_NO_OID_SET;
   gss_OID_set        nt_oid_set   = GSS_C_NO_OID_SET;
   gss_OID_set        cred_mechs   = GSS_C_NO_OID_SET;
   gss_cred_id_t      ini_cred     = GSS_C_NO_CREDENTIAL;
   gss_cred_id_t      acc_cred	   = GSS_C_NO_CREDENTIAL;
   gss_ctx_id_t	      ini_ctx	   = GSS_C_NO_CONTEXT;
   gss_ctx_id_t	      acc_ctx	   = GSS_C_NO_CONTEXT;
   gss_cred_usage_t   cred_usage   = 0;
   OM_uint32          lifetime     = 0;
   OM_uint32	      ini_lifetime = 0;
   OM_uint32	      acc_lifetime = 0;
   OM_uint32	      ini_recflags = 0;
   OM_uint32	      acc_recflags = 0;
   OM_uint32	      ini_retflags = 0;
   OM_uint32	      acc_retflags = 0;
   int                i;
   int                equal	   = 0;
   int		      xargc;

   fh = stdout;

   hostname[0] = username[0] = '\0';
   dsp_name.value     = NULL;
   dsp_name.length    = 0;
   exp_name.value     = NULL;
   exp_name.length    = 0;


   init_timer();
   show_timer_resolution();

   xargc = argc;
   xargv = argv;

   for ( xargc=argc, xargv=argv ; xargc>0 ; xargc-- , xargv++ ) {
      if ( xargc>1  &&  !strcmp( xargv[0], "-ini") ) {
	 arg_initiator = xargv[1];
	 xargv++; xargc--;
      } else if ( xargc>1  &&  !strcmp( xargv[0], "-acc") ) {
	 arg_acceptor  = xargv[1];
	 xargv++; xargc--;
      } else if ( !strcmp(xargv[0],"-e") ) {
	 force_export = TRUE;
      }
   }

   if ( arg_initiator!=NULL ) {
      nametype        = gss_C_NT_USER_NAME;
      if ( arg_initiator[1]==':' ) {
	 switch (arg_initiator[0]) {
	    case 's':
	    case 'S':	nametype = gss_C_NT_HOSTBASED_SERVICE;
			arg_initiator += 2;
			break;

	    case 'u':
	    case 'U':	nametype = gss_C_NT_USER_NAME;
			arg_initiator += 2;
			break;

	 }
      }
      in_pr_name.value   = arg_initiator;
      in_pr_name.length  = strlen(arg_initiator);

   } else { /* else   (arg_myname==NULL) */

#ifdef _WIN32
      LPTSTR  Wptr = username;
      DWORD   Wlen = sizeof(username)-1;

      GetUserName( Wptr, &Wlen );

      in_pr_name.value   = username;
      if ( Wlen>0 && username[Wlen-1]=='\0' ) {
	 /* Windows includes the trailing \0 in the count ... */
	 in_pr_name.length = Wlen-1;
      } else {
	 in_pr_name.length = Wlen;
      }
      nametype        = gss_C_NT_USER_NAME;
#else
      gethostname(hostname, sizeof(hostname)-1);
      sprintf(username, "sample@%.*s", sizeof(username)-10, hostname);
      in_pr_name.value   = username;
      in_pr_name.length  = strlen((char *)in_pr_name.value);
      nametype        = gss_C_NT_HOSTBASED_SERVICE;
#endif

   } /* endif (arg_myname==NULL) */

   maj_stat = gss_indicate_mechs( &min_stat, &mech_oid_set );
   print_status("gss_indicate_mechs", GSS_C_NO_OID, maj_stat, min_stat);
   if ( maj_stat!=GSS_S_COMPLETE ) {

      rc = 1;

   } else {

      mech1_oid = &(mech_oid_set->elements[0]);

      for ( i=0 ; (Uint)i < (mech_oid_set->count) ; i++ ) {
         nt_oid_set = GSS_C_NO_OID_SET;
	 mech_oid = &(mech_oid_set->elements[i]);
	 maj_stat = gss_inquire_names_for_mech( &min_stat, mech_oid,
						&nt_oid_set );
	 print_status("gss_inquire_names_for_mech",
 		      mech_oid, maj_stat, min_stat);
	 if ( maj_stat!=GSS_S_COMPLETE ) {
	    rc = 1;
	 }

	 rc |= drop_oid_set( &nt_oid_set );
      }

   }

   maj_stat = gss_import_name( &min_stat,
			       &in_pr_name, nametype, &out_name );
   print_status("gss_import_name", GSS_C_NO_OID, maj_stat, min_stat);
   if ( maj_stat!=GSS_S_COMPLETE ) {
      rc = 1;
   } else {

     rc |= display_name( "newly imported", out_name );

#if 1
     maj_stat = gss_canonicalize_name( &min_stat, out_name,
				       mech1_oid, &can_name );
     print_status("gss_canonicalize_name", mech1_oid, maj_stat, min_stat);
     if ( maj_stat!=GSS_S_COMPLETE ) {
	rc = 1;
     } else {
	rc |= display_name( "canonicalized", can_name );

	exp_name.length = 0;
	maj_stat = gss_export_name( &min_stat, can_name, &exp_name );
        print_status("gss_export_name", mech1_oid, maj_stat, min_stat);
	if (maj_stat!=GSS_S_COMPLETE) {
	   rc = 1;
	} else {
	   maj_stat = gss_import_name( &min_stat, &exp_name,
				       gss_C_NT_EXPORT_NAME, &ini_name );
	   print_status("gss_import_name", mech1_oid, maj_stat, min_stat);
	   if (maj_stat!=GSS_S_COMPLETE) {
	      rc = 1;
	   } else {

	      rc |= display_name( "reimported", ini_name );

	   }

	   maj_stat = gss_duplicate_name( &min_stat, ini_name, &dup_name );
	   print_status("gss_duplicate_name", mech1_oid, maj_stat, min_stat);
	   if (maj_stat!=GSS_S_COMPLETE) {
	      rc = 1;
	   } else {

	      rc |= display_name( "duplicated", dup_name );

	   }

	   maj_stat = gss_compare_name( &min_stat, out_name, can_name, &equal);
	   print_status("gss_compare_name", mech1_oid, maj_stat, min_stat);
	   if (maj_stat!=GSS_S_COMPLETE) {
	      rc = 1;
	   } else {
	      fprintf(stdout, "gss_compare_name(name,"
                              " canonicalize(name))) = %s\n",
		      (equal==0) ? "FALSE" : "TRUE" );
	   }

	   maj_stat = gss_compare_name( &min_stat, out_name, dup_name, &equal);
	   print_status("gss_compare_name", mech1_oid, maj_stat, min_stat);
	   if (maj_stat!=GSS_S_COMPLETE) {
	      rc = 1;
	   } else {
	      fprintf(stdout, "gss_compare_name(name,"
                              " duplicate(import(export(name)))) = %s\n",
		      (equal==0) ? "FALSE" : "TRUE" );
	   }

	   rc |= drop_name( &dup_name );

	   maj_stat = gss_compare_name( &min_stat, ini_name, can_name, &equal);
	   print_status("gss_compare_name", mech1_oid, maj_stat, min_stat);
	   if (maj_stat!=GSS_S_COMPLETE) {
	      rc = 1;
	   } else {
	      fprintf(stdout, "gss_compare_name(can_name,"
                              " import(export(can_name))) = %s\n",
		      (equal==0) ? "FALSE" : "TRUE" );

#else
     {
	{
	   {
#endif

	      maj_stat = gss_acquire_cred( &min_stat, GSS_C_NO_NAME, GSS_C_INDEFINITE,
					   GSS_C_NO_OID_SET, GSS_C_INITIATE,
					   &ini_cred, &cred_mechs, &lifetime );
	      print_status("gss_acquire_cred", GSS_C_NO_OID, maj_stat, min_stat);
	      if (maj_stat!=GSS_S_COMPLETE) {
		 rc = 1;
	      } else {
		 print_lifetime("gss_acquire_cred(INITIATE) returned lifetime", lifetime);
	      }

	      rc |= drop_oid_set( &cred_mechs );

	      maj_stat = gss_inquire_cred( &min_stat, ini_cred, &cred_name,
					   &lifetime, &cred_usage, &cred_mechs );
	      print_status("gss_inquire_cred", GSS_C_NO_OID, maj_stat, min_stat);
	      if (maj_stat!=GSS_S_COMPLETE) {
		 rc = 1;
	      } else {
		 print_lifetime("gss_inquire_cred() returned lifetime", lifetime);
		 rc |= display_name( "default credential", cred_name );
	      }

	      rc |= drop_cred( &ini_cred );

	      maj_stat = gss_acquire_cred( &min_stat, cred_name, GSS_C_INDEFINITE,
					   cred_mechs, GSS_C_INITIATE,
					   &ini_cred, NULL, &lifetime );
	      print_status("gss_acquire_cred", mech1_oid, maj_stat, min_stat);
	      if (maj_stat!=GSS_S_COMPLETE) {
		 rc = 1;
	      } else {
		 print_lifetime("gss_acquire_cred(INITIATE) returned lifetime", lifetime);
	      }

	      rc |= drop_name( &cred_name );
	      rc |= drop_oid_set( &cred_mechs );

	      maj_stat = gss_inquire_cred( &min_stat, ini_cred, &cred_name,
					   &lifetime, &cred_usage, &cred_mechs );
	      print_status("gss_inquire_cred", GSS_C_NO_OID, maj_stat, min_stat);
	      if (maj_stat!=GSS_S_COMPLETE) {
		 rc = 1;
	      } else {
		 print_lifetime("gss_inquire_cred() returned lifetime", lifetime);
		 rc |= display_name( "explicit credential", cred_name );
	      }
	      
	      rc |= drop_oid_set( &cred_mechs );
	      rc |= drop_name( &cred_name );

	      if ( arg_acceptor!=NULL ) {
	         nametype        = gss_C_NT_USER_NAME;
		 if ( arg_acceptor[1]==':' ) {
		    switch (arg_acceptor[0]) {
		       case 's':
		       case 'S':       nametype = gss_C_NT_HOSTBASED_SERVICE;
				       arg_acceptor += 2;
				       break;

		       case 'u':
		       case 'U':       nametype = gss_C_NT_USER_NAME;
				       arg_acceptor += 2;
				       break;

		    }
		 }
		 in_pr_name.value  = arg_acceptor;
		 in_pr_name.length = strlen((char *)in_pr_name.value);
	      }

	      maj_stat = gss_import_name( &min_stat, &in_pr_name, nametype, &acc_name );
	      print_status("gss_import_name", GSS_C_NO_OID, maj_stat, min_stat);
	      if (maj_stat!=GSS_S_COMPLETE) {
		 rc = 1;
	      } else {
		 gss_name_t  tmp_name = GSS_C_NO_NAME;

		 maj_stat = gss_canonicalize_name( &min_stat, acc_name, mech1_oid, &tmp_name );
		 print_status("gss_canonicalize_name", GSS_C_NO_OID, maj_stat, min_stat);
		 if ( maj_stat==GSS_S_COMPLETE ) {
		    rc |= display_name( "canonicalized acceptor", tmp_name );
		 }
		 rc |= drop_name( &tmp_name );
	      }

	      maj_stat = gss_acquire_cred( &min_stat, acc_name, GSS_C_INDEFINITE,
					   GSS_C_NO_OID_SET, GSS_C_ACCEPT,
					   &acc_cred, NULL, &lifetime );
	      if (maj_stat!=GSS_S_COMPLETE) {
		 rc = 1;
	      } else {
		 print_lifetime("gss_acquire_cred(ACCEPT) returned lifetime", lifetime);
	      }

	      maj_stat = gss_inquire_cred( &min_stat, acc_cred, &cred_name,
					   &lifetime, &cred_usage, &cred_mechs );
	      print_status("gss_inquire_cred", GSS_C_NO_OID, maj_stat, min_stat);
	      if (maj_stat!=GSS_S_COMPLETE) {
		 rc = 1;
	      } else {
		 print_lifetime("gss_inquire_cred() returned lifetime", lifetime);
		 rc |= display_name( "explicit accepting cred", cred_name );
	      }

	      rc |= display_name( "target name", acc_name );

//	      for ( i=1; i<100000 ; i++ ) {
		  acc_ctx = GSS_C_NO_CONTEXT;
		  ini_ctx = GSS_C_NO_CONTEXT;

		  rc |= create_context( acc_name, GSS_C_NO_OID, GSS_C_INTEG_FLAG|GSS_C_CONF_FLAG|GSS_C_MUTUAL_FLAG,
				        GSS_C_INDEFINITE, ini_cred, acc_cred,
					&src_name, &ini_ctx, &acc_ctx,
					&ini_mech, &acc_mech,
					&ini_retflags, &acc_retflags,
					&ini_lifetime, &acc_lifetime );
//		  if (rc!=0) {
//		     fprintf(stdout, " ERROR: failure attempting to establish security context #%7lu !\n", i);
//		     break;
//		  } else {
//		     fprintf(stdout, " Security context #%7lu created!\n", i);
//		  }
//	      }

	      if ( src_name!=GSS_C_NO_NAME ) {
		 rc |= display_name( "authenticated source", src_name );
		 print_lifetime("gss_init_sec_context() returned lifetime  ", ini_lifetime);
		 print_lifetime("gss_accept_sec_context() returned lifetime", acc_lifetime);
	      }

	      rc |= check_context_lifetime( "initiator", mech1_oid, ini_ctx );
	      rc |= check_context_lifetime( "acceptor",  mech1_oid, acc_ctx );

	      if ( (ini_retflags & GSS_C_INTEG_FLAG & acc_retflags)!=0 ) {
		 OM_uint32   max_insize;
		 int	     do_conf;

	         rc |= transfer_signed_message( 4, "initiator", ini_ctx, "acceptor",  acc_ctx );
	         rc |= transfer_signed_message( 4, "acceptor",  acc_ctx, "initiator", ini_ctx );
//	         rc |= transfer_signed_message( 2, "initiator", ini_ctx, "initiator", ini_ctx );
//	         rc |= transfer_signed_message( 2, "acceptor",  acc_ctx, "acceptor",  acc_ctx );

		 for ( i=290 ; i>0 ; i -= 70 ) {
		    maj_stat = gss_wrap_size_limit( &min_stat, ini_ctx, 0, 0, (OM_uint32)i, &max_insize );
		    fprintf(stdout, "  gss_wrap_size_limit( output=%lu ) => max_input = %lu\n",
				    (Ulong) i, (Ulong) max_insize );
		    print_status("gss_wrap_size_limit", GSS_C_NO_OID, maj_stat, min_stat);
		    if (maj_stat!=GSS_S_COMPLETE) {
		       break;
		    }

		 } /* end for() */

		 do_conf = (ini_retflags & GSS_C_CONF_FLAG & acc_retflags);
//	         rc |= transfer_wrapped_message( 2, do_conf, "initiator", ini_ctx, "initiator", ini_ctx );
//	         rc |= transfer_wrapped_message( 2, do_conf, "acceptor",  acc_ctx, "acceptor",  acc_ctx );
	         rc |= transfer_wrapped_message( 4, do_conf, "initiator", ini_ctx, "acceptor",  acc_ctx );
	         rc |= transfer_wrapped_message( 4, do_conf, "acceptor",  acc_ctx, "initiator", ini_ctx );
		 
	         rc |= signed_message_speed(   100, ini_ctx, acc_ctx );
	         rc |= signed_message_speed(   500, ini_ctx, acc_ctx );
	         rc |= signed_message_speed(  2000, ini_ctx, acc_ctx );
//	         rc |= signed_message_speed(  5000, ini_ctx, acc_ctx );
//	         rc |= signed_message_speed( 20000, ini_ctx, acc_ctx );
//	         rc |= signed_message_speed(100000, ini_ctx, acc_ctx );
//	         rc |= signed_message_speed(500000, ini_ctx, acc_ctx );

	      }  /* endif GSS_C_INTEG_FLAG */

	      rc |= drop_name( &src_name );

	      if ( force_export || (ini_retflags & GSS_C_TRANS_FLAG & acc_retflags)!=0 )  {
	         rc |= exp_imp_context( 1, "initiator", mech1_oid, &ini_ctx );
	         rc |= exp_imp_context( 1, "acceptor",  mech1_oid, &acc_ctx );
//      	         rc |= exp_imp_context( 100, "initiator", mech1_oid, &ini_ctx );
//	         rc |= exp_imp_context( 100, "acceptor",  mech1_oid, &acc_ctx );
	      }

	      rc |= check_context_lifetime( "initiator", mech1_oid, ini_ctx );
	      rc |= check_context_lifetime( "acceptor",  mech1_oid, acc_ctx );

	      
	      rc |= drop_context( &ini_ctx );
	      rc |= drop_context( &acc_ctx );


	      rc |= drop_oid_set( &cred_mechs );
	      rc |= drop_name( &cred_name );

	      rc |= drop_cred( &ini_cred );
	      rc |= drop_cred( &acc_cred );

	   }

	}

	rc |= drop_buffer( &exp_name );

     }

     rc |= drop_name( &can_name );
     rc |= drop_name( &out_name );

   } /* import_name == OK */

   rc |= drop_context( &ini_ctx );
   rc |= drop_context( &acc_ctx );

   rc |= drop_cred( &ini_cred ); /* just for safety */
   rc |= drop_cred( &acc_cred ); /* just for safety */

   rc |= drop_name( &ini_name );
   rc |= drop_name( &acc_name );

   rc |= drop_name( &cred_name );

   rc |= drop_oid_set( &cred_mechs );
   rc |= drop_oid_set( &mech_oid_set );

   return(rc);

} /* main() */



/*
 * transfer_signed_message()
 *
 *
 */
int
transfer_signed_message( int    count,
			 char * src_name, gss_ctx_id_t src_ctx,
			 char * dst_name, gss_ctx_id_t dst_ctx )
{
   gss_buffer_desc   mic_token;
   gss_buffer_desc   message;
   OM_uint32	     maj_stat, min_stat;
   gss_qop_t	     qop;
   static int	     cntr = 1;
   int		     rc   = 0;
   int		     i;
   char		     sendmsg[256];

   for ( i=1 ; i<count+1 ; i++ ) {

      fprintf(stdout, " signed message #%02d from %s to %s:\n", i, src_name, dst_name );
      sprintf(sendmsg , "This is test message #%04d\n", cntr);
      message.value  = sendmsg;
      message.length = strlen(sendmsg);
      mic_token.value  = NULL;
      mic_token.length = 0;

      maj_stat = gss_get_mic( &min_stat, src_ctx, 0, &message, &mic_token );
      print_status("gss_get_mic", GSS_C_NO_OID, maj_stat, min_stat);
      if (maj_stat!=GSS_S_COMPLETE) {

	 rc |= 1;
	 rc |= drop_buffer( &mic_token );
	 break;

      } else {

	 maj_stat = gss_verify_mic( &min_stat, dst_ctx, &message, &mic_token, &qop );
	 print_status("gss_verify_mic", GSS_C_NO_OID, maj_stat, min_stat);
	 if (maj_stat!=GSS_S_COMPLETE) {
	    rc |= 1;
	    rc |= drop_buffer( &mic_token );
	    break;
	 }

      }

      cntr++;
      rc |= drop_buffer( &mic_token );

   } /* end for() */

   return(rc);

} /* transfer_signed_message() */




#define SIGN_CYCLES  16
/*
 * signed_message_speed()
 *
 *
 */
int
signed_message_speed( size_t  msg_size,
		      gss_ctx_id_t ini_ctx, gss_ctx_id_t acc_ctx )
{
   gss_buffer_desc   mic1_token;
   gss_buffer_desc   mic2_token;
   gss_buffer_desc   message;
   OM_uint32	     maj_stat, min_stat;
   gss_qop_t	     qop;
   size_t            buf_elems;
   Ulong	     time_i2a_sign[SIGN_CYCLES];
   Ulong	     time_i2a_verify[SIGN_CYCLES];
   Ulong	     time_a2i_sign[SIGN_CYCLES];
   Ulong	     time_a2i_verify[SIGN_CYCLES];
   Ulong	     tval;
   static int	     cntr = 1;
   int		     rc   = 0;
   size_t	     i;
   Ushort	   * buf1 = NULL;
   Ushort	   * buf2 = NULL;
   Ushort            ranval;

   /* first, create two buffers of the requested size */

   mic1_token.value  = NULL;
   mic1_token.length = 0;
   mic2_token.value  = NULL;
   mic2_token.length = 0;

   buf_elems = (msg_size + 1) / 2;
   buf1      = malloc( buf_elems * 2 );
   buf2      = malloc( buf_elems * 2 );

   if ( buf1==NULL || buf2==NULL ) {
      fprintf(stdout, "ERROR:  malloc(%lu) failed: %s\n", (Ulong) (buf_elems * 2), strerror(errno) );
      rc = 1;
      goto error;
   }

   /* and fill both buffers with random data */
   srand( (unsigned) time(NULL) );
   for ( i=0 ; i<buf_elems ; i++ ) {
      ranval = (Ushort) rand();
      buf1[i] = ranval;
      buf2[i] = (ranval ^ 0xa5c3) - 33;
   }

   init_timer();

   for ( i=0 ; i<SIGN_CYCLES ; i++ ) {

      message.value     = (void *)buf1;
      message.length    = msg_size;
      mic1_token.value  = NULL;
      mic1_token.length = 0;

      start_timer();
      maj_stat = gss_get_mic( &min_stat, ini_ctx, 0, &message, &mic1_token );
      time_i2a_sign[i] = read_timer();
      if ( maj_stat!=GSS_S_COMPLETE ) {
	 print_status("gss_get_mic", GSS_C_NO_OID, maj_stat, min_stat);
	 rc = 1;
	 goto error;
      }

      message.value     = (void *)buf2;
      message.length    = msg_size;
      mic2_token.value  = NULL;
      mic2_token.length = 0;

      start_timer();
      maj_stat = gss_get_mic( &min_stat, acc_ctx, 0, &message, &mic2_token );
      time_a2i_sign[i] = read_timer();
      if ( maj_stat!=GSS_S_COMPLETE ) {
	 print_status("gss_get_mic", GSS_C_NO_OID, maj_stat, min_stat);
	 rc = 1;
	 goto error;
      }

      message.value     = (void *)buf1;
      message.length    = msg_size;
      start_timer();
      maj_stat = gss_verify_mic( &min_stat, acc_ctx, &message, &mic1_token, &qop );
      time_i2a_verify[i] = read_timer();
      if ( maj_stat!=GSS_S_COMPLETE ) {
	 print_status("gss_verify_mic", GSS_C_NO_OID, maj_stat, min_stat);
	 rc = 1;
	 goto error;
      }

      message.value     = (void *)buf2;
      message.length    = msg_size;
      start_timer();
      maj_stat = gss_verify_mic( &min_stat, ini_ctx, &message, &mic2_token, &qop );
      time_a2i_verify[i] = read_timer();
      if ( maj_stat!=GSS_S_COMPLETE ) {
	 print_status("gss_verify_mic", GSS_C_NO_OID, maj_stat, min_stat);
	 rc = 1;
	 goto error;
      }

      rc |= drop_buffer( &mic1_token );
      rc |= drop_buffer( &mic2_token );

   } /* end for() */

   tval = sample_avg( time_i2a_sign,   ARRAY_ELEMENTS(time_i2a_sign)   );
   fprintf(stdout, "Performance of get_mic    ini-->acc  (%8lu Bytes) = %s\n",
		   (Ulong) msg_size, sprint_timer(tval) );

   tval = sample_avg( time_i2a_verify, ARRAY_ELEMENTS(time_i2a_verify) );
   fprintf(stdout, "Performance of verify_mic ini-->acc  (%8lu Bytes) = %s\n",
		   (Ulong) msg_size, sprint_timer(tval) );

   tval = sample_avg( time_a2i_sign,   ARRAY_ELEMENTS(time_a2i_sign)   );
   fprintf(stdout, "Performance of get_mic    acc-->ini  (%8lu Bytes) = %s\n",
		   (Ulong) msg_size, sprint_timer(tval) );

   tval = sample_avg( time_a2i_verify, ARRAY_ELEMENTS(time_a2i_verify) );
   fprintf(stdout, "Performance of verify_mic acc-->ini  (%8lu Bytes) = %s\n",
		   (Ulong) msg_size, sprint_timer(tval) );

error:
   if ( buf1!=NULL ) { free(buf1);  buf1 = NULL; }
   if ( buf2!=NULL ) { free(buf2);  buf2 = NULL; }

   rc |= drop_buffer( &mic1_token );
   rc |= drop_buffer( &mic2_token );

   return(rc);

} /* signed_message_speed() */





/*
 * transfer_wrapped_message()
 *
 *
 */
int
transfer_wrapped_message( int    count,  int  do_conf,
			 char * src_name, gss_ctx_id_t src_ctx,
			 char * dst_name, gss_ctx_id_t dst_ctx )
{
   gss_buffer_desc   wrap_token;
   gss_buffer_desc   message;
   gss_buffer_desc   out_message;
   OM_uint32	     maj_stat, min_stat;
   gss_qop_t	     qop;
   static int	     cntr = 1;
   int		     rc   = 0;
   int		     i;
   int               conf;
   char		     sendmsg[256];

   wrap_token.value   = NULL;
   wrap_token.length  = 0;
   out_message.value  = NULL;
   out_message.length = 0;

   for ( i=1 ; i<count+1 ; i++ ) {

      fprintf(stdout, " wrapped message #%02d from %s to %s:\n", i, src_name, dst_name );
      sprintf(sendmsg , "This is test message #%04d\n", cntr);
      message.value      = sendmsg;
      message.length     = strlen(sendmsg);

      maj_stat = gss_wrap( &min_stat, src_ctx, do_conf, 0,
			   &message, &conf, &wrap_token );
      print_status("gss_wrap", GSS_C_NO_OID, maj_stat, min_stat);
      if (maj_stat!=GSS_S_COMPLETE) {
	 rc |= 1;
	 break;

      }

      maj_stat = gss_unwrap( &min_stat, dst_ctx, &wrap_token,
			     &out_message, &conf, &qop );
      print_status("gss_unwrap", GSS_C_NO_OID, maj_stat, min_stat);
      if (maj_stat!=GSS_S_COMPLETE) {
	 rc |= 1;
	 break;
      }

      if ( message.length!=out_message.length
	   ||  memcmp(message.value, out_message.value, message.length) ) {
	 fprintf(stdout, "message comparison failed for unwrapped message!\n");
	 rc |= 1;
	 break;
      }

      if ( (do_conf!=0 && conf==0) || (do_conf==0 && conf!=0) ) {
	 fprintf(stdout, "confidentiality mismatch!  in=%s  out=%s\n",
	         (do_conf==0) ? "FALSE" : "TRUE",
		 (conf==0)    ? "FALSE" : "TRUE" );
	 rc |= 1;
	 break;
      }

      cntr++;
      rc |= drop_buffer( &wrap_token  );
      rc |= drop_buffer( &out_message );

   } /* end for() */

   rc |= drop_buffer( &wrap_token );
   rc |= drop_buffer( &out_message );

   return(rc);

} /* transfer_wrapped_message() */




/*
 * inquire_context()
 *
 *
 */
int
inquire_context( char * type_of_context, gss_ctx_id_t ctx )
{
   gss_name_t    src_name    = GSS_C_NO_NAME;
   gss_name_t    targ_name   = GSS_C_NO_NAME;
   gss_OID       mech_oid    = GSS_C_NO_OID;
   OM_uint32	 ctx_flags   = 0;
   OM_uint32     lifetime    = 0;
   OM_uint32     maj_stat, min_stat;
   int           local_ini   = 0;
   int           pending     = 0;
   int		 rc          = 0;
   char		 tmpbuf[256];

   maj_stat = gss_inquire_context( &min_stat, ctx, &src_name, &targ_name,
				   &lifetime, &mech_oid, &ctx_flags,
				   &local_ini, &pending );
   print_status("gss_inquire_context", GSS_C_NO_OID, maj_stat, min_stat);
   if (maj_stat!=GSS_S_COMPLETE) {

      rc = 1;

   } else { /* maj_stat==GSS_S_COMPLETE */
      sprintf(tmpbuf, "gss_inquire_context(%.100s) returned lifetime", type_of_context);
      print_lifetime(tmpbuf, lifetime);

      if ( src_name==GSS_C_NO_NAME ) {
	 if (pending==0) {
	    fprintf(stdout, "  ERROR: missing src_name!\n");
	    rc = 1;
	 }
      } else {
	 rc |= display_name( "inquire_ctx source", src_name );
      }

      if ( targ_name==GSS_C_NO_NAME ) {
	 if ( pending==0) {
	    fprintf(stdout, " ERROR: missing targ_name!\n");
	    rc = 1;
	 }
      } else {
	 rc |= display_name( "inquire_ctx target", targ_name );
      }

	    
   } /* maj_stat==GSS_S_COMPLETE */

   rc |= drop_name( &src_name );
   rc |= drop_name( &targ_name );

   return(rc);

} /* inquire_context() */


/*
 * check_context_lifetime()
 *
 *
 */
int
check_context_lifetime( char * type_of_context, gss_OID mech, gss_ctx_id_t ctx )
{
   OM_uint32   lifetime = 0;
   OM_uint32   maj_stat, min_stat;
   int         rc = 0;
   char        tmpbuf[256];

   maj_stat = gss_context_time( &min_stat, ctx, &lifetime );
   print_status("gss_context_time", mech, maj_stat, min_stat );

   if ( maj_stat!=GSS_S_COMPLETE ) {

      rc = 1;

   } else {

      sprintf(tmpbuf, "gss_context_time() returned %10s context lifetime",
		     type_of_context );
      print_lifetime(tmpbuf, lifetime);

   }

   return(rc);

} /* check_context_lifetime() */


/*
 * display_name()
 *
 *
 */
int
display_name( char * type_of_name, gss_name_t name )
{
   gss_buffer_desc   dsp_name;
   gss_OID	     nt_oid	 = GSS_C_NO_OID;
   OM_uint32         maj_stat, min_stat;
   int		     rc = 0;

   dsp_name.length = 0;
   dsp_name.value  = NULL;

   maj_stat = gss_display_name( &min_stat, name, &dsp_name, &nt_oid );
   print_status("gss_display_name", GSS_C_NO_OID, maj_stat, min_stat);
   if (maj_stat!=GSS_S_COMPLETE) {

      rc = 1;

   } else {

      fprintf(stdout, "  displaying %s name => '%.*s'\n",
			 type_of_name, (int)(dsp_name.length), (char *)(dsp_name.value) );
   }

   rc |= drop_buffer(&dsp_name);

   return(rc);

} /* display_name() */


/*
 * create_context()
 *
 *
 */
int
create_context( gss_name_t      p_target,         gss_OID          p_mech,
	        OM_uint32	p_recflags,	  OM_uint32	   p_lifetime,
	        gss_cred_id_t   p_ini_cred,       gss_cred_id_t    p_acc_cred,
	        gss_name_t    * pp_source,
		gss_ctx_id_t  * pp_ini_ctx,       gss_ctx_id_t   * pp_acc_ctx,
		gss_OID	      * pp_ini_mech,      gss_OID        * pp_acc_mech,
	        OM_uint32     *	pp_ini_retflags,  OM_uint32	 * pp_acc_retflags,
		OM_uint32     * pp_ini_lifetime,  OM_uint32	 * pp_acc_lifetime )
{
   gss_buffer_desc   ini_token;
   gss_buffer_desc   acc_token;
   OM_uint32	     ini_maj_stat, acc_maj_stat;
   OM_uint32	     ini_min_stat, acc_min_stat;
   Ulong	     now;
   Ulong	     delay[32];
   Uint		     call_count = 0;
   Uint		     i;
   int		     rc    = 0;

   (*pp_ini_ctx)      = (*pp_acc_ctx)      = GSS_C_NO_CONTEXT;
   (*pp_ini_mech)     = (*pp_acc_mech)     = GSS_C_NO_OID;
   (*pp_ini_retflags) = (*pp_acc_retflags) = 0;
   (*pp_ini_lifetime) = (*pp_acc_lifetime) = 0;
   (*pp_source)       = GSS_C_NO_NAME;

   ini_token.value  = NULL;
   ini_token.length = 0;
   acc_token.value  = NULL;
   acc_token.length = 0;

   start_timer();

   do {
      now = read_timer();
      ini_maj_stat = gss_init_sec_context( &ini_min_stat, p_ini_cred, pp_ini_ctx,
					   p_target, p_mech,
					   p_recflags, p_lifetime,
					   GSS_C_NO_CHANNEL_BINDINGS,
					   (acc_token.length==0) ? GSS_C_NO_BUFFER : &acc_token,
					   pp_ini_mech, &ini_token, pp_ini_retflags,
					   pp_ini_lifetime );
      delay[call_count++] = read_timer() - now;
      print_status("gss_init_sec_context", p_mech, ini_maj_stat, ini_min_stat);
      rc |= drop_buffer( &acc_token );
      if ( GSS_ERROR(ini_maj_stat)!=GSS_S_COMPLETE )
	 goto error;

      if ( (GSS_SUPPLEMENTARY_INFO(ini_maj_stat)&GSS_S_CONTINUE_NEEDED)!=0
	   && ini_token.length==0 ) {
	 fprintf(fh, "ERROR: gss_init_sec_context() returns CONTINUE_NEEDED but no token!\n");
	 goto error;
      }
      if ( ini_token.length>0 ) {
	 now = read_timer();
	 /* sleep(1); */
	 acc_maj_stat = gss_accept_sec_context( &acc_min_stat, pp_acc_ctx,
						p_acc_cred, &ini_token,
						GSS_C_NO_CHANNEL_BINDINGS,
						pp_source, pp_acc_mech,
						&acc_token, pp_acc_retflags,
						pp_acc_lifetime, NULL );

	 delay[call_count++] = read_timer() - now;
         print_status("gss_accept_sec_context", p_mech, acc_maj_stat, acc_min_stat);
	 rc |= drop_buffer( &ini_token );
	 if ( GSS_ERROR(acc_maj_stat)!=GSS_S_COMPLETE )
	    goto error;

	 if ( (GSS_SUPPLEMENTARY_INFO(acc_maj_stat)&GSS_S_CONTINUE_NEEDED)!=0
	      && acc_token.length==0 ) {
	    fprintf(fh, "ERROR: gss_accept_sec_context() returns CONTINUE_NEEDED but no token!\n");
	    goto error;
	 }
	 /* sleep(1); */
      }  /* endif (ini_token.length>0) */

   } while ( (GSS_SUPPLEMENTARY_INFO(ini_maj_stat)&GSS_S_CONTINUE_NEEDED)!=0 );

   if ( ini_maj_stat!=GSS_S_COMPLETE ) {
      fprintf(fh, "ERROR: context establishment loop ended with\n",
		  "   ini_maj_stat!=GSS_S_COMPLETE (0x%08lx)!\n",
		  (unsigned long)ini_maj_stat);
      rc = 1;
   }

   if ( acc_maj_stat!=GSS_S_COMPLETE ) {
      fprintf(fh, "ERROR: context establishment loop ended with\n"
		  "   acc_maj_stat!=GSS_S_COMPLETE (0x%08lx)!\n",
		  (unsigned long) acc_maj_stat);
      rc = 1;
   }

   if ( ini_token.length!=0 ) {
      fprintf(fh, "ERROR: ini_token left over from context establishment!\n");
      rc = 1;
   }

   if ( acc_token.length!=0 ) {
      fprintf(fh, "ERROR: acc_token left over from context establishment!\n");
      rc = 1;
   }

   rc |= drop_buffer( &ini_token );
   rc |= drop_buffer( &acc_token );

   if ( rc==0 ) {
      Ulong ini_sum   = 0;
      Ulong acc_sum   = 0;
      Ulong all_sum   = 0;
      Uint  ini_cnt   = 0;
      Uint  acc_cnt   = 0;
      Uint  num_exchg = 0;

      for ( i=0 ; i<call_count ; i++ ) {
	 if ( i%2==0 ) {
	    ini_sum += delay[i];
	    ini_cnt++;
	 } else {
	    acc_sum += delay[i];
	    acc_cnt++;
	 }
	 all_sum += delay[i];
      }

      num_exchg = call_count - 1; /* Fencepost warning: 4 calls mean 3 message exchanges ... */

      fprintf(fh, "  %u-way Authentication completed in                  %s\n",
	          num_exchg, sprint_timer(all_sum) );
      fprintf(fh, "    %u calls to gss_init_sec_context() amounting to   %s\n",
		  ini_cnt, sprint_timer(ini_sum) );
      fprintf(fh, "    %u calls to gss_accept_sec_context() amounting to %s\n",
		  acc_cnt, sprint_timer(acc_sum) );
   }

   return(rc);

error:
   drop_context( pp_acc_ctx );
   drop_context( pp_ini_ctx );
   drop_name( pp_source );
   (*pp_ini_mech)     = (*pp_acc_mech)     = GSS_C_NO_OID;
   (*pp_ini_lifetime) = (*pp_acc_lifetime) = 0;
   (*pp_ini_retflags) = (*pp_acc_retflags) = 0;

   rc = 1;

   return(rc);

} /* create_context() */




/*
 * exp_imp_context()
 *
 *
 */
int
exp_imp_context( Uint count,
		 char * type_of_context, gss_OID mech, gss_ctx_id_t * ctx )
{
   gss_buffer_desc   exp_context;
   OM_uint32	     maj_stat, min_stat;
   unsigned long     time_diff;
   unsigned int      i;
   int               rc = 0;

   exp_context.value  = NULL;
   exp_context.length = 0;

   if ( count==1 ) {
      start_timer();
      maj_stat = gss_export_sec_context( &min_stat, ctx, &exp_context );

      if ( maj_stat==GSS_S_COMPLETE) {
	 maj_stat = gss_import_sec_context( &min_stat, &exp_context, ctx );
      }

      drop_buffer( &exp_context );
      time_diff = read_timer();
   }
      
   exp_context.value  = NULL;
   exp_context.length = 0;

   start_timer();

   for ( i=0 ; i<count ; i++ ) {
      maj_stat = gss_export_sec_context( &min_stat, ctx, &exp_context );
      if (count==1 || maj_stat!=GSS_S_COMPLETE)
	 print_status("gss_export_sec_context", mech, maj_stat, min_stat);

      if (maj_stat!=GSS_S_COMPLETE) {

         rc = 1;
	 break;
	       
      }

      if ( *ctx!=NULL ) {
	 fprintf(fh, "ERROR: context handle wasn't ZEROed!\n");
	 *ctx = NULL;
      }
      if ( count==1 )
         fprintf(fh,"  context export succeeded for %.50s.\n", type_of_context);

      maj_stat = gss_import_sec_context( &min_stat, &exp_context, ctx );
      if (count==1 || maj_stat!=GSS_S_COMPLETE)
         print_status("gss_import_sec_context", mech, maj_stat, min_stat);

      if (maj_stat!=GSS_S_COMPLETE) {
	 rc = 1;
	 break;
      }
      if ( count==1 )
	 fprintf(fh, "  context import succeeded for %.50s.\n", type_of_context);

      rc |= drop_buffer( &exp_context );

   }

   if ( rc==0 ) { 
      if ( count!=1 ) time_diff = read_timer();
      fprintf(fh, "  context import succeeded %d times for %.50s.\n",
		  i, type_of_context);
      fprintf(fh, "  average time per export+import cycle = %s\n",
		  sprint_timer(time_diff/i) );
   }

   rc |= drop_buffer( &exp_context );

   return(rc);

} /* exp_imp_context() */




/*
 *
 *
 */
int
drop_buffer( gss_buffer_desc * buf )
{
   OM_uint32  min_stat, maj_stat;
   int        rc = 0;

   if ( buf->length!=0 && buf->value!=NULL ) {

      maj_stat = gss_release_buffer( &min_stat, buf );
      if (maj_stat!=GSS_S_COMPLETE)
	 print_status("gss_release_buffer", GSS_C_NO_OID, maj_stat, min_stat);

      if ( maj_stat!=GSS_S_COMPLETE ) {
	 rc = 1;
      } else {
	 if ( buf->length!=0 ) {
	    fprintf(fh, "((gss_buffer_t)->length  was not zeroed!\n");
	    rc = 1;
	    buf->length = 0;
	 }
      }

   }

   buf->length = 0;
   buf->value  = NULL;

   return(rc);

} /* drop_buffer() */



/*
 *
 *
 */
int
drop_name( gss_name_t * name )
{
   OM_uint32 min_stat, maj_stat;
   int  rc = 0;

   if ( *name!=GSS_C_NO_NAME ) {

      maj_stat = gss_release_name( &min_stat, name );
      print_status("gss_release_name", GSS_C_NO_OID, maj_stat, min_stat);

      if ( maj_stat!=GSS_S_COMPLETE ) {
	 rc = 1;
      }
   }

   if ( *name!=GSS_C_NO_NAME ) {
     fprintf(fh, "(gss_name_t)out_name   was not zeroed!\n");
     *name = GSS_C_NO_NAME;
     rc = 1;
   }

   return(rc);

} /* drop_name() */




/*
 *
 *
 */
int
drop_oid_set( gss_OID_set * oid_set )
{
   OM_uint32   min_stat, maj_stat;
   int         rc = 0;
  
   if ( *oid_set!=GSS_C_NO_OID_SET ) {
      maj_stat = gss_release_oid_set( &min_stat, oid_set );
      print_status("gss_release_oid_set", GSS_C_NO_OID, maj_stat, min_stat);
      if (maj_stat!=GSS_S_COMPLETE) {
	 rc = 1;
      }
      if ( *oid_set!=GSS_C_NO_OID_SET ) {
	 fprintf(fh, "gss_release_oid_set() didn't clear handle !!\n");
	 *oid_set = GSS_C_NO_OID_SET;
	 rc = 1 ;
      }
   }

   return(rc);

} /* drop_oid_set() */




/*
 *
 *
 */
int
drop_cred( gss_cred_id_t * cred )
{
   OM_uint32 min_stat, maj_stat;
   int  rc = 0;

   if ( *cred!=GSS_C_NO_CREDENTIAL ) {
      maj_stat = gss_release_cred( &min_stat, cred );
      print_status("gss_release_cred", GSS_C_NO_OID, maj_stat, min_stat);
      if ( maj_stat!=GSS_S_COMPLETE ) {
	 rc = 1;
      }
   }

   if ( *cred!=GSS_C_NO_CREDENTIAL ) {
     fprintf(fh, "(gss_cred_id_t)cred   was not zeroed!\n");
     rc = 1;
     *cred = GSS_C_NO_CREDENTIAL;
   }

   return(rc);

} /* drop_cred() */




/*
 * drop_context()
 *
 *
 */
int
drop_context( gss_ctx_id_t  * ctx )
{
   OM_uint32 min_stat, maj_stat;
   int  rc = 0;

   if ( *ctx!=GSS_C_NO_CONTEXT ) {
      maj_stat = gss_delete_sec_context( &min_stat, ctx, NULL );
      print_status("gss_delete_sec_context", GSS_C_NO_OID, maj_stat, min_stat);
      if ( maj_stat!=GSS_S_COMPLETE ) {
	 rc = 1;
      }
   }

   if ( *ctx!=GSS_C_NO_CONTEXT ) {
     fprintf(fh, "(gss_ctx_id_t)context   was not zeroed!\n");
     rc = 1;
     *ctx = GSS_C_NO_CONTEXT;
   }

   return(rc);

} /* drop_context() */




static char *
calling_error[] = {
      "",
      "GSS_S_CALL_INACCESSIBLE_READ",
      "GSS_S_CALL_INACCESSIBLE_WRITE",
      "GSS_S_CALL_BAD_STRUCTURE"
};

static char *
routine_error[] = {
      "GSS_S_COMPLETE",
      "GSS_S_BAD_MECH",
      "GSS_S_BAD_NAME",
      "GSS_S_BAD_NAMETYPE",
      "GSS_S_BAD_BINDINGS",
      "GSS_S_BAD_STATUS",
      "GSS_S_BAD_MIC",
      "GSS_S_NO_CRED",
      "GSS_S_NO_CONTEXT",
      "GSS_S_DEFECTIVE_TOKEN",
      "GSS_S_DEFECTIVE_CREDENTIAL",
      "GSS_S_CREDENTIALS_EXPIRED",
      "GSS_S_CONTEXT_EXPIRED",
      "GSS_S_FAILURE",
      "GSS_S_BAD_QOP",
      "GSS_S_UNAUTHORIZED",
      "GSS_S_UNAVAILABLE",
      "GSS_S_DUPLICATE_ELEMENT",
      "GSS_S_NAME_NOT_MN"
};

static char *
suppl_information[] = {
      "GSS_S_CONTINUE_NEEDED",
      "GSS_S_DUPLICATE_TOKEN",
      "GSS_S_OLD_TOKEN",
      "GSS_S_UNSEQ_TOKEN"
};


void
print_status( char       * call_name,
	      gss_OID      mech_oid,
	      OM_uint32    major_status,
	      OM_uint32    minor_status )
{
   gss_buffer_desc   message;
   char		     tmpbuf[1024];
   OM_uint32	     context;
   OM_uint32	     min_stat, maj_stat;
   OM_uint32	     calling;
   OM_uint32	     routine;
   OM_uint32	     supplinfo;
   Uint		     idx;
   Uint		     cntr;
   Uint		     len;

   message.length = 0;
   message.value  = NULL;
   tmpbuf[0]      = '\0';

   calling   = (GSS_CALLING_ERROR(major_status)>>GSS_C_CALLING_ERROR_OFFSET);
   routine   = (GSS_ROUTINE_ERROR(major_status)>>GSS_C_ROUTINE_ERROR_OFFSET);
   supplinfo = GSS_SUPPLEMENTARY_INFO(major_status)>>GSS_C_SUPPLEMENTARY_OFFSET;

   strcat(tmpbuf, "(");
   cntr = 0;

   if ( major_status==GSS_S_COMPLETE ) {

      strcat(tmpbuf, routine_error[0]);

   } else {

      if ( calling!=0 ) {
	 if ( calling >= ARRAY_ELEMENTS(calling_error) ) {
	    fprintf(fh, "%s() returned unknown calling error %u!\n",
			call_name, calling);
	 } else {
	    strcat(tmpbuf, calling_error[calling]);
	    cntr++;
	 }
      }

      if ( routine!=0 ) {
	 if ( routine >= ARRAY_ELEMENTS(routine_error) ) {
	    fprintf(fh, "%s() returned unknown routine error %u!\n",
		        call_name, routine);
	 } else {
	    if (cntr>0)
	       strcat(tmpbuf, "|");
	    strcat(tmpbuf, routine_error[routine]);
	    cntr++;
	 }
      }

      if ( supplinfo!=0 ) {
	 for ( idx=0 ; idx<ARRAY_ELEMENTS(suppl_information) ; idx++ ) {
	    if ( (supplinfo&1)!=0 ) {
	       if (cntr>0)
		  strcat(tmpbuf, "|");
	       strcat(tmpbuf, suppl_information[idx]);
	       cntr++;
	    }
	    supplinfo /= 2;
	 }
	 if ( supplinfo!=0 ) {
	    fprintf(fh, "%s() returned unknown supplementary information!\n",
			call_name );
	 }
      }

   }

   strcat(tmpbuf,")");
   len = (Uint)strlen(tmpbuf);
   fprintf( fh, "%s() == %s%s\n",
	    call_name, len>50 ? "\n        " : "", tmpbuf );

   /* don't print status codes while everything is fine */
   if ( major_status!=GSS_S_COMPLETE ) {
      context = 0;
      fprintf(fh, "gss_display_status(0x%08lx,GSS_S_GSS_CODE) =\n", major_status);
      fflush(fh);

      do {
	 message.value  = NULL;
	 message.length = 0;

	 maj_stat = gss_display_status( &min_stat, major_status,
				        GSS_C_GSS_CODE, GSS_C_NO_OID,
				        &context, &message );
	 if (maj_stat==GSS_S_COMPLETE) {
	    if (message.value==NULL || message.length==0 ) {
	       fprintf(fh, "ERROR: missing output from gss_display_status()!\n");
	    } else {
	       fprintf(fh, "  >> %.*s\n", message.length, (char *)message.value );
	    }
	 }
	 fflush(fh);
	 if ( message.value!=NULL && message.length>0 ) {
	    (void) gss_release_buffer( &min_stat, &message );
	 }

      } while ( context!=0 && maj_stat==GSS_S_COMPLETE );

      /* don't print minor status information if there isn't any */
      if (minor_status!=0) {

         context = 0;
         fprintf(fh, "gss_display_status(0x%08lx,GSS_S_MECH_CODE) =\n", major_status);
	 fflush(fh);
	 do {
	    message.value  = NULL;
	    message.length = 0;

	    maj_stat = gss_display_status( &min_stat, minor_status,
					   GSS_C_MECH_CODE, GSS_C_NO_OID,
					   &context, &message );
	    if (maj_stat==GSS_S_COMPLETE) {
	       if (message.value==NULL || message.length==0 ) {
		  fprintf(fh, "ERROR: missing output from gss_display_status()!\n");
	       } else {
		  fprintf(fh, "  >> %.*s\n", message.length, (char *)message.value );
	       }
	    }
	    fflush(fh);
	    if ( message.value!=NULL && message.length>0 ) {
	       (void) gss_release_buffer( &min_stat, &message );
	    }

	 } while ( context!=0 && maj_stat==GSS_S_COMPLETE );

      } /* endif (minor_status!=0) */

   } /* endif (major_status!=0) */

   return;

} /* print_status_codes() */




void
print_lifetime(char * prefix, OM_uint32 p_lifetime)
{
   char   * fmt = "%s: %02dh %02dm %02ds\n";
   Ulong  hours, mins, secs;

   hours = mins = secs = 0;

   if (p_lifetime==GSS_C_INDEFINITE) {
      
      fmt = "%.150s: Indefinite\n";

   } else if (p_lifetime==0) {

      fmt = "%.150s: Expired\n";

   } else {

      hours= p_lifetime/3600;
      mins = (p_lifetime - hours*3600)/60;
      secs = (p_lifetime - hours*3600 - mins*60);

   }

   fprintf(fh, fmt, prefix, (int)hours, (int)mins, (int)secs);

   return;

} /* print_lifetime() */


