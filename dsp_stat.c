#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/dsp_stat.c#2 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/dsp_stat.c#2 $
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


struct major_names_s {
	char  *name;
	char  *desc;
};


gn_min_stat_desc * registered_min_stat[GN_MAX_MECHS];
OM_uint32	   gn_min_stat_counter = 0;

/***********************************************************************
 * error handling of this generic GSS-API v2 implementation:
 *
 *   major_status values:  These are completely specified at the GSS-API
 *                         and therefore all handled within the generic
 *			   layer (here in this module).
 *
 *   minor_status values:  These are classified as "implementation specific"
 *
 *
 *
 *
 ***********************************************************************/


#define MAJ_ENTRY(code,desc)   { "GSS_S_" # code, desc }


static struct major_names_s routine_errors[] = {
    MAJ_ENTRY(COMPLETE,                 ""                              ),
    MAJ_ENTRY(BAD_MECH,			"Unsupported mechanism"		),
    MAJ_ENTRY(BAD_NAME,			"Invalid name"			),
    MAJ_ENTRY(BAD_NAMETYPE,		"Unsupported nametype"		),
    MAJ_ENTRY(BAD_BINDINGS,		"Bad channel bindings"		),
    MAJ_ENTRY(BAD_STATUS,		"Invalid status code"		),
    MAJ_ENTRY(BAD_SIG,			"MIC did not verify"		),
    MAJ_ENTRY(NO_CRED,			"No valid credentials provided (or available)" ),
    MAJ_ENTRY(NO_CONTEXT,		"No valid context specified"	),
    MAJ_ENTRY(DEFECTIVE_TOKEN,		"Defective token provided"	),
    MAJ_ENTRY(DEFECTIVE_CREDENTIAL,	"Defective credentials detected"),
    MAJ_ENTRY(CREDENTIALS_EXPIRED,	"Credentials have expired"	),
    MAJ_ENTRY(CONTEXT_EXPIRED,		"Security context has expired"	),
    MAJ_ENTRY(FAILURE,			"Miscellaneous Failure"		),
    MAJ_ENTRY(BAD_QOP,			"Invalid QOP value supplied"	),
    MAJ_ENTRY(UNAUTHORIZED,		"Operation unauthorized"	),
    MAJ_ENTRY(UNAVAILABLE,		"Operation/Feature unavailable"	),
    MAJ_ENTRY(DUPLICATE_ELEMENT,	"Credential element already exists" ),
    MAJ_ENTRY(NAME_NOT_MN,	        "Provided name is not a mechanism name" ),
};

static struct major_names_s suppl_info[] = {
    MAJ_ENTRY(CONTINUE_NEEDED,		"Further token exchanges required"  ),
    MAJ_ENTRY(DUPLICATE_TOKEN,		"Duplicate token detected"	    ),
    MAJ_ENTRY(OLD_TOKEN,		"Old token detected"		    ),
    MAJ_ENTRY(UNSEQ_TOKEN,		"Tokens not in sequential order"    ),
    MAJ_ENTRY(GAP_TOKEN,	        "An earlier token is still missing" ),
};

static struct major_names_s calling_errors[] = {
   { NULL, NULL },
    MAJ_ENTRY(CALL_INACCESSIBLE_READ,	"Invalid pointer for input arg"  ),
    MAJ_ENTRY(CALL_INACCESSIBLE_WRITE,	"Invalid pointer for output arg" ),
    MAJ_ENTRY(CALL_BAD_STRUCTURE,	"Invalid structure supplied"	  ),
};


struct num_to_text_s {
	OM_uint32      value;
	char         * name;
        char         * desc;
};

