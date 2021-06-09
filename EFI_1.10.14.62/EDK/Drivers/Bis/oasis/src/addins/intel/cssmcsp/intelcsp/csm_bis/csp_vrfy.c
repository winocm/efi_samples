/*-----------------------------------------------------------------------
 *      File:   csp_vrfy.c
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
 * This file contains the CSP for WfM BIS crypto functions for signature verification.
 */

#include "tal_inc.h"
#include "csp_inc.h"
#include "oidsalg.h"
#include "ber_der.h"
#include "csp_icl_dsa.h"

typedef enum dsa_sig_param_order {
    DSA_SIG_PARAM_R = 0,
    DSA_SIG_PARAM_S = 1,
    DSA_SIG_PARAM_COUNT = 2
} DSA_SIG_PARAM_ORDER;

#define SUPPORTED_DSA_PUBKEY_EFFECTIVE_KEYSIZE    1024

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                      Static functions declaration                        */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
static CSSM_BOOL s_Csp_VerifyHash_DSA(CSSM_CSP_HANDLE CSPHandle,
                                      const CSSM_KEY_PTR PubKey_ptr, 
                                      const uint8* Digest_ptr,
                                      uint32 DigestLen,
                                      const CSSM_DATA_PTR Signature_ptr);

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                      Static function definition                          */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*****************************
 *           CSP_DSA         *
 *****************************/
/*-----------------------------------------------------------------------------
 *Name: s_Csp_VerifyHash_DSA
 *
 *Description:
 * This function does DSA verification for the digest/hash of the input
 * message.
 *
 *Parameters: 
 * CSPHandle(input)- CSSM managed CSP handle is used for up calls to memory 
 *                   functions.
 * PubKey_ptr(input)-The public key to do the signature verification.
 * DataBufs_ptr(input)- A pointer to one or more CSSM_DATA structures
 *                      containing the supplied message data. 
 * DataBufCount(input)- The number of DataBufs_ptr. 
 * Signature(output)  - A pointer to the CSSM_DATA structure for the signature.
 *
 *Returns:
 * CSSM_TRUE  - Signature Verification succeeded
 * CSSM_FALSE - Signature Verification failed. If the signature is wrong the 
 *              error code is set to 0, otherwise, the error code represents
 *              the error condition.
 *
 *Error Codes:
 * error codes set by this function
 *   CSSM_CSP_KEY_ALGID_MISMATCH
 *   CSSM_CSP_KEY_FORMAT_INCORRECT
 *   CSSM_CSP_OPERATION_FAILED
 *   CSSM_CSP_INVALID_KEY_SIZE_IN_BITS
 *   CSSM_CSP_KEY_KEYHEADER_INCONSISTENT
 *
 * error codes set by sub-functions
 *   CSSM_CSP_INVALID_POINTER
 *   CSSM_CSP_INVALID_DATA_POINTER
 *   CSSM_CSP_ERR_OUTBUF_LENGTH
 *   CSSM_CSP_INVALID_KEY
 *   CSSM_CSP_KEY_FORMAT_INCORRECT
 *   CSSM_CSP_INVALID_KEYCLASS
 *   CSSM_CSP_NOT_ENOUGH_BUFFER
 *   CSSM_CSP_MEMORY_ERROR
 *---------------------------------------------------------------------------*/
