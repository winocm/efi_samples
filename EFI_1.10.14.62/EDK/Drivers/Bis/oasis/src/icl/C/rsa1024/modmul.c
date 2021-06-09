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

/* Module name: modmul.c
   Computes t = a * b mod n
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"


void __stdcall ICL_ModMul (RSAData *a, RSAData *b, RSAData *n, RSAData *t)
{
  register int	cnt, k;
  ICLWord	*bval, bi, bitmask;
  ICLWord	uarray[MODULUS+1];
  RSAData	u;

/* u = 0 */
    u.length = 1;
    u.value = uarray;
    for (cnt=MODULUS; cnt>=0; --cnt)
        uarray[cnt] = 0;
    bval = b->value;

/* start the main loop */
    for (cnt=b->length-1; cnt>=0; --cnt) {
        bi = bval[cnt];
        for (bitmask=(ICLWord)1<<(WORDSIZE-1); bitmask!=0; bitmask>>=1) {
/* shl u,1 */
            uarray[u.length] = 0;
            for (k=u.length-1; k>=0; --k) {
                uarray[k+1] |= uarray[k] >> (WORDSIZE-1);
                uarray[k] <<= 1;
            }
            if (uarray[u.length])   ++u.length;

/* if u>=n then u = u - n */
            if (ICL_Compare (&u, n)>=0)
                ICL_Subtract (&u, n);

/* u = u+bi*a */
            if (bitmask & bi) {
                ICL_Add (&u, a, &u);
                while (u.length>1 && uarray[u.length-1]==0)
                    --u.length;
            }
/* if u>=n then u = u - n */
            if (ICL_Compare (&u, n)>=0)
                ICL_Subtract (&u, n);
        }
    }

/* copy u to t and adjust length */
    bval = t->value;
    for (cnt=u.length; cnt>=0; --cnt)
        bval[cnt] = uarray[cnt];
    for (cnt=u.length-1; bval[cnt]==0; --cnt) ;
    t->length = cnt+1;
}
