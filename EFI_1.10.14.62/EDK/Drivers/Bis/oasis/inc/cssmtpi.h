/*-----------------------------------------------------------------------
 *      File:   CSSMTPI.H
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

#ifndef _CSSMTPI_H
#define _CSSMTPI_H    

#include "cssmtype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cssm_spi_tp_funcs {
    CSSM_BOOL (CSSMAPI *CertGroupVerify) (
                                      CSSM_TP_HANDLE TPHandle,
                                      CSSM_CL_HANDLE CLHandle,
                                      CSSM_CSP_HANDLE CSPHandle,
                                      const CSSM_DL_DB_LIST_PTR DBList,
									  const CSSM_CERTGROUP_PTR CertGroupToBeVerified,
									  const CSSM_VERIFYCONTEXT_PTR VerifyContext); 
    CSSM_BOOL (CSSMAPI *CrlVerify) (CSSM_TP_HANDLE TPHandle,
                                    CSSM_CL_HANDLE CLHandle,
                                    CSSM_CSP_HANDLE CSPHandle,
                                    const CSSM_DL_DB_LIST_PTR DBList,
                                    const CSSM_DATA_PTR CrlToBeVerified,
									CSSM_CRL_TYPE CrlType, 
									CSSM_CRL_ENCODING CrlEncoding,
                                    const CSSM_CERTGROUP_PTR SignerCertGroup,
                                    const CSSM_VERIFYCONTEXT_PTR VerifyContext);
    CSSM_RETURN (CSSMAPI *ApplyCrlToDb) (CSSM_TP_HANDLE TPHandle, 
                                         CSSM_CL_HANDLE CLHandle,
                                         CSSM_CSP_HANDLE CSPHandle,
                                         const CSSM_DL_DB_LIST_PTR DBList,
										 const CSSM_DATA_PTR CrlToBeApplied,
										CSSM_CRL_TYPE CrlType,
										CSSM_CRL_ENCODING CrlEncoding,
										const CSSM_CERTGROUP_PTR SignerCert,
										const CSSM_VERIFYCONTEXT_PTR SignerVerifyContext);
    void * (CSSMAPI *PassThrough) (CSSM_TP_HANDLE TPHandle, 
                                   CSSM_CL_HANDLE CLHandle, 
                                   CSSM_CSP_HANDLE CSPHandle, 
                                   const CSSM_DL_DB_LIST_PTR DBList, 
                                   uint32 PassThroughId,
                                   const void *InputParams);
} CSSM_SPI_TP_FUNCS, *CSSM_SPI_TP_FUNCS_PTR;

#ifdef __cplusplus
}
#endif

#endif
