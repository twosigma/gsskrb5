#include "mysccs.h"
SCCSID("@(#)$Id: //tools/src/freeware/gsskrb5/cmp_name.c#1 $")

/************************************************************************
 * $Id: //tools/src/freeware/gsskrb5/cmp_name.c#1 $
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
 * gn_gss_compare_name()
 *
 *
 */
OM_uint32
gn_gss_compare_name(
	OM_uint32	FAR *	pp_min_stat,	/* minor_status		*/
	gss_name_t		p_in_name1,	/* name1		*/
	gss_name_t		p_in_name2,	/* name2		*/
	int		FAR *	pp_out_result	/* name_equal		*/
     )
{
   char          * this_Call = " gn_gss_compare_name(): ";
   gn_name_desc  * pname1;
   gn_name_desc  * pname2;
   gn_name_desc  * xchgname;
   Uchar         * tmp_name      = NULL;
   size_t          tmp_name_len  = 0;
   gn_nt_tag_et    tmp_nt_tag;
   OM_uint32       maj_stat;

   (*pp_min_stat)   = 0;
   (*pp_out_result) = FALSE;
   maj_stat         = GSS_S_COMPLETE;

   maj_stat = gn_check_name( pp_min_stat, &p_in_name1, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   maj_stat = gn_check_name( pp_min_stat, &p_in_name2, this_Call );
   if (maj_stat!=GSS_S_COMPLETE)
      return(maj_stat);

   pname1 = p_in_name1;
   pname2 = p_in_name2;

   if ( pname1->nt_tag==NT_ANONYMOUS
	||  pname2->nt_tag==NT_ANONYMOUS ) {
      /* RFC2078: 7.6: "If either name presented to GSS_Compare_name()     */
      /*                denotes an anonymous principal, GSS_Compare_name() */
      /*                shall indicate FALSE"                              */
      (*pp_out_result) = FALSE;
      return(GSS_S_COMPLETE);
   }

   if ( pname1->mech_tag==MECH_INVALID_TAG
	&&  pname2->mech_tag==MECH_INVALID_TAG ) {
      /* neither name is a MN, so direct comparison may be impossible      */
      /* we can detect exact matches, but we cannot reliably say anything  */
      /* about differing names                                             */

      if ( pname1->next==NULL
	   &&  pname2->next==NULL
	   &&  pname1->nt_tag == pname2->nt_tag
	   &&  pname1->prname_len == pname2->prname_len
	   &&  !memcmp(pname1->prname, pname2->prname, pname1->prname_len ) ) {

	 /* This looks like an exact match, so return TRUE */ 
	 (*pp_out_result) = TRUE;
	 return(GSS_S_COMPLETE);
      }

      /* These names are incomparable, because neither is a MN */
      RETURN_MIN_MAJ( MINOR_NEED_ONE_MECHNAME, GSS_S_BAD_NAMETYPE );

   } else if ( pname1->mech_tag!=MECH_INVALID_TAG
	       &&  pname2->mech_tag!=MECH_INVALID_TAG ) {
      /* both names are MNs */

      if ( pname1->mech_tag!=pname2->mech_tag ) {

	 /* The two MNs have the same Nametype but differing mechanisms  */
	 return(GSS_S_BAD_NAMETYPE);

      } else {

	 /* The two MNs have the same Nametype and the same Mechanism */
	 /* so the binary (canonical) representations are comparable  */ 

	 if ( pname1->name_len==pname2->name_len
	      &&  !memcmp(pname1->name, pname2->name, pname1->name_len) ) {

	    (*pp_out_result) = TRUE;

	 } else {

	    (*pp_out_result) = FALSE;
	 }

      }

   } else { /* one MN, one non-MN */

      if ( pname1->mech_tag==MECH_INVALID_TAG ) {
	 /* pname1 is a non-MN, but we want to have pname2 be the non-MN */
	 xchgname = pname1;  pname1 = pname2;  pname2 = xchgname;
      }

      /* pname2 is a non-MN now                  */
      /* canonicalize pname2 to the mechanism of */
      /* of the MN pname1 and compare the names  */

/* Cleanup ALERT:  the following call create a dynamic object for */
/*                 (tmp_name), which needs to be freed when       */
/*                 there is an error later on!                    */
      maj_stat = (gn_mech[pname1->mech_tag]
		         ->fp_canonicalize_name)( pp_min_stat,
						  pname2->prname,
						  pname2->prname_len,
						  pname2->nt_tag,
						  &tmp_name,
						  &tmp_name_len,
						  &tmp_nt_tag );
      if (maj_stat!=GSS_S_COMPLETE)
	 goto cleanup;

      if ( tmp_name_len==pname1->name_len
	   &&  !memcmp(tmp_name, pname1->name, tmp_name_len) ) {

	 /* internal binary names match */ 
	 (*pp_out_result) = TRUE;

      } else {

	 /* internal binary names do not match */
	 (*pp_out_result) = FALSE;

      }

   }

cleanup:
   if ( tmp_name!=NULL ) {
      sy_clear_free( (void **) &tmp_name, tmp_name_len );
   }

   return(maj_stat);

} /* gn_gss_compare_name() */
