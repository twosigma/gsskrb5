/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/gss-sample/oid-func.c#3 $
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

#include "gss-misc.h"

typedef unsigned char   Uchar;
typedef unsigned int    Uint;
typedef unsigned long   Ulong;

#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif


/*
 * xigss_alloc_buffer()
 * 
 *
 *
 */
static OM_uint32
xigss_alloc_buffer( OM_uint32    * pp_min_stat,
		    gss_buffer_t   p_buffer,
		    void         * p_source,
		    size_t         p_source_len,
		    int            p_add_nul
     )
{
   void   * ptr;
   size_t   dst_len;

   (*pp_min_stat)   = 0;
   p_buffer->length = 0;
   p_buffer->value  = NULL;

   if (p_source_len==0)
      return(GSS_S_COMPLETE);

   dst_len = p_source_len + ((p_add_nul==FALSE) ? 0 : 1);
   if ( dst_len==0 ) {
      return(GSS_S_COMPLETE);
   }

   ptr = malloc( dst_len );
   if ( ptr==NULL ) {
      return(GSS_S_FAILURE);
   }

   if ( p_source!=NULL && p_source_len>0 ) {
      memcpy( ptr, p_source, p_source_len );
   }

   p_buffer->value  = ptr;
   p_buffer->length = p_source_len;
   if ( p_add_nul ) {
      ((unsigned char *)ptr)[dst_len-1] = '\0';
   }

   return(GSS_S_COMPLETE);

} /* xigss_alloc_buffer() */




/*
 * xgss_release_buffer()
 *
 * Needed to release the output of xgss_oid_to_str()
 *
 */
OM_uint32
xgss_release_buffer( OM_uint32     * pp_min_stat,
		     gss_buffer_t    p_buffer
    )
{

   (*pp_min_stat) = 0;

   if ( p_buffer == GSS_C_NO_BUFFER ) {
      return(GSS_S_COMPLETE);
   }

   if ( p_buffer->value==NULL  &&  p_buffer->length==0 ) {
      return(GSS_S_COMPLETE);
   }

   if ( p_buffer->length!=0  &&  p_buffer->value!=NULL ) {
      memset( p_buffer->value, 0, p_buffer->length);
      free( p_buffer->value );

      p_buffer->value  = NULL;
      p_buffer->length = (size_t)0;

      return(GSS_S_COMPLETE);
   }

   return(GSS_S_CALL_BAD_STRUCTURE|GSS_S_FAILURE);

} /* xgss_release_buffer() */




/*
 * xgss_oid_to_str()
 *
 * Status: This function is *NOT* part of GSS-API v2 !!
 *
 * LIMITS:  For simplicity this function works on a
 *          preallocated output buffer that will limit
 *          the size of an OID which can be string-i-fied.
 *
 */

#define OID_STRING_BUFFER_MAX	 1024

