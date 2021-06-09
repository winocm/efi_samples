/*-----------------------------------------------------------------------------
 *      File:   addmgr.c
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
 * This file contains the code used to manage add-in modules.
 */
/*
 * The CSSM_BIS implementation of this code was inspected 8/19/98.
 */

#include "internal.h"

cssm_ATTACHED_MODULE_NODE_PTR ModuleListHead = NULL;

/*------------------------------------------------------------------------------
 *Name: cssm_GetHandle
 *
 *Description:
 * Return a 32 bit number for use as a handle, 
 *
 *Parameters: 
 * None
 *
 *Returns: 
 * a CSSM_HANDLE
 *
 *----------------------------------------------------------------------------*/
CSSM_HANDLE cssm_GetHandle ()
{
    static void * Value = (void *) 0xFF;
    Value = (void *) (& (((unsigned char *) Value)[1]));
    return (CSSM_HANDLE)Value;
}


/*------------------------------------------------------------------------------
 *Name: cssm_GetModuleRecord
 *
 *Description:
 * This function gets the module information record given the addin handle.
 * It returns a pointer to the record and, if requested, 
 * callback functions for the module.
 *
 *Parameters:
 * Handle      (input) - Handle of addin whose info is to be found
 * ServiceType (input) - Type of addin service to be retrieved
 * CallBack   (output) - Pointer to a structure pointer that will hold
 *                       the callback functions registered by the addin
 *
 *Returns:
 * NULL - In the case of failure.
 * not NULL - Pointer to the module information.
 *----------------------------------------------------------------------------*/
cssm_ATTACHED_MODULE_NODE_PTR cssm_GetModuleRecord (CSSM_HANDLE Handle,
                                                  CSSM_SERVICE_MASK ServiceType,
                                                  void **CallBack)
{
    cssm_ATTACHED_MODULE_NODE_PTR ModuleInfo;

    /* Make sure that app has done a CSSM_Init */
    if (cssm_CheckInit () == CSSM_FAIL)
        return NULL;

    /* Clear the error */
    CSSM_ClearError ();
	
    /* Iterate through the addin list looking for the handle */    
    ModuleInfo = ModuleListHead;
    while (ModuleInfo) {
        cssm_ATTACH_INFO_NODE_PTR HandleInfo = ModuleInfo->AttachInfo;

        while (HandleInfo) {          
            if (Handle == HandleInfo->Handle)
                break;
            HandleInfo = HandleInfo->Next;
        }
        if (HandleInfo) break;

        ModuleInfo = ModuleInfo->Next;
    }
	
    /* If the handle was not found, return failure */
    if (!ModuleInfo) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_ADDIN_HANDLE);
        return NULL;
    }

    /* If requested, get the callback function pointer for this addin */
    if (CallBack) {
        uint32 i;

        for (i=0; i<ModuleInfo->AddInJT->NumberOfServiceTables; i++) {
            if (ModuleInfo->AddInJT->Services[i].ServiceType == ServiceType) {
                *CallBack = 
                   ModuleInfo->AddInJT->Services[i].FunctionTable.ServiceFuncs;
                break;
            }
        }

        if (i == ModuleInfo->AddInJT->NumberOfServiceTables) {
            CSSM_SetError (&CssmGUID, CSSM_FUNCTION_NOT_IMPLEMENTED);
            return NULL;
        }
    }

    return ModuleInfo;
}


/*------------------------------------------------------------------------------
 *Name: cssm_GetModuleRecordByGUID
 *
 *Description:
 * This function gets the module information record given the addin GUID.
 * It returns a pointer to the record.
 *
 *Parameters:
 * GUID (input) - GUID of the addin whose info is to be found
 *
 *Returns:
 * NULL - If this GUID is not found.
 * not NULL - Pointer to the module information for this addin.
 *----------------------------------------------------------------------------*/
cssm_ATTACHED_MODULE_NODE_PTR cssm_GetModuleRecordByGUID 
                                               (const CSSM_GUID_PTR GUID)
{
    cssm_ATTACHED_MODULE_NODE_PTR ModuleInfo;

    /* Existence of GUID pointer is checked by calling function */
    /* Iterate through the addin list looking for the matching GUID */   
    ModuleInfo = ModuleListHead;
    while (ModuleInfo) {
        if (cssm_memcmp (GUID, &ModuleInfo->GUID, sizeof(CssmGUID)) == 0) 
            break;
        ModuleInfo = ModuleInfo->Next;
    }
    
    return ModuleInfo;
}


/*------------------------------------------------------------------------------
 *Name: cssm_NewModuleRecord
 *
 *Description:
 * Allocates, initializes and adds to the attached module list a node for 
 * the module identified by the input GUID. The presence of this empty node 
 * indicates that the addin with this GUID is about to be attached and 
 * register its services. This is performed separately from RegisterServices
 * to insure that applications cannot register as CSSM addins.
 *
 *Parameters:
 * GUID (input) - GUID of the addin 
 *
 *Returns:
 * CSSM_FAIL - Either an addin with this GUID is already loaded or
 *             insufficient memory was available.
 * CSSM_OK - Node successfully added.
 *----------------------------------------------------------------------------*/
