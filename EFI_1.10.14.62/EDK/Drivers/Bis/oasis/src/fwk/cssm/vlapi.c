/*-----------------------------------------------------------------------
 *      File:   vlapi.c
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
 * This file contains the functions that are contained in the Verification Library 
 * portion of the CSSM exported functions.
 */

#include "cssm.h"
#include "cssmport.h"
#include "internal.h"
#include "cssmvli.h"


/*---------------------------------------------------------------
 *Name: CSSM_VL_InstantiateVoFromLocation
 *
 *Description:
 *  This function calls the specified VLModule to instantiate 
 *  the specified VoIdentifier or VoName from the VoBundle available
 *  at the VoBundleLocation.  If the VoIdentifier is provided, 
 *  the VoName will be ignored, and the Vo with the specified Id
 *  will be instantiated.  If the VoIdentifier is not provided and
 *  a VoName is provided, the Vo with the specified Name will be 
 *  instantiated.  If neither the VoIdentifier nor the VoName are 
 *  provided, the behavior is indeterminite.
 *
 *Parameters: 
 *  VLHandle         (input) - The handle to the VLModule that will 
 *                             perform the instantiation
 *  VoBundleLocation (input) - The location of the bundle 
 *                             containing the Vo to instantiate.
 *  VoIdentifier     (input) - The identifier for the Vo to instantiate
 *  VoName           (input) - An alternate identifier for the 
 *                             Vo to instantiate
 *
 *Returns:
 *  non 0 - A handle to the instantiated Vo
 *  0 - an error occurred 
 *
 *----------------------------------------------------------------*/
CSSM_VO_HANDLE CSSMAPI CSSM_VL_InstantiateVoFromLocation
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_LOCATION_PTR VoBundleLocation,
                                     CSSM_VO_UID_PTR VoIdentifier,
                                     CSSM_STRING VoName)
{
    CSSM_SPI_VL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /* Get the callback pointers associated with the specified handle */
    if (cssm_GetModuleRecord (VLHandle, CSSM_SERVICE_VL, 
                              (CSSM_SPI_VL_FUNCS_PTR *)&CallBack) == NULL)
        return CSSM_INVALID_HANDLE;

    /*
     * Obtain the bundle location from the registry. 
     * Call the callback function. 
     */
    if (CallBack->InstantiateVoFromLocation) {
        return CallBack->InstantiateVoFromLocation (VLHandle, VoBundleLocation, 
                                                    VoIdentifier, VoName);
    } else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_INVALID_HANDLE;
    }
}


/*---------------------------------------------------------------
 *Name: CSSM_VL_FreeVo
 *
 *Description:
 *  Allow the VLModule to cleanup all internal state information 
 *  associated with the specified instantiation.
 *
 *Parameters: 
 *  VLHandle (input) - Handle to the VLModule to perform this operation.
 *  VOHandle (input) - Handle to the Vo whose state can be freed.
 *
 *Returns:
 *  CSSM_OK - The free was successful 
 *  CSSM_FAIL - The free failed.
 *
 *----------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_VL_FreeVo  (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle)
{
    CSSM_SPI_VL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /* Get the callback pointers associated with the specified handle */
    if (cssm_GetModuleRecord (VLHandle, CSSM_SERVICE_VL, 
                              (CSSM_SPI_VL_FUNCS_PTR *)&CallBack) == NULL)
        return CSSM_FAIL;

    /*
     * Call the callback function. 
     */
    if (CallBack->FreeVo) {
        return CallBack->FreeVo (VLHandle, VOHandle);
    } else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_FAIL;
    }
}

/*---------------------------------------------------------------
 *Name: CSSM_VL_GetDoInfoByName
 *
 *Description:
 *  This function retrieves information about the specified data object
 *
 *Parameters: 
 *  VLHandle (input) - Handle to the VLModule to perform this operation.
 *  VOHandle (input) - Handle to the Vo containing the specified data object.
 *  JoinName (input) - The name of the data object whose info is requested.
 *
 *Returns:
 *  non NULL - a copy of the DoInfo for the specified data object
 *  NULL - an error occurred
 *
 *----------------------------------------------------------------*/
CSSM_VL_DO_INFO_PTR CSSMAPI CSSM_VL_GetDoInfoByName
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_STRING JoinName)
{
    CSSM_SPI_VL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /* Get the callback pointers associated with the specified handle */
    if (cssm_GetModuleRecord (VLHandle, CSSM_SERVICE_VL, 
                              (CSSM_SPI_VL_FUNCS_PTR *)&CallBack) == NULL)
        return NULL;

    /*
     * Call the callback function. 
     */
    if (CallBack->GetDoInfoByName) {
        return CallBack->GetDoInfoByName (VLHandle, VOHandle, JoinName);
    } else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return NULL;
    }
}


