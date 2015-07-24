#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/gssmain.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/gssmain.c#2 $
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


#include "gssmaini.h"

 
gss_OID_desc gn_gss_oids[] = {
  {  6, (void *)"\x2b\x06\x01\x05\x06\x02"			}, /* IANA: hostbased service (deprecated) */
  {  6, (void *)"\x2b\x06\x01\x05\x06\x03"			}, /* IANA: anonymous name     */
  {  6, (void *)"\x2b\x06\x01\x05\x06\x04"			}, /* IANA: exported name      */
  { 10, (void *)"\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x01"	}, /* MIT: Printable User name */
  { 10, (void *)"\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x02"	}, /* MIT: Machine UID name    */
  { 10, (void *)"\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x03"	}, /* MIT: String UID name     */
  { 10, (void *)"\052\206\110\206\367\022\001\002\001\004"	}, /* MIT: Hostbased Service   */
  {  0, NULL							}
};

int gn_gss_oids_num = ARRAY_ELEMENTS(gn_gss_oids);

int gn_gss_initialized = FALSE;

/****************************************************************/
/* Official & standardized GSS-API Name Types, exported as data */
/****************************************************************/

gss_OID EXPORT_DATA GSS_C_NT_HOSTBASED_SERVICE_X	= &(gn_gss_oids[0]);
gss_OID EXPORT_DATA GSS_C_NT_ANONYMOUS			= &(gn_gss_oids[1]);
gss_OID EXPORT_DATA GSS_C_NT_EXPORT_NAME		= &(gn_gss_oids[2]);
gss_OID EXPORT_DATA GSS_C_NT_USER_NAME			= &(gn_gss_oids[3]);
gss_OID EXPORT_DATA GSS_C_NT_MACHINE_UID_NAME		= &(gn_gss_oids[4]);
gss_OID EXPORT_DATA GSS_C_NT_STRING_UID_NAME		= &(gn_gss_oids[5]);
gss_OID EXPORT_DATA GSS_C_NT_HOSTBASED_SERVICE		= &(gn_gss_oids[6]);



/* gss_display_status() */

OM_uint32 EXPORT_FUNCTION
gss_display_status(
	OM_uint32	FAR *	minor_status,		/* out */
	OM_uint32		status_value,		/* in  */
	int			status_type,		/* in  */
	gss_OID			mech_type,		/* in  */
	OM_uint32	FAR *	message_context,	/* out */
	gss_buffer_t		status_string		/* out */
     )
{
   OM_uint32  major_status;
   DEBUG_BEGIN_VARS( )

   TRY_INIT_GSSAPI( );

   DEBUG_BEGIN(display_status)

   DEBUG_ARG((tf, "\t\t status_value          = 0x%08lx,\n"
	          "\t\t status_type           = %d,\n"
	          "\t\t mech_type             = ptr:%p,\n"
	          "\t\t&message_context       = ptr:%p,\n"
	          "\t\t status_string         = ptr:%p\n"
	          "\t)\n",
	          (unsigned long)status_value,
	          (int)status_type,
	          mech_type,
	          message_context,
	          status_string ))

   DEBUG_ARG_OID(     mech_type     )
   DEBUG_ARG_BUFHEAD( status_string )

   CLEAR_OUT_BUFFER(  status_string );

   major_status = gn_gss_display_status( minor_status, status_value,
					 status_type, mech_type,
					 message_context, status_string );

   DEBUG_RET( )
   DEBUG_RET_BOOL_REF( message_context )
   DEBUG_RET_BUF(      status_string   )
 
   return(major_status);

} /* gss_display_status() */




/*
 * gss_release_oid_set()
 *
 * Status: ** done (30-Aug-1996) mrex
 *         spec under discussion
 */
OM_uint32 EXPORT_FUNCTION
gss_release_oid_set(
        OM_uint32	FAR *	minor_status,		/* out    */
        gss_OID_set	FAR *	set			/* in/out */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   TRY_INIT_GSSAPI( );

   DEBUG_BEGIN(release_oid_set)

   DEBUG_ARG((tf, "\t\t&oid_set               = ptr:%p\n"
		  "\t)\n",
	          set ))

   DEBUG_ARG_OID_SET_REF( set )

   major_status = gn_gss_release_oid_set(minor_status, set);

   DEBUG_RET( )
   DEBUG_RET_OID_SET_REF( set )

   return(major_status);

} /* gss_release_oid_set() */



/*
 * gss_indicate_mechs()
 *
 * Status:  ** done (30-Aug-1996) mrex
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_indicate_mechs(
        OM_uint32	FAR *	minor_status,		/* out */
        gss_OID_set	FAR *	mech_set		/* out */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_OID_SET( mech_set );

   INIT_GSSAPI( );

   DEBUG_BEGIN(indicate_mechs)

   DEBUG_ARG((tf, "\t\t&mech_set              = ptr:%p\n"
		  "\t)\n",
	           mech_set ))

   major_status = gn_gss_indicate_mechs(minor_status, mech_set);

   DEBUG_RET( )
   DEBUG_RET_OID_SET_REF( mech_set )	          

   return(major_status);

} /* gss_indicate_mechs() */



/*
 * gss_create_empty_oid_set()
 *
 * Status:  ** done (20-Aug-1996) mrex
 */
OM_uint32 EXPORT_FUNCTION
gss_create_empty_oid_set(
        OM_uint32	FAR *	minor_status,		/* out */
	gss_OID_set	FAR *	oid_set			/* out */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   TRY_INIT_GSSAPI( );

   DEBUG_BEGIN(create_empty_oid_set)

   DEBUG_ARG((tf, "\t\t&oid_set               = ptr:%p\n"
		  "\t)\n",
	          oid_set))

   CLEAR_OUT_OID_SET( oid_set );

   major_status = gn_gss_create_empty_oid_set(minor_status, oid_set);

   DEBUG_RET( )
   DEBUG_RET_OID_SET_REF( oid_set )

   return(major_status);

} /* gss_create_empty_oid_set() */


/*
 * gss_add_oid_set_member()
 *
 * Status:  **
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_add_oid_set_member(
	OM_uint32	FAR *	minor_status,		/* out    */
	gss_OID			member_oid,		/* in     */
	gss_OID_set	FAR *	oid_set			/* in/out */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   TRY_INIT_GSSAPI( );

   DEBUG_BEGIN(add_oid_set_member)

   DEBUG_ARG((tf, "\t\t member_oid            = ptr:%p,\n"
		  "\t\t&oid_set               = ptr:%p\n"
		  "\t)\n",
	          member_oid,
	          oid_set ))

   DEBUG_ARG_OID(         member_oid )
   DEBUG_ARG_OID_SET_REF( oid_set    )

   major_status = gn_gss_add_oid_set_member( minor_status,
					     member_oid, oid_set );

   DEBUG_RET( )
   DEBUG_RET_OID_SET_REF( oid_set )
	      
   return(major_status);

} /* gss_add_oid_set_member() */




