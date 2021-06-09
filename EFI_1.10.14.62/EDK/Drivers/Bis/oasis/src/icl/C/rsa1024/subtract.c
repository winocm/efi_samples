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

/* Module name: subtract.c
   Computes t = t - a
   assuming t>=a
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

/* return t = t-a */
void __stdcall ICL_Subtract (RSAData *t, RSAData *a)
{
  int			pos;
  register ICLWord	*aval, *tval, msb, sum;
  ICLWord		aneg, carry;
  long			length;

  aval=a->value;
  tval=t->value;
  length = a->length;

  carry=1;
  for(pos=0; pos<length; ++pos) {
    sum=(tval[pos] & 0x7FFFFFFF) + ((aneg=~aval[pos]) & 0x7FFFFFFF) + carry;
    msb=(tval[pos] >> 31) + (aneg >> 31) + (sum >> 31);
    carry = (msb & 2)!=0;
    if (msb & 1)
      tval[pos]=sum | 0x80000000;
    else
      tval[pos]=sum & 0x7FFFFFFF;
  }
/* ... just add the carry-out (0/1) for the residue, if any */
  length = t->length;
  for (; pos<length; ++pos) {
    sum=(tval[pos] & 0x7FFFFFFF) + carry + 0x7FFFFFFF;
    msb=(tval[pos] >> 31) + (sum >> 31) + 1;
    carry = (msb & 2)!=0;
    if (msb & 1)
      tval[pos]=sum | 0x80000000;
    else
      tval[pos]=sum & 0x7FFFFFFF;
  }
/* compute the length of t */
  --pos;
  while (pos>0 && tval[pos]==0)
    --pos;
  t->length = pos + 1;
}
