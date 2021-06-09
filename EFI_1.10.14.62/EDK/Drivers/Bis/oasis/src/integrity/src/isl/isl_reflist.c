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

extern ISL_SIG_SECTION_METHODS SignedListMethods;


/*
**	ISL_SIG_SECTION_LIST class public methods
*/
/* class methods */
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
static	ISL_SIZE SizeofObject_isl_reflist()		/* returns sizeof object */
{
	return sizeof(ISL_SIG_SECTION_LIST);
}

	/* object methods */
/*-----------------------------------------------------------------------------
 * Name: InitHeaderSection
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
InitHeaderSection(
	ISL_SIG_SECTION_LIST_PTR SigSecListPtr,
	ISL_SECTION_INFO_PTR SectionInfoPtr)
{
	ISL_HEADER_SECTION_PTR HeaderPtr;
	ISL_ARCHIVE_CONTEXT_PTR ArchivePtr;

	if (SigSecListPtr == NULL ||
		SigSecListPtr->Parent == NULL)
		return ISL_FAIL;

	ArchivePtr = SigSecListPtr->Parent;
	HeaderPtr = &SigSecListPtr->Header;
	return(isl_BuildHeaderSection(
						HeaderPtr,
						ArchivePtr->Memory,
						SectionInfoPtr));
}

/*-----------------------------------------------------------------------------
 * Name: isl_InitializeSignedListFromImage
 *
 * Description: Construcutor for exisiting object list
 *
 * Parameters: 
 * Context (input) - memory allocated for this instance
 * ArchivePtr (input) - archive which contains signature
 * Name (input) - disk file of external representation
 * Image (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/	
ISL_STATUS isl_InitializeSignedListFromImage(
	ISL_SIG_SECTION_LIST *Context,	
	ISL_ARCHIVE_CONTEXT_PTR ArchivePtr,
	ISL_CONST_DATA Name,
    ISL_CONST_DATA Image)
{
    ISL_MEMORY_CONTEXT_PTR MemoryPtr;
	ISL_SECTION_INFO_GROUP SectionInfoGroup;
	uint32 i;

	if (Context == NULL || 
		ArchivePtr == NULL) 
	{
		return ISL_FAIL;
	}

    cssm_memset(Context, 0, sizeof(ISL_SIG_SECTION_LIST));
  	Context->Parent = ArchivePtr;
    Context->Memory = ArchivePtr->Memory;
    MemoryPtr = Context->Memory;

    Context->Name.Length = Name.Length;
	Context->Name.Data = isl_CopyMemory(
        MemoryPtr, 
        (void*)Name.Data,
		Name.Length);
	if (Context->Name.Data == NULL) {
		return ISL_FAIL;
	}
    Context->Image.Length = Image.Length;
    Context->Image.Data = isl_CopyMemory(
        MemoryPtr,
        (void*)Image.Data,
        Image.Length);
	if (Context->Image.Data == NULL) {
		return ISL_FAIL;
	}

	if (isl_JarFileDecode(
			MemoryPtr,	
			Context->Image, 
			&SectionInfoGroup) != ISL_OK)
	{
		return ISL_FAIL;
	}

	if (SectionInfoGroup.NumberOfSections == 0) return ISL_FAIL;

	/* Initialize Manifest Header then process the Manifest Sections */
	if (InitHeaderSection(Context, SectionInfoGroup.Sections) != ISL_OK)
		return ISL_FAIL;

    {
        ISL_LIST_PTR EndListNodePtr = NULL;

	    Context->SectionCount = SectionInfoGroup.NumberOfSections - 1;
        if (Context->SectionCount == 0) return ISL_OK;

	    for(i=0; i < Context->SectionCount; i++)
	    {	
            ISL_LIST_PTR SignedNodePtr;
            ISL_SIG_SECTION_PTR SignedObjectPtr;
            ISL_STATUS SignedObjectBuilt;

            SignedObjectPtr = isl_AllocateMemory(
                MemoryPtr,
                sizeof(ISL_SIG_SECTION));

            if (SignedObjectPtr == NULL) return ISL_FAIL;

            SignedObjectBuilt = isl_BuildSigSection(
                Context,
                ArchivePtr,
			    SignedObjectPtr, 
			    SectionInfoGroup.Sections + (i + 1));

            if (SignedObjectBuilt != ISL_OK) return ISL_FAIL;

            SignedNodePtr = isl_AllocateMemory(
			    MemoryPtr,
                sizeof(ISL_LIST));
	        if (SignedNodePtr == NULL) return ISL_FAIL;
            SignedNodePtr->Node = SignedObjectPtr;

            if (EndListNodePtr)
            {
                EndListNodePtr->Next = SignedNodePtr;
                EndListNodePtr = SignedNodePtr;
            }
            else
            {
                Context->Section = SignedNodePtr;
                EndListNodePtr = SignedNodePtr;
            }

	    }
    }

	return ISL_OK;
}