/*
 * gss_import_name()
 *
 * Status:  **
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_import_name(
	OM_uint32	FAR *	minor_status,		/* out */
	gss_buffer_t		input_name_buffer,	/* in  */
        gss_OID			input_name_type,	/* in  */
        gss_name_t	FAR *	output_name		/* out */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_NAME( output_name );

   INIT_GSSAPI( );

   DEBUG_BEGIN(import_name)

   DEBUG_ARG((tf, "\t\t input_name_buffer     = ptr:%p,\n"
	          "\t\t input_name_type       = ptr:%p,\n"
		  "\t\t&output_name           = ptr:%p\n"
		  "\t)\n",
	          input_name_buffer,
	          input_name_type,
	          output_name ))

   DEBUG_ARG_BUF( input_name_buffer )
   DEBUG_ARG_OID( input_name_type   )

   major_status = gn_gss_import_name( minor_status, input_name_buffer,
				      input_name_type, output_name );

   DEBUG_RET( )
   DEBUG_RET_NAME_REF( output_name )

   return(major_status);

} /* gss_import_name() */



/*
 * gss_duplicate_name()
 *
 * Status: **
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_duplicate_name(
	OM_uint32	FAR *	minor_status,		/* out */
	gss_name_t		src_name,		/* in  */
	gss_name_t	FAR *   dest_name		/* out */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_NAME( dest_name );

   INIT_GSSAPI( );

   DEBUG_BEGIN(duplicate_name)

   DEBUG_ARG((tf, "\t\t src_name              = ptr:%p,\n"
	          "\t\t&dest_name             = ptr:%p\n"
		  "\t)\n",
	           src_name, dest_name ))

   DEBUG_ARG_NAME( src_name )

   major_status = gn_gss_duplicate_name( minor_status, src_name, dest_name );

   DEBUG_RET( )
   DEBUG_RET_NAME_REF( dest_name )

   return(major_status);

} /* gss_duplicate_name() */



/*
 * gss_release_name()
 *
 * Status: **
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_release_name(
	OM_uint32	FAR *	minor_status,		/* out    */
	gss_name_t	FAR *	in_name			/* in/out */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   TRY_INIT_GSSAPI( );

   DEBUG_BEGIN(release_name)

   DEBUG_ARG((tf, "\t\t&in_name               = ptr:%p\n"
		  "\t)\n",
	           in_name ))

   DEBUG_ARG_NAME_REF( in_name )

   major_status = gn_gss_release_name( minor_status, in_name );

   DEBUG_RET( )
   DEBUG_RET_NAME_REF( in_name )

   return(major_status);

} /* gss_release_name() */





/*
 * gss_release_buffer()
 *
 * Status: **
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_release_buffer(
        OM_uint32	FAR *	minor_status,		/* out    */
        gss_buffer_t		buffer			/* in/out */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   TRY_INIT_GSSAPI( );

   DEBUG_BEGIN(release_buffer)

   DEBUG_ARG((tf, "\t\t buffer                = ptr:%p\n"
		  "\t)\n",
	           (void *)buffer ))

   DEBUG_ARG_BUF( buffer )

   major_status = gn_gss_release_buffer( minor_status, buffer );

   DEBUG_RET( )
   DEBUG_RET_BUFHEAD( buffer )

   return(major_status);

} /* gss_release_buffer() */




/*
 * gss_context_time()
 *
 *
 */

OM_uint32 EXPORT_FUNCTION
gss_context_time(
	OM_uint32	FAR *	minor_status,		/* out  */
	gss_ctx_id_t		context_handle,		/* in   */
	OM_uint32	FAR *	time_rec		/* out  */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   INIT_GSSAPI( );

   DEBUG_BEGIN(context_time)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p,\n"
		  "\t\t&time_rec              = ptr:%p\n"
		  "\t)\n",
	          context_handle,
	          time_rec ))

   DEBUG_ARG_CTX( context_handle )

   major_status = gn_gss_context_time( minor_status,
				       context_handle, time_rec );

   DEBUG_RET( )
   DEBUG_RET_LIFET_REF( time_rec )

   return(major_status);

} /* gss_context_time() */



/*
 * gss_acquire_cred()
 *
 * Status:  under construction (23-Aug-1996) mrex
 */

OM_uint32 EXPORT_FUNCTION
gss_acquire_cred(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_name_t		desired_name,		/* in    */
	OM_uint32		time_req,		/* in    */
	gss_OID_set		desired_mechs,		/* in    */
	gss_cred_usage_t	cred_usage,		/* in    */
	gss_cred_id_t	FAR *	output_cred_handle,	/* out   */
	gss_OID_set	FAR *	actual_mechs,		/* out   */
	OM_uint32	FAR *	time_rec		/* out   */
     )
{
   OM_uint32    major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_CRED(    output_cred_handle );
   CLEAR_OUT_OID_SET( actual_mechs       );

   INIT_GSSAPI( );

   DEBUG_BEGIN(acquire_cred)

   DEBUG_ARG((tf, "\t\t desired_name          = ptr:%p,\n"
	          "\t\t time_req              = 0x%08lx,\n"
	          "\t\t desired_mechs         = ptr:%p,\n"
	          "\t\t cred_usage            = %d,\n"
	          "\t\t&output_cred_handle    = ptr:%p,\n"
	          "\t\t&actual_mechs          = ptr:%p,\n"
	          "\t\t&time_rec              = ptr:%p\n"
	          "\t)\n",
	       desired_name,
	       (unsigned long)time_req,
	       desired_mechs,
	       (int)cred_usage,
	       output_cred_handle,
	       actual_mechs,
	       time_rec ))

   DEBUG_ARG_NAME(    desired_name  )
   DEBUG_ARG_LIFET(   time_req      )
   DEBUG_ARG_OID_SET( desired_mechs )
   DEBUG_ARG_USAGE(   cred_usage    )

   major_status = gn_gss_acquire_cred( minor_status, desired_name,
				       time_req, desired_mechs,
				       cred_usage, output_cred_handle,
				       actual_mechs, time_rec );

   DEBUG_RET( )
   DEBUG_RET_CRED_REF(    output_cred_handle )
   DEBUG_RET_OID_SET_REF( actual_mechs       )
   DEBUG_RET_LIFET_REF(   time_rec           )

   return(major_status);

} /* gss_acquire_cred() */





/* ======================================================================= */
/* ======================================================================= */
/* ======================================================================= */
/* ======================================================================= */
/* ======================================================================= */