CSSM_RETURN cssm_NewModuleRecord (const CSSM_GUID_PTR GUID)
{
    cssm_ATTACHED_MODULE_NODE_PTR ModuleInfo;
	
    /* Existence of GUID pointer is checked by calling function */
    /* Verify that a record for this GUID does not already exist */
    if (cssm_GetModuleRecordByGUID(GUID) != NULL) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_GUID);
        return CSSM_FAIL;
    }

    /* Allocate memory for the new node */
    ModuleInfo = cssm_calloc(1, sizeof(cssm_ATTACHED_MODULE_NODE), 0);
    if (!ModuleInfo) {
        CSSM_SetError (&CssmGUID, CSSM_MEMORY_ERROR);
        return CSSM_FAIL;
    }
	
    /* Initialize the new node */
    ModuleInfo->GUID = *GUID;
    
    /* Insert the new node into the addin list */
    ModuleInfo->Next = ModuleListHead;
    ModuleListHead = ModuleInfo;

    return CSSM_OK;
}


/*------------------------------------------------------------------------------
 *Name: cssm_UpdateModuleRecord
 *
 *Description:
 * This function is called by Register and Deregister services.
 * It either adds the input FunctionTable to or removes the existing 
 * function table from the attached module list node associated with 
 * the input GUID.
 *
 *Parameters:
 * GUID (input) - GUID of the addin whose function table is to be modified
 * FunctionTable (input) - if NULL, this parameter indicates that the
 *                         function table should be removed from the addin node.
 *                         if not NULL, this function table should be copied 
 *                         into the addin node
 *
 *Returns:
 * CSSM_OK - if the registration or deregistration was successful
 * CSSM_FAIL - if the inputs are invalid or memory allocation fails
 *
 *----------------------------------------------------------------------------*/
#define ALL_SERVICES_MASK (CSSM_SERVICE_CSP | CSSM_SERVICE_DL | CSSM_SERVICE_CL | CSSM_SERVICE_TP | CSSM_SERVICE_VL)
CSSM_RETURN cssm_UpdateModuleRecord (
                                 const CSSM_GUID_PTR GUID,
                                 const CSSM_REGISTRATION_INFO_PTR FunctionTable)
{
    cssm_ATTACHED_MODULE_NODE_PTR ModuleInfo;
    CSSM_MODULE_FUNCS_PTR         Src, Dest;
    uint32 i,j, size;
		
    //
    // BUGBUG Init'd size to 0 to avoid warning 4
    size = 0;

    /* Get the node for the module being registered or deregistered */
    if ((ModuleInfo = cssm_GetModuleRecordByGUID(GUID)) == NULL) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_GUID);
        return CSSM_FAIL;
    }

    /* If Deregistering, */
    /* Free the memory associated with the callback function ptrs and return */
    if (NULL == FunctionTable) {
        if (ModuleInfo->AddInJT) {
            CSSM_REGISTRATION_INFO_PTR Callback = ModuleInfo->AddInJT;
            for (i = 0; i < Callback->NumberOfServiceTables; i++)
                cssm_free(Callback->Services[i].FunctionTable.ServiceFuncs, 0);
            cssm_free (Callback->Services, 0);
            cssm_free (Callback, 0);
            ModuleInfo->AddInJT = NULL;
        }

        return CSSM_OK;
    }
	
    /* 
     * If Registering,
     */

    /* Validate that this module is not already registered */
    if (ModuleInfo->AddInJT != NULL) {
        CSSM_SetError (&CssmGUID, CSSM_ADDIN_ALREADY_REGISTERED);
        return CSSM_FAIL;
    }

    /* Validate that this FunctionTable contains at least 1 service */
    /* and does not include any unknown services */
    if (!FunctionTable->NumberOfServiceTables || !FunctionTable->Services ||
        ((FunctionTable->ServiceSummary & ~ALL_SERVICES_MASK) != 0))
    {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_SERVICE_MASK);
        return CSSM_FAIL;
    }

    /* Validate that each service table is a known service type */
    /* and that it contains a callback function table */
    for (i=0; i <FunctionTable->NumberOfServiceTables ; i++)
    {
        if (!FunctionTable->Services[i].FunctionTable.ServiceFuncs ||
            ((FunctionTable->Services[i].ServiceType & ~ALL_SERVICES_MASK) != 0))
        {
            CSSM_SetError (&CssmGUID, CSSM_INVALID_SERVICE_MASK);
            return CSSM_FAIL;
        }
    }

    /* Allocate and copy the Registration Info */
    ModuleInfo->AddInJT = cssm_malloc(sizeof(CSSM_REGISTRATION_INFO), 0);
    if (!ModuleInfo->AddInJT) {
        CSSM_SetError (&CssmGUID, CSSM_MEMORY_ERROR);
        return CSSM_FAIL;
    }
    *ModuleInfo->AddInJT = *FunctionTable;

    /* Allocate memory for the new Services */
    ModuleInfo->AddInJT->Services = 
        cssm_calloc(FunctionTable->NumberOfServiceTables, 
                                              sizeof(CSSM_MODULE_FUNCS), 0);
    if (!ModuleInfo->AddInJT->Services) {
        cssm_free(ModuleInfo->AddInJT, 0);
        ModuleInfo->AddInJT = NULL;
        CSSM_SetError (&CssmGUID, CSSM_MEMORY_ERROR);
        return CSSM_FAIL;
    }

    /* Allocate and copy the callback function table for each service */
    Src = FunctionTable->Services;
    Dest = ModuleInfo->AddInJT->Services;
    for (i=0; i<FunctionTable->NumberOfServiceTables; i++, Src++, Dest++) {
        Dest->ServiceType = Src->ServiceType;

        switch (Src->ServiceType) {
            case CSSM_SERVICE_TP :
                size = sizeof (CSSM_SPI_TP_FUNCS);
                break;
				
            case CSSM_SERVICE_CSP :
                size = sizeof (CSSM_SPI_CSP_FUNCS);
                break;

            case CSSM_SERVICE_CL :
                size = sizeof (CSSM_SPI_CL_FUNCS);
                break;

            case CSSM_SERVICE_DL :
                size = sizeof (CSSM_SPI_DL_FUNCS);
                break;

            case CSSM_SERVICE_VL :
                size = sizeof (CSSM_SPI_VL_FUNCS);
                break;
        }
		
        Dest->FunctionTable.ServiceFuncs = cssm_malloc (size, 0);
        if (!Dest->FunctionTable.ServiceFuncs) {
            CSSM_REGISTRATION_INFO_PTR Callback = ModuleInfo->AddInJT;
            for (j=0; j < i; j++)
                cssm_free(Callback->Services[j].FunctionTable.ServiceFuncs, 0);
            cssm_free(Callback->Services, 0);
            cssm_free(Callback, 0);
            ModuleInfo->AddInJT = NULL;
            CSSM_SetError (&CssmGUID, CSSM_MEMORY_ERROR);
            return CSSM_FAIL;
        }
        cssm_memcpy(Dest->FunctionTable.ServiceFuncs, 
                    Src->FunctionTable.ServiceFuncs, size);
    }

    return CSSM_OK;
}


