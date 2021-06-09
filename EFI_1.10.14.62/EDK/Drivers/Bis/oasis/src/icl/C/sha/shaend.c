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


#include "../../include/icl.h"

// bitwise rotation to the left
#define rotl(x,n)   (((x)>>(32 - (n))) | ((x) << (n)))

// bitwise rotation to the right
#define rotr(x,n)   (((x)<<(32 - (n))) | ((x) >> (n)))

// translates little endian <----> big endian
#define bswap(y)   ((rotr(y, 8) & 0xff00ff00) |  \
                   (rotl(y, 8) & 0x00ff00ff))
   


int ICL_SHAEnd(ICLSHAState *SHAState,ICLSHADigest digest)
{
 ICLWord APPEND_STR[16]={0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 ICLWord Length[2];
 ICLData dat;
 long int index,i;

/* Save length  */
/* In reverse order */ 
 Length[1]= bswap(SHAState->count[0]);
 Length[0]= bswap(SHAState->count[1]);

 dat.value = (ICLByte *) APPEND_STR;
 index = (SHAState->count[0] >> 3) & 0x3F;
 dat.length = (index<56) ? (56-index) : (120-index);
 
 ICL_SHAProcess(&dat,SHAState);
 dat.value  = (ICLByte * ) Length;
 dat.length = 8;
 ICL_SHAProcess(&dat,SHAState);

/* Print State vector to digest */

 ((ICLWord *) digest)[0] = bswap(SHAState->state[0]);
 ((ICLWord *) digest)[1] = bswap(SHAState->state[1]);
 ((ICLWord *) digest)[2] = bswap(SHAState->state[2]);
 ((ICLWord *) digest)[3] = bswap(SHAState->state[3]);
 ((ICLWord *) digest)[4] = bswap(SHAState->state[4]);


/* Clear state information */

 SHAState->state[0] =0;
 SHAState->state[1] =0;
 SHAState->state[2] =0;
 SHAState->state[3] =0;
 SHAState->state[4] =0;

 SHAState->count[0] =0;
 SHAState->count[1] =0;

 for (i=0;i<16;i++)
  ((ICLWord *) SHAState->buffer)[i] = 0UL;
 
 return(0);
}
