/*-----------------------------------------------------------------------------
 *      File:   context.h
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
 * This file contains the data types and function prototypes for the functions 
 * used internally to manage cryptographic contexts.
 */
/*
 * The CSSM_BIS implementation of this code was inspected 8/19/98.
 */

#ifndef CSSM_CONTEXT_H
#define CSSM_CONTEXT_H

#include "internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cssm_context_node {
    CSSM_CC_HANDLE ContextHandle;
    CSSM_CONTEXT_PTR Context;
    struct cssm_context_node *Next;
} cssm_CONTEXT_NODE, *cssm_CONTEXT_NODE_PTR;


CSSM_CONTEXT_PTR cssm_CreateContext (CSSM_CSP_HANDLE CSPHandle,
                                     CSSM_CONTEXT_TYPE Class, 
                                     uint32 AlgorithmID,
                                     uint32 NumberAttributes);

CSSM_RETURN      cssm_AddAttribute  (CSSM_CONTEXT_ATTRIBUTE_PTR Attribute,
                                     CSSM_ATTRIBUTE_TYPE Type,
                                     void *Data);

void             cssm_FreeContext   (CSSM_CONTEXT_PTR Context);

CSSM_CC_HANDLE   cssm_InsertContext (CSSM_CONTEXT_PTR Context);

CSSM_CONTEXT_PTR cssm_GetContext    (CSSM_CC_HANDLE CCHandle);

CSSM_BOOL   cssm_IsBadCryptoDataPtr (CSSM_CRYPTO_DATA_PTR Data);

#ifdef __cplusplus
}
#endif

#endif
