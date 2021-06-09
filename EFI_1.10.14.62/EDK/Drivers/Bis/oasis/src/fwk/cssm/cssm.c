/*-----------------------------------------------------------------------------
 *      File:   cssm.c
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
 * This file contains the CSSM core functions.
 */
/*
 * The CSSM_BIS implementation of this code was inspected 8/19/98.
 */

#include "internal.h"

CSSM_BOOL  CssmInit = CSSM_FALSE;
CSSM_API_MEMORY_FUNCS CssmMemFuncs;
CSSM_ERROR GlobalErr;

/*---------------------------------------------------------------
 *Name: CSSM_Init
 *
 *Description:
 *  Function used to initialize CSSM
 *
 *Parameters: 
 *  Version - Version of CSSM requested by app.  Must be 2.0.
 *  Reserved - Memory management functions from the application
 *
 *Returns:
 *  CSSM_FAIL - CSSM unable to initialize
 *  CSSM_OK - Initialization complete
 *
 *-------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_Init (const CSSM_VERSION_PTR Version,
                               const void *Reserved)
{
    CSSM_API_MEMORY_FUNCS_PTR MemoryFuncs = (CSSM_API_MEMORY_FUNCS_PTR) Reserved;
    extern cssm_ATTACH_INFO_NODE CachedAttachInfoNode;

#ifdef ISL_SELF_CHECK
    if (cssm_SelfCheck() != CSSM_OK)
        return CSSM_FAIL;
#endif

    if (CssmInit) /* If we've already been initialized, FAIL */
    {
        CSSM_SetError (&CssmGUID, CSSM_ALREADY_INITIALIZED);
        return CSSM_FAIL;
    }

    /* Check for correct version information */
    if (!Version) 
        return CSSM_FAIL;
    if (Version->Major != CSSM_MAJOR || Version->Minor != CSSM_MINOR) 
        return CSSM_FAIL;

    /* Verify that memory functions are available */
    if (!MemoryFuncs || 
        !MemoryFuncs->malloc_func  || !MemoryFuncs->calloc_func ||
        !MemoryFuncs->realloc_func || !MemoryFuncs->free_func ) 
        return CSSM_FAIL;

    /* Initialize globals */
    CssmMemFuncs = *MemoryFuncs;
    cssm_memset(&GlobalErr, 0, sizeof(CSSM_ERROR));
    cssm_memset(&CachedAttachInfoNode, 0, sizeof (cssm_ATTACH_INFO_NODE));

    CssmInit = CSSM_TRUE;

    return CSSM_OK;
}


/*---------------------------------------------------------------
 *Name: CSSM_ModuleAttach
 *
 *Description:
 *   Attach and load the addin module
 *
 *Parameters: 
 *   GUID            - GUID of the addin module
 *   Version         - Version requested of the addin module
 *   MemoryFuncs     - Memory functions for use by the addin module
 *   SubServiceID    - ID of the service requested 
 *   SubServiceFlags - Flags of the service requested 
 *   Application     - ID passed back in notification callback
 *   Notification    - Callback used by addin to notify app of addin events
 *   Reserved        - TBD
 *
 *Returns:
 *   0 - unable to attach to addin
 *   not 0 - handle to addin
 *
 *-------------------------------------------------------------*/
