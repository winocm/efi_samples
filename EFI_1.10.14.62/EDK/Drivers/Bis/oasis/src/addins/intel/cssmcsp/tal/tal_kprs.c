/*-----------------------------------------------------------------------
 *      File:   tal_kprs.c
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
 * This file contains key format functions for PKCS1,3,8 and FIPS186.
 */

#include "tal_inc.h"
#include "ber_der.h"

/*########################### CSSM_BIS ###########################*/
#ifndef CSSM_BIS
/*#include ".\keyfmt\tal_pkcs1.h" */
/*#include ".\keyfmt\tal_pkcs8.h" */
/*#include ".\keyfmt\tal_raw.h"   */
#endif
/*########################### CSSM_BIS ###########################*/

/*-----------------------------------------------------------------------------
 *Name: TAL_ParseKeyBlob(...)
 *
 *Description:
 *	Given a KeyBlob_ptr, parses the key data and returns the parts as a
 *	series of CSSM_DATA structures. The key parts are returned in the
 *	order that they are found in the KeyBlob_ptr.
 *
 *	Keyblobs to be parsed must be unwrapped, use the current header
 *	format, and have the actual key data present. Passing key handle or
 *	label references will cause an error.
 *
 *  Output key data is in little-ENDIAN byte order.
 *
 *Supported Keyblob Types:
 *	BlobDescription: CSSM_KEYCLASS_SESSION_KEY, CSSM_KEYBLOB_RAW_FORMAT_NONE
 *      AlgorithmId: Any
 *     Return Order: N/A
 *
 *	BlobDescription: CSSM_KEYCLASS_PUBLIC_KEY, CSSM_KEYBLOB_RAW_FORMAT_PKCS1
 *      AlgorithmId: CSSM_ALGID_RSA
 *     Return Order: modulus, public exponent
 *
 *	BlobDescription: CSSM_KEYCLASS_PRIVATE_KEY, CSSM_KEYBLOB_RAW_FORMAT_PKCS1
 *      AlgorithmId: CSSM_ALGID_RSA
 *     Return Order: modulus, public exponent, ...
 *
 *	BlobDescription: CSSM_KEYCLASS_PUBLIC_KEY, CSSM_KEYBLOB_RAW_FORMAT_FIPS186
 *      AlgorithmId: CSSM_ALGID_DSA
 *     Return Order: p, q, g, y
 *
 *	BlobDescription: CSSM_KEYCLASS_PRIVATE_KEY, CSSM_KEYBLOB_RAW_FORMAT_FIPS186
 *      AlgorithmId: CSSM_ALGID_DSA
 *     Return Order: p, q, g, x
 *
 *	BlobDescription: CSSM_KEYCLASS_PRIVATE_KEY, CSSM_KEYBLOB_RAW_FORMAT_PKCS8
 *      AlgorithmId: CSSM_ALGID_RSA
 *     Return Order: Ber Encoded CSSM_KEYBLOB_RAW_FORMAT_PKCS1 private key blob
 *
 *	BlobDescription: CSSM_KEYCLASS_PRIVATE_KEY, CSSM_KEYBLOB_RAW_FORMAT_PKCS8
 *      AlgorithmId: CSSM_ALGID_DSA
 *     Return Order: Ber Encoded CSSM_KEYBLOB_RAW_FORMAT_FIPS186 private key
 *                   blob
 *
 *Parameters:
 *  KeyBlob_ptr(input) - A pointer to a KeyBlob_ptr to be parsed.
 *	OutData_ptr(output) - A pointer to number of CSSM_DATA output buffers. 
 *                    Caller has to allocate the CSSM_DATA buffers and put the
 *                     number in OutDataCount_ptr.
 *	OutDataCount_ptr(input/output) - contains the number of OutData_ptr
 *
 *Returns:
 *  CSSM_OK:   The KeyBlob_ptr is parsed. The output params contains the parts.
 *  CSSM_FAIL: Can not parse the key blob
 *---------------------------------------------------------------------------*/
 CSSM_RETURN TAL_ParseKeyBlob(const CSSM_KEY_PTR KeyBlob_ptr,
                              CSSM_DATA_PTR OutData_ptr,
                              uint32 *OutDataCount_ptr)
{
    uint8	*dataPtr = NULL;
    uint32	keyDataLength = 0;
    sint32	rtnCode;
    CSSM_KEYHEADER_PTR	header = NULL;

    /* Check parameters */
    if ((OutData_ptr == NULL) || (OutDataCount_ptr == NULL))
    {
        TAL_SetError(CSSM_CSP_INVALID_POINTER);
        return CSSM_FAIL;
    }
    /* Verify the key structure */
    if (TAL_ValidateInKeyParam(KeyBlob_ptr) != CSSM_OK)
	{
        return CSSM_FAIL;
	}
    dataPtr = KeyBlob_ptr->KeyData.Data;
    header = &KeyBlob_ptr->KeyHeader;
    keyDataLength = KeyBlob_ptr->KeyData.Length;

    /* Check the header's magic number, the key isn't wrapped and */
    /* the data is actually present */
    if ((header->HeaderVersion != CSSM_KEYHEADER_VERSION) ||
        (header->WrapAlgorithmId != CSSM_ALGID_NONE))
    {
        TAL_SetError(CSSM_CSP_INVALID_KEY);
        return CSSM_FAIL;
    }
    /* Parse the key for each key class and format */
    switch (header->KeyClass)
    {
    case CSSM_KEYCLASS_PUBLIC_KEY:
        switch (header->Format)
        {
        case CSSM_KEYBLOB_RAW_FORMAT_FIPS186:
            if ((rtnCode = TAL_Parse_FIPS186_Key(KeyBlob_ptr,
                                              OutData_ptr,
                                              OutDataCount_ptr)) != CSSM_OK)
            {
                TAL_SetError(rtnCode);
                return CSSM_FAIL;
            }
            break;

    /*######################### bgn not CSSM_BIS ##########################*/
#ifndef CSSM_BIS
        case CSSM_KEYBLOB_RAW_FORMAT_PKCS1:
            if ((rtnCode = TAL_Parse_PKCS1_Key(KeyBlob_ptr,
                                              OutData_ptr,
                                              OutDataCount_ptr)) != CSSM_OK)
            {
                TAL_SetError(rtnCode);
                return CSSM_FAIL;
            }
            break;

        /* CSSM_KEYBLOB_RAW_FORMAT_PKCS3
         * This format is specified in RSA's PKCS #3. It is the raw public
         * value bignum in big endian format.
         */
        case CSSM_KEYBLOB_RAW_FORMAT_PKCS3:
            /* PKCS #3 is specific to Diffie-Hellman */
            if (header->AlgorithmId != CSSM_ALGID_DH)
            {
                TAL_SetError(CSSM_CSP_KEY_FORMAT_INCORRECT);
                return CSSM_FAIL;
            }
            *OutDataCount_ptr = 1;
            cssm_memcpy(OutData_ptr[0].Data, dataPtr, keyDataLength);
            OutData_ptr[0].Length = keyDataLength;
#endif /*CSSM_BIS*/
    /*######################### end not CSSM_BIS ##########################*/

        default:
            TAL_SetError(CSSM_CSP_KEY_FORMAT_INCORRECT);
            return CSSM_FAIL;
        } /* switch FORMAT */
        break;

    /*######################### bgn not CSSM_BIS ##########################*/
#ifndef CSSM_BIS
    case CSSM_KEYCLASS_SESSION_KEY:
        switch (header->BlobType)
        {
        case CSSM_KEYBLOB_RAW:
            switch (header->Format)
            {
            case CSSM_KEYBLOB_RAW_FORMAT_NONE:
                if ((rtnCode = TAL_Parse_RAW_Key(KeyBlob_ptr,
                                                OutData_ptr,
                                                OutDataCount_ptr)) != CSSM_OK)
                {
                    TAL_SetError(rtnCode);
                    return CSSM_FAIL;
                }
                break;
            default:
                TAL_SetError(CSSM_CSP_KEY_FORMAT_INCORRECT);
                return CSSM_FAIL;
            }
        default:
            TAL_SetError(CSSM_CSP_KEY_BLOBTYPE_INCORRECT);
            return CSSM_FAIL;
        } /* switch BlobType */
        break;

    case CSSM_KEYCLASS_PRIVATE_KEY:
        switch (header->Format)
        {
        case CSSM_KEYBLOB_RAW_FORMAT_PKCS1:
            if ((rtnCode = TAL_Parse_PKCS1_Key(KeyBlob_ptr,
                                              OutData_ptr,
                                              OutDataCount_ptr)) != CSSM_OK)
            {
                TAL_SetError(rtnCode);
                return CSSM_FAIL;
            }
            break;

        case CSSM_KEYBLOB_RAW_FORMAT_FIPS186:
            if ((rtnCode = TAL_Parse_FIPS186_Key(KeyBlob_ptr,
                                              OutData_ptr,
                                              OutDataCount_ptr)) != CSSM_OK)
            {
                TAL_SetError(rtnCode);
                return CSSM_FAIL;
            }
            break;

        case CSSM_KEYBLOB_RAW_FORMAT_PKCS8:
            {
                uint32 KeyAlgID, KeyDataLength, Format;
                uint32 KeyDataOffset;
                
                /* Make sure enough output buffers were passed */
                if (*OutDataCount_ptr != 1)
				{
                    TAL_SetError(CSSM_CSP_INVALID_DATA_COUNT);              
                    return CSSM_FAIL;
                }
                if ((rtnCode = TAL_Parse_PKCS8_RawKey(KeyBlob_ptr, &KeyAlgID,
                          &Format, &KeyDataLength, &KeyDataOffset)) != CSSM_OK)
                {
                    TAL_SetError(rtnCode);
                    return CSSM_FAIL;
                }
                if (header->AlgorithmId != KeyAlgID) 
                {
                    TAL_SetError(CSSM_CSP_KEY_KEYHEADER_INCONSISTENT);
                    return CSSM_FAIL;
                }
                /* Make sure the output buffer has space and is big enough,
                   then copy to output buffer. */
                if (OutData_ptr[0].Data == NULL)
                {
                    OutData_ptr[0].Data = cssm_calloc(1, KeyDataLength, NULL);                                                  
                    if (OutData_ptr[0].Data == NULL)
                    {
                        TAL_SetError(CSSM_CSP_MEMORY_ERROR);
                        return CSSM_FAIL;
                    }
                }
                else
                {	/*Check & make sure that caller has provided enough space*/
                    if (OutData_ptr[0].Length < KeyDataLength)
                    {
                        TAL_SetError(CSSM_CSP_NOT_ENOUGH_BUFFER);
                        return CSSM_FAIL;           
                    }                   
                }
                OutData_ptr[0].Length = KeyDataLength;
                cssm_memcpy(OutData_ptr[0].Data, 
                            KeyBlob_ptr->KeyData.Data+KeyDataOffset,
                            OutData_ptr[0].Length);
                break;
            }
        default:
            TAL_SetError(CSSM_CSP_KEY_FORMAT_INCORRECT);
            return CSSM_FAIL;
        } /* switch FORMAT */
        break;
#endif /*CSSM_BIS*/
    /*######################### end not CSSM_BIS ##########################*/

    default:
        TAL_SetError(CSSM_CSP_INVALID_KEYCLASS);
        return CSSM_FAIL;
    } /* switch KeyClass */

    return CSSM_OK;
}

