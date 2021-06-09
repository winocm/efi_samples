/*-----------------------------------------------------------------------
 *      File:   smvl_do.c
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

/*-----------------------------------------------------------------------------
 * Name: vl_FreeAttributes
 *
 * Description: This function frees attributes in CSSM_VL_ATTRIBUTE_GROUP
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * AttributeGrp (input) - struct containing attributes to be freed
 *
 * Return value: 
 * NONE
 * 
 * Error Codes: 
 * NONE 
 *---------------------------------------------------------------------------*/
static void 
vl_FreeAttributes(
    CSSM_VL_HANDLE VLHandle,
    CSSM_VL_ATTRIBUTE_GROUP AttributeGrp)
{
    CSSM_VL_ATTRIBUTE_PTR AttributePtr; 
    uint32 i;

    AttributePtr = AttributeGrp.Attributes;
    if (AttributePtr == NULL) return;

    for(i=0; i < AttributeGrp.NumberOfAttributes; i++)
    {
        g_fx.free_func(VLHandle, AttributePtr[i].Info.Label.Name.Data);
        g_fx.free_func(VLHandle, AttributePtr[i].Value.Data);
    }
    g_fx.free_func(VLHandle, AttributePtr);
    return;
}

/*-----------------------------------------------------------------------------
 * Name: smvl_SetAttributes
 *
 * Description: This function sets the attributes CSSM_VL_ATTRIBUTE_GROUP_PTR
 * specified in ISL_SECTION_PTR
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * AttributeGrpPtr (output) - struct to be filled with attributes
 * SectionPtr (input) - contains attributes in archive specific encoding
 *
 * Return value:
 * CSSM_OK:
 * CSSM_FAIL:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static CSSM_RETURN
smvl_SetAttributes(
    CSSM_VL_HANDLE VLHandle,
    CSSM_VL_ATTRIBUTE_GROUP_PTR vlAttrGrpPtr,
    ISL_ATTRIBUTE_GROUP_PTR islAttrGrpPtr)
{
    CSSM_VL_ATTRIBUTE_PTR AttributePtr = NULL;
    ISL_NAME_VALUE NameValue;
    uint32 i;

    if (vlAttrGrpPtr == NULL ||
        islAttrGrpPtr == NULL)
    {
        return CSSM_FAIL;
    }

    {
        cssm_memset(vlAttrGrpPtr, 0, sizeof(CSSM_VL_ATTRIBUTE_GROUP_PTR));
        if (islAttrGrpPtr->NumberOfAttributes == 0) return CSSM_OK;
        vlAttrGrpPtr->NumberOfAttributes = islAttrGrpPtr->NumberOfAttributes;
        AttributePtr = g_fx.calloc_func(
                        VLHandle, 
                        vlAttrGrpPtr->NumberOfAttributes,
                        sizeof(CSSM_VL_ATTRIBUTE)); 
        if (AttributePtr == NULL)
        {
            return CSSM_FAIL;
        }

        for(i=0; i < vlAttrGrpPtr->NumberOfAttributes; i++)
        {
            NameValue = islAttrGrpPtr->AttributesPtr[i];
            AttributePtr[i].Info.AttributeFormat = 
                CSSM_DB_ATTRIBUTE_FORMAT_BLOB;
            AttributePtr[i].Info.AttributeNameFormat = 
                CSSM_DB_ATTRIBUTE_NAME_AS_BLOB;

            AttributePtr[i].Info.Label.Name.Length = NameValue.Name.Length;
            AttributePtr[i].Info.Label.Name.Data = g_fx.malloc_func(
                VLHandle,
                AttributePtr[i].Info.Label.Name.Length);
            if (AttributePtr[i].Info.Label.Name.Data == NULL) goto FAIL;
            cssm_memcpy(AttributePtr[i].Info.Label.Name.Data,
                        NameValue.Name.Data,
                        AttributePtr[i].Info.Label.Name.Length);
        
            AttributePtr[i].Value.Length = NameValue.Value.Length;
            AttributePtr[i].Value.Data = g_fx.malloc_func(
                VLHandle, 
                AttributePtr[i].Value.Length);

            if (AttributePtr[i].Value.Data == NULL) goto FAIL;
            cssm_memcpy(AttributePtr[i].Value.Data,
                        NameValue.Value.Data,
                        AttributePtr[i].Value.Length);
        }

        vlAttrGrpPtr->Attributes = AttributePtr;
        return CSSM_OK;
    }

FAIL:
    {
        if (AttributePtr == NULL) return CSSM_FAIL;

        for(i=0; i < vlAttrGrpPtr->NumberOfAttributes; i++)
        {
            g_fx.free_func(VLHandle, AttributePtr[i].Info.Label.Name.Data);
            g_fx.free_func(VLHandle, AttributePtr[i].Value.Data);
        }
        g_fx.free_func(VLHandle,  AttributePtr);
        return CSSM_FAIL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: VL_GetDoInfoByName
 *
 * Description: Returns DoInfo referenced by Name in the VO
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * VOHandle (input) - a handle to an instantiate VO
 * JoinName (input) - name of Do to search for
 *
 * Return value:
 * CSSM_VL_DO_INFO_PTR: The requested DoInfo
 * NULL: unable to return the DoInfo
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_VL_DO_INFO_PTR CSSMVLI VL_GetDoInfoByName
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_STRING JoinName)
{
    CSSM_VL_DO_INFO_PTR DoInfoPtr;
    SMVL_VO_CONTEXT_PTR VoContextPtr;
    uint32 error;

    
    VoContextPtr = smvl_GetVoContext(VOHandle);
    if (VoContextPtr == NULL)
    { 
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_INVALID_VO_HANDLE);
        return NULL;
    }

    DoInfoPtr = NULL;
    {
        ISL_SIG_SECTION_LIST_PTR SignedListPtr;
        ISL_SIG_SECTION_PTR SigSectionPtr;
        ISL_CONST_DATA Name;

        SignedListPtr = VoContextPtr->SignedListPtr;
        Name.Data = (const uint8 *)JoinName;
        Name.Length = cssm_strlen(JoinName);

        SigSectionPtr = SignedListMethods.FindSignedObject(
            SignedListPtr, 
            Name);
        if (SigSectionPtr == NULL)
        {
            error = CSSM_VL_UNKNOWN_NAME;
            goto FAIL;
        }

        DoInfoPtr = g_fx.calloc_func(VLHandle, sizeof(CSSM_VL_DO_INFO), 1);
        if (DoInfoPtr == NULL)
        {
            error = CSSM_MEMORY_ERROR;
            goto FAIL;
        }

        /* Initialize the DoInfo struct */
        cssm_memcpy(DoInfoPtr->JoinName, JoinName, sizeof(CSSM_STRING));
        DoInfoPtr->VoHandle = VOHandle;
        DoInfoPtr->DoType = CSSM_VL_DO_TYPE_BLOB_BYTE_DATA_UNKNOWN_FORMAT;

        /* Initialize Vo Specific Attributes */
        {
            ISL_ATTRIBUTE_GROUP_PTR AttributeGrpPtr;
            CSSM_RETURN AttributesSet;

            AttributeGrpPtr = SignedObjectMethods.GetAttributeGroup(
                SigSectionPtr);
            if (AttributeGrpPtr == NULL)
            {
                error = CSSM_MEMORY_ERROR;
                goto FAIL;
            }
            AttributesSet = smvl_SetAttributes(
                VLHandle,
                &DoInfoPtr->VoSpecificAttributes,
                AttributeGrpPtr);

            SignedObjectMethods.FreeAttributeGroup(
                SigSectionPtr,
                AttributeGrpPtr);

            if (CSSM_OK != AttributesSet)
            {
                error = CSSM_MEMORY_ERROR;
                goto FAIL;
            }
        }

        /* Initialize Bundle Wide Attributes */
        {
            ISL_ATTRIBUTE_GROUP_PTR AttributeGrpPtr;
            ISL_MANIFEST_SECTION_PTR ManifestSectionPtr;
            CSSM_RETURN AttributesSet;


            ManifestSectionPtr = SignedObjectMethods.GetManifestSection(
                SigSectionPtr);
            if (ManifestSectionPtr == NULL)
            {
                error = CSSM_MEMORY_ERROR;
                goto FAIL;
            }
            AttributeGrpPtr = ManifestSectionMethods.GetAttributeGroup(
                ManifestSectionPtr);
            if (AttributeGrpPtr == NULL)
            {
                error = CSSM_MEMORY_ERROR;
                goto FAIL;
            }
            AttributesSet = smvl_SetAttributes(
                VLHandle,
                &DoInfoPtr->BundleWideAttributes,
                AttributeGrpPtr);
             ManifestSectionMethods.FreeAttributeGroup(
                ManifestSectionPtr,
                AttributeGrpPtr);
            if ( CSSM_OK != AttributesSet)
            {
                error = CSSM_MEMORY_ERROR;
                goto FAIL;
            }
        }

        CSSM_ClearError();
        return DoInfoPtr;
    }

