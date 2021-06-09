/*-----------------------------------------------------------------------
 *      File:   tal_cspi.c
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
 * This file is part of the Token Adaptation Layer (TAL) source code base.
 * The TAL code makes it easier for CSP venders to develop CSPs that plug
 * into the Intel CDSA infrastructure.
 * This file contains the supported WfM BIS CSP spi functions
 */

#include "tal_cspi.h"
#include "tal_glob.h"
#include "tal_csp.h"
#include "tal_cntx.h"
#include "tal_defs.h"
#include "tal_util.h"

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          Crypto functions                                */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/

/*-----------------------------------------------------------------------------
 *Name: CSP_VerifyData
 *
 *Description:
 *	Validates the context, input and output parameters, and makes a 
 *  CSP_VerifyData call.
 *
 *Parameters:
 *  CSPHandle(input) - CSSM managed CSP handle is used for up calls to memory
 *                     functions
 *  CCHandle (input) - CSSM managed crypto operation handle.
 *  Context_ptr (input) - The context contains all the attributes associated
 *                        for the crypto operation.
 *  DataBufs_ptr (input) - vector of input data buffers
 *  DataBufCount (input) - Number of input data buffer
 *  Signature_ptr (input)- Signature to verify
 *
 *Returns:
 * CSSM_TRUE  - Signature Verification succeeded
 * CSSM_FALSE - Signature Verification failed. If the signature is wrong the 
 *              error code is set to 0, otherwise, the error code represents
 *              the error condition.
 *
 *Error Codes:
 * error codes set by this function
 *   CSSM_CSP_INVALID_SIGNATURE
 *
 * error codes set by sub-functions
 *   CSSM_CSP_INVALID_CONTEXT_POINTER
 *   CSSM_CSP_INVALID_ATTR_KEY
 *   CSSM_CSP_INVALID_CONTEXT
 *   CSSM_CSP_INVALID_KEYCLASS
 *   CSSM_CSP_KEY_USAGE_INCORRECT
 *   CSSM_CSP_INVALID_DATA_COUNT
 *   CSSM_CSP_INVALID_DATA_POINTER
 *   CSSM_CSP_INVALID_DATA
 *   And error codes set by TALCSP_VerifyData.
 *---------------------------------------------------------------------------*/
CSSM_BOOL CSSMSPI CSP_VerifyData(CSSM_CSP_HANDLE CSPHandle,
                                 CSSM_CC_HANDLE CCHandle,
                                 const CSSM_CONTEXT_PTR Context_ptr,
                                 const CSSM_DATA_PTR DataBufs_ptr,
                                 uint32 DataBufCount,
                                 const CSSM_DATA_PTR Signature_ptr)
{
    /* Check the context type, key, key usage, and key class */
    if (TAL_CheckContext(Context_ptr, CSP_CRYPTO_VERIFY) != CSSM_OK)
	{
        return CSSM_FALSE;
	}
    /* Verify the input and output buffers */
    if (TAL_ValidateInDataParam(DataBufs_ptr, DataBufCount) != CSSM_OK)
	{
        return CSSM_FALSE;
	}
    if (TAL_ValidateInDataParam(Signature_ptr, 1) != CSSM_OK)
    {
        TAL_SetError(CSSM_CSP_INVALID_SIGNATURE);
        return CSSM_FALSE;
    }
    /* Call crypto func to sign data, returns the signature */
    return TALCSP_VerifyData(CSPHandle, CCHandle, Context_ptr,
                             DataBufs_ptr, DataBufCount, Signature_ptr);
}

