/*-----------------------------------------------------------------------
 *      File:   tal_fips186.c
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
 * This file is part of the Token Adaptation Layer (TAL) source code base.
 * The TAL code makes it easier for CSP venders to develop CSPs that plug
 * into the Intel CDSA infrastructure.
 * This file contains functions to handle the DSA key structure defined in
 * the FIPS186.
 */

#include "tal_fips186.h"
#include "ber_der.h"
#include "cssmport.h"
#include "cssmerr.h"

/*----------------------------------------------------------------------
 *	Name: TAL_Parse_FIPS186_Key(...)
 *
 *	Description:
 *	Given a dsa keyblob, parses the key data and returns the parts as a
 *	series of CSSM_DATA structures. The key parts are returned in the
 *	order that they are found in the keyblob.
 *
 *	Keyblobs to be parsed must be unwrapped, use the current header
 *	format, and have the actual key data present. Passing key handle or
 *	label references will cause an error.
 *
 *  Output key data is in little-ENDIAN byte order.
 *
 *  Supported Keyblob Types:
 *
 *	BlobDescription: CSSM_KEYCLASS_PUBLIC_KEY, CSSM_KEYBLOB_RAW_FORMAT_FIPS186
 *      AlgorithmId: CSSM_ALGID_DSA
 *     Return Order: p, q, g, y
 *
 *	BlobDescription: CSSM_KEYCLASS_PRIVATE_KEY, CSSM_KEYBLOB_RAW_FORMAT_FIPS186
 *      AlgorithmId: CSSM_ALGID_DSA
 *     Return Order: p, q, g, x
 *
 * CSSM_KEYBLOB_RAW_FORMAT_FIPS186
 * This format is a condensed version of the complete standard for
 * exchanging DSA keys. It is the BER encoding of the following ASN.1
 * sequence.
 *	DSSPublicKey ::= SEQUENCE {
 *      primeModulus INTEGER, -- p
 *      primeDivisor INTEGER, -- q
 *      orderQ INTEGER, -- g
 *      publicKey INTEGER -- y }
 *
 *	DSSPrivateKey ::= SEQUENCE {
 *      primeModulus INTEGER, -- p
 *      primeDivisor INTEGER, -- q
 *      orderQ INTEGER, -- g
 *      privateKey INTEGER -- x }
 *
 *	Parameters:
 *  KeyBlob_ptr(input) - A pointer to a keyblob to be parsed.
 *	OutData_ptr(output) - A pointer to number of CSSM_DATA output buffers.
 *                        Caller has to allocate the CSSM_DATA buffers and put
 *                        the number in OutDataCount_ptr.
 *	OutDataCount_ptr(input/output) - contains the number of OutData_ptr
 *
 *	Returns:
 *     0 CSSM_OK:       The keyblob is parsed.The output parameters contains
 *                      the parts.
 *    -1 CSSM_FAIL:     Can not parse the key blob
 *    >0 CSSM_CSP_XXX:	Error Code
 *--------------------------------------------------------------------*/
