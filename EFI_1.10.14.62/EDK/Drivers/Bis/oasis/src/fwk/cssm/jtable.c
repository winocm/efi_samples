/*-----------------------------------------------------------------------------
 *      File:   jtable.c
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
 * This file contains the jump table functions required for OS Absent testing
 */

#include "cssm.h"
#include "cssmtype.h"
#include "cssm_jtable.h"


VOID_FUNC_PTR cssm_JumpTable(uint32 FunctionNumber);



VOID_FUNC_PTR cssm_JumpTable(uint32 FunctionNumber)
{
    switch (FunctionNumber)
    {
    case JTENTRY_CSSM_Init:
        return (VOID_FUNC_PTR) CSSM_Init;
    case JTENTRY_CSSM_ModuleAttach:
        return (VOID_FUNC_PTR) CSSM_ModuleAttach;
    case JTENTRY_CSSM_ModuleDetach:
        return (VOID_FUNC_PTR) CSSM_ModuleDetach;
    case JTENTRY_CSSM_GetError:
        return (VOID_FUNC_PTR) CSSM_GetError;
    case JTENTRY_CSSM_SetError:
        return (VOID_FUNC_PTR) CSSM_SetError;
    case JTENTRY_CSSM_ClearError:
        return (VOID_FUNC_PTR) CSSM_ClearError;
    case JTENTRY_CSSM_CSP_CreateSignatureContext:
        return (VOID_FUNC_PTR) CSSM_CSP_CreateSignatureContext;
    case JTENTRY_CSSM_CSP_CreateDigestContext:
        return (VOID_FUNC_PTR) CSSM_CSP_CreateDigestContext;
    case JTENTRY_CSSM_DeleteContext:
        return (VOID_FUNC_PTR) CSSM_DeleteContext;
    case JTENTRY_CSSM_DigestData:
        return (VOID_FUNC_PTR) CSSM_DigestData;
    case JTENTRY_CSSM_VerifyData:
        return (VOID_FUNC_PTR) CSSM_VerifyData;
    case JTENTRY_CSSM_QueryKeySizeInBits:
        return (VOID_FUNC_PTR) CSSM_QueryKeySizeInBits;
    case JTENTRY_CSSM_CL_CertGetFirstFieldValue:
        return (VOID_FUNC_PTR) CSSM_CL_CertGetFirstFieldValue;
    case JTENTRY_CSSM_CL_CertAbortQuery:
        return (VOID_FUNC_PTR) CSSM_CL_CertAbortQuery;
    case JTENTRY_CSSM_CL_CertGetKeyInfo:        
        return (VOID_FUNC_PTR) CSSM_CL_CertGetKeyInfo;
    case JTENTRY_CSSM_VL_InstantiateVoFromLocation:
        return (VOID_FUNC_PTR) CSSM_VL_InstantiateVoFromLocation;
    case JTENTRY_CSSM_VL_FreeVo:
        return (VOID_FUNC_PTR) CSSM_VL_FreeVo;
    case JTENTRY_CSSM_VL_GetDoInfoByName:
        return (VOID_FUNC_PTR) CSSM_VL_GetDoInfoByName;
    case JTENTRY_CSSM_VL_FreeDoInfos:       
        return (VOID_FUNC_PTR) CSSM_VL_FreeDoInfos;
    case JTENTRY_CSSM_VL_GetFirstSignatureInfo:
        return (VOID_FUNC_PTR) CSSM_VL_GetFirstSignatureInfo;
    case JTENTRY_CSSM_VL_AbortScan: 
        return (VOID_FUNC_PTR) CSSM_VL_AbortScan;
    case JTENTRY_CSSM_VL_FreeSignatureInfo:
        return (VOID_FUNC_PTR) CSSM_VL_FreeSignatureInfo;
    case JTENTRY_CSSM_VL_SetDoLMapEntries:     
        return (VOID_FUNC_PTR) CSSM_VL_SetDoLMapEntries;
    case JTENTRY_CSSM_VL_VerifyRootCredentialsDataAndContainment:
        return (VOID_FUNC_PTR) CSSM_VL_VerifyRootCredentialsDataAndContainment;
    default:
        return NULL;
    }
}