/*---------------------------------------------------------------
 *Name: CSSM_VL_FreeDoInfos
 *
 *Description:
 *  This function frees the input structures
 *
 *Parameters: 
 *  VLHandle (input) - Handle to the VLModule to perform this operation.
 *  DoInfos (input) - The structure to be freed.
 *  NumberOfDoInfos (input) - The length of the DoInfos array.
 *
 *Returns:
 *  CSSM_OK - The free was successful.
 *  CSSM_FAIL - an error occurred.
 *
 *----------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_VL_FreeDoInfos
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_DO_INFO_PTR DoInfos,
                                     uint32 NumberOfDoInfos)
{
    CSSM_SPI_VL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /* Get the callback pointers associated with the specified handle */
    if (cssm_GetModuleRecord (VLHandle, CSSM_SERVICE_VL, 
                              (CSSM_SPI_VL_FUNCS_PTR *)&CallBack) == NULL)
        return CSSM_FAIL;

    /*
     * Call the callback function. 
     */
    if (CallBack->FreeDoInfos) {
        return CallBack->FreeDoInfos (VLHandle, DoInfos, NumberOfDoInfos);
    } else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_FAIL;
    }
}


/*---------------------------------------------------------------
 *Name: CSSM_VL_GetFirstSignatureInfo
 *
 *Description:
 *  This function retrieves info about the first signature over 
 *  the specified Vo.
 *
 *Parameters: 
 *  VLHandle (input) - Handle to the VLModule to perform this operation.
 *  VOHandle (input) - Handle to the Vo whose signature info is requested.
 *  SignerIteratorHandle (output) - Handle to use to obtain info 
 *                                  about additional signatures.
 *
 *Returns:
 *  non NULL - a copy of the first SignatureInfo
 *  NULL - an error occurred
 *
 *----------------------------------------------------------------*/
CSSM_VL_SIGNATURE_INFO_PTR CSSMAPI CSSM_VL_GetFirstSignatureInfo
                                    (CSSM_VL_HANDLE VLHandle, 
                                     CSSM_VO_HANDLE VOHandle,
                                     CSSM_HANDLE *SignerIteratorHandle)
{
    CSSM_SPI_VL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /* Get the callback pointers associated with the specified handle */
    if (cssm_GetModuleRecord (VLHandle, CSSM_SERVICE_VL, 
                              (CSSM_SPI_VL_FUNCS_PTR *)&CallBack) == NULL)
        return NULL;

    /*
     * Call the callback function. 
     */
    if (CallBack->GetFirstSignatureInfo) {
        return CallBack->GetFirstSignatureInfo (VLHandle, VOHandle, SignerIteratorHandle);
    } else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return NULL;
    }
}

/*---------------------------------------------------------------
 *Name: CSSM_VL_AbortScan
 *
 *Description:
 *  This function aborts the Vo query identified by IteratorHandle.
 *
 *Parameters: 
 *  VLHandle (input) - Handle to the VLModule to perform this operation.
 *  IteratorHandle (input) - Handle identifying a signature query.
 *
 *Returns:
 *  CSSM_OK - The abort was successful.
 *  CSSM_FAIL - an error occurred.
 *
 *----------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_VL_AbortScan
                                    (CSSM_VL_HANDLE VLHandle, 
                                     CSSM_HANDLE IteratorHandle)
{
    CSSM_SPI_VL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /* Get the callback pointers associated with the specified handle */
    if (cssm_GetModuleRecord (VLHandle, CSSM_SERVICE_VL, 
                              (CSSM_SPI_VL_FUNCS_PTR *)&CallBack) == NULL)
        return CSSM_FAIL;

    /*
     * Call the callback function. 
     */
    if (CallBack->AbortScan) {
        return CallBack->AbortScan (VLHandle, IteratorHandle);
    } else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_FAIL;
    }
}


/*---------------------------------------------------------------
 *Name: CSSM_VL_FreeSignatureInfo
 *
 *Description:
 *  This function frees the input structure.
 *
 *Parameters: 
 *  VLHandle (input) - Handle to the VLModule to perform this operation.
 *  SignatureInfo (input) - The structure to be freed.
 *
 *Returns:
 *  CSSM_OK - The free was successful.
 *  CSSM_FAIL - an error occurred.
 *
 *----------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_VL_FreeSignatureInfo
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_SIGNATURE_INFO_PTR SignatureInfo)
{
    CSSM_SPI_VL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /* Get the callback pointers associated with the specified handle */
    if (cssm_GetModuleRecord (VLHandle, CSSM_SERVICE_VL, 
                              (CSSM_SPI_VL_FUNCS_PTR *)&CallBack) == NULL)
        return CSSM_FAIL;

    /*
     * Call the callback function. 
     */
    if (CallBack->FreeSignatureInfo) {
        return CallBack->FreeSignatureInfo (VLHandle, SignatureInfo);
    } else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_FAIL;
    }
}

