/*-----------------------------------------------------------------------
 *      File:   ISLUTIL.C
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
//#include "memory.h"
#include "islutil.h"
#include "isl_internal.h"
#define MAXBYTES 3

/*-----------------------------------------------------------------------------
 * Name: islutilEncodeValue
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
char islutilEncodeValue(uint8 value) 
{
	char EncodedChar = '=';

	if (value >= 0) {
		if (value <= 25)
			EncodedChar = (uint8) (value + 'A');
	    else if (value <=51)
			EncodedChar = (uint8) (value - 25 - 1 + 'a');
		else if (value <=61)
			EncodedChar = (uint8) (value - 51 - 1 + '0');
		else if (value == 62)
			EncodedChar = '+';
		else if (value == 63)
			EncodedChar = '/';
	}
	return EncodedChar;
}

/*-----------------------------------------------------------------------------
 * Name: islutilDecodeChar
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
uint8 islutilDecodeChar(char EncodedChar) 
{
	uint8 value = 0;

	if ((EncodedChar >= 'A') && (EncodedChar  <= 'Z'))
		value = (uint8) (EncodedChar - 'A');
	else if ((EncodedChar >= 'a') && (EncodedChar  <= 'z'))
		value = (uint8) (EncodedChar - 'a' + 26);
	else if ((EncodedChar >= '0') && (EncodedChar  <= '9'))
		value = (uint8) (EncodedChar - '0' + 52);
	else if (EncodedChar == '+')
		value = 62;
	else if (EncodedChar == '/')
		value = 63;

	return value;
}

/*-----------------------------------------------------------------------------
 * Name: islutil_GetBits
 *
 * Description:
 * This functions returns the (right adjusted) n-bit field of x that
 * begins at position p.  Assume that bit position 0 is at the right end
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
uint32 islutil_GetBits(uint32 x, int p, int n) 
{
	return(x>>(p+1-n)) & ~(~0 <<n);
}

	
/*-----------------------------------------------------------------------------
 * Name: islutil_EncodeGroup
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
void islutil_EncodeGroup(const uint8 *InputData, uint8 BytesInGroup, char* EncryptedData)
{

	uint32 lData = 0;
	int i;
	uint8 pos, value;
	uint8 CharCount = 0;

	/* copy 24-bits data to an unsigned long integer (right adjusted) */
	for (i=0; i<BytesInGroup; i++) {
		lData <<= 8;
		lData |= InputData[i];
	}

	/* if few than 3 bytes are available in the input, zero bits are padded on 
	   the right*/
	for (i=BytesInGroup; i<3; i++)
		lData <<= 8;

	
	if (BytesInGroup * 8 % 6 > 0)
		CharCount = (uint8) (BytesInGroup * 8 / 6 + 1);
	else
		CharCount = (uint8) (BytesInGroup * 8 / 6);
	
	/* encode each 6-bits of input data to one character, starting
	   from bit23.  bit0 is the right most bit */
	for (i=0, pos=23; i<CharCount; i++) {
		value = (uint8) islutil_GetBits(lData, pos, 6);
		EncryptedData[i] = islutilEncodeValue(value);
		pos-=6;
	}

	/* pad with '=' on the right if the number of characters encoded are fewer than
	   4 characters */
	for (i=CharCount; i<4; i++)
		EncryptedData[i] = '=';
}


uint32 IslUtil_Base64EncodeSize(ISL_DATA InputData)
{
	uint32 NumberOfGroup = 0;
	uint32 Length;
	uint8 RemainingBytes = 0;

	/* calculate how many 3-byte (24-bit) groups are there in the input data */
	NumberOfGroup = InputData.Length / MAXBYTES;

	/* calculate the number of bytes in the last group.  If there are fewer than 3 bytes in
	   the last group, pad zero bits to the right to form the last group */
	RemainingBytes =  (uint8) (InputData.Length % MAXBYTES);
	if (RemainingBytes > 0)
		NumberOfGroup += 1;
	else
		RemainingBytes = MAXBYTES;

	/* allocate an output buffer -- each 24-bits group of the input will
	   need 4 bytes of output buffer.  Also, add a "\r\n" after 64 bytes */
	Length = ((NumberOfGroup << 2) + (NumberOfGroup >> 4)) << 1;

	return Length;
}