/*
 * gss_release_cred()
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_release_cred(
	OM_uint32	FAR *	minor_status,		/* out     */
	gss_cred_id_t	FAR *	cred_handle		/* out     */
     )
{
   OM_uint32    major_status;
   DEBUG_BEGIN_VARS( )

   INIT_GSSAPI( );

   DEBUG_BEGIN(release_cred)

   DEBUG_ARG((tf, "\t\t&cred_handle           = ptr:%p\n"
	          "\t)\n",
	          cred_handle ))

   DEBUG_ARG_CRED_REF( cred_handle )

   major_status = gn_gss_release_cred( minor_status, cred_handle );

   DEBUG_RET( )
   DEBUG_RET_CRED_REF( cred_handle )

   return(major_status);

} /* gss_release_cred() */



/* gss_init_sec_context() */

OM_uint32 EXPORT_FUNCTION
gss_init_sec_context(
	OM_uint32	FAR *	minor_status,		/* out    */
	gss_cred_id_t		initiator_cred_handle,	/* in     */
	gss_ctx_id_t	FAR *	context_handle,		/* in/out */
	gss_name_t		target_name,		/* in     */
	gss_OID			mech_type,		/* in     */
	OM_uint32		req_flags,		/* in     */
	OM_uint32		time_req,		/* in     */
	gss_channel_bindings_t
				input_chan_bindings,	/* in     */
	gss_buffer_t		input_token,		/* in     */
	gss_OID		FAR *	actual_mech_type,	/* out    */
	gss_buffer_t		output_token,		/* out    */
	OM_uint32	FAR *	ret_flags,		/* out    */
	OM_uint32	FAR *	time_rec		/* out    */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   TRY_INIT_GSSAPI( ); /* We catch mechanism INIT failures later on    */
		       /* to get the context handle correctly cleared  */

   DEBUG_BEGIN(init_sec_context)

   DEBUG_ARG((tf, "\t\t initiator_cred_handle = ptr:%p,\n"
		  "\t\t&context_handle        = ptr:%p,\n"
	          "\t\t target_name           = ptr:%p,\n"
	          "\t\t mech_type             = ptr:%p,\n"
	          "\t\t req_flags             = 0x%08lx,\n"
	          "\t\t time_req              = 0x%08lx,\n"
	          "\t\t input_chan_bindings   = ptr:%p,\n"
	          "\t\t input_token           = ptr:%p,\n"
	          "\t\t&acutal_mech_type      = ptr:%p,\n"
	          "\t\t output_token          = ptr:%p,\n"
	          "\t\t&ret_flags             = ptr:%p,\n"
	          "\t\t&time_rec              = ptr:%p\n"
	          "\t)\n",
	          initiator_cred_handle,
	          context_handle,
	          target_name,
	          mech_type,
	          (unsigned long)req_flags,
	          (unsigned long)time_req,
	          input_chan_bindings,
	          input_token,
	          actual_mech_type,
	          output_token,
	          ret_flags,
	          time_rec ))


   DEBUG_ARG_CRED(    initiator_cred_handle )
   DEBUG_ARG_CTX_REF( context_handle        )
   DEBUG_ARG_NAME(    target_name           )
   DEBUG_ARG_OID(     mech_type             )
   DEBUG_ARG_FLAGS(   req_flags             )
   DEBUG_ARG_LIFET(   time_req              )
   DEBUG_ARG_CHBIND(  input_chan_bindings   )
   DEBUG_ARG_BUF(     input_token           )
   DEBUG_ARG_BUFHEAD( output_token          )

   CLEAR_OUT_OID(    actual_mech_type );
   CLEAR_OUT_BUFFER( output_token     );

   if ( gn_avail_mech_oids.count==0 ) {
      /* Catch mechanism initialization failures here */
      CLEAR_OUT_CTX( context_handle );
      (*minor_status) = MINOR_NO_MECHANISM_AVAIL;
      return(GSS_S_FAILURE);
   }

   major_status = gn_gss_init_sec_context( minor_status, initiator_cred_handle,
					   context_handle, target_name,
					   mech_type, req_flags, time_req,
					   input_chan_bindings,
					   input_token, actual_mech_type,
					   output_token, ret_flags, time_rec );

   DEBUG_RET( )
   DEBUG_RET_LIFET_REF( time_rec         )
   DEBUG_RET_FLAGS_REF( ret_flags        )
   DEBUG_RET_OID_REF(   actual_mech_type )
   DEBUG_RET_CTX_REF(   context_handle   )
   DEBUG_RET_BUF(       output_token     )

   return(major_status);

} /* gss_init_sec_context() */





/*
 * gss_accept_sec_context()
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_accept_sec_context(
	OM_uint32	FAR *	minor_status,		/* out      */
	gss_ctx_id_t	FAR *	context_handle,		/* in/out   */
	gss_cred_id_t		acceptor_cred_handle,	/* in       */
	gss_buffer_t		input_token,		/* in       */
	gss_channel_bindings_t	input_chan_bindings,	/* in       */
	gss_name_t	FAR *	src_name,		/* in?/out  */
	gss_OID		FAR *	mech_type,		/* out      */
	gss_buffer_t		output_token,		/* (in)/out */
	OM_uint32	FAR *	ret_flags,		/* out      */
	OM_uint32	FAR *	time_rec,		/* out      */
	gss_cred_id_t	FAR *	delegated_cred_handle	/* out      */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   TRY_INIT_GSSAPI( ); /* We catch mechanism INIT failures later on    */
		       /* to get the context handle correctly cleared  */

   DEBUG_BEGIN(accept_sec_context)

   DEBUG_ARG((tf, "\t\t&context_handle        = ptr:%p,\n"
	          "\t\t acceptor_cred_handle  = ptr:%p,\n"
		  "\t\t input_token           = ptr:%p,\n"
	          "\t\t input_chan_bindings   = ptr:%p,\n"
	          "\t\t&src_name              = ptr:%p,\n"
	          "\t\t&mech_type             = ptr:%p,\n"
	          "\t\t output_token          = ptr:%p,\n"
	          "\t\t&ret_flags             = ptr:%p,\n"
	          "\t\t&time_rec              = ptr:%p,\n"
	          "\t\t&delegated_cred_handle = ptr:%p\n"
	          "\t)\n",
	          context_handle,
	          acceptor_cred_handle,
	          input_token,
	          input_chan_bindings,
	          src_name,
	          mech_type,
	          output_token,
	          ret_flags,
	          time_rec,
	          delegated_cred_handle ))

   DEBUG_ARG_CTX_REF( context_handle        )
   DEBUG_ARG_CRED(    acceptor_cred_handle  )
   DEBUG_ARG_BUF(     input_token           )
   DEBUG_ARG_CHBIND(  input_chan_bindings   )
   DEBUG_ARG_BUFHEAD( output_token          )
   DEBUG_ARG_CRED(    delegated_cred_handle )

   CLEAR_OUT_CRED(    delegated_cred_handle );
   CLEAR_OUT_OID(     mech_type             );
   CLEAR_OUT_BUFFER(  output_token          );
   CLEAR_OUT_NAME(    src_name		    );

   if ( gn_avail_mech_oids.count==0 ) {
      /* Catch mechanism initialization failures here */
      CLEAR_OUT_CTX( context_handle );
      (*minor_status) = MINOR_NO_MECHANISM_AVAIL;
      return(GSS_S_FAILURE);
   }

   major_status = gn_gss_accept_sec_context( minor_status, context_handle,
					     acceptor_cred_handle,
					     input_token,
					     input_chan_bindings, src_name,
					     mech_type, output_token,
					     ret_flags, time_rec,
					     delegated_cred_handle );

   DEBUG_RET( )
   DEBUG_RET_CTX_REF(   context_handle        )
   DEBUG_RET_CRED(      acceptor_cred_handle  )
   DEBUG_RET_NAME_REF(  src_name              )
   DEBUG_RET_OID_REF(   mech_type             )
   DEBUG_RET_BUF(       output_token          )
   DEBUG_RET_FLAGS_REF( ret_flags             )
   DEBUG_RET_LIFET_REF( time_rec              )
   DEBUG_RET_CRED_REF(  delegated_cred_handle )

   return(major_status);

} /* gss_accept_sec_context() */