OM_uint32
xgss_oid_to_str(  OM_uint32	* pp_min_stat,	/* minor_status		*/
		  gss_OID	  p_oid,	/* oid			*/
		  gss_buffer_t	  p_buffer	/* oid_str		*/
     )
{
   char        oidbuf[OID_STRING_BUFFER_MAX];
   char        numbuf[64];
   Ulong       num  = 0; 
   OM_uint32   maj_stat;
   int         more;
   Uint        i, curlen, len, maxch;
   Uchar     * bytes;
   Uchar       uch;

   (*pp_min_stat) = 0;
   maxch = (sizeof(oidbuf)/sizeof(oidbuf[0])) - 4;
   memset(oidbuf, 0, sizeof(oidbuf));

   if ( p_buffer==NULL ) {
      (*pp_min_stat) = 0;
      return(GSS_S_CALL_INACCESSIBLE_WRITE|GSS_S_FAILURE);
   }

   p_buffer->length = 0;
   p_buffer->value  = NULL;

   if ( p_oid==NULL  ||  p_oid->length==0  ||  p_oid->elements==NULL ) {
      (*pp_min_stat) = 0;
      return(GSS_S_CALL_BAD_STRUCTURE|GSS_S_FAILURE);
   }

   /* In an OID, the first two elements are encoded into the first byte */
   /* split up the first byte here					*/
   bytes = ((unsigned char *)p_oid->elements);
   uch = bytes[0];
   sprintf(oidbuf, "{%u %u", (unsigned int)(uch/40), (unsigned int)(uch%40));

   curlen = strlen(oidbuf);
   more = FALSE;
   for( i=1 ; i<p_oid->length ; i++ ) {
      uch  = bytes[i];
      num  = (uch & 0x7f) + ( (more) ? (num<<7) : 0 );
      more = ( (uch>127) ? TRUE : FALSE );
      if (more==FALSE) {
	 sprintf(numbuf," %lu", (unsigned long)num);
	 len = strlen(numbuf);
	 if ( len + 1 + curlen > maxch ) {
	    (*pp_min_stat) = 0;
	    return(GSS_S_FAILURE);
	 }
	 memcpy( &(oidbuf[curlen]), numbuf, (size_t)(len+1) );
	 curlen += len;
      }
   }

   if (more) {
      (*pp_min_stat) = 0;
      return(GSS_S_CALL_BAD_STRUCTURE|GSS_S_FAILURE);
   }

   oidbuf[curlen++] = '}';
   oidbuf[curlen]   = '\0';
   /* The above trailing Zero is just for debugging, we don't count it   */
   /* gss_buffer_t should not include a trailing zero!                   */
   /* For safety, we always include a trailing zero, but do not count it */

   maj_stat = xigss_alloc_buffer( pp_min_stat, p_buffer,
			          (void *)oidbuf,
			          (size_t)curlen,
			          TRUE );

   return(maj_stat);

} /* xgss_oid_to_str() */




/*
 * xigss_copy_oid()
 *
 * create a dynamically allocated copy of an OID
 *
 */
static OM_uint32
xigss_copy_oid( OM_uint32    * pp_min_stat,
	        gss_OID        p_src_oid,
		gss_OID	     * pp_dst_oid
    )
{
   gss_OID     oid       = NULL;  /* dynamic, free on error */
   void      * elements  = NULL;  /* dynamic, free on error */
   size_t      length    = 0;
   OM_uint32   maj_stat  = GSS_S_COMPLETE;

   (*pp_min_stat) = 0;

   if ( pp_dst_oid==NULL )
      return(GSS_S_FAILURE);

   (*pp_dst_oid) = GSS_C_NO_OID;

   /* cloning a non-OID is simple :-)  */
   if ( p_src_oid==GSS_C_NO_OID )
      return(GSS_S_COMPLETE);

   if ( p_src_oid->elements==NULL || p_src_oid->length==0 )
      return(GSS_S_FAILURE|GSS_S_CALL_BAD_STRUCTURE);

/* cleanup alert: the next call creates a dynamic object */
   oid = malloc(sizeof(gss_OID_desc));
   if ( oid==NULL )
      return(GSS_S_FAILURE);

   length = p_src_oid->length;
   elements = malloc(length);
   if ( elements==NULL ) {
      maj_stat = GSS_S_FAILURE;
      goto error;
   }

   memcpy( elements, p_src_oid->elements, length );
   oid->elements = elements;
   oid->length   = length;

   if ( maj_stat!=GSS_S_COMPLETE ) {
error:
      if ( elements!=NULL ) { free(elements);  elements = NULL; }
      if ( oid     !=NULL ) { free(oid);       oid      = NULL; }
   }

   (*pp_dst_oid) = oid;

   return(maj_stat);

} /* xigss_copy_oid() */




/*
 * xgss_str_to_oid()
 *
 *
 *
 */
