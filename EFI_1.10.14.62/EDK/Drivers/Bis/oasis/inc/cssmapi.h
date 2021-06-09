/* SCCSID: inc/cssmapi.h, dss_cdsa_fwk, fwk_rel2, dss_971010 1.21 10/23/97 17:53:31 */
/*-----------------------------------------------------------------------
 *      File:   CSSMAPI.H
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
/*
 * (C) COPYRIGHT International Business Machines Corp. 1996, 1997
 * All Rights Reserved
 * Licensed Materials - Property of IBM
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
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _CSSMAPI_H
#define _CSSMAPI_H    

#include "cssmdefs.h"

/* API Functions */
#ifdef __cplusplus
extern "C" {
#endif

/* CSSM functions */
/*
 * Core Functions.
 */
CSSM_RETURN       CSSMAPI CSSM_Init (
                                const CSSM_VERSION_PTR Version,
                                const void * Reserved);

CSSM_CSSMINFO_PTR CSSMAPI CSSM_GetInfo (
                                const CSSM_MEMORY_FUNCS_PTR MemoryFunctions,
                                uint32 *NumCssmInfos); 

CSSM_RETURN       CSSMAPI CSSM_FreeInfo (  
                                const CSSM_CSSMINFO_PTR CssmInfo,
                                const CSSM_MEMORY_FUNCS_PTR MemoryFunctions,
                                uint32 NumCssmInfos);

/* Module Management functions */
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
                                const void * Reserved);

CSSM_RETURN          CSSMAPI CSSM_ModuleDetach (
                                CSSM_MODULE_HANDLE ModuleHandle);

CSSM_LIST_PTR        CSSMAPI CSSM_ListModules (
                                CSSM_SERVICE_MASK ServiceMask,
                                CSSM_BOOL MatchAll);

CSSM_RETURN          CSSMAPI CSSM_FreeList (CSSM_LIST_PTR List);

CSSM_MODULE_INFO_PTR CSSMAPI CSSM_GetModuleInfo (
                                const CSSM_GUID_PTR ModuleGUID,
                                CSSM_SERVICE_MASK ServiceMask,
                                uint32 SubserviceID,
                                CSSM_INFO_LEVEL InfoLevel);

CSSM_RETURN          CSSMAPI CSSM_FreeModuleInfo (
                                CSSM_MODULE_INFO_PTR ModuleInfo);

/* Error Handling */
CSSM_ERROR_PTR CSSMAPI CSSM_GetError  (void);  

CSSM_RETURN    CSSMAPI CSSM_SetError  (CSSM_GUID_PTR guid, 
                                       uint32  error);
void           CSSMAPI CSSM_ClearError(void);          

/* Utility Functions */
void        CSSMAPI CSSM_Free         (void *MemPtr, 
                                       CSSM_HANDLE AddInHandle);
CSSM_RETURN CSSMAPI CSSM_GetAPIMemoryFunctions (
                                       CSSM_HANDLE AddInHandle,
                                       CSSM_API_MEMORY_FUNCS_PTR AppMemoryFuncs);
CSSM_BOOL   CSSMAPI CSSM_CompareGuids (CSSM_GUID guid1,CSSM_GUID guid2);


/* Cryptographic Context API */
CSSM_CC_HANDLE CSSMAPI CSSM_CSP_CreateSignatureContext (
                                       CSSM_CSP_HANDLE CSPHandle,
                                       uint32 AlgorithmID,
                                       const CSSM_CRYPTO_DATA_PTR PassPhrase,
                                       const CSSM_KEY_PTR Key);

CSSM_CC_HANDLE CSSMAPI CSSM_CSP_CreateSymmetricContext (
                                       CSSM_CSP_HANDLE CSPHandle,
                                       uint32 AlgorithmID,
                                       uint32 Mode,
                                       const CSSM_KEY_PTR Key,
                                       const CSSM_DATA_PTR InitVector, 
                                       uint32 Padding,
                                       uint32 Params);

CSSM_CC_HANDLE CSSMAPI CSSM_CSP_CreateDigestContext (
                                       CSSM_CSP_HANDLE CSPHandle,
                                       uint32 AlgorithmID);