/* gss_process_context_token() */

OM_uint32 EXPORT_FUNCTION
gss_process_context_token(
	OM_uint32	FAR *	minor_status,		/* out	 */
	gss_ctx_id_t		context_handle,		/* in    */
	gss_buffer_t		token_buffer		/* in    */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   INIT_GSSAPI( );

   DEBUG_BEGIN(process_context_token)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p,\n"
	          "\t\t token_buffer          = ptr:%p\n"
	          "\t)\n",
	           context_handle,
	           token_buffer ))

   DEBUG_ARG_CTX( context_handle )
   DEBUG_ARG_BUF( token_buffer )

   major_status = gn_gss_process_context_token( minor_status,
						context_handle,
						token_buffer );

   DEBUG_RET( )
   DEBUG_RET_CTX( context_handle )

   return(major_status);

} /* gss_process_context_token() */





/* gss_delete_sec_context() */

OM_uint32 EXPORT_FUNCTION
gss_delete_sec_context(
	OM_uint32	FAR *	minor_status,		/* out      */
	gss_ctx_id_t	FAR *	context_handle,		/* in/out   */
	gss_buffer_t		output_token		/* (in)/out */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   TRY_INIT_GSSAPI( );

   DEBUG_BEGIN(delete_sec_context)

   DEBUG_ARG((tf, "\t\t&context_handle        = ptr:%p,\n"
	          "\t\t output_token          = ptr:%p\n"
	          "\t)\n",
	          context_handle,
	          output_token ))
   DEBUG_ARG_CTX_REF( context_handle )
   /* this is no input parameter: DEBUG_ARG_BUF( output_token ) */

   CLEAR_OUT_BUFFER( output_token );

   major_status = gn_gss_delete_sec_context( minor_status,
					     context_handle, output_token );

   DEBUG_RET( )
   DEBUG_RET_CTX_REF( context_handle )
   DEBUG_RET_BUF(     output_token   )

   return(major_status);

} /* gss_delete_sec_context() */






/*
 * gss_get_mic()
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_get_mic(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_ctx_id_t		context_handle,		/* in    */
	gss_qop_t		in_qop,			/* in    */
	gss_buffer_t		message_buffer,		/* in    */
	gss_buffer_t		token_buffer		/* out   */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_BUFFER( token_buffer );

   INIT_GSSAPI( );

   DEBUG_BEGIN(get_mic)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p,\n"
	          "\t\t in_qop                = 0x%08lx,\n"
	          "\t\t message_buffer        = ptr:%p,\n"
		  "\t\t token_buffer          = ptr:%p\n"
	          "\t)\n",
	          context_handle,
	          (unsigned long) in_qop,
	          message_buffer,
	          token_buffer ))

   DEBUG_ARG_CTX(     context_handle )
   DEBUG_ARG_BUF(     message_buffer )
   DEBUG_ARG_BUFHEAD( token_buffer   )

   major_status = gn_gss_get_mic( minor_status, context_handle,
				  in_qop, message_buffer, token_buffer );

   DEBUG_RET( )
   DEBUG_RET_BUF(     token_buffer   )
   DEBUG_RET_CTX(     context_handle )

   return(major_status);

} /* gss_get_mic() */


  
/*
 * gss_sign()
 *
 * same as gss_get_mic(), Backwards compatibility to GSS-API v1
 */
OM_uint32 EXPORT_FUNCTION
gss_sign(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_ctx_id_t		context_handle,		/* in    */
	int			in_qop,			/* in    */
	gss_buffer_t		message_buffer,		/* in    */
	gss_buffer_t		message_token		/* out   */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_BUFFER( message_token );

   INIT_GSSAPI( );

   DEBUG_BEGIN(sign)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p,\n"
	          "\t\t in_qop                = 0x%08lx,\n"
	          "\t\t message_buffer        = ptr:%p,\n"
		  "\t\t message_token         = ptr:%p\n"
	          "\t)\n",
	          context_handle,
	          (unsigned long) in_qop,
	          message_buffer,
	          message_token ))

   DEBUG_ARG_CTX(     context_handle )
   DEBUG_ARG_BUF(     message_buffer )
   DEBUG_ARG_BUFHEAD( message_token  )

   major_status = gn_gss_get_mic( minor_status, context_handle,
				  (gss_qop_t)in_qop, message_buffer, message_token );

   DEBUG_RET( )
   DEBUG_RET_BUF(     message_token  )
   DEBUG_RET_CTX(     context_handle )

   return(major_status);

} /* gss_sign() */





/*
 * gss_verify_mic()
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_verify_mic(
	OM_uint32	FAR *	minor_status,		/* out     */
	gss_ctx_id_t		context_handle,		/* in      */
	gss_buffer_t		message_buffer,		/* in      */
	gss_buffer_t		token_buffer,		/* in      */
	gss_qop_t	FAR *	qop_state		/* out     */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   INIT_GSSAPI( );

   DEBUG_BEGIN(verify_mic)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p,\n"
	          "\t\t message_buffer        = ptr:%p,\n"
	          "\t\t token_buffer          = ptr:%p,\n"
	          "\t\t&qop_state             = ptr:%p\n"
	          "\t)\n",
	          context_handle,
	          message_buffer,
	          token_buffer,
	          qop_state ))

   DEBUG_ARG_CTX( context_handle )
   DEBUG_ARG_BUF( message_buffer )
   DEBUG_ARG_BUF( token_buffer )

   major_status = gn_gss_verify_mic( minor_status, context_handle,
				     message_buffer, token_buffer, qop_state );

   DEBUG_RET( )
   DEBUG_VALUE_REF(RET, qop_state)
   DEBUG_RET_CTX( context_handle )

   return(major_status);

} /* gss_verify_mic() */



