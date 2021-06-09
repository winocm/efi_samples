/*-----------------------------------------------------------------------
 *      File:   smvl_domap.c
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
#include "islutil.h"


/* VL Global Data Structures */
#pragma warning (disable : 4100 4057 4102)

static const CSSM_VOBUNDLE_UID nullIdentifier = 
{0x00, 0x00, 0x00, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};

#define VOBUNDLE_IDENTIFIER_STR "ManifestPersistentId"

/*-----------------------------------------------------------------------------
 * Name: smvl_GetVoBundleIdentifier
 *
 * Description: Retreives the persistent uids for the archive
 *
 * Parameters:
 * ArchivePtr (input)
 * VoBundleIdentifierPtr (output)
 *
 * Return value:
 * CSSM_OK: 
 * CSSM_FAIL:
 * 
 * Error Codes:
 *
 * Notes:
 * At some point we need to deal with legacy manifests. My best idea is to
 * hash the manifest file and convert that value to a uid.
 *---------------------------------------------------------------------------*/
CSSM_RETURN
smvl_GetVoBundleIdentifier(
    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr, 
    CSSM_VOBUNDLE_UID_PTR VoBundleIdentifierPtr)
{
    ISL_CONST_DATA AttributeName;
    ISL_CONST_DATA AttributeValue;
    ISL_STATUS AttributeFound;

    if (ArchivePtr == NULL ||
        VoBundleIdentifierPtr == NULL)
    {
        return CSSM_FAIL;
    }

    AttributeName.Data = VOBUNDLE_IDENTIFIER_STR;
    AttributeName.Length = sizeof(VOBUNDLE_IDENTIFIER_STR) - 1;

    AttributeFound = ArchiveMethods.FindAttributeValue(
        ArchivePtr,
        AttributeName,
        &AttributeValue);

    if (AttributeFound != ISL_OK) return CSSM_FAIL;

    return CSSM_OK;
}


/*-----------------------------------------------------------------------------
 * Name: smvl_LocationToServiceParameter
 *
 * Description: This function converts a CSSM_VL_LOCATION to SM specific
 * service and parameters encoding
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * Location (input) - CSSM_VL_LOCATION to convert
 * GetServicePtr (output) - SM specific getdata method
 * GetParametersPtr (output) - SM specific parameters encoding
 *
 * Return value:
 * CSSM_OK:
 * CSSM_FAIL:
 * 
 * Error Codes:
 *
 *---------------------------------------------------------------------------*/
