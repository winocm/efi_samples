/*-----------------------------------------------------------------------
 *      File:   smvl.h
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

#ifndef _SMVL_H
#define _SMVL_H

/* Windows & CSSM includes */
#include "cssm.h"
#include "cssmport.h"
#include "integapi.h"

#define CSSMVLI CSSMAPI

typedef struct smvl_context {
    ISL_CONFIG_PTR SMConfig;
    ISL_CONFIG_PTR PKCS7Config;
    ISL_SIGNATURE_METHODS_PTR PKCS7SignatureMethods;
    ISL_DIGEST_METHODS_PTR JarSHA1Methods;
    ISL_SIGN_VERIFY_METHODS_PTR PKCS7DSAMethods;
    ISL_CLASS_PTR PKCS7SignatureClass;
    CSSM_MODULE_HANDLE CSPHandle;
} SMVL_CONTEXT, *SMVL_CONTEXT_PTR;

typedef struct smvl_vo_context {
    CSSM_VO_UID VOIdentifier;
    CSSM_STRING VOName;
    CSSM_VOBUNDLE_UID VoBundleIdentifier;
    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr;
    ISL_SIGNATURE_CONTEXT_PTR SignaturePtr;
    ISL_SIG_SECTION_LIST_PTR SignedListPtr;
} SMVL_VO_CONTEXT, * SMVL_VO_CONTEXT_PTR;


#ifndef IS_EQUAL
#define IS_EQUAL(x,y) \
( ((x).Length == (y).Length) && \
  (!cssm_memcmp((x).Data, (y).Data, (y).Length) )
#endif

/* VL Global Data Structures */

extern SMVL_CONTEXT_PTR gsContextPtr;
extern ISL_MEMORY_METHODS gISL_mem_funcs;
extern CSSM_SPI_MEMORY_FUNCS  g_fx;

extern struct isl_archive_methods ArchiveMethods;
extern struct isl_manifest_section_methods ManifestSectionMethods;
extern struct isl_config_methods ArchiveConfigMethods;
extern struct isl_signature_methods PKCS7SignatureMethods;
extern struct isl_certificate_methods X509CertMethods; 
extern struct isl_signer_methods PKCS7SignerMethods;
extern ISL_SIG_SECTION_METHODS SignedListMethods;
extern ISL_SIGNED_SECTION_METHODS SignedObjectMethods;

extern ISL_GET_DATA_METHODS getCSSMDataMethods;
extern ISL_CONFIG_METHODS PKCS7SignatureConfigMethods;
extern ISL_DIGEST_METHODS JarSHA1Methods;
extern ISL_SIGN_VERIFY_METHODS PKCS7DSAMethods;
extern ISL_GET_DATA_METHODS getMemoryMethods;


#ifdef __cplusplus
extern "C" {
#endif
#if 0
CSSM_RETURN
smvl_FindAttributeInHeader(
    ISL_HEADER_SECTION_PTR SectionPtr,
    ISL_CONST_DATA AttributeName,
    ISL_CONST_DATA_PTR AttributeValuePtr);

CSSM_RETURN
smvl_GetIdentifier(
    ISL_HEADER_SECTION_PTR SectionPtr,
    ISL_CONST_DATA IdentifierName,
    CSSM_GUID_PTR IdentifierPtr);
#endif

SMVL_VO_CONTEXT_PTR
smvl_GetVoContext(
    CSSM_VO_HANDLE VOHandle);

CSSM_VL_VOBUNDLE_REGISTRY_INFO_PTR CSSMVLI VL_ImportVoBundle
            (CSSM_VL_HANDLE VLHandle,
             const CSSM_VL_LOCATION_PTR VOBundleLocation);

CSSM_VO_HANDLE CSSMVLI VL_InstantiateVoFromLocation
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VL_LOCATION_PTR VOBundleLocation,
             CSSM_VO_UID_PTR VOIdentifier,
             CSSM_STRING VOName);

CSSM_VO_HANDLE CSSMVLI VL_InstantiateVo
            (CSSM_VL_HANDLE VLHandle,
             const CSSM_VL_LOCATION_PTR VOBundleLocation,
             const CSSM_VO_UID_PTR VOIdentifier,
             uint32 MaxDepth);

CSSM_RETURN CSSMVLI VL_FreeVo  
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle);