/*
 * gss_verify()
 *
 * same as gss_verify_mic(), Backwards compatibility to GSS-API v1
 */
OM_uint32 EXPORT_FUNCTION
gss_verify(
	OM_uint32	FAR *	minor_status,		/* out     */
	gss_ctx_id_t		context_handle,		/* in      */
	gss_buffer_t		message_buffer,		/* in      */
	gss_buffer_t		token_buffer,		/* in      */
	int		FAR *	qop_state		/* out     */
     )
{
   OM_uint32   major_status;
   gss_qop_t   l_qop_state = 0;
   DEBUG_BEGIN_VARS( )

   INIT_GSSAPI( );

   DEBUG_BEGIN(verify)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p,\n"
	          "\t\t message_buffer        = ptr:%p,\n"
	          "\t\t token_buffer          = ptr:%p,\n"
	          "\t\t&qop_state             = ptr:%p\n"
	          "\t)\n",
	          context_handle,
	          message_buffer,
	          token_buffer,
	          qop_state ))

   DEBUG_ARG_CTX( context_handle )
   DEBUG_ARG_BUF( message_buffer )
   DEBUG_ARG_BUF( token_buffer )

   major_status = gn_gss_verify_mic( minor_status, context_handle,
				     message_buffer, token_buffer, &l_qop_state );

   if ( qop_state!=NULL )
	*qop_state = l_qop_state;

   DEBUG_RET( )
   DEBUG_VALUE_REF(RET, qop_state)
   DEBUG_RET_CTX( context_handle )

   return(major_status);

} /* gss_verify() */



/*
 * gss_wrap()
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_wrap(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_ctx_id_t		context_handle,		/* in    */
	int			conf_req_flag,		/* in    */
	gss_qop_t		qop_req,		/* in    */
	gss_buffer_t		input_message,		/* in    */
	int		FAR *	conf_state,		/* out   */
	gss_buffer_t		output_message		/* out   */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_BUFFER(  output_message );

   INIT_GSSAPI( );

   DEBUG_BEGIN(wrap)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p\n"
	          "\t\t conf_req_flag         = %d,\n"
	          "\t\t qop_req               = 0x%08lx,\n"
	          "\t\t input_message         = ptr:%p,\n"
	          "\t\t&conf_state            = ptr:%p,\n"
	          "\t\t output_message        = ptr:%p"
	          "\t)\n",
	          context_handle,
	          (int)conf_req_flag,
	          (unsigned long)qop_req,
	          input_message,
	          conf_state,
	          output_message ))

   DEBUG_ARG_CTX(     context_handle )
   DEBUG_ARG_BOOL(    conf_req_flag  )
   DEBUG_ARG_BUF(     input_message  )
   DEBUG_ARG_BUFHEAD( output_message )

   major_status = gn_gss_wrap( minor_status, context_handle,
			       conf_req_flag, qop_req,
			       input_message, conf_state, output_message );

   DEBUG_RET( )
   DEBUG_RET_BOOL_REF( conf_state     )
   DEBUG_RET_CTX(      context_handle )
   DEBUG_RET_BUF(      output_message )

   return(major_status);

} /* gss_wrap() */



/*
 * gss_seal()
 *
 * same as gss_wrap(), Backwards compatibility to GSS-API v1
 */
OM_uint32 EXPORT_FUNCTION
gss_seal(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_ctx_id_t		context_handle,		/* in    */
	int			conf_req_flag,		/* in    */
	int			qop_req,		/* in    */
	gss_buffer_t		input_message,		/* in    */
	int		FAR *	conf_state,		/* out   */
	gss_buffer_t		output_message		/* out   */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_BUFFER(  output_message );

   INIT_GSSAPI( );

   DEBUG_BEGIN(seal)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p\n"
	          "\t\t conf_req_flag         = %d,\n"
	          "\t\t qop_req               = 0x%08lx,\n"
	          "\t\t input_message         = ptr:%p,\n"
	          "\t\t&conf_state            = ptr:%p,\n"
	          "\t\t output_message        = ptr:%p"
	          "\t)\n",
	          context_handle,
	          (int)conf_req_flag,
	          (unsigned long)qop_req,
	          input_message,
	          conf_state,
	          output_message ))

   DEBUG_ARG_CTX(     context_handle )
   DEBUG_ARG_BOOL(    conf_req_flag  )
   DEBUG_ARG_BUF(     input_message  )
   DEBUG_ARG_BUFHEAD( output_message )

   major_status = gn_gss_wrap( minor_status, context_handle,
			       conf_req_flag, (gss_qop_t)qop_req,
			       input_message, conf_state, output_message );

   DEBUG_RET( )
   DEBUG_RET_BOOL_REF( conf_state     )
   DEBUG_RET_CTX(      context_handle )
   DEBUG_RET_BUF(      output_message )

   return(major_status);

} /* gss_seal() */



/*
 * gss_unwrap()
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_unwrap(
	OM_uint32	FAR *	minor_status,		/* out  */
	gss_ctx_id_t		context_handle,		/* in   */
	gss_buffer_t		input_message,		/* in   */
	gss_buffer_t		output_message,		/* out  */
	int		FAR *	conf_state,		/* out  */
	gss_qop_t	FAR *	qop_state		/* out  */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_BUFFER( output_message );

   INIT_GSSAPI( );

   DEBUG_BEGIN(unwrap)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p,\n"
	          "\t\t input_message         = ptr:%p,\n"
	          "\t\t output_message        = ptr:%p,\n"
	          "\t\t&conf_state            = ptr:%p,\n"
	          "\t\t&qop_state             = ptr:%p\n"
	          "\t)\n",
	          context_handle,
	          input_message,
	          output_message,
	          conf_state,
	          qop_state ))

   DEBUG_ARG_CTX(     context_handle )
   DEBUG_ARG_BUF(     input_message  )
   DEBUG_ARG_BUFHEAD( output_message )

   major_status = gn_gss_unwrap( minor_status, context_handle, input_message,
				 output_message, conf_state, qop_state );

   DEBUG_RET( )
   DEBUG_RET_BOOL_REF(    conf_state           )
   DEBUG_VALUE_REF( RET,  qop_state            )
   DEBUG_RET_CTX(         context_handle       )
   DEBUG_RET_BUF(         output_message       )

   return(major_status);

} /* gss_unwrap() */



/*
 * gss_unseal()
 *
 * same as gss_unwrap(), Backwards compatibility to GSS-API v1
 */