#define MIN_STRUCT_ENTRY(code,desc)	\
	{  (code), # code , desc }


#define MIN_ENTRY(code,desc)     MIN_STRUCT_ENTRY( MINOR_ ## code , desc )

static struct num_to_text_s minor_error[] = {
    MIN_ENTRY(NO_ERROR,			""				),
    MIN_ENTRY(OUT_OF_MEMORY,		"Out of Memory"			),
    MIN_ENTRY(BAD_CRED_USAGE,		"Invalid cred_usage value"	),
    MIN_ENTRY(INVALID_BUFFER,		"Invalid gss_buffer handle"	),
    MIN_ENTRY(INVALID_OID,		"Invalid gss_OID handle"	),
    MIN_ENTRY(INVALID_OID_SET,		"Invalid gss_OID_set handle"	),
    MIN_ENTRY(INVALID_CTX,		"Invalid gss_ctx_id_t handle"   ),
    MIN_ENTRY(INVALID_CRED,		"Invalid gss_cred_id_t_handle"  ),
    MIN_ENTRY(INVALID_NAME,		"Invalid gss_name_t handle"	),
    MIN_ENTRY(INCOMPLETE_CTX,	        "Security context not completely established" ),
    MIN_ENTRY(AT_MISSING_IN_SVC_NAME,	"@ missing in service name"	),
    MIN_ENTRY(BAD_HOSTNAME_IN_SVC_NAME, "Bad hostname in service name"  ),
    MIN_ENTRY(SERVICE_MISSING,		"Service part missing in name"	),
    MIN_ENTRY(GETHOSTNAME_FAILED,	"Syscall gethostname() failed?!" ),
    MIN_ENTRY(HOSTNAME_MISSING,		"Hostname is missing"		),
    MIN_ENTRY(NAME_TOO_LONG,		"Supplied name is too long"     ),
    MIN_ENTRY(HOST_LOOKUP_FAILURE,	"Hostname lookup failure"	),
    MIN_ENTRY(HOSTNAME_TOO_LONG,	"Hostname too long"		),
    MIN_ENTRY(INVALID_HOSTNAME,		"Syntactically incorrect hostname" ),
    MIN_ENTRY(NOT_YET_IMPLEMENTED,      "Operation/feature not yet implemented" ),
    MIN_ENTRY(INTERNAL_BUFFER_OVERRUN,  "Internal buffer overrun"       ),

    MIN_ENTRY(NAME_CONTAINS_NUL,	"Name contains a NUL character" ),
    MIN_ENTRY(INTERNAL_ERROR,		"Programming error in mechanism" ),
    MIN_ENTRY(YOU_DONT_EXIST,		"Unable to obtain your username" ),
    MIN_ENTRY(INVALID_USERNAME,		"Invalid syntax for username"	),
    MIN_ENTRY(BAD_UID,			"The UID is invalid"		),
    MIN_ENTRY(FEATURE_UNAVAILABLE,      "Feature unavailable (on this platform)" ),
    MIN_ENTRY(NOT_A_TOKEN,		"This is not a valid token"	),
    MIN_ENTRY(INVALID_LENGTH_FIELD,	"A DER-encoded length field in the token is invalid" ),
    MIN_ENTRY(MAY_BE_TRUNCATED,		"The token length field suggests the token is trunctated" ),
    MIN_ENTRY(TRAILING_GARBAGE,         "The token length field suggests there is trailing garbage" ),
    MIN_ENTRY(UNKNOWN_MECH_OID,		"Unknown mechanism requested"	),
    MIN_ENTRY(WRONG_LEN_EXPNAME,	"The length field of the exported name doesn't match the token size" ),

    MIN_ENTRY(NEED_ONE_MECHNAME,        "At least one of the names must be a mechanism name (MN)" ),
    MIN_ENTRY(MN_DISJUNCT_MECHLIST,     "The two names do not share a common mechanism" ),
    MIN_ENTRY(WRONG_CONTEXT,	        "Context is recognized, but inadequate to this function" ),
    MIN_ENTRY(CRED_CHANGED,	        "Credentials handle was changed since initial call" ),
    MIN_ENTRY(FLAGS_CHANGED,	        "Service flags were changed since initial call" ),
    MIN_ENTRY(MECH_CHANGED,	        "Mechanism oid was changed since initial call" ),
    MIN_ENTRY(CHANBIND_CHANGED,	        "Channel bindings changed since initial call" ),
    MIN_ENTRY(WRONG_CREDENTIAL,	        "Wrong credential type for requested operation" ),
    MIN_ENTRY(DEAD_CONTEXT,	        "Context is not or no longer operational" ),

    MIN_ENTRY(INCOMPLETE_CONTEXT,       "Security context incomplete (not fully established)" ),
    MIN_ENTRY(ESTABLISHED_CONTEXT,      "Security context already fully established" ),
    MIN_ENTRY(TARGET_CHANGED,	        "Target name changed since initial call" ),
    MIN_ENTRY(NO_TARGET,	        "Missing target name" ),
    MIN_ENTRY(INPUT_MISSING,	        "Input token missing" ),
    MIN_ENTRY(WRONG_TOKEN,	        "API call / Token type mismatch" ),
    MIN_ENTRY(MISC_ERROR,	        "Platform specific error that didn't fit renumbering scheme" ),
    MIN_ENTRY(REFLECTED_MIC,	        "Reflection attack?  This MIC token is for our peer" ),
    MIN_ENTRY(REFLECTED_WRAP,	        "Reflection attack?  This Wrap token is for our peer" ),
    MIN_ENTRY(SIZE_TOO_SMALL,	        "Output size limit below minimum token size" ),
    MIN_ENTRY(WRONG_MECHANISM,		"Token's mechanism tag doesn't match this context" ),

    MIN_ENTRY(INVALID_STATUS,	        "Unrecognized minor status value" ),

    MIN_ENTRY(INIT_FAILED,	        "Mechanism initialzation error" ),
    MIN_ENTRY(SHLIB_NOT_FOUND,	        "Shared library/DLL not found" ),
    MIN_ENTRY(SHLIB_INVALID,	        "Invalid shared library/DLL encountered" ),

    MIN_ENTRY(NO_MECHANISM_AVAIL,       "No mechanism available (DLL missing or platform not supported?)" ),
    MIN_ENTRY(NO_TOKEN_EXPECTED,        "No token expected for initial call to gss_init_sec_context()" ),
    MIN_ENTRY(NO_CRED_FOR_MECH,	        "No credentials available for requested mechanism" ),

    MIN_ENTRY(SIMULATED_ERROR,		"Simulated Error"		),

    { 0,    NULL,			NULL }   
};




/*
 * gn_minor_error()
 *
 *
 */
void
gn_minor_error( OM_uint32 min_stat, char * buffer )
{
   char			   * this_Call = "gn_minor_error";
   struct num_to_text_s    * ptr;

   buffer[0]= '\0';

   for ( ptr = &(minor_error[0]) ;  ptr->name!=NULL ; ptr++ ) {
      if ( ptr->value == min_stat ) {
	 sprintf(buffer, /* "%.50s: %.150s", ptr->name, */ "%.150s", ptr->desc );
	 break;
      }
   }

   if ( buffer[0]=='\0' ) {
      DEBUG_STRANGE((tf, "  S: %s(): 0x%08lx not found in minor status list!\n",
			 this_Call, (long)min_stat))
   }

   if ( buffer[0]=='\0' ) {
      sprintf(buffer, "internal error with minor status 0x%08lx\n", (long)min_stat);
   }

   return;

} /* gn_minor_error() */



/*
 * gn_maj_stat_text()
 *
 *
 */
int
gn_maj_stat_text( OM_uint32 maj_stat, int errtype,  int  * pp_counter,
		  char * buffer, char ** pp_label, char ** pp_desc )
{
   char			   * label = NULL;
   char                    * unknown_fmt = "(%u): Unknown %.100s";
   char			   * desc  = NULL;
   char			   * name  = NULL;
   struct major_names_s	   * sptr  = NULL;
   OM_uint32		     idx;
   int			     skip;
   int			     i;

   switch( errtype ) {

      case MAJ_STAT_CALLING:
		  (*pp_counter)=0;
		  name = "calling error number";
		  idx = GSS_CALLING_ERROR( maj_stat ) >> GSS_C_CALLING_ERROR_OFFSET ;
		  if ( idx<ARRAY_ELEMENTS(calling_errors) )
		     sptr = &(calling_errors[0]);
		  break;

      case MAJ_STAT_ROUTINE:
		  (*pp_counter)=0;
		  name = "routine error number";
		  idx = GSS_ROUTINE_ERROR( maj_stat ) >> GSS_C_ROUTINE_ERROR_OFFSET ;
		  if ( idx<ARRAY_ELEMENTS(routine_errors) )
		     sptr = &(routine_errors[0]);
		  break;

      case MAJ_STAT_INFORMATORY:
		  name		= "informatory status";
		  idx		= GSS_SUPPLEMENTARY_INFO(maj_stat) >> GSS_C_SUPPLEMENTARY_OFFSET;
		  skip		= (*pp_counter);
		  (*pp_counter) = 0;
		  for ( i=0 ; i<(ARRAY_ELEMENTS(suppl_info)) ; i++ ) {
		     if ( i < skip )
			continue;
		     if ( (idx&(1<<i))!=0 ) {
			sptr = &(suppl_info[0]);
			if ( (idx & ( ~((1<<(i+1))-1)))!=0 ) {
			   /* we need further calls if there are remaining bits */
			   (*pp_counter) = i+1;
			}
			idx = i;  /* move array index [i] into "idx" */
			break;
		     }
		  }
		  if ( sptr==NULL ) {
		     /* remove valid informational bits from output bitset */
		     idx = idx & ~((1<<ARRAY_ELEMENTS(suppl_info))-1);
		     unknown_fmt = "(0x%04lx): Unknown %.100s";
		  }
		  break;

      default:
		  name = "error type";
		  idx  = errtype;
		  break;

   }

   if ( sptr==NULL ) { /* unrecognized error */

      if ( buffer!=NULL ) {
	 (*pp_counter) = 0;
	 sprintf(buffer, unknown_fmt, (Uint) idx, name);
      }

   } else { /* else (sptr!=NULL)  we have a printable equivalent */

      label = sptr[idx].name;
      desc  = sptr[idx].desc;
      if ( buffer!=NULL ) {
	 sprintf(buffer, /* "%.50s: %.150s", label, */ "%.150s", desc );
      }

   } /* endif (sptr!=NULL) */

   if ( pp_label!=NULL ) { *pp_label = label; }
   if ( pp_desc !=NULL ) { *pp_desc  = desc; }

   return(sptr==NULL);

} /* gn_maj_stat_text() */




/*
 * gn_gss_display_status()
 *
 *
 */
OM_uint32
gn_gss_display_status(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	OM_uint32		p_in_statval,	/* status_value		*/
	int			p_in_stattype,	/* status_type		*/
	gss_OID			p_in_mech,	/* mech_type		*/
	OM_uint32	FAR *	pp_msg_context,	/* message_context	*/
	gss_buffer_t		p_out_text	/* status_string	*/
     )
{
   char             * this_Call   = "gn_gss_display_status";
   char             * msg         = NULL;
   size_t	      msg_len     = 0;
   gn_mech_tag_et     in_mech_tag = MECH_INVALID_TAG;
   gn_mech_tag_et     mech_tag    = MECH_INVALID_TAG;
   gn_min_stat_desc * min_list;
   OM_uint32          maj_stat    = GSS_S_COMPLETE;
   OM_uint32	      min_stat;
   OM_uint32	      min_val;
   OM_uint32	      min_class;
   OM_uint32	      min_msel;
   int		      context;
   int	              counter;
   int		      have;
   int		      i;
   int		      need_release = 0;
   char               tmpbuf[256];

   (*pp_min_stat) = 0;
   tmpbuf[0]      = '\0';

   /* sanity check output parameter "status_string" (gss_buffer_t) */
   if ( p_out_text==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): no output gss_buffer_t for status string!\n", this_Call))
      RETURN_MIN_MAJ( MINOR_NO_ERROR, GSS_S_CALL_INACCESSIBLE_WRITE | GSS_S_FAILURE );
   }

   /* clearing output parameter */
   p_out_text->value  = NULL;
   p_out_text->length = 0;

   /* sanity check on in/out parameter "message_context" */
   if ( pp_msg_context==NULL ) {
      DEBUG_ERR((tf, "  E: %s(): missing message_context parameter!\n", this_Call))
      ERROR_RETURN( MINOR_NO_ERROR, GSS_S_CALL_INACCESSIBLE_READ | GSS_S_FAILURE );
   }

   switch( p_in_stattype ) {

      case GSS_C_GSS_CODE:
	    if ( p_in_statval==0 ) {
	       /* catch GSS_S_COMPLETE here ... :-)  */
	       (*pp_msg_context) = 0;
	       msg = "All is well that ends well";

	    } else {

	       counter = (*pp_msg_context)>>8;
	       context = (*pp_msg_context)&0xff;
	       have = 0;
	       if ( GSS_CALLING_ERROR(p_in_statval)!=0 ) {
		  if (have==context) {
		     gn_maj_stat_text( p_in_statval, MAJ_STAT_CALLING,
				       &counter, tmpbuf, NULL, NULL );
		  }
		  have++;
	       }
	       if ( GSS_ROUTINE_ERROR(p_in_statval)!=0 ) {
		  if (have==context) {
		     gn_maj_stat_text( p_in_statval, MAJ_STAT_ROUTINE,
				       &counter, tmpbuf, NULL, NULL );
		  }
		  have++;
	       }
	       if ( GSS_SUPPLEMENTARY_INFO(p_in_statval)!=0 ) {
		  if ( have==context) {
		     gn_maj_stat_text( p_in_statval, MAJ_STAT_INFORMATORY,
				       &counter, tmpbuf, NULL, NULL );
		  }
		  have++;
	       }

	       if ( context>=have ) {
		  DEBUG_ERR((tf, "  E: %s(): wrong context value 0x%08lx!\n",
				 (Ulong) *pp_msg_context ))
		  ERROR_RETURN( MINOR_NO_ERROR, GSS_S_FAILURE );
	       }
	       
	       if ( counter==0 ) {
		  context++;
	       }

	       if ( context>=have ) {
		  (*pp_msg_context) = 0;
	       } else {
		  (*pp_msg_context) = (OM_uint32) ((context&0xff) + (counter<<8));
	       }
	       msg = tmpbuf;
	    }

	    maj_stat = gn_alloc_buffer( pp_min_stat, p_out_text,
					msg, strlen(msg), TRUE );
	    break;


      case GSS_C_MECH_CODE:
	    /* convert mechanism_oid into mech_tag -- in case a mech OID was supplied */
	    if ( p_in_mech!=GSS_C_NO_OID ) {
	       maj_stat = gn_oid_to_mech_tag( pp_min_stat, p_in_mech, &in_mech_tag );
	       if ( maj_stat!=GSS_S_COMPLETE )
		  goto error;
	    }

	    if ( p_in_statval==0 ) {
	       /* make this case look the same for all mechanisms */
	       (*pp_msg_context)  = 0;
	       msg = "No mech-specific Error";
	       maj_stat = gn_alloc_buffer( pp_min_stat, p_out_text,
					   msg, strlen(msg), TRUE );

	    } else {  /* else  (p_in_statval!=0) */

	       min_val   = (p_in_statval & MINOR_VALUE_MASK);
	       min_class = (p_in_statval & MINOR_CLASS_MASK)>>MINOR_CLASS_SHIFT;
	       min_msel  = (p_in_statval & MINOR_MSEL_MASK)>>MINOR_MSEL_SHIFT;

	       if ( min_msel==MINOR_MSEL_GENERIC ) {

		  switch( min_class ) {

		     case MINOR_CLASS_GENERIC:
			   /* this is one of *our* generic minor status codes,   */
			   /* as defined in *our* own header file "gn_spi.h"     */
			   gn_minor_error( p_in_statval, tmpbuf );
			   maj_stat = gn_alloc_buffer( pp_min_stat, p_out_text,
						    tmpbuf, strlen(tmpbuf), TRUE );
			   if (maj_stat!=GSS_S_COMPLETE)
			      goto error;

			   break;

			
		     case MINOR_CLASS_ERRNO:
			   msg = strerror( min_val );
			   if ( msg==NULL || !sy_strcasecmp(msg, "no error") ) {
			      RETURN_MIN_MAJ( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );
			   }
			   maj_stat = gn_alloc_buffer( pp_min_stat, p_out_text,
						       msg, strlen(msg), TRUE );
			   if (maj_stat!=GSS_S_COMPLETE)
			      goto error;

			   break;


		     case MINOR_CLASS_LASTERROR:
			   maj_stat = sy_lasterror( pp_min_stat, min_val,
						    tmpbuf, 254 );
			   if ( maj_stat!=GSS_S_COMPLETE )
			      goto error;

			   maj_stat = gn_alloc_buffer( pp_min_stat, p_out_text,
						       tmpbuf, strlen(tmpbuf), TRUE );
			   if (maj_stat!=GSS_S_COMPLETE)
			      goto error;

			   break;


		     case MINOR_CLASS_REGISTERED:
			   tmpbuf[0] = '\0';
			   for ( i=0 ; i<ARRAY_ELEMENTS(registered_min_stat) ; i++ ) {
			      if ( registered_min_stat[i]!=NULL ) {
				 for( min_list = registered_min_stat[i]; min_list->label!=NULL ; min_list++ ) {
				    if ( (min_list->value)==p_in_statval ) {
				       sprintf(tmpbuf, "%.150s", min_list->description);
				       break;
				    }
				 }
			      }
			      if ( tmpbuf[0]!='\0' )
				 break;
			   }

			   if ( tmpbuf[0]=='\0' ) {
			      DEBUG_ERR((tf, "  E: %s(): invalid registered minor_status value 0x%08lx!\n",
					     this_Call, (long) p_in_statval ))
			      ERROR_RETURN( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );
			   }
			   maj_stat = gn_alloc_buffer( pp_min_stat, p_out_text,
						       tmpbuf, strlen(tmpbuf), TRUE );
			   if (maj_stat!=GSS_S_COMPLETE)
			      goto error;

			   break;


		     default:
			   DEBUG_ERR((tf, "  E: %s(): invalid minor_status value 0x%08lx!\n",
					  this_Call, (long) p_in_statval ))
			   ERROR_RETURN( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );
		  }

	       } else { /* mech specific minor status value, dispatch it */

		  if ( in_mech_tag!=MECH_INVALID_TAG ) {
		     if ( in_mech_tag!=(gn_mech_tag_et)min_msel ) {
			DEBUG_ERR((tf, "  E: %s(): mismatch mech_oid and minor_status value 0x%08lx!\n",
				       this_Call, (long) p_in_statval ))
			ERROR_RETURN( MINOR_INVALID_STATUS, GSS_S_FAILURE );
		     }
		  }
		  
		  /* this is a mechanism specific minor status value, */
		  /* have it translated by the mechanism itself       */
		  if (min_msel==0  ||  min_msel>mech_last_tag) {
		     DEBUG_ERR((tf, "  E: %s(): minor_status value out of range 0x%08lx!\n",
				    this_Call, (long) p_in_statval ))
		     ERROR_RETURN( MINOR_INVALID_STATUS, GSS_S_BAD_STATUS );
		  }

		  /* NOTICE: if the returned message needs to be released, */
		  /*         we'll do it here immediately, not at the end  */
		  maj_stat = (gn_mech[min_msel]
			      ->fp_display_minstat)( pp_min_stat,
						     p_in_statval,
						     &msg,
						     &msg_len,
						     &need_release );
		  if (maj_stat==GSS_S_COMPLETE) {
		     /* in case of success, create gss_buffer_t with message */
		     maj_stat = gn_alloc_buffer( pp_min_stat, p_out_text,
					         msg, msg_len, TRUE );
		  }

		  if ( msg!=NULL && msg_len>0 && need_release!=0 ) {
		     /* in case the message from the mechanism was dynamic, */
		     /* release the buffer here                             */
		     (void)(gn_mech[min_msel]
			     ->fp_release_token)( &min_stat, &msg, &msg_len );
		  }

		  if (maj_stat!=GSS_S_COMPLETE)
		     goto error;

	       }

	    } /* endif (p_in_statval!=0) */

	    break;


      default:
	    DEBUG_ERR((tf, "  E: %s(): bad status type %lu!\n",
			   this_Call, (Ulong) p_in_stattype))
	    ERROR_RETURN( MINOR_NO_ERROR, GSS_S_BAD_STATUS );
   }

   if (maj_stat!=GSS_S_COMPLETE)
      goto error;


   if ( maj_stat!=GSS_S_COMPLETE ) {
      OM_uint32  min_stat;
error:
      if ( pp_msg_context!=NULL ) {
	 (*pp_msg_context) = 0;
      }
      if ( p_out_text!=NULL
	   &&  p_out_text->length>0  &&  p_out_text->value!=NULL ) {
	 (void) gn_gss_release_buffer( &min_stat, p_out_text );
      }
   }

   return(maj_stat);

} /* gn_gss_display_status() */