CSSM_VO_UID_BINDING_GROUP_PTR CSSMVLI VL_GetNestedVoHandles
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle);

CSSM_RETURN CSSMVLI VL_FreeVoHandles
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_UID_BINDING_GROUP_PTR VoHandles);

CSSM_VL_ATTRIBUTE_GROUP_PTR CSSMVLI VL_GetRootVoAttributes
            (CSSM_VL_HANDLE VLHandle, 
             CSSM_VO_HANDLE VOHandle);

CSSM_RETURN CSSMVLI VL_FreeVoAttributes
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VL_ATTRIBUTE_GROUP_PTR VoAttributes);

CSSM_VL_DO_INFO_PTR CSSMVLI VL_GetDoInfoByName
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle,
             const CSSM_STRING JoinName);

CSSM_VL_DO_INFO_PTR CSSMVLI VL_GetRootDoInfos
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle,
             uint32 *NumberOfDoInfos);

CSSM_VL_DO_INFO_PTR CSSMVLI VL_GetInstantiatedDoInfos
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle,
             uint32 *NumberOfDoInfos);

CSSM_RETURN CSSMVLI VL_FreeDoInfos
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VL_DO_INFO_PTR DoInfos,
             uint32 NumberOfDoInfos);

CSSM_VL_SIGNATURE_INFO_PTR CSSMVLI VL_GetFirstSignatureInfo
            (CSSM_VL_HANDLE VLHandle, 
             CSSM_VO_HANDLE VOHandle,
             CSSM_HANDLE *SignerIteratorHandle);

CSSM_VL_SIGNATURE_INFO_PTR CSSMVLI VL_GetNextSignatureInfo
            (CSSM_VL_HANDLE VLHandle, 
             CSSM_HANDLE SignerIteratorHandle,
             CSSM_BOOL *NoMoreSigners);

CSSM_RETURN CSSMVLI VL_AbortScan
            (CSSM_VL_HANDLE VLHandle, 
             CSSM_HANDLE IteratorHandle);

CSSM_RETURN CSSMVLI VL_FreeSignatureInfo
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VL_SIGNATURE_INFO_PTR SignatureInfo);

CSSM_CERTGROUP_PTR CSSMVLI VL_GetSignerCertificateGroup
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle,
             const CSSM_DATA_PTR SignerCert);

CSSM_VL_DO_LMAP_PTR CSSMVLI VL_GetRootDoLocationMap
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle);

CSSM_VL_DO_LMAP_PTR CSSMVLI VL_GetInstantiatedDoLocationMap
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle);

CSSM_RETURN CSSMVLI VL_SetDoLMapEntries
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle,
             const CSSM_VL_DO_LMAP_PTR NewLocationEntries);

CSSM_RETURN CSSMVLI VL_ArchiveDoLMap
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle);

CSSM_RETURN CSSMVLI VL_FreeDoLMap
            (CSSM_VL_HANDLE VLHandle,
             const CSSM_VL_DO_LMAP_PTR DoLMap);

CSSM_VL_VERIFICATION_HANDLE CSSMVLI VL_VerifyRootCredentialsDataAndContainment
            (CSSM_VL_HANDLE VLHandle,
             CSSM_VO_HANDLE VOHandle,
             const CSSM_DATA_PTR SignerCertificate,
             uint32 NumberOfPreferredCsps, 
             const CSSM_VL_PREFERRED_CSP_PTR PreferredCSPs,
             uint32 NumberOfContainments,
             const CSSM_VL_DO_CONTAINMENT_LOCATION_PTR ContainmentsToVerify);


uint32 CSSMVLI VL_SelfVerifyCertificate(
                CSSM_VL_HANDLE       VLHandle,
                const CSSM_DATA_PTR  Certificate
                );
                
                
#ifdef __cplusplus
}
#endif


#endif /* _SMVL_H */