CSSM_CC_HANDLE CSSMAPI CSSM_CSP_CreateRandomGenContext (
                                       CSSM_CSP_HANDLE CSPHandle,
                                       uint32 AlgorithmID,
                                       const CSSM_CRYPTO_DATA_PTR Seed,
                                       uint32 Length);

CSSM_CC_HANDLE CSSMAPI CSSM_CSP_CreateAsymmetricContext (
                                       CSSM_CSP_HANDLE CSPHandle,
                                       uint32 AlgorithmID,
                                       const CSSM_CRYPTO_DATA_PTR PassPhrase,
                                       const CSSM_KEY_PTR Key,
                                       uint32 Padding);

CSSM_CC_HANDLE CSSMAPI CSSM_CSP_CreateKeyGenContext (
                                       CSSM_CSP_HANDLE CSPHandle,
                                       uint32 AlgorithmID,
                                       const CSSM_CRYPTO_DATA_PTR PassPhrase,
                                       uint32 KeySizeInBits,
                                       const CSSM_CRYPTO_DATA_PTR Seed,
                                       const CSSM_DATA_PTR Salt,
                                       const CSSM_DATE_PTR StartDate,
                                       const CSSM_DATE_PTR EndDate,
                                       const CSSM_DATA_PTR Params);

CSSM_CC_HANDLE CSSMAPI CSSM_CSP_CreatePassThroughContext (
                                       CSSM_CSP_HANDLE CSPHandle,
                                       const CSSM_KEY_PTR Key,
                                       const CSSM_DATA_PTR ParamBufs,
                                       uint32 ParamBufCount);

CSSM_RETURN CSSMAPI CSSM_DeleteContext(CSSM_CC_HANDLE CCHandle);


/*
 * Cryptographic Sessions and Logon API
 */
CSSM_RETURN CSSMAPI CSSM_CSP_Login    (CSSM_CSP_HANDLE CSPHandle,
                                       const CSSM_CRYPTO_DATA_PTR Password,
                                       const CSSM_DATA_PTR pReserved);

CSSM_RETURN CSSMAPI CSSM_CSP_Logout   (CSSM_CSP_HANDLE CSPHandle);

CSSM_RETURN CSSMAPI CSSM_CSP_ChangeLoginPassword (
                                       CSSM_CSP_HANDLE CSPHandle,
                                       const CSSM_CRYPTO_DATA_PTR OldPassword,
                                       const CSSM_CRYPTO_DATA_PTR NewPassword);


/* 
 * Cryptography Operations API 
 */

/* Returns a signature item */
CSSM_RETURN CSSMAPI CSSM_SignData     (CSSM_CC_HANDLE CCHandle,
                                       const CSSM_DATA_PTR DataBufs,
                                       uint32 DataBufCount,
                                       CSSM_DATA_PTR Signature);

CSSM_BOOL   CSSMAPI CSSM_VerifyData   (CSSM_CC_HANDLE CCHandle,
                                       const CSSM_DATA_PTR DataBufs,
                                       uint32 DataBufCount,
                                       const CSSM_DATA_PTR Signature);

/* Returns a digest item */
CSSM_RETURN CSSMAPI CSSM_DigestData   (CSSM_CC_HANDLE CCHandle,
                                       const CSSM_DATA_PTR DataBufs,
                                       uint32 DataBufCount,
                                       CSSM_DATA_PTR Digest);

CSSM_RETURN CSSMAPI CSSM_QuerySize    (CSSM_CC_HANDLE CCHandle,
                                       CSSM_BOOL Encrypt,
                                       uint32 QuerySizeCount,
                                       CSSM_QUERY_SIZE_DATA_PTR DataBlock);

/* Returns the encrypted data */
CSSM_RETURN CSSMAPI CSSM_EncryptData  (CSSM_CC_HANDLE CCHandle,
                                       const CSSM_DATA_PTR ClearBufs,
                                       uint32 ClearBufCount,
                                       CSSM_DATA_PTR CipherBufs,
                                       uint32 CipherBufCount,
                                       uint32 *bytesEncrypted,
                                       CSSM_DATA_PTR RemData);

