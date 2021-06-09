/*-----------------------------------------------------------------------------
 *      File:   clapi.c
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
 * This file contains the functions that are contained in the CLI portion
 * of the CSSM exported functions.
 */

#include "cssm.h"
#include "cssmport.h"
#include "internal.h"
#include "cssmcli.h"

/*---------------------------------------------------------------
 *
 *Name: CSSM_CL_CertGetFirstFieldView
 *
 *Description:
 *   Exported function for CL module to return each field
 *
 *Parameters: 
 *   CLHandle - handle of CL module
 *   Cert - pointer to the certificate
 *   CertField - pointer to an OID of field 
 *   ResultHandle - pointer to handle that describes this search
 *   NumberOfFields - pointer to the number of fields in the 
 *
 *Returns:
 *  Null - CL unable to retreive data of certificate
 *  non Null - pointer to the first match OID
 *
 *----------------------------------------------------------------*/
CSSM_DATA_PTR CSSMAPI CSSM_CL_CertGetFirstFieldValue (CSSM_CL_HANDLE CLHandle, 
                                                 const CSSM_DATA_PTR Cert, 
                                                 const CSSM_OID_PTR CertField,
                                                 CSSM_HANDLE_PTR ResultsHandle,
                                                 uint32 *NumberOfMatchedFields)
{
    CSSM_SPI_CL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /*
	 * Get the ModuleInfo record and the callback pointers associated with the
	 * specified handle and the service mask.
	 */
	if (cssm_GetModuleRecord (CLHandle, CSSM_SERVICE_CL, 
                              (CSSM_SPI_CL_FUNCS_PTR *)&CallBack) == NULL)
		RETURN (NULL)

	/*
 	 * Call the callback function. 
	 * Make sure that function structure and function pointer is valid 
	 */
    if (CallBack->CertGetFirstFieldValue)
        return CallBack->CertGetFirstFieldValue (CLHandle, Cert, 
                               CertField, ResultsHandle, NumberOfMatchedFields);
    else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return NULL;
    }
}


/*---------------------------------------------------------------
 *
 *Name: CSSM_CL_CertAbortQuery
 *
 *Description:
 *   Exported function for CL module to abort retreiving fields
 *
 *Parameters: 
 *   CLHandle - handle of CL module
 *   ResultHandle - handle that describes this search
 *
 *Returns:
 *  CSSM_FAIL - CL unable to terminate search
 *  CSSM_OK - search has been terminated
 *
 *----------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_CL_CertAbortQuery (CSSM_CL_HANDLE CLHandle, 
                                            CSSM_HANDLE ResultsHandle)
{
    CSSM_SPI_CL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /*
	 * Get the ModuleInfo record and the callback pointers associated with the
	 * specified handle and the service mask.
	 */
	if (cssm_GetModuleRecord (CLHandle, CSSM_SERVICE_CL, 
                              (CSSM_SPI_CL_FUNCS_PTR *)&CallBack) == NULL)
		return CSSM_FAIL;

	/*
 	 * Call the callback function. 
	 * Make sure that function structure and function pointer is valid 
	 */
    if (CallBack->CertAbortQuery)
        return CallBack->CertAbortQuery (CLHandle, ResultsHandle);
    else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_FAIL;
    }
}


/*---------------------------------------------------------------
 *
 *Name: CSSM_CL_CertGetKeyInfo
 *
 *Description:
 *   Exported function for CL module to retreiving the key from
 *   the certificate
 *
 *Parameters: 
 *   CLHandle - handle of CL module
 *   Cert - pointer to the certificate
 *
 *Returns:
 *  Null - CL unable to retreive the key
 *  non Null - pointer to the key
 *
 *----------------------------------------------------------------*/
CSSM_KEY_PTR CSSMAPI CSSM_CL_CertGetKeyInfo (CSSM_CL_HANDLE CLHandle, 
                                            const CSSM_DATA_PTR Cert)
{
    CSSM_SPI_CL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /*
	 * Get the ModuleInfo record and the callback pointers associated with the
	 * specified handle and the service mask.
	 */
	if (cssm_GetModuleRecord (CLHandle, CSSM_SERVICE_CL, 
                              (CSSM_SPI_CL_FUNCS_PTR *)&CallBack) == NULL)
        return NULL;

	/*
 	 * Call the callback function. 
	 * Make sure that function structure and function pointer is valid 
	 */
    if (CallBack->CertGetKeyInfo)
        return CallBack->CertGetKeyInfo (CLHandle, Cert);
    else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return NULL;
    }
}

