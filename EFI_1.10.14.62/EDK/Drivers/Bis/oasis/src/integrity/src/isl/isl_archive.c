/*-----------------------------------------------------------------------
 *      File: archive.c  
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
**	Implementation of ISL_ARCHIVE_CONTEXT class 
*/

#include "isl_internal.h"
#include "islutil.h"
#include "isl_parse.h"


extern ISL_STATUS isl_VerifyManSectWithAllDigestValues(ISL_SIG_SECTION_PTR pSigSect);

/* internal routines */
/*-----------------------------------------------------------------------------
 * Name: isl_CopyMemory
 *
 * Description:
 * Allocates memory and copies a buffer into that allocated memory
 *
 * Parameters:
 * Context (input)
 * Buffer (input)
 * Size (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
void *isl_CopyMemory(ISL_MEMORY_CONTEXT *Context, const void *Buffer, ISL_SIZE Size)
{
	struct isl_memory_buffer *item;
	ISL_MEMORY_METHODS *MemoryMethods;
	void * AllocRef;

	if (!Context) return NULL;
	MemoryMethods = Context->MemoryMethods;
	AllocRef = Context->AllocRef;

	if (Buffer == NULL) {
		return NULL;
	}

	item = MemoryMethods->malloc_func(sizeof(struct isl_memory_buffer), AllocRef);

	if (item == NULL) {
		ISL_SetError(CSSM_MEMORY_ERROR);		
		return 0;
	}
	item->Buffer = MemoryMethods->malloc_func((uint32)Size, AllocRef);
	if (item->Buffer == NULL) {
		MemoryMethods->free_func(item, AllocRef);
		ISL_SetError(CSSM_MEMORY_ERROR);
		return 0;
	}
	item->Next = Context->Buffers;
	Context->Buffers = item;
	cssm_memcpy(item->Buffer, Buffer, (uint32)Size);
	return item->Buffer;
}

/*-----------------------------------------------------------------------------
 * Name: isl_AllocateMemory
 *
 * Description:
 * This function will allocate memory from a memory context
 *
 * Parameters:
 * Context (input)
 * size (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
void *isl_AllocateMemory(ISL_MEMORY_CONTEXT *Context, ISL_SIZE size)
{
	ISL_MEMORY_METHODS *MemoryMethods;
	void *AllocRef;
	struct isl_memory_buffer *item;

	if (Context == NULL) return NULL;
	MemoryMethods = Context->MemoryMethods;
	AllocRef = Context->AllocRef;

	item = MemoryMethods->malloc_func(
        sizeof(struct isl_memory_buffer), 
        AllocRef);
	if (item == NULL)
	{
		ISL_SetError(CSSM_MEMORY_ERROR);
		return 0;
	}
	item->Buffer = MemoryMethods->malloc_func((uint32)size, AllocRef);
	if (item->Buffer == NULL) {
		MemoryMethods->free_func(item, AllocRef);
		ISL_SetError(CSSM_MEMORY_ERROR);
		return 0;
	}
	item->Next = Context->Buffers;
	Context->Buffers = item;
	cssm_memset(item->Buffer, 0, (uint32)size);
	return item->Buffer;
}


/*-----------------------------------------------------------------------------
 * Name: isl_FreeMemory
 *
 * Description:
 * Frees memory associated with a memory context
 *
 * Parameters:
 * Context (input)
 * Buffer (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
void isl_FreeMemory(ISL_MEMORY_CONTEXT *Context, const void *Buffer)
{
	ISL_MEMORY_METHODS *MemoryMethods;
	void * AllocRef;
	ISL_MEMORY_BUFFER_PTR	Curr = NULL;
	ISL_MEMORY_BUFFER_PTR	Prev = NULL;

	if (Context == NULL ||
		Buffer == NULL) return;

	MemoryMethods = Context->MemoryMethods;
	AllocRef = Context->AllocRef;

	for(Curr = Context->Buffers;										/* looping through all buffers */
		Curr;															/* exit condition */
		Curr = Curr->Next)												/* getting next buffer */
	{
		if (Buffer == Curr->Buffer)										/* match pointers */
		{
			if (!Prev)													/* testing for head of list */
				Context->Buffers = Curr->Next;							/* head of list is next buffer */
			else
				Prev->Next = Curr->Next;								/* pointer previous bufffer to next buffer */
			
			MemoryMethods->free_func((void*) Buffer, AllocRef);	        /* free buffer */
			MemoryMethods->free_func((void*) Curr, AllocRef);		    /* free book keeping */
				 
			return;
		}
		Prev = Curr;													/* save current pointer */
	}
	return;
}

