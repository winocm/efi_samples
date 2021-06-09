/* SCCSID: %W% %I% %H% %T% */
/*-----------------------------------------------------------------------
 *      File:   CSSMCSPI.H
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

#ifndef _CSSMCSPI_H
#define _CSSMCSPI_H    

#include "cssmtype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cssm_spi_csp_funcs {
#ifndef CSSM_BIS
    /* Information Functions */
    CSSM_RETURN (CSSMAPI *QuerySize) (CSSM_CSP_HANDLE CSPHandle,
                                      CSSM_CC_HANDLE CCHandle,
                                      const CSSM_CONTEXT_PTR Context,
                                      CSSM_BOOL Encrypt,
                                      uint32 QuerySizeCount,
                                      CSSM_QUERY_SIZE_DATA_PTR DataBlock);
    /* Crypto Functions */
    CSSM_RETURN (CSSMAPI *SignData) (CSSM_CSP_HANDLE CSPHandle,
                                     CSSM_CC_HANDLE CCHandle,
                                     const CSSM_CONTEXT_PTR Context,
                                     const CSSM_DATA_PTR DataBufs,
                                     uint32 DataBufCount,
                                     CSSM_DATA_PTR Signature);
#endif
    CSSM_BOOL (CSSMAPI *VerifyData) (CSSM_CSP_HANDLE CSPHandle,
                                     CSSM_CC_HANDLE CCHandle,
                                     const CSSM_CONTEXT_PTR Context,
                                     const CSSM_DATA_PTR DataBufs,
                                     uint32 DataBufCount,
                                     const CSSM_DATA_PTR Signature);
    CSSM_RETURN (CSSMAPI *DigestData) (CSSM_CSP_HANDLE CSPHandle,
                                       CSSM_CC_HANDLE CCHandle,
                                       const CSSM_CONTEXT_PTR Context,
                                       const CSSM_DATA_PTR DataBufs,
                                       uint32 DataBufCount,
                                       CSSM_DATA_PTR Digest);
#ifndef CSSM_BIS
    CSSM_RETURN (CSSMAPI *EncryptData) (CSSM_CSP_HANDLE CSPHandle,
                                        CSSM_CC_HANDLE CCHandle,
                                        const CSSM_CONTEXT_PTR Context,
                                        const CSSM_DATA_PTR ClearBufs,
                                        uint32 ClearBufCount,
                                        CSSM_DATA_PTR CipherBufs,
                                        uint32 CipherBufCount,
                                        uint32 *bytesEncrypted,
                                        CSSM_DATA_PTR RemData);
    CSSM_RETURN (CSSMAPI *DecryptData) (CSSM_CSP_HANDLE CSPHandle,
                                        CSSM_CC_HANDLE CCHandle,
                                        const CSSM_CONTEXT_PTR Context,
                                        const CSSM_DATA_PTR CipherBufs,
                                        uint32 CipherBufCount,
                                        CSSM_DATA_PTR ClearBufs,
                                        uint32 ClearBufCount,
                                        uint32 *bytesDecrypted,
                                        CSSM_DATA_PTR RemData);
#endif
    CSSM_RETURN (CSSMAPI *QueryKeySizeInBits) (CSSM_CSP_HANDLE CSPHandle,
                                               CSSM_CC_HANDLE CCHandle,
											   const CSSM_KEY_PTR Key,
                                               CSSM_KEY_SIZE_PTR KeySize);
