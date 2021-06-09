/*

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

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
 *  INTEL CONFIDENTIAL
 *  This file, software, or program is supplied under the terms
 *  of a licence agreement or nondisclosure agreement with
 *  Intel Corporation and may not be copied or disclosed except
 *  in accordance with the terms of that agreement. This file,
 *  software, or program contains copyrighted material and/or
 *  trade secret information of Intel Corporation, and must be
 *  treated as such. Intel reserves all rights in this material,
 *  except as the licence agreement or nondisclosure agreement
 *  specifically indicate.
*/

/*	Module name: bytes.c
	Byte/Word related functions.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/iclproc.h"

/* pads the given structures unused high words with 0's to upto MODULUS words */
void __stdcall ICL_Padding0 (RSAData *a)
{
	long	cnt;
	ICLWord	*aval;

	cnt = a->length;
	aval = &a->value[cnt];
	while (cnt++<MODULUS)
	  *(aval++) = 0;
}


/* pads the excess bytes of RSAData whose size is "ByteLength" bytes.
   RSAData.length is not modified.
*/
void __stdcall	ICL_PadBytes0 (RSAData *a, long ByteLength)
{
	ICLWord	*aval;

	aval = &a->value[ICL_WordCount (ByteLength)];
/* Word.length */
	ByteLength = (sizeof (ICLWord) - (ByteLength & 0x03)) * 8;	/* 32, 24, 26, 8 */
/* if the last word is not fully used, strip off the excess bytes */
	if (ByteLength<32)
	  *aval = *aval & ( (~(ICLWord)0) >> ByteLength );
}


/* the exact number of bytes to store this word-aligned data */
int __stdcall ICL_ByteCount (RSAData *a)
{
	register long	length;
	ICLWord			sval;

/* the base length. Note that, a->length is word.length */
	length = (a->length-1) * sizeof (ICLWord);
	sval = a->value[a->length-1];
/* if there are excess bytes, then add them to the base length */
	if (sval & 0xFF000000)		length += 4;
	else if (sval & 0x00FF0000)	length += 3;
	else if (sval & 0x0000FF00)	length += 2;
	else if (sval & 0x000000FF)	length += 1;
	return length;
}
