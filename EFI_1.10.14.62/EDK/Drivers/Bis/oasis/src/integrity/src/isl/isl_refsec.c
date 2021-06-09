/*-----------------------------------------------------------------------
 *      File: reflist.c  
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
**	ISL_SIG_SECTION_LIST class public methods
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "isl_internal.h"
#include "islutil.h"

extern ISL_SIGNED_SECTION_METHODS SignedObjectMethods;
#define gsGetImage "GetImage"

/* implement a simple get data method for manifest section images */
/*-----------------------------------------------------------------------------
 * Name: cssmdataid
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static void cssmdataid(
	ISL_SERVICE_CLASS *AlgClass,		/* return service type code */
	ISL_CONST_DATA_PTR AlgID,			/* return archive-specific encoding */
	ISL_CONST_DATA_PTR ServiceName)		/* return human-readable description */
{
	/* used only for testing */
	*AlgClass = ISL_ServiceGetData;
	AlgID->Data = 0;
	AlgID->Length = 0;
	ServiceName->Data = (const uint8*)gsGetImage;
	ServiceName->Length = sizeof(gsGetImage)-1;
}
/*-----------------------------------------------------------------------------
 * Name: cssmdatasize
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_SIZE cssmdatasize()
{
	return sizeof(ISL_DATA);
}

/*-----------------------------------------------------------------------------
 * Name: cssmdatainit
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS cssmdatainit(
	ISL_GET_DATA_SERVICE_CONTEXT *memory,
	ISL_CONST_DATA parameters)
{
	ISL_DATA_PTR out = (ISL_DATA_PTR) memory;
	out->Data = (void*) parameters.Data;
	out->Length = parameters.Length;
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: cssmdataupdate
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_ERROR_CODE cssmdataupdate(
	ISL_GET_DATA_SERVICE_CONTEXT *context,
	ISL_CONST_DATA *data)
{
	ISL_DATA_PTR it = (ISL_DATA_PTR) context;
	data->Data = (const void *) it->Data;
	data->Length = it->Length;
	it->Length = 0;			/* next call gets "end of stream" */
	return ISL_NO_ERROR;
}

/*-----------------------------------------------------------------------------
 * Name: cssmdatarecycle
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
#pragma warning(disable: 4100)
static ISL_ERROR_CODE cssmdatarecycle(
	ISL_GET_DATA_SERVICE_CONTEXT *context)
{	
	return ISL_NO_ERROR;
}
#pragma warning(default:4100)


/*-----------------------------------------------------------------------------
 * Name: cssmdatainitwithclass
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
#pragma warning(disable: 4100)
static ISL_STATUS cssmdatainitwithclass(
	ISL_GET_DATA_SERVICE_CONTEXT *Memory,
	ISL_CONST_DATA Parameters,					/* archive-specific-encoded parameters */
	ISL_CLASS_PTR Class,						/* class structure */
	ISL_MANIFEST_SECTION_PTR Section)			/* manifest section, if any */
{
	return cssmdatainit(Memory, Section->SectionInfo.Image);
}
#pragma warning(default:4100)

/*-----------------------------------------------------------------------------
 * Name: SizeofObject
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_GET_DATA_METHODS getManSectImage = { 
	{cssmdataid, 0},
	cssmdatasize, 
	cssmdatainit, 
	cssmdataupdate, 
	cssmdatarecycle,
	cssmdatainitwithclass};
/*-----------------------------------------------------------------------------
 * Name: SizeofObject
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
ISL_SIZE SizeofObject_isl_refsec()
{
    return sizeof(ISL_SIG_SECTION);
}

/*-----------------------------------------------------------------------------
 * Name: isl_VerifyManSectWithAllDigestValues
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
isl_VerifyManSectWithAllDigestValues(
	ISL_SIG_SECTION_PTR pSigSect)
{
	ISL_CONST_DATA message;
	ISL_GET_DATA_METHODS *GetDataMethods;

	if (pSigSect == NULL ||
		pSigSect->ManSect == NULL ||
		pSigSect->ManSect->SectionInfo.Image.Data == NULL ||
		pSigSect->ManSect->SectionInfo.Image.Length == 0)
		return ISL_FAIL;

	message.Data = (uint8*)pSigSect->ManSect->SectionInfo.Image.Data;
	message.Length = (long) pSigSect->ManSect->SectionInfo.Image.Length;

	GetDataMethods = &getManSectImage;
	/* compute the digest of manifest section using the digest algorithms */
	/* indicated in the signed object list */
	if (ISL_OK != isl_VerifyDigestValues(
				pSigSect->ManSect, 
				pSigSect->SectionInfo.Algorithm,
				GetDataMethods,
				message, 
				NULL,
				NULL)) 
		return ISL_FAIL;
    	
	return ISL_OK;
}
/*-----------------------------------------------------------------------------
 * Name: GetManifestSection
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
ISL_MANIFEST_SECTION_PTR GetManifestSection_isl_refsec(
    ISL_SIG_SECTION_PTR Context)
{
    if (Context == NULL) return NULL;
    return Context->ManSect;
}
 
/*-----------------------------------------------------------------------------
 * Name: FindAttributeValue
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
ISL_STATUS FindAttributeValue_isl_refsec(			/* find metadata in section */
	ISL_SIG_SECTION_PTR Context,		/* archive context */
	ISL_CONST_DATA Attribute,			/* attribute "name" */
	ISL_CONST_DATA_PTR Value)			/* fill in Value and Length if found */
{
	ISL_MEMORY_CONTEXT_PTR MemoryPtr;
    ISL_LIST_PTR curr;
	
	if (Context == NULL ||
        Context->Parent == NULL ||
        Attribute.Data == NULL || 
		Value == NULL) 
	{
		return ISL_FAIL;
	}

    MemoryPtr = Context->Parent->Memory;
	for (curr = Context->SectionInfo.Attribute; curr; curr = curr->Next)
	{
        ISL_NAME_VALUE_PTR AttributePtr;

        AttributePtr = curr->Node;
		if (IS_EQUAL(AttributePtr->Name, Attribute))
		{
			Value->Length  = AttributePtr->Value.Length;
			Value->Data = AttributePtr->Value.Data; 
			return ISL_OK;
		}
	}

	return ISL_FAIL;
}

/*-----------------------------------------------------------------------------
 * Name: GetAttributeGroup
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
ISL_ATTRIBUTE_GROUP_PTR GetAttributeGroup_isl_refsec(
    ISL_SIG_SECTION_PTR Context)
{
    if (Context == NULL) return NULL;

    return isl_BuildAttributeGrp(
        Context->SectionInfo.Attribute,
        Context->Parent->Memory);
}

/*-----------------------------------------------------------------------------
 * Name: FreeAttributeGroup
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
ISL_STATUS FreeAttributeGroup_isl_refsec(
    ISL_SIG_SECTION_PTR Context,
    ISL_ATTRIBUTE_GROUP_PTR AttributeInfoPtr)
{
    if (Context == NULL) return ISL_FAIL;

    return isl_FreeAttributeGrp(
        AttributeInfoPtr,
        Context->Parent->Memory);
}

ISL_SIGNED_SECTION_METHODS SignedObjectMethods = {
    {0, 0},
	/* class methods */
	SizeofObject_isl_refsec,			/* returns sizeof object */
	/* object methods */
	isl_VerifyManSectWithAllDigestValues,
    GetManifestSection_isl_refsec,
	FindAttributeValue_isl_refsec,
    GetAttributeGroup_isl_refsec,
    FreeAttributeGroup_isl_refsec
} ;