/*-----------------------------------------------------------------------------
 *Name: CSP_DigestData
 *
 *Description:
 *	Validates the context, input and output parameters, and calls the CSP 
 *  specific TALCSP_DigestData function.
 *
 *Parameters:
 *  CSPHandle(input) - CSSM managed CSP handle is used for up calls to memory
 *                     functions
 *  CCHandle (input) - CSSM managed crypto operation handle.
 *  Context_ptr (input)  - The context contains all the attributes associated
 *                         for the crypto operation.
 *  DataBufs_ptr (input) - vector of input data buffers
 *  DataBufCount (input) - number of data buffer
 *  Digest_ptr (output)  - buffer to hold the digest
 *
 *Returns:
 *    CSSM_OK:	 call is successful.
 *    CSSM_FAIL: call failed.
 *
 *Error Codes:
 * error codes set by this function
 *	 none
 *
 * error codes set by sub-functions
 *   CSSM_CSP_INVALID_CONTEXT_POINTER
 *   CSSM_CSP_INVALID_ATTR_KEY
 *   CSSM_CSP_INVALID_CONTEXT
 *   CSSM_CSP_INVALID_KEYCLASS
 *   CSSM_CSP_KEY_USAGE_INCORRECT
 *   CSSM_CSP_INVALID_DATA_COUNT
 *   CSSM_CSP_INVALID_DATA_POINTER
 *   CSSM_CSP_INVALID_DATA
 *   And error codes set by TALCSP_DigestData.
*---------------------------------------------------------------------------*/
CSSM_RETURN CSSMSPI CSP_DigestData(CSSM_CSP_HANDLE CSPHandle,
                                   CSSM_CC_HANDLE CCHandle,
                                   const CSSM_CONTEXT_PTR Context_ptr,
                                   const CSSM_DATA_PTR DataBufs_ptr,
                                   uint32 DataBufCount,
                                   CSSM_DATA_PTR Digest_ptr)
{
    /* Check the context type */
    if (TAL_CheckContext(Context_ptr, CSP_CRYPTO_DIGEST) != CSSM_OK)
        return CSSM_FAIL;

    /* Verify the input and output buffers */
    if (TAL_ValidateInDataParam(DataBufs_ptr, DataBufCount) != CSSM_OK)
	{
        return CSSM_FAIL;
	}
    if (TAL_ValidateOutDataParam(Digest_ptr, 1) != CSSM_OK)
	{
        return CSSM_FAIL;
	}

    /* Call crypto func to caculate the digest of the input message */
    return TALCSP_DigestData(CSPHandle, CCHandle, Context_ptr, 
                             DataBufs_ptr, DataBufCount, Digest_ptr);
}

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          Hidden functions                                */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/

/*-----------------------------------------------------------------------------
 *Name: CSP_QueryKeySizeInBits
 *
 *Description:
 *	Validates both input and output parameters, finds key from the supplied
 *  context and makes a CSP TALCSP_QueryKeySizeInBits call.
 *
 *Parameters:
 *  CSPHandle(input) - CSSM managed CSP handle is used for up calls to memory
 *                     functions
 *  CCHandle (input) - CSSM managed crypto operation handle.
 *  Key_ptr (input)  - Pointer to the CSSM_KEY data structure, for which the
 *                     size is requested.
 *  KeySize_ptr (output) - buffer to hold key size
 *
 *Returns:
 *    CSSM_OK:	 call is successful.
 *    CSSM_FAIL: call failed.
 *
 *Error Codes:
 * error codes set by this function
 *   CSSM_CSP_INVALID_POINTER
 *
 * error codes set by sub-functions
 *   CSSM_CSP_INVALID_KEY_POINTER
 *   CSSM_CSP_INVALID_KEY
 *	 And error codes set by TALCSP_QueryKeySizeInBits.
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMSPI CSP_QueryKeySizeInBits (CSSM_CSP_HANDLE CSPHandle,
                                            CSSM_CC_HANDLE CCHandle,
                                            const CSSM_KEY_PTR Key_ptr,
                                            CSSM_KEY_SIZE_PTR KeySize_ptr)
{
    /* Touch the variable to surpass the level 4 warning for unused variables */
    CSPHandle;
    CCHandle;

    /* Verify the key structure */
    if (TAL_ValidateInKeyParam(Key_ptr) != CSSM_OK)
	{
        return CSSM_FAIL;
	}
    if (KeySize_ptr == NULL)
    {
        TAL_SetError(CSSM_CSP_INVALID_POINTER);
        return CSSM_FAIL;
    }
    /* Query Key Size */
    return TALCSP_QueryKeySizeInBits(Key_ptr, KeySize_ptr);
}

