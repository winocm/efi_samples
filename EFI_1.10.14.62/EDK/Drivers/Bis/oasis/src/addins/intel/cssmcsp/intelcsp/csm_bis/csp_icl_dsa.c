/*-----------------------------------------------------------------------
 *      File:   csp_icl_dsa.c
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *-----------------------------------------------------------------------
 */
/* 
 * INTEL CONFIDENTIAL 
 * This file, software, or program is supplied under the terms of a 
 * license agreement or nondisclosure agreement with Intel Corporation 
 * and may not be copied or disclosed except in accordance with the 
 * terms of that agreement. This file, software, or program contains 
 * copyrighted material and/or trade secret information of Intel 
 * Corporation, and must be treated as such. Intel reserves all rights 
 * in this material, except as the license agreement or nondisclosure 
 * agreement specifically indicate. 
 */ 
/* 
 * WARNING: EXPORT RESTRICTED. 
 * This software is subject to the U.S. Export Administration Regulations 
 * and other U.S. law, and may not be exported or re-exported to certain 
 * countries (currently Afghanistan (Taliban-controlled areas), Cuba, Iran, 
 * Iraq, Libya, North Korea, Serbia (except Kosovo), Sudan and Syria) or to 
 * persons or entities prohibited from receiving U.S. exports (including Denied 
 * Parties, Specially Designated Nationals, and entities on the Bureau of 
 * Export Administration Entity List or involved with missile technology or 
 * nuclear, chemical or biological weapons).
 */ 
/*
 * This file contains the fucntions use ICL DSA data structures and
 * the decode functions for DSA signaturing.
 */

#include "tal_inc.h"
#include "csp_inc.h"
#include "csp_icl_dsa.h"
#include "ber_der.h"

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                      Static function declaration                         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
static sint32	s_BER2verylong(const uint8 *InOctet_ptr,
							   uint32      InOctetSize, 
                               uint8       *bignum_ptr,
							   int         bignumBufSize);

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                      Static function definition                          */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: s_BER2verylong
 *
 *Description:
 * This function decodes a large BER integer into an array of unsigned long
 * values,  returns count of unsigned long, or zero if the number is too large
 * (ignoring sign). The BER integer has ?. 
 * (NOTE: zero has a length of one).
 *
 *Parameters: 
 * InOctets_ptr(input) -  pointer to input BER_BIT_STRING octets
 * InOctetsLen(Input) -   The Length of InOctets_ptr;
 * bignum_ptr(output) -   The output buffer
 * bignumBufSize(input) - The size of output buffer bignum_ptr
 *
 *Returns:
 * sint32 - The count of unsigned long values in the octect
 * -1     - Invalid input Octets
 * -2     - The number is too large
 *---------------------------------------------------------------------------*/
static sint32
s_BER2verylong(const uint8 *InOctet_ptr,
			   uint32      InOctetSize,
               uint8       *bignum_ptr,
			   int         bignumBufSize)
{
    sint32          i, Length;
    const  uint8    *octet_ptr;
	BER_PARSED_ITEM parsedBERItem;

	if (InOctet_ptr == NULL)
	{
		return -1;
	}
    octet_ptr = &InOctet_ptr[0];
    parsedBERItem.Tag = NULL;
	parsedBERItem.ContentLength = 0;
    parsedBERItem.Content = NULL;

	/* expand a single BER item */
    octet_ptr = BER_ExpandItem(InOctet_ptr, InOctetSize, &parsedBERItem);
	if ((octet_ptr == NULL) || 
		(parsedBERItem.Tag[0] != BER_INTEGER))
	{
			return(-1);
	}
    octet_ptr = parsedBERItem.Content;

	/* error: number does not fit */
	Length = parsedBERItem.ContentLength;
    if (Length > bignumBufSize)
		return -2;

    /* clean the output buffer */
    cssm_memset(bignum_ptr, 0, bignumBufSize);

	/* value zero has a length of one, special case for ICL */
    if (Length == 0)
	{
		Length = 1;
		return Length;
	}

    /* test endian value dynamically */
    i = 0x1234;
    if (((uint8*) &i)[0] == 0x34)	// LITTLE ENDIAN -- intel
    {
        for (i = 0; i < Length; i++) 
            bignum_ptr[Length-i-1] = octet_ptr[i];

        while ((Length > 1) && (bignum_ptr[Length-1] == 0))
			Length--;
    } 
    else                            // BIG ENDIAN
    {
        cssm_memcpy(bignum_ptr +(bignumBufSize-Length), octet_ptr, Length);
    }
    return 	Length;
}

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                    ICL DSA datastructure functions                       */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: Internal_CreateICLDSAPubKeyStruct
 *
 *Description:
 * This function creates a ICLDSSPublicKey sturcture in memory. Caller has
 * to call Internal_DeleteICLDSAPubKeyStruct to free the data structure.
 *
 *Returns:
 * ICLDSSPublicKey - a pointer to the new created ICLDSSPublicKey sturcture
 * NULL            - Memory error
 *---------------------------------------------------------------------------*/
