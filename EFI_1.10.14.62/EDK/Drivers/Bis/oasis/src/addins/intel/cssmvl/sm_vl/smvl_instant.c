/*-----------------------------------------------------------------------
 *      File:   smvl_instant.c
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
#include "cssmport.h"
#include "guids.h"
    /* VL-specific includes */
#include "smvl.h"
#include "islutil.h"

extern CSSM_SPI_MEMORY_FUNCS  g_fx;

#define VO_IDENTIFIER_STR "SignerInformationPersistentId"
#define VO_NAME_STR "SignerInformationName"

#pragma warning (disable: 4100 4057)


/*-----------------------------------------------------------------------------
 * Name: GetVOInfo
 *
 * Description:
 *
 * Parameters:
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_RETURN
GetVOInfo(
    ISL_SIG_SECTION_LIST_PTR SignedListPtr,
    SMVL_VO_CONTEXT_PTR VoContextPtr)
{

    if (VoContextPtr == NULL) return CSSM_FAIL;
    {   
        ISL_CONST_DATA AttributeName;
        ISL_CONST_DATA AttributeValue;

        CSSM_RETURN ret;

        AttributeName.Data = VO_IDENTIFIER_STR;
        AttributeName.Length = sizeof(VO_IDENTIFIER_STR) - 1;
        ret = SignedListMethods.FindAttributeValue(
            SignedListPtr, 
            AttributeName,
            &AttributeValue);
        if (ret != CSSM_OK)
        {
            /* Add code to handle legacy manifests */
            goto FAIL;
        }

        AttributeName.Data = VO_NAME_STR;
        AttributeName.Length = sizeof(VO_NAME_STR) - 1;
        ret = SignedListMethods.FindAttributeValue(
            SignedListPtr, 
            AttributeName, 
            &AttributeValue);
        if (ret != CSSM_OK)
        {
            /* Add code to handle legacy manifests */
            goto FAIL;
        }
        if (AttributeValue.Length > sizeof(CSSM_STRING) - 1) goto FAIL;
        cssm_memcpy(
            VoContextPtr->VOName,
            AttributeValue.Data,
            AttributeValue.Length);
        VoContextPtr->VOName[AttributeValue.Length] = 0;

        return(CSSM_OK);
    }

FAIL:
    {
        return CSSM_FAIL;
    }
}
#pragma warning (default: 4100)


