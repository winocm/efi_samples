/*-----------------------------------------------------------------------
 *      File:   smvl_main.c
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
#include "smvl.h"


CSSM_RETURN 
CSSMVLI 
VL_Initialize(
    CSSM_MODULE_HANDLE Handle, 
    uint32 VerMajor, 
    uint32 VerMinor);

CSSM_RETURN 
CSSMVLI 
VL_Uninitialize(
    CSSM_MODULE_HANDLE Handle);

/* VL Global Data Structures */
CSSM_SPI_MEMORY_FUNCS  g_fx;
ISL_MEMORY_METHODS gISL_mem_funcs;
SMVL_CONTEXT_PTR gsContextPtr = NULL;

/* VL wrapper memory functions */
static void * isl_malloc(uint32 Size, void *AllocRef)
{
    return g_fx.malloc_func((CSSM_HANDLE)AllocRef, Size);
}
static void isl_free(void *MemPtr, void *AllocRef)
{
    g_fx.free_func((CSSM_HANDLE)AllocRef, MemPtr);
}
static void * isl_realloc(void *MemPtr, uint32 Size, void *AllocRef)
{
    return g_fx.realloc_func((CSSM_HANDLE)AllocRef, MemPtr, Size);
}
static void * isl_calloc(uint32 Num, uint32 Size, void *AllocRef)
{
    return g_fx.calloc_func((CSSM_HANDLE)AllocRef, Num, Size);
}

/* Local #defines */
#define USE_CSSM_MEM_FUNCS


static const CSSM_SPI_VL_FUNCS FunctionTable =
{
    VL_InstantiateVoFromLocation,
    VL_FreeVo,
    VL_GetDoInfoByName,
    VL_FreeDoInfos,
    VL_GetFirstSignatureInfo,
    VL_AbortScan,
    VL_FreeSignatureInfo,
    VL_SetDoLMapEntries,
    VL_VerifyRootCredentialsDataAndContainment,
    VL_SelfVerifyCertificate
};

/*-----------------------------------------------------------------------------
 * Name: AddInAuthenticate
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMAPI B2CC8C10_F00B_11d1_878B_00A0C91A2629_AddInAuthenticate
     (const char* cssmCredentialPath, 
      const char* cssmSection,
      const char* AppCredential, 
      const char* AppSection)
{
    CSSM_REGISTRATION_INFO VLRegInfo;
    CSSM_MODULE_FUNCS      Services;
    cssmCredentialPath;    cssmSection;
    AppCredential;         AppSection;

    /* Register the VL function table with the authenticated CSSM */
    /* Fill in Registration information */
    VLRegInfo.Initialize            = VL_Initialize;  
    VLRegInfo.Terminate             = VL_Uninitialize;
    VLRegInfo.EventNotify           = NULL;
    VLRegInfo.GetModuleInfo         = NULL;
    VLRegInfo.FreeModuleInfo        = NULL;
    VLRegInfo.ThreadSafe            = CSSM_FALSE;
    VLRegInfo.ServiceSummary        = CSSM_SERVICE_VL;
    VLRegInfo.NumberOfServiceTables = 1;
    VLRegInfo.Services              = &Services;

    /* Fill in Services */
    Services.ServiceType  = CSSM_SERVICE_VL;
    Services.FunctionTable.VlFuncs = (CSSM_SPI_VL_FUNCS_PTR)&FunctionTable;

    /* Fill in Function Table */
  
   /* register its services with the CSSM */
    return CSSM_RegisterServices(
                &intel_preos_vlm_guid, 
                &VLRegInfo, 
                &g_fx, NULL); 
}

