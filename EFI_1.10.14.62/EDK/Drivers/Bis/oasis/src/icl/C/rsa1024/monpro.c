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

/* Module name: monpro.c
   Montgomery Product (The m-ary add-shift method)
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

#define LOWWORDMASK         ( ~(ICLWord)0 >> (WORDSIZE/2) )


/* This function reads n0prime and the modulus n from global variables
   ModExpN0prime and ModExpModulus, thereby eliminating passing the
   same variable over and over again for function call overhead.
*/
// The folling #ifndef has been modified from the original. If the msvc version
// of the function is used, it results in the needed symbol _allmul. We were
// not able to get the neccesary library linked in so we are using the other 
// version
//#ifndef _MSC_VER
#ifndef BOBALOO_12313123111
void __stdcall MontProduct (ICLWord ModExpN0prime, RSAData ModExpModulus,
	RSAData *a, RSAInt *b, RSAData *t)
{
  register long	i, j;
  ICLWord	*tval, *aval, *bval, ai, m, C, S, sum, carry;
  ICLWord	a0b0, a0b1, a1b0, a1b1, mlow, mhigh;

  tval = t->value;
  aval = a->value;
  bval = b->value;
  t->length = ModExpModulus.length;

/* initialize t with zero, max(t.length) = s+1 words */
  for (i=MODULUS+1; i>=0; --i)
    tval[i]=0;

/* go into multiplication loop, by shifting at the same time */
  for (i=0; i<ModExpModulus.length; ++i) {
    ai=aval[i];
    C = 0;
    for (j=0; j<ModExpModulus.length; ++j) {
/* (C,S):=t[j] + b[j]*a[i] + C */
      a0b0 = (LOWWORDMASK & ai) * (LOWWORDMASK & bval[j]);
      a0b1 = (LOWWORDMASK & ai) * (bval[j] >> (WORDSIZE/2));
      a1b0 = (ai >> (WORDSIZE/2)) * (LOWWORDMASK & bval[j]);
      a1b1 = (ai >> (WORDSIZE/2)) * (bval[j] >> (WORDSIZE/2));
      sum=(a0b0 & LOWWORDMASK)+(C & LOWWORDMASK)+(tval[j] & LOWWORDMASK);
      carry=(a0b0 >> (WORDSIZE/2))+(a0b1 & LOWWORDMASK)+(a1b0 & LOWWORDMASK)+
        (C >> (WORDSIZE/2))+(tval[j] >> (WORDSIZE/2))+(sum >> (WORDSIZE/2));
      S=(sum & LOWWORDMASK)+(carry << (WORDSIZE/2));
      C=a1b1+(a0b1 >> (WORDSIZE/2))+(a1b0 >> (WORDSIZE/2))+(carry >> (WORDSIZE/2));
/* t[j]:=S */
      tval[j]=S;
    }

/* (C,S):=t[s] + C */
    sum=(C & LOWWORDMASK) + (tval[ModExpModulus.length] & LOWWORDMASK);
    carry=(C >> (WORDSIZE/2)) + (tval[ModExpModulus.length] >> (WORDSIZE/2)) +
        (sum >> (WORDSIZE/2));
/* t[s]:=S;   t[s+1]:=C */
    tval[ModExpModulus.length]=(sum & LOWWORDMASK) + (carry << (WORDSIZE/2));
    tval[ModExpModulus.length+1]=(carry >> (WORDSIZE/2));

/* m = t[0] * n0prime  */
    m = tval[0] * ModExpN0prime;
    mlow = m & LOWWORDMASK;
    mhigh = m >> (WORDSIZE/2);

/* (C,S):=m*n[0]+ t[0]     discard sum, it must be 0 */
    a0b0 = mlow * (LOWWORDMASK & ModExpModulus.value[0]);
    a0b1 = mlow * (ModExpModulus.value[0] >> (WORDSIZE/2));
    a1b0 = mhigh * (LOWWORDMASK & ModExpModulus.value[0]);
    a1b1 = mhigh * (ModExpModulus.value[0] >> (WORDSIZE/2));
    sum=(a0b0 & LOWWORDMASK)+(tval[0] & LOWWORDMASK);
    carry=(a0b0 >> (WORDSIZE/2))+(a0b1 & LOWWORDMASK)+(a1b0 & LOWWORDMASK)+
      (tval[0] >> (WORDSIZE/2))+(sum >> (WORDSIZE/2));
    C=a1b1 + (a1b0 >> (WORDSIZE/2))+(a0b1 >> (WORDSIZE/2))+(carry >> (WORDSIZE/2));

    for (j=1; j<ModExpModulus.length; ++j) {
/* (C,S):=t[j] + m*n[j] + C */
      a0b0 = mlow * (LOWWORDMASK & ModExpModulus.value[j]);
      a0b1 = mlow * (ModExpModulus.value[j] >> (WORDSIZE/2));
      a1b0 = mhigh * (LOWWORDMASK & ModExpModulus.value[j]);
      a1b1=  mhigh * (ModExpModulus.value[j] >> (WORDSIZE/2));

      sum=(a0b0 & LOWWORDMASK)+(C & LOWWORDMASK)+(tval[j] & LOWWORDMASK);
      carry = (a0b0 >> (WORDSIZE/2))+(a0b1 & LOWWORDMASK)+(a1b0 & LOWWORDMASK)+
        (C >> (WORDSIZE/2))+(tval[j] >> (WORDSIZE/2))+(sum>>(WORDSIZE/2));
      S = (sum & LOWWORDMASK)+(carry << (WORDSIZE/2));
      C = a1b1+(a0b1 >> (WORDSIZE/2))+(a1b0 >> (WORDSIZE/2))+(carry >> (WORDSIZE/2));
/* t[j-1]:=S */      
      tval[j-1]=S;
    }
/* (C,S):=t[s] + C */
    sum=(tval[ModExpModulus.length] & LOWWORDMASK) + (C & LOWWORDMASK);
    carry = (tval[ModExpModulus.length] >> (WORDSIZE/2)) + (C >> (WORDSIZE/2)) +
        (sum >> (WORDSIZE/2));
/* t[s-1]:=S */
    tval[ModExpModulus.length-1] = (sum & LOWWORDMASK) + (carry << (WORDSIZE/2));
/* t[s]:=t[s+1] + C */
    tval[ModExpModulus.length] = tval[ModExpModulus.length+1] + (carry >> (WORDSIZE/2));
  }
  tval[ModExpModulus.length+1] = 0;
/* do not discard the leading zero words! */
  i = ModExpModulus.length;
/* now, t.length==n.length */
  if (tval[i]==0)
    while (i-->0) {
      if (tval[i] < ModExpModulus.value[i])  return;
      if (tval[i] > ModExpModulus.value[i])  break;
    }
/* the t->value[ModExp.length] word must be zero */
  tval[ModExpModulus.length]=0;
/* Note that,if a subtraction is needed, t.length=n.length */
  C=1;     /* initial carry is 1 */
  for (i=0; i<ModExpModulus.length; ++i) {
    S = (tval[i] & (~(ICLWord)0 >> 1)) + ((m=~ModExpModulus.value[i]) & (~(ICLWord)0 >> 1)) + C;
    carry=(tval[i] >> (WORDSIZE-1)) + (m >> (WORDSIZE-1)) + (S >> (WORDSIZE-1));
    C = (carry & 2)!=0;
    tval[i] = (carry & 1) ? (S | ((ICLWord)1 << (WORDSIZE-1))) : (S & (~(ICLWord)0 >> 1));
  }
}