/*-----------------------------------------------------------------------------
 * TAL_CreateKeyBlob
 *
 *Description:
 *	Given a KeyBlob_ptr structure and a set of key parts, the keydata
 *	portion of the KeyBlob_ptr is formulated based on the values in the
 *	key header. The key parts are represented as an array of CSSM_DATA
 *	structures, and are required to be in the order specified for each
 *	key type below.
 *
 *	Keyblobs to be created must be unwrapped, use the current header
 *	format, and have the actual key data present. Requesting key handle
 *	or label references will cause an error.
 *
 *  Input key data is assumed to be little-ENDIAN byte order.
 *
 *Supported Keyblob Types:
 *	BlobDescription: CSSM_KEYCLASS_SESSION_KEY, CSSM_KEYBLOB_RAW_FORMAT_NONE
 *      AlgorithmId: Any
 *     Data Order: N/A
 *
 *	BlobDescription: CSSM_KEYCLASS_PUBLIC_KEY, CSSM_KEYBLOB_RAW_FORMAT_PKCS1
 *      AlgorithmId: CSSM_ALGID_RSA
 *     Data Order: modulus, public exponent
 *
 *	BlobDescription: CSSM_KEYCLASS_PRIVATE_KEY, CSSM_KEYBLOB_RAW_FORMAT_PKCS1
 *      AlgorithmId: CSSM_ALGID_RSA
 *     Data Order: modulus, public exponent, ...
 *
 *	BlobDescription: CSSM_KEYCLASS_PUBLIC_KEY, CSSM_KEYBLOB_RAW_FORMAT_FIPS186
 *      AlgorithmId: CSSM_ALGID_DSA
 *     Data Order: p, q, g, y
 *
 *	BlobDescription: CSSM_KEYCLASS_PRIVATE_KEY, CSSM_KEYBLOB_RAW_FORMAT_FIPS186
 *      AlgorithmId: CSSM_ALGID_DSA
 *     Data Order: p, q, g, x
 *
 *	BlobDescription: CSSM_KEYCLASS_PRIVATE_KEY, CSSM_KEYBLOB_RAW_FORMAT_PKCS8
 *      AlgorithmId: CSSM_ALGID_DSA/CSSM_ALGID_DSA
 *     Data Order: PrivateKey
 *
 *Parameters:
 *  KeyBlob_ptr(output) - A pointer to a KeyBlob_ptr buffer. Caller has to
 *                        allocate this buffer.
 *	InData_ptr(input) - A pointer to number of CSSM_DATA input buffers. 
 *	InDataCount(input) - contains the number of InData_ptr
 *
 *Returns:
 *  CSSM_OK:   The KeyBlob_ptr is created.
 *  CSSM_FAIL: Can not create the key blob form InData_ptr
 *---------------------------------------------------------------------------*/
