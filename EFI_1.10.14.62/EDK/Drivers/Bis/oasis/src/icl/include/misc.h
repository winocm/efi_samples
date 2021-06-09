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

// returns 2^p
#define ICLPower2(p) ((ICLWord)1<<(p))

// bitwise rotation to the left
#define rotl(x,n)   (((x)>>(32 - (n))) | ((x) << (n)))

// bitwise rotation to the right
#define rotr(x,n)   (((x)<<(32 - (n))) | ((x) >> (n)))

// rotation to the right, size specific, with masking
#define Srotr(x,n,bits)   (((x) << ((bits) - ((n)&((bits)-1))) ) | ((x) >> ((n)&((bits)-1))))

// rotation to the left, size specific, with masking
#define Srotl(x,n,bits)   (((x) >> ((bits) - ((n)&((bits)-1))) ) | ((x) << ((n)&((bits)-1))))

// translates little endian <----> big endian
#define bswap(y)   ((rotr(y, 8) & 0xff00ff00) |  \
                   (rotl(y, 8) & 0x00ff00ff))
   

// prototypes for misc.c in ICLProc directory

void __stdcall ICL_memoryset(char *addr, char value, long length);
void __stdcall ICL_memorycopy(char *dest, char *src, long length);