sint32 TAL_Parse_FIPS186_Key(const CSSM_KEY_PTR KeyBlob_ptr,
                             CSSM_DATA_PTR OutData_ptr,
                             uint32 *OutDataCount_ptr)
{
    /* DSA_PRIKEY_PARAM_COUNT = DSA_PUBKEY_PARAM_COUNT = 4 */
    const uint8         *dataPtr = NULL;
    uint32              keyDataLength = 0, i, j;
    uint16              dataCount;
    CSSM_KEYHEADER_PTR	header = NULL;
    unsigned char       berTag[DSA_PRIKEY_PARAM_COUNT * 2];
    BER_PARSED_ITEM     berData[DSA_PRIKEY_PARAM_COUNT];

    /* Check parameters */
    if ((KeyBlob_ptr == NULL) || (KeyBlob_ptr->KeyData.Data == NULL))
	{
        return CSSM_CSP_INVALID_KEY;
    }
    if ((OutData_ptr == NULL) || (OutDataCount_ptr == NULL))
	{
        return CSSM_CSP_INVALID_POINTER;
    }
    dataPtr = KeyBlob_ptr->KeyData.Data;
    header = &KeyBlob_ptr->KeyHeader;
    keyDataLength = KeyBlob_ptr->KeyData.Length;

    /* Check the key class */
    switch (header->KeyClass)
    {
    case CSSM_KEYCLASS_PUBLIC_KEY:
        dataCount = DSA_PUBKEY_PARAM_COUNT;
        break;
    case CSSM_KEYCLASS_PRIVATE_KEY:
        dataCount = DSA_PRIKEY_PARAM_COUNT;
        break;
    default:
        return CSSM_CSP_INVALID_KEYCLASS;
    }
    /* Check the header's magic number, the key isn't wrapped and
       the data is actually present */
    if ((header->HeaderVersion != CSSM_KEYHEADER_VERSION) ||
        (header->WrapAlgorithmId != CSSM_ALGID_NONE) ||
        (header->AlgorithmId != CSSM_ALGID_DSA) ||
        (header->BlobType != CSSM_KEYBLOB_RAW_BERDER) ||
        (header->Format	  != CSSM_KEYBLOB_RAW_FORMAT_FIPS186))
	{
        return CSSM_CSP_INVALID_KEY;
    }
    /* Make sure enough output buffers were passed */
    if (*OutDataCount_ptr < dataCount)
	{
        return CSSM_CSP_NOT_ENOUGH_BUFFER;
    }
    /* Parse the key data */
    for (i = 0; i < dataCount; i++)
	{
        berData[i].Tag = &berTag[i * 2];
    }
    dataPtr = BER_ExpandItem(dataPtr, keyDataLength, berData);
    if ((dataPtr != NULL) && (*berData[0].Tag == BER_CONSTRUCTED_SEQUENCE))
	{
        *OutDataCount_ptr = BER_ExpandSet(berData[0].Content,
                                          berData[0].ContentLength,
                                          dataCount, 0, NULL, berData);
        /* dataCount = DSA_PRIKEY_PARAM_COUNT = DSA_PUBKEY_PARAM_COUNT = 4 */
		if (*OutDataCount_ptr != dataCount)
		{
			return CSSM_CSP_INVALID_KEY;
		}
		else
		{
			/* Check element tag, check/allocate space, and copy the data */
			for (i = 0; i < *OutDataCount_ptr; i++)
			{
				if (*berData[i].Tag != BER_INTEGER)
				{
					return CSSM_CSP_INVALID_KEY;
				}
				if (berData[i].Content[0] == 0x00)
				{
					berData[i].Content++;
					berData[i].ContentLength--;
				}
				/* Allocate space for empty buffer */
				if (OutData_ptr[i].Data == NULL)
				{
					OutData_ptr[i].Data = 
						cssm_calloc(1, berData[i].ContentLength + 1, NULL);
					if (OutData_ptr[i].Data == NULL)
					{
						return CSSM_CSP_MEMORY_ERROR;
					}
				}
				else
				{
					/*Check & make sure that caller has provided enough space*/
					if (OutData_ptr[i].Length < berData[i].ContentLength)
					{
						return CSSM_CSP_ERR_OUTBUF_LENGTH;
					}
				}
				/* Now, copy it */
				for (j = 0; j < berData[i].ContentLength; j++) 
				{
					OutData_ptr[i].Data[berData[i].ContentLength - j - 1] =
						berData[i].Content[j];
				}
				OutData_ptr[i].Length = berData[i].ContentLength;
			}
		}
	}
	else
	{
        return CSSM_FAIL;
	}
    return CSSM_OK;
}

/*----------------------------------------------------------------------
 * TAL_Build_FIPS186_Key
 *
 *	Given a dsa keyblob structure and a set of key parts, the keydata
 *	portion of the keyblob is formulated to a ber_der encoded keyblob which
 *  has the fips186 key elements. The key parts are represented as an array
 *	of CSSM_DATA structures, and are required to be in the order specified
 *  for each key type below.
 *
 *	Keyblobs to be created is unwrapped and have the actual key data present.
 *  The following key header fields are set to refelect the key
 *      HeaderVersion	= CSSM_KEYHEADER_VERSION;
 *      BlobType        = CSSM_KEYBLOB_RAW_BERDER;
 *      Format          = CSSM_KEYBLOB_RAW_FORMAT_FIPS186;
 *      WrapAlgorithmId = CSSM_ALGID_NONE;
 *      WrapMode        = CSSM_ALGMODE_NONE;
 *
 *  Input key data is assumed to be little-ENDIAN byte order.
 *
 *  Supported Keyblob Types:
 *
 *	BlobDescription: CSSM_KEYCLASS_PUBLIC_KEY, CSSM_KEYBLOB_RAW_FORMAT_FIPS186
 *      AlgorithmId: CSSM_ALGID_DSA
 *     Return Order: p, q, g, y
 *
 *	BlobDescription: CSSM_KEYCLASS_PRIVATE_KEY, CSSM_KEYBLOB_RAW_FORMAT_FIPS186
 *      AlgorithmId: CSSM_ALGID_DSA
 *     Return Order: p, q, g, x
 *
 * CSSM_KEYBLOB_RAW_FORMAT_FIPS186
 * This format is a condensed version of the complete standard for
 * exchanging DSA keys. It is the BER encoding of the following ASN.1
 * sequence.
 *	DSSPublicKey ::= SEQUENCE {
 *      primeModulus INTEGER, -- p
 *      primeDivisor INTEGER, -- q
 *      orderQ INTEGER, -- g
 *      publicKey INTEGER -- y }
 *
 *	DSSPrivateKey ::= SEQUENCE {
 *      primeModulus INTEGER, -- p
 *      primeDivisor INTEGER, -- q
 *      orderQ INTEGER, -- g
 *      privateKey INTEGER -- x }
 *
 *	Parameters:
 *  KeyBlob_ptr(output) - A pointer to a keyblob buffer. Caller has to allocate
 *                        this buffer.
 *	InData(input) - A pointer to number of CSSM_DATA input buffers. 
 *	InDataCount(input) - contains the number of InData
 *
 *	Returns:
 *     0 CSSM_OK:     The keyblob is parsed. The out param contains the parts.
 *    -1 CSSM_FAIL:	  Can not parse the key blob
 *    >0 CSSM_CSP_XXX:Error Code
 *--------------------------------------------------------------------*/