/* Returns the decrypted data */
CSSM_RETURN CSSMAPI CSSM_DecryptData  (const CSSM_CC_HANDLE CCHandle,
                                       const CSSM_DATA_PTR CipherBufs,
                                       uint32 CipherBufCount,
                                       CSSM_DATA_PTR ClearBufs,
                                       uint32 ClearBufCount,
                                       uint32 *bytesDecrypted,
                                       CSSM_DATA_PTR RemData);

/* Query to get the key size */
CSSM_RETURN CSSMAPI CSSM_QueryKeySizeInBits (
                                       CSSM_CSP_HANDLE CSPHandle,
                                       CSSM_CC_HANDLE CCHandle,
                                       const CSSM_KEY_PTR Key,
                                       CSSM_KEY_SIZE_PTR KeySize);

/* Returns the symmetric key */
CSSM_RETURN CSSMAPI CSSM_GenerateKey  (CSSM_CC_HANDLE CCHandle,
                                       uint32 KeyUsage,
                                       uint32 KeyAttr,
                                       const CSSM_DATA_PTR KeyLabel,
                                       CSSM_KEY_PTR Key);

/* Returns the asymmetric key pair */
CSSM_RETURN CSSMAPI CSSM_GenerateKeyPair (
                                       CSSM_CC_HANDLE CCHandle,
                                       uint32 PublicKeyUsage,
                                       uint32 PublicKeyAttr,
                                       const CSSM_DATA_PTR PublicKeyLabel,
                                       CSSM_KEY_PTR PublicKey,
                                       uint32 PrivateKeyUsage,
                                       uint32 PrivateKeyAttr,
                                       const CSSM_DATA_PTR PrivateKeyLabel,
                                       CSSM_KEY_PTR PrivateKey);

/* Returns the random data */
CSSM_RETURN CSSMAPI CSSM_GenerateRandom(CSSM_CC_HANDLE CCHandle,
                                       CSSM_DATA_PTR RandomNumber);

CSSM_RETURN CSSMAPI CSSM_CSP_ObtainPrivateKeyFromPublicKey (
                                       CSSM_CSP_HANDLE CSPHandle,
                                       const CSSM_KEY_PTR PublicKey,
                                       CSSM_KEY_PTR PrivateKey);

CSSM_RETURN CSSMAPI CSSM_WrapKey      (CSSM_CC_HANDLE CCHandle,
                                       const CSSM_CRYPTO_DATA_PTR PassPhrase, 
                                       const CSSM_KEY_PTR Key,
                                       const CSSM_DATA_PTR DescriptiveData,
                                       CSSM_WRAP_KEY_PTR WrappedKey);

CSSM_RETURN CSSMAPI CSSM_UnwrapKey    (CSSM_CC_HANDLE CCHandle,
                                       const CSSM_CRYPTO_DATA_PTR NewPassPhrase,
                                       const CSSM_KEY_PTR PublicKey,
                                       const CSSM_WRAP_KEY_PTR WrappedKey,
                                       uint32 KeyUsage,
                                       uint32 KeyAttr,
                                       const CSSM_DATA_PTR KeyLabel,
                                       CSSM_KEY_PTR UnwrappedKey,
                                       CSSM_DATA_PTR DescriptiveData);

CSSM_RETURN CSSMAPI CSSM_FreeKey      (CSSM_CSP_HANDLE CSPHandle,
                                       CSSM_KEY_PTR KeyPtr);

CSSM_RETURN CSSMAPI CSSM_GenerateAlgorithmParams 
                                      (CSSM_CC_HANDLE CCHandle,
                                       uint32 ParamBits,
                                       CSSM_DATA_PTR Param);

/*
 * Miscellaneous Functions 
 */
CSSM_RETURN CSSMAPI CSSM_RetrieveUniqueId (CSSM_CSP_HANDLE CSPHandle,
                                           CSSM_DATA_PTR UniqueID);

CSSM_RETURN CSSMAPI CSSM_RetrieveCounter (CSSM_CSP_HANDLE CSPHandle,
                                          CSSM_DATA_PTR Counter);

CSSM_RETURN CSSMAPI CSSM_VerifyDevice (CSSM_CSP_HANDLE CSPHandle,
                                       CSSM_DATA_PTR DeviceCert);
/*
 * Extensibility Functions
 */
