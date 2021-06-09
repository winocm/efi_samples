/*-----------------------------------------------------------------------
 *      File:   util.c
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *---------------------------------------------------------------------
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

//#include <stdio.h>
//#include <sys/stat.h>
#include "isl_internal.h"
#include "islutil.h"

static uint32 gErrorCode = 0;

/*-----------------------------------------------------------------------------
 * Name: isl_ParseFilePath
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
isl_ParseFilePath(
    ISL_CONST_DATA      FilePath,
    ISL_CONST_DATA_PTR  FileDir,
    ISL_CONST_DATA_PTR  FileName,
    ISL_CONST_DATA_PTR  FileTitle,
    ISL_CONST_DATA_PTR  FileExt)
{
	const uint8	*ptr;
	const uint8	*oldptr;
	const uint8 *endptr;
	const uint8	*pBS;
	const uint8	*pFS;
	uint32	ptrlen;

	if (!FilePath.Data) return ISL_FAIL;

	ptr = FilePath.Data;
	oldptr = FilePath.Data;
	endptr = FilePath.Data + FilePath.Length;
	while (ptr)	
	{
		oldptr = ++ptr;
		ptrlen = (uint32)(endptr - ptr);
		pBS = isl_memchr(oldptr, '\\', ptrlen);
		pFS = isl_memchr(oldptr, '/', ptrlen);
		ptr = (pBS > pFS) ? pBS : pFS;
	}

	ptr = isl_memchr(oldptr, '.', (uint32)(endptr - oldptr));
	ptr = (ptr)? ptr : endptr;
	if (FileExt)
	{
	   FileExt->Length = (uint32)(endptr - ptr) - 1;
	   FileExt->Data = ptr + 1;
	}
	if (FileTitle)
	{
	   FileTitle->Length = (uint32)(ptr - oldptr);
	   FileTitle->Data = oldptr;
	}
	if (FileName)
	{
	   FileName->Length = (uint32)(endptr - oldptr);
	   FileName->Data = oldptr;
	}
	if (FileDir)
	{
	   FileDir->Length = (uint32)(oldptr - FilePath.Data); 
	   FileDir->Data = FilePath.Data;
	}
	return ISL_OK;
}


#pragma warning( disable : 4035)
/*-----------------------------------------------------------------------------
 * Name: isl_memchr
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
void *isl_memchr(const void *s, char c, unsigned int n)
{
    unsigned int    Index = 0;
    char *          ret;
    int             Found = 0;

    ret = (void*)s;
    
    while (Index < n)
    {
        if (*ret == c)
        {
            Found = 1;
            break;
        }
        Index++;
        ret++;
    }
    
    if (!Found)
        ret = NULL;

    return ret;
}
#pragma warning( default : 4035)

void ISL_SetError(uint32 ErrCode)
{
	gErrorCode = ErrCode;
}

uint32 ISL_GetError()
{
	uint32 ErrCode;

	ErrCode = gErrorCode;
	gErrorCode = 0;
	return ErrCode;
}

/*-----------------------------------------------------------------------------
 * Name:  isl_VerifyDigestValues
 *
 * Description:
 *
 * Parameters:
 * Context (input) - Context used for GetDataMethods
 * AlgInfoPtr (input) - Digests to verify against
 * GetDataMethods (input) - GetDataMethods for retrieving data
 * GetParameters (input) - Parameters for GetDataMethods
 * OutputContext (input) - User context for callback
 * Update (input/output) - Callback for reporting data as being retrieved
 * for verification
 *
 * Return value:
 * ISL_OK if Verified
 * ISL_FAIL, otherwise
 *
 * Error Codes:
 *
 *---------------------------------------------------------------------------*/
