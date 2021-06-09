/*-----------------------------------------------------------------------
 *      File:   m_parse.c
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
#include "isl_internal.h"
#include "islutil.h"
#include "isl_parse.h"

extern ISL_CONFIG_METHODS ArchiveConfigMethods;

static ISL_ALG_INFO_PTR 
AllocAlgList(
	ISL_MEMORY_CONTEXT_PTR MemoryPtr,
	unsigned long count);				/* length of list */

static unsigned long
CountItemsInList(
	const unsigned char *ptr, 
	unsigned int len);

static ISL_STATUS 
InitAlgList(
	ISL_ALG_INFO_PTR list, 
	ISL_CONST_DATA Value,
	ISL_ARCHIVE_CONTEXT_PTR	pArchive);

ISL_STATUS
ParseToNewline(
	ISL_CONST_DATA Input,		/* buffer to be parsed */
	ISL_CONST_DATA_PTR UpdatedInput);

ISL_STATUS
ParseToEndOfSection(
	ISL_CONST_DATA Input,					/* buffer to be parsed */
	ISL_CONST_DATA_PTR UpdatedInput,		/* pointer to remaining buffer */
	ISL_CONST_DATA_PTR SectionImage);		/* SectionImage */

static ISL_STATUS 
AddManifestSection(
	ISL_ARCHIVE_CONTEXT_PTR  pArchive, 
	ISL_MANIFEST_SECTION_PTR pManSect);

static ISL_STATUS 
InitializeManifestSection(
	ISL_ARCHIVE_CONTEXT_PTR pArchive, 
	ISL_MANIFEST_SECTION_PTR pManSect);

static ISL_STATUS
GetManifestSection(
	ISL_CONST_DATA_PTR ptr, 
	ISL_MANIFEST_SECTION *data);

static ISL_ALG_INFO_PTR 
MatchDigest(
	ISL_CONST_DATA Name, 
	ISL_ALG_INFO_PTR AlgList);

static ISL_STATUS AddAttribute(
	ISL_CONST_DATA Name, 
	ISL_CONST_DATA Value, 
	ISL_SECTION_PTR SectionPtr,
	ISL_MEMORY_CONTEXT_PTR MemoryPtr);

