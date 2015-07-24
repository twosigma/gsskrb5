/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/gss_argm.h#1 $
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


#ifndef _GSS_ARGM_H_
#  define _GSS_ARGM_H_

/************************************************************************/
/*									*/
/*  Argument vectors for GSS-API calls in MACROs			*/
/*  for consistency of function declarators and function pointers	*/
/*									*/
/************************************************************************/

/*
 *   GSS-API Version 1 functionality (RFC 1508 & 1509 )
 */
#define ARG8_ACQUIRE_CRED					   \
	OM_uint32     FAR * ,	/* minor_status			*/ \
	gss_name_t	    ,	/* desired_name			*/ \
	OM_uint32	    ,	/* time_req			*/ \
	gss_OID_set	    ,	/* desired_mechs		*/ \
	gss_cred_usage_t    ,	/* cred_usage			*/ \
	gss_cred_id_t FAR * ,	/* output_cred_handle		*/ \
	gss_OID_set   FAR * ,	/* actual_mechs			*/ \
	OM_uint32     FAR *	/* time_rec			*/ 



#define ARG2_RELEASE_CRED					   \
	OM_uint32     FAR * ,		/* minor_status		*/ \
	gss_cred_id_t FAR *		/* cred_handle		*/



#define ARG13_INIT_SEC_CONTEXT					   \
	OM_uint32     FAR *    ,	/* minor_status		*/ \
	gss_cred_id_t          ,	/* initiator_cred_handle */ \
	gss_ctx_id_t  FAR *    ,	/* context_handle	*/ \
	gss_name_t             ,	/* target_name		*/ \
	gss_OID		       ,	/* mech_type		*/ \
	OM_uint32	       ,	/* req_flags		*/ \
	OM_uint32	       ,	/* time_req		*/ \
	gss_channel_bindings_t ,	/* input_chan_bindings	*/ \
	gss_buffer_t	       ,	/* input_token		*/ \
	gss_OID	      FAR *    ,	/* actual_mech_type	*/ \
	gss_buffer_t	       ,	/* output_token		*/ \
	OM_uint32     FAR *    ,	/* ret_flags		*/ \
	OM_uint32     FAR *		/* time_rec		*/



#define ARG11_ACCEPT_SEC_CONTEXT				   \
	OM_uint32     FAR *    ,	/* minor_status		*/ \
	gss_ctx_id_t  FAR *    ,	/* context_handle	*/ \
	gss_cred_id_t	       ,	/* acceptor_cred_handle */ \
	gss_buffer_t	       ,	/* input_token_buffer	*/ \
	gss_channel_bindings_t ,	/* input_chan_bindings	*/ \
	gss_name_t    FAR *    ,	/* src_name		*/ \
	gss_OID	      FAR *    ,	/* mech_type		*/ \
	gss_buffer_t	       ,	/* output_token		*/ \
	OM_uint32     FAR *    ,	/* ret_flags		*/ \
	OM_uint32     FAR *    ,	/* time_rec		*/ \
	gss_cred_id_t FAR *		/* delegated_cred_handle */



#define ARG3_PROCESS_CONTEXT_TOKEN				   \
	OM_uint32     FAR * ,		/* minor_status		*/ \
	gss_ctx_id_t	    ,		/* context_handle	*/ \
	gss_buffer_t			/* token_buffer		*/



#define ARG3_DELETE_SEC_CONTEXT					   \
	OM_uint32     FAR * ,		/* minor_status		*/ \
	gss_ctx_id_t  FAR * ,		/* context_handle	*/ \
	gss_buffer_t			/* output_token		*/



#define ARG3_CONTEXT_TIME					   \
	OM_uint32     FAR * ,		/* minor_status		*/ \
	gss_ctx_id_t	    ,		/* context_handle	*/ \
	OM_uint32     FAR *		/* time_rec		*/



/* v2 name for gss_sign() */
#define ARG5_GET_MIC						   \
	OM_uint32     FAR * ,		/* minor_status		*/ \
	gss_ctx_id_t	    ,		/* context_handle	*/ \
	gss_qop_t	    ,		/* qop_req		*/ \
	gss_buffer_t	    ,		/* message_buffer	*/ \
	gss_buffer_t			/* message_token	*/



