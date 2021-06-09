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

/* Module name: compare.c
   Compares two RSAData integers. The return value is
   -1     a < b
   0      a == b
   1      a > b

   RSAData numbers are assumed to be normalized, i.e. the most significand
   word is not zero.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

int __stdcall ICL_Compare (RSAData *a, RSAData *b)
{
  register long	alen, blen;

  alen=a->length;
  blen=b->length;
  if (alen > blen) return 1;
  if (alen < blen) return -1;
  while (alen-->0) {
    if (a->value[alen] > b->value[alen]) return 1;
    if (a->value[alen] < b->value[alen]) return -1;
  }
  return 0;
}
