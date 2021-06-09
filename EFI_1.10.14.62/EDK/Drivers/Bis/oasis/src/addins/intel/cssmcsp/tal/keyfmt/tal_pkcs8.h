/*-----------------------------------------------------------------------
 *      File:  tal_pkcs8.h
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
#ifndef	_TAL_PKCS8_H
#define	_TAL_PKCS8_H

#include "cssmtype.h"

typedef enum algid_item_order {
	ALGID_ITEM_ALG = 0,
	ALGID_ITEM_PARAM = 1,
	ALGID_ITEM_COUNT = 2
} ALGID_ITEM_ORDER;

typedef enum pkcs8_raw_param_order {
	PKCS8_RAW_PARAM_VERSION = 0,
	PKCS8_RAW_PARAM_ALGORITHM = 1,
	PKCS8_RAW_PARAM_KEYDATA = 2,
	PKCS8_RAW_PARAM_COUNT = 3
} PKCS8_RAW_PARAM_ORDER;

typedef enum pkcs8_encrypted_param_order {
	PKCS8_ENCRYPTED_PARAM_ALGORITHM = 0,
	PKCS8_ENCRYPTED_PARAM_KEYDATA = 1,
	PKCS8_ENCRYPTED_PARAM_COUNT = 2
} PKCS8_ENCRYPTED_PARAM_ORDER;


typedef enum pkcs8_with_pkcs5_param_order {
	PKCS8_WITH_PKCS5_PARAM_DERIVE_ALG = 0,
	PKCS8_WITH_PKCS5_PARAM_ITERATION_COUNT = 1,
	PKCS8_WITH_PKCS5_PARAM_SALT = 2,
	PKCS8_WITH_PKCS5_PARAM_ENCRYPTED_DATA = 3,
	PKCS8_WITH_PKCS5_PARAM_COUNT = 4
} PKCS8_WITH_PKCS5_PARAM_ORDER;

/****************************************************************************/
/* extern functions															*/
/****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

extern sint32 TAL_Parse_PKCS8_RawKey(   const CSSM_KEY_PTR	KeyBlob_ptr,
									uint32				*KeyAlgID_ptr,
									uint32				*KeyFormat_ptr,
									uint32				*KeyDataLength_ptr,
									uint32				*KeyDataOffset_ptr);

extern sint32 TAL_Parse_PKCS8_EncryptedKey( const CSSM_KEY_PTR	KeyBlob_ptr,
										uint32				*WrapKeyAlgID_ptr,
										uint32				*WrapKeyAlgMode_ptr,
										uint32				*KeyFormat_ptr,
										uint32				*KeyDataLength_ptr,
										uint32				*KeyDataOffset_ptr);

extern sint32 TAL_Parse_PKCS8w5_EncryptedKey(
										const CSSM_KEY_PTR	KeyBlob_ptr,
										uint32				*DeriveKeyAlg_ptr,
										uint32				*WrapKeyAlgID_ptr,
										uint32				*WrapKeyAlgMode_ptr,
										uint32				*IterationCount_ptr,
										uint32				*SaltLength_ptr,
										uint32				*SaltOffset_ptr,
										uint32				*KeyFormat_ptr,
										uint32				*KeyDataLength_ptr,
										uint32				*KeyDataOffset_ptr);

/*########################### SPACE_REDUCE ###########################*/
#ifndef SPACE_REDUCE

extern sint32 TAL_Create_PKCS8_RawKey(
									CSSM_KEY_PTR		KeyBlob_ptr,
									uint32				*NeedKeyDataSize_ptr,
									uint32				KeyAlgID,
									const CSSM_DATA_PTR	KeyData_ptr);

extern sint32 TAL_Create_PKCS8_EncryptedKey(CSSM_KEY_PTR		KeyBlob_ptr,
										uint32				*NeedKeyDataSize_ptr,
										uint32				WrapKeyAlgID,
										uint32				WrapKeyAlgMode,
										const CSSM_DATA_PTR	WrappedKeyData_ptr);

extern sint32 TAL_Create_PKCS8w5_EncryptedKey(
										CSSM_KEY_PTR		KeyBlob_ptr,
						    			uint32				*NeedKeyDataSize_ptr,
										uint32				DeriveKeyAlg,
										uint32				WrapKeyAlgID,
										uint32				WrapKeyAlgMode,
										uint32				IterationCount,
										const CSSM_DATA_PTR	Salt_ptr,
										const CSSM_DATA_PTR	WrappedKeyData_ptr);
#endif /*SPACE_REDUCE*/
/*######################## end SPACE_REDUCE ###########################*/

#ifdef __cplusplus
}
#endif

#endif