OM_uint32
xgss_str_to_oid(  OM_uint32	* pp_min_stat,	     /* minor_status	*/
		  gss_buffer_t	  p_in_oid_string,   /* oid_string	*/
		  gss_OID	* pp_out_oid	     /* oid		*/
     )
{
   gss_OID_desc    oid;
   OM_uint32	   maj_stat;
   Uchar           elements[256];
   Uchar           tmpbuf[4];
   char          * ptr;
   int             opening_brace;
   int             closing_brace;
   int             within_number;
   int             add_number;
   Ulong	   uvalue;
   Uint            len;
   Uint            i,j;
   size_t          pos;
   Uchar           uc;

   (*pp_min_stat) = 0;
   maj_stat       = GSS_S_COMPLETE;
   (*pp_out_oid)  = GSS_C_NO_OID;

   ptr = (char *) p_in_oid_string->value;

   opening_brace = closing_brace = 0;
   within_number = add_number    = 0;
   len = 0;

   for ( pos=0 ; pos<p_in_oid_string->length ; pos++ ) {

      uc = ptr[pos];

      if ( uc=='{' ) {

	 within_number = 0;
	 opening_brace = 1;

      } else if ( opening_brace && uc=='}' ) {

	 within_number = 0;
	 closing_brace = 1;
	 add_number    = 1;

      } else if ( opening_brace && isascii( uc ) && isdigit( uc ) ) {

	 if ( within_number ) {
	    uvalue *= 10;
	 } else {
	    uvalue = 0;
	 }
	 uvalue += ( uc - ((Uchar)'0') );
	 within_number = 1;

      } else {

	 if ( !isascii(uc) || !isspace(uc) ) {
	    /* invalid character encountered */
	    return(GSS_S_FAILURE);
	 }

	 if ( within_number ) {
	    within_number = 0;
	    add_number    = 1;
	 }

      }

      if ( add_number ) {
	 add_number = 0;

	 i = 0;
	 do {
	    tmpbuf[i++] = (Uchar)(uvalue & 0x7f);
	    uvalue = uvalue / 128;
	 } while ( uvalue > 0 );
	 for ( j=0 ; j<i ; j++ ) {
	    uc = tmpbuf[i-j-1];
	    if ( j+1<i ) {
	       uc |= 0x80;
	    }
	    elements[len++] = uc;
	 }

      }

      if ( closing_brace ) {
	 break;
      }

   }

   if ( opening_brace==0 || closing_brace==0 || within_number==1 ) {
      return(GSS_S_FAILURE);
   }


   if ( len<2 ) {
      /* *I* claim that this is not a valid OID */
      (*pp_min_stat) = 0;
      return(GSS_S_FAILURE);
   }

   /* compress the first two values of the OID into the first byte */
   elements[1] = elements[0] * 40 + elements[1];
   oid.elements = &(elements[1]);
   oid.length   = len-1;

   /* now make a dynamically allocated copy of this OID */
   maj_stat = xigss_copy_oid( pp_min_stat, &oid, pp_out_oid );

   return(maj_stat);

} /* xgss_str_to_oid() */




/*
 * xgss_release_oid()
 *
 *
 */
OM_uint32
xgss_release_oid( OM_uint32   * pp_min_stat,
		  gss_OID     * pp_in_oid
     )
{
   (*pp_min_stat) = 0;

   if ( (*pp_in_oid) == GSS_C_NO_OID ) {
      return(GSS_S_COMPLETE);
   }

   /* We try to free() the components, as they should have  */
   /* dynamically created by xgss_str_to_oid() ...	    */

   if ( (*pp_in_oid)->elements!=NULL ) {
      if ( (*pp_in_oid)->length>0 ) {
	 memset( (*pp_in_oid)->elements, 0, (*pp_in_oid)->length );
      }
      (*pp_in_oid)->elements = NULL;
      (*pp_in_oid)->length   = 0;
   }

   free( (*pp_in_oid) );

   (*pp_in_oid) = GSS_C_NO_OID;

   return(GSS_S_COMPLETE);

} /* xgss_release_oid() */

