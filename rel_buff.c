#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/rel_buff.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/rel_buff.c#1 $
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


/*
 * gn_alloc_buffer()
 * 
 *
 *
 */
OM_uint32
gn_alloc_buffer( OM_uint32    * pp_min_stat,
		 gss_buffer_t   p_buffer,
		 void         * p_source,
		 size_t         p_source_len,
		 int            p_add_nul
     )
{
   char   * this_Call = "gn_alloc_buffer";
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

   ptr = sy_malloc( dst_len );
   if ( ptr==NULL ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   if ( p_source!=NULL && p_source_len>0 ) {
      memcpy( ptr, p_source, p_source_len );
   }

   p_buffer->value  = ptr;
   p_buffer->length = p_source_len;
   if ( p_add_nul ) {
      ((unsigned char *)ptr)[dst_len-1] = '\0';
   }

/*
   DEBUG_ACTION((tf, "  A: %s(): new buffer { length= %lu, value= ptr:%p }\n",
		     this_Call,
		     (unsigned long) p_buffer->length,
		     p_buffer->value ));
*/
   return(GSS_S_COMPLETE);

} /* gn_alloc_buffer() */




/*
 * gn_gss_release_buffer()
 *
 *
 *
 */
OM_uint32
gn_gss_release_buffer( OM_uint32 * p_min_stat, gss_buffer_t p_buffer )
{
   char        * this_Call = "gn_gss_release_buffer";

   (*p_min_stat) = 0;

   if ( p_buffer == GSS_C_NO_BUFFER ) {
/*    DEBUG_ACTION((tf, "  A: %s(): ignoring GSS_C_NO_BUFFER\n", this_Call)); */
      return(GSS_S_COMPLETE);
   }

   if ( p_buffer->value==NULL  &&  p_buffer->length==0 ) {
/*    DEBUG_ACTION((tf, "  A: %s(): ignoring GSS_C_EMPTY_BUFFER\n", this_Call )); */
      return(GSS_S_COMPLETE);
   }

   if ( p_buffer->length!=0  &&  p_buffer->value!=NULL ) {
/*    DEBUG_ACTION((tf, "  A: %s(): free()ing { length= %lu, value= ptr:%p }\n", this_Call, 
		        (unsigned long)(p_buffer->length), p_buffer->value )); */
      /* clear and free the buffer value and clear the gss_buffer_desc */
      sy_clear_free( &(p_buffer->value), p_buffer->length );
      p_buffer->length = (size_t)0;

      return(GSS_S_COMPLETE);
   }

   /* Inconsistent values for p_buffer->length and p_buffer->value */
   /* We probably didn't create this thing, so we leave it alone.  */
   DEBUG_ERR((tf, "  E: %s(): { value= %p, length= %lu } is not a valid buffer\n",
		     this_Call, p_buffer->value,
		     (unsigned long)p_buffer->length ));

   (*p_min_stat) = MINOR_INVALID_BUFFER;

   return(GSS_S_CALL_BAD_STRUCTURE);

} /* gn_gss_release_buffer() */
