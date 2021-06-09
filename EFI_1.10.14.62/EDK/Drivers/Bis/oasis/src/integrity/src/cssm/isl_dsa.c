/*-----------------------------------------------------------------------
 *      File: dsa.c  
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *-----------------------------------------------------------------------
 */
/* 
    * INTEL CONFIDENTIAL 
    * This file, software, or program is supplied under the terms of a 
    * license agreement or nondisclosure agreement with Intel Corporation 
    * and may not be copied or disclosed except in accordance with the 
    * terms of that agreement. This file, software, or program contains 
    * copyrighted material and/or trade secret information of Intel 
    * Corporation, and must be treated as such. Intel reserves all rights 
    * in this material, except as the license agreement or nondisclosure 
    * agreement specifically indicate. 
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
#include "integapi.h"
#include "cssm.h"
#include "ber_der.h"
#include "cssmtype.h"
#include "isl_internal.h"
extern ISL_DIGEST_METHODS PKCS7SHA1Methods;
extern ISL_SIGN_VERIFY_METHODS PKCS7DSAMethods;

/*
**	DSA algorithm glue
*/

/* OIW combination dsa/sha-1 with parameters */
static const unsigned char dsaoid[] = {
	0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 27};  

/* OIW standalone dsa with parameters */    
static const unsigned char pkcs7dsaoid[] = {
	0x06, 0x05, 0x2b, 0x0e, 0x03, 0x02, 12 };	 
/*
**	CSSM version of DSA sign and verify
*/

/* non-combination algorithm DSA for PKCS7 signatures */

/*-----------------------------------------------------------------------------
 * Name: id7
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static void id7(
	ISL_SERVICE_CLASS *algClass,				
	ISL_CONST_DATA *algID,				
	ISL_CONST_DATA *serviceName)
{
	*algClass = ISL_ServiceSignVerify;
	algID->Data = pkcs7dsaoid;
	algID->Length = sizeof(pkcs7dsaoid);
	serviceName->Data = (const uint8 *)"DSA";
	serviceName->Length = sizeof("DSA")-1;
}


ISL_SIGN_VERIFY_METHODS PKCS7DSAMethods = {
	{id7, 0},
	0,
	CSSM_ALGID_SHA1WithDSA
};