/*-----------------------------------------------------------------------------
 * Name: isl_RecycleMemoryContext
 *
 * Description:
 * This function will free ALL memory associated with the memory context 
 *
 * Parameters:
 * Context (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
isl_RecycleMemoryContext(ISL_MEMORY_CONTEXT *Context)
{
	ISL_MEMORY_BUFFER_PTR  item;
	ISL_MEMORY_BUFFER_PTR  temp;

    if (Context == NULL ||
        Context->MemoryMethods == NULL)
        return ISL_FAIL;
	item = Context->Buffers;
	while(item)
	{
		Context->MemoryMethods->free_func(
            item->Buffer, 
            Context->AllocRef);
		temp = item;						/* temp pointer to node */
		item = item->Next;					/* get next node pointer */
		Context->MemoryMethods->free_func(
            temp, 
            Context->AllocRef);
	}

    return ISL_OK;
}
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
static	ISL_SIZE ArchiveSizeofObject()			/* returns sizeof object */
{	
	return sizeof(ISL_ARCHIVE_CONTEXT);
}

/* object methods */

#pragma warning (disable : 4102)

/*-----------------------------------------------------------------------------
 * Class: Archive
 * Name:  AddOldSignature
 *
 * Description:
 * This will attempt to instantiate a signature object from its external
 * representation and add the signature object to the archives signature object
 * list
 *
 * Parameters: 
 * Context (input/output)   : this pointer
 * Name (input)				: archive specific name of signature object
 * SignatureMethods (input) : signature specific methods
 * InMemoryImage (input)    : memory image of external representation
 *
 * Return value:
 * status of operation
 * 
 * Error Codes:
 *
 * Notes:
 * InMemoryImage is assumed to be persistent for the life of the signature object
 *---------------------------------------------------------------------------*/