/*---------------------------------------------------------------
 *Name: CSSM_VL_SetDoLMapEntries
 *
 *Description:
 *  This function sets the data object locations for the specified Vo
 *
 *Parameters: 
 *  VLHandle (input) - Handle to the VLModule to perform this operation.
 *  VOHandle (input) - Handle to the Vo whose DoLocationMap should be set.
 *  NewLocationEntries (input) - The new data object locations.
 *
 *Returns:
 *  CSSM_OK - The new locations were successfully set.
 *  CSSM_FAIL - An error occurred.
 *
 *----------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_VL_SetDoLMapEntries
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_VL_DO_LMAP_PTR NewLocationEntries)
{
    CSSM_SPI_VL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /* Get the callback pointers associated with the specified handle */
    if (cssm_GetModuleRecord (VLHandle, CSSM_SERVICE_VL, 
                              (CSSM_SPI_VL_FUNCS_PTR *)&CallBack) == NULL)
        return CSSM_FAIL;

    /*
     * Call the callback function. 
     */
    if (CallBack->SetDoLMapEntries) {
        return CallBack->SetDoLMapEntries (VLHandle, VOHandle, NewLocationEntries);
    } else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_FAIL;
    }
}

/*---------------------------------------------------------------
 *Name: CSSM_VL_VerifyRootCredentialsDataAndContainment
 *
 *Description:
 *  This function verifies the specified Vo and the data objects it covers.
 *  It also verifies that the input containment points are within 
 *  the appropriate objects.
 *
 *Parameters: 
 *  VLHandle (input) - Handle to the VLModule to perform this operation.
 *  VOHandle (input) - Handle to the Vo to verify.
 *  SignerCertificate (input) - The signature to verify
 *  NumberOfPreferredCsps (input) - Length of the PreferredCsps array
 *  PreferredCsps (input) - CSPs to use in verification.
 *  NumberOfContainments (input) - Length of the ContainmentsToVerify array
 *  ContainmentsToVerify (input) - Containment points to be verified.
 *
 *Returns:
 *  non 0 - The credentials, data objects and containments verified 
 *  0 - The credentials, data objects or containments did not verify 
 *      or an error occurred
 *
 *----------------------------------------------------------------*/
CSSM_VL_VERIFICATION_HANDLE CSSMAPI CSSM_VL_VerifyRootCredentialsDataAndContainment
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_DATA_PTR SignerCertificate,
                                     uint32 NumberOfPreferredCsps, 
                                     const CSSM_VL_PREFERRED_CSP_PTR PreferredCSPs,
                                     uint32 NumberOfContainments,
                                     const CSSM_VL_DO_CONTAINMENT_LOCATION_PTR ContainmentsToVerify)
{
    CSSM_SPI_VL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /* Get the callback pointers associated with the specified handle */
    if (cssm_GetModuleRecord (VLHandle, CSSM_SERVICE_VL, 
                              (CSSM_SPI_VL_FUNCS_PTR *)&CallBack) == NULL)
        return CSSM_INVALID_HANDLE;

    /*
     * Call the callback function. 
     */
    if (CallBack->VerifyRootCredentialsDataAndContainment) {
        return CallBack->VerifyRootCredentialsDataAndContainment (VLHandle, VOHandle, 
                                    SignerCertificate, NumberOfPreferredCsps, PreferredCSPs, 
                                    NumberOfContainments, ContainmentsToVerify);
    } else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_INVALID_HANDLE;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:
//   CSSM_VL_SelfVerifyCertificate
//
// Description:
//   This procedure performs a syntax verification of the passed certificate.  The intent is to do the
//   same verification that would be performed if the certificate were used to sign a signed manifest
//   and the signed manifest were passed to VL_VerifyRootCredentialsDataAndContainment.  The tests
//   do not include testing the signature of the certificate, since the issuer's public key is generally
//   not readily available.
//
// Parameters:
//   VLHandle    - Handle to the Verifiable Object Library that does the operation.
//   Certificate - The certificate to be verified
//
// Returns:
//   ISL_OK - if the verification succeeds
//   other  - if the verification fails


uint32 CSSMAPI CSSM_VL_SelfVerifyCertificate(
    CSSM_VL_HANDLE       VLHandle,
    const CSSM_DATA_PTR  Certificate
    )
{
    CSSM_SPI_VL_FUNCS_PTR CallBack = NULL;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */
    /* Get the callback pointers associated with the specified handle */
    if (cssm_GetModuleRecord (VLHandle, CSSM_SERVICE_VL, 
                              (CSSM_SPI_VL_FUNCS_PTR *)&CallBack) == NULL)
        return CSSM_INVALID_HANDLE;

    /*
     * Call the callback function. 
     */
    if (CallBack->SelfVerifyCertificate) {
        return CallBack->SelfVerifyCertificate (VLHandle, Certificate);
    } else {
        CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
        return CSSM_INVALID_HANDLE;
    }
    
}