/*
 * gn_major_text()
 *
 * Translate a major status code into textual description
 */
int
gn_major_text( OM_uint32 p_major,
	       PRINT_NUMBER * p_text_num,
	       char ** p_text_routine,
	       char ** p_text_call,
	       char ** p_text_suppi,
	       char ** p_text_desc )
{
   char      * this_Call="gn_major_text";
   char      * unknown = "Unknown";
   OM_uint32   routine_c, call_c, suppi_c;
   int         ok = TRUE;
   Uint        i;

   routine_c = GSS_ROUTINE_ERROR(p_major) >>GSS_C_ROUTINE_ERROR_OFFSET;
   call_c    = GSS_CALLING_ERROR(p_major) >>GSS_C_CALLING_ERROR_OFFSET;
   suppi_c   = GSS_SUPPLEMENTARY_INFO(p_major) >>GSS_C_SUPPLEMENTARY_OFFSET;

   (*p_text_routine)= (*p_text_call)= (*p_text_suppi)= (*p_text_desc)= "";

   sprintf( (p_text_num->digits), "(0x%08lx)", (long) p_major);

   if ( call_c!=0 ) {
      if ( call_c>0  &&  call_c<ARRAY_ELEMENTS(calling_errors) ) {
	 (*p_text_call)  = calling_errors[call_c].name;
	 (*p_text_desc)  = calling_errors[call_c].desc;
      } else {
	 DEBUG_STRANGE((tf, "  S: %s(): Unknown Calling Error 0x%08lx !\n",
			     this_Call, (long)GSS_CALLING_ERROR(p_major) ))
	 (*p_text_call) = unknown;
	 ok = FALSE;
      }
   }


   if ( suppi_c!=0 ) {
      for ( i=0 ; i<ARRAY_ELEMENTS(suppl_info) ; i++ ) {
	 if ( (suppi_c&0x01)!=0 ) {
	    (*p_text_suppi) = suppl_info[i].name;
	    (*p_text_desc)  = suppl_info[i].desc;
	    break;
	 }
	 suppi_c /= 2;
      }
      if ( *p_text_suppi=='\0' && suppi_c!=0 ) {
	 DEBUG_STRANGE((tf, "  S: %s(): Unknown Supplementary Info 0x%08lx !\n",
			this_Call, (long) GSS_SUPPLEMENTARY_INFO(p_major) ))
	 (*p_text_suppi) = unknown;
	 ok = FALSE;
      }
   }


   if ( routine_c!=0
	|| ( routine_c==0  &&  call_c==0  &&  suppi_c==0 ) ) {
      if ( routine_c<ARRAY_ELEMENTS(routine_errors) ) {
	 (*p_text_routine) = routine_errors[routine_c].name;
	 (*p_text_desc)    = routine_errors[routine_c].desc;
      } else {
	 DEBUG_STRANGE((tf, "  S: %s(): Unknown Routine Error 0x%08lx !\n",
			this_Call, (long) GSS_ROUTINE_ERROR(p_major) ))
	 (*p_text_routine) = "Unknown";
	 ok = FALSE;
      }
   }

   return(ok);

} /* gn_major_text() */