/*-----------------------------------------------------------------------------
 * Name: ParseVersion
 *
 * Description:
 * Parses a version string into a major/minor version structure
 *
 * Parameters:
 * HeaderInfoPtr (output)
 * Version (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
ParseVersion( 
	ISL_HEADER_SECTION_PTR HeaderInfoPtr,
	CSSM_DATA Version)
{
	const uint8		*ptr = NULL;
	uint32		MinorLength = 0;
	uint32		MajorLength = 0;
	uint32		i = 0;

	if (Version.Length > 7)
		return ISL_FAIL;

	ptr = isl_memchr(Version.Data,'.',Version.Length);	
	if (!ptr)
		return ISL_FAIL;
	
	MajorLength = (uint32)(ptr - Version.Data);
	MinorLength = Version.Length - (MajorLength + 1);
	
	if ( (MajorLength > 3 ) ||
		 (MinorLength > 3 ) )
		return ISL_FAIL;

	ptr = Version.Data;
	HeaderInfoPtr->MajorVersion = 0;
	for (i = 0; i < MajorLength; i++)
	{
		if (!ISL_ISDIGIT(*(ptr+i)) )
			return ISL_FAIL;

		HeaderInfoPtr->MajorVersion *= 10;
		HeaderInfoPtr->MajorVersion = (uint16) (HeaderInfoPtr->MajorVersion + *(ptr+i) - '0');
	}

	ptr = Version.Data + (MajorLength + 1);
	HeaderInfoPtr->MinorVersion = 0;
	for (i = 0; i < MinorLength; i++)
	{
		if (!ISL_ISDIGIT(*(ptr+i)) )
			return ISL_FAIL;

		HeaderInfoPtr->MinorVersion *= 10;
		HeaderInfoPtr->MinorVersion = (uint16) (HeaderInfoPtr->MinorVersion + *(ptr+i) - '0');
	}

	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: InitializeSectionAlgList
 *
 * Description:
 *
 * Parameters:
 * ArchivePtr (output)
 * SectionPtr (output)
 * Value (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_ALG_INFO_PTR
InitializeSectionAlgList(
	ISL_ARCHIVE_CONTEXT_PTR ArchivePtr,
	ISL_SECTION_PTR SectionPtr, 
	ISL_CONST_DATA Value)
{
	ISL_ALG_INFO_PTR		list = NULL;
	unsigned long			count = 0;

	count = CountItemsInList(Value.Data,Value.Length);
	list = AllocAlgList(ArchivePtr->Memory, count);
	if (!list)
		return NULL;

	if (InitAlgList(list, Value, ArchivePtr) != ISL_OK)
		return NULL;

	SectionPtr->Algorithm = list;
	SectionPtr->AlgCount = count;
	return list;
}

/*-----------------------------------------------------------------------------
 * Name: IsHashAlgorithm
 *
 * Description:
 *
 * Parameters:
 * Name (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static uint32
IsHashAlgorithm(
    ISL_CONST_DATA Name)
{
    ISL_CONST_DATA STR;

    STR.Data = (const uint8 *)"Digest-Algorithms";
	STR.Length = sizeof("Digest-Algorithms")-1;

	return IS_EQUAL(Name,STR);
}

/*-----------------------------------------------------------------------------
 * Name: IsName
 *
 * Description:
 *
 * Parameters:
 * Name (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static uint32
IsName(
    ISL_CONST_DATA Name)
{
    ISL_CONST_DATA STR;

    STR.Data = (const uint8 *)"Name";
	STR.Length = sizeof("Name")-1;

	return IS_EQUAL(Name,STR);
}

/*-----------------------------------------------------------------------------
 * Name: IsSectionName
 *
 * Description:
 *
 * Parameters:
 * Name (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static uint32
IsSectionName(
    ISL_CONST_DATA Name)
{
    ISL_CONST_DATA STR;

    STR.Data = (const uint8 *)"SectionName";
    STR.Length = sizeof("SectionName")-1;

	return IS_EQUAL(Name,STR);
}

/*-----------------------------------------------------------------------------
 * Name: IsHash
 *
 * Description:
 *
 * Parameters:
 * Name (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static uint32
IsHash(
    ISL_CONST_DATA Name)
{
    ISL_CONST_DATA STR;

    STR.Data = (const uint8 *)"-Digest";
    STR.Length = sizeof("-Digest")-1;

	return IS_EQUAL(Name,STR);
}

/*-----------------------------------------------------------------------------
 * Name: AddAttribute
 *
 * Description:
 *
 * Parameters:
 * Name (input)
 * Value (input)
 * SectionPtr (output)
 * MemoryPtr (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS 
AddAttribute(
	ISL_CONST_DATA Name, 
	ISL_CONST_DATA Value, 
	ISL_SECTION_PTR SectionPtr,
	ISL_MEMORY_CONTEXT_PTR MemoryPtr)
{
	ISL_NAME_VALUE_PTR AttributePtr = NULL;
    ISL_LIST_PTR AttributeListPtr = NULL;
//	ISL_ARCHIVE_CONTEXT_PTR pArchive = NULL;

    {
	    ISL_LIST_PTR pCurr;

        AttributeListPtr = (ISL_LIST_PTR) isl_AllocateMemory(
            MemoryPtr,
            sizeof(ISL_LIST));
        if (AttributeListPtr == NULL) {
            goto FAIL;
        }

        AttributePtr = (ISL_NAME_VALUE_PTR) isl_AllocateMemory(
            MemoryPtr,
            sizeof(ISL_NAME_VALUE));
	    if (AttributePtr == NULL) {
		    goto FAIL;
	    }

        AttributeListPtr->Node = AttributePtr;
        AttributeListPtr->Next = NULL;

	    AttributePtr->Name = Name;
	    AttributePtr->Value = Value;

	    SectionPtr->AttributeCount++;
	    pCurr = SectionPtr->Attribute;

	    if (pCurr == NULL)
	    {
		    SectionPtr->Attribute = AttributeListPtr;
		    return ISL_OK;
	    }
	    
	    while (pCurr->Next != NULL) {
		    pCurr = pCurr->Next;
        }

	    pCurr->Next = AttributeListPtr;

	    return ISL_OK;
    }

FAIL:
    {
        return ISL_FAIL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: InitializeSection
 *
 * Description:
 * This function will initialize an ISL_SECTION. 
 *
 * Parameters:
 * SectionPtr (output)
 * SectionInfoPtr (input)
 * ArchivePtr (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS
InitializeSection(
	ISL_SECTION_PTR	SectionPtr,
	ISL_SECTION_INFO_PTR SectionInfoPtr,
    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr)
{
	ISL_MEMORY_CONTEXT_PTR MemoryPtr;
	ISL_CONST_DATA Name;
	ISL_CONST_DATA Value;
	ISL_ALG_INFO_PTR DigestPtr = NULL;
	ISL_ALG_INFO_PTR HashInfoPtr;
	uint32 i;

	if (SectionInfoPtr->Attributes.NumberOfAttributes == 0 ||
        ArchivePtr == NULL) 
        return ISL_FAIL;

    MemoryPtr = ArchivePtr->Memory;
	/* first attribute is a name */
	Name.Length = SectionInfoPtr->Attributes.Attributes[0].Info.Label.Name.Length;
	Name.Data = SectionInfoPtr->Attributes.Attributes[0].Info.Label.Name.Data;
	if (IsName(Name)) {
		SectionPtr->Name.Data = SectionInfoPtr->Attributes.Attributes[0].Value.Data;
		SectionPtr->Name.Length = SectionInfoPtr->Attributes.Attributes[0].Value.Length;
	} else {
		return ISL_FAIL;
	}

	SectionPtr->Image = SectionInfoPtr->Image;

	/* search for digest tokens */
	for(i=0; i < SectionInfoPtr->Attributes.NumberOfAttributes; i++)
	{
		Name.Length = SectionInfoPtr->Attributes.Attributes[i].Info.Label.Name.Length;
		Name.Data = SectionInfoPtr->Attributes.Attributes[i].Info.Label.Name.Data;

		if (IsHashAlgorithm(Name))
        {
			Value.Length = SectionInfoPtr->Attributes.Attributes[i].Value.Length;
			Value.Data = SectionInfoPtr->Attributes.Attributes[i].Value.Data;			
			DigestPtr = InitializeSectionAlgList(ArchivePtr, SectionPtr, Value);
			break;
		}
	}

	for(i=0; i < SectionInfoPtr->Attributes.NumberOfAttributes; i++)
	{
		ISL_ATTRIBUTE_INFO_PTR AttributePtr;

		AttributePtr = SectionInfoPtr->Attributes.Attributes + i;
		if (AttributePtr->Info.AttributeNameFormat != CSSM_DB_ATTRIBUTE_NAME_AS_BLOB)
		{
			return ISL_FAIL;
		}
		Name.Length = AttributePtr->Info.Label.Name.Length;
		Name.Data = AttributePtr->Info.Label.Name.Data;	
		Value.Length = AttributePtr->Value.Length;
		Value.Data = AttributePtr->Value.Data;

		if (AddAttribute(Name, Value, SectionPtr, MemoryPtr) != ISL_OK)
			return ISL_FAIL;

		if (IsSectionName(Name))
		{
			SectionPtr->SectionName = Value;
		} else 
		{
			ISL_HASH_INFO_PTR HashNodePtr;

			HashInfoPtr = MatchDigest(Name, DigestPtr);
			if (HashInfoPtr == NULL) continue;

			/* Allocating a HashNode */
			HashNodePtr = (ISL_HASH_INFO_PTR) isl_AllocateMemory(
												MemoryPtr,
												sizeof(ISL_HASH_INFO));
			if(HashNodePtr == NULL) {
				return ISL_FAIL;
			}
			/* Initializing HashNode */
			HashNodePtr->Name = Name;
			HashNodePtr->Value.Length = IslUtil_Base64DecodeSize((ISL_DATA *)&Value);
			if (HashNodePtr->Value.Length == 0)
			{
				return ISL_FAIL;
			}
			HashNodePtr->Value.Data = isl_AllocateMemory(MemoryPtr, HashNodePtr->Value.Length);
			if (HashNodePtr->Value.Data == NULL)
			{
				return ISL_FAIL;
			}

			if (ISL_OK != IslUtil_Base64Decode((ISL_DATA *)&Value, (ISL_DATA *)&HashNodePtr->Value))
			{
				return ISL_FAIL;
			}
			HashNodePtr->Next = HashInfoPtr->Hash;
			/* Inserting at the head of the list */
			HashInfoPtr->Hash = HashNodePtr;
			HashInfoPtr->HashCount++;
		}
	}
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: isl_GetProtocolAndParameters
 *
 * Description:
 *
 * Parameters:
 * Name (input)
 * Protocol (output)
 * Parameters (output)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
