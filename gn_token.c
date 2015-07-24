#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/gn_token.c#3 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/gn_token.c#3 $
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


/***********************************************************************
 * gn_compose_gss_token()
 *
 *  Description:
 *    Compose a gssapi token framing as outlined in RFC2078, Section 3.1:
 *
 *     1. 0x60 -- Tag for [APPLICATION 0] SEQUENCE; indicates that
 *     constructed form, definite length encoding follows.
 *
 *     2. Token length octets, specifying length of subsequent data
 *     (i.e., the summed lengths of elements 3-5 in this list, and of the
 *     mechanism-defined token object following the tag).  This element
 *     comprises a variable number of octets:
 *
 *     2a. If the indicated value is less than 128, it shall be
 *     represented in a single octet with bit 8 (high order) set to "0"
 *     and the remaining bits representing the value.
 *
 *     2b. If the indicated value is 128 or more, it shall be represented
 *     in two or more octets, with bit 8 of the first octet set to "1"
 *     and the remaining bits of the first octet specifying the number of
 *     additional octets.  The subsequent octets carry the value, 8 bits
 *     per octet, most significant digit first.  The minimum number of
 *     octets shall be used to encode the length (i.e., no octets
 *     representing leading zeros shall be included within the length
 *     encoding).
 *
 *     3. 0x06 -- Tag for OBJECT IDENTIFIER
 *
 *     4. Object identifier length -- length (number of octets) of the
 *     encoded object identifier contained in element 5, encoded per
 *     rules as described in 2a. and 2b. above.
 *
 *     5. Object identifier octets -- variable number of octets, encoded
 *     per ASN.1 BER rules:
 *
 *     5a. The first octet contains the sum of two values: (1) the top-
 *     level object identifier component, multiplied by 40 (decimal), and
 *     (2) the second-level object identifier component.  This special
 *     case is the only point within an object identifier encoding where
 *     a single octet represents contents of more than one component.
 *
 *     5b. Subsequent octets, if required, encode successively-lower
 *     components in the represented object identifier.  A component's
 *     encoding may span multiple octets, encoding 7 bits per octet (most
 *     significant bits first) and with bit 8 set to "1" on all but the
 *     final octet in the component's encoding.  The minimum number of
 *     octets shall be used to encode each component (i.e., no octets
 *     representing leading zeros shall be included within a component's
 *     encoding).
 *
 *     (Note: In many implementations, elements 3-5 may be stored and
 *     referenced as a contiguous string constant.)
 *
 *  Parameters:
 *    pp_min_stat     OUT REF   minor error code
 *    p_itoken        IN  REF   inner Token, prepared by gssapi mechanism
 *    p_mech_tag      IN        mech_oid_tag for token framing
 *    
 ***********************************************************************/