OM_uint32 EXPORT_FUNCTION
gss_unseal(
	OM_uint32	FAR *	minor_status,		/* out  */
	gss_ctx_id_t		context_handle,		/* in   */
	gss_buffer_t		input_message,		/* in   */
	gss_buffer_t		output_message,		/* out  */
	int		FAR *	conf_state,		/* out  */
	int	        FAR *	qop_state		/* out  */
     )
{
   OM_uint32   major_status;
   gss_qop_t   l_qop_state = 0;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_BUFFER( output_message );

   INIT_GSSAPI( );

   DEBUG_BEGIN(unseal)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p,\n"
	          "\t\t input_message         = ptr:%p,\n"
	          "\t\t output_message        = ptr:%p,\n"
	          "\t\t&conf_state            = ptr:%p,\n"
	          "\t\t&qop_state             = ptr:%p\n"
	          "\t)\n",
	          context_handle,
	          input_message,
	          output_message,
	          conf_state,
	          qop_state ))

   DEBUG_ARG_CTX(     context_handle )
   DEBUG_ARG_BUF(     input_message  )
   DEBUG_ARG_BUFHEAD( output_message )

   major_status = gn_gss_unwrap( minor_status, context_handle, input_message,
				 output_message, conf_state, &l_qop_state );

   if ( qop_state!=NULL )
	*qop_state = l_qop_state;

   DEBUG_RET( )
   DEBUG_RET_BOOL_REF(    conf_state           )
   DEBUG_VALUE_REF( RET,  qop_state            )
   DEBUG_RET_CTX(         context_handle       )
   DEBUG_RET_BUF(         output_message       )

   return(major_status);

} /* gss_unseal() */

 


/*
 * gss_compare_name()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_compare_name(
	OM_uint32	FAR *	minor_status,		/* out  */
	gss_name_t		name1,			/* in   */
	gss_name_t		name2,			/* in   */
	int		FAR *	name_equal		/* out  */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   INIT_GSSAPI( );

   DEBUG_BEGIN(compare_name)

   DEBUG_ARG((tf, "\t\t name1                 = ptr:%p,\n"
	          "\t\t name2                 = ptr:%p,\n"
	          "\t\t&name_equal            = ptr:%p\n"
	          "\t)\n",
	          name1,
	          name2,
	          name_equal ))

   DEBUG_ARG_NAME( name1 )
   DEBUG_ARG_NAME( name2 )

   major_status = gn_gss_compare_name( minor_status,
				       name1, name2, name_equal );

   DEBUG_RET( )
   DEBUG_RET_BOOL_REF( name_equal )

   return(major_status);

} /* gss_compare_name() */





/*
 * gss_display_name()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_display_name(
	OM_uint32	FAR *	minor_status,		/* out   */
        gss_name_t		input_name,		/* in    */
        gss_buffer_t		output_name_buffer,	/* out   */
        gss_OID		FAR *	output_name_type	/* out   */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   INIT_GSSAPI( );

   DEBUG_BEGIN(display_name)

   DEBUG_ARG((tf, "\t\t input_name            = ptr:%p,\n"
	          "\t\t output_name_buffer    = ptr:%p,\n"
	          "\t\t&output_name_type      = ptr:%p\n"
	          "\t)\n",
	          input_name,
	          output_name_buffer,
	          output_name_type ))

   DEBUG_ARG_NAME(    input_name         )
   DEBUG_ARG_BUFHEAD( output_name_buffer )

   CLEAR_OUT_BUFFER(  output_name_buffer );
   CLEAR_OUT_OID(     output_name_type   );

   major_status = gn_gss_display_name( minor_status, input_name,
				       output_name_buffer, output_name_type );

   DEBUG_RET( )
   DEBUG_RET_BUF(     output_name_buffer )
   DEBUG_RET_OID_REF( output_name_type   )

   return(major_status);

} /* gss_display_name() */







/*
 * gss_inquire_cred()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_inquire_cred(
	OM_uint32	FAR *	minor_status,		/* out  */
	gss_cred_id_t		cred_handle,		/* in   */
	gss_name_t	FAR *	name,			/* out  */
	OM_uint32	FAR *	lifetime,		/* out  */
	gss_cred_usage_t FAR *	cred_usage,		/* out  */
	gss_OID_set	FAR *	mechanisms		/* out  */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_NAME(     name       );
   CLEAR_OUT_OID_SET(  mechanisms );

   INIT_GSSAPI( );

   DEBUG_BEGIN(inquire_cred)

   DEBUG_ARG((tf, "\t\t cred_handle           = ptr:%p,\n"
	          "\t\t&name                  = ptr:%p,\n"
	          "\t\t&lifetime              = ptr:%p,\n"
	          "\t\t&cred_usage            = ptr:%p,\n"
	          "\t\t&mechanisms            = ptr:%p\n"
	          "\t)\n",
	          cred_handle,
	          name,
	          lifetime,
	          cred_usage,
	          mechanisms ))

   DEBUG_ARG_CRED( cred_handle )

   major_status = gn_gss_inquire_cred( minor_status, cred_handle, name,
				       lifetime, cred_usage, mechanisms );

   DEBUG_RET( )
   DEBUG_RET_NAME_REF(    name       )
   DEBUG_RET_LIFET_REF(   lifetime   )
   DEBUG_RET_USAGE_REF(   cred_usage )
   DEBUG_RET_OID_SET_REF( mechanisms )

   return(major_status);

} /* gss_inquire_cred() */




/**********************************************************************/
/**********************************************************************/
/*******************                               ********************/
/*******************  New fuctions for GSS-API v2  ********************/
/*******************                               ********************/
/**********************************************************************/
/**********************************************************************/