sint32 TAL_Build_FIPS186_Key(CSSM_KEY_PTR KeyBlob_ptr,
                            const CSSM_DATA_PTR InData_ptr,
							uint32 InDataCount)
{
    /* DSA_PRIKEY_PARAM_COUNT = FIPS186_PUBLIC_PARAM_COUNT = 4 */
    uint8	*destPtr = NULL;
    uint32	i, j, msb_is_set;
    uint16	dataCount;
    CSSM_KEYHEADER_PTR header = NULL;
    BER_PARSED_ITEM berData[DSA_PRIKEY_PARAM_COUNT];
    unsigned char berTag[] = {BER_INTEGER};
    unsigned char berSeqTag[] = {BER_CONSTRUCTED_SEQUENCE};
    uint8 *tempContent;

    /* Check parameters */
    if ((KeyBlob_ptr == NULL) || (KeyBlob_ptr->KeyData.Data == NULL))
	{
        return CSSM_CSP_INVALID_KEY;
    }
    if ((InData_ptr == NULL) || (InDataCount == 0))
	{
        return CSSM_CSP_INVALID_POINTER;
    }
    destPtr = KeyBlob_ptr->KeyData.Data;
    header = &KeyBlob_ptr->KeyHeader;

    /* Check the key algorithm */
    if (header->AlgorithmId	!= CSSM_ALGID_DSA)
	{
        return CSSM_CSP_INVALID_KEY;
    }
    /* Check the key class */
    switch (header->KeyClass)
    {
    case CSSM_KEYCLASS_PUBLIC_KEY:
        dataCount = DSA_PUBKEY_PARAM_COUNT;
        break;
    case CSSM_KEYCLASS_PRIVATE_KEY:
        dataCount = DSA_PRIKEY_PARAM_COUNT;
        break;
    default:
        return CSSM_CSP_INVALID_KEYCLASS;
    }
    /* Make sure correct number of buffers were passed */
    if (InDataCount != dataCount)
        return CSSM_CSP_INTERNAL_ERROR;

    /* Construct the key data */
    for (i = 0; i < InDataCount; i++)
	{
        msb_is_set = 0;
        berData[i].Tag = berTag;
        tempContent = cssm_calloc(1, InData_ptr[i].Length + 1, NULL);
        if (InData_ptr[i].Data[InData_ptr[i].Length - 1] & 0x80)
		{
            msb_is_set = 1;
            tempContent[0] = 0x00;
        }
        for (j = 0; j < InData_ptr[i].Length; j++)
		{
            if (msb_is_set)
                tempContent[InData_ptr[i].Length-j] = InData_ptr[i].Data[j];
            else
                tempContent[InData_ptr[i].Length-j-1] = InData_ptr[i].Data[j];
        }
        if (msb_is_set)
		{
            berData[i].ContentLength = InData_ptr[i].Length + 1;
		}
        else
		{
            berData[i].ContentLength = InData_ptr[i].Length;
		}
        berData[i].Content = tempContent;
    }
    KeyBlob_ptr->KeyData.Length = 
        DER_ConstructItem(berSeqTag,(sint16)InDataCount,berData,destPtr);
    for (i = 0; i < InDataCount; i++)
	{
        cssm_free((void *)berData[i].Content, NULL);        
	}
    /* Fill the fields value in the key header, 
       FIPS Pub 186 is specific to DSS keys */
    header->HeaderVersion	= CSSM_KEYHEADER_VERSION;
    header->BlobType        = CSSM_KEYBLOB_RAW_BERDER;
    header->Format          = CSSM_KEYBLOB_RAW_FORMAT_FIPS186;
    header->WrapAlgorithmId = CSSM_ALGID_NONE;
    header->WrapMode        = CSSM_ALGMODE_NONE;

    return CSSM_OK;
}