void * CSSMAPI CSSM_CSP_PassThrough (CSSM_CC_HANDLE CCHandle, 
                                     uint32 PassThroughId,
                                     const void *InData);

/* Trust Policy API */
/* Trust Policy Operations */
CSSM_BOOL CSSMAPI CSSM_TP_CertGroupVerify (
                                      CSSM_TP_HANDLE TPHandle,
                                      CSSM_CL_HANDLE CLHandle,
                                      CSSM_CSP_HANDLE CSPHandle,
                                      const CSSM_DL_DB_LIST_PTR DBList,
                                      const CSSM_CERTGROUP_PTR CertGroupToBeVerified,
                                      const CSSM_VERIFYCONTEXT_PTR VerifyContext); 
                                  
CSSM_BOOL CSSMAPI CSSM_TP_CrlVerify (CSSM_TP_HANDLE TPHandle,
                                CSSM_CL_HANDLE CLHandle,
                                CSSM_CSP_HANDLE CSPHandle,
                                const CSSM_DL_DB_LIST_PTR DBList,
                                const CSSM_DATA_PTR CrlToBeVerified,
                                CSSM_CRL_TYPE CrlType, 
                                CSSM_CRL_ENCODING CrlEncoding,
                                const CSSM_CERTGROUP_PTR SignerCertGroup,
                                const CSSM_VERIFYCONTEXT_PTR VerifyContext);

CSSM_RETURN CSSMAPI CSSM_TP_ApplyCrlToDb (CSSM_TP_HANDLE TPHandle,
                                CSSM_CL_HANDLE CLHandle,
                                CSSM_CSP_HANDLE CSPHandle,
                                const CSSM_DL_DB_LIST_PTR DBList,
                                const CSSM_DATA_PTR CrlToBeApplied,
                                CSSM_CRL_TYPE CrlType,
                                CSSM_CRL_ENCODING CrlEncoding,
                                const CSSM_CERTGROUP_PTR SignerCert,
                                const CSSM_VERIFYCONTEXT_PTR SignerVerifyContext);

/*
 * Extensibility Functions
 */

void * CSSMAPI CSSM_TP_PassThrough (CSSM_TP_HANDLE TPHandle, 
                                    CSSM_CL_HANDLE CLHandle, 
                                    CSSM_CSP_HANDLE CSPHandle, 
                                    const CSSM_DL_DB_LIST_PTR DBList,
                                    uint32 PassThroughId,
                                    const void *InputParams);


/* Certificate Library API */
/* Certificate Operations */
CSSM_BOOL CSSMAPI CSSM_CL_CertVerify (CSSM_CL_HANDLE CLHandle, 
                                      CSSM_CC_HANDLE CCHandle, 
                                      const CSSM_DATA_PTR CertToBeVerified, 
                                      const CSSM_DATA_PTR SignerCert, 
                                      const CSSM_FIELD_PTR VerifyScope,
                                      uint32 ScopeSize);
/* Returns the field value */
CSSM_DATA_PTR CSSMAPI CSSM_CL_CertGetFirstFieldValue (
                                                 CSSM_CL_HANDLE CLHandle, 
                                                 const CSSM_DATA_PTR Cert, 
                                                 const CSSM_OID_PTR CertField,
                                                 CSSM_HANDLE_PTR ResultsHandle,
                                                 uint32 *NumberOfMatchedFields);
/* Returns the field value */
CSSM_DATA_PTR CSSMAPI CSSM_CL_CertGetNextFieldValue (CSSM_CL_HANDLE CLHandle, 
                                                     CSSM_HANDLE ResultsHandle);

CSSM_RETURN CSSMAPI CSSM_CL_CertAbortQuery (CSSM_CL_HANDLE CLHandle, 
                                            CSSM_HANDLE ResultsHandle);

CSSM_KEY_PTR CSSMAPI CSSM_CL_CertGetKeyInfo (CSSM_CL_HANDLE CLHandle, 
                                             const CSSM_DATA_PTR Cert);

/*
 * Certificate Revocation List Operations
 */
CSSM_BOOL CSSMAPI CSSM_CL_CrlVerify (CSSM_CL_HANDLE CLHandle, 
                                     CSSM_CC_HANDLE CCHandle, 
                                     const CSSM_DATA_PTR CrlToBeVerified, 
                                     const CSSM_DATA_PTR SignerCert,
                                     const CSSM_FIELD_PTR VerifyScope,
                                     uint32 ScopeSize);