static CSSM_BOOL s_Csp_VerifyHash_DSA(CSSM_CSP_HANDLE CSPHandle,
                                      const CSSM_KEY_PTR PubKey_ptr, 
                                      const uint8* Digest_ptr,
                                      uint32 DigestLen,
                                      const CSSM_DATA_PTR Signature_ptr)
{
    /* These buffers are internally used by ICL use MODULUSBYTES instead */
    /* of sizeof(ICLSHADigest)+1                                         */
    uint8           sigBuf_r[MODULUSBYTES],
                    sigBuf_s[MODULUSBYTES];

    ICLDSSPublicKey	*pDssPubKey;
    ICLData         message, signature[DSA_SIG_PARAM_COUNT];

    /* Touch the variable to surpass the level 4 warning for unused variables */
    CSPHandle;

    /* Check the algorithm in ket header */
    if (PubKey_ptr->KeyHeader.AlgorithmId != CSSM_ALGID_DSA)
    {
        TAL_SetError(CSSM_CSP_KEY_ALGID_MISMATCH);
        return CSSM_FALSE;
    }
    /* Check the key format */
    if (PubKey_ptr->KeyHeader.Format != CSSM_KEYBLOB_RAW_FORMAT_FIPS186)
    {
        TAL_SetError(CSSM_CSP_KEY_FORMAT_INCORRECT);
        return CSSM_FALSE;
    }
/* Step 1: BER Decode the public key and check the EffectiveKeySizeInBits */
    pDssPubKey = Internal_CreateICLDSAPubKeyStruct();
    if (Internal_BERDecode_ICLDSAPublicKey(PubKey_ptr, pDssPubKey) != CSSM_OK)
    {
        Internal_DeleteICLDSAPubKeyStruct(pDssPubKey);
        TAL_SetError(CSSM_CSP_OPERATION_FAILED);
        return CSSM_FALSE;
    }
    if ((sint32)PubKey_ptr->KeyHeader.EffectiveKeySizeInBits != 
		SUPPORTED_DSA_PUBKEY_EFFECTIVE_KEYSIZE)
    {
        Internal_DeleteICLDSAPubKeyStruct(pDssPubKey);
        TAL_SetError(CSSM_CSP_INVALID_KEY_SIZE_IN_BITS);
        return CSSM_FALSE;
    }
    if (pDssPubKey->PrimeModulus.length*8 /*8-bit*/ 
        != (sint32)PubKey_ptr->KeyHeader.EffectiveKeySizeInBits)
    {
        Internal_DeleteICLDSAPubKeyStruct(pDssPubKey);
        TAL_SetError(CSSM_CSP_KEY_KEYHEADER_INCONSISTENT);
        return CSSM_FALSE;
    }
/*Step 2: Decode the signature */
    message.value  = (uint8*)Digest_ptr;
    message.length = DigestLen;
    signature[DSA_SIG_PARAM_R].value = sigBuf_r;
    signature[DSA_SIG_PARAM_R].length = sizeof(sigBuf_r);
    signature[DSA_SIG_PARAM_S].value = sigBuf_s;
    signature[DSA_SIG_PARAM_S].length = sizeof(sigBuf_s);
    if (Internal_BERDecode_DSASignature(Signature_ptr->Data,
		                                Signature_ptr->Length,
										signature, DSA_SIG_PARAM_COUNT)
        != DSA_SIG_PARAM_COUNT)
    {
        Internal_DeleteICLDSAPubKeyStruct(pDssPubKey);
        TAL_SetError(CSSM_CSP_OPERATION_FAILED);
        return CSSM_FALSE;
    }
/*Step 3: Verify - Set the error code to 0 and return CSSM_FALSE for */
/*        any bad signature */
    if (ICL_DSSVerifyDigest(&message,&signature[0],&signature[1],pDssPubKey)
        !=0)
    {
        Internal_DeleteICLDSAPubKeyStruct(pDssPubKey);
        TAL_SetError(0);
        return CSSM_FALSE;
    }

    Internal_DeleteICLDSAPubKeyStruct(pDssPubKey);
    return CSSM_TRUE;
}

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                         Sub functions definition                         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: Csp_VerifyHash_DSA
 *
 *Description:
 * This function computes a SHA1 message digest from the supplied input message
 * data and calls s_Csp_VerifyHash_DSA to do the DSA verify hash operation.
 *
 *Parameters: 
 * CSPHandle(input)- CSSM managed CSP handle is used for up calls to memory 
 *                   functions
 * PubKey_ptr(input)-The public key to do the signature verification.
 * AlgID (input) - Requested verification algorithmn ID
 * DataBufs_ptr(input) - A pointer to one or more CSSM_DATA structures
 *                       containing the supplied data. 
 * DataBufCount(input) - The number of DataBufs_ptr. 
 * Signature_ptr(output) - A pointer to the CSSM_DATA structure for the 
 *                         signature.
 *
 *Returns:
 * CSSM_TRUE  - Signature Verification succeeded
 * CSSM_FALSE - Signature Verification failed. If the signature is wrong the 
 *              error code is set to 0, otherwise, the error code represents
 *              the error condition.
 *
 *Error Codes:
 * error codes set by this function
 *   CSSM_CSP_INVALID_ALGORITHM
 *
 * error codes set by sub-functions
 *   Please reference function s_Csp_VerifyHash_DSA and Csp_DigestData_SHA1.
 *---------------------------------------------------------------------------*/
