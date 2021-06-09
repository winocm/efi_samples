/*-----------------------------------------------------------------------------
 *      File:   spi.c
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
 * This file contains the functions that are contained in the SPI portion
 * of the CSSM exported functions.
 */

#include "cssm.h"
#include "cssmport.h"
#include "internal.h"
#include "cssmspi.h"

cssm_ATTACH_INFO_NODE CachedAttachInfoNode;
extern CSSM_API_MEMORY_FUNCS CssmMemFuncs;

/*---------------------------------------------------------------
 *Name: cssm_GetMemory
 *
 *Description:
 *  Locate the memory functions for a given handle
 *
 *Parameters: 
 *  Handle - handle of add-in module
 *
 *Returns:
 *  Null - CSSM was unable locate the memory functions
 *  non Null - pointer to the memory functions
 *
 *----------------------------------------------------------------*/
CSSM_API_MEMORY_FUNCS_PTR cssm_GetMemory (CSSM_HANDLE Handle)

{
    cssm_ATTACHED_MODULE_NODE_PTR ModuleInfo;
    cssm_ATTACH_INFO_NODE_PTR HandleInfo;
    extern cssm_ATTACHED_MODULE_NODE_PTR ModuleListHead;

    //
    // BUGBUG Init'd to zero to avoid warning 4
    //
    HandleInfo = NULL;

    /* If no handle is input, return the current CssmMemoryFunctions */
    if (!Handle)
        return &CssmMemFuncs;

	/* If the handle equals what is in the cache mem ptr, 
       return the cache mem ptr functions */
    if (CachedAttachInfoNode.Handle == Handle)
        return &CachedAttachInfoNode.MemoryFunctions;

    /* Find the memory functions associated with this handle */
    /* Iterate through the attached module list looking for the handle */    
    ModuleInfo = ModuleListHead;
    while (ModuleInfo) {
        HandleInfo = ModuleInfo->AttachInfo;
        while (HandleInfo != NULL) {          /* Loop through the handles for this addin */
            if (Handle == HandleInfo->Handle) /* Found the handle, so exit loop */
				break;
            HandleInfo = HandleInfo->Next;    /* Iterate to the next handle */
        }
        if (HandleInfo) break;         /* Found the handle, so exit loop */
		ModuleInfo = ModuleInfo->Next; /* Iterate to the next addin */
    }
	
    /* If the handle was not found, return failure */
	if (!ModuleInfo) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_ADDIN_HANDLE);
        return NULL;
	}

    /* Copy this attach info into the currently cached area */
	cssm_memcpy(&CachedAttachInfoNode, HandleInfo, sizeof(cssm_ATTACH_INFO_NODE));

    /* Return the memory function pointers */
	return &CachedAttachInfoNode.MemoryFunctions;
}


/*---------------------------------------------------------------
 *
 *Name: cssm_SpiMalloc
 *
 *Description:
 *   All add-in mallocs come through here to get memory from an app
 *
 *Parameters: 
 *   handle - identifies the application to supply memory functions
 *   size - the number of bytes to malloc
 *
 *Returns:
 *   NULL - error in allocating memory
 *   not NULL - pointer to memory
 *
 *-------------------------------------------------------------*/
void * cssm_SpiMalloc (CSSM_HANDLE handle, uint32 size)
{
    CSSM_API_MEMORY_FUNCS_PTR MemFuncsPtr = cssm_GetMemory (handle);

    if (!MemFuncsPtr)
        return NULL;

    return MemFuncsPtr->malloc_func(size, MemFuncsPtr->AllocRef);
}

/*---------------------------------------------------------------
 *
 *Name: cssm_SpiFree
 *
 *Description:
 *   All add-in frees come through here to get memory from an app
 *
 *Parameters: 
 *   handle - identifies the application to supply memory functions
 *   mem - pointer to memory to be freed
 *
 *Returns:
 *   None
 *
 *-------------------------------------------------------------*/
void cssm_SpiFree (CSSM_HANDLE handle, void *mem)
{
    CSSM_API_MEMORY_FUNCS_PTR MemFuncsPtr = cssm_GetMemory (handle);

    if (!MemFuncsPtr)
        return;

   MemFuncsPtr->free_func(mem, MemFuncsPtr->AllocRef);
}

