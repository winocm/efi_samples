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

/* Module name: rem.c
   Computes the remainder of division a by b
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

#define ShiftRight(m) { posb=m.length-1;\
                        for (posa=0; posa<posb; ++posa)\
                          m.value[posa]=\
                            (m.value[posa]>>1)|(m.value[posa+1]<<31);\
                        m.value[posb]>>=1;\
                        if (m.value[posb]==0 && posb>=1)\
                          m.length=posb;}

void __stdcall ICL_Rem (RSAData *a, RSAData *b, RSAData *r)
{
  register int	posa, posb;
  int		worddiff, bdiff;
  RSAData	sb, rem;
  ICLWord	sbarray[2*MODULUS+2], remarray[2*MODULUS+2];
  ICLWord	msw, *bval;

  if (ICL_Compare (a, b)<0) {    /* a = a mod b   already */
    bval = a->value;
    for (posa=a->length-1; posa>=0; --posa)
      r->value[posa] = bval[posa];
    r->length = a->length;
    for (posa=a->length; posa<MODULUS; ++posa)  /* fill upper words with 0 */
        r->value[posa] = 0;
    return;
  }
  rem.value = remarray;
  sb.value = sbarray;

/* sbval will contain the shifted b-value thru iterations
   rem contains the computed partial remainder
*/
  bval=a->value;
  for (posa=a->length-1; posa>=0; --posa)
    remarray[posa]=bval[posa];

/* find the difference btw a and b in bits */
  msw = remarray[(rem.length=a->length)-1];
  for (posa=0; msw!=0; ++posa)
    msw>>=1;
  msw = b->value[b->length-1];
  for (posb=0; msw!=0; ++posb)
    msw>>=1;
/* posa and posb indicate the actual bit indices of MSWs */
  worddiff = rem.length - b->length - (posa<posb);
  bdiff = (posa>=posb) ? posa-posb : (8*sizeof(ICLWord))+posa-posb;

/* OK, the difference is 32*worddiff + bdiff.
Now shift b and copy to sb */
  bval = b->value;
  if (bdiff==0) {
    for (posa=worddiff+(posb=b->length-1); posb>=0; --posb, --posa)
      sbarray[posa]=bval[posb];
    sb.length = rem.length;
  }
  else {
    for (posa=worddiff+(posb=b->length-1); posb>0; --posb, --posa)
      sbarray[posa]=(bval[posb] << bdiff) | (bval[posb-1] >> (32-bdiff));
    sbarray[worddiff] = bval[0] << bdiff;
    sbarray[rem.length]=0;
    sbarray[b->length+worddiff] = bval[b->length-1] >> (32-bdiff);
    sb.length = rem.length + (sbarray[rem.length]!=0);
  }

/* fill the lower words with 0 */
  for (posa=worddiff-1; posa>=0; --posa)
    sbarray[posa]=0;

/* OK, now start the reduction loop */
  worddiff=(worddiff << 5) + bdiff;     /* number of bits to scan */
  while (worddiff-->=0) {
    if (ICL_Compare (&rem, &sb)>=0)
      ICL_Subtract (&rem, &sb);
    ShiftRight (sb);
  }

/* copy the remainder in rem to r */
  bval = r->value;
  for (posa=0; posa<rem.length; ++posa)
    bval[posa] = remarray[posa];
  r->length = rem.length;

/* fill the upper words with 0's */
  while (posa<MODULUS)
    bval[posa++]=0;
}

void __stdcall RSA_Rem (RSAData *a, RSAData *b, RSAInt *r)
{
  register int	posa, posb;
  int		worddiff, bdiff;
  RSAData	sb, rem;
  ICLWord	sbarray[2*MODULUS+2], remarray[2*MODULUS+2];
  ICLWord	msw, *bval;

  if (ICL_Compare (a, b)<0) {    /* a = a mod b   already */
    bval = a->value;
    for (posa=a->length-1; posa>=0; --posa)
      r->value[posa] = bval[posa];
    r->length = a->length;
    for (posa=a->length; posa<MODULUS; ++posa)  /* fill upper words with 0 */
        r->value[posa] = 0;
    return;
  }
  rem.value = remarray;
  sb.value = sbarray;

/* sbval will contain the shifted b-value thru iterations */
  bval=a->value;
  for (posa=a->length-1; posa>=0; --posa)
    remarray[posa]=bval[posa];
  rem.length=a->length;

/* find the difference btw a and b in bits */
  msw = remarray[rem.length-1];
  for (posa=0; msw!=0; ++posa)
    msw>>=1;
  msw = b->value[b->length-1];
  for (posb=0; msw!=0; ++posb)
    msw>>=1;
  worddiff = rem.length - b->length - (posa<posb);
  bdiff = (posa>=posb) ? posa-posb : 32+posa-posb;

/* OK, the difference is 32*worddiff + bdiff. Now shift b and copy to sb */
  bval = b->value;
  if (bdiff==0) {
    for (posa=worddiff+(posb=b->length-1); posb>=0; --posb, --posa)
      sbarray[posa]=bval[posb];
    sb.length = rem.length;
  }
  else {
    for (posa=worddiff+(posb=b->length-1); posb>0; --posb, --posa)
      sbarray[posa]=(bval[posb] << bdiff) | (bval[posb-1] >> (32-bdiff));
    sbarray[worddiff] = bval[0] << bdiff;
    sbarray[rem.length]=0;
    sbarray[b->length+worddiff] = bval[b->length-1] >> (32-bdiff);
    sb.length = rem.length + (sbarray[rem.length]!=0);
  }

/* fill the lower words with 0 */
  for (posa=worddiff-1; posa>=0; --posa)
    sbarray[posa]=0;

/* OK, now start the reduction loop */
  worddiff=(worddiff << 5) + bdiff;     /* number of bits to scan */
  while (worddiff-->=0) {
    if (ICL_Compare (&rem, &sb)>=0)
      ICL_Subtract (&rem, &sb);
    ShiftRight (sb);
  }

/* copy the remainder in rem to r */
  bval = r->value;
  for (posa=0; posa<rem.length; ++posa)
    bval[posa] = remarray[posa];
  r->length = rem.length;

/* fill the upper words with 0's */
  while (posa<MODULUS)
    bval[posa++]=0;
}