FAIL:
    {
        if (DoInfoPtr)
        {
            vl_FreeAttributes(VLHandle, DoInfoPtr->VoSpecificAttributes);
            vl_FreeAttributes(VLHandle, DoInfoPtr->BundleWideAttributes);
            g_fx.free_func(VLHandle, DoInfoPtr);
        }
        CSSM_SetError(&intel_preos_vlm_guid, error);
        return NULL;        
    }
}

/*-----------------------------------------------------------------------------
 * Name: VL_FreeDoInfos
 *
 * Description: Frees an array of CSSM_VL_DO_INFOs and substructures
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * DoInfos (input)
 * NumberOfDoInfs (input)
 *
 * Return value:
 * CSSM_OK: 
 * CSSM_FAIL:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMVLI VL_FreeDoInfos  (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_DO_INFO_PTR DoInfos,
                                     uint32 NumberOfDoInfos)
{
    uint32 i;
    CSSM_VL_DO_INFO_PTR DoInfoPtr;

    if (DoInfos == NULL ||
        NumberOfDoInfos == 0)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_MEMORY_ERROR);
        return CSSM_FAIL;
    }

    for(i=0; i < NumberOfDoInfos; i++)
    {
        DoInfoPtr = DoInfos + i;
        vl_FreeAttributes(VLHandle, DoInfoPtr->BundleWideAttributes);
        vl_FreeAttributes(VLHandle, DoInfoPtr->VoSpecificAttributes);
    }

    g_fx.free_func(VLHandle, DoInfos);
    CSSM_ClearError();
    return CSSM_OK;
}
