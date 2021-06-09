/*-----------------------------------------------------------------------
 *      File: manisect.c  
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
**	Implementation of ISL_MANIFEST_SECTIONMethods class for Jar archive format
*/
#include "isl_internal.h"
#include "islutil.h"

extern struct isl_manifest_section_methods ManifestSectionMethods;

/*
**	Signable Object (Manifest Section) Class
*/
/* class methods */
static 	
ISL_SIZE SizeofObject_isl_manisec()			/* returns sizeof object */
{
	return sizeof(ISL_MANIFEST_SECTION);
}

/* object methods */
	

/*-----------------------------------------------------------------------------
 * Class: ManifestSections
 * Name:  VerifyCommon
 *
 * Description:
 *
 * Parameters:
 * Context (input)
 * OutputContext (input)
 * Update (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static  
ISL_STATUS VerifyCommon(
	ISL_MANIFEST_SECTION_PTR Context,		/* context */
	void *OutputContext,					/* object to update */
	ISL_STATUS (*Update)(					/* object's update method */
		void *UpdateContext,				/* (place OutputContext here) */
		ISL_CONST_DATA Buffer)) 			/* Length and Value of update */
{
	ISL_GET_DATA_METHODS * GetDataMethods;
	ISL_CONST_DATA Parameters;

	if (Context == NULL ||
		Context->Parent == NULL)
		return ISL_FAIL;
	{
	    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr = Context->Parent;
		ISL_LMAP_ENTRY_PTR MapEntryPtr;
	    ISL_LMAP_PTR MapPtr;
	    uint32 i;

        // Parameters may be written over with correct value in the Is_EQUAL block
        Parameters = Context->Parameters; 

	    MapPtr = ArchiveMethods.GetLocationMap(ArchivePtr);
        if (MapPtr == NULL) return ISL_FAIL;

		GetDataMethods = NULL;
		for(i=0; i < MapPtr->NumberOfEntries; i++)
		{
			MapEntryPtr = MapPtr->MapEntries + i;
			if (IS_EQUAL(MapEntryPtr->JoinName, Context->Parameters) ||
                IS_EQUAL(MapEntryPtr->JoinName, Context->SectionInfo.Name))
			{
				GetDataMethods = MapEntryPtr->GetDataMethod;
				Parameters = MapEntryPtr->GetParameters;
				break;
			}
		}
		if (GetDataMethods == NULL)
        {
            Parameters = Context->Parameters;
		    GetDataMethods = Context->GetDataMethods;
        }
	}
    if (GetDataMethods == NULL) return ISL_FAIL;

    if (ISL_OK != isl_VerifyDigestValues(
						Context,
						Context->SectionInfo.Algorithm,
						GetDataMethods, 
						Parameters, 
						OutputContext, 
						Update))
	{
		return ISL_FAIL;
    }

	return ISL_OK;
}
/*-----------------------------------------------------------------------------
 * Class: ManifestSections
 * Name:  Verify
 *
 * Description:
 * Verifies the integrity of object referenced by this manifest section
 *
 * Parameters:
 * Context (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static	
ISL_STATUS Verify_isl_manisec(
	ISL_MANIFEST_SECTION_PTR Context) 		/* context */
{
    return VerifyCommon(Context, NULL, NULL);
}
#if 0
/*-----------------------------------------------------------------------------
 * Class: ManifestSections
 * Name:  AddAttribute
 *
 * Description:
 * Add metadata to section
 *
 * Parameters:
 * Context (input) - manifest section context
 * Attribute (input) - attribute name
 * Value (input) - attribute value
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/	
static	
ISL_STATUS AddAttribute(
	ISL_MANIFEST_SECTION_PTR Context,
	ISL_CONST_DATA Attribute,
	ISL_CONST_DATA Value)
{
    ISL_MEMORY_CONTEXT_PTR MemoryPtr;
	ISL_NAME_VALUE *item;
	ISL_NAME_VALUE *temp, *prev;
	uint32 i, j, length;
	uint8 *ptr;

	if (Context == NULL || 
		Context->Parent == NULL ||
		Attribute.Data == NULL ||
		Value.Data == NULL)
	{
		return ISL_FAIL;
	}

    MemoryPtr = Context->Parent->Memory;
	/* allocate memory for item */
	item = (ISL_NAME_VALUE *)isl_AllocateMemory(
        MemoryPtr, 
        sizeof(ISL_NAME_VALUE));
	if (item == NULL) {
		return ISL_FAIL;
	}
	/* add attribute and value to the manifest's attribute list */
	item->Name.Length = Attribute.Length;
	item->Name.Data = isl_CopyMemory(
        MemoryPtr, 
		(void *)Attribute.Data, 
        Attribute.Length);
	if (item->Name.Data == NULL) {
		return ISL_FAIL;
	}

	//strip out '\r' and '\n' in the value
	length = 0;
	for (i=0; i<Value.Length; i++){
		if ((Value.Data[i] != '\r') && (Value.Data[i] != '\n'))
			length++;
	}

	//allocate memory
	item->Value.Length = length;
	item->Value.Data = isl_AllocateMemory(MemoryPtr, length);
	if (item->Value.Data == NULL) {
		return ISL_FAIL;
	}

	/* copy value */
	ptr = (uint8 *)item->Value.Data;
	for (i=0, j=0; i<Value.Length; i++){
		if ((Value.Data[i] != '\r') && (Value.Data[i] != '\n')){
			ptr[j] = Value.Data[i];
			j++;
		}
	}

	item->Next = NULL;

	/* add the attribute to the end of attribute list */
	if (Context->SectionInfo.Attribute == NULL) 
    {
 		Context->SectionInfo.Attribute = item;
    }
    else
    {
        prev = NULL;
	    temp = Context->SectionInfo.Attribute;
	    while (temp!= NULL)	{
		    prev = temp;
		    temp = temp->Next;
	    }
		prev->Next = item;
    }
	Context->SectionInfo.AttributeCount++;
	return ISL_OK;

}
#endif