/*-----------------------------------------------------------------------------
 * Name: CloneSignVerifyMethod
 *
 * Description:
 *
 * Parameters:
 * Handle (input)
 * TemplatePtr (input)
 * CSPHandle (input)
 *
 * Return value:
 * ISL_SIGN_VERIFY_METHODS_PTR
 * NULL - if call is unable to clone sign/verify methods
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
#pragma warning (disable: 4100)
ISL_SIGN_VERIFY_METHODS_PTR
CloneSignVerifyMethod(
    CSSM_MODULE_HANDLE Handle,
    ISL_SIGN_VERIFY_METHODS_PTR TemplatePtr,
    CSSM_CSP_HANDLE CSPHandle)
{
    ISL_SIGN_VERIFY_METHODS_PTR MethodPtr;
    
    if (TemplatePtr == NULL) return NULL;
#ifdef USE_CSSM_MEM_FUNCS
    MethodPtr = cssm_malloc(sizeof(ISL_SIGN_VERIFY_METHODS), NULL);
#else
    MethodPtr = g_fx.malloc_func(Handle, sizeof(ISL_SIGN_VERIFY_METHODS));
#endif
    if (MethodPtr == NULL) goto FAIL;
    cssm_memcpy(MethodPtr, TemplatePtr, sizeof(ISL_SIGN_VERIFY_METHODS));
    MethodPtr->CSPHandle = CSPHandle;
    return MethodPtr;

FAIL:
    {
#ifdef USE_CSSM_MEM_FUNCS
        cssm_free(MethodPtr, NULL);
#else
        g_fx.free_func(Handle, MethodPtr);
#endif
    return NULL;
    }
}
#pragma warning (default: 4100)

/*-----------------------------------------------------------------------------
 * Name: CloneDigestMethod
 *
 * Description:
 *
 * Parameters:
 * Handle (input)
 * TemplatePtr (input)
 * CSPHandle (input)
 *
 * Return value:
 * ISL_DIGEST_METHODS_PTR
 * NULL - if unable to clone digest method
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
#pragma warning (disable: 4100)
ISL_DIGEST_METHODS_PTR
CloneDigestMethod(
    CSSM_MODULE_HANDLE Handle,
    ISL_DIGEST_METHODS_PTR TemplatePtr,
    CSSM_MODULE_HANDLE CSPHandle)
{
    ISL_DIGEST_METHODS_PTR MethodPtr = NULL;

    if (TemplatePtr == NULL) return NULL;
#ifdef USE_CSSM_MEM_FUNCS
    MethodPtr = cssm_malloc(sizeof(ISL_DIGEST_METHODS), NULL);
#else
    MethodPtr = g_fx.malloc_func(Handle, sizeof(ISL_DIGEST_METHODS));
#endif
    if (MethodPtr == NULL) goto FAIL;
    cssm_memcpy(MethodPtr, TemplatePtr, sizeof(ISL_DIGEST_METHODS));
    MethodPtr->CSPHandle = CSPHandle;
    return MethodPtr;

FAIL:
    {
    #ifdef USE_CSSM_MEM_FUNCS
        cssm_free(MethodPtr, NULL);
    #else
        g_fx.free_func(Handle, MethodPtr);
    #endif
        return NULL;
    }
}
#pragma warning (default: 4100)

/*-----------------------------------------------------------------------------
 * Name: InitializeArchiveConfig
 *
 * Description:
 *
 * Parameters:
 * Handle (input)
 * smvlContextPtr (input)
 *
 * Return value:
 * CSSM_OK
 * CSSM_FAIL
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_RETURN
InitializeArchiveConfig(
    CSSM_MODULE_HANDLE Handle,
    SMVL_CONTEXT_PTR smvlContextPtr)
{

    CSSM_VERSION csp_version = {INTEL_CSM_MAJOR_VER, INTEL_CSM_MINOR_VER};
    uint32 size;
    ISL_STATUS status;

    if (smvlContextPtr == NULL) return CSSM_FAIL;
    cssm_memset(smvlContextPtr, 0, sizeof(SMVL_CONTEXT_PTR));

    smvlContextPtr->CSPHandle = CSSM_ModuleAttach (
        &intel_preos_csm_guid, 
        &csp_version,
        NULL, 
        0, 
        0, 
        0, 
        NULL, 
        NULL, 
        NULL, 
        NULL);

    if (smvlContextPtr->CSPHandle == 0) return CSSM_FAIL;

    /* Initializing Signature Methods */

#ifdef USE_CSSM_MEM_FUNCS
    smvlContextPtr->PKCS7SignatureMethods = cssm_malloc(
        sizeof(ISL_SIGNATURE_METHODS), 
        NULL);
#else
    smvlContextPtr->PKCS7SignatureMethods = g_fx.malloc_func(
        Handle, 
        sizeof(ISL_SIGNATURE_METHODS));
#endif
    if (smvlContextPtr->PKCS7SignatureMethods == NULL) goto FAIL;
    cssm_memcpy(
        smvlContextPtr->PKCS7SignatureMethods, 
        &PKCS7SignatureMethods, 
        sizeof(ISL_SIGNATURE_METHODS));