#ifndef CSSM_BIS
    CSSM_RETURN (CSSMAPI *GenerateKey) (CSSM_CSP_HANDLE CSPHandle,
                                        CSSM_CC_HANDLE CCHandle,
                                        const CSSM_CONTEXT_PTR Context,
                                        uint32 KeyUsage,
                                        uint32 KeyAttr,
                                        const CSSM_DATA_PTR KeyLabel,
                                        CSSM_KEY_PTR Key);
    CSSM_RETURN (CSSMAPI *GenerateKeyPair) (CSSM_CSP_HANDLE CSPHandle,
                                            CSSM_CC_HANDLE CCHandle,
                                            const CSSM_CONTEXT_PTR Context,
                                            uint32 PublicKeyUsage,
                                            uint32 PublicKeyAttr,
                                            const CSSM_DATA_PTR PublicKeyLabel,
                                            CSSM_KEY_PTR PublicKey,
                                            uint32 PrivateKeyUsage,
                                            uint32 PrivateKeyAttr,
                                            const CSSM_DATA_PTR PrivateKeyLabel,
                                            CSSM_KEY_PTR PrivateKey);
    CSSM_RETURN (CSSMAPI *GenerateRandom) (CSSM_CSP_HANDLE CSPHandle,
                                           CSSM_CC_HANDLE CCHandle,
                                           const CSSM_CONTEXT_PTR Context,
                                           CSSM_DATA_PTR RandomNumber);
    CSSM_RETURN (CSSMAPI *GenerateAlgorithmParams) (CSSM_CSP_HANDLE CSPHandle,
                                                    CSSM_CC_HANDLE CCHandle,
                                                    const CSSM_CONTEXT_PTR Context,
                                                    uint32 ParamBits,
                                                    CSSM_DATA_PTR Param);
    CSSM_RETURN (CSSMAPI *WrapKey) (CSSM_CSP_HANDLE CSPHandle,
                                    CSSM_CC_HANDLE CCHandle,
                                    const CSSM_CONTEXT_PTR Context,
                                    const CSSM_CRYPTO_DATA_PTR PassPhrase, 
                                    const CSSM_KEY_PTR Key,
									const CSSM_DATA_PTR DescriptiveData,
                                    CSSM_WRAP_KEY_PTR WrappedKey);
    CSSM_RETURN (CSSMAPI *UnwrapKey) (CSSM_CSP_HANDLE CSPHandle, 
                                      CSSM_CC_HANDLE CCHandle,
                                      const CSSM_CONTEXT_PTR Context,
                                      const CSSM_CRYPTO_DATA_PTR NewPassPhrase,
									  const CSSM_KEY_PTR PublicKey,
                                      const CSSM_WRAP_KEY_PTR WrappedKey,
                                      uint32 KeyUsage,
                                      uint32 KeyAttr,
                                      const CSSM_DATA_PTR KeyLabel,
                                      CSSM_KEY_PTR UnwrappedKey,
									  CSSM_DATA_PTR DescriptiveData);
	CSSM_RETURN (CSSMAPI *FreeKey) (CSSM_CSP_HANDLE CSPHandle,
								  CSSM_KEY_PTR KeyPtr);
    /* Expandability Functions */
    void * (CSSMAPI *PassThrough) (CSSM_CSP_HANDLE CSPHandle, 
                                   CSSM_CC_HANDLE CCHandle,
                                   const CSSM_CONTEXT_PTR Context,
                                   uint32 PassThroughId,
                                   const void * InData);
    /* User Login Functions */
    CSSM_RETURN (CSSMAPI *Login) (CSSM_CSP_HANDLE CSPHandle,
                                  const CSSM_CRYPTO_DATA_PTR Password,
                                  const CSSM_DATA_PTR Reserved);
    CSSM_RETURN (CSSMAPI *Logout) (CSSM_CSP_HANDLE CSPHandle);
    CSSM_RETURN (CSSMAPI *ChangeLoginPassword) 
                                      (CSSM_CSP_HANDLE CSPHandle,
                                       const CSSM_CRYPTO_DATA_PTR OldPassword,
                                       const CSSM_CRYPTO_DATA_PTR NewPassword);
    CSSM_RETURN (CSSMAPI *ObtainPrivateKeyFromPublicKey) (
                                                 CSSM_CSP_HANDLE CSPHandle,
                                                 const CSSM_KEY_PTR PublicKey,
                                                 CSSM_KEY_PTR PrivateKey);
#endif
} CSSM_SPI_CSP_FUNCS, *CSSM_SPI_CSP_FUNCS_PTR;

#ifdef __cplusplus
}
#endif


#endif
