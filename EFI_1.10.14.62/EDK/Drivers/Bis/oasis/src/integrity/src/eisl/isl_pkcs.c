/*-----------------------------------------------------------------------
 *      File:   pkcs.c
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

#include "isl_internal.h"

static const uint8 signedDataoid[] = 
{0x2a, 0x86, 0x48, 0xf7, 0x0d, 0x07, 0x02};

static
ISL_STATUS
ParseSignedData(
	ISL_MEMORY_CONTEXT_PTR	MemoryPtr,
	BER_PARSED_ITEM			SignedData,
    ISL_PARSED_PKCS         *pPKCS);

/*-----------------------------------------------------------------------------
 * Name: isl_ParsePKCS7
 *
 * Description:
 *
 * Parameters:
 * MemoryPtr (input)
 * Image (input)
 * pPKCS (ouput)
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS
isl_ParsePKCS7(
    ISL_MEMORY_CONTEXT_PTR  MemoryPtr,
	ISL_CONST_DATA	        Image,
    ISL_PARSED_PKCS         *pPKCS)
{
	BER_PARSED_ITEM		    SignedData;	
	BER_PARSED_ITEM		    msgitem;
	BER_PARSED_ITEM         ContentType;
    BER_PARSED_ITEM         Content;
	const uint8			    *currentPosition = NULL;
	uint32			        remainingInput = 0;
    
	currentPosition = (uint8 *)Image.Data;
	remainingInput =  Image.Length;

    /* BER_CONSTRUCTED_SEQUENCE */
	currentPosition = BER_ExpandItem(
        currentPosition,
        remainingInput, 
        &msgitem);
	if (!currentPosition) 
		return ISL_FAIL;

    /* CONTENT TYPE = 2a 86 48 f7 0d 07 02 */
	currentPosition = msgitem.Content;
	remainingInput  = msgitem.ContentLength;
	currentPosition = BER_ExpandItem(currentPosition, 
					                remainingInput, 
					                &ContentType);
	if (!currentPosition)
		return ISL_FAIL;

	remainingInput  = 
        msgitem.ContentLength - 
        (uint32)(currentPosition - msgitem.Content);
	currentPosition = BER_ExpandItem(currentPosition, 
					                 remainingInput, 
					                 &Content);
	if (!currentPosition)
		return ISL_FAIL;
	currentPosition = Content.Content;
	remainingInput  = Content.ContentLength;
	currentPosition = BER_ExpandItem(currentPosition, 
					                 remainingInput, 
					                 &SignedData);
	if (!currentPosition)
	    return ISL_FAIL;

    if (ISL_OK != ParseSignedData(MemoryPtr, SignedData, pPKCS))
        return ISL_FAIL;

	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: ParseSignedData
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
ParseSignedData(
	ISL_MEMORY_CONTEXT_PTR	MemoryPtr,
	BER_PARSED_ITEM			SignedData,
    ISL_PARSED_PKCS         *pPKCS)
{
    BER_PARSED_ITEM		    *pNode = NULL;	
	const uint8				*currentPosition = NULL;
	uint32				    remainingInput = 0;
	sint32				    Count = 0;
	sint16                  i = 0;

	currentPosition = SignedData.Content;
	remainingInput  = SignedData.ContentLength;
	Count = BER_CountItems(currentPosition, remainingInput);
	if (Count <= 0) return ISL_FAIL;
	pNode = isl_AllocateMemory(MemoryPtr, Count * sizeof(BER_PARSED_ITEM));
	if (!pNode) {
		return ISL_FAIL;
	}
	for (i=0; i < Count; i++)
	{
		currentPosition = BER_ExpandItem(currentPosition, 
						                 remainingInput, 
						                 pNode + i);
		if (!currentPosition)
			return ISL_FAIL;
		remainingInput  = SignedData.ContentLength - (uint32)(currentPosition - SignedData.Content);
	}

    pPKCS->Node  = pNode;
	pPKCS->Count = Count;
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: isl_GetCertsFromPKCS
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
isl_GetCertsFromPKCS(
	ISL_MEMORY_CONTEXT_PTR MemoryPtr,
	ISL_PARSED_PKCS		    PKCS,
	ISL_CONST_DATA_PTR	    *pCerts,
	uint32			        *pCount)	
{
    uint32				i = 0;
	const uint8			*currentPosition = NULL;
	BER_PARSED_ITEM		*pNode;
	sint32				Count;

	if (pCount == NULL || 
        pCerts == NULL)
		return ISL_FAIL;

	*pCerts = NULL;
    for(i= 0; i < PKCS.Count; i++)
    {
        pNode = PKCS.Node + i;
	    if (pNode->Tag[0] == (BER_CONSTRUCTED | BER_CLASS_CONTEXTSPECIFIC | 0))
	        goto PARSE;
    }
	*pCount = 0;		/* no embedded certificates */
    return ISL_OK;

PARSE:
    {
	    Count = BER_CountItems(pNode->Content, pNode->ContentLength);
	    if (Count < 0)  return ISL_FAIL;

	    *pCount = Count;
	    if (*pCount == 0) return ISL_OK;
	    *pCerts = isl_AllocateMemory(
            MemoryPtr, 
            *pCount * sizeof(ISL_CONST_DATA));
	    if (!(*pCerts)){
		    return ISL_FAIL;
	    }

	    currentPosition = pNode->Content;
	    for(i=0; i < *pCount; i++)
	    {
            uint32  len = 0;

            len = BER_SizeofObject(currentPosition);
            if (len == 0) return ISL_FAIL;
		    (*pCerts)[i].Data = currentPosition;
		    (*pCerts)[i].Length = len;
		    currentPosition += len;
	    }

	    return ISL_OK;
    }
}

/*-----------------------------------------------------------------------------
 * Name: isl_GetSignedContentFromPKCS
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
isl_GetSignedContentFromPKCS(
	ISL_PARSED_PKCS	    PKCS,
	ISL_CONST_DATA_PTR	pSignedContent)
{
	const uint8		*currentPosition = NULL;
	uint32			remainingInput  = 0;
	BER_PARSED_ITEM Node;

    currentPosition = PKCS.Node[2].Content;
    remainingInput = PKCS.Node[2].ContentLength;
	currentPosition = BER_ExpandItem(currentPosition, remainingInput, &Node);
    if (!currentPosition) return ISL_FAIL;
	remainingInput  = PKCS.Node[2].ContentLength - (uint32)(currentPosition - PKCS.Node[2].Content);
    currentPosition = BER_ExpandItem(currentPosition, remainingInput, &Node);
    currentPosition = Node.Content;
    remainingInput = Node.ContentLength;
    currentPosition = BER_ExpandItem(currentPosition, remainingInput, &Node);
	pSignedContent->Data = Node.Content;
	pSignedContent->Length = Node.ContentLength;
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: isl_GetSignersFromPKCS
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
isl_GetSignersFromPKCS(
	ISL_MEMORY_CONTEXT_PTR  MemoryPtr,
    ISL_PARSED_PKCS         PKCS,
    ISL_CONST_DATA_PTR	    *pSignerImage,
	uint32				    *pSignerCount)
{
	sint32	                       count = 0;
	uint32	                       iSigners = 0;
	BER_PARSED_ITEM                *pSignerNodes = NULL;
    BER_PARSED_ITEM                *pNode;
    uint32                         NodeCount;

    *pSignerCount = 0;
	*pSignerImage  = NULL;
    pNode = PKCS.Node;
    NodeCount = PKCS.Count;

	/* skip over optional certificates and CRL which have context-specific tags */
	for (iSigners = 3; iSigners < NodeCount; iSigners++) {
		if (pNode[iSigners].Tag[0] == (BER_CONSTRUCTED | BER_SET)) 
			break;
    }

	if (iSigners >= NodeCount)
		return ISL_FAIL;

	count = BER_CountItems(pNode[iSigners].Content, pNode[iSigners].ContentLength);
	if (count <= 0)
		return ISL_OK;

	pSignerNodes = isl_AllocateMemory(MemoryPtr, count * sizeof(BER_PARSED_ITEM));
	if (!pSignerNodes) {
		return ISL_FAIL;
	}
	*pSignerCount = BER_ExpandSet(
                      pNode[iSigners].Content,
				      pNode[iSigners].ContentLength,
				      count,
				      0,
				      NULL,
				      pSignerNodes);
	
	if (*pSignerCount == 0)
		return ISL_OK;

	*pSignerImage = isl_AllocateMemory(MemoryPtr, *pSignerCount * sizeof(ISL_CONST_DATA));
	if (!*pSignerImage) {
		return ISL_FAIL;
	}
	for(iSigners=0; iSigners < *pSignerCount; iSigners++)
    {
	    pSignerImage[iSigners]->Data = pSignerNodes[iSigners].Content;
        pSignerImage[iSigners]->Length = pSignerNodes[iSigners].ContentLength;
    }
    return ISL_OK;
}
