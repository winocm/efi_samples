/*-----------------------------------------------------------------------
 *      File:   INTERNAL.H
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
 *  INTEL CONFIDENTIAL
 *  This file, software, or program is supplied under the terms
 *  of a licence agreement or nondisclosure agreement with
 *  Intel Corporation and may not be copied or disclosed except
 *  in accordance with the terms of that agreement. This file,
 *  software, or program contains copyrighted material and/or
 *  trade secret information of Intel Corporation, and must be
 *  treated as such. Intel reserves all rights in this material,
 *  except as the licence agreement or nondisclosure agreement
 *  specifically indicate.
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

#ifndef _INTERNAL_H
#define _INTERNAL_H    

//#include <assert.h>
#include "cssm.h"
#include "integapi.h"
#include "eislapi.h"
#include "ber_der.h"
#include "isl_def.h"
#include "isl_tags.h"
#include "cssmport.h"
#include "isl_parse.h"

/* Place internal Data Structures in isl_def.h (not here)  */

#define HASH_STR_LENGTH 7

#define IS_EQUAL(Name,String) ((String).Length == (Name).Length && \
							   !cssm_memcmp((Name).Data,(String).Data, (String).Length))

#define ISL_ISDIGIT(c)  ( (c) >= '0' && (c) <= '9')
#define ISL_ISALPHA(c)  (   ( (c) >= 'a' && (c) <= 'z')  || \
                        ( (c) >= 'A' && (c) <= 'Z')  )
#define ISL_ISALNUM(c)  (ISL_ISDIGIT(c) || ISL_ISALPHA(c))
#define ISL_TOUPPER(c)  ( ( (c) >= 'a' && (c) <= 'z' ) ? (c) + 'A' - 'a' : (c))

#define IS_HEADERCHAR(c) ( ISL_ISALNUM(c) || (c) == '-' || (c) == '_' ) 
#define IS_NEWLINE(c)	 ( (c) == '\n' || (c) == '\r')
//#define ASSERT(X) {}

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

extern struct isl_archive_methods ArchiveMethods;
extern struct isl_manifest_section_methods ManifestSectionMethods;
extern struct isl_config_methods ArchiveConfigMethods;
extern struct isl_signature_methods	PKCS7SignatureMethods;
extern struct isl_certificate_methods X509CertMethods; 
extern struct isl_signer_methods PKCS7SignerMethods;
extern ISL_SIG_SECTION_METHODS SignedListMethods;

extern ISL_GET_DATA_METHODS getCSSMDataMethods;
extern ISL_CONFIG_METHODS PKCS7SignatureConfigMethods;
extern ISL_DIGEST_METHODS JarSHA1Methods;
extern ISL_SIGN_VERIFY_METHODS PKCS7DSAMethods;
extern ISL_GET_DATA_METHODS getMemoryMethods;

#define STATIC static
extern const ISL_CONST_DATA	HASH_ALGORITHMS_STR;
extern const ISL_CONST_DATA	HASH_STR;
extern const ISL_CONST_DATA	NAME_STR;
extern const ISL_CONST_DATA	MANIFEST_VERSION_STR;
extern const ISL_CONST_DATA	SECTION_NAME;
extern const ISL_DATA		NULL_DATA;
extern const ISL_CONST_DATA	NULL_CONST_DATA;
extern const ISL_DATA		SIGNATURE_VERSION_STR;

