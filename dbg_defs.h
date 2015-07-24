/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/dbg_defs.h#1 $
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


#ifndef _DBG_DEFS_H_
#  define _DBG_DEFS_H_

#  ifdef NDEBUG

#define DEBUG_ONLY_CMD(x)

#define DEBUG_INIT(p_minor_status, major_status)

#define DEBUG_BEGIN_VARS() 

#define DEBUG_BEGIN(func) 

#define DEBUG_RET()

#define DEBUG_ARG(x)
#define DEBUG_ACTION(x)


#define DEBUG_STRANGE(x)
#define DEBUG_ERR(x)
#define DEBUG_VAL(x)

#define DEBUG_OID(oid)
#define DEBUG_ARG_OID(oid)
#define DEBUG_RET_OID(oid)

#define DEBUG_OID_REF(p_oid)
#define DEBUG_ARG_OID_REF(p_oid)
#define DEBUG_RET_OID_REF(p_oid)

#define DEBUG_OID_SET(oidset)
#define DEBUG_ARG_OID_SET(oidset)
#define DEBUG_RET_OID_SET(oidset)

#define DEBUG_OID_SET_REF(p_set)
#define DEBUG_ARG_OID_SET_REF(p_set)
#define DEBUG_RET_OID_SET_REF(p_set)

#define DEBUG_BUF(buf)
#define DEBUG_ARG_BUF(buf)
#define DEBUG_RET_BUF(buf)

#define DEBUG_ARG_BUFHEAD(buf)
#define DEBUG_RET_BUFHEAD(buf)

#define DEBUG_ARG_NAME(x)
#define DEBUG_ARG_NAME_REF(x)
#define DEBUG_RET_NAME(x)
#define DEBUG_RET_NAME_REF(x)

#define DEBUG_ARG_CTX(x)
#define DEBUG_ARG_CTX_REF(x)
#define DEBUG_RET_CTX(x)
#define DEBUG_RET_CTX_REF(x)

#define DEBUG_ARG_CRED(x)
#define DEBUG_ARG_CRED_REF(x)
#define DEBUG_RET_CRED(x)
#define DEBUG_RET_CRED_REF(x)

#define DEBUG_ARG_LIFET(x)   
#define DEBUG_ARG_LIFET_REF(x)
#define DEBUG_RET_LIFET(x)
#define DEBUG_RET_LIFET_REF(x)

#define DEBUG_ARG_FLAGS(x)
#define DEBUG_ARG_FLAGS_REF(x)
#define DEBUG_RET_FLAGS(x)
#define DEBUG_RET_FLAGS_REF(x)

#define DEBUG_ARG_BOOL(x)
#define DEBUG_ARG_BOOL_REF(x)
#define DEBUG_RET_BOOL(x)
#define DEBUG_RET_BOOL_REF(x)

#define DEBUG_ARG_USAGE(x)
#define DEBUG_ARG_USAGE_REF(x)
#define DEBUG_RET_USAGE(x)
#define DEBUG_RET_USAGE_REF(x)

#define DEBUG_ARG_CHBIND(x)

#define DEBUG_VALUE(flag, value)
#define DEBUG_VALUE_REF(flag, value)

#define VERIFY_READ_PTR(ptr)	if (ptr==NULL) {			  \
				   return(GSS_S_CALL_INACCESSIBLE_READ);  \
				}

#define VERIFY_WRITE_PTR(ptr)	if (ptr==NULL) {			  \
				   return(GSS_S_CALL_INACCESSIBLE_WRITE); \
				}


#  else /* !NDEBUG */


/****************************************
 * DEFINITIONS for Debugging the library
 ****************************************/
typedef enum dbg_info_e {
	INFO_ARG	= (1<<0),
	INFO_ARG_VAL	= (1<<1),
	INFO_RET	= (1<<2),
	INFO_RET_VAL	= (1<<3),
	INFO_ACTION	= (1<<4),
	INFO_OID	= (1<<5),
	INFO_OID_DATA	= (1<<6),
	INFO_OID_SET	= (1<<7),
	INFO_BUF	= (1<<8),
	INFO_BUF_DATA	= (1<<9),
	INFO_NAME	= (1<<10),
	INFO_CRED	= (1<<11),
	INFO_CTX	= (1<<12),
	INFO_CTX_RET	= (1<<13),
	INFO_QOP	= (1<<14),
	INFO_LIFET	= (1<<15),
	INFO_USAGE	= (1<<16),
	INFO_FLAGS	= (1<<17),
	INFO_BOOL       = (1<<18)
} dbg_info_et;