#ifdef USE_CSSM_MEM_FUNCS
    smvlContextPtr->PKCS7SignatureClass = cssm_malloc(sizeof(ISL_CLASS), NULL);
#else
    smvlContextPtr->PKCS7SignatureClass = g_fx.malloc_func(
        Handle, 
        sizeof(ISL_CLASS));
#endif
    if (smvlContextPtr->PKCS7SignatureClass == NULL) goto FAIL;
    smvlContextPtr->PKCS7SignatureMethods->ServiceMethods.Class = 
        smvlContextPtr->PKCS7SignatureClass;

    size = (uint32)ArchiveConfigMethods.SizeofObject();
#ifdef USE_CSSM_MEM_FUNCS
    smvlContextPtr->PKCS7Config = cssm_malloc(size, NULL);
#else
    smvlContextPtr->PKCS7Config = g_fx.malloc_func(Handle, size);
#endif
    if (smvlContextPtr->PKCS7Config == NULL) goto FAIL;
    smvlContextPtr->PKCS7SignatureMethods->ServiceMethods.Class->ClassContext 
        = smvlContextPtr->PKCS7Config;

    /* Initializing JARSHA1 Methods */
    smvlContextPtr->JarSHA1Methods = CloneDigestMethod(
                        Handle,
                        &JarSHA1Methods, 
                        smvlContextPtr->CSPHandle);
    if (smvlContextPtr->JarSHA1Methods == NULL) goto FAIL;

    /* Initializing PKCS7DSA Methods */
    smvlContextPtr->PKCS7DSAMethods = CloneSignVerifyMethod(
                        Handle,
                        &PKCS7DSAMethods, 
                        smvlContextPtr->CSPHandle);
    if (smvlContextPtr->PKCS7DSAMethods == NULL) goto FAIL;

    size = (uint32)ArchiveConfigMethods.SizeofObject();
#ifdef USE_CSSM_MEM_FUNCS
    smvlContextPtr->SMConfig = cssm_malloc(size, NULL);
#else
    smvlContextPtr->SMConfig = g_fx.malloc_func(Handle, size);
#endif
    if (smvlContextPtr->SMConfig == NULL) goto FAIL;
    status = ArchiveConfigMethods.Initialize(
        smvlContextPtr->SMConfig,
        &gISL_mem_funcs);
    if (status != ISL_OK) goto FAIL;

#if 0
    status = ArchiveConfigMethods.AddAlgorithm(
        smvlContextPtr->SMConfig,
        (ISL_SERVICE_CLASS_METHODS*)&getCSSMDataMethods);
    if (status != ISL_OK) goto FAIL;
#endif

    status = ArchiveConfigMethods.AddAlgorithm(
        smvlContextPtr->SMConfig,
        (ISL_SERVICE_CLASS_METHODS*)&getMemoryMethods);
    if (status != ISL_OK) goto FAIL;

    status = ArchiveConfigMethods.AddAlgorithm(
        smvlContextPtr->SMConfig,
        (ISL_SERVICE_CLASS_METHODS*)&X509CertMethods);
    if (status != ISL_OK) goto FAIL;

    status = ArchiveConfigMethods.AddAlgorithm(
        smvlContextPtr->SMConfig,
        (ISL_SERVICE_CLASS_METHODS*)smvlContextPtr->PKCS7SignatureMethods);
    if (status != ISL_OK) goto FAIL;
        
    status = ArchiveConfigMethods.AddAlgorithm(
        smvlContextPtr->SMConfig,
        (ISL_SERVICE_CLASS_METHODS*)smvlContextPtr->JarSHA1Methods);
    if (status != ISL_OK) goto FAIL;

    status = ArchiveConfigMethods.Initialize(
        smvlContextPtr->PKCS7Config,
        &gISL_mem_funcs);
    if (status != ISL_OK) goto FAIL;

    status = ArchiveConfigMethods.AddAlgorithm(
        smvlContextPtr->PKCS7Config,
        (ISL_SERVICE_CLASS_METHODS*)smvlContextPtr->PKCS7DSAMethods);

    if (status != ISL_OK) goto FAIL;
    return(ISL_OK);

