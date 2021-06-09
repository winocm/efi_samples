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

/* Module name: rsa.c
   RSA Public and Private Key Operations.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/iclproc.h"


void static FillWithZero (char *, long length);

int ICL_RSAPublicKeyOperation (ICLData             *Input,
							 	ICLRSAPublicKey    *RSAPublicKey,
								ICLData            *Output)
{
  RSAData	l_Input, l_Output, l_Exponent, l_Modulus;
  ICLWord	iarray[MODULUS], oarray[MODULUS],
  			earray[MODULUS], marray[MODULUS];

  l_Input.value = iarray;
  l_Output.value = oarray;
  l_Exponent.value = earray;
  l_Modulus.value = marray;

/* copy the byte aligned input numbers to local word sized full size var.s */
/* note that, zero padding is also done */
  ICL_MoveByte2Word (Input, &l_Input);
  ICL_MoveByte2Word (&RSAPublicKey->PublicExponent, &l_Exponent);
  ICL_MoveByte2Word (&RSAPublicKey->Modulus, &l_Modulus);

/* compute the exponentiation */
  ICL_ModExp (&l_Input, &l_Exponent, &l_Modulus, &l_Output);

  ICL_MoveWord2Byte (&l_Output, Output);

/* zeroize the sensitive information */  
  FillWithZero ((char *)iarray, MODULUSBYTES);
  FillWithZero ((char *)oarray, MODULUSBYTES);
  FillWithZero ((char *)marray, MODULUSBYTES);
  FillWithZero ((char *)earray, MODULUSBYTES);

  return 0;
}


int ICL_RSAPrivateKeyOperation (ICLData            *Input,
			 					ICLRSAPrivateKey   *RSAPrivateKey,
			 					ICLData            *Output)
{
	RSAData		l_Input, l_Output, l_Modulus, l_Exp0, l_Exp1,
        l_Coeff, l_Prime0, l_Prime1;
	ICLWord		iarray[MODULUS], oarray[MODULUS], marray[MODULUS],
				p0array[MODULUS], p1array[MODULUS],
				e0array[MODULUS], e1array[MODULUS], carray[MODULUS];

/* reserve memory space for the local variables' value arrays */
	l_Input.value = iarray;
	l_Output.value = oarray;
	l_Modulus.value = marray;
	l_Exp0.value = e0array;
	l_Exp1.value = e1array;
	l_Prime0.value = p0array;
	l_Prime1.value = p1array;
	l_Coeff.value = carray;

/* copy the input parameters to local ones and ... */
	ICL_MoveByte2Word (Input, &l_Input);
	ICL_MoveByte2Word (&RSAPrivateKey->PrimeExponent[0], &l_Exp0);
	ICL_MoveByte2Word (&RSAPrivateKey->PrimeExponent[1], &l_Exp1);
	ICL_MoveByte2Word (&RSAPrivateKey->Prime[0], &l_Prime0);
	ICL_MoveByte2Word (&RSAPrivateKey->Prime[1], &l_Prime1);
 	ICL_MoveByte2Word (&RSAPrivateKey->Coefficient, &l_Coeff);

/* compute the exponentiation using CRT */
	ICL_ModExpCRT (&l_Input, &l_Exp0, &l_Exp1, &l_Prime0, &l_Prime1, &l_Coeff, &l_Output);

/* copy the result to PlainText argument */
	ICL_MoveWord2Byte (&l_Output, Output);

/* zeroize the local variables */
    FillWithZero ((char *)iarray, MODULUSBYTES);
    FillWithZero ((char *)oarray, MODULUSBYTES);
    FillWithZero ((char *)marray, MODULUSBYTES);
    FillWithZero ((char *)e0array, MODULUSBYTES);
    FillWithZero ((char *)e1array, MODULUSBYTES);
    FillWithZero ((char *)p0array, MODULUSBYTES);
    FillWithZero ((char *)p1array, MODULUSBYTES);
    FillWithZero ((char *)carray, MODULUSBYTES);

	return 0;
}


void static FillWithZero (char *w, long length)
{
    while (length-->0)
        *(w++) = 0;
}
