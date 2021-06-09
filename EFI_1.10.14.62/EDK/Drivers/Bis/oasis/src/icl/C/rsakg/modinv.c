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

/* Module name: modinv.c
   Modular Inverse Computation.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/rsakg.h"
#include "../../include/iclproc.h"

/* the modular inverse of "Number" is computed where the modulus is "Modulus"
   The inverse is located in "inverse".
   This function returns 0 on success, -1 on failure to find the inverse.
   length of input and output is byte based.
*/
void __stdcall	ICL_ModularInverse (ICLData *Number,
			ICLData *Modulus, ICLData *Inverse)
{
  int		k;
  long		NumberLength, ModulusLength;
  RSAData	s, t, g, *inv;
  ICLWord	sarray[MODULUS], tarray[MODULUS], garray[MODULUS];

/* value initialization */
  s.value=sarray;
  t.value=tarray;
  g.value=garray;

  inv = (RSAData *)Inverse;     /* an alias pointer for Inverse */

/* adjust the lengths of inputs to reflect their word numbers */
  NumberLength = Number->length;		/* first, save their lengths */
  ModulusLength = Modulus->length;

  Number->length = ICL_WordCount (NumberLength);
  Modulus->length = ICL_WordCount (ModulusLength);
/* pad any leading byte(s) */
  ICL_PadBytes0 ((RSAData *)Number, NumberLength);
  ICL_PadBytes0 ((RSAData *)Modulus, ModulusLength);


/* run the Extended Euclidean Algorithm */
  ICL_EEA ((RSAData *)Modulus, (RSAData *)Number, &s, &t, &g, &k);

  if ((g.length==1) && (g.value[0]==1)) {	/* if GCD(.)==1 */
    if ((k & 1)==0) {   /* if number of iterations is even, make a subtract */
      ICL_MoveWord ((RSAData *)Modulus, inv);
      ICL_Subtract (inv, &t);	/* Inverse = Modulus - t */
    }
    else
      ICL_MoveWord (&t, inv);
  }
  else {	/* cannot find an inverse for this modulus, since GCD(.)!=1 */
    Inverse->length=1;	/* return 0 to indicate this */
    Inverse->value[0]=0;
  }
/* fill the higher words with 0's */
  for (k=inv->length; k<MODULUS; ++k)
    inv->value[k] = 0;
/* and find the actual length */
  k=inv->length;
  while (k>1 && inv->value[k-1]==0)
    --k;
  inv->length = k;

/* restore the lengths of the input variables */
  Number->length = NumberLength;
  Modulus->length = ModulusLength;
/* find the byte-length of the Inverse */
  Inverse->length = ICL_ByteCount (inv);
}
