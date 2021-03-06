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
     
/* Module name: add.c
   Computes t = a + b
     
   The length of the result, t, is set to MODULUS.
*/
     
#include "../../include/icl.h"
#include "../../include/rsa.h"
     
void __stdcall ICL_Add (RSAData *a, RSAData *b, RSAData *t) 
{
  register ICLWord     *aval, *bval, *tval; 
  register long          pos, length;
  unsigned long          sum, carry, msb;
     
  aval=a->value;
  bval=b->value;
  tval=t->value;
/* perform addition by using the smaller's length, then .... */
  length = (a->length > b->length) ? b->length : a->length; 
  carry = 0;
  for (pos=0; pos<length; ++pos) {
    sum=(aval[pos] & (WORDMASK >> 1)) +
          (bval[pos] & (WORDMASK >> 1)) + carry;
    msb=(aval[pos] >> (WORDSIZE-1)) + (bval[pos] >> (WORDSIZE-1)) +
      (sum >> (WORDSIZE-1));
    carry = (msb & 2)!=0;
    if (msb & 1)
      tval[pos]=sum | ((ICLWord)1 << (WORDSIZE-1));
    else
      tval[pos]=sum & (WORDMASK >> 1);
  }
/* just add the carry-out (0/1) for the residue, if any */
  if (a->length > b->length)
    length = a->length;
  else {
    length = b->length;
    aval = bval;
  }
  for (; pos<length; ++pos) {
    if (carry==0)
      tval[pos]=aval[pos];
    else {
      sum=(aval[pos] & (WORDMASK >> 1)) + carry; 
      msb=(aval[pos] >> (WORDSIZE-1)) + (sum >> (WORDSIZE-1)); 
      carry = (msb & 2)!=0;
      if (msb & 1)
     tval[pos]=sum | ((ICLWord)1 << (WORDSIZE-1));
      else
     tval[pos]=sum & (WORDMASK >> 1);
    }
  }
  tval[pos]=carry;
/* leading 0s are not discarded */
  while (pos>0 && tval[pos]==0)
      --pos;
  t->length = pos+1;
}
