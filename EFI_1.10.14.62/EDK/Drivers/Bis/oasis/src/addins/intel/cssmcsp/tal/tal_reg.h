/*-----------------------------------------------------------------------
 *      File:   tal_reg.h
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
 * This is the header file for TAL module registration functions.
 */

#ifndef	_TAL_REG_H
#define	_TAL_REG_H

#include "cssmtype.h"
#include "cssmspi.h"

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          type-declaration                                */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
typedef CSSM_RETURN (CSSMAPI *FP_Initialize)(CSSM_MODULE_HANDLE Handle,
                                             uint32 VerMajor,
                                             uint32 VerMinor);
typedef CSSM_RETURN (CSSMAPI *FP_Terminate)(CSSM_MODULE_HANDLE Handle);

typedef CSSM_RETURN (CSSMAPI *FP_EventNotify)(CSSM_MODULE_HANDLE Handle,
                                              const CSSM_EVENT_TYPE Event,
                                              uint32 Param);
typedef CSSM_MODULE_INFO_PTR (CSSMAPI *FP_GetModuleInfo)
                                       (CSSM_MODULE_HANDLE ModuleHandle,
                                        CSSM_SERVICE_MASK ServiceMask,
                                        uint32 SubserviceID,
                                        CSSM_INFO_LEVEL InfoLevel);
typedef CSSM_RETURN (CSSMAPI *FP_FreeModuleInfo)
                                        (CSSM_MODULE_HANDLE ModuleHandle,  
                                         CSSM_MODULE_INFO_PTR ModuleInfo);

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          Extern Functions                                */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/

/* Initialize CSSM_REGISTRATION_INFO function */
extern CSSM_RETURN	TAL_Initialize_RegInfo(
                                   CSSM_REGISTRATION_INFO_PTR pRegInfo,
                                   FP_Initialize fp_Initialize,
                                   FP_Terminate fp_Terminate,
                                   FP_EventNotify fp_EventNotify,
                                   FP_GetModuleInfo fp_GetModuleInfo,
                                   FP_FreeModuleInfo fp_FreeModuleInfo,
                                   const CSSM_SPI_CSP_FUNCS_PTR	CspFuncs_ptr,
                                   const CSSM_SPI_DL_FUNCS_PTR	DlFuncs_ptr);

/****************************************************************************/
#endif /*_TAL_REG_H*/