/* v2 name for gss_verify() */
#define ARG5_VERIFY_MIC						   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_ctx_id_t	   ,		/* context_handle	*/ \
	gss_buffer_t	   ,		/* message_buffer	*/ \
	gss_buffer_t	   ,		/* token_buffer		*/ \
	gss_qop_t    FAR *		/* qop_state		*/
	


/* v2 name for gss_seal() */
#define ARG7_WRAP						   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_ctx_id_t	   ,		/* context_handle	*/ \
	int		   ,		/* conf_req_flag	*/ \
	gss_qop_t	   ,		/* qop_req		*/ \
	gss_buffer_t	   ,		/* input_message_buffer */ \
	int	     FAR * ,		/* conf_state		*/ \
	gss_buffer_t			/* output_message_buffer*/



/* v2 name for gss_unseal() */
#define ARG6_UNWRAP						   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_ctx_id_t	   ,		/* context_handle	*/ \
	gss_buffer_t	   ,		/* input_message_buffer */ \
	gss_buffer_t	   ,		/* output_message_buffer*/ \
	int	     FAR * ,		/* conf_state		*/ \
	gss_qop_t    FAR *		/* qop_state		*/



#define ARG6_DISPLAY_STATUS					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	OM_uint32	   ,		/* status_value		*/ \
	int		   ,		/* status_type		*/ \
	gss_OID		   ,		/* mech_type		*/ \
	OM_uint32    FAR * ,		/* message_context	*/ \
	gss_buffer_t			/* status_string	*/



#define ARG2_INDICATE_MECHS					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_OID_set  FAR *		/* mech_set		*/



#define ARG4_COMPARE_NAME					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_name_t	   ,		/* name1		*/ \
	gss_name_t	   ,		/* name2		*/ \
	int	     FAR *		/* name_equal		*/



#define ARG4_DISPLAY_NAME					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_name_t	   ,		/* input_name		*/ \
	gss_buffer_t	   ,		/* output_name_buffer	*/ \
	gss_OID	     FAR *		/* output_name_type	*/



#define ARG4_IMPORT_NAME					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_buffer_t	   ,		/* input_name_buffer	*/ \
	gss_OID		   ,		/* input_name_type	*/ \
	gss_name_t   FAR *		/* output_name		*/



#define ARG2_RELEASE_NAME					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_name_t   FAR *		/* input_name		*/



#define ARG2_RELEASE_BUFFER					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_buffer_t			/* buffer		*/



#define ARG2_RELEASE_OID_SET					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_OID_set  FAR *		/* set			*/



#define ARG6_INQUIRE_CRED					   \
	OM_uint32	  FAR * ,	/* minor_status		*/ \
	gss_cred_id_t		,	/* cred_handle		*/ \
	gss_name_t	  FAR * ,	/* name			*/ \
	OM_uint32	  FAR * ,	/* lifetime		*/ \
	gss_cred_usage_t  FAR * ,	/* cred_usage		*/ \
	gss_OID_set	  FAR *		/* mechanisms		*/




/*
 * New functionality in GSS-API Version 2
 */
#define ARG11_ADD_CRED						   \
	OM_uint32     FAR * ,		/* minor_status		*/ \
	gss_cred_id_t	    ,		/* input_cred_handle	*/ \
	gss_name_t	    ,		/* desired_name		*/ \
	gss_OID		    ,		/* desired_mech		*/ \
	gss_cred_usage_t    ,		/* cred_usage		*/ \
	OM_uint32	    ,		/* initiator_time_req	*/ \
	OM_uint32	    ,		/* acceptor_time_req	*/ \
	gss_cred_id_t FAR * ,		/* output_cred_handle	*/ \
	gss_OID_set   FAR * ,		/* actual_mechs		*/ \
	OM_uint32     FAR * ,		/* initiator_time_rec	*/ \
	OM_uint32     FAR *		/* acceptor_time_rec	*/



