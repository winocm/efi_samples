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

extern void __stdcall ICL_SHATransform(ICLWord *state,ICLWord *buffer);

int ICL_SHAProcess(ICLData *Message,ICLSHAState *SHAState)
{
 long int index,i,partlen,j;
 ICLWord k;
 
 index = (SHAState->count[0]>>3)& 0x3F;
 
/* Add length of message to the counter*/
 k = Message->length <<3;
 SHAState->count[0] +=k;
 if (SHAState->count[0] < k )  
      SHAState->count[1]++; 

 SHAState->count[1] += Message->length >>29;
 
 partlen = 64 - index;
 
 if (Message->length >= partlen)
 {
   for (i=0;i<partlen;i++)
        SHAState->buffer[i+index]=Message->value[i];

   ICL_SHATransform(SHAState->state, (ICLWord *)SHAState->buffer);

   for (i=partlen;i<Message->length-63;i+=64)
    ICL_SHATransform(SHAState->state, (ICLWord *)&Message->value[i]);
   index=0;    
 }
 else i=0;

  for (j=0;j<Message->length-i;j++)
     SHAState->buffer[index+j] = Message->value[i+j];
     
         
 return(0);
}
