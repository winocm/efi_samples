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

/* Module name: rsa.h
   Data and private function declarations for RSA.
*/

#ifndef _ICL_RSA_INCLUDE_
#define _ICL_RSA_INCLUDE_


/***************************************************************/
/* RSA Encryption/Decryption declarations                      */
/***************************************************************/

#ifdef  _MSC_VER
#define DWORDSIZE    64
typedef unsigned __int64 ICLDWord;
#endif

typedef struct {
	long	length;				/* number of ICLWords */
	ICLWord value[MODULUS];
} RSAInt;

typedef struct {
	long	length;
	ICLWord	*value;				/* number of words */
} RSAData;



#if defined(_MSC_VER)
#define LowWord(x)		( (ICLWord)(x) )
#define HighWord(x)		( (ICLWord)( ((x) >> WORDSIZE) & 0xffffffff ) )
#endif

#define WORDMASK ( ~(ICLWord)0 )

#ifdef __cplusplus
extern "C" {
#endif


void     __stdcall ICL_ModExp     (RSAData *a, RSAData *e, RSAData *n, RSAData *t);
void     __stdcall ICL_ModExpCRT  (RSAData *C, RSAData *dp, RSAData *dq,
						 			RSAData *p, RSAData *q, RSAData *coeff, RSAData *t);
void	__stdcall  ICL_ModExpBQH  (RSAData *a, RSAData *e, RSAData *n, RSAData *t);

ICLWord __stdcall N0Prime        (ICLWord n0);
void __stdcall    ICL_Add        (RSAData *a, RSAData *b, RSAData *t);
void __stdcall    ICL_Mul        (RSAData *a, RSAData *b, RSAData *t);
void __stdcall    ICL_ModMul     (RSAData *a, RSAData *b, RSAData *n, RSAData *t);

int  __stdcall    ICL_Compare    (RSAData *a, RSAData *b);
void __stdcall    ICL_ModAdd     (RSAData *a, RSAData *b, RSAData *n, RSAData *t);
void __stdcall    ICL_ModSub     (RSAData *a, RSAData *b, RSAData *n, RSAData *t);
void __stdcall    MontProduct    (ICLWord ModExpN0prime, RSAData ModExpModulus, 
								  RSAData *a, RSAInt *b, RSAData *t);
void __stdcall    MontSquare     (ICLWord ModExpN0prime, RSAData ModExpModulus,
								  RSAData *a, RSAData *t);
void __stdcall    ICL_Rem        (RSAData *a, RSAData *b, RSAData *r);
void __stdcall    RSA_Rem        (RSAData *a, RSAData *b, RSAInt *r);
void __stdcall    ICL_Subtract   (RSAData *t, RSAData *a);
void __stdcall    ICL_Padding0   (RSAData *a);

#ifdef __cplusplus
}
#endif


#endif      /* _ICL_RSA_INCLUDE_ */