ISL_STATUS isl_VerifyDigestValues(
	ISL_MANIFEST_SECTION_PTR		Context,
	ISL_ALG_INFO_PTR				AlgInfoPtr,
	ISL_GET_DATA_METHODS *			GetDataMethods,
 	ISL_CONST_DATA                  GetParameters,
    void *OutputContext,					/* object to update */
	ISL_STATUS (*Update)(					/* object's update method */
		void *UpdateContext,				/* (place OutputContext here) */
		ISL_CONST_DATA Buffer)) 			/* Length and Value of update */  
{
	/* variables to be allocated and freed */
  	CSSM_CC_HANDLE					*DigestHandlesPtr = NULL;
	ISL_GET_DATA_SERVICE_CONTEXT    *GetDataContext = NULL;
	ISL_DATA_PTR					DigestValues = NULL;
	uint32                          NumberOfDigests = 0;

	/* variables to be returned */
	ISL_STATUS retval = ISL_FAIL;

	/* intermediate variables */
    ISL_MEMORY_CONTEXT_PTR  MemoryPtr;
	ISL_CONST_DATA                  block;
    ISL_STATUS                      status;
	ISL_ERROR_CODE					error;
	ISL_ALG_INFO_PTR				digest;
	ISL_HASH_INFO_PTR hash;
    uint32                          ContextSize;
    uint32                          i;
	uint32							count;


	/* setup the get data routine */

    if (Context == NULL ||
		Context->Parent == NULL ||
		GetDataMethods == NULL ||
		GetDataMethods->SizeofObject == NULL ||
		GetDataMethods->InitializeWithClass == NULL ||
		GetDataMethods->Update == NULL ||
		GetDataMethods->Recycle == NULL)
	{
		return ISL_FAIL;
	}

	MemoryPtr = Context->Parent->Memory;
	ContextSize = (uint32)GetDataMethods->SizeofObject();
	GetDataContext = isl_AllocateMemory(MemoryPtr, ContextSize);
	if (GetDataContext == NULL) goto FAIL;

	status = GetDataMethods->InitializeWithClass(
		GetDataContext,
		GetParameters, 
		GetDataMethods->ServiceMethods.Class, 
		Context);
	if (status != ISL_OK) goto FAIL;

    NumberOfDigests = Context->SectionInfo.AlgCount;
    DigestHandlesPtr = isl_AllocateMemory(
        MemoryPtr, 
        NumberOfDigests * sizeof(CSSM_CC_HANDLE));
	if (DigestHandlesPtr == NULL) goto FAIL;
	DigestValues = isl_AllocateMemory(
        MemoryPtr, 
        NumberOfDigests * sizeof(ISL_DATA));
	if (DigestValues == NULL) goto FAIL;
/* Begin Hack */
/* The CSP is current unable to allocate digest data */
	for(i = 0; i < NumberOfDigests; i++)
	{
		DigestValues[i].Length = 20;
		DigestValues[i].Data = isl_AllocateMemory(MemoryPtr, DigestValues[i].Length);
		if (DigestValues[i].Data == NULL) goto FAIL;
	}
/* End Hack */

    /* setup all the digest algorithms */
	for (i = 0, digest = AlgInfoPtr; 
        digest; 
        i++, digest = digest->Next) 
    {
		if (digest->Methods != NULL)
		{
			DigestHandlesPtr[i] = CSSM_CSP_CreateDigestContext(
									digest->Methods->CSPHandle,
									digest->Methods->AlgorithmID);
		}
    }

	/* run all the data through all the digests */
	for (count = 0;;count++) 
	{
		error = GetDataMethods->Update(GetDataContext, &block);
		if (error != ISL_NO_ERROR) {
			goto FAIL;
		}

		/* need to handle corner case of an empty thing */
		/* so check for break only after first iteration */
		if (count != 0)
		{
			if (block.Length == 0)
			{
				break;
			}
			else
			{
				/* don't have staged hashes */
				goto FAIL;
			}
		}

        if (Update)
        {
            if (ISL_OK != (*Update)(OutputContext, block)) return ISL_FAIL;
        }

		for (i = 0, digest = AlgInfoPtr; digest; i++, digest = digest->Next) 
		{
			if (CSSM_OK != CSSM_DigestData(DigestHandlesPtr[i], 
										   (CSSM_DATA_PTR)&block, 
										   1, 
										   (CSSM_DATA_PTR)(DigestValues + i)))
			{
				continue;
			}
		}
	}
	error = GetDataMethods->Recycle(GetDataContext);
	if (error != ISL_NO_ERROR) 
	{
		goto FAIL;
	}

	for (i = 0, digest = AlgInfoPtr; digest; i++, digest = digest->Next) 
	{
		if (CSSM_OK != CSSM_DeleteContext(DigestHandlesPtr[i]))
		{
			continue;
		}

    }
	status = ISL_FAIL;
	for (i = 0, digest = AlgInfoPtr; digest; i++, digest = digest->Next) 
    {
		if (digest->Methods != NULL)
		{
			status = ISL_FAIL;
			for (hash = digest->Hash; hash; hash = hash->Next) 
			{
				if (IS_EQUAL(hash->Value, DigestValues[i]))
				{
					status = ISL_OK;
					break;
				}
			}

			if (status != ISL_OK) goto FAIL;
		}
	}

	if (status == ISL_OK) retval = ISL_OK;
FAIL:
    {
	    if (DigestValues != NULL)
	    {
		    for(i = 0; i < NumberOfDigests; i++)
		    {
			    isl_FreeMemory(MemoryPtr, DigestValues[i].Data);
		    }
	    }
	    isl_FreeMemory(MemoryPtr, GetDataContext);
	    isl_FreeMemory(MemoryPtr, DigestValues);
	    isl_FreeMemory(MemoryPtr, DigestHandlesPtr);
	    return retval;
    }
}