CSSM_BOOL CSSMAPI CSSM_CL_IsCertInCrl (CSSM_CL_HANDLE CLHandle, 
                                       const CSSM_DATA_PTR Cert, 
                                       const CSSM_DATA_PTR Crl);

/* Returns the first item in the CRL */
CSSM_DATA_PTR CSSMAPI CSSM_CL_CrlGetFirstFieldValue (
                                               CSSM_CL_HANDLE CLHandle, 
                                               const CSSM_DATA_PTR Crl,
                                               const CSSM_OID_PTR CrlField,
                                               CSSM_HANDLE_PTR ResultsHandle,
                                               uint32 *NumberOfMatchedFields);

/* Returns the next item in the CRL */
CSSM_DATA_PTR CSSMAPI CSSM_CL_CrlGetNextFieldValue (CSSM_CL_HANDLE CLHandle, 
                                                    CSSM_HANDLE ResultsHandle);

CSSM_RETURN CSSMAPI CSSM_CL_CrlAbortQuery (CSSM_CL_HANDLE CLHandle, 
                                           CSSM_HANDLE ResultsHandle);

/*
 * Extensibility Functions 
 */

void * CSSMAPI CSSM_CL_PassThrough (CSSM_CL_HANDLE CLHandle, 
                                    CSSM_CC_HANDLE CCHandle, 
                                    uint32 PassThroughId, 
                                    const void * InputParams);


/*
 * Data Storage API
 */

CSSM_DB_HANDLE CSSMAPI CSSM_DL_DbOpen (CSSM_DL_HANDLE DLHandle,
                            const char *DbName,
                            const CSSM_NET_ADDRESS_PTR DbLocation,   
                            const CSSM_DB_ACCESS_TYPE_PTR AccessRequest,
                            const CSSM_USER_AUTHENTICATION_PTR UserAuthentication,
                            const void *OpenParameters);

CSSM_RETURN CSSMAPI CSSM_DL_DbClose (CSSM_DL_DB_HANDLE DLDBHandle);

CSSM_DB_HANDLE CSSMAPI CSSM_DL_DbCreate (CSSM_DL_HANDLE DLHandle,
                            const char *DbName,
                            const CSSM_NET_ADDRESS_PTR DbLocation,   
                            const CSSM_DBINFO_PTR DBInfo,
                            const CSSM_DB_ACCESS_TYPE_PTR AccessRequest,
                            const CSSM_USER_AUTHENTICATION_PTR UserAuthentication,
                            const void *OpenParameters); 

CSSM_RETURN CSSMAPI CSSM_DL_DbDelete (
                     CSSM_DL_HANDLE DLHandle,
                     const char *DbName,
                     const CSSM_NET_ADDRESS_PTR DbLocation,
                     const CSSM_USER_AUTHENTICATION_PTR UserAuthentication);

CSSM_RETURN CSSMAPI CSSM_DL_Authenticate (
                      CSSM_DL_DB_HANDLE DLDBHandle,
                      const CSSM_DB_ACCESS_TYPE_PTR AccessRequest,
                      const CSSM_USER_AUTHENTICATION_PTR UserAuthentication);

/*
 * Data Record operations API.
 */

CSSM_DB_UNIQUE_RECORD_PTR CSSMAPI CSSM_DL_DataInsert (
                           CSSM_DL_DB_HANDLE DLDBHandle,
                           CSSM_DB_RECORDTYPE RecordType,
                           const CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR Attributes, 
                           const CSSM_DATA_PTR Data);

CSSM_RETURN CSSMAPI CSSM_DL_DataDelete (
                       CSSM_DL_DB_HANDLE DLDBHandle,
                       const CSSM_DB_UNIQUE_RECORD_PTR UniqueRecordIdentifier);

CSSM_RETURN CSSMAPI CSSM_DL_DataModify (
                            CSSM_DL_DB_HANDLE DLDBHandle,
                            const CSSM_DB_RECORDTYPE RecordType,
                            CSSM_DB_UNIQUE_RECORD_PTR  UniqueRecordIdentifier,
                            CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR  AttributesToBeModified,
                            CSSM_DATA_PTR DataToBeModified);