/*-----------------------------------------------------------------------------
 * Class: ManifestSections
 * Name:  FindAttributeValue
 *
 * Description:
 * Find metadata in manifest section
 *
 * Parameters:
 * Context (input) - Manifest section context
 * Attribute (input) - attribute name
 * Value (input) - attribute value
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static	
ISL_STATUS FindAttributeValue_isl_manisec(
	ISL_MANIFEST_SECTION_PTR Context,
	ISL_CONST_DATA Attribute,
	ISL_CONST_DATA_PTR Value)
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
 * Class: ManifestSections
 * Name:  GetAttributeGroup
 *
 * Description:
 * Find metadata in manifest section
 *
 * Parameters:
 * Context (input) - Manifest section context
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_ATTRIBUTE_GROUP_PTR 
GetAttributeGroup_isl_manisec(
    ISL_MANIFEST_SECTION_PTR Context)
{
    ISL_ATTRIBUTE_GROUP_PTR AttributeGrpPtr = NULL;

    AttributeGrpPtr;

    if (Context == NULL) return NULL;
    return isl_BuildAttributeGrp(
        Context->SectionInfo.Attribute,
        Context->Parent->Memory);
}

/*-----------------------------------------------------------------------------
 * Class: ManifestSections
 * Name:  FreeAttributeGroup
 *
 * Description:
 * Find metadata in manifest section
 *
 * Parameters:
 * Context (input) - Manifest section context
 * AttributeGrpPtr (input) - attribute name
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_STATUS 
FreeAttributeGroup_isl_manisec(
    ISL_MANIFEST_SECTION_PTR Context,
    ISL_ATTRIBUTE_GROUP_PTR AttributeGrpPtr)
{
    if (Context == NULL) return ISL_FAIL;

    return isl_FreeAttributeGrp(
        AttributeGrpPtr,
        Context->Parent->Memory);
}

struct isl_manifest_section_methods ManifestSectionMethods = {
	{0, 0},
	SizeofObject_isl_manisec,
	Verify_isl_manisec,
	FindAttributeValue_isl_manisec,
    GetAttributeGroup_isl_manisec,
    FreeAttributeGroup_isl_manisec
};
