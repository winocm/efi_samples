/*-----------------------------------------------------------------------
 *      File: x509cert.c  
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
#include "isl_internal.h"
#include "oidscert.h"
#include "guids.h"

static
ISL_STATUS 
isl_X509Cert_Recycle(ISL_CERTIFICATE *Context);

/*
**	X509 certificate class
*/
/* service methods */
/*-----------------------------------------------------------------------------
 * Name: X509id
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static void 
X509id(
	ISL_SERVICE_CLASS *algClass,				
	ISL_CONST_DATA *algID,				
	ISL_CONST_DATA *serviceName)
{
	*algClass = ISL_ServiceParseCertificate;
	algID->Data = (uint8 *)"cer";
	algID->Length = cssm_strlen((char *)algID->Data);;
	serviceName->Data = (uint8 *)"X509";
	serviceName->Length = cssm_strlen((char *)serviceName->Data);
}
/*
**	X509 certificate class
*/

/* class methods */
/*-----------------------------------------------------------------------------
 * Name: isl_X509Cert_SizeofObject
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
STATIC
ISL_SIZE 
isl_X509Cert_SizeofObject()			/* returns sizeof object */
{
    NAMEPROC( ISL_DSCM_TAG, ISL_EISL_TAG, ISL_X509CERT_TAG,0x00);
	return sizeof(struct isl_certificate);
}
	
