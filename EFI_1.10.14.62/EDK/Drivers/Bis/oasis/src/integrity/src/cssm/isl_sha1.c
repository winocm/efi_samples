/*-----------------------------------------------------------------------
 *      File: sha1.c  
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
/*
**	SHA-1 algorithm glue
*/
#include "cssm.h"
#include "guids.h"
#include "isl_internal.h"

/* 
**	The SHA-1 OID is defined in the OIW/SECSIG stable agreements Mar, 1995.
**	URL: http://nemo.ncsl.nist.gov/oiw/agreements/stable/OSI/12s_9506.txt
**	iso(1) identified-organization(3) oiw(14) secsig(3) algorithm(2) sha1(26)
*/
extern CSSM_CSP_HANDLE gCSPHandle;

static unsigned char sha1oid[] = { 6, 5, 1 * 40 + 3, 14, 3, 2, 26};

ISL_CONST_DATA gsSHA1 = { sizeof("SHA-1")-1,
                          (const uint8 *)"SHA-1"};

/*-----------------------------------------------------------------------------
 * Name: pkcs7identify
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
//static
//void pkcs7identify(
//	ISL_SERVICE_CLASS *algClass,				
//	ISL_CONST_DATA *algID,				
//	ISL_CONST_DATA *serviceName)
//{
//	*algClass = ISL_ServiceDigest;
//	algID->Data = sha1oid;
//	algID->Length = sizeof(sha1oid);
//	*serviceName = gsSHA1;
//}

/*-----------------------------------------------------------------------------
 * Name: jaridentify
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
void jaridentify(
	ISL_SERVICE_CLASS *algClass,				
	ISL_CONST_DATA *algID,				
	ISL_CONST_DATA *serviceName)
{
	*algClass = ISL_ServiceDigest;
	*algID = gsSHA1;
	*serviceName = gsSHA1;
}	

ISL_DIGEST_METHODS JarSHA1Methods = { 
	{ jaridentify },
	0,
	CSSM_ALGID_SHA1,
};