CSSM_DB_UNIQUE_RECORD_PTR CSSMAPI CSSM_DL_DataGetFirst (
                       CSSM_DL_DB_HANDLE DLDBHandle,
                       const CSSM_QUERY_PTR Query,
                       CSSM_HANDLE_PTR ResultsHandle,
                       CSSM_BOOL  *EndOfDataStore,
                       CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR Attributes,
                       CSSM_DATA_PTR Data);

CSSM_DB_UNIQUE_RECORD_PTR CSSMAPI CSSM_DL_DataGetNext (
                                  CSSM_DL_DB_HANDLE DLDBHandle,
                                  CSSM_HANDLE ResultsHandle,
                                  CSSM_BOOL *EndOfDataStore,
                                  CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR Attributes, 
                                  CSSM_DATA_PTR Data);

CSSM_RETURN CSSMAPI CSSM_DL_AbortQuery (CSSM_DL_DB_HANDLE DLDBHandle,
                                        CSSM_HANDLE ResultsHandle);


CSSM_RETURN CSSMAPI CSSM_DL_DataGetFromUniqueRecordId (
                                    CSSM_DL_DB_HANDLE DLDBHandle,
                                    const CSSM_DB_UNIQUE_RECORD_PTR UniqueRecord,
                                    CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR Attributes,
                                    CSSM_DATA_PTR  Data);


CSSM_RETURN CSSMAPI CSSM_DL_FreeUniqueRecord (
                                       CSSM_DL_DB_HANDLE DLDBHandle,
                                       CSSM_DB_UNIQUE_RECORD_PTR UniqueRecord);

/*
 * Extensibility Functions 
 */

void * CSSMAPI CSSM_DL_PassThrough (CSSM_DL_DB_HANDLE DLDBHandle, 
                                    uint32 PassThroughId,
                                    const void * InputParams); 




/*
 * Verification Library Operations
 */
/* Registry operations */
CSSM_VOBUNDLE_UID_PTR CSSMAPI CSSM_VL_ImportVoBundle
                                    (const CSSM_VL_LOCATION_PTR VoBundleLocation,
                                     const CSSM_SUBSERVICE_UID_PTR VLModule);

CSSM_RETURN CSSMAPI CSSM_VL_DeleteVoBundle
                                    (const CSSM_VOBUNDLE_UID_PTR VoBundleId);

CSSM_VL_VOBUNDLE_REGISTRY_INFO_PTR CSSMAPI CSSM_VL_GetVoBundleRegistryInfo
                                    (const CSSM_VOBUNDLE_UID_PTR VoBundleId);

CSSM_RETURN CSSMAPI CSSM_VL_FreeVoBundleRegistryInfo
                                    (CSSM_VL_VOBUNDLE_REGISTRY_INFO_PTR VoBundleInfo);

CSSM_VL_VO_REGISTRY_INFO_PTR CSSMAPI CSSM_VL_GetNestedVoRegistryInfo
                                    (const CSSM_VO_UID_PTR VoIdentifier,
                                     uint32 MaxDepth);

CSSM_RETURN CSSMAPI CSSM_VL_FreeVoRegistryInfo
                                    (CSSM_VL_VO_REGISTRY_INFO_PTR VoInfo);


/* VLM operations */
CSSM_VO_HANDLE CSSMAPI CSSM_VL_InstantiateVoFromLocation
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_LOCATION_PTR VoBundleLocation,
                                     CSSM_VO_UID_PTR VoIdentifier,
                                     CSSM_STRING VoName);

CSSM_VO_HANDLE CSSMAPI CSSM_VL_InstantiateVo
                                    (CSSM_VL_HANDLE VLHandle,
                                     const CSSM_VOBUNDLE_UID_PTR VoBundleId,
                                     const CSSM_VO_UID_PTR VoIdentifier,
                                     uint32 MaxDepth);

CSSM_RETURN CSSMAPI CSSM_VL_FreeVo  (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle);

CSSM_VO_UID_BINDING_GROUP_PTR CSSMAPI CSSM_VL_GetNestedVoHandles
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle);

