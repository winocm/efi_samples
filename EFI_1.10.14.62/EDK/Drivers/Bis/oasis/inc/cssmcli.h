/* SCCSID: inc/cssmcli.h, dss_cdsa_fwk, fwk_rel2, dss_971010 1.4 10/23/97 17:53:32 */
/*
 * (C) COPYRIGHT International Business Machines Corp. 1996, 1997
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _CSSMCLI_H
#define _CSSMCLI_H

#include "cssmtype.h"
//#include "cssmspi.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct cssm_spi_cl_funcs {
#ifndef CSSM_BIS
    CSSM_BOOL     (CSSMAPI *CertVerify)(CSSM_CL_HANDLE CLHandle,
                                        CSSM_CC_HANDLE CCHandle,
                                        const CSSM_DATA_PTR CertToBeVerified,
                                        const CSSM_DATA_PTR SignerCert,
                                        const CSSM_FIELD_PTR VerifyScope,
                                        uint32 ScopeSize);
#endif
    CSSM_DATA_PTR  (CSSMAPI *CertGetFirstFieldValue)
                                       (CSSM_CL_HANDLE CLHandle,
                                        const CSSM_DATA_PTR Cert,
                                        const CSSM_OID_PTR CertField,
                                        CSSM_HANDLE_PTR ResultsHandle,
                                        uint32 *NumberOfMatchedFields);
#ifndef CSSM_BIS
    CSSM_DATA_PTR  (CSSMAPI *CertGetNextFieldValue)
                                       (CSSM_CL_HANDLE CLHandle,
                                        CSSM_HANDLE ResultsHandle);
#endif
    CSSM_RETURN    (CSSMAPI *CertAbortQuery)
                                       (CSSM_CL_HANDLE CLHandle,
                                        CSSM_HANDLE ResultsHandle);
    CSSM_KEY_PTR   (CSSMAPI *CertGetKeyInfo)
                                       (CSSM_CL_HANDLE CLHandle,
                                        const CSSM_DATA_PTR Cert);
#ifndef CSSM_BIS
    CSSM_BOOL     (CSSMAPI *CrlVerify) (CSSM_CL_HANDLE CLHandle,
                                        CSSM_CC_HANDLE CCHandle,
                                        const CSSM_DATA_PTR CrlToBeVerified,
                                        const CSSM_DATA_PTR SignerCert,
                                        const CSSM_FIELD_PTR VerifyScope,
                                        uint32 ScopeSize);
    CSSM_BOOL     (CSSMAPI *IsCertInCrl)(CSSM_CL_HANDLE CLHandle,
                                        const CSSM_DATA_PTR Cert,
                                        const CSSM_DATA_PTR Crl);
    CSSM_DATA_PTR (CSSMAPI *CrlGetFirstFieldValue)
                                       (CSSM_CL_HANDLE CLHandle,
                                        const CSSM_DATA_PTR Crl,
                                        const CSSM_OID_PTR CrlField,
                                        CSSM_HANDLE_PTR ResultsHandle,
                                        uint32 *NumberOfMatchedFields);
    CSSM_DATA_PTR (CSSMAPI *CrlGetNextFieldValue)
                                       (CSSM_CL_HANDLE CLHandle,
                                        CSSM_HANDLE ResultsHandle);
    CSSM_RETURN   (CSSMAPI *CrlAbortQuery)
                                       (CSSM_CL_HANDLE CLHandle,
                                        CSSM_HANDLE ResultsHandle);
    void * (CSSMAPI *PassThrough)(CSSM_CL_HANDLE CLHandle,
                                        CSSM_CC_HANDLE CCHandle,
                                        uint32 PassThroughId,
                                        const void * InputParams);
#endif
} CSSM_SPI_CL_FUNCS, *CSSM_SPI_CL_FUNCS_PTR;

#ifdef __cplusplus
}
#endif

#endif