#define ARG7_INQUIRE_CRED_BY_MECH				   \
	OM_uint32	 FAR * ,	/* minor_status		*/ \
	gss_cred_id_t	       ,	/* cred_handle		*/ \
	gss_OID		       ,	/* mech_type		*/ \
	gss_name_t	 FAR * ,	/* name			*/ \
	OM_uint32	 FAR * ,	/* initiator_lifetime	*/ \
	OM_uint32	 FAR * ,	/* acceptor_lifetime	*/ \
	gss_cred_usage_t FAR *		/* cred_usage		*/



#define ARG9_INQUIRE_CONTEXT					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_ctx_id_t	   ,		/* context_handle	*/ \
	gss_name_t   FAR * ,		/* initiator_name	*/ \
	gss_name_t   FAR * ,		/* acceptor_name	*/ \
	OM_uint32    FAR * ,		/* lifetime_rec		*/ \
	gss_OID	     FAR * ,		/* mech_type		*/ \
	OM_uint32    FAR * ,		/* ret_flags		*/ \
	int	     FAR * ,		/* locally_initiated	*/ \
	int	     FAR *		/* open			*/



#define ARG6_WRAP_SIZE_LIMIT					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_ctx_id_t	   ,		/* context handle	*/ \
	int		   ,		/* conf_req_flag	*/ \
	gss_qop_t	   ,		/* qop_req		*/ \
	OM_uint32	   ,		/* requested output size*/ \
	OM_uint32    FAR *		/* maximum input size	*/



#define ARG3_EXPORT_SEC_CONTEXT					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_ctx_id_t FAR * ,		/* context_handle	*/ \
	gss_buffer_t			/* interprocess_token	*/



#define ARG3_IMPORT_SEC_CONTEXT					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_buffer_t	   ,		/* interprocess_token	*/ \
	gss_ctx_id_t FAR *		/* context_handle	*/



#define ARG2_CREATE_EMPTY_OID_SET				   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_OID_set  FAR *		/* oid_set		*/



#define ARG3_ADD_OID_SET_MEMBER					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_OID		   ,		/* member_oid		*/ \
	gss_OID_set  FAR *		/* oid_set		*/



#define ARG4_TEST_OID_SET_MEMBER				   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_OID		   ,		/* member		*/ \
	gss_OID_set	   ,		/* set			*/ \
	int	     FAR *		/* present		*/



#define ARG3_INQUIRE_NAMES_FOR_MECH				   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_OID		   ,		/* mechanism_oid	*/ \
	gss_OID_set  FAR *		/* name_types		*/



#define ARG3_INQUIRE_MECHS_FOR_NAME				   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_name_t	   ,		/* input_name		*/ \
	gss_OID_set  FAR *		/* mechanism oids	*/



#define ARG4_CANONICALIZE_NAME					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_name_t	   ,		/* input_name		*/ \
	gss_OID		   ,		/* mechanism_type	*/ \
	gss_name_t   FAR *		/* output_name		*/



#define ARG3_EXPORT_NAME					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_name_t	   ,		/* input_name		*/ \
	gss_buffer_t			/* output_name_blob	*/



#define ARG3_DUPLICATE_NAME					   \
	OM_uint32    FAR * ,		/* minor_status		*/ \
	gss_name_t	   ,		/* src_name		*/ \
	gss_name_t   FAR *		/* dest_name		*/
	

/* The following calls were removed from the GSSAPI v2 spec because     */
/* they cause confusion about the nature of OIDs (static or dynamically */
/* allocated memory) , which can easily cause problems for shared       */
/* library schemes.  These calls didn't add real value anyway.          */
/*								        */
/* However, they're implemented/used internally for visualizing         */
/* debug output.						        */

#define ARG3_OID_TO_STR                                            \
        OM_uint32    FAR * ,            /* minor_status         */ \
        gss_OID            ,            /* oid                  */ \
        gss_buffer_t                    /* oid_string           */



#define ARG3_STR_TO_OID                                            \
        OM_uint32    FAR * ,            /* minor_status         */ \
        gss_buffer_t       ,            /* oid_string           */ \
        gss_OID      FAR *              /* oid                  */


#define ARG2_RELEASE_OID                                           \
        OM_uint32    FAR * ,            /* minor_status         */ \
        gss_OID      FAR *              /* in_oid               */


#endif /* _GSS_ARGM_H_ */