/*-----------------------------------------------------------------------------
 * Name: IslUtil_Base64Encode
 *
 * Description:
 * This function performs base64 encoding
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS 
IslUtil_Base64Encode(
	ISL_DATA InputData, 
	ISL_DATA_PTR pOutputBuff)
{
	char EncryptedData[4];
	uint32 NumberOfGroup = 0;
	uint32 Length;
	uint8 RemainingBytes = 0;
	uint32 i, j, k;

	Length = IslUtil_Base64EncodeSize(InputData);

	if (pOutputBuff == NULL ||
		pOutputBuff->Data == NULL ||
		pOutputBuff->Length != Length ||
		InputData.Length == 0 ||
		InputData.Data == NULL)
		return ISL_FAIL;

	/* calculate the number of bytes in the last group.  If there are fewer 
       than 3 bytes in the last group, pad zero bits to the right to form 
       the last group */
	NumberOfGroup = InputData.Length / MAXBYTES;
	RemainingBytes =  (uint8) (InputData.Length % MAXBYTES);
	if (RemainingBytes > 0)
		NumberOfGroup += 1;
	else
		RemainingBytes = MAXBYTES;
	
	/* encode each 24-bits group of the input data and store the
	   encoded data to the output buffer.*/
	for (i=0, j=0, k=1; k<=NumberOfGroup; k++) {
		if (k == NumberOfGroup)
        {
			islutil_EncodeGroup(
                &InputData.Data[i],
                RemainingBytes,
                EncryptedData);
        }
		else
        {
			islutil_EncodeGroup(&InputData.Data[i], MAXBYTES, EncryptedData);
        }
		/* copy 4 bytes of encoded data to the output buffer */ 
		cssm_memcpy(&pOutputBuff->Data[j], &EncryptedData[0], 4);
		i+=3;
		j+=4;

		/* add '\r\n' characters after every 64 characters are encoded */
		if ((k & 0xf) == 0) {
			pOutputBuff->Data[j] = '\r';
			j++;
			pOutputBuff->Data[j] = '\n';
			j++;
		}

	}
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: IslUtil_Base64DecodeSize
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
uint32 IslUtil_Base64DecodeSize(ISL_DATA_PTR pEncodedData)
{
	uint32 i;
	uint32 NumberOfChars = 0;

	/* calculate how many valid charcaters are there in the encoded data */
	i = 0;
	while (i < pEncodedData->Length && pEncodedData->Data[i] != '=') {
		if ((pEncodedData->Data[i] >= 'A' && pEncodedData->Data[i] <= 'Z') ||
			 (pEncodedData->Data[i] >= 'a' && pEncodedData->Data[i] <= 'z') ||
			 (pEncodedData->Data[i] >= '0' && pEncodedData->Data[i] <= '9') ||
			 (pEncodedData->Data[i] == '+') || (pEncodedData->Data[i] == '/')) 
        {
				NumberOfChars++;
				i++;
		}
		else if((pEncodedData->Data[i] == '\n') ||
                (pEncodedData->Data[i] == '\r'))
        {
			i++;
        }
		else
        {
			return 0; /* Encoded data contains invalid character */
        }
	}

	/* allocate the output buffer, each character will need 6-bits 
       output buffer */
	return (NumberOfChars * 6 / 8); 	
}

/*-----------------------------------------------------------------------------
 * Name: IslUtil_Base64Decode
 *
 * Description:
 * This function performs base64 decoding
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS IslUtil_Base64Decode(
	ISL_DATA_PTR pEncodedData,
	ISL_DATA_PTR pDecodedData)
{
	uint32 i;
	uint32 NumberOfChars = 0;
	uint32 j, l;
	int pos;

	if (pDecodedData == NULL ||
		pEncodedData == NULL)
	{
		return ISL_FAIL;
	}

	NumberOfChars = IslUtil_Base64DecodeSize(pEncodedData);
	if (NumberOfChars > pDecodedData->Length) {
		return ISL_FAIL;
	}

	j= 0; 
	i= 0;
	/* decode input data */
	while ((i < pEncodedData->Length) && (pEncodedData->Data[i] != '=')) {
		uint32 temp = 0;
		uint32 value = 0;
		uint32 k = 0;
		uint32 NumberOfBytesUsed = 0;
		uint32 BitsToShift = 0;

		/* decode 4 characters (each character will be decoded to
		   a 6-bits group) and store the 24-bits in the right most bits of
		   uint32 */
		while (k < 4) {
			if ((pEncodedData->Data[i] != '\n') && 
                (pEncodedData->Data[i] != '\r')) 
            {
				value <<= 6;
				temp = islutilDecodeChar(pEncodedData->Data[i]);
				value |= temp;
				k++;
			}
			i++;
			if (pEncodedData->Data[i] == '=')
				break;
		
		}
	
		
	 	/* if less than 4 characters were decoded, shift the value to the
		   left up to the 2nd byte of uint32 and pad the rest bits with 
           zero's */
		BitsToShift = 24 - k * 6 ;
		value <<= BitsToShift; 

		/* calculate the number of bytes in the decoded value that
		   we need to copy to the output buffer */
		NumberOfBytesUsed = k * 6 / 8;

		/* copy decoded value to the output buffer starting
		   from the 2nd bytes of decoded value */
		pos = 23;
		for (l=0; l<NumberOfBytesUsed; l++) {
			pDecodedData->Data[j] = (uint8) islutil_GetBits(value, pos, 8);
			pos -= 8;
			j++;
		}
	
	}

	return ISL_OK;
}