/* API Functions */
#ifdef __cplusplus
extern "C" {
#endif

ISL_STATUS isl_VerifyDigestValues(
	ISL_MANIFEST_SECTION_PTR		Context,
	ISL_ALG_INFO_PTR				AlgInfoPtr,
	ISL_GET_DATA_METHODS *			GetDataMethods,
 	ISL_CONST_DATA                  GetParameters,
    void *OutputContext,					/* object to update */
	ISL_STATUS (*Update)(					/* object's update method */
		void *UpdateContext,				/* (place OutputContext here) */
		ISL_CONST_DATA Buffer)); 			/* Length and Value of update */  

uint32
isl_CountItemsInList(ISL_LIST_PTR ListNodePtr);
	
ISL_STATUS
isl_FindAttributeInHeader(
    ISL_HEADER_SECTION_PTR HeaderSectionPtr,
    ISL_CONST_DATA Attribute,
    ISL_CONST_DATA_PTR ValuePtr);

ISL_CERTIFICATE_GROUP_PTR
isl_BuildCertificateGroup(
    ISL_LIST_PTR CertificateListPtr,
    ISL_MEMORY_CONTEXT_PTR MemoryPtr);

ISL_STATUS
isl_FreeCertificateGroup(
    ISL_CERTIFICATE_GROUP_PTR CertGrpPtr,
    ISL_MEMORY_CONTEXT_PTR MemoryPtr);

ISL_ATTRIBUTE_GROUP_PTR
isl_BuildAttributeGrp(
    ISL_LIST_PTR AttributeListPtr,
    ISL_MEMORY_CONTEXT_PTR MemoryPtr);

ISL_STATUS
isl_FreeAttributeGrp(
    ISL_ATTRIBUTE_GROUP_PTR AttributeGrpPtr,
    ISL_MEMORY_CONTEXT_PTR MemoryPtr);

/* Exported cert functions from x509cert.c */

ISL_STATUS
isl_GetProtocolAndParameters(
	ISL_CONST_DATA Name, 
	ISL_CONST_DATA_PTR Protocol, 
	ISL_CONST_DATA_PTR Parameters);

/* PKCS#7 Parsing Routines */
ISL_STATUS
isl_ParsePKCS7(
    ISL_MEMORY_CONTEXT_PTR  MemoryPtr,
	ISL_CONST_DATA	        Image,
    ISL_PARSED_PKCS         *pPKCS);

ISL_STATUS
isl_GetCertsFromPKCS(
	ISL_MEMORY_CONTEXT_PTR  MemoryPtr,
	ISL_PARSED_PKCS		    PKCS,
	ISL_CONST_DATA_PTR	    *pCerts,
	uint32			        *pCount);

ISL_STATUS	
isl_GetSignedContentFromPKCS(
	ISL_PARSED_PKCS	PKCS,
	ISL_CONST_DATA_PTR	pSignedContent);

ISL_STATUS
isl_GetSignersFromPKCS(
	ISL_MEMORY_CONTEXT_PTR	MemoryPtr,
    ISL_PARSED_PKCS         PKCS,
    ISL_CONST_DATA_PTR	    *pSignerInfo,
	uint32				    *pSignerCount);

/* PKCS7 Initialization Routines */

ISL_STATUS 
isl_InitializeSignedListFromImage(	                /* constructor for existing object list */
		ISL_SIG_SECTION_LIST *memory,			/* memory allocated for this instance */
		ISL_ARCHIVE_CONTEXT *Context,		    /* archive which contains signature */
		ISL_CONST_DATA Name,					/* disk file of external representation */
        ISL_CONST_DATA Image);

ISL_STATUS
isl_BuildSignatureContext(
	ISL_SIGNATURE_CONTEXT_PTR pSigRoot);

ISL_STATUS
isl_BuildSigners(
	ISL_SIGNATURE_CONTEXT_PTR	pSignatureContext);

ISL_STATUS 
isl_BuildSignedObject(
	ISL_CONST_DATA_PTR ptr, 
	ISL_SIG_SECTION_PTR pSigSect);

ISL_STATUS
isl_BuildSigSection(
    ISL_SIG_SECTION_LIST_PTR SignedObjectListPtr,
    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr,
	ISL_SIG_SECTION_PTR SignedObjectPtr,
	ISL_SECTION_INFO_PTR SectionInfoPtr);

ISL_STATUS
isl_GetSignedObjectListHeader(
    ISL_CONST_DATA_PTR ptr, 
	ISL_SIG_SECTION_LIST_PTR Context);

ISL_STATUS
isl_GetNameValuePair(
	ISL_ARCHIVE_CONTEXT_PTR pArchive,
	ISL_CONST_DATA_PTR input, 
	ISL_DATA_PTR Name, 
	ISL_DATA_PTR Value);

ISL_CONST_DATA_PTR 
isl_SkipNewLine(
	ISL_CONST_DATA_PTR ptr);

ISL_CONST_DATA_PTR 
isl_GetSectionName(
	ISL_ARCHIVE_CONTEXT_PTR pArchive,
	ISL_CONST_DATA_PTR ptr, 
	ISL_DATA_PTR pName);

ISL_ALG_INFO_PTR
isl_FindSHA(
	ISL_ALG_INFO_PTR	pAlgList);

ISL_ALG_INFO_PTR 
isl_BuildManSectAlgList(
	ISL_MANIFEST_SECTION_PTR pManSect, 
	ISL_DATA Value);

ISL_ALG_INFO_PTR
isl_BuildSigSectAlgList(
	ISL_SIG_SECTION_PTR pSigSect,
	ISL_DATA Value);

ISL_STATUS 
isl_AddDigest(
	ISL_DATA Name, 
	ISL_DATA Value, 
	ISL_ALG_INFO_PTR pAlgNode);

/* Parsing Methods */

ISL_STATUS
isl_BuildHeaderSection(
	ISL_HEADER_SECTION_PTR HeaderPtr,
	ISL_MEMORY_CONTEXT_PTR MemoryPtr,
	ISL_SECTION_INFO_PTR SectionInfoPtr);

extern void *
isl_CopyMemory(
	ISL_MEMORY_CONTEXT_PTR MemoryPtr, 
	const void *buffer, 
	ISL_SIZE size);

extern void *
isl_AllocateMemory(
	ISL_MEMORY_CONTEXT_PTR MemoryPtr, 
	ISL_SIZE size);

extern void 
isl_FreeMemory(
	ISL_MEMORY_CONTEXT_PTR MemoryPtr, 
	const void *buffer);

extern
ISL_STATUS
isl_RecycleMemoryContext(ISL_MEMORY_CONTEXT *Context);

void *
isl_memchr(const void *s, char c, unsigned int n);

/* Credential Routines */


ISL_STATUS
isl_BuildManifest(
	ISL_ARCHIVE_CONTEXT_PTR pArchive);

ISL_STATUS
isl_BuildManifestSection(
	ISL_MANIFEST_SECTION_PTR ManifestSectionPtr,
	ISL_SECTION_INFO_PTR SectionInfoPtr);

ISL_STATUS 
isl_GetManifestHeader(
	ISL_CONST_DATA_PTR ptr, 
	ISL_ARCHIVE_CONTEXT_PTR pArchive);

ISL_STATUS
isl_InitSignatureContext(
	ISL_ARCHIVE_CONTEXT_PTR   pArchive,
	ISL_SIGNATURE_CONTEXT_PTR pSig,
	ISL_DATA				  Name);

ISL_STATUS
isl_InitSigSectionList(
	ISL_SIGNATURE_CONTEXT_PTR pSigRoot, 
	ISL_SIG_SECTION_LIST_PTR  pSigSecList);


ISL_STATUS isl_InitializeNewSignature(				/* constructor for new archive */
		ISL_SIGNATURE_CONTEXT *memory,			/* memory allocated for this instance */
		ISL_CONFIG_PTR configContext, 			/* my algorithm<->code extension configuration */
		ISL_ARCHIVE_CONTEXT *Archive,			/* archive which will contain signature */
		ISL_CONST_DATA Name); 					/* where this signature is being constructed */

/* Error Reporting Functions */
extern void isl_ReportError(
	ISL_ARCHIVE_CONTEXT_PTR	pArchive,
	ISL_ERROR_CODE			ErrCode,
	char *					ErrStr);

extern const char * isl_GetErrorStr(
	const uint32			FuncCode);

/* Misc #defines */
#define SECTION_NAME_TITLE  "SectionName: "
#define NAME_TITLE "Name: "
#define HASH_ALGORITHM_TITLE "Digest-Algorithms: "
#define MAX_BYTES_IN_LINE 72

/* Win32 Specific Functions */
#ifdef OASIS
#define IS_DIGESTED_SECTION(SecHead) 1
#else
#define IS_DIGESTED_SECTION(SecHead) \
(																	/* section is */		\
	( (SecHead)->Characteristics & (IMAGE_SCN_MEM_READ) )   &&		/* readable */			\
   !( (SecHead)->Characteristics & (IMAGE_SCN_MEM_WRITE | 			/* not writable */		\
									IMAGE_SCN_MEM_DISCARDABLE))		/* not discardable */	\
)
#endif

#ifdef OASIS
#define IS_EXECUTABLE_SECTION(SecHead) 1
#else
#define IS_EXECUTABLE_SECTION(SecHead) \
(																	/* section is */		\
	( (SecHead)->Characteristics & (IMAGE_SCN_MEM_READ) ) &&		/* readable */			\
	( (SecHead)->Characteristics & (IMAGE_SCN_MEM_EXECUTE |			/* executable */		\
									IMAGE_SCN_CNT_CODE) ) &&		/* or code */			\
   !( (SecHead)->Characteristics & (IMAGE_SCN_MEM_WRITE) )			/* not writable */		\
)
#endif

ISL_STATUS
isl_ParseFilePath(
    ISL_CONST_DATA      FilePath,
    ISL_CONST_DATA_PTR  FileDir,
    ISL_CONST_DATA_PTR  FileName,
    ISL_CONST_DATA_PTR  FileTitle,
    ISL_CONST_DATA_PTR  FileExt);

uint32
isl_IsManifestVersion(
    ISL_DATA Name);
uint32
isl_IsSignatureVersion(
    ISL_DATA Name);

#ifdef __cplusplus
}
#endif

/* Do not edit or remove the following string. 
 * It should be expanded at check-in by version control. */
#ifdef SCCS_ID
static const char sccs_id_INC_INTERNAL_H[] = { "Fix ME" }; 
#endif

#endif