/*---------------------------------------------------------------
 *
 *Name: cssm_SpiRealloc
 *
 *Description:
 *   All add-in reallocs come through here to get memory from an app
 *
 *Parameters: 
 *   handle - identifies the application to supply memory functions
 *   mem - pointer to previously allocated memory
 *   size - the number of bytes to malloc
 *
 *Returns:
 *   NULL - error in allocating memory
 *   non NULL - pointer to memory
 *
 *-------------------------------------------------------------*/
void * cssm_SpiRealloc (CSSM_HANDLE handle, void *mem, uint32 size)
{
    CSSM_API_MEMORY_FUNCS_PTR MemFuncsPtr = cssm_GetMemory (handle);

    if (!MemFuncsPtr)
        return NULL;

    return MemFuncsPtr->realloc_func(mem, size, MemFuncsPtr->AllocRef);
}

/*---------------------------------------------------------------
 *
 *Name: cssm_SpiCalloc
 *
 *Description:
 *   All add-in callocs come through here to get memory from an app
 *
 *Parameters: 
 *   handle - identifies the application to supply memory functions
 *   num - value to initialize memory
 *   size - the number of bytes to malloc
 *
 *Returns:
 *   NULL - error in allocating memory
 *   non NULL - pointer to memory
 * 
 *-------------------------------------------------------------*/
void * cssm_SpiCalloc (CSSM_HANDLE handle, uint32 num, uint32 size)
{
    CSSM_API_MEMORY_FUNCS_PTR MemFuncsPtr = cssm_GetMemory (handle);

    if (!MemFuncsPtr)
        return NULL;

    return MemFuncsPtr->calloc_func(num, size, MemFuncsPtr->AllocRef);
}


/*---------------------------------------------------------------
 *Name: cssm_RegisterServices
 *
 *Description:
 *   Generic function for all add-in registration
 *
 *Parameters: 
 *  GUID - guid of add-in
 *  FunctionTable - functions provided by the add-in 
 *  UpcallTable - table for return of the memory functions
 *
 *Returns:
 *   CSSM_FAIL - unable to complete registration
 *   CSSM_OK - registration completed
 *
 *-------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_RegisterServices (
                             const CSSM_GUID_PTR GUID,
                             const CSSM_REGISTRATION_INFO_PTR FunctionTable,
                             CSSM_SPI_MEMORY_FUNCS_PTR UpcallTable,
                             void *Reserved)
{
	Reserved;

    /* make sure that app has done a CSSM_Init */
    if (cssm_CheckInit () == CSSM_FAIL)
        return CSSM_FAIL;

    /* clear the error */
    CSSM_ClearError ();

    /* Check validity of pointers passed in */
    if (!GUID) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_GUID);
        return CSSM_FAIL;
    }

    if (!UpcallTable) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_POINTER);
        return CSSM_FAIL;
    }

    if (!FunctionTable) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_POINTER);
        return CSSM_FAIL;
    }

    /* Initialize the upcall memory table */
    UpcallTable->malloc_func  = cssm_SpiMalloc;
    UpcallTable->free_func    = cssm_SpiFree;
    UpcallTable->realloc_func = cssm_SpiRealloc;
    UpcallTable->calloc_func  = cssm_SpiCalloc;

    /* 
	 * Update the ModuleRecord for the specified GUID with the
	 * function pointers in the FunctionTable.
	 */
    return cssm_UpdateModuleRecord (GUID, FunctionTable);
}

/*---------------------------------------------------------------
 *Name: CSSM_DeregisterServices
 *
 *Description:
 *   Generic function for deregistering of add-in with CSSM
 *
 *Parameters: 
 *  GUID - guid of add-in
 *
 *Returns:
 *   CSSM_FAIL - unable to complete deregistration
 *   CSSM_OK - deregistration completed
 *
 *-------------------------------------------------------------*/
CSSM_RETURN CSSMAPI CSSM_DeregisterServices (const CSSM_GUID_PTR GUID)
{
    /* make sure that app has done a CSSM_Init */
    if (cssm_CheckInit () == CSSM_FAIL)
        return CSSM_FAIL;

    /* clear the error */
    CSSM_ClearError ();

    /* Check validity of incoming pointers */
    if (!GUID) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_GUID);
        return CSSM_FAIL;
    }

	/* 
	 * Update the ModuleRecord for the specified GUID with the
	 * function pointers in the FunctionTable.
	 */
    return cssm_UpdateModuleRecord (GUID, NULL);
}


