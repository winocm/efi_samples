/*-----------------------------------------------------------------------------
 *      File:   cssm_jtable.h
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

#ifndef CSSM_JTABLE_H
#define CSSM_JTABLE_H

#define JTENTRY_CSSM_Init                           0
#define JTENTRY_CSSM_ModuleAttach                   1
#define JTENTRY_CSSM_ModuleDetach                   2
#define JTENTRY_CSSM_GetError                       3
#define JTENTRY_CSSM_SetError                       4
#define JTENTRY_CSSM_ClearError                     5
#define JTENTRY_CSSM_CSP_CreateSignatureContext     6
#define JTENTRY_CSSM_CSP_CreateDigestContext        7
#define JTENTRY_CSSM_DeleteContext                  8
#define JTENTRY_CSSM_DigestData                     9
#define JTENTRY_CSSM_VerifyData                    10
#define JTENTRY_CSSM_QueryKeySizeInBits            11
#define JTENTRY_CSSM_CL_CertGetFirstFieldValue     12
#define JTENTRY_CSSM_CL_CertAbortQuery             13
#define JTENTRY_CSSM_CL_CertGetKeyInfo             14
#define JTENTRY_CSSM_VL_InstantiateVoFromLocation  15
#define JTENTRY_CSSM_VL_FreeVo                     16
#define JTENTRY_CSSM_VL_GetDoInfoByName            17
#define JTENTRY_CSSM_VL_FreeDoInfos                18
#define JTENTRY_CSSM_VL_GetFirstSignatureInfo      19
#define JTENTRY_CSSM_VL_AbortScan                  20
#define JTENTRY_CSSM_VL_FreeSignatureInfo          21
#define JTENTRY_CSSM_VL_SetDoLMapEntries           22
#define JTENTRY_CSSM_VL_VerifyRootCredentialsDataAndContainment 23

typedef void (*VOID_FUNC_PTR)(void);
typedef VOID_FUNC_PTR (*CSSM_GET_FUNC_PTR)(uint32) ;

#endif