FAIL:
    {
        VL_Uninitialize(Handle);
        return CSSM_FAIL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: VL_Initialize
 *
 * Description:
 * This function initializes the VL by checking that the app has requested
 * a supported version and by initializing the supporting libraries & 
 * structures, such as the query table, der encode/decode functions, and 
 * transport functions.
 * 
 * Parameters: 
 * VerMajor (input) - The major version number to be compared for compatiblity
 * VerMinor (input) - The minor version number to be compared for compatiblity
 *
 * Return value:
 * An indicator of whether or not the VL was initialized
 *
 * Error Codes:
 * CSSM_VL_INITIALIZE_FAIL
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMVLI VL_Initialize
    (CSSM_MODULE_HANDLE Handle, 
     uint32 VerMajor, 
     uint32 VerMinor)
{
    /* CheckVersion */
    if (VerMajor != INTEL_SMV2_VLM_MAJOR_VER || 
        VerMinor != INTEL_SMV2_VLM_MINOR_VER)
    {
        CSSM_SetError(&intel_preos_vlm_guid, CSSM_INCOMPATIBLE_VERSION);
        return CSSM_FAIL;
    }

    if (gsContextPtr != NULL)
    {
        return CSSM_OK;
    }

    /* Need to Initialize the Global Configuration */
    gISL_mem_funcs.malloc_func = isl_malloc;
    gISL_mem_funcs.free_func = isl_free;
    gISL_mem_funcs.realloc_func = isl_realloc;
    gISL_mem_funcs.calloc_func = isl_calloc;
    gISL_mem_funcs.AllocRef = (void*)Handle;
#ifdef USE_CSSM_MEM_FUNCS
    gsContextPtr = cssm_malloc(sizeof(SMVL_CONTEXT), NULL);
#else
    gsContextPtr = g_fx.malloc_func(Handle, sizeof(SMVL_CONTEXT));
#endif
    if (gsContextPtr == NULL) goto FAIL;
    cssm_memset(gsContextPtr, 0, sizeof(SMVL_CONTEXT));
    if (InitializeArchiveConfig(Handle, gsContextPtr) != CSSM_OK) goto FAIL;

    return CSSM_OK;
FAIL:
    {
#ifdef USE_CSSM_MEM_FUNCS
        cssm_free(gsContextPtr, NULL);
#else
        g_fx.free_func(Handle, gsContextPtr);
#endif
        gsContextPtr = NULL;
        return CSSM_FAIL;
    }

}


/*-----------------------------------------------------------------------------
 * Name: VL_Uninitialize
 *
 * Description:
 * Uninitializes the VL by uninitializing the supporting libraries 
 * & structures, such as the query table, der encode/decode functions, and 
 * transport functions.
 * 
 * Parameters: 
 *
 * Return value:
 * An indicator of whether or not the VL was uninitialized
 *
 * Error Codes:
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMVLI VL_Uninitialize(CSSM_MODULE_HANDLE Handle)
#pragma warning (disable: 4100)
{

    if (gsContextPtr == NULL) return CSSM_OK;

    CSSM_ModuleDetach(gsContextPtr->CSPHandle);
    ArchiveConfigMethods.Recycle(gsContextPtr->PKCS7Config);
    ArchiveConfigMethods.Recycle(gsContextPtr->SMConfig);
#ifdef USE_CSSM_MEM_FUNCS
    cssm_free(gsContextPtr->PKCS7SignatureMethods, NULL);
    cssm_free(gsContextPtr->JarSHA1Methods, NULL);
    cssm_free(gsContextPtr->PKCS7DSAMethods, NULL);
    cssm_free(gsContextPtr->PKCS7SignatureClass, NULL);
    cssm_free(gsContextPtr->PKCS7Config, NULL);
    cssm_free(gsContextPtr->SMConfig, NULL);
    cssm_free(gsContextPtr, NULL);
#else
    g_fx.free_func(Handle, gsContextPtr->PKCS7SignatureMethods);
    g_fx.free_func(Handle, gsContextPtr->JarSHA1Methods);
    g_fx.free_func(Handle, gsContextPtr->PKCS7DSAMethods);
    g_fx.free_func(Handle, gsContextPtr->PKCS7SignatureClass);
    g_fx.free_func(Handle, gsContextPtr->PKCS7Config);
    g_fx.free_func(Handle, gsContextPtr->SMConfig);
    g_fx.free_func(Handle, gsContextPtr);
#endif
    gsContextPtr = NULL;

    CSSM_ClearError();
    return CSSM_OK;
}
#pragma warning (default: 4100)

