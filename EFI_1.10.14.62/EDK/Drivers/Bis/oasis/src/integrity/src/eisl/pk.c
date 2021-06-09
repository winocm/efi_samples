/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:
  
  pk.c

Abstract:


Revision History

--*/
/* This file implements parsing and reading of uncompressed
   PKWARE archive format files */

#include <efi.h>
#include <efidriverlib.h>
//#include <stdio.h>
//#include <sys/stat.h>
//#include <time.h>
#include "isltype.h"
#include "isl_internal.h"
//#include "pkapi.h"

//pause(char*);

static void
CRC32_Init(
	uint32					table[256]);

static uint32
CRC32(
	uint32					table[256],
	ISL_CONST_DATA_PTR		pData);

static ISL_STATUS
ParseLocalFile(
	PK_LOCAL_FILE_PTR pLocal,
	ISL_CONST_DATA Input,
    ISL_CONST_DATA_PTR UpdatedInput);

static ISL_STATUS
ParseCentralFile(
	PK_CENTRAL_FILE_PTR CentralPtr,
	ISL_CONST_DATA Input,
    ISL_CONST_DATA_PTR UpdatedInput);

static ISL_STATUS
ParseCentralDirEnd(
	ISL_MEMORY_CONTEXT_PTR	pMem,
	ISL_CONST_DATA Input,
    ISL_CONST_DATA_PTR UpdatedInput,
	PK_ARCHIVE_PTR	pArchive);

/*-----------------------------------------------------------------------------
 * Name: pk_InitializeFromMemory
 *
 * Description:
 *
 * Parameters:
 * MemPtr (input)
 * Fileimage (input)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
PK_ARCHIVE_PTR
pk_InitializeFromMemory(
	ISL_MEMORY_CONTEXT_PTR MemPtr,
	ISL_CONST_DATA	Fileimage )
{
	PK_ARCHIVE_PTR pkArchivePtr = NULL;
    ISL_CONST_DATA UpdatedInput;
    ISL_CONST_DATA Input;
    uint32 localFileSig=   LOCAL_FILE_HEADER_SIGNATURE;
	uint32 centralFileSig= CENTRAL_FILE_HEADER_SIGNATURE;
	uint32 endCenDirSig=   END_OF_CENTRAL_DIR_SIGNATURE;

	uint8* pos = NULL;
    UpdatedInput.Length = 0;

   

	pkArchivePtr = isl_AllocateMemory(MemPtr, sizeof(PK_ARCHIVE));
	if (pkArchivePtr == NULL) {
		return NULL;
	}
	pkArchivePtr->pMem = MemPtr;
	pkArchivePtr->Archive = Fileimage;

	/* Start parsing */
    Input = Fileimage;
	pos = (uint8*)Fileimage.Data;


	/* We look for all of the local files first */
	while ( Input.Length > 4 
    &&    ( 0 == EfiCompareMem( &localFileSig,  (void*)Input.Data, sizeof(uint32) ) )
    	  )
		   
	{
	 	PK_LOCAL_FILE_PTR LocalPtr;

		LocalPtr = isl_AllocateMemory(MemPtr, sizeof(PK_LOCAL_FILE));
		if (LocalPtr == NULL ||
            ISL_OK != ParseLocalFile(LocalPtr, Input, &UpdatedInput))
        {
			goto RECYCLE_AND_FAIL;
		}		

		LocalPtr->Next = pkArchivePtr->LocalFiles;
		pkArchivePtr->LocalFiles = LocalPtr;
        Input = UpdatedInput;
 		
	}


	/* Now we look for all of the central files */
	while (Input.Length > 4 
    &&    ( 0 == EfiCompareMem( &centralFileSig,  (void*)Input.Data, sizeof(uint32) ) )
		  )
	{
		PK_CENTRAL_FILE_PTR CentralPtr;

		CentralPtr = isl_AllocateMemory(MemPtr, sizeof(PK_CENTRAL_FILE));
		if (CentralPtr == NULL ||
		    ISL_OK != ParseCentralFile(CentralPtr, Input, &UpdatedInput)) {
			goto RECYCLE_AND_FAIL;
        }
		CentralPtr->Next = pkArchivePtr->CentralFiles;
		pkArchivePtr->CentralFiles = CentralPtr;
        Input = UpdatedInput;
	}

	/* Finally, parse the end of the Central Directory */
	while (Input.Length > 4 
    &&    ( 0 == EfiCompareMem( &endCenDirSig,  (void*)Input.Data, sizeof(uint32) ) )
		  )
	{
		if (ISL_OK != ParseCentralDirEnd(
                        MemPtr, 
                        Input, 
                        &UpdatedInput, 
                        pkArchivePtr))
			goto RECYCLE_AND_FAIL;
        Input = UpdatedInput;
	}

	/* Check to see if file was fully parsed */
	if (UpdatedInput.Length != 0) goto RECYCLE_AND_FAIL;

	return pkArchivePtr;