/*------------------------------------------------------------------------------
 *Name: cssm_RemoveModuleRecord
 *
 *Description:
 * Deletes the node associated with this add-in module.
 *
 *Parameters: 
 * GUID - guid of the add-in module
 *
 *Returns:
 * CSSM_FAIL - unable to delete the add-in record
 * CSSM_OK - add-in record is removed
 *
 *----------------------------------------------------------------------------*/
CSSM_RETURN cssm_RemoveModuleRecord (const CSSM_GUID_PTR GUID)
{
    cssm_ATTACHED_MODULE_NODE_PTR ModuleInfo, 
                                  PreviousModuleInfo = NULL;
    cssm_ATTACH_INFO_NODE_PTR     HandleInfo, 
                                  NextHandleInfo;
    uint32 i;

    /* Iterate through the list looking for the node with the input GUID */
    ModuleInfo = ModuleListHead;
    while (ModuleInfo) {

        /* If not found, proceed to the next node */
        if (cssm_memcmp (GUID, &ModuleInfo->GUID, sizeof(CssmGUID)) != 0) {
            PreviousModuleInfo = ModuleInfo;
            ModuleInfo = ModuleInfo->Next;

        } else { /* If found, */

            /* Remove it from the list */
            if (!PreviousModuleInfo)
                ModuleListHead = ModuleInfo->Next;
            else
                PreviousModuleInfo->Next = ModuleInfo->Next;

            /* Free the attach information for every attach */
            HandleInfo = ModuleInfo->AttachInfo;
            while (HandleInfo) {
                NextHandleInfo = HandleInfo->Next;
                cssm_free (HandleInfo, 0);
                HandleInfo = NextHandleInfo;
            }

            /* Free the addin function tables */
            if (ModuleInfo->AddInJT) {
                CSSM_REGISTRATION_INFO_PTR Callback = ModuleInfo->AddInJT;
                for (i=0; i < Callback->NumberOfServiceTables; i++) 
                   cssm_free(Callback->Services[i].FunctionTable.ServiceFuncs, 0);
                cssm_free(Callback->Services, 0);
                cssm_free(Callback, 0);
            }

            /* Free the node */
            cssm_free (ModuleInfo, 0);
            return CSSM_OK;
        }
    }
	
    /* No node was found for the input GUID */
    CSSM_SetError (&CssmGUID, CSSM_INVALID_GUID);
    return CSSM_FAIL;
}