#else     /* if this is Microsoft Visual C/C++ compiler */
void __stdcall MontProduct (ICLWord ModExpN0prime, RSAData ModExpModulus,
	RSAData *a, RSAInt *b, RSAData *t)
{
  register long	i, j;
  ICLWord	*tval, *aval, *bval, ai, m, C, S, carry;
  ICLDWord	CS;

  tval = t->value;
  aval = a->value;
  bval = b->value;
  t->length = ModExpModulus.length;

/* initialize t with zero, max(t.length) = s+1 words */
  for (i=MODULUS+1; i>=0; --i)
    tval[i]=0;

/* go into multiplication loop, by shifting at the same time */
  for (i=0; i<ModExpModulus.length; ++i) {
    ai=aval[i];
    C = 0;
    for (j=0; j<ModExpModulus.length; ++j) {
/* (C,S):=t[j] + b[j]*a[i] + C;		t[j]:=S */
	  CS = (ICLDWord)bval[j]*ai + (ICLDWord)C + (ICLDWord)tval[j];
      tval[j]=LowWord (CS);
	  C = HighWord (CS);
    }
/* (C,S):=t[s] + C;		t[s]:=S;	t[s+1]:=C */
	CS = (ICLDWord)tval[ModExpModulus.length] + (ICLDWord)C;
	tval[ModExpModulus.length] = LowWord (CS);
	tval[ModExpModulus.length+1] = HighWord (CS);

/* m = t[0] * n0prime  */
    m = tval[0]*ModExpN0prime;

/* (C,S):=m*n[0] + t[0]     discard sum, it must be 0 */
	CS = (ICLDWord)m*ModExpModulus.value[0] + (ICLDWord)tval[0];
	C = HighWord (CS);

    for (j=1; j<ModExpModulus.length; ++j) {
/* (C,S):=t[j] + m*n[j] + C;	t[j-1]:=S */
	  CS = (ICLDWord)m*ModExpModulus.value[j] + (ICLDWord)C + (ICLDWord)tval[j];
	  tval[j-1] = LowWord (CS);
	  C = HighWord (CS);
    }
/* (C,S):=t[s] + C;	t[s-1]:=S */
	CS = (ICLDWord)tval[ModExpModulus.length] + (ICLDWord)C;
	tval[ModExpModulus.length-1] = LowWord (CS);
	C = HighWord (CS);
/* t[s]:=t[s+1] + C */
	CS = (ICLDWord)tval[ModExpModulus.length+1] + (ICLDWord)C;
    tval[ModExpModulus.length] = LowWord (CS);
	tval[ModExpModulus.length+1] = HighWord (CS);
  }
  tval[ModExpModulus.length+1] = 0;
/* do not discard the leading zero words! */
  i = ModExpModulus.length;
/* now, t.length==n.length */
  if (tval[i]==0) /* if this is not 0, perform the subtraction */
    while (i-->0) {
      if (tval[i] < ModExpModulus.value[i])  return;
      if (tval[i] > ModExpModulus.value[i])  break;
    }
/* the t->value[ModExp.length] word must be zero */
  tval[ModExpModulus.length]=0;
/* Note that,if a subtraction is needed, t.length=n.length */
  C=1;     /* initial carry is 1 */
  for (i=0; i<ModExpModulus.length; ++i) {
    S = (tval[i] & (~(ICLWord)0 >> 1)) + ((m=~ModExpModulus.value[i]) & (~(ICLWord)0 >> 1)) + C;
    carry=(tval[i] >> (WORDSIZE-1)) + (m >> (WORDSIZE-1)) + (S >> (WORDSIZE-1));
    C = (carry & 2)!=0;
    tval[i] = (carry & 1) ? (S | ((ICLWord)1 << (WORDSIZE-1))) : (S & (~(ICLWord)0 >> 1));
  }
}

#endif  /* _MSC_VER */