/*-----------------------------------------------------------------------------
 * Name: VL_FreeVo
 *
 * Description:
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * VOHandle (input) - a handle to an instantiate VO
 *
 * Return value:
 * 
 * CSSM_OK:
 * CSSM_FAIL:
 *
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMVLI VL_FreeVo  (CSSM_VL_HANDLE VLHandle,
                                CSSM_VO_HANDLE VOHandle)
{
    SMVL_VO_CONTEXT_PTR VoContextPtr;
    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr;

    VoContextPtr = (SMVL_VO_CONTEXT_PTR) VOHandle;
    if (VoContextPtr == NULL)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_INVALID_VO_HANDLE);
        return CSSM_FAIL;
    }

    ArchivePtr = VoContextPtr->ArchivePtr;
    if (ISL_OK != ArchiveMethods.Recycle(ArchivePtr))
    {
        return CSSM_FAIL;
    }

    g_fx.free_func(VLHandle, ArchivePtr);
    g_fx.free_func(VLHandle, VoContextPtr);
    CSSM_ClearError();
    return CSSM_OK;
}

/*-----------------------------------------------------------------------------
 * Name: VL_InstantiateVoFromLocation
 *
 * Description:
 * Instantiates a VO from a bundle. The bundle is specified by location. The
 * VO is specified either by VOIdentifier or VOName.
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * VOBundleLocation (input) - a handle to an instantiate VO
 * VOIdentifier (input/optional) - which VO in the bundle to instantiate
 * VOName (input/optional) - which VO in the bundle to instantiate
 *
 * Return value:
 * CSSM_VO_HANDLE - if VO successfully instantiated
 * CSSM_INVALID_HANDLE - otherwise
 * 
 * CSSM_OK:
 * CSSM_FAIL:
 *
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_VO_HANDLE CSSMVLI VL_InstantiateVoFromLocation
    (CSSM_VL_HANDLE VLHandle,
     CSSM_VL_LOCATION_PTR VOBundleLocation,
     CSSM_VO_UID_PTR VOIdentifier,
     CSSM_STRING VOName)
{
    SMVL_VO_CONTEXT_PTR VoContextPtr;

    if (VOIdentifier == NULL &&
        VOName == NULL)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_INVALID_POINTER);
        return (&intel_preos_vlm_guid, CSSM_INVALID_HANDLE);
    }

    if (VOBundleLocation->MediaType != CSSM_VL_MEDIA_TYPE_MEMORY)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_INVALID_POINTER);
        return (&intel_preos_vlm_guid, CSSM_INVALID_HANDLE);
    }

    VoContextPtr = NULL;
    {
        ISL_ARCHIVE_CONTEXT_PTR ArchivePtr;
        ISL_CONST_DATA ptr;
        ISL_SIZE size;
        uint32 error;

        VoContextPtr = g_fx.calloc_func(VLHandle, 1, sizeof(SMVL_VO_CONTEXT));
        if (VoContextPtr == NULL) 
        {
            CSSM_SetError(&intel_preos_vlm_guid, CSSM_MEMORY_ERROR);
            goto FAIL;
        }

        ptr.Data = VOBundleLocation->Location.MemoryRef.Data;
        ptr.Length = VOBundleLocation->Location.MemoryRef.Length;
        size = ArchiveMethods.SizeofObject();
        ArchivePtr = g_fx.malloc_func(VLHandle, (uint32)size);
        if (ArchivePtr == NULL)
        {
            CSSM_SetError(&intel_preos_vlm_guid, CSSM_MEMORY_ERROR);
            goto FAIL;
        }

        VoContextPtr->ArchivePtr = ArchivePtr;
        if (ISL_OK != ArchiveMethods.InitializeOld(
                        ArchivePtr,
                        gsContextPtr->SMConfig,
                        ptr))
        {
            error = ISL_GetError();
            error = (error == 0) ? CSSM_VL_INVALID_BUNDLE : error;
            CSSM_SetError(&intel_preos_vlm_guid, error);
            goto FAIL;
        }

        {
            CSSM_BOOL SignatureFound = CSSM_FALSE;
            ISL_SIGNATURE_INFO_GROUP_PTR SigGroupPtr;
            uint32 SigIndex;
            const uint32 VONameLength = cssm_strlen(VOName);

            SigGroupPtr = ArchiveMethods.GetSignatureGroup(ArchivePtr);
            if (SigGroupPtr == NULL)
            {
                error = ISL_GetError();
                error = (error == 0) ? CSSM_VL_INVALID_BUNDLE : error;
                CSSM_SetError(&intel_preos_vlm_guid, error);
                goto FAIL;
            }

            for(SigIndex = 0; 
                SigIndex < SigGroupPtr->NumberOfSignatures; 
                SigIndex++)
            {
                ISL_SIGNATURE_INFO SignatureInfo;

                SignatureInfo = SigGroupPtr->SignatureInfoPtr[SigIndex];
                if (GetVOInfo(
                        SignatureInfo.SignedListPtr, 
                        VoContextPtr) != CSSM_OK)
                {
                    CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_INVALID_BUNDLE);
                    goto FAIL;
                }

                 if (VOIdentifier != NULL)
                 {
                     if (cssm_memcmp(
                                VOIdentifier, 
                                &VoContextPtr->VOIdentifier, 
                                sizeof(CSSM_VO_UID) == 0))
                     {
                         VoContextPtr->SignaturePtr = SignatureInfo.SignaturePtr;
                         VoContextPtr->SignedListPtr = SignatureInfo.SignedListPtr;
                         SignatureFound = CSSM_TRUE;
                         break;
                     }
                 }
                 else
                 {
                     if (VONameLength == cssm_strlen(VoContextPtr->VOName) &&
                         cssm_memcmp(
                            VOName, 
                            VoContextPtr->VOName,
                            VONameLength) == 0)
                     {
                         VoContextPtr->SignaturePtr = SignatureInfo.SignaturePtr;
                         VoContextPtr->SignedListPtr = SignatureInfo.SignedListPtr;
                         SignatureFound = CSSM_TRUE;
                         break;
                     }
                 }
            }
            if (SignatureFound != CSSM_TRUE)
            {
                CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_NOT_IN_BUNDLE);
                goto FAIL;
            }

            return (CSSM_VO_HANDLE) VoContextPtr;
        }

    }

FAIL:
    {
        if (VoContextPtr)
        {
            ArchiveMethods.Recycle(VoContextPtr->ArchivePtr);
            g_fx.free_func(VLHandle, VoContextPtr->ArchivePtr);
        }
        g_fx.free_func(VLHandle, VoContextPtr);
        return CSSM_INVALID_HANDLE;
    }
}
