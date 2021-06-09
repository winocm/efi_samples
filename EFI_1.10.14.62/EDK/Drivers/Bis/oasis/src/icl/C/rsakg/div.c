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

/* Module name: div.c
   Multi-precision integer division.
   Only the quotient is returned.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/rsakg.h"

#define ShiftLeftOneBit(w) {register int cmp=w.length;\
			    w.value[cmp]=0;\
			    while (cmp-->0) {\
			      w.value[cmp+1]|=w.value[cmp] >> 31;\
			      w.value[cmp]   =w.value[cmp] << 1;\
			    }\
			    cmp=w.length;\
			    w.length=(w.value[cmp]!=0) ? cmp+1 : cmp;\
			   }

/* A simple restoring division */
void __stdcall	ICL_Div (RSAData *dividend, RSAData *divisor, RSAData *divide)
{
  register ICLWord	bitmask;
  register int		cnt, blength;
  RSAData		rem;
  ICLWord		*tval, *aval, remarray[MODULUS];

  blength=divisor->length;
/* Save time: if dividend<divisor then the result is obviously zero! */
  tval=divide->value;
  divide->length=1;
  for (cnt=0; cnt<MODULUS; ++cnt)
    tval[cnt] = 0;
  if (dividend->length<blength) return;

/* start with zero remainder. Note that, Subtract routine *MUST* be able to
deal with denormalized numbers. "rem" is a denormalized 'zero'! */
  rem.value=remarray;
  rem.length=blength;
  aval = dividend->value;
  for (cnt=0; cnt<MODULUS; ++cnt)
    remarray[cnt]=0;
/* start looping */
  for (cnt=dividend->length-1; cnt>=0; --cnt) {
    tval[cnt]=0;
    for (bitmask=0x80000000; bitmask!=0; bitmask>>=1) {
      remarray[0]|= (aval[cnt] & bitmask)!=0;     /* get the next a bit  */
      tval[cnt]<<=1;                   /* adjust the quotient             */
      if (ICL_Compare (&rem, divisor)>=0) {   /* subtract and shift left         */
	    ICL_Subtract (&rem, divisor);
	    tval[cnt] |= 1;                /* it divides! put 1 into quotient */
      }
      ShiftLeftOneBit (rem);           /* and shift partial remainder     */
    }
  }
/* This function discards the remainder. */
  cnt=dividend->length-1;
/* discard the leading 0's and find the normalized length of the result */
  while (cnt>0 && tval[cnt]==0)
    --cnt;
  divide->length=cnt+1;
}