OM_uint32
gn_compose_gss_token( OM_uint32        * pp_min_stat,
		      Uchar            * p_itoken,
		      size_t             p_itoken_len,
		      gn_mech_tag_et     p_mech_tag,
		      gss_buffer_desc  * p_obuffer )
{
   char          * this_Call = "gn_compose_token";
   unsigned char * ptr;
   gss_OID_desc  * poid;
   size_t          output_len;
   size_t	   token_len;
   size_t          token_len_bytes;
   size_t          oid_len_bytes;
   unsigned char   enc_oidlen[sizeof(size_t)+4];
   unsigned char   enc_toklen[sizeof(size_t)+4];
   OM_uint32       maj_stat;

   (*pp_min_stat)    = 0;
   maj_stat          = GSS_S_COMPLETE;
   p_obuffer->length = 0;
   p_obuffer->value  = NULL;

   if ( (poid=gn_mech_tag_to_oid( p_mech_tag ))==GSS_C_NO_OID ) {
      DEBUG_ERR((tf, "Internal ERROR: %s(): unknown mech_oid tag %u!\n",
		 this_Call, (unsigned int) p_mech_tag ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }
   oid_len_bytes  = gn_encode_length( poid->length, &(enc_oidlen[0]) );

   token_len  = 0;
   token_len += 1;                /* length of Tag for OID		*/
   token_len += oid_len_bytes;    /* length of oid DER encoding		*/
   token_len += poid->length;	  /* length of oid elements             */
   token_len += p_itoken_len;     /* length of internal token		*/

   token_len_bytes = gn_encode_length( token_len, &(enc_toklen[0]) );
   output_len  = 0;
   output_len += 1;               /* length of Tag for GSS-API token    */
   output_len += token_len_bytes; /* length of token length info        */
   output_len += token_len;       /* length of carry-on luggage         */ 

   maj_stat = gn_alloc_buffer( pp_min_stat, p_obuffer, NULL, output_len, 0 );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ptr = p_obuffer->value;
   *(ptr++) = (unsigned char)0x60;         /* ASN.1 class APPLICATION */

   memcpy( ptr, &(enc_toklen[0]), token_len_bytes );
   ptr += token_len_bytes;

   *(ptr++) = (unsigned char)0x06;         /* ASN.1 type identifier "OID" */

   memcpy( ptr, &(enc_oidlen[0]), oid_len_bytes );
   ptr += oid_len_bytes;

   memcpy( ptr, poid->elements, poid->length );
   ptr += poid->length;

   memcpy( ptr, p_itoken, p_itoken_len );

   return(maj_stat);

} /* gn_compose_gss_token() */


/*
 * gn_predict_gss_token_len()
 *
 *
 */
OM_uint32
gn_predict_gss_token_len( OM_uint32        * pp_min_stat,
			  size_t             p_itoken_len,
			  gn_mech_tag_et     p_mech_tag,
			  size_t           * pp_out_len )
{
   char          * this_Call = "gn_predict_gss_token_len";
   gss_OID_desc  * poid;
   size_t          output_len;
   size_t	   token_len;
   size_t          token_len_bytes;
   size_t          oid_len_bytes;
   unsigned char   enc_oidlen[sizeof(size_t)+4];
   unsigned char   enc_toklen[sizeof(size_t)+4];
   OM_uint32       maj_stat;

   (*pp_min_stat)    = 0;
   maj_stat          = GSS_S_COMPLETE;
   (*pp_out_len)     = 0;

   if ( (poid=gn_mech_tag_to_oid( p_mech_tag ))==GSS_C_NO_OID ) {
      DEBUG_ERR((tf, "Internal ERROR: %s(): unknown mech_oid tag %u!\n",
		 this_Call, (unsigned int) p_mech_tag ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }
   oid_len_bytes  = gn_encode_length( poid->length, &(enc_oidlen[0]) );

   token_len  = 0;
   token_len += 1;                /* length of Tag for OID		*/
   token_len += oid_len_bytes;    /* length of oid DER encoding		*/
   token_len += poid->length;	  /* length of oid elements             */
   token_len += p_itoken_len;     /* length of internal token		*/

   token_len_bytes = gn_encode_length( token_len, &(enc_toklen[0]) );
   output_len  = 0;
   output_len += 1;               /* length of Tag for GSS-API token    */
   output_len += token_len_bytes; /* length of token length info        */
   output_len += token_len;       /* length of carry-on luggage         */ 

   (*pp_out_len) = output_len;

   return(maj_stat);

} /* gn_predict_gss_token_len() */




/***********************************************************************
 * gn_encode_length()
 *
 *  Description:
 *    encode the length according to the rules in RFC2078, Section 3.1:
 *
 *     2a. If the indicated value is less than 128, it shall be
 *     represented in a single octet with bit 8 (high order) set to "0"
 *     and the remaining bits representing the value.
 *
 *     2b. If the indicated value is 128 or more, it shall be represented
 *     in two or more octets, with bit 8 of the first octet set to "1"
 *     and the remaining bits of the first octet specifying the number of
 *     additional octets.  The subsequent octets carry the value, 8 bits
 *     per octet, most significant digit first.  The minimum number of
 *     octets shall be used to encode the length (i.e., no octets
 *     representing leading zeros shall be included within the length
 *     encoding).
 *
 *  Arguments:
 *     p_length   IN       length value that is to be encoded
 *     p_buffer   OUT REF  buffer for encoded representation
 *
 *  Return value:
 *     number of octects for the encoded representation
 *
 **********************************************************************/
size_t
gn_encode_length( size_t p_length, unsigned char *p_buffer )
{
   unsigned char  tmpbuf[sizeof(size_t)+2];
   size_t         len, i;

   if ( p_length<128 ) {
      p_buffer[0] = (unsigned char) p_length;
      return( 1 );
   }

   for ( len=1; len<=sizeof(size_t) ; len++ ) {
      tmpbuf[len] = (unsigned char )(p_length & 0xff);
      p_length = (p_length>>8);
      if (p_length==0)
	 break;
   }

   p_buffer[0] = (unsigned char) (len | 0x80);
   for (i=0; i<len; i++) {
      p_buffer[i+1] = tmpbuf[len-i];
   }

   return(len+1);

} /* gn_encode_length() */



/*
 * gn_decode_length()
 *
 *
 */
OM_uint32
gn_decode_length( OM_uint32     * pp_min_stat,
		  unsigned char * p_buf,
		  size_t          p_buflen,
		  size_t        * pp_nbytes,
		  size_t        * pp_value )
{
   char            * this_Call = "gn_decode_length";
   size_t	     nbytes;
   size_t            len, i;
   OM_uint32         length;
   OM_uint32	     limit;
   OM_uint32	     maj_stat;

   (*pp_min_stat) = 0;
   (*pp_nbytes)   = 0;
   (*pp_value)    = 0;

   if ( 0==p_buflen )
      goto error;

   len = (size_t) (*p_buf);

   if ( len <= 0x7fu ) {
      nbytes = 1;
      length = (OM_uint32)len;
      goto done_decoding_length;
   }

   nbytes = (len & 0x7fu);
   if ( nbytes + 1 > p_buflen ) {
       /* error -- buffer is shorter than length field */
       goto error;
   }

   length = 0u;
   limit  = ((OM_uint32)-1)>>8;
   for ( i=1 ; i<=nbytes ; i++ ) {
      if ( length>limit ) {
         DEBUG_ERR((tf, "  E: %s(): asn1 length field exceed 32-bit unsigned int\n",
		          this_Call ));
	 goto error;
      }
      length  = (length<<8);
      length += (unsigned long)(p_buf[i]);
      if (length==0) {
         DEBUG_STRANGE((tf, "  S: %s(): DER-violation: leading zeroes in length\n",
		          this_Call ));
      }
   }
   nbytes += 1; /* add the length byte of the length field */

done_decoding_length:
   if ( length <= (p_buflen - nbytes) ) {
      maj_stat     = GSS_S_COMPLETE;
      (*pp_value)  = length;
      (*pp_nbytes) = nbytes;
   } else {
error:
      (*pp_min_stat) = MINOR_INVALID_LENGTH_FIELD;
      maj_stat       = GSS_S_DEFECTIVE_TOKEN;
   }

   return(maj_stat);

} /* gn_decode_length() */



/*
 * gn_parse_gss_token()
 *
 *
 */
OM_uint32
gn_parse_gss_token( OM_uint32        *  pp_min_stat,
		    gss_buffer_desc  *  p_itoken,
		    gss_OID_desc     *  p_token_oid, /* optional */
		    gn_mech_tag_et   *  pp_mech_tag,
		    void             ** pp_odata,
		    size_t           *  pp_olen )
{
   char            * this_Call = "gn_parse_gss_token";
   unsigned char   * ptr;
   gss_OID_desc      oid;
   OM_uint32         maj_stat;
   size_t            oid_len, netto_len, skip, len;

   (*pp_mech_tag) = MECH_INVALID_TAG;
   (*pp_odata)    = NULL;
   (*pp_olen)     = 0;
   if ( p_token_oid!=0 ) {
       p_token_oid->elements = 0;
       p_token_oid->length   = 0;
   }

   if ( p_itoken==NULL ) {
      RETURN_MIN_MAJ( MINOR_INVALID_BUFFER, GSS_S_CALL_INACCESSIBLE_READ );
   }

   len = p_itoken->length;
   ptr = p_itoken->value;

   if ( len==0  ||  ptr==NULL ) {
      RETURN_MIN_MAJ( MINOR_INVALID_BUFFER, GSS_S_CALL_BAD_STRUCTURE );
   }

   if ( len<4  ||  (*ptr)!=0x60 ) { /* check ASN.1 class APPLICATION */
      RETURN_MIN_MAJ( MINOR_NOT_A_TOKEN, GSS_S_DEFECTIVE_TOKEN );
   }

   ptr += 1;       len -= 1;  /* skip ASN.1 class tag */

   maj_stat = gn_decode_length( pp_min_stat, ptr, len, &skip, &netto_len );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ptr += skip;    len -= skip; /* skip netto_len encoded length field */

   if ( len<2  ||  (*ptr)!=0x06 ) { /* check ASN.1 type identifier "OID" */
      RETURN_MIN_MAJ( MINOR_NOT_A_TOKEN, GSS_S_DEFECTIVE_TOKEN );
   }

   if ( len!=netto_len ) { /* verify token length */
       RETURN_MIN_MAJ( (len<netto_len) ? MINOR_MAY_BE_TRUNCATED : MINOR_TRAILING_GARBAGE,
		       GSS_S_DEFECTIVE_TOKEN );
   }

   ptr += 1;       len -= 1;    /* skip ASN.1 type identifier "OID" */

   maj_stat = gn_decode_length( pp_min_stat, ptr, len, &skip, &oid_len );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   ptr += skip;    len -= skip; /* skip oid_len encoded length field */

   oid.length   = (OM_uint32)oid_len;
   oid.elements = ptr;
   if ( p_token_oid!=0 ) {
       p_token_oid->length   = (OM_uint32)oid_len;
       p_token_oid->elements = ptr;
   }

   maj_stat = gn_oid_to_mech_tag( pp_min_stat, &oid, pp_mech_tag );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   if ( *pp_mech_tag==MECH_INVALID_TAG ) {
      RETURN_MIN_MAJ( MINOR_UNKNOWN_MECH_OID, GSS_S_BAD_MECH );
   }

   ptr += oid_len; len -= oid_len; /* skip encoded OID value (elements) */

   (*pp_odata) = ptr;
   (*pp_olen)  = len;

   return(GSS_S_COMPLETE);

} /* gn_parse_gss_token() */



/******************************************************************
 * gn_parse_expname_token()
 *
 *  Description:
 *    Parse the framing of the exported names format defined in
 *    GSS-API v2 (RFC2078), Section 3.2:
 *
 *    3.2: Mechanism-Independent Exported Name Object Format
 *
 *       Length    Name          Description
 *
 *      2               TOK_ID          Token Identifier
 *                                      For exported name objects, this
 *                                      must be hex 04 01.
 *      2               MECH_OID_LEN    Length of the Mechanism OID
 *      MECH_OID_LEN    MECH_OID        Mechanism OID, in DER
 *      4               NAME_LEN        Length of name
 *      NAME_LEN        NAME            Exported name; format defined in
 *                                      applicable mechanism draft.
 *
 *
 ******************************************************************/
OM_uint32
gn_parse_expname_token( OM_uint32         *  pp_min_stat,
			gss_buffer_desc   *  p_ibuffer,
			gn_mech_tag_et    *  pp_mech_tag,
			void              ** pp_oname,
			size_t            *  pp_olen )
{
   unsigned char   * ptr;
   gss_OID_desc      oid;
   OM_uint32	     maj_stat;
   size_t            len;
   size_t            oid_len;
   size_t            nbytes_oid_len;
   size_t            mech_oid_len;
   unsigned long     name_len;
   gn_mech_tag_et    mech_tag;

   (*pp_oname)    = NULL;
   (*pp_olen)     = 0;
   (*pp_mech_tag) = MECH_INVALID_TAG;

   if ( p_ibuffer==NULL
	||  p_ibuffer->length==0  ||  p_ibuffer->value==NULL ) {
      RETURN_MIN_MAJ( MINOR_INVALID_BUFFER, GSS_S_BAD_NAME );
   }

   len = p_ibuffer->length;
   ptr = p_ibuffer->value;

   /* Checking TOK_ID */
   if ( len<9  ||  ptr[0]!= 0x04  ||  ptr[1] != 0x01 ) {
      RETURN_MIN_MAJ( MINOR_NOT_A_TOKEN, GSS_S_BAD_NAME );
   }

   /* calculating MECH_OID_LEN from 2-byte field */
   /* (most significant octet first)             */
   mech_oid_len = (((unsigned int)ptr[2])<<8) + ptr[3];

   /* length plausibility check */
   if ( len < 8 + mech_oid_len ) {
      RETURN_MIN_MAJ( MINOR_MAY_BE_TRUNCATED, GSS_S_BAD_NAME );
   }

   /* check for ASN.1 class and type identifier for OID (0x06) */
   if ( ptr[4]!= 0x06 ) {
      RETURN_MIN_MAJ( MINOR_INVALID_OID, GSS_S_BAD_NAME );
   }

   /* decode length field from DER-encoded OID */
   maj_stat = gn_decode_length( pp_min_stat, &(ptr[5]),
				len-5, &nbytes_oid_len, &oid_len );

   if ( maj_stat!=GSS_S_COMPLETE ) {
      return(GSS_S_BAD_NAME);
   }

   /* check if the outer length field for the DER-encoded OID is */
   /* equal to the sum of the length of the components           */
   if ( mech_oid_len != ( 1 + nbytes_oid_len + oid_len ) ) {
      RETURN_MIN_MAJ( MINOR_WRONG_LEN_EXPNAME, GSS_S_BAD_NAME );
   }

   oid.length   = (OM_uint32)oid_len;
   oid.elements = (void *)&(ptr[5+nbytes_oid_len]);

   /* look up mech_oid_tag for the mechanism OID in this exported name */
   maj_stat = gn_oid_to_mech_tag( pp_min_stat, &oid, &mech_tag );
   if ( maj_stat!=GSS_S_COMPLETE ) {
      return(GSS_S_BAD_NAME);
   }

   if ( mech_tag==MECH_INVALID_TAG ) {
      /* mechanism OID not recognized */
      RETURN_MIN_MAJ( MINOR_UNKNOWN_MECH_OID, GSS_S_BAD_NAME );
   }

   /* skip past the mechanism OID */
   len  -= ( 4 + mech_oid_len );
   ptr  += ( 4 + mech_oid_len );

   if ( len<4 ) {
      RETURN_MIN_MAJ( MINOR_MAY_BE_TRUNCATED, GSS_S_BAD_NAME );
   }

   /* parse the 4-byte length field for the name */
   /* (most significant octet first)             */
   name_len = ptr[0]; 
   name_len = (name_len<<8) + ptr[1];
   name_len = (name_len<<8) + ptr[2];
   name_len = (name_len<<8) + ptr[3];

   len -= 4;
   ptr += 4;

   if ( name_len!=len ) {
      RETURN_MIN_MAJ( MINOR_WRONG_LEN_EXPNAME, GSS_S_BAD_NAME );
   }

   (*pp_mech_tag)  = mech_tag;
   (*pp_oname)     = ptr;
   (*pp_olen)      = len;

   return(GSS_S_COMPLETE);

} /* gn_parse_expname_token() */




/*
 * gn_create_expname_token()
 *
 */
OM_uint32
gn_create_expname_token( OM_uint32          * pp_min_stat,
			 void               * p_name,
			 size_t               p_len,
			 gn_mech_tag_et       p_mech_tag,
			 gss_buffer_desc    * p_obuffer )
{
   char          * this_Call = "gn_create_expname_token";
   size_t          olen;
   unsigned char   enc_oidlen[sizeof(size_t)+4];
   size_t          der_oid_len;
   size_t          oid_len_bytes;
   gss_OID_desc  * poid;
   unsigned char * ptr;

   p_obuffer->value  = NULL;
   p_obuffer->length = 0;

   if ( (poid=gn_mech_tag_to_oid( p_mech_tag ))==GSS_C_NO_OID ) {
      DEBUG_ERR((tf, "Internal ERROR: %s(): unknown mech_oid tag %u!\n",
		 this_Call, (unsigned int)p_mech_tag ))
      RETURN_MIN_MAJ( MINOR_INTERNAL_ERROR, GSS_S_FAILURE );
   }

   /* DER-encode the length of the OID elements */
   oid_len_bytes = gn_encode_length( poid->length, &(enc_oidlen[0]) );

   /* calculate full length of OID in DER-encoding:     */
   /*            tag(0x06) + encoded length + elements  */
   der_oid_len = 1 + oid_len_bytes + poid->length;

   /* calculate full length of exported name token */
   /* tag(0401) + 2-byte length of OID + full DER-encoded OID   */
   /*             + 4-byte length of mech-token + mech-token    */
   olen = 2 + 2 + der_oid_len + 4 + p_len;
   ptr = sy_malloc( olen );
   if ( ptr==NULL ) {
      RETURN_MIN_MAJ( MINOR_OUT_OF_MEMORY, GSS_S_FAILURE );
   }

   p_obuffer->value  = ptr;
   p_obuffer->length = olen;

   ptr[0] = 0x04;
   ptr[1] = 0x01;

   /* fill the 2-byte length field for inlined DER-encoded     */
   /* mechanism OID,  most significant octet first             */
   /* (btw. this is "IP network byte order" aka "bigendian"    */
   ptr[2] = (unsigned char) ((der_oid_len>>8) & 0xff);
   ptr[3] = (unsigned char) (der_oid_len & 0xff);

   ptr[4] = 0x06;
   
   ptr += 5;

   memcpy( ptr, &(enc_oidlen[0]), oid_len_bytes );
   ptr += oid_len_bytes;

   memcpy( ptr, poid->elements, poid->length );
   ptr += poid->length;

   /* fill the 4-byte length field for the mechanism-specific     */
   /* exported name part,  most significant octet first           */
   /* (btw. this is "IP network byte order" aka "bigendian"       */
   ptr[0] = (unsigned char) (p_len>>24) & 0xff;
   ptr[1] = (unsigned char) (p_len>>16) & 0xff;
   ptr[2] = (unsigned char) (p_len>>8)  & 0xff;
   ptr[3] = (unsigned char) (p_len) & 0xff;

   memcpy( &(ptr[4]), p_name, p_len );

   return(GSS_S_COMPLETE);

} /* gn_create_expname_token() */
