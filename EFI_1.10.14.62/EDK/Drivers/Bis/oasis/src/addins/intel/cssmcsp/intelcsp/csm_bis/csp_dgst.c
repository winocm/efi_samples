/*-----------------------------------------------------------------------
 *      File:   csp_dgst.c
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
 * This file contains the CSP for WfM BIS digest crypto functions.
 */

#include "tal_inc.h"
#include "icl.h"

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                            Module variables                              */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
ICLSHAState     m_Staged_SHA_State;
CSSM_CC_HANDLE	m_Staged_SHA_CCHandle;
CSSM_CSP_HANDLE m_Staged_SHA_CSPHandle;

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          Csp function definition                         */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*****************************
 *          CSP_SHA1         *
 *****************************/
/*-----------------------------------------------------------------------------
 *Name: Csp_DigestData_SHA1
 *
 *Description:
 * This function computes a message digest for SHA1 algorithm from the supplied
 * input data.
 *
 *Parameters: 
 * CSPHandle(input)- CSSM managed CSP handle is used for up calls to memory 
 *                   functions
 * DataBufs_ptr(input) - A pointer to one or more CSSM_DATA structures
 *                       containing the supplied data. 
 * DataBufCount(input) - The number of DataBufs_ptr. 
 * Digest_ptr(output)  - A pointer to the CSSM_DATA structure for the message
 *                       digest. This function will uses CSPHandle to allocate 
 *                       a digest buffer if the applicatio does not supply one. 
 *                       Then, application has to free this buffer.
 *
 *Returns:
 * CSSM_OK -   Generates a digest
 * CSSM_FAIL - Digest generation failed
 *
 *Error Codes:
 * error codes set by this function
 *   CSSM_CSP_OPERATION_FAILED
 *
 * error codes set by sub-function TAL_MakeOutBuffer
 *   CSSM_CSP_INVALID_DATA_POINTER
 *   CSSM_CSP_MEMORY_ERROR
 *   CSSM_CSP_ERR_OUTBUF_LENGTH
 *---------------------------------------------------------------------------*/
CSSM_RETURN Csp_DigestData_SHA1(CSSM_CSP_HANDLE CSPHandle,
                                const CSSM_DATA_PTR DataBufs_ptr,
                                uint32 DataBufCount,
                                CSSM_DATA_PTR Digest_ptr)
{
    ICLData         message;
    uint32          i;
    ICLSHAState     shaState;
    ICLSHADigest    shaDigest;

    /* Make sure the output buffer is there and has enough space */
    if (TAL_MakeOutBuffer(CSPHandle, Digest_ptr, sizeof(ICLSHADigest))
        != CSSM_OK)
        return CSSM_FAIL;

    /* SHA1 operation */
    if (ICL_SHABegin(&shaState) !=0)
    {
        TAL_SetError(CSSM_CSP_OPERATION_FAILED);
        return CSSM_FAIL;
    }
	/* Process vector of inputs */
    for (i = 0; i < DataBufCount; i++)
    {
        message.value  = DataBufs_ptr[i].Data;
        message.length = DataBufs_ptr[i].Length;
        if (ICL_SHAProcess(&message, &shaState) !=0)
        {
            TAL_SetError(CSSM_CSP_OPERATION_FAILED);
            return CSSM_FAIL;
        }
    }	
    if (ICL_SHAEnd(&shaState, shaDigest) !=0)
    {
        TAL_SetError(CSSM_CSP_OPERATION_FAILED);
        return CSSM_FAIL;
    }
    /* Copy result to output buffer */
    Digest_ptr->Length = sizeof(shaDigest);
    cssm_memcpy(Digest_ptr->Data, (uint8*)shaDigest, Digest_ptr->Length);
    return CSSM_OK;
}

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          Crypto functions                                */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: TALCSP_DigestData
 *
 *Description:
 * This function does the capability check and redirect to digest function
 * for the requested digest algorithm.
 *
 *Parameters: 
 * CSPHandle(input) - CSSM managed CSP handle is used for up calls to memory
 *                    functions
 * CCHandle (input) - CSSM managed crypto operation handle.
 * Context_ptr (input)  - The context contains all the attributes associated
 *                        for the crypto operation.
 * DataBufs_ptr(input) - A pointer to one or more CSSM_DATA structures 
 *                       containing the supplied data. 
 * DataBufCount(input) - The number of DataBufs_ptr. 
 * Digest_ptr(output)  - A pointer to the CSSM_DATA structure for the message
 *                       digest.
 *
 *Returns:
 * CSSM_OK -   Generates a digest
 * CSSM_FAIL - Digest generation failed
 *---------------------------------------------------------------------------*/
CSSM_RETURN TALCSP_DigestData(CSSM_CSP_HANDLE CSPHandle,
                              CSSM_CC_HANDLE CCHandle,
                              const CSSM_CONTEXT_PTR Context_ptr,
                              const CSSM_DATA_PTR DataBufs_ptr,
                              uint32 DataBufCount,
                              CSSM_DATA_PTR Digest_ptr)
{
    /* Touch the variable to surpass the level 4 warning for unused variables */
    CCHandle;

    /* The CSP for WfM BIS only supports SHA1 algorithm */
    if (Context_ptr->AlgorithmType != CSSM_ALGID_SHA1)
    {
        TAL_SetError(CSSM_CSP_INVALID_ALGORITHM);
        return CSSM_FAIL;
    }
    /* Call CSP SHA1 function to compute the digest */
    return Csp_DigestData_SHA1(CSPHandle, DataBufs_ptr,
                               DataBufCount, Digest_ptr);
}