RECYCLE_AND_FAIL:

	(void)pk_RecycleArchive(pkArchivePtr);
	return NULL;
}

/*-----------------------------------------------------------------------------
 * Name: CRC32_Init
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
CRC32_Init(
	uint32					table[256])
{
	uint32	i, j, pos, num;

	/* First, we need to initialize the CRC table, using the seed specified
	   by the PKWARE specification */
	num = 0xEDB88320L;

    for (i = 0; i < 256; i++)
	{
		pos = i;
        for (j = 8; j > 0; j--)
        {
           if (pos & 1)
                pos = (pos >> 1) ^ num;
            else
                pos >>= 1;
        }
        table[i] = pos;
    }
}

/*-----------------------------------------------------------------------------
 * Name: CRC32
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static uint32
CRC32(
	uint32					table[256],
	ISL_CONST_DATA_PTR		pData)
{
	uint32		newCRC, i;
	const uint8	*data;

	if (!pData || !pData->Length)
		return 0;

	/* Now, we can get to the business of calculating the CRC */
    newCRC = 0xFFFFFFFF;
	data = pData->Data;
	for (i=0; i<pData->Length; i++)
	{
		newCRC = ((newCRC>>8) & 0x00FFFFFF) ^ table[ (newCRC^data[i]) & 0xFF ];
	}

    return( newCRC^0xFFFFFFFF );
}


/*-----------------------------------------------------------------------------
 * Name: pk_RecycleArchive
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
pk_RecycleArchive(
	PK_ARCHIVE_PTR	pArchive )
{
	PK_LOCAL_FILE_PTR			pLocal = NULL;
	PK_CENTRAL_FILE_PTR			pCentral = NULL;

	if (!pArchive)
		return ISL_OK;

	/* Free local file structures */
	while (pArchive->LocalFiles)
	{
		pLocal = pArchive->LocalFiles;
		pArchive->LocalFiles = pArchive->LocalFiles->Next;
		isl_FreeMemory(pArchive->pMem, pLocal);
	}

	/* Free central file structures */
	while (pArchive->CentralFiles)
	{
		pCentral = pArchive->CentralFiles;
		pArchive->CentralFiles = pArchive->CentralFiles->Next;
		isl_FreeMemory(pArchive->pMem, pCentral);
	}

	/* Free central directory end */
	if (pArchive->DirEnd)
	{
		isl_FreeMemory(pArchive->pMem, pArchive->DirEnd);
	}

	/* Free the archive itself */
	isl_FreeMemory(pArchive->pMem, pArchive);

	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: pk_FindFile
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
pk_FindFile(
	PK_ARCHIVE_PTR		pArchive,
	ISL_CONST_DATA		Filename,
	ISL_CONST_DATA_PTR	Filedata )
{
	PK_LOCAL_FILE_PTR	pLocal = NULL;

	if (pArchive == NULL ||
        Filename.Length == 0 || 
        Filedata == NULL )
    {
		return ISL_FAIL;
    }
	pLocal = pArchive->LocalFiles;

	while (pLocal)
	{
		if (pLocal->Filename.Length == Filename.Length)
		{
			if (0 == cssm_memcmp(
                        pLocal->Filename.Data, 
                        Filename.Data, 
                        Filename.Length))
			{
			    *Filedata = pLocal->Filedata;
				return ISL_OK;
			}
		}
		pLocal = pLocal->Next;
	}

	return ISL_FAIL;
}

/*-----------------------------------------------------------------------------
 * Name: pk_CreateFileEnumerator
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
PK_ITERATOR_PTR
pk_CreateFileEnumerator(
	PK_ARCHIVE_PTR PKArchivePtr)
{
	PK_ITERATOR_PTR	toReturn;

	if (PKArchivePtr == NULL)
		return NULL;

	/* RLM: 4/14/98: why are we calling cssm_malloc() instead of the local
	 * memory allocation routines.
	 */
	toReturn = isl_AllocateMemory(PKArchivePtr->pMem, sizeof(PK_ITERATOR));
	if (toReturn == NULL) return NULL;

	toReturn->LocalFilePtr = PKArchivePtr->LocalFiles;
	toReturn->PKArchivePtr = PKArchivePtr;
	return toReturn;
}