CSSM_RETURN CSSMAPI CSSM_VL_FreeVoHandles
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_UID_BINDING_GROUP_PTR VoHandles);

CSSM_VL_ATTRIBUTE_GROUP_PTR CSSMAPI CSSM_VL_GetRootVoAttributes
                                    (CSSM_VL_HANDLE VLHandle, 
                                     CSSM_VO_HANDLE VOHandle);

CSSM_RETURN CSSMAPI CSSM_VL_FreeVoAttributes
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_ATTRIBUTE_GROUP_PTR VoAttributes);

CSSM_VL_DO_INFO_PTR CSSMAPI CSSM_VL_GetDoInfoByName
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_STRING JoinName);

CSSM_VL_DO_INFO_PTR CSSMAPI CSSM_VL_GetRootDoInfos
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     uint32 *NumberOfDoInfos);

CSSM_VL_DO_INFO_PTR CSSMAPI CSSM_VL_GetInstantiatedDoInfos
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     uint32 *NumberOfDoInfos);

CSSM_RETURN CSSMAPI CSSM_VL_FreeDoInfos
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_DO_INFO_PTR DoInfos,
                                     uint32 NumberOfDoInfos);

CSSM_VL_SIGNATURE_INFO_PTR CSSMAPI CSSM_VL_GetFirstSignatureInfo
                                    (CSSM_VL_HANDLE VLHandle, 
                                     CSSM_VO_HANDLE VOHandle,
                                     CSSM_HANDLE *SignerIteratorHandle);

CSSM_VL_SIGNATURE_INFO_PTR CSSMAPI CSSM_VL_GetNextSignatureInfo
                                    (CSSM_VL_HANDLE VLHandle, 
                                     CSSM_HANDLE SignerIteratorHandle,
                                     CSSM_BOOL *NoMoreSigners);

CSSM_RETURN CSSMAPI CSSM_VL_AbortScan
                                    (CSSM_VL_HANDLE VLHandle, 
                                     CSSM_HANDLE IteratorHandle);

CSSM_RETURN CSSMAPI CSSM_VL_FreeSignatureInfo
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VL_SIGNATURE_INFO_PTR SignatureInfo);

CSSM_CERTGROUP_PTR CSSMAPI CSSM_VL_GetSignerCertificateGroup
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_DATA_PTR SignerCert);

CSSM_VL_DO_LMAP_PTR CSSMAPI CSSM_VL_GetRootDoLocationMap
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle);

CSSM_VL_DO_LMAP_PTR CSSMAPI CSSM_VL_GetInstantiatedDoLocationMap
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle);

CSSM_RETURN CSSMAPI CSSM_VL_FreeDoLMap
                                    (CSSM_VL_HANDLE VLHandle,
                                     const CSSM_VL_DO_LMAP_PTR DoLMap);

CSSM_RETURN CSSMAPI CSSM_VL_SetDoLMapEntries
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_VL_DO_LMAP_PTR NewLocationEntries);

CSSM_RETURN CSSMAPI CSSM_VL_ArchiveDoLMap
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle);

CSSM_VL_VERIFICATION_HANDLE CSSMAPI CSSM_VL_VerifyRootCredentials
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_DATA_PTR SignerCertificate,
                                     uint32 NumberOfPreferredCsps, 
                                     const CSSM_VL_PREFERRED_CSP_PTR PreferredCSPs);

CSSM_VL_VERIFICATION_HANDLE CSSMAPI CSSM_VL_VerifyRootCredentialsDataAndContainment
                                    (CSSM_VL_HANDLE VLHandle,
                                     CSSM_VO_HANDLE VOHandle,
                                     const CSSM_DATA_PTR SignerCertificate,
                                     uint32 NumberOfPreferredCsps, 
                                     const CSSM_VL_PREFERRED_CSP_PTR PreferredCSPs,
                                     uint32 NumberOfContainments,
                                     const CSSM_VL_DO_CONTAINMENT_LOCATION_PTR ContainmentsToVerify);



uint32 CSSMAPI CSSM_VL_SelfVerifyCertificate(
                                    CSSM_VL_HANDLE       VLHandle,
                                    const CSSM_DATA_PTR  Certificate
                                    );
    
#ifdef __cplusplus
}
#endif


#endif