/*-----------------------------------------------------------------------------
 * Name:  isl_CountItemsInList
 *
 * Description: Counts the number of items in a List. 
 *
 * Parameters: 
 * ListNodePtr(input) - head a list of nodes
 *
 * Return value:
 * the number items in the list
 * 
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
uint32
isl_CountItemsInList(ISL_LIST_PTR ListNodePtr)
{
    uint32 count = 0;
    while (ListNodePtr)
    {
        count++;
        ListNodePtr = ListNodePtr->Next;
    }
    return count;
}

/*-----------------------------------------------------------------------------
 * Name:  isl_FindAttributeInHeader
 *
 * Description: Searches for an attribute name/value pair based on an 
 * attribute name and returns the value
 *
 * Parameters: 
 * HeaderSectionPtr (input) - header to search
 * Name (input) - key for searching for attribute name/value pair
 * ValuePtr (output) - value of name/value pair if found
 *
 * Return value:
 * ISL_OK if attribute name/value pair found
 * ISL_FAIL, otherwise
 * 
 * Error Codes:
 * None
 *
 * Notes:
 * No memory allocate when returning the value
 *---------------------------------------------------------------------------*/
ISL_STATUS
isl_FindAttributeInHeader(
    ISL_HEADER_SECTION_PTR HeaderSectionPtr,
    ISL_CONST_DATA Name,
    ISL_CONST_DATA_PTR ValuePtr)
{
    ISL_LIST_PTR ListNodePtr;

    if (HeaderSectionPtr == NULL ||
        ValuePtr == NULL)
    {
        return ISL_FAIL;
    }

    for(ListNodePtr = HeaderSectionPtr->Attribute;
        ListNodePtr != NULL;
        ListNodePtr = ListNodePtr->Next)
    {
        ISL_NAME_VALUE_PTR NameValuePtr = ListNodePtr->Node;
        if (NameValuePtr &&
            IS_EQUAL(NameValuePtr->Name, Name))
        {
            *ValuePtr = NameValuePtr->Value;
            return ISL_OK;
        }
    }
    return ISL_FAIL;
}

/*-----------------------------------------------------------------------------
 * Name:  isl_FindAttributeInHeader
 *
 * Description: Searches for an attribute name/value pair based on an 
 * attribute name and returns the value
 *
 * Parameters: 
 * HeaderSectionPtr (input) - header to search
 * Name (input) - key for searching for attribute name/value pair
 * ValuePtr (output) - value of name/value pair if found
 *
 * Return value:
 * ISL_OK if attribute name/value pair found
 * ISL_FAIL, otherwise
 * 
 * Error Codes:
 * None
 *
 * Notes:
 * No memory allocate when returning the value
 *---------------------------------------------------------------------------*/
