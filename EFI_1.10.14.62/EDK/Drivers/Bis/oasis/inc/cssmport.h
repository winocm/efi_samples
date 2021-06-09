/*-----------------------------------------------------------------------
 *      File:   cssmport.h
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *
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

#ifndef _CSSMPORT_H
#define _CSSMPORT_H    

#include "cssm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Memory Management Routines */
void * cssm_malloc  (uint32 size, void* allocRef);
void * cssm_calloc  (uint32 num_elem, uint32 num_bytes, void* allocRef);
void   cssm_free    (void * mem_ptr, void* allocRef);
void * cssm_realloc (void * old_ptr, uint32 num_bytes, void* allocRef);
void * cssm_memset  (void * ptr, sint32  value, uint32 num_bytes);
void * cssm_memcpy  (void * dest_ptr, const void * src_ptr, uint32 num_bytes);
void * cssm_memmove (void * dest_ptr, const void * src_ptr, uint32 num_bytes);
sint32 cssm_memcmp  (const void * ptr1, const void * ptr2, uint32 count);

/* Dynamic Loading of Libraries */
typedef CSSM_RETURN (CSSMAPI *ADDIN_AUTH_FUNC_PTR) 
                    (const char* cssmCredentialPath, const char* cssmSection,
                     const char* AppCredential, const char* AppSection);

ADDIN_AUTH_FUNC_PTR cssm_GetAddinAuthenticateFuncPtr(CSSM_GUID_PTR Guid);

/* String-handling Routines */
uint32 CSSMAPI cssm_strlen(const char *);

/* RNG Routines */
void   CSSMAPI cssm_srand(uint32 Seed);
sint32 CSSMAPI cssm_rand(void);

#ifndef CSSM_BIS
/* VO Registry Routines */
CSSM_VOBUNDLE_UID_PTR CSSMAPI 
cssm_vlreg_ImportVoBundle   (CSSM_VL_VOBUNDLE_REGISTRY_INFO_PTR VoBundleInfo);

CSSM_RETURN CSSMAPI 
cssm_vlreg_DeleteVoBundle   (const CSSM_VOBUNDLE_UID_PTR VoBundleId);

CSSM_VL_VOBUNDLE_REGISTRY_INFO_PTR CSSMAPI
cssm_vlreg_GetVoBundleInfo  (const CSSM_VOBUNDLE_UID_PTR VoBundleId,
                             CSSM_BOOL AsPointer);

CSSM_RETURN CSSMAPI 
cssm_vlreg_FreeVoBundleInfo (CSSM_VL_VOBUNDLE_REGISTRY_INFO_PTR VoBundleInfo);

CSSM_RETURN CSSMAPI 
cssm_vlreg_FreeVoInfoArray  (CSSM_VL_VO_REGISTRY_INFO_PTR VoInfos, 
                             uint32 NumberOfVoInfos);


typedef struct cssm_vobundleid_node {
    CSSM_VOBUNDLE_UID Id;
    struct cssm_vobundleid_node *NextNode;
} CSSM_VOBUNDLEID_NODE, *CSSM_VOBUNDLEID_NODE_PTR;

CSSM_VOBUNDLEID_NODE_PTR  CSSMAPI 
cssm_vlreg_GetPointerToVoBundleIds (const CSSM_VO_UID_PTR VoIdentifier);


/* Initialization and termination routines */
CSSM_RETURN CSSMAPI cssm_port_init(void);
CSSM_RETURN CSSMAPI cssm_port_terminate(void);

/* Addin Registry Routines */
CSSM_LIST_PTR CSSMAPI cssm_reg_GetModuleList(
                                    CSSM_SERVICE_MASK ServiceMask,
                                    CSSM_BOOL MatchAll);
CSSM_RETURN CSSMAPI cssm_reg_FreeModuleList (CSSM_LIST_PTR List);
CSSM_MODULE_INFO_PTR CSSMAPI cssm_reg_GetModuleInfo (
                                    const CSSM_GUID_PTR ModuleGUID,
                                    CSSM_SERVICE_MASK ServiceMask,
                                    uint32 SubserviceID,
                                    CSSM_INFO_LEVEL InfoLevel);
CSSM_RETURN CSSMAPI cssm_reg_FreeModuleInfo (CSSM_MODULE_INFO_PTR ModuleInfo);
CSSM_BOOL   CSSMAPI cssm_reg_HasDynamicModuleInfo (
                                    const CSSM_GUID_PTR ModuleGUID,
                                    CSSM_SERVICE_MASK ServiceMask,
                                    uint32 SubserviceID,
                                    CSSM_INFO_LEVEL InfoLevel);
CSSM_RETURN CSSMAPI cssm_reg_GetVersion (
                                    const CSSM_GUID_PTR ModuleGUID, 
                                    uint32 *Major, uint32 *Minnor);
CSSM_CSSMINFO_PTR CSSMAPI cssm_reg_GetCSSMInfo (
                                    const CSSM_MEMORY_FUNCS_PTR MemoryFunctions,
                                    uint32 *NumCssmInfos); 
CSSM_RETURN CSSMAPI cssm_reg_FreeCSSMInfo (
                                    const CSSM_CSSMINFO_PTR CssmInfo,
                                    const CSSM_MEMORY_FUNCS_PTR MemoryFunctions,
                                    uint32 NumCssmInfos);
CSSM_RETURN cssm_GetManifestAndSection(
                                    CSSM_GUID_PTR pGuid, 
                                    void** Manifest, 
                                    char **szManifestSection);

/* Pointer Verification Routines */
CSSM_BOOL CSSMAPI cssm_IsBadStrPtr   (const char *str, uint32  length);
CSSM_BOOL CSSMAPI cssm_IsBadCodePtr  (CSSM_CALLBACK  code_ptr);
CSSM_BOOL CSSMAPI cssm_IsBadReadPtr  (const void * ptr, uint32 length);
CSSM_BOOL CSSMAPI cssm_IsBadWritePtr (void * ptr, uint32 length);

/* Time Routines */
uint32 CSSMAPI cssm_GetTime();
void cssm_GetDate(CSSM_DATE_PTR date);
sint32 cssm_GetSecondsSince1970();
sint32 cssm_mktime(CSSM_DATE_AND_TIME_PTR time_to_convert);

/* String-handling Routines */
sint32 CSSMAPI cssm_strcmp(const char *, const char *);
char * CSSMAPI cssm_strcpy(char *, const char *);

/* VO Registry Routines */
CSSM_VL_VO_REGISTRY_INFO_PTR CSSMAPI 
cssm_vlreg_GetNestedVoRegistryInfo (const CSSM_VO_UID_PTR VoIdentifier,
                                    uint32 MaxDepth);

/* Testing Routines */
#if defined (TEST_ALLOW_ALLOC_FAIL)
uint32 test_cause_alloc_fail(void *allocRef, sint32 fail_on);
#endif

#endif /* #ifndef CSSM_BIS */

#ifdef __cplusplus
}
#endif

#endif
