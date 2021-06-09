/*-----------------------------------------------------------------------
 *      File:   csp_auth.c
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
 * This file contains the CSP for WfM BIS authenticate function.
 */

#include "tal_inc.h"
#include "csp_reg.h"
#include "guids.h"

CSSM_REGISTRATION_INFO	csp_RegInfo;
CSSM_SPI_MEMORY_FUNCS	csp_MemoryFunctions;

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                                  Addin                                   */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 * Name: AddInAuthenticate
 *
 *Description:
 * This function is called by CSSM when the CSP is loaded.
 * It is used to register the CSP's function pointers with CSSM so that 
 * future calls from the application can be successfully dispatched to the CSP.
 * It is also the means by which the CSP receives the memory function pointers
 * that will be used to allocate data structures for the application.
 *
 *Parameters: 
 * cssmCredentialPath (input) : Unused
 * cssmSection        (input) : Unused
 * AppCredential      (input) : Unused
 * AppSection         (input) : Unused
 *
 *Return value:
 *   CSSM_FAIL                  - authentication failed
 *   CSSM_OK                    - authentication succeeded,
 * 
 *Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMAPI F77DDF91_FF0D_11d1_9CB7_00A0C9708162_AddInAuthenticate
                     (const char* cssmCredentialPath,
					  const char* cssmSection,
                      const char* AppCredential,
					  const char* AppSection)
{
    CSSM_SPI_CSP_FUNCS	CspFuncs;
    CSSM_GUID           guid = intel_preos_csm_bis_guid_2_0_0;

    /* Touch the variable to surpass the level 4 warning for unused variables */
    cssmCredentialPath;
    cssmSection;
    AppCredential;
    AppSection;

    /* Copy the guid value to TAL */
    cssm_memcpy(tal_guid_ptr, &guid, sizeof(CSSM_GUID));

    /* Fill in Function Table */
    cssm_memset(&CspFuncs, 0, sizeof(CSSM_SPI_CSP_FUNCS));
    CspFuncs.VerifyData     = CSP_VerifyData;
    CspFuncs.DigestData     = CSP_DigestData;
    CspFuncs.QueryKeySizeInBits     = CSP_QueryKeySizeInBits;

    /* Fill in Registration information */
    if (TALCSP_Initialize_RegInfo(&csp_RegInfo, &CspFuncs, NULL) != CSSM_OK)
        return CSSM_FAIL;

    /* Register the CSP function table with CSSM */
    return CSSM_RegisterServices(&guid, &csp_RegInfo,
                                 &csp_MemoryFunctions, NULL);
}

