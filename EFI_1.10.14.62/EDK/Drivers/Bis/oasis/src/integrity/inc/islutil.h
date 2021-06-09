/*-----------------------------------------------------------------------
 *      File:   islutil.h
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *---------------------------------------------------------------------
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

#ifndef _ISLUTIL_H_
#define _ISLUTIL_H_
#include "isltype.h"


/* API Functions */
#ifdef __cplusplus
extern "C" {
#endif
uint32 IslUtil_Base64EncodeSize(ISL_DATA InputData);

ISL_STATUS 
IslUtil_Base64Encode(
	ISL_DATA InputData, 
	ISL_DATA_PTR pOutputBuff);

ISL_STATUS IslUtil_Base64Decode(
	ISL_DATA_PTR pEncodedData, 
	ISL_DATA_PTR pDecodedData);

uint32 IslUtil_Base64DecodeSize(ISL_DATA_PTR pEncodedData);

#ifdef __cplusplus
}
#endif
#endif /* _ISLUTIL_H_ */