/*
 * gss_add_cred()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_add_cred(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_cred_id_t		input_cred_handle,	/* in    */
	gss_name_t		desired_name,		/* in    */
	gss_OID			desired_mech,		/* in    */
	gss_cred_usage_t	cred_usage,		/* in    */
	OM_uint32		initiator_time_req,	/* in    */
	OM_uint32		acceptor_time_req,	/* in    */
	gss_cred_id_t	FAR *	output_cred_handle,	/* out   */
	gss_OID_set	FAR *	actual_mechs,		/* out   */
	OM_uint32	FAR *	initiator_time_rec,	/* out   */
	OM_uint32	FAR *	acceptor_time_rec	/* out   */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_CRED(    output_cred_handle );
   CLEAR_OUT_OID_SET( actual_mechs       );

   INIT_GSSAPI( );

   DEBUG_BEGIN(add_cred)

   DEBUG_ARG((tf, "\t\t input_cred_handle     = ptr:%p,\n"
	          "\t\t desired_name          = ptr:%p,\n"
	          "\t\t desired_mech          = ptr:%p,\n"
	          "\t\t cred_usage            = %d,\n"
	          "\t\t initiator_time_req    = 0x%08lx,\n"
	          "\t\t acceptor_time_req     = 0x%08lx,\n"
	          "\t\t&output_cred_handle    = ptr:%p,\n"
	          "\t\t&actual_mechs          = ptr:%p,\n"
	          "\t\t&initiator_time_rec    = ptr:%p,\n"
	          "\t\t&acceptor_time_rec     = ptr:%p\n"
	          "\t)\n",
	          input_cred_handle,
	          desired_name,
	          desired_mech,
	          (int)cred_usage,
	          (unsigned long)initiator_time_req,
	          (unsigned long)acceptor_time_req,
	          output_cred_handle,
	          actual_mechs,
	          initiator_time_rec,
	          acceptor_time_rec ))

   DEBUG_ARG_CRED(  input_cred_handle  )
   DEBUG_ARG_NAME(  desired_name       )
   DEBUG_ARG_OID(   desired_mech       )
   DEBUG_ARG_USAGE( cred_usage         )
   DEBUG_ARG_LIFET( initiator_time_req )
   DEBUG_ARG_LIFET( acceptor_time_req  )
 
   major_status = gn_gss_add_cred( minor_status, input_cred_handle,
				   desired_name, desired_mech, cred_usage,
				   initiator_time_req, acceptor_time_req,
				   output_cred_handle, actual_mechs,
				   initiator_time_rec, acceptor_time_rec );

   DEBUG_RET( )
   DEBUG_RET_CRED_REF(    output_cred_handle )
   DEBUG_RET_OID_SET_REF( actual_mechs       )
   DEBUG_RET_LIFET_REF(   initiator_time_rec )
   DEBUG_RET_LIFET_REF(   acceptor_time_rec  )

   return(major_status);

} /* gss_add_cred() */





/*
 * gss_inquire_cred_by_mech()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_inquire_cred_by_mech(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_cred_id_t		cred_handle,		/* in    */
	gss_OID			mech_type,		/* in    */
	gss_name_t	FAR *	name,			/* out   */
	OM_uint32	FAR *	initiator_lifetime,	/* out   */
	OM_uint32	FAR *	acceptor_lifetime,	/* out   */
	gss_cred_usage_t FAR *	cred_usage		/* out   */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_NAME( name );

   INIT_GSSAPI( );

   DEBUG_BEGIN(inquire_cred_by_mech)

   DEBUG_ARG((tf, "\t\t cred_handle           = ptr:%p,\n"
	          "\t\t mech_type             = ptr:%p,\n"
	          "\t\t&name                  = ptr:%p\n"
	          "\t\t&initiator_lifetime    = ptr:%p,\n"
	          "\t\t&acceptor_lifetime     = ptr:%p,\n"
	          "\t\t&cred_usage            = ptr:%p\n"
	          "\t)\n",
	          cred_handle,
	          mech_type,
	          name,
	          initiator_lifetime,
	          acceptor_lifetime,
	          cred_usage ))

   DEBUG_ARG_CRED( cred_handle )
   DEBUG_ARG_OID(  mech_type   )

   major_status = gn_gss_inquire_cred_by_mech( minor_status, cred_handle,
					       mech_type, name,
					       initiator_lifetime,
					       acceptor_lifetime,
					       cred_usage );

   DEBUG_RET( )
   DEBUG_RET_NAME_REF( name )
   DEBUG_RET_LIFET_REF( initiator_lifetime )
   DEBUG_RET_LIFET_REF( acceptor_lifetime )
   DEBUG_RET_USAGE_REF( cred_usage )

   return(major_status);

} /* gss_inquire_cred_by_mech() */




/*
 * gss_inquire_context()
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_inquire_context(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_ctx_id_t		context_handle,		/* in    */
	gss_name_t	FAR *	initiator_name,		/* out   */
	gss_name_t	FAR *	acceptor_name,		/* out   */
	OM_uint32	FAR *	lifetime_rec,		/* out   */
	gss_OID		FAR *	mech_type,		/* out   */
	OM_uint32	FAR *	ret_flags,		/* out   */
	int		FAR *	locally_initiated,	/* out   */
	int		FAR *	out_open		/* out   */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_NAME( initiator_name );
   CLEAR_OUT_NAME( acceptor_name  );
   CLEAR_OUT_OID(  mech_type      );

   INIT_GSSAPI( );

   DEBUG_BEGIN(inquire_context)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p,\n"
	          "\t\t&initiator_name        = ptr:%p,\n"
	          "\t\t&acceptor_name         = ptr:%p,\n"
	          "\t\t&lifetime_rec          = ptr:%p,\n"
	          "\t\t&mech_type             = ptr:%p,\n"
	          "\t\t&ret_flags             = ptr:%p,\n"
	          "\t\t&locally_initiated     = ptr:%p,\n"
	          "\t\t&out_open              = ptr:%p\n"
	          "\t)\n",
	           context_handle,
	           initiator_name,
	           acceptor_name,
	           lifetime_rec,
	           mech_type,
	           ret_flags,
	           locally_initiated,
	           out_open ))

   DEBUG_ARG_CTX( context_handle )

   major_status = gn_gss_inquire_context( minor_status, context_handle,
					  initiator_name, acceptor_name,
					  lifetime_rec, mech_type,
					  ret_flags, locally_initiated,
					  out_open );

   DEBUG_RET( )
   DEBUG_RET_NAME_REF(  initiator_name    )
   DEBUG_RET_NAME_REF(  acceptor_name     )
   DEBUG_RET_LIFET_REF( lifetime_rec      )
   DEBUG_RET_OID_REF(   mech_type         )
   DEBUG_RET_FLAGS_REF( ret_flags         )
   DEBUG_RET_BOOL_REF(  locally_initiated )
   DEBUG_RET_BOOL_REF(  out_open          )

   return(major_status);

} /* gss_inquire_context() */





/*
 * gss_wrap_size_limit()
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_wrap_size_limit(
	OM_uint32	FAR *	minor_status,		/* out  */
	gss_ctx_id_t		context_handle,		/* in   */
	int			conf_req_flag,		/* in   */
	gss_qop_t		qop_req,		/* in   */
	OM_uint32		requested_output_size,	/* in   */
	OM_uint32	FAR *	maximum_input_size	/* out  */
)
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   INIT_GSSAPI( );

   DEBUG_BEGIN(wrap_size_limit)

   DEBUG_ARG((tf, "\t\t context_handle        = ptr:%p,\n"
	          "\t\t conf_req_flag         = %d,\n"
	          "\t\t qop_req               = 0x%08lx,\n"
	          "\t\t requested_output_size = %lu (0x%08lx),\n"
	          "\t\t&maximum_input_size    = ptr:%p\n"
	          "\t)\n",
	          context_handle,
	          (int) conf_req_flag,
	          (unsigned long)qop_req,
	          (unsigned long) requested_output_size,
	          (unsigned long) requested_output_size,
	          maximum_input_size ))

   DEBUG_ARG_CTX( context_handle )

   major_status = gn_gss_wrap_size_limit( minor_status, context_handle,
					  conf_req_flag, qop_req,
					  requested_output_size,
					  maximum_input_size );

   DEBUG_RET( )
   DEBUG_VALUE_REF( RET , maximum_input_size )

   return(major_status);

} /* gss_wrap_size_limit() */