/*-----------------------------------------------------------------------------
 * Name: pk_GetNextFile
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
pk_GetNextFile(
	PK_ITERATOR_PTR		PKIteratorPtr,
	ISL_CONST_DATA_PTR	FilenamePtr,
	ISL_CONST_DATA_PTR	FiledataPtr)
{
	if (PKIteratorPtr == NULL || 
		FilenamePtr == NULL || 
		FiledataPtr == NULL || 
		PKIteratorPtr->LocalFilePtr == NULL)
		return ISL_FAIL;

	/* RLM: 4/14/98: we're doing shallow copies here...is that okay? */
	*FilenamePtr = PKIteratorPtr->LocalFilePtr->Filename;
	*FiledataPtr = PKIteratorPtr->LocalFilePtr->Filedata;
	PKIteratorPtr->LocalFilePtr = PKIteratorPtr->LocalFilePtr->Next;

	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: pk_RecycleFileEnumerator
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
pk_RecycleFileEnumerator(
	PK_ITERATOR_PTR		pIterator )
{
	if (pIterator)
		isl_FreeMemory(pIterator->PKArchivePtr->pMem, pIterator);
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: ParseLocalFile
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS
ParseLocalFile(
	PK_LOCAL_FILE_PTR pLocal,
    ISL_CONST_DATA Input,
    ISL_CONST_DATA_PTR UpdatedInput)
{
    const uint8 * pos;
    uint32 length;
	uint32 crc;
	uint32 table[256];


	if (pLocal == NULL ||
        Input.Data == NULL ||
        Input.Length == 0)
		return ISL_FAIL;


	/* Initialize local variables */
	CRC32_Init(table);
    pos = Input.Data;
    length = Input.Length;

	/* Skip over signature */
    if (4 > length) return ISL_FAIL;

	pos += 4;
    length -= 4;

	/* Allocate the data structure and patch the linked list */

	/* RLM: 4/14/98: we've added pLocal to the linked list, but don't
	 * restore the list to its original state (and free up pLocal) if
	 * we fail after this point.  Is that okay?
	 */

	/* RLM: 4/14/98: more memory sharing in the next line...OK? */
    if (sizeof(PK_LOCAL_FILE_HEADER) > length) return ISL_FAIL;

	pLocal->Header = (PK_LOCAL_FILE_HEADER_PTR)pos;
	pos += sizeof(PK_LOCAL_FILE_HEADER);
    length -= sizeof(PK_LOCAL_FILE_HEADER);

	/* Set up pointers to filename, extra field, and file data */
	pLocal->Filename.Length = pLocal->Header->FilenameLength;
	pLocal->ExtraField.Length = pLocal->Header->ExtraFieldLength;
	pLocal->Filedata.Length = pLocal->Header->CompressedSize;

	/* RLM: 4/14/98: We're sharing a lot of memory in the code below.
	 * are we sure no-one is going to free up (*pos)?
	 */
    if (pLocal->Filename.Length > length) return ISL_FAIL;
	pLocal->Filename.Data = pos;
	pos += pLocal->Filename.Length;
    length -= pLocal->Filename.Length;

    if (pLocal->ExtraField.Length > length) return ISL_FAIL;
	pLocal->ExtraField.Data = pos;
	pos += pLocal->ExtraField.Length;
    length -= pLocal->ExtraField.Length;

	if (pLocal->Filedata.Length > length) return ISL_FAIL;
    pLocal->Filedata.Data = pos;
	pos += pLocal->Filedata.Length;
    length -= pLocal->Filedata.Length;

	/* Check CRC */
	crc = CRC32(table, &pLocal->Filedata);
	if (crc != pLocal->Header->crc32) return ISL_FAIL;


	/* If bit 3 of general purpose bit flag is set, then there is
	   another descriptor struct to worry about */
	if (pLocal->Header->GeneralPurposeBitFlag & 0x00001000)
	{
	  /* RLM: 4/14/98: Yet more memory sharing.  OK? */
        if (sizeof(ISL_DATA_DESCRIPTOR) > length) return ISL_FAIL;
        
		pLocal->DataDescriptor = (ISL_DATA_DESCRIPTOR_PTR)pos;
		pos += sizeof(ISL_DATA_DESCRIPTOR);
        length -= sizeof(ISL_DATA_DESCRIPTOR);
	
		/* Make sure this archive is uncompressed */
		if (pLocal->DataDescriptor->CompressedSize !=
			pLocal->DataDescriptor->UncompressedSize)
		{	
			return ISL_FAIL;
		}
	} 
	else {
		
		/* Make sure this archive is uncompressed */
		if (pLocal->Header->CompressedSize !=
			pLocal->Header->UncompressedSize)
		{
			return ISL_FAIL;
		}
	}

	
    if (UpdatedInput)
    {
        UpdatedInput->Data = pos;
        UpdatedInput->Length = length;
    }

	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: ParseCentralFile
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS
ParseCentralFile(
	PK_CENTRAL_FILE_PTR	pCentral,
	ISL_CONST_DATA Input,
    ISL_CONST_DATA_PTR UpdatedInput)
{
    const uint8* pos;
    uint32 length;

	if (pCentral == NULL ||
        Input.Data == NULL ||
        Input.Length == 0)
		return ISL_FAIL;

    /* Initialize Local Variables */
    pos = Input.Data;
    length = Input.Length;
	/* Skip over the signature */
    if (4 > length) return ISL_FAIL;
	pos += 4;
    length -= 4;

	/* Grab the header */
	/* RLM: 4/14/98: memory-sharing...OK? */
    if (sizeof(PK_CENTRAL_FILE_HEADER) > length) return ISL_FAIL;
	pCentral->Header = (PK_CENTRAL_FILE_HEADER_PTR)pos;
	pos += sizeof(PK_CENTRAL_FILE_HEADER);
    length -= sizeof(PK_CENTRAL_FILE_HEADER);

	/* Check to make sure the data is not compressed */
	if (pCentral->Header->CompressedSize !=
		pCentral->Header->UncompressedSize)
		return ISL_FAIL;

	/* Set pointers to filename, extra field, and file comment */
	pCentral->Filename.Length = pCentral->Header->FilenameLength;
	pCentral->ExtraField.Length = pCentral->Header->ExtraFieldLength;
	pCentral->FileComment.Length = pCentral->Header->FileCommentLength;
	/* RLM: 4/14/98: memory sharing...OK? */
    if (pCentral->Filename.Length > length) return ISL_FAIL;
	pCentral->Filename.Data = pos;
	pos += pCentral->Filename.Length;
    length -= pCentral->Filename.Length;

	/* RLM: 4/14/98: memory sharing...OK? */
    if (pCentral->ExtraField.Length > length) return ISL_FAIL;
	pCentral->ExtraField.Data = pos;
	pos += pCentral->ExtraField.Length;
    length -= pCentral->ExtraField.Length;

	/* RLM: 4/14/98: memory sharing...OK? */
    if (pCentral->FileComment.Length > length) return ISL_FAIL;
	pCentral->FileComment.Data = pos;
	pos += pCentral->FileComment.Length;
    length -= pCentral->FileComment.Length;

    if (UpdatedInput)
    {
        UpdatedInput->Data = pos;
        UpdatedInput->Length = length;
    }
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: ParseCentralDirEnd
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS
ParseCentralDirEnd(
	ISL_MEMORY_CONTEXT_PTR	pMem,
	ISL_CONST_DATA Input,
    ISL_CONST_DATA_PTR UpdatedInput,
	PK_ARCHIVE_PTR	pArchive)
{
    const uint8 * pos;
    uint32 length;
	PK_CENTRAL_DIR_END_PTR pEnd = NULL;

	if (pArchive == NULL ||
        Input.Data == NULL ||
        Input.Length == 0)
		return ISL_FAIL;

    /* Initialize Local Variables */
    pos = Input.Data;
    length = Input.Length;
	/* Skip over signature */
    if (4 > length) return ISL_FAIL;
	pos += 4;
    length -= 4;

	pEnd = isl_AllocateMemory(pMem, sizeof(PK_CENTRAL_DIR_END));
	if (!pEnd) {
		return ISL_FAIL;
	}
	pArchive->DirEnd = pEnd;

	/* RLM: 4/14/98: memory sharing...OK? */
    if (sizeof(PK_CENTRAL_DIR_END_HEADER) > length) return ISL_FAIL;
	pEnd->Header = (PK_CENTRAL_DIR_END_HEADER_PTR)pos;
	pos += sizeof(PK_CENTRAL_DIR_END_HEADER);
    length -= sizeof(PK_CENTRAL_DIR_END_HEADER);

	/* Set up the zip file comment field */
	pEnd->ZipFileComment.Length = pEnd->Header->ZipfileCommentLength;
	/* RLM: 4/14/98: memory sharing...OK? */
    if (pEnd->ZipFileComment.Length > length) return ISL_FAIL;
	pEnd->ZipFileComment.Data = pos;
	pos += pEnd->ZipFileComment.Length;
    length -= pEnd->ZipFileComment.Length;

    if (UpdatedInput)
    {
        UpdatedInput->Data = pos;
        UpdatedInput->Length = length;
    }
	return ISL_OK;
}

