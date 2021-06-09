/*-----------------------------------------------------------------------------
 *      File:   cspapi.c
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *-----------------------------------------------------------------------------
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
 * This file contains the functions that are in the CSP portion
 * of the CSSM exported functions.
 */

#include "cssm.h"
#include "cssmport.h"
#include "internal.h"
#include "cssmcspi.h"
#include "context.h"

/*---------------------------------------------------------------
 *
 *Name: CSSM_VerifyData
 *
 *Description:
 *  Exported API for verifying a signature
 *
 *Parameters: 
 *  CCHandle (input) - the context handle
 *  DataBufs (input) - array of data buffers that holds
 *                     information that needs to be sign
 *  DataBufCount (input) - number of data buffers pointed to
 *                         by parameter DataBufs
 *  Signature (input) - signature to verify
 *
 *Returns:
 *  CSSM_FALSE - CSP wasn't able to complete function or
 *               signature did not verify.  Check actual
 *               state by call CSSM_GetLAstError
 *  CSSM_TRUE - the signature matches the data
 *
 *-------------------------------------------------------------*/
CSSM_BOOL CSSMAPI CSSM_VerifyData (CSSM_CC_HANDLE CCHandle,
                                   const CSSM_DATA_PTR DataBufs,
                                   uint32 DataBufCount,
                                   const CSSM_DATA_PTR Signature)
{
    CSSM_SPI_CSP_FUNCS_PTR CallBack = NULL;
    CSSM_CONTEXT_PTR       Context;

    /* CheckInit & ClearError are done in cssm_GetContext */
    /* Get CSP handle and actual context information */
    if ((Context = cssm_GetContext (CCHandle)) == NULL)
        return CSSM_FALSE;

    /*
     * Get the ModuleInfo record and the callback pointers associated with the
     * specified handle and the service mask.
     */
    if (cssm_GetModuleRecord (Context->CSPHandle, CSSM_SERVICE_CSP, 
                              (CSSM_SPI_CSP_FUNCS_PTR *)&CallBack) == NULL)
        return CSSM_FALSE;

    /*
     * Call the callback function. 
     * Make sure that function structure and function pointer is valid 
     */
    if (CallBack->VerifyData)
        return CallBack->VerifyData (Context->CSPHandle, CCHandle, Context, 
                                     DataBufs, DataBufCount, Signature);
    else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_FALSE;
    }
}


/*---------------------------------------------------------------
 *
 *Name: CSSM_DigestData
 *
 *Description:
 *  Exported API for performing a digest on data
 *
 *Parameters: 
 *  CCHandle (input) - the context handle
 *  DataBufs (input) - array of data buffers that holds
 *                     information that needs to be hashed
 *  DataBufCount (input) - number of data buffers pointed to
 *                         by parameter DataBufs
 *  Digest (output) - pointer to hash result
 *
 *Returns:
 *  CSSM_FAIL - CSP wasn't able to complete function
 *  CSSM_OK - Call successful executed by CSP add-in
 *
 *-------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_DigestData (CSSM_CC_HANDLE CCHandle,
                                     const CSSM_DATA_PTR DataBufs,
                                     uint32 DataBufCount,
                                     CSSM_DATA_PTR Digest)
{
    CSSM_SPI_CSP_FUNCS_PTR CallBack = NULL;
    CSSM_CONTEXT_PTR       Context;

    /* CheckInit & ClearError are done in cssm_GetContext */
    /* Get CSP handle and actual context information */
    if ((Context = cssm_GetContext (CCHandle)) == NULL)
        return CSSM_FAIL;

    /*
     * Get the ModuleInfo record and the callback pointers associated with the
     * specified handle and the service mask.
     */
    if (cssm_GetModuleRecord (Context->CSPHandle, CSSM_SERVICE_CSP, 
                              (CSSM_SPI_CSP_FUNCS_PTR *)&CallBack) == NULL) 
        return CSSM_FAIL;

    /*
     * Call the callback function. 
     * Make sure that function structure and function pointer is valid 
     */
    if (CallBack->DigestData)
        return CallBack->DigestData (Context->CSPHandle, CCHandle, Context, 
                                     DataBufs, DataBufCount, Digest);
    else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_FAIL;
    }
}

/*---------------------------------------------------------------
 *
 *Name: CSSM_QueryKeySizeInBits
 *
 *Description:
 *  Exported API for querying key size
 *
 *Parameters: 
 *  CSPHandle (input) - the CSP handle
 *  CCHandle (input) - context handle containing the key
 *  Key (input/optional) - key for which size is to be determined
 *  KeySize (output) -  pointer to keysize
 *
 *Returns:
 *  CSSM_FAIL - CSP wasn't able to complete function
 *  CSSM_OK - Call successful executed by CSP add-in
 *
 *-------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_QueryKeySizeInBits (CSSM_CSP_HANDLE CSPHandle,
                                             CSSM_CC_HANDLE CCHandle,
                                             const CSSM_KEY_PTR Key,
                                             CSSM_KEY_SIZE_PTR KeySize)
{
    CSSM_SPI_CSP_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /*
     * Get the callback pointers associated with the
     * specified handle and the service mask.
     */
    if (cssm_GetModuleRecord (CSPHandle, CSSM_SERVICE_CSP, 
                              (CSSM_SPI_CSP_FUNCS_PTR *)&CallBack) == NULL) {
        RETURN (CSSM_FAIL)
    }

    /*
     * Call the callback function. 
     */
    if (CallBack->QueryKeySizeInBits)
        return CallBack->QueryKeySizeInBits (CSPHandle, CCHandle, Key, KeySize);
    else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_FAIL;
    }
}