CSSM_BOOL Csp_VerifyHash_DSA(CSSM_CSP_HANDLE CSPHandle,
							 uint32 AlgID,
                             const CSSM_KEY_PTR PubKey_ptr, 
                             const CSSM_DATA_PTR DataBufs_ptr,
                             uint32 DataBufCount,
                             const CSSM_DATA_PTR Signature_ptr)
{
    uint8           DigestBuf[MAX_BUFFER_LEN];
    CSSM_DATA       DigestData;	

    /*Initialize the digest buffer */
    DigestData.Length = sizeof(DigestBuf);
    DigestData.Data = DigestBuf;

    /*Verify the only supported signature algorithm SHA1WithDSA */
    if (AlgID != CSSM_ALGID_SHA1WithDSA)
    {
        TAL_SetError(CSSM_CSP_INVALID_ALGORITHM);
        return CSSM_FALSE;
    }
    /*Caculate the Digest of the input message using SHA1 algorithm */
    if (Csp_DigestData_SHA1(CSPHandle, DataBufs_ptr, DataBufCount, &DigestData)
                            != CSSM_OK)
	{
        return CSSM_FALSE;
	}

    /* Verify the Digest using DSA algorithm */
    return s_Csp_VerifyHash_DSA(CSPHandle, PubKey_ptr, 
                                DigestData.Data, DigestData.Length,
                                Signature_ptr);
}

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          Crypto functions                                */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: TALCSP_VerifyData
 *
 *Description:
 * This function verifies the input data against the provided signature.
 *
 *Parameters: 
 * CSPHandle(input) - CSSM managed CSP handle is used for up calls to memory
 *                    functions
 * CCHandle (input) - CSSM managed crypto operation handle.
 * Context_ptr (input) - The context contains all the attributes associated for
 *                       the crypto operation.
 * DataBufs_ptr(input) - A pointer to one or more CSSM_DATA structures
 *                       containing the data be verified. 
 * DataBufCount(input) - The number of DataBufs_ptr to be verified.
 * Signature_ptr(output) - A pointer to a CSSM_DATA structure which contains the
 *                         signature and the size of the signature.
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
 *   CSSM_CSP_INVALID_ATTR_KEY
 *   CSSM_CSP_KEY_BLOBTYPE_INCORRECT
 *
 * error codes set by sub-functions
 *   Please reference function Csp_VerifyData.
 *---------------------------------------------------------------------------*/
CSSM_BOOL TALCSP_VerifyData(CSSM_CSP_HANDLE CSPHandle,
                            CSSM_CC_HANDLE CCHandle,
                            const CSSM_CONTEXT_PTR Context_ptr,
                            const CSSM_DATA_PTR DataBufs_ptr,
                            uint32 DataBufCount,
                            const CSSM_DATA_PTR Signature_ptr)
{
    CSSM_KEY_PTR    PubKey_ptr;

    /* Touch the variable to surpass the level 4 warning for unused variables */
    CCHandle;

    /* Check the length of the signature */
    if ((Context_ptr->AlgorithmType != CSSM_ALGID_NONE) && 
        (Signature_ptr->Length == 0))
    {
        TAL_SetError(CSSM_CSP_INVALID_SIGNATURE);
        return CSSM_FALSE;
    }
    /* Get the public key from context attributes. */
    /* Key, KeyClass, KeyUsage, and private key BlobType verications are done */
    /* in the TAL level KeyFormat checking is in s_Csp_VerifyHash_DSA */
    if ((PubKey_ptr = TAL_GetKeyFromContext(Context_ptr,
                                            CSSM_KEYCLASS_PUBLIC_KEY)) == NULL)
    {
        TAL_SetError(CSSM_CSP_INVALID_ATTR_KEY);
        return CSSM_FALSE;
    }
    /* Check the public BlobType */
    if (PubKey_ptr->KeyHeader.BlobType != CSSM_KEYBLOB_RAW_BERDER)
    {
        TAL_SetError(CSSM_CSP_KEY_BLOBTYPE_INCORRECT);
        return CSSM_FALSE;
    }
    return Csp_VerifyHash_DSA(CSPHandle, Context_ptr->AlgorithmType,
                              PubKey_ptr,
                              DataBufs_ptr, DataBufCount,
                              Signature_ptr);
}

