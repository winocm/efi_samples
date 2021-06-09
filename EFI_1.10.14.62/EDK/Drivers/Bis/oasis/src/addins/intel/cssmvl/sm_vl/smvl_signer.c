/*-----------------------------------------------------------------------
 *      File:   smvl_signer.c
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
#include "guids.h"
    /* VL-specific includes */
#include  "smvl.h"

extern CSSM_SPI_MEMORY_FUNCS  g_fx;

#pragma warning (disable : 4100)

typedef struct vl_signature_info_iterator
{
    ISL_SIGNER_GROUP_PTR SignerInfoPtr;
    uint32 SignerIndex;
} VL_SIGNATURE_INFO_ITERATOR, *VL_SIGNATURE_INFO_ITERATOR_PTR;


/*-----------------------------------------------------------------------------
 * Name: InitializeCertList
 *
 * Description:
 *
 * Parameters:
 * CssmCertList (input/output)
 * IslCertList (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_DATA_PTR
InitializeCertList(
    CSSM_DATA_PTR CssmCertList, 
    ISL_CERTIFICATE_GROUP_PTR CertificateGroupPtr)
{
    ISL_CERTIFICATE_PTR CertificatePtr;
    ISL_CONST_DATA theCert;
    uint32 i;
    /* note i am checking to see if enough memory has been allocated
       will added stronger checking later */

    for(i=0; i < CertificateGroupPtr->NumberOfCertificates; i++)
    {
        CertificatePtr = CertificateGroupPtr->Certificates[i];
        X509CertMethods.Flatten(CertificatePtr, &theCert);

        CssmCertList->Data = (uint8 *)theCert.Data;
        CssmCertList->Length = theCert.Length;
        CssmCertList++;
    }

    return CssmCertList;
}

/*-----------------------------------------------------------------------------
 * Name: CreateCertGroup
 *
 * Description:
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * SignaturePtr (input)
 * SignerCert (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_CERTGROUP_PTR
CreateCertGroup(
    CSSM_VL_HANDLE VLHandle,
    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr,
    ISL_SIGNATURE_CONTEXT_PTR SignaturePtr,
    CSSM_DATA_PTR SignerCert)
{
    CSSM_CERTGROUP_PTR CertGroupPtr = NULL;
    CSSM_DATA_PTR CertList = NULL;
    uint32 error;

    ISL_CERTIFICATE_GROUP_PTR SignerCertsPtr;
    ISL_CERTIFICATE_GROUP_PTR ArchiveCertsPtr;

    //
    // BUGBUG - init'd error with zero to avoid warning 4
    //
    error = 0;

    SignerCertsPtr = NULL;
    ArchiveCertsPtr = NULL;
    {
        CertGroupPtr = g_fx.calloc_func(
            VLHandle,
            1,
            sizeof(CSSM_CERTGROUP));
        if (CertGroupPtr == NULL) {
            error = CSSM_MEMORY_ERROR;
                goto FAIL;
        }

        CertGroupPtr->CertType = CSSM_CERT_X_509v3;
        CertGroupPtr->CertEncoding = CSSM_CERT_ENCODING_DER;

        SignerCertsPtr = PKCS7SignatureMethods.GetCertificateGroup(
            SignaturePtr);
        if (SignerCertsPtr == NULL) 
        {
            goto FAIL;
        }
        ArchiveCertsPtr = ArchiveMethods.GetCertificateGroup(ArchivePtr);
        if (ArchiveCertsPtr == NULL)
        {
            goto FAIL;
        }
        if ( SignerCert != NULL )
        {
            /* Count SingnerCert as one cert only when it is not NULL */
            CertGroupPtr->NumCerts = 1 + ArchiveCertsPtr->NumberOfCertificates +
                SignerCertsPtr->NumberOfCertificates;
        }
        else
        {
            CertGroupPtr->NumCerts = ArchiveCertsPtr->NumberOfCertificates +
                SignerCertsPtr->NumberOfCertificates;
        }
        CertList = g_fx.calloc_func(
                        VLHandle, 
                        CertGroupPtr->NumCerts,
                        sizeof(CSSM_DATA));
        if (CertList == NULL)
        {
            error = CSSM_MEMORY_ERROR;
            goto FAIL;
        }
        CertGroupPtr->CertList = CertList;
        /* Attach SingnerCert to certlist only when it is not NULL */
        if ( SignerCert != NULL )
        {
            *(CertList++) = *SignerCert;
        }

        CertList = InitializeCertList(CertList, SignerCertsPtr);
        if (CertList == NULL)
        {
            error = CSSM_MEMORY_ERROR;
            goto FAIL;
        }
        CertList = InitializeCertList(CertList, ArchiveCertsPtr);     
        if (CertList == NULL)
        {
            error = CSSM_MEMORY_ERROR;
            goto FAIL;
        }
        
        CertGroupPtr->reserved = NULL;
        return CertGroupPtr;
    }