/*
 * gss_export_sec_context()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_export_sec_context(
	OM_uint32	FAR *	minor_status,		/* out  */
	gss_ctx_id_t	FAR *	context_handle,		/* out  */
	gss_buffer_t		interprocess_token	/* out  */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_BUFFER( interprocess_token );

   INIT_GSSAPI( );

   DEBUG_BEGIN(export_sec_context)

   DEBUG_ARG((tf, "\t\t&context_handle        = ptr:%p,\n"
	          "\t\t interprocess_token    = ptr:%p\n"
	          "\t)\n",
	          context_handle,
	          interprocess_token ))

   DEBUG_ARG_CTX(     context_handle     )
   DEBUG_ARG_BUFHEAD( interprocess_token )

   major_status = gn_gss_export_sec_context( minor_status, context_handle,
					     interprocess_token );

   DEBUG_RET( )
   DEBUG_RET_CTX_REF( context_handle     )
   DEBUG_RET_BUF(     interprocess_token )

   return(major_status);

} /* gss_export_sec_context() */




/*
 * gss_import_sec_context()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_import_sec_context(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_buffer_t		interprocess_token,	/* in    */
	gss_ctx_id_t	FAR *	context_handle		/* out   */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_CTX( context_handle );

   INIT_GSSAPI( );

   DEBUG_BEGIN(import_sec_context)

   DEBUG_ARG((tf, "\t\t interprocess_token    = ptr:%p,\n"
	          "\t\t&context_handle        = ptr:%p\n"
	          "\t)\n",
	          interprocess_token,
	          context_handle ))

   major_status = gn_gss_import_sec_context( minor_status, interprocess_token,
					     context_handle );

   DEBUG_RET( )
   DEBUG_RET_CTX_REF( context_handle )

   return(major_status);

} /* gss_import_sec_context() */



/*
 * gss_test_oid_set_member()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_test_oid_set_member(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_OID			member,			/* in    */
	gss_OID_set		set,			/* in    */
	int		FAR *	present			/* out   */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   TRY_INIT_GSSAPI( );

   DEBUG_BEGIN(test_oid_set_member)

   DEBUG_ARG((tf, "\t\t member                = ptr:%p,\n"
	          "\t\t set                   = ptr:%p,\n"
	          "\t\t&present               = ptr:%p\n"
	          "\t)\n",
	          member,
	          set,
	          present ))

   DEBUG_ARG_OID(     member )
   DEBUG_ARG_OID_SET( set    )

   major_status = gn_gss_test_oid_set_member( minor_status,
					      member, set, present );

   DEBUG_RET( )
   DEBUG_RET_BOOL_REF( present )

   return(major_status);

} /* gss_test_oid_set_member() */



/*
 * gss_inquire_names_for_mech()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_inquire_names_for_mech(
	OM_uint32	FAR *	minor_status,		/* out */
	gss_OID			mech_oid,		/* in  */
	gss_OID_set	FAR *	name_types		/* out */
     )
{
   OM_uint32 major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_OID_SET( name_types );

   INIT_GSSAPI( );

   DEBUG_BEGIN(inquire_names_for_mech)

   DEBUG_ARG((tf, "\t\t mech_oid              = ptr:%p,\n"
	          "\t\t&name_types            = ptr:%p\n"
	          "\t)\n",
	          mech_oid,
	          name_types ))

   major_status = gn_gss_inquire_names_for_mech( minor_status,
						 mech_oid, name_types );

   DEBUG_RET( )
   DEBUG_RET_OID_SET_REF( name_types )

   return(major_status);

} /* gss_inquire_names_for_mech() */





/*
 * gss_inquire_mechs_for_name()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_inquire_mechs_for_name(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_name_t		input_name,		/* in    */
	gss_OID_set	FAR *	mech_oids		/* out   */

     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_OID_SET( mech_oids );

   INIT_GSSAPI( );

   DEBUG_BEGIN(inquire_mechs_for_name)

   DEBUG_ARG((tf, "\t\t input_name            = ptr:%p,\n"
	          "\t\t&mech_oids             = ptr:%p\n"
	          "\t)\n",
	          input_name,
	          mech_oids ))

   DEBUG_ARG_NAME( input_name )

   major_status = gn_gss_inquire_mechs_for_name( minor_status,
						 input_name, mech_oids );

   DEBUG_RET( )
   DEBUG_RET_OID_SET_REF( mech_oids )

   return(major_status);

} /* gss_inquire_mechs_for_name() */





/*
 * gss_canonicalize_name()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_canonicalize_name(
	OM_uint32	FAR *	minor_status,		/* out  */
	gss_name_t		input_name,		/* in   */
	gss_OID			mech_type,		/* in   */
	gss_name_t	FAR *	output_name		/* out  */
     )
{
   OM_uint32   major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_NAME( output_name );

   INIT_GSSAPI( );

   DEBUG_BEGIN(canonicalize_name)

   DEBUG_ARG((tf, "\t\t input_name            = ptr:%p,\n"
	          "\t\t mech_type             = ptr:%p,\n"
	          "\t\t&output_name           = ptr:%p\n"
	          "\t)\n",
	          input_name,
	          mech_type,
	          output_name ))

   DEBUG_ARG_NAME( input_name )
   DEBUG_ARG_OID(  mech_type  )

   major_status = gn_gss_canonicalize_name( minor_status, input_name,
					    mech_type, output_name );

   DEBUG_RET( )
   DEBUG_RET_NAME_REF( output_name )

   return(major_status);

} /* gss_canonicalize_name() */





/*
 * gss_export_name()
 *
 *
 *
 */
OM_uint32 EXPORT_FUNCTION
gss_export_name(
	OM_uint32	FAR *	minor_status,		/* out   */
	gss_name_t		input_name,		/* in    */
	gss_buffer_t		exported_name		/* out   */
     )
{
   OM_uint32 major_status;
   DEBUG_BEGIN_VARS( )

   CLEAR_OUT_BUFFER( exported_name );

   INIT_GSSAPI( );

   DEBUG_BEGIN(export_name)

   DEBUG_ARG((tf, "\t\t input_name            = ptr:%p,\n"
	          "\t\t exported_name         = ptr:%p\n"
	          "\t)\n",
	          input_name,
	          exported_name ))

   DEBUG_ARG_NAME(    input_name    )
   DEBUG_ARG_BUFHEAD( exported_name )

   major_status = gn_gss_export_name( minor_status,
				      input_name, exported_name );

   DEBUG_RET( )
   DEBUG_RET_BUF( exported_name )

   return(major_status);

} /* gss_export_name() */





/* end of gssmain.c */
