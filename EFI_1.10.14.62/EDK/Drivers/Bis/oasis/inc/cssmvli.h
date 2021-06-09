/*-----------------------------------------------------------------------
 *      File:   CSSMVLI.H
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

#ifndef _CSSMVLI_H
#define _CSSMVLI_H    

#include "cssmtype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cssm_spi_vl_funcs {
#ifndef CSSM_BIS
    CSSM_VL_VOBUNDLE_REGISTRY_INFO_PTR (CSSMAPI *ImportVoBundle)
                                    (CSSM_VL_HANDLE VLHandle,
                                     const CSSM_VL_LOCATION_PTR VoBundleLocation);
    CSSM_VO_HANDLE (CSSMAPI *InstantiateVo)
                                    (CSSM_VL_HANDLE VLHandle,
                                     const CSSM_VL_LOCATION_PTR VoBundleLocation,
                                     const CSSM_VO_UID_PTR VoIdentifier,
                                     uint32 MaxDepth);
#endif
    CSSM_VO_HANDLE (CSSMAPI *InstantiateVoFromLocation)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_LOCATION_PTR VoBundleLocation,
                                     CSSM_VO_UID_PTR VoIdentifier,
                                     CSSM_STRING VoName);
    CSSM_RETURN (CSSMAPI *FreeVo)   (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle);
#ifndef CSSM_BIS
    CSSM_VO_UID_BINDING_GROUP_PTR (CSSMAPI *GetNestedVoHandles)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle);
    CSSM_RETURN (CSSMAPI *FreeVoHandles)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_UID_BINDING_GROUP_PTR VoHandles);
    CSSM_VL_ATTRIBUTE_GROUP_PTR (CSSMAPI *GetRootVoAttributes)
                                    (CSSM_VL_HANDLE VLHandle, 
                                     CSSM_VO_HANDLE VOHandle);
    CSSM_RETURN (CSSMAPI *FreeVoAttributes)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_ATTRIBUTE_GROUP_PTR VoAttributes);
#endif
    CSSM_VL_DO_INFO_PTR (CSSMAPI *GetDoInfoByName)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_STRING JoinName);
#ifndef CSSM_BIS
    CSSM_VL_DO_INFO_PTR (CSSMAPI *GetRootDoInfos)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     uint32 *NumberOfDoInfos);
    CSSM_VL_DO_INFO_PTR (CSSMAPI *GetInstantiatedDoInfos)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     uint32 *NumberOfDoInfos);
#endif
    CSSM_RETURN (CSSMAPI *FreeDoInfos)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_DO_INFO_PTR DoInfos,
                                     uint32 NumberOfDoInfos);
    CSSM_VL_SIGNATURE_INFO_PTR (CSSMAPI *GetFirstSignatureInfo)
                                    (CSSM_VL_HANDLE VLHandle, 
                                     CSSM_VO_HANDLE VOHandle,
                                     CSSM_HANDLE *SignerIteratorHandle);
#ifndef CSSM_BIS
    CSSM_VL_SIGNATURE_INFO_PTR (CSSMAPI *GetNextSignatureInfo)
                                    (CSSM_VL_HANDLE VLHandle, 
                                     CSSM_HANDLE SignerIteratorHandle,
                                     CSSM_BOOL *NoMoreSigners);
#endif
    CSSM_RETURN (CSSMAPI *AbortScan)(CSSM_VL_HANDLE VLHandle, 
                                     CSSM_HANDLE IteratorHandle);
    CSSM_RETURN (CSSMAPI *FreeSignatureInfo)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_SIGNATURE_INFO_PTR SignatureInfo);
#ifndef CSSM_BIS
    CSSM_CERTGROUP_PTR (CSSMAPI *GetSignerCertificateGroup)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_DATA_PTR SignerCert);
    CSSM_VL_DO_LMAP_PTR (CSSMAPI *GetRootDoLocationMap)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle);
    CSSM_VL_DO_LMAP_PTR (CSSMAPI *GetInstantiatedDoLocationMap)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle);
    CSSM_RETURN (CSSMAPI *FreeDoLMap)
                                    (CSSM_VL_HANDLE VLHandle,
                                     const CSSM_VL_DO_LMAP_PTR DoLMap);
#endif
    CSSM_RETURN (CSSMAPI *SetDoLMapEntries)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_VL_DO_LMAP_PTR NewLocationEntries);
#ifndef CSSM_BIS
    CSSM_RETURN (CSSMAPI *ArchiveDoLMap)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle);
    CSSM_VL_VERIFICATION_HANDLE (CSSMAPI *VerifyRootCredentials)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_DATA_PTR SignerCertificate,
                                     uint32 NumberOfPreferredCsps, 
                                     const CSSM_VL_PREFERRED_CSP_PTR PreferredCSPs);
#endif
    CSSM_VL_VERIFICATION_HANDLE (CSSMAPI *VerifyRootCredentialsDataAndContainment)
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_DATA_PTR SignerCertificate,
                                     uint32 NumberOfPreferredCsps, 
                                     const CSSM_VL_PREFERRED_CSP_PTR PreferredCSPs,
                                     uint32 NumberOfContainments,
                                     const CSSM_VL_DO_CONTAINMENT_LOCATION_PTR ContainmentsToVerify);


    uint32 (CSSMAPI *SelfVerifyCertificate)
                (
                CSSM_VL_HANDLE       VLHandle,
                const CSSM_DATA_PTR  Certificate
                );
        
} CSSM_SPI_VL_FUNCS, *CSSM_SPI_VL_FUNCS_PTR;

#ifdef __cplusplus
}
#endif

#endif
