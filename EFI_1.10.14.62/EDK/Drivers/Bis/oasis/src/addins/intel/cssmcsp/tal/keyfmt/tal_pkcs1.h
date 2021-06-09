/*-----------------------------------------------------------------------
 *      File:  tal_pkcs1.h
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
#ifndef	_TAL_PKCS1_H
#define	_TAL_PKCS1_H

#include "cssmtype.h"

typedef enum rsa_prikey_param_order {
	RSA_PRIKEY_PARAM_E = 0,
	RSA_PRIKEY_PARAM_D = 1,
	RSA_PRIKEY_PARAM_N = 2,
	RSA_PRIKEY_PARAM_P = 3,
	RSA_PRIKEY_PARAM_Q = 4,
	RSA_PRIKEY_PARAM_EXP1  = 5,
	RSA_PRIKEY_PARAM_EXP2  = 6,
	RSA_PRIKEY_PARAM_COEFF = 7,
	RSA_PRIKEY_PARAM_COUNT = 8
} RSA_PRIKEY_PARAM_ORDER;

typedef enum rsa_pubkey_param_order {
	RSA_PUBKEY_PARAM_E = 0,
	RSA_PUBKEY_PARAM_N = 1,
	RSA_PUBKEY_PARAM_COUNT = 2
} RSA_PUBKEY_PARAM_ORDER;

/****************************************************************************/
/* extern functions															*/
/****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

extern sint32 TAL_Parse_PKCS1_Key(const CSSM_KEY_PTR KeyBlob_ptr,
								 CSSM_DATA_PTR OutData_ptr,
								 uint32 *OutDataCount_ptr);

extern sint32 TAL_Build_PKCS1_Key(CSSM_KEY_PTR KeyBlob_ptr,
								 const CSSM_DATA_PTR InData,
								 uint32 InDataCount);

#ifdef __cplusplus
}
#endif

#endif