/*-----------------------------------------------------------------------------
 * Name: FindSignedObject
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static	ISL_SIG_SECTION_PTR FindSignedObject_isl_reflist(	/* return found object or NULL */
		ISL_SIG_SECTION_LIST *Context,			/* Context */
		ISL_CONST_DATA Name)					/* archive-unique name */
{

	if (Context == NULL ||
		Name.Data == NULL || 
		Name.Length == 0) 
	{
		return NULL;
	}

    {
        ISL_LIST_PTR NodePtr;

	    for(NodePtr = Context->Section;
	        NodePtr;
		    NodePtr = NodePtr->Next) 
        {
	        ISL_MANIFEST_SECTION_PTR SignableObjectPtr;
	        ISL_SIG_SECTION_PTR	SignedObjectPtr;

	        SignedObjectPtr = NodePtr->Node;
		    SignableObjectPtr = SignedObjectPtr->ManSect;
		    if (IS_EQUAL(Name, SignableObjectPtr->SectionInfo.Name) ||
			    IS_EQUAL(Name, SignableObjectPtr->SectionInfo.SectionName) )
		    {
			    return SignedObjectPtr;
		    } 
	    }
	    return NULL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: FindAttributeValue
 *
 * Description:
 * This function find metadata in the archive header.
 *
 * Parameters:
 * Context (input) - archive context
 * Attribute (input) - attribute "name"
 * Value (output) - fill in Value if found
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_STATUS 
FindAttributeValue_isl_reflist(
	ISL_SIG_SECTION_LIST_PTR Context,
	ISL_CONST_DATA Attribute,
	ISL_CONST_DATA_PTR Value)
{
    ISL_STATUS FoundAttributeStatus;

    if (Context == NULL) return ISL_FAIL;

    FoundAttributeStatus = isl_FindAttributeInHeader(
        &Context->Header,
        Attribute,
        Value);

    return FoundAttributeStatus;
}

/*-----------------------------------------------------------------------------
 * Name: GetSignedObjectGroup
 *
 * Description:
 * This function returns a group of signature objects contained in the archive.
 *
 * Parameters:
 * Context(input)
 *
 * Return value:
 * 
 * Error Codes:
 *
 * Notes:
 * Use FreeSignatureGroup to release memory.
 *---------------------------------------------------------------------------*/
static
ISL_SIG_SECTION_GROUP_PTR 
GetSignedObjectGroup_isl_reflist(
    ISL_SIG_SECTION_LIST_PTR Context)
{
    ISL_SIG_SECTION_GROUP_PTR SignedObjectGrpPtr;
    ISL_LIST_PTR CurrNodePtr;
    ISL_SIG_SECTION_PTR *SigObjPtr;

    uint32 NumberOfSignedObjects;

    if (Context == NULL) return NULL;

    SignedObjectGrpPtr = isl_AllocateMemory(
        Context->Memory,
        sizeof(ISL_SIG_SECTION_GROUP));
    if (SignedObjectGrpPtr == NULL) return NULL;

    NumberOfSignedObjects = isl_CountItemsInList(Context->Section);
    if (NumberOfSignedObjects == 0)
    {
        SignedObjectGrpPtr->NumberOfSignedObjects = 0;
        SignedObjectGrpPtr->SignedObjects = NULL;
        return SignedObjectGrpPtr;
    }

    SignedObjectGrpPtr->NumberOfSignedObjects = NumberOfSignedObjects;
    SigObjPtr = isl_AllocateMemory(
        Context->Memory,
        sizeof(ISL_SIG_SECTION_PTR) * NumberOfSignedObjects);
    if (SigObjPtr == NULL)
    {
        goto FAIL;
    }

    SignedObjectGrpPtr->SignedObjects = SigObjPtr;    
    for(CurrNodePtr = Context->Section;
        CurrNodePtr != NULL;
        CurrNodePtr = CurrNodePtr->Next)
    {
        *SigObjPtr = CurrNodePtr->Node;
        SigObjPtr++;
    }
    return SignedObjectGrpPtr;

FAIL:
    {
        isl_FreeMemory(Context->Memory, SignedObjectGrpPtr);
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: FreeSignedObjectGroup
 *
 * Description:
 * This function returns an SM Specific representation of a location map
 *
 * Parameters:
 * Context (input)
 * SignatureInfoGroupPtr (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_STATUS 
FreeSignedObjectGroup_isl_reflist(
    ISL_SIG_SECTION_LIST_PTR Context,
    ISL_SIG_SECTION_GROUP_PTR SignedObjectInfoPtr)
{
    if (Context == NULL) return ISL_FAIL;

    if (SignedObjectInfoPtr == NULL) return ISL_OK;

    return ISL_OK;
}

ISL_SIG_SECTION_METHODS SignedListMethods = {
	{0, 0},
	SizeofObject_isl_reflist,
	FindSignedObject_isl_reflist,
    FindAttributeValue_isl_reflist,
    GetSignedObjectGroup_isl_reflist,
    FreeSignedObjectGroup_isl_reflist
};

