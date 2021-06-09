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

/* Module name: rsakg.h
   Header file for RSA Key Generation routines and variables
*/

#if !defined(ICL_RSAKG_INCLUDE)
#define ICL_RSAKG_INCLUDE	1


/* Maximum number of bases to use in weak primality test */
#define	MAXNUMBEROFBASES                64

/* The small prime list to be used in trial division step */
extern ICLWord	PrimeList[];

#ifdef __cplusplus
extern "C" {
#endif

int __stdcall	ICL_TrialDivision
					(RSAData	*PseudoPrime);

int __stdcall	ICL_WeakPseudoPrimality
					(RSAData	*PseudoPrime,
 					 int		Count);

void __stdcall	ICL_NextOdd
					(RSAData	*Odd);

void __stdcall	ICL_ModularInverse
					(ICLData	*Number,
					ICLData		*Modulus,
					ICLData		*Inverse);

void __stdcall	ICL_EEA
					(RSAData	*a,
 					RSAData		*b,
				 	RSAData		*vpp,
 					RSAData		*upp,
 					RSAData		*rpp,
 					int			*k);

void __stdcall	ICL_Div
					(RSAData	*Dividend,
					RSAData		*Divisor,
					RSAData		*Divide);

void __stdcall	ICL_GCD
					(RSAData	*a,
					RSAData		*b,
					RSAData		*gcd);

int __stdcall	ICL_FindPrime
					(RSAData	*PseudoPrime);

void __stdcall	ICL_FindPublicExponent
					(RSAData	*PublicExpCandidate,
					RSAData		*PhiN);

int __stdcall	RSA_Remainder
					(RSAData	*a,
					ICLWord     b);

#ifdef __cplusplus
}
#endif

#endif