ISL_CERTIFICATE_GROUP_PTR
isl_BuildCertificateGroup(
    ISL_LIST_PTR CertificateListPtr,
    ISL_MEMORY_CONTEXT_PTR MemoryPtr)
{
    ISL_CERTIFICATE_GROUP_PTR CertGrpPtr;
    ISL_CERTIFICATE_PTR *CertObjPtr;
    ISL_CERTIFICATE_PTR CurrCertObjPtr;
    ISL_LIST_PTR CurrNodePtr;

    uint32 NumberOfCertificates;

    CertGrpPtr = isl_AllocateMemory(
        MemoryPtr,
        sizeof(ISL_CERTIFICATE_GROUP));
    if (CertGrpPtr == NULL) return NULL;

    NumberOfCertificates = isl_CountItemsInList(CertificateListPtr);
    if (NumberOfCertificates == 0)
    {
        CertGrpPtr->NumberOfCertificates = 0;
        CertGrpPtr->Certificates = NULL;
        return CertGrpPtr;
    }

    CertGrpPtr->NumberOfCertificates = NumberOfCertificates;
    CertObjPtr = isl_AllocateMemory(
        MemoryPtr,
        sizeof(ISL_CERTIFICATE_PTR) * NumberOfCertificates);
    if (CertObjPtr == NULL)
    {
        goto FAIL;
    }

    CertGrpPtr->Certificates = CertObjPtr;    
    for(CurrNodePtr = CertificateListPtr, CurrCertObjPtr = *CertObjPtr;
        CurrNodePtr != NULL;
        CurrNodePtr = CurrNodePtr->Next, CurrCertObjPtr++)
    {
        CurrCertObjPtr = CurrNodePtr->Node;
    }
    return CertGrpPtr;

FAIL:
    {
        if (CertGrpPtr)
        {
            isl_FreeMemory(MemoryPtr, CertGrpPtr->Certificates);
        }
        isl_FreeMemory(MemoryPtr, CertGrpPtr);
        return NULL;
    }
}

ISL_STATUS
isl_FreeCertificateGroup(
    ISL_CERTIFICATE_GROUP_PTR CertGrpPtr,
    ISL_MEMORY_CONTEXT_PTR MemoryPtr)
{
    if (CertGrpPtr == NULL) return ISL_OK;

    if (MemoryPtr == NULL) return ISL_FAIL;
    isl_FreeMemory(MemoryPtr, CertGrpPtr->Certificates);
    isl_FreeMemory(MemoryPtr, CertGrpPtr);
    return ISL_OK;
}

ISL_ATTRIBUTE_GROUP_PTR
isl_BuildAttributeGrp(
    ISL_LIST_PTR AttributeListPtr,
    ISL_MEMORY_CONTEXT_PTR MemoryPtr)
{
    ISL_ATTRIBUTE_GROUP_PTR AttributeGrpPtr = NULL;

    if (MemoryPtr == NULL) return NULL;

    {
        ISL_NAME_VALUE_PTR AttributesPtr;
        ISL_LIST_PTR ListNodePtr;
        uint32 AttributeCount;

        AttributeGrpPtr = isl_AllocateMemory(
            MemoryPtr,
            sizeof(ISL_ATTRIBUTE_GROUP));
        if (AttributeGrpPtr == NULL) goto FAIL;

        AttributeCount = isl_CountItemsInList(AttributeListPtr);
        if (AttributeCount == 0) return AttributeGrpPtr;
        AttributeGrpPtr->NumberOfAttributes = AttributeCount;

        AttributesPtr = isl_AllocateMemory(
            MemoryPtr,
            AttributeCount * sizeof(ISL_NAME_VALUE));
        if (AttributesPtr == NULL) goto FAIL;
        AttributeGrpPtr->AttributesPtr = AttributesPtr;

        for(ListNodePtr = AttributeListPtr;
            ListNodePtr != NULL;
            ListNodePtr = ListNodePtr->Next)
        {
            ISL_NAME_VALUE_PTR ListAttributePtr;

            ListAttributePtr = ListNodePtr->Node;
            if (ListAttributePtr == NULL) goto FAIL;
            *AttributesPtr = *ListAttributePtr;
            AttributesPtr++;
        }
        return AttributeGrpPtr;
    }

FAIL:
    {
        if (AttributeGrpPtr)
        {
            isl_FreeMemory(MemoryPtr, AttributeGrpPtr->AttributesPtr);
        }
        isl_FreeMemory(MemoryPtr, AttributeGrpPtr);
        return NULL;
    }
}


ISL_STATUS
isl_FreeAttributeGrp(
    ISL_ATTRIBUTE_GROUP_PTR AttributeGrpPtr,
    ISL_MEMORY_CONTEXT_PTR MemoryPtr)
{
    if (MemoryPtr == NULL) return ISL_FAIL;

    if (AttributeGrpPtr)
    {
        isl_FreeMemory(MemoryPtr, AttributeGrpPtr->AttributesPtr);
    }
    isl_FreeMemory(MemoryPtr, AttributeGrpPtr);
    return ISL_OK;

}
