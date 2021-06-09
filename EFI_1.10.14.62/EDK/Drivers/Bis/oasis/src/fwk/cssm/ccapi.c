/*-----------------------------------------------------------------------------
 *      File:   ccapi.c
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
 * This file contains the context functions exported by CSSM
 */
/*
 * The CSSM_BIS implementation of this code was inspected 8/19/98.
 */

#include "internal.h"
#include "context.h"


/*---------------------------------------------------------------
 *Name: CSSM_CSP_CreateSignatureContext
 *
 *Description:
 *   Function for creating a signature/verification context
 *
 *Parameters: 
 *   CSPHandle   - handle to CSP providing the service
 *   AlgorithmID - identifier describing the crypto algorithm
 *   PassPhrase  - pointer to structure containing the passphrase
 *   Key         - pointer to structure containing the public key
 *
 *Returns:
 *   0 - unable to create context
 *   non 0 - handle for the created context
 *
 *----------------------------------------------------------------*/
CSSM_CC_HANDLE CSSMAPI CSSM_CSP_CreateSignatureContext (
                                          CSSM_CSP_HANDLE CSPHandle,
                                          uint32 AlgorithmID,
                                          const CSSM_CRYPTO_DATA_PTR PassPhrase,
                                          const CSSM_KEY_PTR Key)
{
    CSSM_CC_HANDLE   CCHandle;
    CSSM_CONTEXT_PTR Context;
    uint32           NumberAttributes = 0;

    /* CheckInit and Clear Error are done in cssm_CreateContext */

    /* Determine the number of attributes */
    if (!cssm_IsBadCryptoDataPtr (PassPhrase))
        NumberAttributes++;

    if (Key && Key->KeyData.Data && Key->KeyData.Length)
        NumberAttributes++;

    /* Create and initialize a context structure */ 
    Context = cssm_CreateContext (CSPHandle, CSSM_ALGCLASS_SIGNATURE, 
                                  AlgorithmID, NumberAttributes);
    if (!Context)
        return CSSM_INVALID_HANDLE;

    /* Fill in the attributes */
    if (!cssm_IsBadCryptoDataPtr (PassPhrase))
        if (cssm_AddAttribute(&Context->ContextAttributes[--NumberAttributes], 
                              CSSM_ATTRIBUTE_PASSPHRASE, PassPhrase) == CSSM_FAIL)
            goto CREATE_SIG_CTX_FAIL;

    if (Key && Key->KeyData.Data && Key->KeyData.Length) 
        if (cssm_AddAttribute (&Context->ContextAttributes[--NumberAttributes], 
                               CSSM_ATTRIBUTE_KEY, Key) == CSSM_FAIL) 
            goto CREATE_SIG_CTX_FAIL;

    /* Insert this context into the list of contexts */
    if ((CCHandle = cssm_InsertContext (Context)) != CSSM_INVALID_HANDLE)
        return CCHandle;
    else
        goto CREATE_SIG_CTX_FAIL;

CREATE_SIG_CTX_FAIL:
    cssm_FreeContext (Context);
    return CSSM_INVALID_HANDLE;
}


/*---------------------------------------------------------------
 *Name: CSSM_CSP_CreateDigestContext
 *
 *Description:
 *   Function for creating a digest context
 *
 *Parameters: 
 *   CSPHandle   - handle to CSP providing the service
 *   AlgorithmID - identifier describing the digest algorithm
 *
 *Returns:
 *   0 - unable to create context
 *   non 0 - handle for the created context
 *
 *----------------------------------------------------------------*/
CSSM_CC_HANDLE CSSMAPI CSSM_CSP_CreateDigestContext (CSSM_CSP_HANDLE CSPHandle,
                                                     uint32 AlgorithmID)
{
    CSSM_CC_HANDLE   CCHandle;
    CSSM_CONTEXT_PTR Context;

    /* CheckInit and Clear Error are done in cssm_CreateContext */

    /* Create and initialize a context structure */ 
    Context = cssm_CreateContext (CSPHandle, CSSM_ALGCLASS_DIGEST, 
                                  AlgorithmID, 0);
    if (!Context)
        return CSSM_INVALID_HANDLE;

    /* Insert this context into the list of contexts */
    if ((CCHandle = cssm_InsertContext (Context)) != CSSM_INVALID_HANDLE)
        return CCHandle;
    else {
        cssm_FreeContext (Context);
        return CSSM_INVALID_HANDLE;
    }
}


/*---------------------------------------------------------------
 *Name: CSSM_DeleteContext
 *
 *Description:
 *   Function for deleting the context
 *
 *Parameters: 
 *   CCHandle - handle to a context
 *
 *Returns:
 *   CSSM_FAIL - unable to delete the specified context
 *   CSSM_OK - context is deleted
 *
 *----------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_DeleteContext (CSSM_CC_HANDLE CCHandle)
{
    extern cssm_CONTEXT_NODE_PTR ContextHead;
    cssm_CONTEXT_NODE_PTR PreviousContext = NULL, TempContext;

    /* Make sure that app has done a CSSM_Init */
    if (cssm_CheckInit () == CSSM_FAIL)
        return CSSM_FAIL;

    /* Clear the error */
    CSSM_ClearError ();

    /* Find the context */
    TempContext = ContextHead;
    while (TempContext) {

        if (TempContext->ContextHandle != CCHandle) {

            /* This is not the specified context */
            /* Proceed to the next context node */
            PreviousContext = TempContext;
            TempContext = TempContext->Next;

        } else {
            /* This is the context to delete */

            /* Fix the context list */
            if (TempContext == ContextHead)
                ContextHead = ContextHead->Next;
            else 
                PreviousContext->Next = TempContext->Next;

#ifndef CSSM_BIS
            /* Callback to csp to notify of context deletion */
            {
            cssm_ATTACHED_MODULE_NODE_PTR ModuleInfo;
            ModuleInfo = cssm_GetModuleRecord (TempContext->Context->CSPHandle, 
                                               0, NULL);
            if (ModuleInfo)
                if (ModuleInfo->AddInJT && ModuleInfo->AddInJT->EventNotify)
                    ModuleInfo->AddInJT->EventNotify 
                      (TempContext->Context->CSPHandle, 
                       CSSM_EVENT_DELETE_CONTEXT, TempContext->ContextHandle);
            }
#endif

            /* Free the context and the context node */
            cssm_FreeContext (TempContext->Context);
            cssm_free (TempContext, 0);

            return CSSM_OK;
        } 
    }
   
    /* This handle does not refer to any available contexts */
    CSSM_SetError (&CssmGUID, CSSM_INVALID_CONTEXT_HANDLE);
    return CSSM_FAIL;
}