CSSM_RETURN
smvl_LocationToServiceParameter(
    CSSM_VL_HANDLE VLHandle,
    CSSM_VL_LOCATION Location,
    ISL_GET_DATA_METHODS_PTR *GetServicePtr,
    ISL_CONST_DATA_PTR GetParametersPtr)
{
    ISL_CONST_DATA_PTR TempParamsPtr = NULL;

    if (GetServicePtr == NULL ||
        GetParametersPtr == NULL)
    {
        return CSSM_FAIL;
    }

    *GetServicePtr = NULL;
    cssm_memset(GetParametersPtr, 0, sizeof(ISL_CONST_DATA));

    switch(Location.MediaType)
    {
    case CSSM_VL_MEDIA_TYPE_MEMORY:
        *GetServicePtr = &getMemoryMethods;

        /*  NO API TO SERIALIZE SO NO NEED TO ENCODE PARAMETERS 
            OTHERWISE, NEED TO USE THE CLASS CONTEXT TO PASS
            APPLICATION SPECIFIC DATA */
        TempParamsPtr = g_fx.malloc_func(VLHandle, sizeof(ISL_CONST_DATA));
        if (TempParamsPtr == NULL) goto FAIL;
        GetParametersPtr->Data = (uint8 *)TempParamsPtr;
        GetParametersPtr->Length = sizeof(ISL_CONST_DATA);
        TempParamsPtr->Data = Location.Location.MemoryRef.Data;
        TempParamsPtr->Length = Location.Location.MemoryRef.Length;
        break;

    default:
        return CSSM_FAIL;
    }
    return CSSM_OK;

FAIL:
    {
        g_fx.free_func(VLHandle, (void *)GetParametersPtr->Data);
        return CSSM_FAIL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: smvl_LocationToServiceParameter
 *
 * Description: This function converts a CSSM_VL_LOCATION to SM specific
 * service and parameters encoding
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * Location (input) - CSSM_VL_LOCATION to convert
 * GetServicePtr (output) - SM specific getdata method
 * GetParametersPtr (output) - SM specific parameters encoding
 *
 * Return value:
 * CSSM_OK:
 * CSSM_FAIL:
 * 
 * Error Codes:
 *
 *---------------------------------------------------------------------------*/
void
smvl_FreeLocationToServiceParameter(
    CSSM_VL_HANDLE VLHandle,
    CSSM_VL_LOCATION Location,
    ISL_GET_DATA_METHODS_PTR GetServicePtr,
    ISL_CONST_DATA GetParameters)
{
    switch(Location.MediaType)
    {
    case CSSM_VL_MEDIA_TYPE_MEMORY:
        GetServicePtr;
        g_fx.free_func(VLHandle, (void *)GetParameters.Data);
    default:
        return;
    }
    return;
}

/*-----------------------------------------------------------------------------
 * Name: VL_SetDoLMapEntries
 *
 * Description:
 *
 * Parameters:
 * VLHandle (input) - VL handle is used for up calls to memory functions
 * VOHandle (input) - a handle to an instantiate VO
 * NewLocationEntries(input)
 *
 * Return value:
 * CSSM_OK:
 * CSSM_FAIL:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMVLI VL_SetDoLMapEntries
                                (CSSM_VL_HANDLE VLHandle,
                                 CSSM_VO_HANDLE VOHandle,
                                 const CSSM_VL_DO_LMAP_PTR NewLocationEntries)
{
    SMVL_VO_CONTEXT_PTR VoContextPtr;
    ISL_LMAP_ENTRY_PTR MapEntryPtr = NULL;
    CSSM_RETURN ret = CSSM_FAIL;

    VoContextPtr = smvl_GetVoContext(VOHandle);
    if (VoContextPtr == NULL)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_INVALID_VO_HANDLE);    
        return CSSM_FAIL;
    }

    if (NewLocationEntries == NULL)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_INVALID_POINTER); 
        return CSSM_FAIL;
    }

    {
        ISL_ARCHIVE_CONTEXT_PTR ArchivePtr;
        ISL_LMAP UpdateMap;
        uint32 i;
        CSSM_VOBUNDLE_UID VoBundleIdentifier;

        ArchivePtr = VoContextPtr->ArchivePtr;
        MapEntryPtr = g_fx.calloc_func(
            VLHandle,
            NewLocationEntries->NumberOfMapEntries,
            sizeof(ISL_LMAP_ENTRY));

        if (MapEntryPtr == NULL)
        {
            CSSM_SetError(&intel_preos_vlm_guid, CSSM_MEMORY_ERROR);
            return CSSM_FAIL;
        }

        if (CSSM_OK != smvl_GetVoBundleIdentifier(
                           ArchivePtr, 
                           &VoBundleIdentifier))
        {
            CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_INVALID_BUNDLE);
            goto EXIT;
        }

        for (i=0; i < NewLocationEntries->NumberOfMapEntries; i++)
        {
            if (cssm_memcmp(
                    &NewLocationEntries->MapEntries[i].VoBundleIdentifier,
                    &VoBundleIdentifier,
                    sizeof(CSSM_VOBUNDLE_UID)) != 0 &&
                cssm_memcmp(
                    &NewLocationEntries->MapEntries[i].VoBundleIdentifier,
                    &nullIdentifier,
                    sizeof(CSSM_VOBUNDLE_UID)) != 0)
            {
                CSSM_SetError(&intel_preos_vlm_guid, CSSM_VL_INVALID_BUNDLE);
                goto EXIT;
            }

            MapEntryPtr[i].JoinName.Length = cssm_strlen(
                NewLocationEntries->MapEntries[i].JoinName);

            MapEntryPtr[i].JoinName.Data = g_fx.malloc_func(
                VLHandle,
                MapEntryPtr[i].JoinName.Length);
            if (MapEntryPtr[i].JoinName.Data == NULL)
            {
                CSSM_SetError(&intel_preos_vlm_guid, CSSM_MEMORY_ERROR);
                goto EXIT;
            }
            cssm_memcpy(
                (void *)MapEntryPtr[i].JoinName.Data, 
                NewLocationEntries->MapEntries[i].JoinName,
                MapEntryPtr[i].JoinName.Length);

            if (CSSM_OK != smvl_LocationToServiceParameter(
                            VLHandle,
                            NewLocationEntries->MapEntries[i].MapEntry,
                            &MapEntryPtr[i].GetDataMethod,
                            &MapEntryPtr[i].GetParameters))
            {
                CSSM_SetError(&intel_preos_vlm_guid, CSSM_MEMORY_ERROR);
                goto EXIT;
            }
        }

        UpdateMap.NumberOfEntries = NewLocationEntries->NumberOfMapEntries;
        UpdateMap.MapEntries = MapEntryPtr;
        if (ISL_OK != ArchiveMethods.SetLocationMap(
                        ArchivePtr,
                        UpdateMap)) 
        {
            CSSM_SetError(&intel_preos_vlm_guid, CSSM_MEMORY_ERROR);
            goto EXIT;
        }

        ret = CSSM_OK;
    goto EXIT;
    }

EXIT:
    {
        ISL_LMAP_ENTRY MapEntry;
        uint32 i;

        for (i=0; i < NewLocationEntries->NumberOfMapEntries; i++)
        {
            MapEntry = MapEntryPtr[i];
            g_fx.free_func(VLHandle, (void *)MapEntry.JoinName.Data);
            smvl_FreeLocationToServiceParameter(
                VLHandle,
                NewLocationEntries->MapEntries[i].MapEntry,
                MapEntryPtr[i].GetDataMethod,
                MapEntryPtr[i].GetParameters);
        }
        g_fx.free_func(VLHandle, MapEntryPtr);
        return ret;
    }
}

#pragma warning (default : 4100)
