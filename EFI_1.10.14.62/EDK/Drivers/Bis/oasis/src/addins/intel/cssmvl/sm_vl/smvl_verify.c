/*-----------------------------------------------------------------------
 *      File:   smvl_verify.c
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
 * This file implements all of the global module functions
 */

    /* Windows & CSSM includes */
#include "cssm.h"
#include "cssmspi.h"
#include "cssmport.h"
#include "guids.h"
    /* VL-specific includes */
#include  "smvl.h"
#pragma warning (disable : 4100)

/* VL Global Data Structures */
/*-----------------------------------------------------------------------------
 * Name: VL_VerifyRootCredentialsDataAndContainment
 *
 * Description:
 * This function verifies a both the credentials and data object of a
 * VO and will also range checking for the DO
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * VOHandle (input) - A handle to an instantiate VO
 * SignerCertificate (input) -  
 * NumberOfPreferredCsps (input) - 
 * PreferredCSPs (input) -
 * NumberOfContainments (input)
 * ContainmentsToVerify (input)
 *
 * Return value:
 * CSSM_VL_VERIFICATION_HANDLE
 * CSSM_INVALID_HANDLE
 * 
 * Error Codes:
 *
 * Notes:
 * This implementation does not support a number of options and requires
 * the following in order verify the credentials and data objects
 *
 * NumberOfPreferredCsps == 0
 * PreferredCSPS == NULL
 * NumberOfContainments == 0
 * ContainmentsToVerify == NULL
 *
 *---------------------------------------------------------------------------*/
CSSM_VL_VERIFICATION_HANDLE CSSMVLI VL_VerifyRootCredentialsDataAndContainment
    (CSSM_VL_HANDLE VLHandle,
     CSSM_VO_HANDLE VOHandle,
     const CSSM_DATA_PTR SignerCertificate,
     uint32 NumberOfPreferredCsps, 
     const CSSM_VL_PREFERRED_CSP_PTR PreferredCSPs,
     uint32 NumberOfContainments,
     const CSSM_VL_DO_CONTAINMENT_LOCATION_PTR ContainmentsToVerify)
{
    SMVL_VO_CONTEXT_PTR VoContextPtr;
    ISL_MEMORY_CONTEXT  MemoryContext;
    ISL_CONST_DATA      Cert;
    uint32              error=  ISL_OK;

    static uint32 Count = 1;

    /* resources to track */
    ISL_CERTIFICATE_PTR CertificatePtr = NULL;

    VoContextPtr = smvl_GetVoContext(VOHandle);
    if (VoContextPtr == NULL)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_INVALID_VO_HANDLE);
        return 0;
    }

    if (NumberOfPreferredCsps > 0)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_INVALID_PREFERENCES);
        return 0;
    }
        
    if ( NumberOfContainments > 0)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_CONTAINMENT_NOT_SUPPORTED);
        return 0;
    }

    {
        uint32 SignedObjectIndex;
        ISL_SIG_SECTION_GROUP_PTR SignedObjectInfoPtr;

        CertificatePtr = g_fx.malloc_func(VLHandle, (uint32)(X509CertMethods.SizeofObject()));
        if (CertificatePtr == NULL) 
        {
            error = CSSM_MEMORY_ERROR;
            goto FAIL;
        }
    
        Cert.Data = SignerCertificate->Data;
        Cert.Length = SignerCertificate->Length;

        MemoryContext.MemoryMethods = &gISL_mem_funcs;
        MemoryContext.AllocRef = NULL;
        MemoryContext.Buffers = NULL;

        if (ISL_OK != X509CertMethods.Initialize(
                        CertificatePtr, 
                        &MemoryContext, 
                        Cert))
        {
            error = CSSM_VL_INVALID_CERTIFICATE;
            goto FAIL;
        }


        if (ISL_OK != ArchiveMethods.Verify(
                        VoContextPtr->ArchivePtr,
                        VoContextPtr->SignaturePtr,
                        &X509CertMethods,
                        CertificatePtr))
        {
            error = CSSM_VL_INVALID_BUNDLE;
            goto FAIL;
        }

        X509CertMethods.Recycle(CertificatePtr);
        g_fx.free_func(VLHandle, CertificatePtr);
        CertificatePtr = NULL;

        SignedObjectInfoPtr = SignedListMethods.GetSignedObjectInfos(
            VoContextPtr->SignedListPtr);
        if (SignedObjectInfoPtr == NULL)
        {
            goto FAIL;
        }

        for(SignedObjectIndex = 0;
            SignedObjectIndex < SignedObjectInfoPtr->NumberOfSignedObjects;
            SignedObjectIndex++)
        {
            ISL_MANIFEST_SECTION_PTR ManifestSectionPtr;

            ManifestSectionPtr = SignedObjectMethods.GetManifestSection(
                SignedObjectInfoPtr->SignedObjects[SignedObjectIndex]);

            if (ISL_OK != ManifestSectionMethods.Verify(ManifestSectionPtr))
            {
                error = CSSM_VL_DATA_VERIFY_FAIL;
                goto FAIL;
            }
        }
        CSSM_ClearError();
    return Count++;
    }
FAIL:
    {
        X509CertMethods.Recycle(CertificatePtr);
        g_fx.free_func(VLHandle, CertificatePtr);
        CSSM_SetError(&intel_preos_vlm_guid, error);
        return CSSM_INVALID_HANDLE;
    }
}
#pragma warning (default : 4100)




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:
//   VL_SelfVerifyCertificate
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


uint32 CSSMVLI VL_SelfVerifyCertificate(
    CSSM_VL_HANDLE       VLHandle,
    const CSSM_DATA_PTR  Certificate
    )
{
    ISL_CERTIFICATE_PTR ParsedCertificatePtr = NULL;
    uint32 error = ISL_OK;
    ISL_MEMORY_CONTEXT MemoryContext;
    ISL_CONST_DATA TempCert;

    ParsedCertificatePtr = g_fx.malloc_func(VLHandle,(uint32)(X509CertMethods.SizeofObject()));
    if (ParsedCertificatePtr == NULL) {
        error = CSSM_MEMORY_ERROR;
        goto SELF_VERIFY_FAIL;
    }

    TempCert.Data = Certificate->Data;
    TempCert.Length = Certificate->Length;

    MemoryContext.MemoryMethods = &gISL_mem_funcs;
    MemoryContext.AllocRef = NULL;
    MemoryContext.Buffers = NULL;

    // Following call does basic syntax checking of the certificate including any checking of date
    // integrity.
    error = X509CertMethods.Initialize(
        ParsedCertificatePtr, 
        &MemoryContext, 
        TempCert
        );
    if (error != ISL_OK) {
        error = CSSM_VL_INVALID_CERTIFICATE;
        goto SELF_VERIFY_FAIL;
    }

    // And then we still haven't verified the cert's own signature.  But the verification of a signed
    // manifest does not verify the signer's certificate.  It's hard to do so, since the certificate does
    // not necessarily have the issuer's public key and there's no way to get it.  Furthermore, there is
    // no very strong security benefit in checking the signature, since self-signed certificates are
    // allowed and tools for making self-signed certificates are readily available to any intruder.

SELF_VERIFY_FAIL:
    {
        X509CertMethods.Recycle(ParsedCertificatePtr);
        g_fx.free_func(VLHandle, ParsedCertificatePtr);
        return error;
    }

} // VL_SelfVerifyCertificate

