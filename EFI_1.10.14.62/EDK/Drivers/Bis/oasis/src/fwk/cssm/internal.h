/*-----------------------------------------------------------------------------
 *      File:   internal.h
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
 * This file contains the data types and function prototypes for the functions 
 * used internally to manage addin attaches.
 */
/*
 * The CSSM_BIS implementation of this code was inspected 8/19/98.
 */

#ifndef CSSM_INTERNAL_H
#define CSSM_INTERNAL_H

#include "cssm.h"
#include "cssmport.h"
#include "cssmspi.h"
#include "guids.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CssmGUID intel_preos_cssm_guid
extern CSSM_BOOL CssmInit;
#define cssm_CheckInit()  ((CssmInit == CSSM_TRUE) ? CSSM_OK : CSSM_FAIL)

typedef struct cssm_attach_info_node {
    CSSM_HANDLE Handle;
    CSSM_API_MEMORY_FUNCS MemoryFunctions;
#ifndef CSSM_BIS
    uint32 Application;
    CSSM_NOTIFY_CALLBACK Notification;
	CSSM_VERSION Version;
    uint32 SubserviceID;
    uint32 SubserviceFlags;
#endif
    struct cssm_attach_info_node *Next;
} cssm_ATTACH_INFO_NODE, *cssm_ATTACH_INFO_NODE_PTR;

typedef struct cssm_attached_module_node {
    CSSM_GUID GUID;                        /* GUID of addin */
    cssm_ATTACH_INFO_NODE_PTR AttachInfo;  /* Handles issued for this add-in */
    CSSM_REGISTRATION_INFO_PTR AddInJT;    /* Entry points for the addin's fns */
    struct cssm_attached_module_node *Next;/* Next addin in list */
} cssm_ATTACHED_MODULE_NODE, *cssm_ATTACHED_MODULE_NODE_PTR;
 

CSSM_HANDLE cssm_GetHandle ();

cssm_ATTACHED_MODULE_NODE_PTR cssm_GetModuleRecord (CSSM_HANDLE Handle,
                                                    CSSM_SERVICE_MASK ServiceType,
                                                    void **CallBack);

cssm_ATTACHED_MODULE_NODE_PTR cssm_GetModuleRecordByGUID 
                                                   (const CSSM_GUID_PTR GUID);


CSSM_RETURN cssm_NewModuleRecord     (const CSSM_GUID_PTR GUID);
CSSM_RETURN cssm_UpdateModuleRecord  (const CSSM_GUID_PTR GUID,
                const CSSM_REGISTRATION_INFO_PTR FunctionTable);
CSSM_RETURN cssm_RemoveModuleRecord  (const CSSM_GUID_PTR GUID);


#ifndef CSSM_BIS
CSSM_HANDLE CSSMAPI cssm_ModuleAttach (const CSSM_GUID_PTR GUID,
                                      CSSM_VERSION_PTR Version,
                                      const CSSM_API_MEMORY_FUNCS_PTR MemoryFunc,
                                      uint32 SubserviceID,
                                      uint32 SubserviceFlags,
                                      uint32 Application,
                                      const CSSM_NOTIFY_CALLBACK Notification,
                                      const char *AppFileName,
                                      const char *AppPathName,
                                      CSSM_EVENT_TYPE eventType,
                                      CSSM_EVENT_TYPE detachEventType);
CSSM_RETURN CSSMAPI cssm_ModuleDetach (CSSM_HANDLE AddInHandle,
                                      CSSM_EVENT_TYPE eventType);
#endif

#ifdef ISL_SELF_CHECK
CSSM_RETURN cssm_SelfCheck();
#endif

#ifdef ISL_INCLUDED
CSSM_RETURN cssm_AddinLoadAndCheckIntegrity(CSSM_VO_HANDLE ModuleManifest, 
                                            char *ModuleManifestSectionName,
                                            char* AppCredentialPath, 
                                            char* AppSectionName,
                                            CSSM_GUID_PTR ModuleGuid);
CSSM_RETURN cssm_GetDoAttribute            (CSSM_VO_HANDLE VOHandle,
                                            CSSM_STRING DoName,
                                            CSSM_DB_ATTRIBUTE_INFO_PTR AttrName,
                                            CSSM_DATA *AttrValue);
CSSM_MODULE_INFO_PTR      cssm_BuildModuleInfoStruct
                                           (CSSM_VO_HANDLE ModuleManifest, 
                                            CSSM_STRING SectionName);
CSSM_APP_SERVICE_INFO_PTR cssm_BuildAppInfoStruct
                                           (CSSM_VO_HANDLE AppManifest, 
                                            CSSM_STRING SectionName);
void cssm_FreeAppInfoStruct        (CSSM_APP_SERVICE_INFO_PTR AppInfoPtr);
void cssm_FreeModuleAuthInfoStruct (CSSM_MODULE_INFO_PTR ModuleDescription);
#endif

#ifdef __cplusplus
}
#endif

#endif
