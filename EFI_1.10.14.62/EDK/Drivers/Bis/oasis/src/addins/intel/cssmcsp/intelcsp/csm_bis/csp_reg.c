/*-----------------------------------------------------------------------
 *      File:   csp_reg.c
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
 * This file contains the CSP for WfM BIS registritation functions.
 */

#include "tal_inc.h"
#include "csp_reg.h"
#include "guids.h"

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                          Registration functions                          */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: CSP_Initialize
 *
 *Description:
 * This function initializes the CSP by checking that the app has requested
 * a supported version. 
 * 
 *Parameters: 
 * Handle (input) - CSSM managed add-in handle.
 * VerMajor (input) - The major version number to be compared for compatiblity
 * VerMinor (input) - The minor version number to be compared for compatiblity
 *
 *Return value:
 * An indicator of whether or not the CL was initialized
 *
 *Error Codes:
 * CSSM_CSP_VERSION_ERROR
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSP_Initialize(CSSM_MODULE_HANDLE Handle,
                                   uint32 nVerMajor,
								   uint32 nVerMinor)
{
    /* Touch the variable to surpass the level 4 warning for unused variables */
    Handle;

    /* CheckVersion */
    if (nVerMajor != INTEL_CSM_MAJOR_VER || nVerMinor != INTEL_CSM_MINOR_VER)
    {
        TAL_SetError(CSSM_CSP_VERSION_ERROR);
        return CSSM_FAIL;
    }
    return CSSM_OK;
}

/*-----------------------------------------------------------------------------
 *Name: CSP_Terminate
 *
 *Description:
 * This function un-initializes the CSP.
 * 
 *Parameters: 
 * CSPHandle(input) - CSSM managed CSP handle is used for up calls to memory
 *                    functions
 *
 *Return value:
 * CSSM_OK -   Staged operation succeeded
 * CSSM_FAIL - Staged operation failed
 *
 *Error Codes:
 * none
 *---------------------------------------------------------------------------*/
CSSM_RETURN	CSSMAPI CSP_Terminate(CSSM_MODULE_HANDLE Handle)
{
    /* Touch the variable to surpass the level 4 warning for unused variables */
    Handle;

    return CSSM_OK;
}

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                              Extern  functions                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: TALCSP_Initialize_RegInfo
 *
 *Description:
 * This function calls TAL functio to initialize the CSSM_REGISTRATION_INFO_PTR
 * data structure.
 *
 *Parameters: 
 * RegInfo_ptr(input/output) -  pointer to CSSM_REGISTRATION_INFO to be setup
 * CspFuncs_ptr(input)- contains the CSP supported Crypto API functions
 * DlFuncs_ptr(input) - contains the CSP supported DL API functions
 *
 *Returns:
 * CSSM_OK -   Initialization succeeded
 * CSSM_FAIL - Initialization failed
 *---------------------------------------------------------------------------*/
CSSM_RETURN	TALCSP_Initialize_RegInfo(
                             CSSM_REGISTRATION_INFO_PTR RegInfo_ptr,
                             const CSSM_SPI_CSP_FUNCS_PTR	CspFuncs_ptr,
                             const CSSM_SPI_DL_FUNCS_PTR    DlFuncs_ptr)
{
    return TAL_Initialize_RegInfo(RegInfo_ptr,
                                  CSP_Initialize,
                                  CSP_Terminate,
                                  NULL,
                                  NULL,
                                  NULL,
                                  CspFuncs_ptr,
                                  DlFuncs_ptr);
}
