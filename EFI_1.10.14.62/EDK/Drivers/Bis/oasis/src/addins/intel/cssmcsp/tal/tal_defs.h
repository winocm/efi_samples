/*-----------------------------------------------------------------------
 *      File:   tal_defs.h
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
 * This is the header file for token adaptation layer utility functions.
 * This file is part of the Token Adaptation Layer (TAL) source code base.
 * The TAL code makes it easier for CSP venders to develop CSPs that plug
 * into the Intel CDSA infrastructure.
 */

#ifndef	_TAL_DEFS_H
#define	_TAL_DEFS_H

#include "cssmtype.h"

#define MAX_HASHED_OUTPUT_LEN	20
#define MAX_BUFFER_LEN          128
#define CSP_HANDLE_SELF	        0
#define MAX_SIGNATURE_LEN	    256

/*######################### bgn not CSSM_BIS ##########################*/
#ifndef CSSM_BIS

#define MAX_FILEPATH_LEN        256
#define MAX_KEYBUF_LEN	        1024

#define CSP_MD5_DIGEST_SIZE	    16
#define CSP_MD2_DIGEST_SIZE	    16
#define CSP_SHA1_DIGEST_SIZE    20
#define CSP_MAX_DIGEST_SIZE	    20

#define CSP_RSA_PKCS_PADDING_SIZE	11

#define CSP_DES_KEYSIZE_IN_BYTES 8
#define CSP_DES_BLOCK	         8
#define CSP_RC5_BLOCK	         8

typedef struct csp_pbe_params {
CSSM_DATA_PTR	Password;
CSSM_DATA_PTR   InitVector;
} CSP_PBE_PARAMS, *CSP_PBE_PARAMS_PTR;


#endif /*CSSM_BIS*/
/*######################### end not CSSM_BIS ##########################*/

typedef enum csp_session_type {
    CSP_CRYPTO_NONE	        = CSSM_ALGCLASS_NONE,
    CSP_CRYPTO_CUSTOM	    = CSSM_ALGCLASS_CUSTOM,
    CSP_CRYPTO_KEYXCH	    = CSSM_ALGCLASS_KEYXCH,
    CSP_CRYPTO_SIGNATURE    = CSSM_ALGCLASS_SIGNATURE,
    CSP_CRYPTO_SYMMETRIC    = CSSM_ALGCLASS_SYMMETRIC,
    CSP_CRYPTO_DIGEST	    = CSSM_ALGCLASS_DIGEST,
    CSP_CRYPTO_RANDOMGEN    = CSSM_ALGCLASS_RANDOMGEN,
    CSP_CRYPTO_UNIQUEGEN    = CSSM_ALGCLASS_UNIQUEGEN,
    CSP_CRYPTO_MAC	        = CSSM_ALGCLASS_MAC,
    CSP_CRYPTO_ASYMMETRIC	= CSSM_ALGCLASS_ASYMMETRIC,
    CSP_CRYPTO_KEYGEN	    = CSSM_ALGCLASS_KEYGEN,
    CSP_CRYPTO_DERIVE	    = CSSM_ALGCLASS_DERIVEKEY,

    CSP_CRYPTO_ENC	        = 50,	/* CSSM_ALGCLASS_SYMMETRIC or CSSM_ALGCLASS_ASYMMETRIC */
    CSP_CRYPTO_DEC	        = 51,	/* CSSM_ALGCLASS_SYMMETRIC or CSSM_ALGCLASS_ASYMMETRIC */
    CSP_CRYPTO_WRAP	        = 52,	/* CSSM_ALGCLASS_SYMMETRIC or CSSM_ALGCLASS_ASYMMETRIC */
    CSP_CRYPTO_UNWRAP	    = 53,	/* CSSM_ALGCLASS_SYMMETRIC or CSSM_ALGCLASS_ASYMMETRIC */
    CSP_CRYPTO_SIGN	        = 54,	/* CSSM_CRYPTO_SIGNATURE  */
    CSP_CRYPTO_VERIFY	    = 55,	/* CSSM_CRYPTO_SIGNATURE  */
    CSP_CRYPTO_GEN_MAC	    = 56,	/* CSSM_ALGCLASS_MAC  */
    CSP_CRYPTO_VERIFY_MAC	= 57,	/* CSSM_ALGCLASS_MAC  */
} CSP_SESSION_TYPE;

#endif /*_TAL_DEFS_H*/