ICLDSSPublicKey*  Internal_CreateICLDSAPubKeyStruct()
{
    ICLDSSPublicKey	*pDssPub;
    uint8           *v;     
    uint32          p_size=MODULUSBYTES+1, q_size=MODULUSBYTES+1;
    uint32          g_size=MODULUSBYTES+1, y_size=MODULUSBYTES+1, nAllocSize=0;

    nAllocSize = sizeof(ICLDSSPublicKey) + p_size + q_size + g_size + y_size;
    if ((v = cssm_calloc(1, nAllocSize, NULL)) == NULL)
        return NULL;

    pDssPub = (ICLDSSPublicKey*)v;
    v += sizeof(ICLDSSPublicKey);
    /* p */
    pDssPub->PrimeModulus.value = (uint8*)v;
    pDssPub->PrimeModulus.length = p_size;
    v += p_size;
    /* q */
    pDssPub->PrimeDivisor.value = (uint8*)v;
    pDssPub->PrimeDivisor.length = q_size;
    v += q_size;
    /* g */
    pDssPub->OrderQ.value = (uint8*)v;
    pDssPub->OrderQ.length = g_size;
    v += g_size;
    /* y */
    pDssPub->PublicKey.value = (uint8*)v;
    pDssPub->PublicKey.length = y_size;

    return pDssPub;
}

/*-----------------------------------------------------------------------------
 *Name: Internal_DeleteICLDSAPubKeyStruct
 *
 *Description:
 * This function frees the space located for ICLDSSPublicKey.
 *
 *Parameters: 
 * pPubKey(input) - A pointer to ICLDSSPublicKey structure to be freed
 *
 *Returns:
 * none
 *---------------------------------------------------------------------------*/
void Internal_DeleteICLDSAPubKeyStruct(ICLDSSPublicKey* pPubKey)
{
    cssm_free(pPubKey, NULL);
}

/*-----------------------------------------------------------------------------
 *Name: Internal_BERDecode_ICLDSAPublicKey
 *
 *Description:
 * This function BER decoded a ICLRSAPublicKey.
 *
 *Parameters:
 * pBERKey(input) - a BerDerencoded DSA public key
 * pPubKey(output)- a pointer to output ICLDSSPublicKey structure buffer
 *
 *Returns:
 * CSSM_OK - Decode succeeded
 * CSSM_FAIL - Decode failed
 *---------------------------------------------------------------------------*/
CSSM_RETURN	Internal_BERDecode_ICLDSAPublicKey(const CSSM_KEY_PTR pBERKey,
                                          ICLDSSPublicKey* pPubKey)
{
	/* DSA public key has p,q,g,y 4 components */
    uint32  outDataCount = DSA_PUBKEY_PARAM_COUNT;

    if (TAL_ParseKeyBlob(pBERKey, (CSSM_DATA_PTR)pPubKey, &outDataCount) != 
		CSSM_OK)
	{
		return CSSM_FAIL;
	}

	if (outDataCount != DSA_PUBKEY_PARAM_COUNT)
	{
		return CSSM_FAIL;
	}
	return CSSM_OK;
}

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*		                  DSA Signature Decode functions                    */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/

/*-----------------------------------------------------------------------------
 *Name: Internal_BERDecode_DSASignature
 *
 *Description:
 * This function decoded a DSA signature octets. A DSA signature is a constructed
 * sequence of signature r and signature s.
 * The following is a tipical DSA signature break down:
 *
 *	30 2C	                 Tag and Length - Constructed Sequence 
 *		  02 14              Tag and Length - Integer
 *			    0F CB 91 9E B3 D2 8B 40    71 34 87 66 81 E6 18 0B
 *			    1F 15 FA 96
 * 		  02 14              Tag and Length - Integer
 *			    7E A8 99 52 93 43 DA 11    41 22 0A BF 01 97 3C BB
 *			    70 E0 7C 66
 *
 *Parameters:
 * Octets(input)- a pointer to the input octect string
 * InOctetCount(input)- number of bytes in the input octets
 * DataBufs(output) - pointer to output decoded ICLData structures
 * DataBufCount(output) - number of supplied output DataBufs
 *
 *Returns:
 * sint16 - Number of decoded ICLData structures
 * -1     - Error
 *---------------------------------------------------------------------------*/
sint16 Internal_BERDecode_DSASignature(const uint8 *InOctet_ptr,
									   uint32      InOctetSize,
									   ICLData     *DataBufs,
									   sint16      DataBufCount)
{
    sint16	     i;
    const  uint8 *octet_ptr=NULL;
    BER_PARSED_ITEM parsedBERItem;

    octet_ptr = &InOctet_ptr[0];
    parsedBERItem.Tag = NULL;
	parsedBERItem.ContentLength = 0;
    parsedBERItem.Content = NULL;

	if (InOctet_ptr == NULL)
	{
		return -1;
	}
	/* expand a single BER item */
    octet_ptr = BER_ExpandItem(octet_ptr, InOctetSize, &parsedBERItem);
    if ((octet_ptr == NULL) || 
        (parsedBERItem.Tag[0] != BER_CONSTRUCTED_SEQUENCE))
	{
        return(-1);
	}
    /* Decode all the embeded octet string and copy to DataBufs */
    octet_ptr = parsedBERItem.Content;
    for (i = 0; i < DataBufCount; i++)
    {
        DataBufs[i].length = s_BER2verylong(octet_ptr,
                                            parsedBERItem.ContentLength,
			                                DataBufs[i].value,
									        DataBufs[i].length);
        /* output buffer was too small */
        if (DataBufs[i].length == 0) 
		{
            return -1;
		}
		/* point to next octet */
        octet_ptr += BER_SizeofObject(octet_ptr);
    }
    return (i);
}