static
ISL_STATUS
AddOldSignature(
	ISL_ARCHIVE_CONTEXT_PTR Context,
	ISL_CONST_DATA Name,
	ISL_SIGNATURE_METHODS *SignatureMethods,
	ISL_CONST_DATA InMemoryImage)
{
	ISL_CONFIG  *SigConfig;
	ISL_SIGNATURE_CONTEXT *SigContext;
	ISL_LIST_PTR SignatureList;
    ISL_SIGNATURE_INFO_PTR SignatureInfoPtr;

	if (SignatureMethods == NULL ||
		SignatureMethods->ServiceMethods.Class == NULL) {
		return ISL_FAIL;
	}

 	SigConfig = (ISL_CONFIG_PTR)SignatureMethods->ServiceMethods.Class->ClassContext;
    
	if (SigConfig == NULL) {
		return ISL_FAIL;
	}
	SigContext= isl_AllocateMemory(
					Context->Memory, 
                    SignatureMethods->SizeofObject());

    if (SigContext == NULL) {
		return ISL_FAIL;
	}
	if (ISL_OK != SignatureMethods->InitializeFromImage(
				SigContext,
				SigConfig,
				Context->Memory,
				Name,
				InMemoryImage))
	{
		return ISL_FAIL;
	}
    /* parse SignedObjectList */


    SignatureList = isl_AllocateMemory(Context->Memory, sizeof(ISL_LIST));
    if (SignatureList == NULL) {
		return ISL_FAIL;
	}

    SignatureInfoPtr = isl_AllocateMemory(
        Context->Memory, 
        sizeof(ISL_SIGNATURE_INFO));
    if (SignatureInfoPtr == NULL) {
        return ISL_FAIL;
    }

    SignatureList->Node = SignatureInfoPtr;

//    SignatureInfoPtr->Parent = Context;
    SignatureInfoPtr->SignaturePtr = SigContext;
    SignatureInfoPtr->SignedListPtr = isl_AllocateMemory(
        Context->Memory, 
        SignedListMethods.SizeofObject());
    if (SignatureInfoPtr->SignedListPtr == NULL) 
	{
		return ISL_FAIL;
	}
    if (ISL_OK != isl_InitializeSignedListFromImage(
                    SignatureInfoPtr->SignedListPtr,
                    Context,
                    Name,
                    SigContext->SignedImage))
    {
		return ISL_FAIL;
	}
    SignatureList->Next = Context->Signatures;

    SignatureInfoPtr->Name.Length = Name.Length;
	SignatureInfoPtr->Name.Data = isl_CopyMemory(
        Context->Memory, 
        Name.Data, 
        Name.Length);
	if (SignatureInfoPtr->Name.Data == NULL){ 
		return ISL_FAIL;
	}
    Context->Signatures = SignatureList;

	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Class: Archive
 * Name:  AddOldCertificate
 *
 * Description:
 * Constructor for existing in-memory archive
 *
 * Parameters: 
 * Context (input/output)   : this pointer
 * configContext (input)   : algorithm<->code extension
 * InMemoryImage (input)   : memory image of external representation
 *
 * Return value:
 * status of operation
 * 
 * Error Codes:
 *
 * Notes:
 * InMemoryImage is assumed to be persistent for the life of the archive object
 *---------------------------------------------------------------------------*/
static
ISL_STATUS
AddOldCertificate(
	ISL_ARCHIVE_CONTEXT_PTR Context,
	ISL_CONST_DATA Name,
	ISL_CERTIFICATE_METHODS *CertMethods,
	ISL_CONST_DATA InMemoryImage)
{
	ISL_CERTIFICATE_PTR CertPtr;
	ISL_LIST_PTR	CertListPtr;

    Name;

    CertPtr = isl_AllocateMemory(Context->Memory, sizeof(ISL_CERTIFICATE));
    if (!CertPtr) {
		return ISL_FAIL;
	}
    if (ISL_OK != CertMethods->Initialize(
                CertPtr,
                Context->Memory,
                InMemoryImage) )
	{
		return ISL_FAIL;
	}

    CertListPtr = isl_AllocateMemory(Context->Memory, sizeof(ISL_LIST));
    if (!CertListPtr) {
		return ISL_FAIL;
	}
    CertListPtr->Node = CertPtr;
    CertListPtr->Next = Context->Certs;
#if 0
	pCertList->Name.Data = isl_AllocateMemory(Context->Memory, Name.Length);
	if (!pCertList->Name.Data){ 
		return ISL_FAIL;
	}
	cssm_memcpy((void*)pCertList->Name.Data, (void*)Name.Data, Name.Length);
	pCertList->Name.Length = Name.Length;
#endif
    Context->Certs = CertListPtr;

    return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Class: Archive
 * Name:  InitializeFromMemory
 *
 * Description:
 * Constructor for existing in-memory archive
 *
 * Parameters: 
 * Context (input/output)   : this pointer
 * configContext (input)   : algorithm<->code extension
 * InMemoryImage (input)   : memory image of external representation
 *
 * Return value:
 * status of operation
 * 
 * Error Codes:
 *
 * Notes:
 * InMemoryImage is assumed to be persistent for the life of the archive object
 *---------------------------------------------------------------------------*/
static	
ISL_STATUS 
InitializeFromMemory(		                        
	ISL_ARCHIVE_CONTEXT *Context,
	ISL_CONFIG_PTR Config,
	ISL_CONST_DATA InMemoryImage)
{
	PK_ARCHIVE_PTR pPKArchive;
	PK_ITERATOR_PTR pkIterator;
	ISL_CONST_DATA Filename;
	ISL_CONST_DATA Filedata;
    ISL_CONST_DATA FilePath;
    ISL_MEMORY_CONTEXT_PTR MemoryPtr;
	
	if (Context == NULL || Config == NULL)
		return ISL_FAIL;
			
	cssm_memset(Context, 0, sizeof(ISL_ARCHIVE_CONTEXT));
	Context->Config =  Config;
//	Context->Methods = &ArchiveMethods;

    /* Allocate the Archive's ISL_MEMORY_CONTEXT */
    MemoryPtr = Config->MemoryMethods->calloc_func(
        sizeof(ISL_MEMORY_CONTEXT),
        1,
        Config->AllocRef);
    if (MemoryPtr == NULL) return ISL_FAIL;
    MemoryPtr->MemoryMethods = Config->MemoryMethods;
    MemoryPtr->AllocRef = Config->AllocRef;
    Context->Memory = MemoryPtr;
    /* Parse the Archive Components */
	pPKArchive = pk_InitializeFromMemory(Context->Memory, InMemoryImage);
	if (pPKArchive == NULL) {
		return ISL_FAIL;
	}

	Filename.Data = (const uint8 *)"META-INF/MANIFEST.MF";
	Filename.Length = 20;
	if (ISL_OK != pk_FindFile(pPKArchive, Filename, &Filedata))
	{
		return ISL_FAIL;
	}
	/* parse the manifest file */
	Context->Manifest.Image.Data = Filedata.Data;
	Context->Manifest.Image.Length = Filedata.Length;
	if (ISL_OK != isl_BuildManifest(Context))
	{
		return ISL_FAIL;
	}

	pkIterator = pk_CreateFileEnumerator(pPKArchive);
	if (pkIterator == NULL) {
		return ISL_FAIL;
	}

	while (ISL_OK == pk_GetNextFile(pkIterator, &FilePath, &Filedata))
	{
        ISL_CONST_DATA          FileName;
        ISL_CONST_DATA          FileTitle;
        ISL_CONST_DATA          FileExt;
		ISL_SERVICE_CLASS_METHODS*	ServiceMethods;
		ISL_SERVICE_CLASS			ServiceClass;
		ISL_CONST_DATA				ServiceName;
		ISL_CONST_DATA				AlgId;

		if (ISL_OK != isl_ParseFilePath(FilePath, NULL, &FileName, &FileTitle, &FileExt))
		{
			goto FAIL;
		}
		
		ServiceMethods = ArchiveConfigMethods.FindAlgorithm(Config, FileExt);
		if (ServiceMethods == NULL) {
			continue;
		}
		ServiceMethods->id(&ServiceClass, &AlgId, &ServiceName);
		switch (ServiceClass) {
			case ISL_ServiceParseSignature:
				if (ISL_OK != AddOldSignature(
								Context, 
								FileTitle, 
								(ISL_SIGNATURE_METHODS *)ServiceMethods, 
								Filedata))
				{
					return ISL_FAIL;
				}
				break;
			case ISL_ServiceParseCertificate:
				if (ISL_OK != AddOldCertificate(
								Context, 
								FileTitle, 
								(ISL_CERTIFICATE_METHODS *)ServiceMethods, 
								Filedata))
				{
					return ISL_FAIL;
				}
				break;
			default:
				continue;
		}
	}	

	(void)pk_RecycleFileEnumerator(pkIterator);
	return ISL_OK;

FAIL:
	(void)pk_RecycleFileEnumerator(pkIterator);
	return ISL_FAIL;
}

/*-----------------------------------------------------------------------------
 * Name: isl_RecycleArchiveContext
 *
 * Description:
 * This function will free all the resources allocate by itself and all
 * subordinate objects
 *
 * Parameters:
 * Context (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_STATUS
isl_RecycleArchiveContext(
	ISL_ARCHIVE_CONTEXT *Context)
{
	if (Context == NULL ||
        Context->Memory == NULL) 
    {
        return ISL_FAIL;
    }
	/* Recycle Signature Certificate Objects */

    isl_RecycleMemoryContext(Context->Memory);
    Context->Memory->MemoryMethods->free_func(Context->Memory, Context->Memory->AllocRef);
	cssm_memset(Context, 0, sizeof(ISL_ARCHIVE_CONTEXT));	/* zero out memory */
	return ISL_OK;
}
	
/*-----------------------------------------------------------------------------
 * Name: FindSignableObject
 *
 * Description:
 * Finds a signable object in the archive
 *
 * Parameters: 
 * Context - the archive context
 * Name - name of signable object to find
 *
 * Return value:
 * ISL_MANIFEST_SECTION_PTR - if object found
 * NULL - if object not found
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static	ISL_MANIFEST_SECTION_PTR FindSignableObject(
		ISL_ARCHIVE_CONTEXT *Context,		/* context */
		ISL_CONST_DATA Name)				/* manifest section name */
{
    ISL_LIST_PTR CurrNodePtr;


	if (Context == NULL ||
		Name.Data == NULL) 
	{
		return NULL;
	}

	/* search through manifest list for the specified section. */
	/* If found, delete it from the list */
	for(CurrNodePtr = Context->Manifest.Section;
		CurrNodePtr;
		CurrNodePtr = CurrNodePtr->Next) 
	{
        ISL_MANIFEST_SECTION_PTR ManSectPtr;

        ManSectPtr = CurrNodePtr->Node;
		if (IS_EQUAL(ManSectPtr->SectionInfo.Name, Name))  {
			return ManSectPtr;
		}
		
	}
	return NULL;
}