isl_GetProtocolAndParameters(
	ISL_CONST_DATA Name, 
	ISL_CONST_DATA_PTR Protocol, 
	ISL_CONST_DATA_PTR Parameters)
{
	if (Protocol == NULL) return ISL_FAIL;
	if (Parameters == NULL) return ISL_FAIL;
	Parameters->Data = isl_memchr(Name.Data,':',Name.Length);
	if (Parameters->Data == NULL) return ISL_FAIL;
	Parameters->Data++;    /* skip over ':' */
	Parameters->Length = Name.Length - (uint32)(Parameters->Data - Name.Data);
	if (Parameters->Length > Name.Length) return ISL_FAIL;
	Protocol->Data = Name.Data;
	Protocol->Length = Name.Length - (Parameters->Length + 1);
	if (Protocol->Length > Name.Length) return ISL_FAIL;
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: isl_BuildHeaderSection
 *
 * Description:
 *
 * Parameters:
 * HeaderPtr (output)
 * MemoryPtr (input)
 * SectionInfoPtr (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
isl_BuildHeaderSection(
	ISL_HEADER_SECTION_PTR HeaderPtr,
	ISL_MEMORY_CONTEXT_PTR MemoryPtr,
	ISL_SECTION_INFO_PTR SectionInfoPtr)
{
    ISL_NAME_VALUE_PTR AttributeNodePtr;
	ISL_NAME_VALUE_PTR NameValuePtr;
	uint32 i;

	if (SectionInfoPtr->Attributes.NumberOfAttributes == 0) return ISL_FAIL;

	if (ISL_OK != ParseVersion(
					HeaderPtr, 
					SectionInfoPtr->Attributes.Attributes[0].Value))
		return ISL_FAIL;

	HeaderPtr->AttributeCount = SectionInfoPtr->Attributes.NumberOfAttributes;
	HeaderPtr->Attribute = isl_AllocateMemory(
        MemoryPtr,
        sizeof(ISL_LIST) * HeaderPtr->AttributeCount);
	if (HeaderPtr->Attribute == NULL)
	{
		return ISL_FAIL;
	}

    AttributeNodePtr = isl_AllocateMemory(
	    MemoryPtr,
		sizeof(ISL_NAME_VALUE)*HeaderPtr->AttributeCount);
    if (AttributeNodePtr == NULL)
    {
        return ISL_FAIL;
    }

	for(i=0;
		i < SectionInfoPtr->Attributes.NumberOfAttributes; 
		i++)
	{
		ISL_ATTRIBUTE_INFO_PTR AttributePtr;

		AttributePtr = SectionInfoPtr->Attributes.Attributes + i;
		if (AttributePtr->Info.AttributeNameFormat != 
            CSSM_DB_ATTRIBUTE_NAME_AS_BLOB)
		{
			return ISL_FAIL;
		}
		
        NameValuePtr = AttributeNodePtr + i;
        NameValuePtr->Name.Length = AttributePtr->Info.Label.Name.Length;
		NameValuePtr->Name.Data = AttributePtr->Info.Label.Name.Data;
		NameValuePtr->Value.Length = AttributePtr->Value.Length;
		NameValuePtr->Value.Data = AttributePtr->Value.Data;

        HeaderPtr->Attribute[i].Node = NameValuePtr;
        HeaderPtr->Attribute[i].Next = &(HeaderPtr->Attribute[i + 1]);
	}
	HeaderPtr->Attribute[HeaderPtr->AttributeCount - 1].Next = NULL;
    return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: isl_BuildManifestSection
 *
 * Description:
 *
 * Parameters:
 * ManifestSectionPtr (output)
 * SectionInfoPtr (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
isl_BuildManifestSection(
	ISL_MANIFEST_SECTION_PTR ManifestSectionPtr,
	ISL_SECTION_INFO_PTR SectionInfoPtr)
{
    ISL_SERVICE_CLASS_METHODS *methods;
	ISL_ARCHIVE_CONTEXT_PTR pArchive;
	ISL_SECTION_PTR SectionPtr;

	if (SectionInfoPtr == NULL) return ISL_FAIL;
	if (ManifestSectionPtr == NULL) return ISL_FAIL;
	pArchive = ManifestSectionPtr->Parent;
	if (pArchive == NULL) return ISL_FAIL;

	SectionPtr = &(ManifestSectionPtr->SectionInfo);
	
	if (InitializeSection(SectionPtr, SectionInfoPtr, pArchive) != ISL_OK)
	{
		return ISL_FAIL;
	}

	if (ManifestSectionPtr->SectionInfo.Name.Data == NULL) return ISL_FAIL;
	if (ManifestSectionPtr->SectionInfo.Name.Length != 13 ||
		cssm_memcmp(
			ManifestSectionPtr->SectionInfo.Name.Data,
			"00LocationMap",
			ManifestSectionPtr->SectionInfo.Name.Length) != 0)
	{
		if (isl_GetProtocolAndParameters(
				ManifestSectionPtr->SectionInfo.Name, 
				&ManifestSectionPtr->Protocol, 
				&ManifestSectionPtr->Parameters) != ISL_OK)
		{
			return ISL_FAIL;
		}
		methods = ArchiveConfigMethods.FindAlgorithm(
			pArchive->Config, 
			ManifestSectionPtr->Protocol);
		
		ManifestSectionPtr->GetDataMethods = (ISL_GET_DATA_METHODS *)methods;
	}
	else
	{
		/* special section */
	}
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: isl_BuildSigSection
 *
 * Description:
 *
 * Parameters:
 * ArchivePtr (input)
 * SigSectionPtr (output)
 * SectionInfoPtr (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
isl_BuildSigSection(
    ISL_SIG_SECTION_LIST_PTR SignedObjectListPtr,
    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr,
	ISL_SIG_SECTION_PTR SignedObjectPtr,
	ISL_SECTION_INFO_PTR SectionInfoPtr)
{	
    ISL_LIST_PTR ManSectListPtr;
	ISL_SECTION_PTR	SectionPtr;

	if (SignedObjectPtr == NULL ||
	    ArchivePtr == NULL) 
        return ISL_FAIL;

    SignedObjectPtr->Parent = SignedObjectListPtr;
 	SectionPtr = &(SignedObjectPtr->SectionInfo);
	
	if (InitializeSection(SectionPtr, SectionInfoPtr, ArchivePtr) != ISL_OK)
	{
		return ISL_FAIL;
	}
 
	for (ManSectListPtr = ArchivePtr->Manifest.Section;
		 ManSectListPtr != NULL;
		 ManSectListPtr = ManSectListPtr->Next)
	{
	    ISL_MANIFEST_SECTION_PTR	ManifestSectionPtr;

        ManifestSectionPtr = ManSectListPtr->Node;
		if (IS_EQUAL(
            ManifestSectionPtr->SectionInfo.Name, 
            SignedObjectPtr->SectionInfo.Name))
		{
			SignedObjectPtr->ManSect = ManifestSectionPtr;
		}
	}
	
	if (SignedObjectPtr->ManSect == NULL) return ISL_FAIL;

	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: MatchDigest
 *
 * Description:
 *
 * Parameters:
 * Name (input)
 * AlgList (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_ALG_INFO_PTR 
MatchDigest(
	ISL_CONST_DATA Name, 
	ISL_ALG_INFO_PTR AlgList)
{
	ISL_CONST_DATA AlgName;		/* digest name e.g. md5, sha-1 */
	uint32 i =0;

	if (Name.Length <= HASH_STR_LENGTH) return NULL;

	AlgName.Length = HASH_STR_LENGTH;

	for(i=0; i <= Name.Length - HASH_STR_LENGTH; i++)
	{
		AlgName.Data = Name.Data + i;

		if (IsHash(AlgName))
		{
			for(AlgName.Data = Name.Data, 	AlgName.Length = i;
				AlgList != NULL;
				AlgList = AlgList->Next)
			{
				if (IS_EQUAL(AlgName, AlgList->AlgName))
					return (AlgList);
			}

			return NULL;
		}
	}

	return NULL;	/* no match found */

}

/*-----------------------------------------------------------------------------
 * Name: AllocAlgList
 *
 * Description:
 *
 * Parameters:
 * MemoryPtr (input)
 * count (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_ALG_INFO_PTR
AllocAlgList(
	ISL_MEMORY_CONTEXT_PTR MemoryPtr,
	unsigned long count)				/* length of list */
{
	unsigned long			i = 0;			/* loop counter */
	ISL_ALG_INFO_PTR		list = NULL;	/* list to be allocated */

	/* allocating list of ISL_ALG_INFO */
	list = (ISL_ALG_INFO_PTR) isl_AllocateMemory(MemoryPtr, count*sizeof(ISL_ALG_INFO));
	if (!list) {
		return NULL;
	}

	/* initializing each item in the list */
	for(i=0; i < count; i++)
	{
		list[i].Parent = NULL;
		list[i].Next = list + (i+1);
	}

    list[count-1].Next = NULL;
	return list;
}

/*-----------------------------------------------------------------------------
 * Name: CountItemsInList
 *
 * Description:
 * Counts the number of items in a space separated list
 *
 * Parameters: 
 * ptr (input)
 * len (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static unsigned long
CountItemsInList(
	const unsigned char *ptr, 
	unsigned int len)
{
	unsigned long count = 0;

	while (len > 0)
	{
		while (len > 0 && *ptr == ' ' && !IS_NEWLINE(*ptr))
		{
			ptr++; len--;
		}

		if (len == 0 || IS_NEWLINE(*ptr))
			return count;

		count++;

		while (len > 0 && IS_HEADERCHAR(*ptr))
		{
			ptr++; len--;
		}
	}

	return count;
}

/*-----------------------------------------------------------------------------
 * Name: InitAlgList
 *
 * Description:
 * This function will initialize algorithm information list using the value
 * of an attribute called Digest-Algorithms
 *
 * Parameters:
 * list (input/output)			: list of algorithm infos
 * Value (input)				: space separated list of digest algorithms
 * pArchive (input)				: used for list of supported algorithms
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS
InitAlgList(
	ISL_ALG_INFO_PTR AlgInfoPtr, 
	ISL_CONST_DATA Value,
	ISL_ARCHIVE_CONTEXT_PTR ArchivePtr)
{
	const uint8	        *oldptr = NULL;
    const uint8         *ptr = NULL;
	uint32	            oldlen = 0;
    uint32              len = 0;

	for(ptr = Value.Data, len = Value.Length;
		AlgInfoPtr != NULL;
		AlgInfoPtr = AlgInfoPtr->Next)
	{
        ISL_SERVICE_CLASS_METHODS *methods;

		if (!ptr)
			return ISL_FAIL;

		/* looking for beginning of token */
		while (len > 0 && !IS_HEADERCHAR(*ptr) )
		{
			ptr++;
			len--;
		}

		/* testing for valid length */
		if (!(len > 0))
			return ISL_FAIL;

		/* looking for end of token */
		oldptr = ptr;
		ptr = isl_memchr(oldptr, ' ', len);
	
		/* calculate the length of token */
		oldlen = (ptr) ? (uint32)(ptr-oldptr) : len;

		/* initialize AlgName */
        AlgInfoPtr->AlgName.Length = oldlen;
        AlgInfoPtr->AlgName.Data = isl_CopyMemory(ArchivePtr->Memory, oldptr, oldlen);
        if (AlgInfoPtr->AlgName.Data == NULL) {
			return ISL_FAIL;
        }
        methods = ArchiveConfigMethods.FindAlgorithm(ArchivePtr->Config, AlgInfoPtr->AlgName);
		if (methods) {
			AlgInfoPtr->Methods = (ISL_DIGEST_METHODS *) methods;
		}
		/* update length of ptr*/
		len -= oldlen;
	}

	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: ParseAttribute
 *
 * Description:
 * This function will accept a buffer and produce a name/value pair. The output
 * ptrs point to portions of the buffer!
 *
 * Parameters: 
 * Input (input)			: buffer to be parsed
 * UpdatedInput (output)	: ptr to remaining buffer
 * NameImage (output)		: ptr to name
 * ValueImage (output)		: ptr to value
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
ParseAttribute(
	ISL_CONST_DATA Input,
	ISL_CONST_DATA_PTR UpdatedInput,
	ISL_CONST_DATA_PTR NameImage,
	ISL_CONST_DATA_PTR ValueImage)
{
    const unsigned char *endptr;		/* pointer to the end of a token */

	if (NameImage == NULL) return ISL_FAIL;
	if (ValueImage == NULL) return ISL_FAIL;

	endptr = Input.Data + Input.Length;

    /* Find Name (oldptr, oldlen) */
	NameImage->Data = Input.Data;
	UpdatedInput->Data = isl_memchr(Input.Data, ':', Input.Length);		
	if (UpdatedInput->Data == NULL) return ISL_FAIL;
	NameImage->Length = (uint32)(UpdatedInput->Data - Input.Data);
	if (NameImage->Length > 70) return ISL_FAIL;

	/* update input pointers (ptr, maxlen) */
	UpdatedInput->Data++;									/* skip ':' */
	UpdatedInput->Data++;									/* skip SPACE */
	if (UpdatedInput->Data > endptr) return ISL_FAIL;		/* check to see if in la-la land */
	UpdatedInput->Length = (uint32)(endptr - UpdatedInput->Data);		/* update length */	
	
	/* Initialize Value */
	ValueImage->Data = UpdatedInput->Data;

	while (endptr > UpdatedInput->Data)
	{
		Input = *UpdatedInput;
		if (ISL_OK != ParseToNewline(Input, UpdatedInput)) return ISL_FAIL;
		if (UpdatedInput->Length == 0)
		{
			ValueImage->Length = (uint32)(UpdatedInput->Data - ValueImage->Data);
			return ISL_OK;
		}
		if (*(UpdatedInput->Data) != ' ') break;			/* no continuation exit loop */
		UpdatedInput->Data++;
		UpdatedInput->Length--;
	}

	ValueImage->Length = (uint32)(UpdatedInput->Data - ValueImage->Data);

	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: CountAttributes
 *
 * Description:
 * This functions counts the number of attributes in a buffer containing a
 * section
 *
 * Parameters:
 * Input (input)		: a buffer containing a section
 *
 * Return value:
 * The number of attributes in the buffer containing a section
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
uint32
CountAttributes(
	ISL_CONST_DATA Input)
{
	uint32 count = 0;
	ISL_CONST_DATA UpdatedInput;
	ISL_CONST_DATA NameImage;
	ISL_CONST_DATA ValueImage;

	while(	Input.Length > 0 &&
			ParseAttribute(Input, &UpdatedInput, &NameImage, &ValueImage) == ISL_OK )
	{
		Input = UpdatedInput;
		count++;
	}
	return count;
}

/*-----------------------------------------------------------------------------
 * Name: CountSections
 *
 * Description:
 * This function will count the number of sections in a buffer
 *
 * Parameters:
 * Input (input)		: buffer to be inspected for sections
 *
 * Return value:
 * Number of sections in the buffer
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
uint32
CountSections(
	ISL_CONST_DATA Input)
{
	uint32 count = 0;
	ISL_CONST_DATA UpdatedInput;
	ISL_CONST_DATA SectionImage;

	while(	Input.Length > 0 &&
			ParseToEndOfSection(Input, &UpdatedInput, &SectionImage) == ISL_OK )
	{
		Input = UpdatedInput;
		count++;
	}
	return count;
}
/*-----------------------------------------------------------------------------
 * Name: StripAttributeValue
 *
 * Description:
 * This function will strip the attribute value of newline characters and
 * continutation characters. This process happens in place with the buffer
 * containing the attribute value being modified.
 *
 * Parameters:
 * AttributeInfoPtr (input/ouput)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS StripAttributeValue(
	ISL_ATTRIBUTE_INFO_PTR AttributeInfoPtr)
{
	uint32 oldlen;
	uint8 *oldptr;
	uint8 *endptr;
	uint8 *DataPtr;

	if (AttributeInfoPtr == NULL) return ISL_FAIL;

	oldptr = AttributeInfoPtr->Value.Data;
	oldlen = AttributeInfoPtr->Value.Length;
	endptr = AttributeInfoPtr->Value.Data + AttributeInfoPtr->Value.Length;
	DataPtr = AttributeInfoPtr->Value.Data;
	while (oldptr < endptr) 
	{
		uint8 *CR;
		uint8 *LF;
		uint8 *ptr;

		CR = isl_memchr(oldptr, '\r', oldlen);
		LF = isl_memchr(oldptr, '\n', oldlen);
		ptr = (CR) ? CR : LF;
        if (ptr == NULL)							/* testing for newline */
		{
			cssm_memmove(DataPtr, oldptr, oldlen);	/* moving in-place */
			DataPtr += oldlen;						/* increment counter */
			break;									/* exit */
		}
		else
		{
			oldlen = (uint32)(ptr - oldptr);		/* calculating length */
			cssm_memmove(DataPtr, oldptr, oldlen);	/* moving in-place */
			DataPtr += oldlen;						/* incrementing length */
		}
		ptr = (LF) ? LF : CR;						/* finding newline */
		ptr++;										/* skipping newline */
        if (ptr < endptr && *ptr == ' ')			/* testing for continuation */
            ptr++;									/* skipping space */
        oldptr = ptr;
		oldlen = (uint32)(endptr - oldptr);
	}
	AttributeInfoPtr->Value.Length = (uint32)(DataPtr - AttributeInfoPtr->Value.Data);
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: ParseSectionAttributes
 *
 * Description:
 * Given a pointer a section image, this function will parse and return the
 * ptrs to all attributes in the section. It expects that the number of 
 * attributes has been pre-calculated with CountAttributes.
 *
 * Parameters: 
 * ArchivePtr (input)			: archive context for memory funcs
 * SectionInfoPtr (output)		: struct to be filled with attributes
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
ParseSectionAttributes(
	ISL_MEMORY_CONTEXT_PTR MemoryPtr,
	ISL_SECTION_INFO_PTR SectionInfoPtr)
{
	uint32 i = 0;
	ISL_CONST_DATA Input;
	ISL_CONST_DATA UpdatedInput;
	ISL_CONST_DATA Name;
	ISL_CONST_DATA Value;
	ISL_ATTRIBUTE_INFO_PTR AttributeInfoPtr;

	if (SectionInfoPtr == NULL) return ISL_FAIL;

	Input.Length = SectionInfoPtr->Image.Length;
	Input.Data = isl_CopyMemory(
        MemoryPtr, 
        SectionInfoPtr->Image.Data, 
        Input.Length);
	if (Input.Data == NULL)
		return ISL_FAIL;

	for(i = 0; i < SectionInfoPtr->Attributes.NumberOfAttributes; i++)
	{
		if (ParseAttribute(Input, &UpdatedInput, &Name, &Value) != ISL_OK) 
			return ISL_FAIL;

		AttributeInfoPtr = SectionInfoPtr->Attributes.Attributes + i;

		AttributeInfoPtr->Info.AttributeNameFormat = 
            CSSM_DB_ATTRIBUTE_NAME_AS_BLOB;

		AttributeInfoPtr->Info.Label.Name.Data = (uint8 *)Name.Data;
		AttributeInfoPtr->Info.Label.Name.Length = Name.Length;

		AttributeInfoPtr->Info.AttributeFormat = CSSM_DB_ATTRIBUTE_FORMAT_BLOB;
		AttributeInfoPtr->Value.Data = (uint8 *)Value.Data;
		AttributeInfoPtr->Value.Length = Value.Length;
		if (ISL_OK != StripAttributeValue(AttributeInfoPtr))
			return ISL_FAIL;
		Input = UpdatedInput;
	}
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: isl_JarFileDecode
 *
 * Description:
 * This function parse an input buffer into a SectionInfoGroup -- sections &
 * attributes for each section
 *
 * Parameters: 
 * ArchivePtr (input)				: archive context for memory funcs
 * Input (input)					: buffer to be parse
 * SectionInfoGroupPtr (output)		: struct to be filled from the parse
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
isl_JarFileDecode(
	ISL_MEMORY_CONTEXT_PTR MemoryPtr,
	ISL_CONST_DATA Input,
	ISL_SECTION_INFO_GROUP_PTR SectionInfoGroupPtr)
{
	uint32 TotalSize = 0;
	uint32 NumberOfSections = 0;
	uint32 NumberOfAttributes = 0;
	uint32 i;
	ISL_CONST_DATA UpdatedInput;
	ISL_CONST_DATA SectionImage;
	ISL_SECTION_INFO_PTR SectionInfoPtrs;
	ISL_ATTRIBUTE_INFO_PTR AttributeInfoPtrs;


	if (SectionInfoGroupPtr == NULL) return ISL_FAIL;

	NumberOfSections = CountSections(Input);
	if (NumberOfSections == 0) return ISL_FAIL;
	TotalSize = NumberOfSections * sizeof(ISL_SECTION_INFO);

	SectionInfoPtrs = isl_AllocateMemory(
						MemoryPtr,
						NumberOfSections * sizeof(ISL_SECTION_INFO));
	if (SectionInfoPtrs == NULL) return ISL_FAIL;

	SectionInfoGroupPtr->NumberOfSections = NumberOfSections;
	SectionInfoGroupPtr->Sections = SectionInfoPtrs;

	for(i = 0; i < NumberOfSections; i++)
	{
		if (ParseToEndOfSection(Input, &UpdatedInput, &SectionImage) != ISL_OK)
        {
            return ISL_FAIL;
        }

		SectionInfoPtrs[i].Image = SectionImage;
		Input = UpdatedInput;
	}

	for(i=0; i < NumberOfSections; i++)
	{
		NumberOfAttributes = CountAttributes(SectionInfoPtrs[i].Image);
		TotalSize += NumberOfAttributes*sizeof(ISL_ATTRIBUTE_INFO);

		AttributeInfoPtrs = isl_AllocateMemory(
		    MemoryPtr,
		    NumberOfAttributes * sizeof(ISL_ATTRIBUTE_INFO));
		if (AttributeInfoPtrs == NULL) return ISL_FAIL;

		SectionInfoPtrs[i].Attributes.NumberOfAttributes = NumberOfAttributes;
		SectionInfoPtrs[i].Attributes.Attributes = AttributeInfoPtrs;
	}

	for(i=0; i < NumberOfSections; i++)
	{
		if (ISL_OK != ParseSectionAttributes(MemoryPtr, SectionInfoPtrs +i))
        {
            return ISL_FAIL;
        }
	}

	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: ParseToEndOfSection
 *
 * Description:
 * This function will scan a buffer for a section
 *
 * Parameters: 
 * Input (input)				: buffer to be parsed 
 * UpdatedInput (output)	    : ptr to the end of section 
 * SectionImage (output)	    : ptr to the section
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
ParseToEndOfSection(
	ISL_CONST_DATA Input,				
	ISL_CONST_DATA_PTR UpdatedInput,	
	ISL_CONST_DATA_PTR SectionImage)	
{
	const uint8 *endptr;

	if (UpdatedInput == NULL) return ISL_FAIL;
	if (SectionImage == NULL) return ISL_FAIL;
	if (Input.Data == NULL) return ISL_FAIL;

	SectionImage->Data = Input.Data;			/* pointer to start of token */
	endptr = Input.Data + Input.Length;			/* pointer to end of buffer */
	do {
		if (ISL_OK != ParseToNewline(Input, UpdatedInput)) return ISL_FAIL;
		if (UpdatedInput->Length == 0) 
		{
			/* corner case when no trailing CR/LF */
			SectionImage->Length = (uint32)(UpdatedInput->Data - SectionImage->Data);
			return ISL_OK;
		}
		Input = *UpdatedInput;
	}
	while(*(UpdatedInput->Data) != '\r' && *(UpdatedInput->Data) != '\n');

	if ( (UpdatedInput->Length > 1) &&
		*(UpdatedInput->Data) == '\r' &&
		*(UpdatedInput->Data + 1) == '\n' )
	{
		UpdatedInput->Data++;
		UpdatedInput->Length --;
	}
	UpdatedInput->Data++;
	UpdatedInput->Length--;
	SectionImage->Length = (uint32)(UpdatedInput->Data - SectionImage->Data);
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: ParseToNewline
 *
 * Description:
 * This function will scan a buffer for a newline
 *
 * Parameters: 
 * Input (input)				: buffer to be parsed 
 * UpdatedInput (output)		: pointer to new line
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
ParseToNewline(
	ISL_CONST_DATA Input,				/* buffer to be parsed */
	ISL_CONST_DATA_PTR UpdatedInput)
{
    const uint8 *pCR;
    const uint8 *pLF;

	pCR = isl_memchr(Input.Data, '\r',Input.Length);
    pLF = isl_memchr(Input.Data, '\n',Input.Length);	

    UpdatedInput->Data = (pLF) ? pLF : pCR;		    /* (CR/LF or CR) or LF */
	if (UpdatedInput->Data == NULL)					/* test for newline */
		return ISL_FAIL;							/* test failed */
	UpdatedInput->Data++;							/* skip over newline */
	UpdatedInput->Length = Input.Length - (uint32)(UpdatedInput->Data - Input.Data);
	return ISL_OK;
}


