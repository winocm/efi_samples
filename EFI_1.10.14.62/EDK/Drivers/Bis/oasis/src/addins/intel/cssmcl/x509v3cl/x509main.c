/*-----------------------------------------------------------------------
 *      File:   x509main.c
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
 * This file implements the CL SPI functions that do not correspond to CL APIs.
 */

#include "x_fndefs.h"
#include "cssmspi.h"

CSSM_SPI_MEMORY_FUNCS CLMemFuncs;
CSSM_CSP_HANDLE       CL_CSPHandle;

/*-----------------------------------------------------------------------------
 * Name: CL_Initialize
 *
 * Description:
 * This function initializes the CL by checking that the app has requested
 * a supported version. 
 * 
 * NOTE: ClearError() is not necessary because it is performed by CSSM.
 *
 * Parameters: 
 * VerMajor (input) : The major version number to be compared for compatiblity
 * VerMinor (input) : The minor version number to be compared for compatiblity
 *
 * Return value:
 * An indicator of whether or not the CL was initialized
 *
 * Error Codes:
 * CSSM_CL_INITIALIZE_FAIL
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMCLI CL_Initialize(CSSM_MODULE_HANDLE Handle, 
                                  uint32 VerMajor, uint32 VerMinor)
{
    CSSM_VERSION    csp_version = {INTEL_CSM_MAJOR_VER, INTEL_CSM_MINOR_VER};
    Handle;

    /* CheckVersion */
    if (VerMajor != INTEL_X509V3_CLM_MAJOR_VER || 
        VerMinor != INTEL_X509V3_CLM_MINOR_VER)
    {
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_INITIALIZE_FAIL);
        return CSSM_FAIL;
    }

    CL_CSPHandle = CSSM_ModuleAttach (&intel_preos_csm_guid, &csp_version,
                                      NULL, 0, 0, 0, NULL, NULL, NULL, NULL);
    if (!CL_CSPHandle)
    {
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_INITIALIZE_FAIL);
        return CSSM_FAIL;
    }

    return CSSM_OK;
}


/*-----------------------------------------------------------------------------
 * Name: CL_Terminate
 *
 * Description:
 * This function un-initializes the CL by releasing its connection to the 
 * Intel WfM CSP.
 * 
 * Parameters: 
 *
 * Return value:
 * An indicator of whether or not the CL was un-initialized
 *
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMCLI CL_Terminate(CSSM_MODULE_HANDLE Handle)
{
    Handle;
    return CSSM_ModuleDetach(CL_CSPHandle);
}


/*-----------------------------------------------------------------------------
 * Name: AddInAuthenticate
 *
 * Description:
 * This function is called by CSSM when the CL is loaded.
 * It is used to register the CL's function pointers with CSSM so that 
 * future calls from the application can be successfully dispatched to the CL.
 * It is also the means by which the CL receives the memory function pointers
 * that will be used to allocate data structures for the application.
 *
 * Parameters: 
 * cssmCredentialPath (input) : Unused
 * cssmSection        (input) : Unused
 * AppCredential      (input) : Unused
 * AppSection         (input) : Unused
 *
 * Return value:
 * An indicator of the success or failure of CSSM_RegisterServices
 * 
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMAPI F38A8023_BDB1_11d1_93C8_00A0C93C3211_AddInAuthenticate
                     (const char* cssmCredentialPath, const char* cssmSection,
                      const char* AppCredential, const char* AppSection)
{
    CSSM_REGISTRATION_INFO CLRegInfo;
    CSSM_MODULE_FUNCS      Services;
    CSSM_SPI_CL_FUNCS      FunctionTable;
    cssmCredentialPath;    cssmSection;
    AppCredential;         AppSection;

    /* Fill in Registration information */
    cssm_memset(&CLRegInfo, 0, sizeof(CSSM_REGISTRATION_INFO));
    CLRegInfo.Initialize            = CL_Initialize;  
    CLRegInfo.Terminate             = CL_Terminate;  
    CLRegInfo.ServiceSummary        = CSSM_SERVICE_CL;
    CLRegInfo.NumberOfServiceTables = 1;
    CLRegInfo.Services              = &Services;

    /* Fill in Services */
    Services.ServiceType = CSSM_SERVICE_CL;
    Services.FunctionTable.ClFuncs = &FunctionTable;

    /* Fill in Function Table */
    cssm_memset(&FunctionTable, 0, sizeof(CSSM_SPI_CL_FUNCS));
    FunctionTable.CertGetFirstFieldValue  = CL_CertGetFirstFieldValue;
    FunctionTable.CertAbortQuery          = CL_CertAbortQuery;
    FunctionTable.CertGetKeyInfo          = CL_CertGetKeyInfo;

    /* Register the CL function table with CSSM */
    return CSSM_RegisterServices(&intel_preos_clm_guid, 
                                 &CLRegInfo, &CLMemFuncs, NULL);
}


