/*-----------------------------------------------------------------------
 *      File:   csp_qksz.c
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
 * This file contains the CSP for WfM BIS query key size functions.
 */

/* The modulus of a dsa key may not be exact to 512, 768, 1024, or 2048 bits. 
   This macro rounds down the modulus to match the advitised capabilities.
   32 is the number that satisfies all the modulus size. */
#define PKI_EFFECTIVE_KEY_LEN(a, b) (((a/32)*32)*8)

#include "tal_inc.h"
#include "csp_icl_dsa.h"

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          Crypto functions                                */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: TALCSP_QueryKeySizeInBits
 *
 *Description:
 * This function queries a crypto service provider for the logical effective
 * and physical size of a DSA key in bits.
 *
 *Parameters: 
 * Key_ptr(input) - Pointer to the CSSM_KEY data structure, for which the size
 *                  is requested.
 * KeySize_ptr(output) - Pointer to a CSSM_KEYSIZE data structure to receive
 *                       the size of the key in bits.
 *
 *Returns:
 * CSSM_OK -   Query key size in bits succeeded
 * CSSM_FAIL - Query key size in bits failed
 *
 *Error Codes:
 * error codes set by this function
 *	 CSSM_CSP_INVALID_ALGORITHM
 *   CSSM_CSP_KEY_BLOBTYPE_INCORRECT
 *   CSSM_CSP_OPERATION_FAILED
 *
 * error codes set by sub-functions
 *   CSSM_CSP_INVALID_POINTER
 *   CSSM_CSP_INVALID_KEY
 *   CSSM_CSP_KEY_FORMAT_INCORRECT
 *   CSSM_CSP_KEY_BLOBTYPE_INCORRECT
 *   CSSM_CSP_KEY_KEYHEADER_INCONSISTENT
 *   CSSM_CSP_NOT_ENOUGH_BUFFER
 *   CSSM_CSP_ERR_OUTBUF_LENGTH
 *   CSSM_CSP_INVALID_KEYCLASS
 *   CSSM_CSP_MEMORY_ERROR
 *   CSSM_CSP_INVALID_KEY
 *   CSSM_CSP_INVALID_POINTER
 *   CSSM_CSP_INVALID_KEYCLASS
 *---------------------------------------------------------------------------*/
CSSM_RETURN TALCSP_QueryKeySizeInBits(const CSSM_KEY_PTR Key_ptr,
                                      CSSM_KEY_SIZE_PTR KeySize_ptr)
{
    CSSM_RETURN     rtn=CSSM_OK;

    KeySize_ptr->KeySizeInBits = 0;
    KeySize_ptr->EffectiveKeySizeInBits = 0;

    /* Only query the key size for DSA key */
    if (Key_ptr->KeyHeader.AlgorithmId != CSSM_ALGID_DSA)
    {
        TAL_SetError(CSSM_CSP_INVALID_ALGORITHM);
        return CSSM_FAIL;
    }
    /* Only parse the key which has EffectiveKeySizeInBits set to 
       CSSM_KEYBLOB_RAW or CSSM_KEYBLOB_RAW_BERDER. The EffectiveKeySizeInBits 
	   is set to the EffectiveKeySizeInBits value in the key header for other 
	   key blob type. */
    switch (Key_ptr->KeyHeader.BlobType)
    {
/*########################### CSSM_BIS ###########################*/
#ifndef CSSM_BIS
    case CSSM_KEYBLOB_REFERENCE:
    case CSSM_KEYBLOB_WRAPPED:
    case CSSM_KEYBLOB_WRAPPED_BERDER:
    case CSSM_KEYBLOB_OTHER:
            KeySize_ptr->KeySizeInBits = Key_ptr->KeyData.Length * 8; /*8-bits*/
            KeySize_ptr->EffectiveKeySizeInBits =
                Key_ptr->KeyHeader.EffectiveKeySizeInBits;
            return CSSM_OK;
#endif
/*########################### CSSM_BIS ###########################*/

    case CSSM_KEYBLOB_RAW:
    case CSSM_KEYBLOB_RAW_BERDER:
            break;	/* break for futher caculation */
    default:TAL_SetError(CSSM_CSP_KEY_BLOBTYPE_INCORRECT);
            return CSSM_FAIL;
    }
    /* Caculate the physical KeySizeInBits and logical EffectiveKeySizeInBits */
    {	
        ICLDSSPublicKey     *pDsaPubKey=NULL;

        if (Key_ptr->KeyHeader.KeyClass == CSSM_KEYCLASS_PUBLIC_KEY)
        {
			/* Create a public key structure to hold the decoded key value.
			   The physical key size in bits is the physical key bytes * 8
			   The logical key size is the round-downed modulus bits. */
            pDsaPubKey = Internal_CreateICLDSAPubKeyStruct();
            if ((rtn = Internal_BERDecode_ICLDSAPublicKey(Key_ptr, pDsaPubKey))
                == CSSM_OK)
            {
                KeySize_ptr->KeySizeInBits = Key_ptr->KeyData.Length*8;/*8-bits*/
                KeySize_ptr->EffectiveKeySizeInBits = 
                    PKI_EFFECTIVE_KEY_LEN(pDsaPubKey->PrimeModulus.length,
										  sizeof(pDsaPubKey->PrimeModulus.length));
            }
            Internal_DeleteICLDSAPubKeyStruct(pDsaPubKey);
        }

/*########################### CSSM_BIS ###########################*/
#ifndef CSSM_BIS
        else if (Key_ptr->KeyHeader.KeyClass == CSSM_KEYCLASS_PRIVATE_KEY)
        {
            pDsaPriKey = Internal_CreateICLDSAPriKeyStruct();
            if ((rtn = Internal_BERDecode_ICLDSAPrivateKey(Key_ptr, pDsaPriKey))
                == CSSM_OK)
            {
                KeySize_ptr->KeySizeInBits = Key_ptr->KeyData.Length*8;/*8-bits*/
                KeySize_ptr->EffectiveKeySizeInBits =
                    PKI_EFFECTIVE_KEY_LEN(pDsaPriKey->PrimeModulus.length,
										  sizeof(pDsaPriKey->PrimeModulus.length));
            }
            Internal_DeleteICLDSAPriKeyStructure(pDsaPriKey);
        }
#endif
/*########################### CSSM_BIS ###########################*/

        else
        {
            TAL_SetError(CSSM_CSP_OPERATION_FAILED);
            rtn = CSSM_FAIL;
        }
    }
    return rtn;
}