FAIL:
    {
        if (CertGroupPtr == NULL)
        {
            g_fx.free_func(VLHandle, CertGroupPtr->CertList);
            g_fx.free_func(VLHandle, CertGroupPtr);
        }
        CSSM_SetError(&intel_preos_vlm_guid, error);
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: CreateSignatureInfo
 *
 * Description:
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * SignerPtr (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_VL_SIGNATURE_INFO_PTR
CreateSignatureInfo(
    CSSM_VL_HANDLE VLHandle,
    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr,
    ISL_SIGNATURE_CONTEXT_PTR SignaturePtr,
    ISL_SIGNER_CONTEXT_PTR SignerPtr)
{
    CSSM_VL_SIGNATURE_INFO_PTR SignatureInfoPtr;
    ISL_CERTIFICATE_PTR CertPtr;
    ISL_CONST_DATA theCert;
    CSSM_DATA SignerCert;

    CertPtr = PKCS7SignerMethods.FindSignerCertificate(SignerPtr);
    if (CertPtr == NULL) return NULL;
    X509CertMethods.Flatten(CertPtr, &theCert);
    SignerCert.Data = (uint8 *)theCert.Data;
    SignerCert.Length = theCert.Length;

    {
        ISL_CONST_DATA SignerSignature;

        SignatureInfoPtr = g_fx.calloc_func(
            VLHandle, 
            1, 
            sizeof(CSSM_VL_SIGNATURE_INFO));
        if (SignatureInfoPtr == NULL)
        {
            goto FAIL;
        }

        SignatureInfoPtr->SigningAlgorithm = CSSM_ALGID_SHA1WithDSA;
        if (PKCS7SignerMethods.GetSignature(SignerPtr, &SignerSignature) 
            != ISL_OK)
        {
            goto FAIL;
        }
        SignatureInfoPtr->Signature.Data = (uint8 *)SignerSignature.Data;
        SignatureInfoPtr->Signature.Length = SignerSignature.Length;
        SignatureInfoPtr->SignerCertGroup = CreateCertGroup(
            VLHandle,
            ArchivePtr,
            SignaturePtr,
            &SignerCert);
        if (SignatureInfoPtr->SignerCertGroup == NULL)
        {
            goto FAIL;
        }

        return SignatureInfoPtr;
    }

FAIL:
    {
        g_fx.free_func(VLHandle, SignatureInfoPtr);
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: VL_GetFirstSignatureInfo
 *
 * Description:
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * VOHandle (input) - a handle to an instantiate VO
 * SignerIteratorHandle (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_VL_SIGNATURE_INFO_PTR CSSMVLI VL_GetFirstSignatureInfo
                                    (CSSM_VL_HANDLE VLHandle, 
                                     CSSM_VO_HANDLE VOHandle,
                                     CSSM_HANDLE *SignerIteratorHandle)
{
    SMVL_VO_CONTEXT_PTR VoContextPtr;

    ISL_SIGNER_GROUP_PTR SignerInfoPtr;
    ISL_SIGNER_CONTEXT_PTR SignerPtr;
    CSSM_VL_SIGNATURE_INFO_PTR SignatureInfoPtr;
    VL_SIGNATURE_INFO_ITERATOR_PTR IteratorPtr;

    
    VoContextPtr = smvl_GetVoContext(VOHandle);
    if (VoContextPtr == NULL)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_INVALID_VO_HANDLE);    
        return NULL;
    }

    if (SignerIteratorHandle == NULL)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_INVALID_POINTER);    
        return NULL;
    }

    *SignerIteratorHandle = 0;

    SignerInfoPtr = PKCS7SignatureMethods.GetSignerGroup(
        VoContextPtr->SignaturePtr);
    if (SignerInfoPtr == NULL ||
        SignerInfoPtr->NumberOfSigners == 0) return NULL;

    SignerPtr = SignerInfoPtr->Signers[0];
    IteratorPtr = NULL;
    {
        IteratorPtr = g_fx.malloc_func(
            VLHandle, 
            sizeof(VL_SIGNATURE_INFO_ITERATOR));
        if (IteratorPtr == NULL){
            goto FAIL;
        }

        IteratorPtr->SignerInfoPtr = SignerInfoPtr;
        IteratorPtr->SignerIndex = 1;

        SignatureInfoPtr = CreateSignatureInfo(
            VLHandle, 
            VoContextPtr->ArchivePtr,
            VoContextPtr->SignaturePtr,
            SignerPtr);

        if (SignatureInfoPtr == NULL)
        {
            goto FAIL;
        }

        *SignerIteratorHandle = (CSSM_HANDLE)IteratorPtr;
        return SignatureInfoPtr;
    }

FAIL:
    {
        g_fx.free_func(VLHandle, IteratorPtr);
        return NULL;
    }
}


/*-----------------------------------------------------------------------------
 * Name: VL_AbortScan
 *
 * Description:
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * IteratorHandle (input)
 *
 * Return value:
 * pass/fail status of the operation
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMVLI VL_AbortScan    
            (CSSM_VL_HANDLE VLHandle, 
             CSSM_HANDLE IteratorHandle)
{
    VL_SIGNATURE_INFO_ITERATOR_PTR IteratorPtr;

    IteratorPtr = (VL_SIGNATURE_INFO_ITERATOR_PTR) IteratorHandle;
    g_fx.free_func(VLHandle, IteratorPtr);
    CSSM_ClearError();
    return CSSM_OK;
}

/*-----------------------------------------------------------------------------
 * Name: VL_FreeSignatureInfo
 *
 * Description:
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * SignatureInfo (input)
 *
 * Return value:
 * pass/fail status of the operation
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMVLI VL_FreeSignatureInfo
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_SIGNATURE_INFO_PTR SignatureInfo)
{
    if (SignatureInfo == NULL ||
        SignatureInfo->SignerCertGroup == NULL)
    {
        return CSSM_OK;
    }

    g_fx.free_func(VLHandle, SignatureInfo->SignerCertGroup->CertList);
    g_fx.free_func(VLHandle, SignatureInfo->SignerCertGroup);
    g_fx.free_func(VLHandle, SignatureInfo);
    CSSM_ClearError();
    return CSSM_OK;
}

#pragma warning (default : 4100)
