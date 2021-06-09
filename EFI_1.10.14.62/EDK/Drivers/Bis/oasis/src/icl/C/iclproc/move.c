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

/*	Module name: move.c
	Move related functions.
*/

#include <efi.h>
#include <efidriverlib.h>
#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/iclproc.h"

//pause(char*);
int DbgPrint( int, char*, ... );	 

#define TraceLine() DbgPrint(-1,"move l# %d\n", __LINE__)


void __stdcall ICL_Padding0 (RSAData *a);


/* copies src to dest and fills the higher order words of dest with 0's */
void __stdcall	ICL_MoveWord (RSAData *src, RSAData *dest)
{
  register long	cnt, length=src->length;
  ICLWord	*dval, *sval, anICLWord;

  dval = dest->value;
  sval = src->value;
  for (cnt=0; cnt<length; ++cnt)
  {
     EfiCopyMem(&anICLWord, &sval[cnt], sizeof(ICLWord));
     dval[cnt] = anICLWord;
  }
/* pad the higher leading words to 0 */
  while (cnt<MODULUS)
    dval[cnt++]=0;
  dest->length = length;
}


/* copies src to dest, both are byte-length data, donot fill the upper words with 0's */
void __stdcall ICL_MoveByte (ICLData *src, ICLData *dest)
{
  register long	cnt=src->length;
  ICLByte	*dval, *sval;

  dval = dest->value;
  sval = src->value;
  dest->length = cnt;
  while (cnt-->0)
    dval[cnt] = sval[cnt];
}


/* copies src to dest and fills the higher order words of dest with 0's */
void __stdcall	ICL_MoveByte2Word (ICLData *src, RSAData *dest)
{
  register long	cnt, length;
  ICLWord	*dval, anIclword;
  ICLByte	*sval;

  dval = dest->value;		/* word aligned */
  sval = src->value;		/* byte aligned */
  length = cnt = src->length/sizeof (ICLWord);	/* Word.length */

/* copy the words except the highest one */
  
  while (cnt-- >0)
  {  
      EfiCopyMem( &anIclword, (void *) &(((ICLWord *)sval)[cnt]), sizeof(ICLWord));
    dval[cnt] = anIclword;
  }
  dest->length = length;

/* copy the last word. cnt is the number of bits to pad */
  cnt = (sizeof (ICLWord) - (src->length & 0x03)) * 8;	/* 32, 24, 26, 8 */
  if (cnt<32) {			/* if the last word is not fully used */
    dval[length] = ((ICLWord *)sval)[length] & ((~(ICLWord)0) >> cnt);
    dest->length++;
  }
  ICL_Padding0 (dest);
}

/* copies src to dest */
void __stdcall	ICL_MoveWord2Byte (RSAData *src, ICLData *dest)
{
  register long length;
  ICLWord	*sval;
  ICLByte	*dval;

  dval = dest->value;		/* byte aligned */
  sval = src->value;		/* word aligned */
/* number of bytes except the 0's in the higher bytes */
  dest->length = length = ICL_ByteCount (src);
  while (length-->0)
    dval[length] = ((ICLByte *)sval)[length];
}