/* object methods */
/*-----------------------------------------------------------------------------
 * Name: RetrieveX509Field
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_STATUS
RetrieveX509Field(
	ISL_CERTIFICATE_PTR Context,
	CSSM_CL_HANDLE CLHandle,
	const CSSM_OID_PTR CertFieldOIDPtr,
	ISL_CONST_DATA_PTR CertFieldPtr)
{
	CSSM_DATA_PTR DataPtr;
	CSSM_HANDLE ResultsHandle;
	uint32 NumberOfMatchedFields;
    ISL_MEMORY_CONTEXT_PTR MemoryPtr;
	ISL_STATUS status = ISL_FAIL;

    if (Context == NULL) return ISL_FAIL;

    MemoryPtr = Context->Memory;
	DataPtr = CSSM_CL_CertGetFirstFieldValue(
				CLHandle,
				(CSSM_DATA_PTR) &Context->theCert,
				CertFieldOIDPtr,
				&ResultsHandle,
				&NumberOfMatchedFields);
	
	if (CSSM_OK != CSSM_CL_CertAbortQuery(
					CLHandle,
					ResultsHandle))
	{
		return ISL_FAIL;
	}

	if (DataPtr == NULL ||
		NumberOfMatchedFields == 0)
	{
		return ISL_FAIL;
	}

	CertFieldPtr->Length = DataPtr->Length;
	CertFieldPtr->Data = isl_AllocateMemory(MemoryPtr, CertFieldPtr->Length);
	if (CertFieldPtr->Data == NULL) goto EXIT;
	cssm_memcpy((void*)CertFieldPtr->Data, DataPtr->Data, CertFieldPtr->Length);
	status = ISL_OK;
EXIT:
    {
	    MemoryPtr->MemoryMethods->free_func(
            DataPtr->Data, 
            MemoryPtr->MemoryMethods->AllocRef);

	    MemoryPtr->MemoryMethods->free_func(
            DataPtr, 
            MemoryPtr->MemoryMethods->AllocRef);
	    return status;
    }
}

/*-----------------------------------------------------------------------------
 * Name: isl_X509Cert_Initialize
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
STATIC
ISL_STATUS 
isl_X509Cert_Initialize(
		ISL_CERTIFICATE *Context,			/* memory for internal rep of cert */
		ISL_MEMORY_CONTEXT *MemoryPtr,	    /* memory allocation methods */
		ISL_CONST_DATA Cert)				/* external rep of certificate */
{
	CSSM_CL_HANDLE CLHandle;
	CSSM_VERSION cl_version = {INTEL_X509V3_CLM_MAJOR_VER, INTEL_X509V3_CLM_MINOR_VER};
	uint8 *pkcs7buffer;
	uint8 *buffer;
	uint32 size;
	CSSM_KEY_PTR KeyPtr = NULL;
    ISL_MEMORY_METHODS_PTR MemoryMethods;

    if (Context != NULL) {
    	cssm_memset(Context, 0, sizeof(ISL_CERTIFICATE));
    }
	if (Context == NULL ||
		MemoryPtr == NULL ||
		Cert.Data == NULL ||
		Cert.Length == 0) 
	{
		return ISL_FAIL;
	}
	pkcs7buffer = 0;
	Context->Memory = MemoryPtr;
    MemoryMethods = MemoryPtr->MemoryMethods;

	CLHandle = CSSM_ModuleAttach (
		&intel_preos_clm_guid, 
		&cl_version,
        MemoryMethods, 
		0, 
		0, 
		0, 
		NULL, 
		NULL, 
		NULL, 
		NULL);

	if (CLHandle == 0) return ISL_FAIL;

    buffer = isl_AllocateMemory(MemoryPtr, Cert.Length);
	if (!buffer) { 
		goto FAIL;
	}

	cssm_memcpy(buffer, Cert.Data, Cert.Length);
	Context->theCert.Length = Cert.Length;
	Context->theCert.Data = buffer;

	if (ISL_OK != RetrieveX509Field(
					Context,
					CLHandle,
					&CSSMOID_X509V1SerialNumber,
					&Context->serialNumber))
	{
		goto FAIL;
	}
	if (ISL_OK != RetrieveX509Field(
					Context,
					CLHandle,
					&CSSMOID_X509V1IssuerName,
					&Context->issuer))
	{
		goto FAIL;
	}
	if (ISL_OK != RetrieveX509Field(
					Context,
					CLHandle,
					&CSSMOID_X509V1SignatureAlgorithm,
					&Context->algorithmID))
	{
		goto FAIL;
	}
	
	KeyPtr = CSSM_CL_CertGetKeyInfo(CLHandle, (CSSM_DATA_PTR)&Cert);
	if (KeyPtr == NULL)
	{
		goto FAIL;
	}
	Context->key = *KeyPtr;
	Context->key.KeyData.Data = isl_AllocateMemory(MemoryPtr, KeyPtr->KeyData.Length);
	if (Context->key.KeyData.Data == NULL)
	{
		goto FAIL;
	}
	cssm_memcpy(Context->key.KeyData.Data, KeyPtr->KeyData.Data, KeyPtr->KeyData.Length);

	MemoryMethods->free_func(KeyPtr->KeyData.Data, MemoryMethods->AllocRef);
	MemoryMethods->free_func(KeyPtr, MemoryMethods->AllocRef);

	/* construct the pkcs7 signer id (Seq(issuer, serial number)) */
	size = Context->serialNumber.Length + 
		   Context->issuer.Length + 
		   BER_LengthOfLength(Context->serialNumber.Length) + 1;
	Context->pkcs7id.Length = size + BER_LengthOfLength(size) + 1;	/* content + length + tag */
	pkcs7buffer = isl_AllocateMemory(MemoryPtr, Context->pkcs7id.Length);
	if (!pkcs7buffer) {
		goto FAIL;
	}

	Context->pkcs7id.Data = pkcs7buffer;
	*pkcs7buffer++ = BER_SEQUENCE | BER_CONSTRUCTED;
	pkcs7buffer += BER_OutputLength(size, pkcs7buffer);
	cssm_memcpy(pkcs7buffer, Context->issuer.Data, Context->issuer.Length);
	pkcs7buffer += Context->issuer.Length;
	*pkcs7buffer++ = BER_INTEGER;
	pkcs7buffer += BER_OutputLength(Context->serialNumber.Length, pkcs7buffer);
	cssm_memcpy(pkcs7buffer, Context->serialNumber.Data, Context->serialNumber.Length);

    if (CSSM_ModuleDetach(CLHandle) != CSSM_OK)
		return ISL_FAIL;

	return ISL_OK;

FAIL:
    {
	    CSSM_ModuleDetach(CLHandle);
	    return ISL_FAIL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: isl_X509Cert_Recycle
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_STATUS 
isl_X509Cert_Recycle(ISL_CERTIFICATE *Context)
{
	if (Context == NULL) return ISL_FAIL;
	/* recycle all memory buffers we allocated (do this last) */
    isl_RecycleMemoryContext(Context->Memory);
	/* clear all the pointers,etc */
	cssm_memset(Context, 0, sizeof(ISL_CERTIFICATE));
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: isl_X509Cert_GetID
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
STATIC
ISL_STATUS 
isl_X509Cert_GetID(ISL_CERTIFICATE *Context, ISL_CONST_DATA_PTR SignerID)
{
	if (Context == NULL ||
		SignerID == NULL)
	{
		return ISL_FAIL;
	}

	*SignerID = Context->pkcs7id;
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: isl_X509Cert_GetKey
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
STATIC
ISL_STATUS
isl_X509Cert_GetKey(ISL_CERTIFICATE *Context, ISL_KEY_BLOCK_PTR Key)
{
	if (Context == NULL ||
		Key == NULL)
	{
		return ISL_FAIL;
	}

	*Key = &(Context->key);
	return ISL_OK;
}


/*-----------------------------------------------------------------------------
 * Name: isl_X509Cert_GetAlgID
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
STATIC
ISL_STATUS 
isl_X509Cert_GetAlgID(ISL_CERTIFICATE *Context, ISL_CONST_DATA *AlgID)
{
	if (Context == NULL ||
		AlgID == NULL) 
	{
		return ISL_FAIL;
	}

	*AlgID = Context->algorithmID;
	return ISL_OK;
}

STATIC
ISL_STATUS
isl_X509Cert_Flatten(
    ISL_CERTIFICATE_PTR Context,
    ISL_CONST_DATA *theCert)
{
    if (Context == NULL ||
        theCert == NULL)
    {
        return ISL_FAIL;
    }
    *theCert = Context->theCert;
    return ISL_OK;
}

ISL_CERTIFICATE_METHODS X509CertMethods = {
		{ X509id },
	isl_X509Cert_SizeofObject,
	isl_X509Cert_Initialize,
	isl_X509Cert_Recycle,
	isl_X509Cert_GetID,
	isl_X509Cert_GetKey,
	isl_X509Cert_GetAlgID,
    isl_X509Cert_Flatten
};