typedef enum dbg_functions_e {
  /* credential management routines */
	FUNC_acquire_cred=0,
	FUNC_add_cred,
	FUNC_inquire_cred,
	FUNC_inquire_cred_by_mech,
	FUNC_release_cred,
  /* context-level routines */
	FUNC_init_sec_context,
	FUNC_accept_sec_context,
	FUNC_delete_sec_context,
	FUNC_process_context_token,
	FUNC_context_time,
	FUNC_inquire_context,
	FUNC_wrap_size_limit,
	FUNC_export_sec_context,
	FUNC_import_sec_context,
  /* per message routines */
	FUNC_get_mic,
	FUNC_sign,
	FUNC_verify_mic,
	FUNC_verify,
	FUNC_wrap,
	FUNC_seal,
	FUNC_unwrap,
	FUNC_unseal,
  /* name manipulation routines */
	FUNC_import_name,
	FUNC_display_name,
	FUNC_compare_name,
	FUNC_release_name,
	FUNC_inquire_names_for_mech,
	FUNC_inquire_mechs_for_name,
	FUNC_canonicalize_name,
	FUNC_export_name,
	FUNC_duplicate_name,
  /* miscellaneous routines */
	FUNC_display_status,
	FUNC_indicate_mechs,
	FUNC_release_buffer,
	FUNC_release_oid_set,
	FUNC_create_empty_oid_set,
	FUNC_add_oid_set_member,
	FUNC_test_oid_set_member,
} dbg_functions_et;


#define DEBUG_ONLY_CMD(x)    x

#define FUNC_MAX     (FUNC_test_oid_set_member+1)

#define DEBUG_INIT(p_minor_status, major_status)			\
	{								\
		major_status = dbg_init( p_minor_status );		\
		if (major_status!=GSS_S_COMPLETE)			\
			return(major_status);				\
	}
	
#define DEBUG_BEGIN_VARS()   char *this_Call; 

