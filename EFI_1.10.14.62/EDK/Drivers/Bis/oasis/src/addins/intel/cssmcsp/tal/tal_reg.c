/*-----------------------------------------------------------------------
 *      File:   tal_reg.c
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
 * This file contains module registration info initialization functions.
 */

#include "tal_inc.h"
#include "csp_reg.h"

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                              Module veriables                            */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
CSSM_SPI_CSP_FUNCS      m_CSP_Funcs;
CSSM_MODULE_FUNCS       m_JT[1];

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                              Extern  functions                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: TAL_Initialize_RegInfo
 *
 *Description:
 * This function initiualizes the CSSM_REGISTRATION_INFO data structure and set
 * the CSP Module functions.
 *
 *Parameters: 
 *Parameters: 
 * RegInfo_ptr(input/output) -  pointer to CSSM_REGISTRATION_INFO to be setup
 * fp_Initialize(input)    - pinter to Module Initialize function
 * fp_Terminate(input)     - pinter to Module Terminate function
 * fp_EventNotify(input)   - pinter to Module EventNotify function
 * fp_GetModuleInfo(input) - pinter to Module GetModuleInfo function
 * fp_FreeModuleInfo(input)- pinter to Module FreeModuleInfo function
 * CspFuncs_ptr(input)- contains the CSP supported Crypto API functions
 * DlFuncs_ptr(input) - contains the CSP supported DL API functions
 *
 *Returns:
 * CSSM_OK -   Initialization is successful
 * CSSM_FAIL - Initialization failed
 *---------------------------------------------------------------------------*/
CSSM_RETURN	TAL_Initialize_RegInfo(CSSM_REGISTRATION_INFO_PTR RegInfo_ptr,
                                   FP_Initialize fp_Initialize,
                                   FP_Terminate fp_Terminate,
                                   FP_EventNotify fp_EventNotify,
                                   FP_GetModuleInfo fp_GetModuleInfo,
                                   FP_FreeModuleInfo fp_FreeModuleInfo,
                                   const CSSM_SPI_CSP_FUNCS_PTR	CspFuncs_ptr,
                                   const CSSM_SPI_DL_FUNCS_PTR  DlFuncs_ptr)
{
   /* Touch the variable to surpass the level 4 warning for unused variables */
    DlFuncs_ptr;

    if (RegInfo_ptr==NULL)
    {
        TAL_SetError(CSSM_CSP_INVALID_POINTER);
        return CSSM_FAIL;
    }
    RegInfo_ptr->Initialize        = fp_Initialize;
    RegInfo_ptr->Terminate         = fp_Terminate;
    RegInfo_ptr->EventNotify       = fp_EventNotify;
    RegInfo_ptr->GetModuleInfo     = fp_GetModuleInfo;
    RegInfo_ptr->FreeModuleInfo    = fp_FreeModuleInfo;
    RegInfo_ptr->ThreadSafe        = CSSM_FALSE;
    RegInfo_ptr->ServiceSummary    = CSSM_SERVICE_CSP;
    RegInfo_ptr->NumberOfServiceTables = sizeof(m_JT)/sizeof(CSSM_MODULE_FUNCS);
    RegInfo_ptr->Services          = m_JT;

    /* Initialize software CSP Module functions */
    m_JT[0].ServiceType = CSSM_SERVICE_CSP;
    m_JT[0].FunctionTable.CspFuncs = &m_CSP_Funcs;

    /* CSP SPI function initialization */
    m_CSP_Funcs.VerifyData              = CspFuncs_ptr->VerifyData;
    m_CSP_Funcs.DigestData              = CspFuncs_ptr->DigestData;
    m_CSP_Funcs.QueryKeySizeInBits      = CspFuncs_ptr->QueryKeySizeInBits;

    return CSSM_OK;
}
