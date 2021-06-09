/*-----------------------------------------------------------------------
 *      File:   EISLAPI.H
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

#ifndef _EISLAPI_H
#define _EISLAPI_H    

#include "isltype.h"

/* API Functions */
#ifdef __cplusplus
extern "C" {
#endif

/* EISL functions */

#ifdef _IBM
ISL_VERIFIED_MODULE_PTR 
ISL_VerifyDataAndCredentials(
	const ISL_CONST_DATA Credentials,
	const ISL_CONST_DATA SectionName,
	const ISL_CONST_DATA Signer,
	const ISL_CONST_DATA PublicKey);

ISL_VERIFIED_MODULE_PTR 
ISL_VerifyData(
	const ISL_MANIFEST_SECTION_PTR Section);

#endif


/* Credential and Attribute Verification Services */

ISL_VERIFIED_MODULE_PTR 
ISL_SelfCheck();

ISL_VERIFIED_MODULE_PTR 
ISL_VerifyAndLoadModuleAndCredentials(
	const ISL_CONST_DATA Credentials,
	const ISL_CONST_DATA SectionName,
	const ISL_CONST_DATA Signer,
	const ISL_CONST_DATA PublicKey);

ISL_VERIFIED_MODULE_PTR
ISL_VerifyAndLoadModuleAndCredentialsWithCertificate(
	const ISL_CONST_DATA Credentials,
	const ISL_CONST_DATA SectionName,
	const ISL_CONST_DATA Signer,
	const ISL_CONST_DATA Certificate);

ISL_VERIFIED_MODULE_PTR
ISL_VerifyLoadedModuleAndCredentials(
	const ISL_CONST_DATA Credentials,
	const ISL_CONST_DATA SectionName,
	const ISL_CONST_DATA Signer,
	const ISL_CONST_DATA PublicKey);

ISL_VERIFIED_MODULE_PTR
ISL_VerifyLoadedModuleAndCredentialsWithCertificate(
	const ISL_CONST_DATA Credentials,
	const ISL_CONST_DATA SectionName,
	const ISL_CONST_DATA Signer,
	const ISL_CONST_DATA Certificate);

ISL_STATUS 
ISL_RecycleVerifiedModuleCredentials(
	const ISL_VERIFIED_MODULE_PTR Verification);


/* Signature Root Methods */

ISL_VERIFIED_SIGNATURE_ROOT_PTR 
ISL_CreateVerifiedSignatureRoot(
	const ISL_CONST_DATA Credentials,
	const ISL_CONST_DATA Signer,
	const ISL_CONST_DATA PublicKey);

ISL_VERIFIED_SIGNATURE_ROOT_PTR 
ISL_CreateVerifiedSignatureRootWithCertificate(
	const ISL_CONST_DATA Credentials,
	const ISL_VERIFIED_CERTIFICATE_PTR Cert);

ISL_VERIFIED_CERTIFICATE_CHAIN_PTR 
ISL_GetCertificateChain(
	const ISL_VERIFIED_MODULE_PTR Module);

ISL_MANIFEST_SECTION_PTR 
ISL_FindManifestSection(
	const ISL_VERIFIED_SIGNATURE_ROOT_PTR Root,
	const ISL_CONST_DATA SectionName);

ISL_ITERATOR_PTR 
ISL_CreateManifestSectionEnumerator(
	const ISL_VERIFIED_SIGNATURE_ROOT_PTR Root);

ISL_MANIFEST_SECTION_PTR 
ISL_GetNextManifestSection(
	const ISL_ITERATOR_PTR Iterator);

ISL_STATUS 
ISL_RecycleManifestSectionEnumerator(
	const ISL_ITERATOR_PTR Iterator);

ISL_STATUS 
ISL_FindSignatureAttribute(
	ISL_VERIFIED_SIGNATURE_ROOT_PTR Root,
	ISL_CONST_DATA Name,
	ISL_CONST_DATA_PTR Value);

ISL_ITERATOR_PTR 
ISL_CreateSignatureAttributeEnumerator(
	const ISL_VERIFIED_SIGNATURE_ROOT_PTR Verification);

ISL_STATUS 
ISL_GetNextSignatureAttribute(
	const ISL_ITERATOR_PTR Iterator,
	ISL_CONST_DATA_PTR Name,
	ISL_CONST_DATA_PTR Value);

ISL_STATUS 
ISL_RecycleSignatureAttributeEnumerator(
	const ISL_ITERATOR_PTR Iterator);

ISL_STATUS 
ISL_RecycleVerifiedSignatureRoot(
	const ISL_VERIFIED_SIGNATURE_ROOT_PTR Root);

/* Certificate Chain Methods */

const ISL_VERIFIED_CERTIFICATE_CHAIN_PTR 
ISL_CreateCertificateChain(
	const ISL_CONST_DATA RootIssuer,
	const ISL_CONST_DATA PublicKey,
	const ISL_CONST_DATA Credential);

const ISL_VERIFIED_CERTIFICATE_CHAIN_PTR 
ISL_CreateCertificateChainWithCertificate(
	const ISL_CONST_DATA Certificate,
	const ISL_CONST_DATA Credential);

uint32 ISL_CopyCertificateChain(
	const ISL_VERIFIED_CERTIFICATE_CHAIN_PTR Verification,
	ISL_VERIFIED_CERTIFICATE_PTR Certs[],
	uint32 MaxCertificates);

ISL_STATUS 
ISL_RecycleVerifiedCertificateChain(
	ISL_VERIFIED_CERTIFICATE_CHAIN_PTR Chain);


/* Certificate Methods */

ISL_STATUS 
ISL_FindCertificateAttribute(
	ISL_VERIFIED_CERTIFICATE_PTR Cert,
	ISL_CONST_DATA Name,
	ISL_CONST_DATA_PTR Value);

ISL_ITERATOR_PTR 
ISL_CreateCertificateAttributeEnumerator(
	ISL_VERIFIED_CERTIFICATE_PTR Cert);

ISL_STATUS 
ISL_GetNextCertificateAttribute(
	ISL_ITERATOR_PTR CertIterator,
	ISL_CONST_DATA_PTR Name,
	ISL_CONST_DATA_PTR Value);

ISL_STATUS 
ISL_RecycleCertificateAttributeEnumerator(
	ISL_ITERATOR_PTR CertIterator);

/* Manifest Methods */

ISL_VERIFIED_SIGNATURE_ROOT_PTR 
ISL_GetManifestSignatureRoot(
	const ISL_MANIFEST_SECTION_PTR Verification);

ISL_VERIFIED_MODULE_PTR 
ISL_VerifyAndLoadModule(
	const ISL_MANIFEST_SECTION_PTR Section);

ISL_VERIFIED_MODULE_PTR 
ISL_VerifyLoadedModule(
	const ISL_MANIFEST_SECTION_PTR Section);

ISL_STATUS 
ISL_FindManifestSectionAttribute(
	ISL_MANIFEST_SECTION_PTR Section,
	ISL_CONST_DATA Name,
	ISL_CONST_DATA_PTR Value);

ISL_ITERATOR_PTR 
ISL_CreateManifestSectionAttributeEnumerator(
	const ISL_MANIFEST_SECTION_PTR Section);

ISL_STATUS 
ISL_GetNextManifestSectionAttribute(
	const ISL_ITERATOR_PTR Iterator,
	ISL_CONST_DATA_PTR Name,
	ISL_CONST_DATA_PTR Value);

ISL_STATUS 
ISL_RecycleManifestSectionAttributeEnumerator(
	const ISL_ITERATOR_PTR Iterator);

ISL_MANIFEST_SECTION_PTR 
ISL_GetModuleManifestSection(
	ISL_VERIFIED_MODULE_PTR Module);

/* Secure Linkage */

ISL_FUNCTION_PTR
ISL_LocateProcedureAddress(
	ISL_VERIFIED_MODULE_PTR Module,
	ISL_CONST_DATA Name);

#define ISL_GetReturnAddress

#if 0
ISL_FUNCTION_PTR 
ISL_GetReturnAddress();
#endif

void * 
ISL_GetLibHandle(
	ISL_VERIFIED_MODULE_PTR Module);

ISL_STATUS 
ISL_CheckAddressWithinModule(
	ISL_VERIFIED_MODULE_PTR Verification,
	ISL_FUNCTION_PTR Address);

uint32 ISL_ContinueVerification(
	ISL_VERIFIED_MODULE_PTR Module,
	uint32 WorkFactor); 

#ifdef __cplusplus
}
#endif

/* Do not edit or remove the following string. 
 * It should be expanded at check-in by version control. */
static const char sccs_id_INC_EISLAPI_H[] = { "Fix ME" }; 

#endif