#define DEBUG_BEGIN(func) \
    this_Call = dbg_begin( FUNC_ ## func , minor_status );

#define DEBUG_RET()							\
    if (DB_FL1(RET))    {						\
       dbg_ret( this_Call, minor_status, major_status );		\
    }

#define DEBUG_ARG(x)	     if (DB_FL1(ARG))      { fprintf x ; fflush(tf); }
#define DEBUG_ACTION(x)	     if (DB_FL1(ACTION))   { char tbuf[128]; fprintf(tf, "%s  ", dbg_snprint_systemtime(tbuf, sizeof(tbuf), NULL, DBG_TIME_TRACE)); fprintf x ; fflush(tf); }


#define DEBUG_STRANGE(x)     if (tf!=NULL)         { char tbuf[128]; fprintf(tf, "%s  ", dbg_snprint_systemtime(tbuf, sizeof(tbuf), NULL, DBG_TIME_TRACE)); fprintf x ; fflush(tf); }
#define DEBUG_ERR(x)         if (tf!=NULL)         { char tbuf[128]; fprintf(tf, "%s  ", dbg_snprint_systemtime(tbuf, sizeof(tbuf), NULL, DBG_TIME_TRACE)); fprintf x ; fflush(tf); }
#define DEBUG_VAL(x)	     if (tf!=NULL)         { fprintf x ; fflush(tf); }

#define DB_FL1(info)	( tf!=NULL					\
	&& dbg_enable							\
	&& (dbg_info &(INFO_ ## info))!=0 )

#define DB_FL2(i,j)  ( tf!=NULL						\
	&& dbg_enable							\
	&& ((dbg_info & ((INFO_ ## i)|(INFO_ ## j)))			\
	       == ((INFO_ ## i)|(INFO_ ## j)) ) )

#define DB_FL3(i,j,k) ( tf!=NULL					\
	&& dbg_enable							\
	&& ( (dbg_info & ((INFO_ ## i)|(INFO_ ## j)|(INFO_ ## k)))	\
	     == ((INFO_ ## i)|(INFO_ ## j)|(INFO_ ## k)) ) )
			   

#define DBG_IN     0
#define DBG_OUT    1
#define DBG_ALWAYS TRUE


  /* Tracing of OID objects */
#define XDEBUG_OID(cond, name, oid)					\
	dbg_oid( (cond), name , (oid) );

#define DEBUG_OID(oid)      XDEBUG_OID( (INFO_OID),              #oid, oid )
#define DEBUG_ARG_OID(oid)  XDEBUG_OID( (INFO_ARG|INFO_ARG_VAL), #oid, oid )
#define DEBUG_RET_OID(oid)  XDEBUG_OID( (INFO_RET|INFO_RET_VAL), #oid, oid )

#define DEBUG_OID_REF(p_oid)						\
  if ( (p_oid!=NULL) ) {						\
	XDEBUG_OID( (INFO_OID),         #p_oid, *p_oid )		\
  }
#define DEBUG_ARG_OID_REF(p_oid)					\
  if ( (p_oid!=NULL) ) {						\
	XDEBUG_OID( (INFO_ARG|INFO_ARG_VAL), #p_oid, *p_oid )		\
  }
#define DEBUG_RET_OID_REF(p_oid)					\
  if ( (p_oid!=NULL) ) {						\
	XDEBUG_OID( (INFO_RET|INFO_RET_VAL), #p_oid, *p_oid )		\
  }

  /* Tracing of OID_SET object */
#define XDEBUG_OID_SET(cond, name, oidset)				\
			dbg_oid_set( (cond), name , (oidset) );

#define DEBUG_OID_SET(oidset)						\
	XDEBUG_OID_SET( (INFO_OID_SET),     #oidset, oidset )
#define DEBUG_ARG_OID_SET(oidset)					\
	XDEBUG_OID_SET( (INFO_ARG|INFO_ARG_VAL), #oidset, oidset )
#define DEBUG_RET_OID_SET(oidset)					\
	XDEBUG_OID_SET( (INFO_RET|INFO_RET_VAL), #oidset, oidset )

#define DEBUG_OID_SET_REF(p_set)     \
  if ( (p_set!=NULL) ) {						\
	XDEBUG_OID_SET( (INFO_OID_SET),     #p_set, *p_set)		\
  }
#define DEBUG_ARG_OID_SET_REF(p_set) \
  if ( (p_set!=NULL) ) {						\
	XDEBUG_OID_SET( (INFO_ARG|INFO_ARG_VAL), #p_set, *p_set)	\
  } 
#define DEBUG_RET_OID_SET_REF(p_set) \
  if ( (p_set!=NULL) ) {						\
	XDEBUG_OID_SET( (INFO_RET|INFO_RET_VAL), #p_set, *p_set)	\
  }

  /* Tracing of gss_buffer_t objects */
#define XDEBUG_BUF(cond,name,buf)					\
	{								\
		dbg_buffer_head( (cond), name, (buf) );			\
		dbg_buffer_content( ((cond)|INFO_BUF_DATA), buf );	\
	}

#define DEBUG_BUF(buf)      XDEBUG_BUF( (INFO_BUF),              #buf, buf )
#define DEBUG_ARG_BUF(buf)  XDEBUG_BUF( (INFO_ARG|INFO_ARG_VAL), #buf, buf )
#define DEBUG_RET_BUF(buf)  XDEBUG_BUF( (INFO_RET|INFO_RET_VAL), #buf, buf )

  /* Tracing of gss_buffer_t return parameter on function entry */ 
#define DEBUG_ARG_BUFHEAD(buf)						\
	dbg_buffer_head( (INFO_ARG|INFO_ARG_VAL), #buf, buf );

#define DEBUG_RET_BUFHEAD(buf)						\
	dbg_buffer_head( (INFO_RET|INFO_RET_VAL), #buf, buf );


#define XDEBUG_PTR(cond, name, ptr)					\
        dbg_print_ptr( (cond), name, ptr );

  /* Tracing of gss_name_t (gn_name_t) objects      */
#define DEBUG_ARG_NAME(x)       /* XDEBUG_PTR( (INFO_ARG|INFO_NAME),  #x, &x ) */
#define DEBUG_ARG_NAME_REF(x)   XDEBUG_PTR( (INFO_ARG|INFO_NAME),  #x, x )
#define DEBUG_RET_NAME(x)       /* gibt's nicht */
#define DEBUG_RET_NAME_REF(x)   XDEBUG_PTR( (INFO_RET|INFO_NAME),  #x, x )

  /* Tracing of gss_ctx_id_t (gn_context_t) objects */
#define DEBUG_ARG_CTX(x)        /* XDEBUG_PTR( (INFO_ARG|INFO_CTX),  #x, &x ) */
#define DEBUG_ARG_CTX_REF(x)    XDEBUG_PTR( (INFO_ARG|INFO_CTX),  #x, x )
#define DEBUG_RET_CTX(x)        /* gibt's nicht */
#define DEBUG_RET_CTX_REF(x)    XDEBUG_PTR( (INFO_RET|INFO_CTX),  #x, x )

  /* Tracing of gss_cred_id_t (gn_cred_t) objects   */
#define DEBUG_ARG_CRED(x)       /* XDEBUG_PTR( (INFO_ARG|INFO_CTX),  #x, &x ) */
#define DEBUG_ARG_CRED_REF(x)   XDEBUG_PTR( (INFO_ARG|INFO_CRED),  #x, x )
#define DEBUG_RET_CRED(x)       /* gibt's nicht */
#define DEBUG_RET_CRED_REF(x)   XDEBUG_PTR( (INFO_RET|INFO_CRED),  #x, x )

  /* Tracing of lifetime values */
#define DEBUG_ARG_LIFET(x)      dbg_lifetime( (INFO_ARG|INFO_LIFET), #x, &x);
#define DEBUG_ARG_LIFET_REF(x)  dbg_lifetime( (INFO_ARG|INFO_LIFET), #x,  x);
#define DEBUG_RET_LIFET(x)	/* dbg_lifetime( (INFO_RET|INFO_LIFET), #x, &x); */
#define DEBUG_RET_LIFET_REF(x)  dbg_lifetime( (INFO_RET|INFO_LIFET), #x,  x);

  /* Tracing of */
#define DEBUG_ARG_FLAGS(x)      dbg_flags( (INFO_ARG|INFO_FLAGS), #x, &x);
#define DEBUG_ARG_FLAGS_REF(x)  dbg_flags( (INFO_ARG|INFO_FLAGS), #x,  x);
#define DEBUG_RET_FLAGS(x)      /* dbg_flags( (INFO_ARG|INFO_FLAGS), #x, &x); */
#define DEBUG_RET_FLAGS_REF(x)  dbg_flags( (INFO_RET|INFO_FLAGS), #x,  x);

#define XDEBUG_BOOL(cond,name,ptr,size)  dbg_bool( cond, name, ptr, size );

  /* Tracing of BOOLEAN values */
#define DEBUG_ARG_BOOL(x)	XDEBUG_BOOL( INFO_ARG|INFO_BOOL, #x, &x, sizeof(x)  )
#define DEBUG_ARG_BOOL_REF(x)   XDEBUG_BOOL( INFO_ARG|INFO_BOOL, #x,  x, sizeof(*x) ) 
#define DEBUG_RET_BOOL(x)	/* */
#define DEBUG_RET_BOOL_REF(x)	XDEBUG_BOOL( INFO_RET|INFO_BOOL, #x,  x, sizeof(*x) )

  /* Tracing of CRED_USAGE values */
#define DEBUG_ARG_USAGE(x)      dbg_usage( (INFO_ARG|INFO_USAGE), #x, &x );
#define DEBUG_ARG_USAGE_REF(x)	dbg_usage( (INFO_ARG|INFO_USAGE), #x,  x );
#define DEBUG_RET_USAGE(x)	/* dbg_usage( (INFO_ARG|INFO_USAGE), #x, &x ); */
#define DEBUG_RET_USAGE_REF(x)	dbg_usage( (INFO_RET|INFO_USAGE), #x,  x );

  /* Tracing of CHANNEL BINDINGS structures */
#define DEBUG_ARG_CHBIND(x)

  /* Tracing of INTEGER values */
#define XDEBUG_VALUE(cond, name, ptr, size)				\
		dbg_integer( cond, name, ptr, size );

#define DEBUG_VALUE(flag, value)					\
	XDEBUG_VALUE( INFO_ ## flag , #value, &value, sizeof(value) )
#define DEBUG_VALUE_REF(flag, value)					\
	XDEBUG_VALUE( INFO_ ## flag , #value, value, sizeof(*value) )

#define VERIFY_READ_PTR(ptr)	if (ptr==NULL) {			  \
				   return(GSS_S_CALL_INACCESSIBLE_READ);  \
				}

#define VERIFY_WRITE_PTR(ptr)	if (ptr==NULL) {			  \
				   return(GSS_S_CALL_INACCESSIBLE_WRITE); \
				}
/****************************************
 * End of debugging definitions
 ****************************************/


/*****************************************
 * Function declarations for dbg_fn.c
 *****************************************/


extern FILE FAR *tf;
extern int	 dbg_enable;
extern OM_uint32 dbg_info;
extern int	 dbg_func[FUNC_MAX];

/*******************************************************
 * Function declarations/prototypes for dbg_fn.c
 *******************************************************/

#define DBG_TIME_TRACE           (1<<0)
#define DBG_TIME_SHOW_TIMEZONE   (1<<1)
#define DBG_TIME_SHOW_DAYNAME    (1<<2)
#define DBG_TIME_SHOW_MILLISEC   (1<<3)

#ifdef _WIN32
#  if !defined(_SSPI_H_) && !defined(__SSPI_H__)
     typedef struct _SECURITY_INTEGER
     {
       unsigned long LowPart;
       long HighPart;
     } SECURITY_INTEGER, *PSECURITY_INTEGER;
     typedef SECURITY_INTEGER TimeStamp;
#  endif /* !_SSPI_H_ */
#  if !defined(_WINDEF_)
     typedef unsigned short      WORD;
#  endif /* !_WINDEF_ */
#  if !defined(_WINBASE_)
     typedef struct _SYSTEMTIME {
       WORD wYear;
       WORD wMonth;
       WORD wDayOfWeek;
       WORD wDay;
       WORD wHour;
       WORD wMinute;
       WORD wSecond;
       WORD wMilliseconds;
     } SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
#  endif /* !_WINBASE_ */
  char * dbg_snprint_systemtime( char * p_buf, size_t p_buf_max, SYSTEMTIME * p_systime, int p_flags );
  char * dbg_snprint_timestamp(  char * p_buf, size_t p_buf_max, TimeStamp  * p_time,    int p_flags );
#endif /* _WIN32 */


char * dbg_snprint_time_t(     char * p_buf, size_t p_buf_max, time_t       p_time_t );


OM_uint32  dbg_init( OM_uint32 * pp_min_stat );

char *dbg_begin( dbg_functions_et func , OM_uint32 * pp_min_stat );
void  dbg_ret( char *p_func_name, OM_uint32 *pp_minor, OM_uint32 p_major );
void  dbg_oid( OM_uint32 p_dbgmask, char * p_parm_name, gss_OID p_oid );
void  dbg_oid_content( OM_uint32 p_dbgmask, gss_OID p_oid );
void  dbg_oid_set( OM_uint32 p_dbgmask,
		   char * p_parm_name , gss_OID_set p_oidset );
void  dbg_buffer_head( OM_uint32 p_dbgmask,
		       char * p_parm_name , gss_buffer_t p_buffer );
void  dbg_hexdump( char *p_base, size_t offset,
		   size_t p_num, size_t p_align );
void  dbg_buffer_content( OM_uint32 p_dbgmask, gss_buffer_t p_buffer );

void  dbg_integer( OM_uint32 p_dbgmask, char * p_parm_name,
		   void * p_baseptr, int size );


void  dbg_lifetime( OM_uint32 p_dbgmask,
		    char  * p_param_name, OM_uint32 * pp_lifetime );

void  dbg_usage( OM_uint32 p_dbgmask,
		 char     * p_param_name, OM_uint32 * pp_usage );


void  dbg_flags( OM_uint32 p_dbgmask,
		 char     * p_param_name, OM_uint32 * pp_flags );
void  dbg_bool( OM_uint32 p_dbgmask,
	         char * p_parm_name, void * p_bool_val, size_t p_bool_size );


void  dbg_print_ptr( OM_uint32    p_dbgmask,
		     char       * p_param_name,   void  ** pp_ptr );

#  endif /* !NDEBUG */

#endif /* _DBG_DEFS_H_ */