CSSM_RETURN TAL_CreateKeyBlob(CSSM_KEY_PTR KeyBlob_ptr,
                              const CSSM_DATA_PTR InData_ptr,
                              uint32 InDataCount)
{
    uint8	*destPtr = NULL;
    sint32  rtnCode;
    CSSM_KEYHEADER_PTR header = NULL;

    /* Check parameters */
    if (InData_ptr == NULL)
    {
        TAL_SetError(CSSM_CSP_INVALID_DATA_POINTER);
        return CSSM_FAIL;
    }
    if (InDataCount == 0)
    {
        TAL_SetError(CSSM_CSP_INVALID_DATA_COUNT);
        return CSSM_FAIL;
    }
    /* Verify the key structure */
    if (TAL_ValidateInKeyParam(KeyBlob_ptr) != CSSM_OK)
        return CSSM_FAIL;

    destPtr = KeyBlob_ptr->KeyData.Data;
    header = &KeyBlob_ptr->KeyHeader;

    /* Check the header's magic number, the key isn't wrapped and
       the data is actually present */
    if ((header->HeaderVersion != CSSM_KEYHEADER_VERSION) ||
        (header->WrapAlgorithmId != CSSM_ALGID_NONE) ||
        ((header->BlobType != CSSM_KEYBLOB_RAW) &&
         (header->BlobType != CSSM_KEYBLOB_RAW_BERDER)))
	{
        TAL_SetError(CSSM_CSP_KEY_BLOBTYPE_INCORRECT);
        return CSSM_FAIL;
    }
    /* Create the key for each key class and format */
    switch (header->KeyClass)
    {
    case CSSM_KEYCLASS_PUBLIC_KEY:
        switch (header->Format)
        {
        case CSSM_KEYBLOB_RAW_FORMAT_FIPS186:
            if ((rtnCode = TAL_Build_FIPS186_Key(KeyBlob_ptr,
                                              InData_ptr,
                                              InDataCount)) != CSSM_OK)
            {
                TAL_SetError(rtnCode);
                return CSSM_FAIL;
            }
            break;

    /*######################### bgn not CSSM_BIS ##########################*/
#ifndef CSSM_BIS
        case CSSM_KEYBLOB_RAW_FORMAT_PKCS1:
            if ((rtnCode = TAL_Build_PKCS1_Key(KeyBlob_ptr,
                                              InData_ptr,
                                              InDataCount)) != CSSM_OK)
            {
                TAL_SetError(rtnCode);
                return CSSM_FAIL;
            }
            break;

        /* CSSM_BLOCDESC_PKCS3
         * This format is used with keys generated as specified in PKCS #3. The
         * public value y is the only value that is placed in the KeyBlob_ptr.
         * This mode is equivalent to CSSM_BLOBDESC_RAW in since there is no 
         * formatting data.
         */
        case CSSM_KEYBLOB_RAW_FORMAT_PKCS3:
            /* PKCS #3 is specific to Diffie-Hellman keys */
            if (header->AlgorithmId != CSSM_ALGID_DH)
            {
                TAL_SetError(CSSM_CSP_KEY_FORMAT_INCORRECT);
                return CSSM_FAIL;
            }
            /* Make sure the correct number of buffers were passed */
            if (InDataCount != 1) {
                TAL_SetError(CSSM_CSP_INVALID_DATA_COUNT);              
                return CSSM_FAIL;
            }
            /* Construct the key data */
            cssm_memcpy(destPtr,InData_ptr[0].Data,InData_ptr[0].Length);
            KeyBlob_ptr->KeyData.Length = InData_ptr[0].Length;
            break;
#endif /*CSSM_BIS*/
    /*######################### end not CSSM_BIS ##########################*/

        default:
            TAL_SetError(CSSM_CSP_KEY_FORMAT_INCORRECT);
            return CSSM_FAIL;
        } /* switch Format */
        break;

    /*######################### end not CSSM_BIS ##########################*/
#ifndef  CSSM_BIS

    case CSSM_KEYCLASS_SESSION_KEY:
        switch (header->Format)
        {
        case CSSM_KEYBLOB_RAW_FORMAT_NONE:
            if ((rtnCode = TAL_Build_RAW_Key(KeyBlob_ptr,
                                             InData_ptr,
                                             InDataCount)) != CSSM_OK)
            {
                TAL_SetError(rtnCode);
                return CSSM_FAIL;
            }
            break;

        default:
            TAL_SetError(CSSM_CSP_KEY_FORMAT_INCORRECT);
            return CSSM_FAIL;
        } /* switch Format */
        break;

    case CSSM_KEYCLASS_PRIVATE_KEY:
        switch (header->Format)
        {
        case CSSM_KEYBLOB_RAW_FORMAT_PKCS1:
            if ((rtnCode = TAL_Build_PKCS1_Key(KeyBlob_ptr,
                                              InData_ptr,
                                              InDataCount)) != CSSM_OK)
            {
                TAL_SetError(rtnCode);
                return CSSM_FAIL;
            }
            break;

        case CSSM_KEYBLOB_RAW_FORMAT_FIPS186:
            if ((rtnCode = TAL_Build_FIPS186_Key(KeyBlob_ptr,
                                                InData_ptr,
                                                InDataCount)) != CSSM_OK)
            {
                TAL_SetError(rtnCode);
                return CSSM_FAIL;
            }
            break;

#ifdef CAP_KEYPAIR_GEN
        case CSSM_KEYBLOB_RAW_FORMAT_PKCS8:
            {
                uint32	NeedSize;
                sint32	rtnCode;

                /* Make sure correct number of buffers were passed */
                if (InDataCount != 1) {
                    TAL_SetError(CSSM_CSP_INVALID_DATA_COUNT);              
                    return CSSM_FAIL;
                }
                /* Try to create the key.  */
                /* Get the output size, if there is no out keydata buffer. */
                rtnCode = TAL_Create_PKCS8_RawKey(KeyBlob_ptr, &NeedSize,
                                            header->AlgorithmId, InData_ptr);

                /* Make sure the output buffer has space and is big enough,
                   then copy to output buffer. */
                if (rtnCode == CSSM_CSP_NOT_ENOUGH_BUFFER)
                {
                    if (KeyBlob_ptr->KeyData.Length == 0)
                    {
                        if ((KeyBlob_ptr->KeyData.Data =
                                cssm_calloc(1, NeedSize, NULL)) == NULL)
                        {
                            TAL_SetError(CSSM_CSP_MEMORY_ERROR);
                            return CSSM_FAIL;
                        }
                        KeyBlob_ptr->KeyData.Length = NeedSize;
                        rtnCode = TAL_Create_PKCS8_RawKey(KeyBlob_ptr, NULL,
                                              header->AlgorithmId, InData_ptr);
                    }
                    else
                    {
                        TAL_SetError(CSSM_CSP_NOT_ENOUGH_BUFFER);
                        return CSSM_FAIL;           
                    }                   
                }
                if (rtnCode != CSSM_OK)
                {
                    TAL_SetError(CSSM_CSP_INVALID_KEY);
                    return CSSM_FAIL;
                }
                break;
            }
#endif /* CAP_KEYPAIR_GEN */

        default:
            TAL_SetError(CSSM_CSP_KEY_FORMAT_INCORRECT);
            return CSSM_FAIL;
        } /* switch Format */
        break;

#endif /*CSSM_BIS*/
    /*######################### end not CSSM_BIS ##########################*/

    default:
        TAL_SetError(CSSM_CSP_INVALID_KEYCLASS);
        return CSSM_FAIL;
    } /* switch KeyClass*/

    return CSSM_OK;
}

