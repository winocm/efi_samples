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

/* Module name: iclproc.h
   Header file for miscellenaous functions.
*/

#if !defined(ICL_PROC_INCLUDE)
#define ICL_PROC_INCLUDE	1

/* Number of words required to store, "ByteLength" bytes */
#define	ICL_WordCount(ByteLength)	( ((ByteLength)+sizeof(ICLWord)-1) / sizeof(ICLWord) )
#define ICL_IsOne(a)	( (a).length==1 && (a).value[0]==1 )
#define ICL_IsZero(m)	( (m).length==1 && (m).value[0]==0 )
#define ICL_IsEven(m)	( ((m).value[0] & 1)!=1 )

#ifdef __cplusplus
extern "C" {
#endif

void __stdcall	ICL_MoveWord
										(RSAData	*src,
					 					RSAData		*dest);

void __stdcall	ICL_MoveByte			(ICLData	*src,
										ICLData		*dest);

void __stdcall	ICL_MoveByte2Word		(ICLData	*src,
										RSAData		*dest);

void __stdcall	ICL_MoveWord2Byte		(RSAData	*src,
										ICLData		*dest);

void __stdcall	ICL_PadBytes0			(RSAData	*a,
										long		ByteLength);

int __stdcall	ICL_ByteCount			(RSAData	*a);

#ifdef __cplusplus
}
#endif


#endif	/* ICL_PROC_INCLUDE */