/*-----------------------------------------------------------------------------
 * Name: isl_BuildManifest
 *
 * Description:
 *
 * Parameters:
 * ArchivePtr (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
isl_BuildManifest(
	ISL_ARCHIVE_CONTEXT_PTR ArchivePtr)
{
	ISL_SECTION_INFO_GROUP SectionInfoGroup;
	
	if (ArchivePtr == NULL) return ISL_FAIL;

	if (isl_JarFileDecode(
			ArchivePtr->Memory,
			ArchivePtr->Manifest.Image, 
			&SectionInfoGroup) != ISL_OK)
	{
		return ISL_FAIL;
	}

	/* The first SectionInfo in the SectionInfoGroup corresponds to the 
       Manifest header and not any one manifest section */
	if (SectionInfoGroup.NumberOfSections == 0) return ISL_FAIL;

	/* Initialize Manifest Header  */
    {
        ISL_HEADER_SECTION_PTR HeaderPtr;

	    HeaderPtr = &ArchivePtr->Manifest.Header;
	    if (ISL_OK != isl_BuildHeaderSection(
						    HeaderPtr,
						    ArchivePtr->Memory,
						    SectionInfoGroup.Sections))
	        return ISL_FAIL;
    }

    /* then process the Manifest Sections */
    {
        ISL_LIST_PTR ManifestSectionListPtr;
	    ISL_MANIFEST_SECTION_PTR ManifestSectionPtr;
	    uint32 i;

	    ArchivePtr->Manifest.SectionCount = 
            SectionInfoGroup.NumberOfSections - 
            1;

	    ArchivePtr->Manifest.Section = isl_AllocateMemory(
            ArchivePtr->Memory, 
            ArchivePtr->Manifest.SectionCount*sizeof(ISL_LIST));
	    if (ArchivePtr->Manifest.Section == NULL) return ISL_FAIL;


        ManifestSectionPtr = isl_AllocateMemory(
            ArchivePtr->Memory,
            ArchivePtr->Manifest.SectionCount*sizeof(ISL_MANIFEST_SECTION));
        if (ManifestSectionPtr == NULL) return ISL_FAIL;

	    ManifestSectionListPtr = ArchivePtr->Manifest.Section;
	    for(i=1; i < SectionInfoGroup.NumberOfSections; i++)
	    {	
		    ManifestSectionListPtr->Next = ManifestSectionListPtr + 1;
            ManifestSectionListPtr->Node = ManifestSectionPtr;

		    ManifestSectionPtr->Parent = ArchivePtr;
		    if (isl_BuildManifestSection(
				    ManifestSectionPtr, 
				    SectionInfoGroup.Sections + i) != ISL_OK)
			    return ISL_FAIL;
            ManifestSectionPtr++;
            ManifestSectionListPtr++;
	    }
	    (--ManifestSectionListPtr)->Next = NULL;
    }
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: Verify
 *
 * Description:
 * Verifies a signature in the context of the archive i.e. the signature's
 * signed content is a signed object list. After verifying the integrity of the
 * signed object list, the integrity of the referenced manifest sections is
 * checked.
 *
 * Parameters:
 * ArchivePtr (input)
 * SignaturePtr (input)
 * CertMethods (input)
 * CertificatePtr (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_STATUS
Verify(
    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr,
	ISL_SIGNATURE_CONTEXT_PTR SignaturePtr,	    /* archive context */
	ISL_CERTIFICATE_METHODS *CertMethods,		/* configuration methods for certificate format */
	ISL_CERTIFICATE_PTR CertificatePtr)			/* the certificate */
{
    ISL_SIGNER_CONTEXT_PTR SignerPtr;
    ISL_LIST_PTR Signatures;
    ISL_SIGNATURE_INFO_PTR SigInfoPtr = NULL;
    ISL_SIG_SECTION_GROUP_PTR SigObjGrpPtr = NULL;

    for(Signatures = ArchivePtr->Signatures;
        Signatures != NULL;
        Signatures = Signatures->Next)
    {

        SigInfoPtr = Signatures->Node;
        if (SigInfoPtr->SignaturePtr == SignaturePtr)
            break;
    }
    if (SigInfoPtr == NULL) return ISL_FAIL;

	SignerPtr = PKCS7SignatureMethods.FindSigner(SignaturePtr, CertificatePtr);
	if (SignerPtr == NULL)
	{
        return ISL_FAIL;
	}
	if (ISL_OK != PKCS7SignerMethods.VerifyUsingCert(
					SignerPtr, 
					CertMethods, 
					CertificatePtr) )
	{
        return ISL_FAIL;
	}

    /* verify the integrity of the manifest sections */
	
	/* 
	 * for each signature section in the signer's information list, locate
	 * the corresponding manifest section, compute the digest of that manfiest
	 * section using the digest algorithm indicated in the signature information
	 * file.  Compare the computed digest against the value listed in the 
	 * signature file
	 */
    {
        uint32 Index;

        SigObjGrpPtr = SignedListMethods.GetSignedObjectInfos(
            SigInfoPtr->SignedListPtr);
        if (SigObjGrpPtr == NULL) return ISL_FAIL;

        for(Index =0;
            Index < SigObjGrpPtr->NumberOfSignedObjects;
            Index++)
	    {
            ISL_SIG_SECTION_PTR SignedObjectPtr;
            ISL_STATUS SignedObjectVerified;

            SignedObjectPtr = SigObjGrpPtr->SignedObjects[Index];
            SignedObjectVerified = isl_VerifyManSectWithAllDigestValues(
                SignedObjectPtr);
		    if (SignedObjectVerified != ISL_OK) goto FAIL;
	    }
        SignedListMethods.FreeSignedObjectInfos(
            SigInfoPtr->SignedListPtr,
            SigObjGrpPtr);
    }
    return ISL_OK;
FAIL:
    {
        SignedListMethods.FreeSignedObjectInfos(
            SigInfoPtr->SignedListPtr,
            SigObjGrpPtr);

        return ISL_FAIL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: SetLocationMap
 *
 * Description: 
 * This function will update the Location Maps based on the information in the
 * MapEntryPtr array.
 *
 * Parameters:
 * MapEntryPtr (input) - 
 * NumberOfEntries (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
SetLocationMap(
    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr,
	ISL_LMAP Map)
{
    ISL_LMAP TempMap;
	uint32 i;

	if (ArchivePtr == NULL) 
	{
		return ISL_FAIL;
	}

    if (Map.NumberOfEntries == 0)
    {
        ArchivePtr->Map.NumberOfEntries = 0;
        ArchivePtr->Map.MapEntries = NULL;
        return ISL_OK;
    }

    TempMap.MapEntries = NULL;
    {
        /* code is a bit leaky as it allocates from the ArchivePtr
           to be more aggressive should give it its own MemoryPtr */
        TempMap.NumberOfEntries = Map.NumberOfEntries;
        TempMap.MapEntries = isl_AllocateMemory(
            ArchivePtr->Memory,
            sizeof(ISL_LMAP_ENTRY) * TempMap.NumberOfEntries);
        if (TempMap.MapEntries == NULL) goto FAIL;

        for(i=0; i < Map.NumberOfEntries; i++)
	    {
            ISL_LMAP_ENTRY_PTR ArchiveEntryPtr = 
                &TempMap.MapEntries[i];

            ArchiveEntryPtr->GetDataMethod = 
                Map.MapEntries[i].GetDataMethod;

            ArchiveEntryPtr->GetParameters.Length =
                Map.MapEntries[i].GetParameters.Length;
            if (ArchiveEntryPtr->GetParameters.Length != 0)
            {
                ArchiveEntryPtr->GetParameters.Data = isl_CopyMemory(
                    ArchivePtr->Memory,
                    Map.MapEntries[i].GetParameters.Data,
                    Map.MapEntries[i].GetParameters.Length);
                if (ArchiveEntryPtr->GetParameters.Data == NULL) goto FAIL;
            }
            else
            {
                ArchiveEntryPtr->GetParameters.Data = NULL;
            }

            ArchiveEntryPtr->JoinName.Length =
                Map.MapEntries[i].JoinName.Length;
            if (ArchiveEntryPtr->JoinName.Length != 0)
            {
                ArchiveEntryPtr->JoinName.Data = isl_CopyMemory(
                    ArchivePtr->Memory,
                    Map.MapEntries[i].JoinName.Data,
                    Map.MapEntries[i].JoinName.Length);
                if (ArchiveEntryPtr->JoinName.Data == NULL) goto FAIL;
            }
            else
            {
                ArchiveEntryPtr->JoinName.Data = NULL;
            }
        }

        ArchivePtr->Map = TempMap;
	    return ISL_OK;
    }
FAIL:
    {
        return ISL_FAIL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: GetLocationMap
 *
 * Description:
 * This function returns an SM Specific representation of a location map
 *
 * Parameters:
 * ArchivePtr (input)
 * MapPtr (output)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_LMAP_PTR
GetLocationMap(
	ISL_ARCHIVE_CONTEXT_PTR ArchivePtr)
{
    /* not going to replicate the map (for now) */
	if (ArchivePtr == NULL) return NULL;
    return &ArchivePtr->Map;
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
FindAttributeValue(		
	ISL_ARCHIVE_CONTEXT_PTR Context,
	ISL_CONST_DATA Attribute,
	ISL_CONST_DATA_PTR Value)
{
    ISL_STATUS FoundAttributeStatus;

    if (Context == NULL) return ISL_FAIL;

    FoundAttributeStatus = isl_FindAttributeInHeader(
        &Context->Manifest.Header,
        Attribute,
        Value);

    return FoundAttributeStatus;
}

/*-----------------------------------------------------------------------------
 * Name: GetSignatureGroup
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
ISL_SIGNATURE_INFO_GROUP_PTR 
GetSignatureGroup(
    ISL_ARCHIVE_CONTEXT_PTR Context)
{
    ISL_SIGNATURE_INFO_GROUP_PTR SignatureGrpPtr;
    ISL_LIST_PTR CurrNodePtr;
    ISL_SIGNATURE_INFO_PTR SigInfoPtr;
    uint32 NumberOfSignatures;

    if (Context == NULL) return NULL;

    SignatureGrpPtr = isl_AllocateMemory(
        Context->Memory,
        sizeof(ISL_SIGNATURE_INFO_GROUP));
    if (SignatureGrpPtr == NULL) return NULL;

    NumberOfSignatures = isl_CountItemsInList(Context->Signatures);
    if (NumberOfSignatures == 0)
    {
        SignatureGrpPtr->NumberOfSignatures = 0;
        SignatureGrpPtr->SignatureInfoPtr = NULL;
        return SignatureGrpPtr;
    }

    SignatureGrpPtr->NumberOfSignatures = NumberOfSignatures;
    SigInfoPtr = isl_AllocateMemory(
        Context->Memory,
        sizeof(ISL_SIGNATURE_INFO) * NumberOfSignatures);
    if (SigInfoPtr == NULL)
    {
        goto FAIL;
    }

    SignatureGrpPtr->SignatureInfoPtr = SigInfoPtr;    
    for(CurrNodePtr = Context->Signatures;
        CurrNodePtr != NULL;
        CurrNodePtr = CurrNodePtr->Next)
    {
        ISL_SIGNATURE_INFO_PTR ListSigInfoPtr;

        ListSigInfoPtr = CurrNodePtr->Node;
        *SigInfoPtr = *ListSigInfoPtr;
        SigInfoPtr++;
    }
    return SignatureGrpPtr;

FAIL:
    {
        isl_FreeMemory(Context->Memory, SignatureGrpPtr);
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: FreeSignatureGroup
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
FreeSignatureGroup(
    ISL_ARCHIVE_CONTEXT_PTR Context,
    ISL_SIGNATURE_INFO_GROUP_PTR SignatureInfoGroupPtr)
{
    if (Context == NULL) return ISL_FAIL;

    if (SignatureInfoGroupPtr == NULL) return ISL_OK;
    isl_FreeMemory(Context->Memory, SignatureInfoGroupPtr->SignatureInfoPtr);
    isl_FreeMemory(Context->Memory, SignatureInfoGroupPtr);
    return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: GetCertificateGroup
 *
 * Description:
 * This function returns a group of certificate objects contained in the 
 * archive.
 *
 * Parameters:
 * Context (input)
 *
 * Return value:
 * 
 * Error Codes:
 *
 * Notes:
 * Use FreeCertificateGroup to release memory
 *---------------------------------------------------------------------------*/
static
ISL_CERTIFICATE_GROUP_PTR 
GetCertificateGroup(
    ISL_ARCHIVE_CONTEXT_PTR Context)
{
    if (Context == NULL) return NULL;

    return isl_BuildCertificateGroup(
        Context->Certs,
        Context->Memory);
}

/*-----------------------------------------------------------------------------
 * Name: FreeCertificateGroup
 *
 * Description:
 * This function returns an SM Specific representation of a location map
 *
 * Parameters:
 * Context (input)
 * CertificateGroupPtr (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_STATUS
FreeCertificateGroup(
    ISL_ARCHIVE_CONTEXT_PTR Context,
    ISL_CERTIFICATE_GROUP_PTR CertificateGroupPtr)
{
    if (Context == NULL) return ISL_FAIL;
    if (CertificateGroupPtr == NULL) return ISL_OK;

    isl_FreeMemory(Context->Memory, CertificateGroupPtr->Certificates);
    isl_FreeMemory(Context->Memory, CertificateGroupPtr);
    return ISL_OK;
}

ISL_ARCHIVE_METHODS ArchiveMethods = {
	{NULL, NULL},
	ArchiveSizeofObject,
	InitializeFromMemory,
	isl_RecycleArchiveContext,
	FindSignableObject,
    Verify,
    GetLocationMap,
    SetLocationMap,
    FindAttributeValue,		
    GetSignatureGroup,
    FreeSignatureGroup,
    GetCertificateGroup,
    FreeCertificateGroup
};