CSSM_MODULE_HANDLE   CSSMAPI CSSM_ModuleAttach (
                                const CSSM_GUID_PTR GUID,
                                const CSSM_VERSION_PTR Version,
                                const CSSM_API_MEMORY_FUNCS_PTR MemoryFuncs,
                                uint32 SubserviceID,
                                uint32 SubserviceFlags,
                                uint32 Application,
                                const CSSM_NOTIFY_CALLBACK Notification, 
                                const char *AppFileName,
                                const char *AppPathName,
                                const void * Reserved)
{
#ifdef CSSM_BIS
    cssm_ATTACHED_MODULE_NODE_PTR ModuleInfo;
    cssm_ATTACH_INFO_NODE_PTR     HandleInfo;
    CSSM_BOOL                     IsFirstAttach = CSSM_FALSE;
    SubserviceID; SubserviceFlags; Application;
#endif
    Reserved;

    /* Make sure that app has done a CSSM_Init */
    if (cssm_CheckInit () == CSSM_FAIL)
        return CSSM_INVALID_HANDLE;

    /* Clear the error */
    CSSM_ClearError ();

    /* Check inputs */
    if (!GUID) {
        CSSM_SetError(&CssmGUID,CSSM_INVALID_GUID);
        return CSSM_INVALID_HANDLE;
    }
    if (!Version) {
        CSSM_SetError(&CssmGUID,CSSM_INVALID_POINTER);
        return CSSM_INVALID_HANDLE;
    }
    if (Notification) {
        CSSM_SetError(&CssmGUID,CSSM_INVALID_POINTER);
        return CSSM_INVALID_HANDLE;
    }
    if (MemoryFuncs) {
        if (!MemoryFuncs->malloc_func  || !MemoryFuncs->calloc_func ||
            !MemoryFuncs->realloc_func || !MemoryFuncs->free_func )
        {
            CSSM_SetError(&CssmGUID,CSSM_INVALID_POINTER);
            return CSSM_INVALID_HANDLE;
        }
    }

#ifdef CSSM_BIS
    /* 
     * If this is the first attach, 
     * create a module record and load the addin module 
     */
    if ((ModuleInfo = cssm_GetModuleRecordByGUID(GUID)) == NULL) {
        ADDIN_AUTH_FUNC_PTR AddInAuthFnPtr;
        IsFirstAttach = CSSM_TRUE;

        /* Create an internal record for this module */
        if (cssm_NewModuleRecord (GUID) == CSSM_FAIL)
            return CSSM_INVALID_HANDLE;

        /* Find the address of the module's AddinAuthenticate function */
        AddInAuthFnPtr = cssm_GetAddinAuthenticateFuncPtr(GUID);
        if (!AddInAuthFnPtr) 
        {
            cssm_RemoveModuleRecord (GUID);
            CSSM_SetError (&CssmGUID, CSSM_INVALID_GUID);
            return CSSM_INVALID_HANDLE;
        } 

        /* Load the module */
        if (AddInAuthFnPtr (NULL, NULL, AppFileName, AppPathName) != CSSM_OK) {
            cssm_RemoveModuleRecord (GUID);
            CSSM_SetError (&CssmGUID, CSSM_ATTACH_FAIL);
            return CSSM_INVALID_HANDLE;
        }

        /* Get the module record and */
        /* verify that a callback function table was registered */
        ModuleInfo = cssm_GetModuleRecordByGUID(GUID);
        if (!ModuleInfo || !ModuleInfo->AddInJT)  {
            cssm_RemoveModuleRecord (GUID);
            CSSM_SetError (&CssmGUID, CSSM_ATTACH_FAIL);
            return CSSM_INVALID_HANDLE;
        }  
    }

    /* 
     * Create a attach info node and add it to the record for this module 
     */

    /* Allocate memory for the attach info node */
    HandleInfo = cssm_malloc (sizeof(cssm_ATTACH_INFO_NODE), 0);
    if (!HandleInfo) {
        if (IsFirstAttach)
            cssm_RemoveModuleRecord (GUID);
        CSSM_SetError (&CssmGUID, CSSM_MEMORY_ERROR);
        return CSSM_INVALID_HANDLE;
    }

    /* Initialize the node */
    HandleInfo->Handle = cssm_GetHandle();
    if (MemoryFuncs) HandleInfo->MemoryFunctions = *MemoryFuncs;
    else             HandleInfo->MemoryFunctions = CssmMemFuncs;

    /* Insert the node at the front of the module's AttachInfo list */
    HandleInfo->Next = ModuleInfo->AttachInfo;
    ModuleInfo->AttachInfo = HandleInfo;

    /* 
     * If this was the initial attach,
     * Initialize the module 
     */
    if (IsFirstAttach) {
        if (ModuleInfo->AddInJT->Initialize) {
            if (ModuleInfo->AddInJT->Initialize(HandleInfo->Handle,
                       Version->Major, Version->Minor) != CSSM_OK) 
            {
                uint32 error = GlobalErr.error;
                CSSM_ModuleDetach(HandleInfo->Handle);
                CSSM_SetError(GUID, error);
                return CSSM_INVALID_HANDLE;
            }
        }
    }
    
    return (HandleInfo->Handle);

#else 
    return cssm_ModuleAttach (GUID, Version, MemoryFuncs, SubserviceID,
                              SubserviceFlags, Application, Notification,
                              AppFileName, AppPathName,
                              CSSM_EVENT_ATTACH, CSSM_EVENT_DETACH);
#endif /* #ifdef CSSM_BIS */
}


/*---------------------------------------------------------------
 *Name: CSSM_ModuleDetach
 *
 *Description:
 *   Detach addin module from CSSM
 *
 *Parameters: 
 *   ModuleHandle - handle to addin 
 *
 *Returns:
 *   CSSM_FAIL - unable to detach module
 *   CSSM_OK - module is detached
 *
 *-------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_ModuleDetach (CSSM_MODULE_HANDLE ModuleHandle)
{
#ifdef CSSM_BIS
    cssm_ATTACHED_MODULE_NODE_PTR ModuleInfo;
    cssm_ATTACH_INFO_NODE_PTR     HandleInfo, 
                                  PreviousHandleInfo = NULL;
    extern cssm_ATTACH_INFO_NODE  CachedAttachInfoNode;
#endif

    /* Make sure that app has done a CSSM_Init */
    if (cssm_CheckInit () == CSSM_FAIL)
        return CSSM_FAIL;

    /* Clear the error */
    CSSM_ClearError ();

#ifdef CSSM_BIS
    /* Get the addin record */
    if ((ModuleInfo = cssm_GetModuleRecord (ModuleHandle, 0, NULL)) == NULL)
        return CSSM_FAIL;

    /* Find the information for this handle within this addin record */
    HandleInfo = ModuleInfo->AttachInfo;
    while (HandleInfo) {
        if (ModuleHandle == HandleInfo->Handle)
            break;
        PreviousHandleInfo = HandleInfo;
        HandleInfo = HandleInfo->Next;
    }

    /* If the ModuleHandle was not found, return failure */
    if (!HandleInfo) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_ADDIN_HANDLE);
        return CSSM_FAIL;
    }

    /* If this was the last attach instance, inform the addin */
    if (!PreviousHandleInfo && !HandleInfo->Next) {
        if (ModuleInfo->AddInJT && ModuleInfo->AddInJT->Terminate)
                ModuleInfo->AddInJT->Terminate (ModuleHandle);
    }

    /* Remove this attach instance from the list */
    if (!PreviousHandleInfo)
        ModuleInfo->AttachInfo = HandleInfo->Next;
    else
        PreviousHandleInfo->Next = HandleInfo->Next;
    if (CachedAttachInfoNode.Handle == ModuleHandle)
        CachedAttachInfoNode.Handle = 0;

    /* Free the memory associated with this handle */             
    cssm_free (HandleInfo, 0);

    /* If this was the last detach, free the module record */
    if (NULL == ModuleInfo->AttachInfo)
        return cssm_RemoveModuleRecord(&ModuleInfo->GUID);

    return CSSM_OK;
#else    
    return cssm_ModuleDetach (ModuleHandle, CSSM_EVENT_DETACH);
#endif /* #ifdef CSSM_BIS */
}


