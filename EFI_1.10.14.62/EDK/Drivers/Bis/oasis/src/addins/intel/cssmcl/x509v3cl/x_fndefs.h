/*-----------------------------------------------------------------------
 *        File:   x_fndefs.h
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
 * This is a header file for functions in this library
 */

#ifndef _X_FNDEFS_H
#define _X_FNDEFS_H

/* CSSM includes */
#include "cssm.h"
#include "cssmport.h"
#include "x509err.h"
#include "guids.h"

/* Certificate-specific includes */
#include "x509defs.h"
#include "ber_der.h"
#include "oidsalg.h"
#include "oidscert.h"

#define CSSMCLI CSSMAPI

#define CERT_TBSCERT                0 /* Position of TbsCert       in Cert    */
#define CERT_SIG_ALG                1 /* Position of SigAlg        in Cert    */

#define TBSCERT_VERSION             0 /* Position of Version       in TbsCert */
#define TBSCERT_SERIAL_NUM          1 /* Position of SerialNumber  in TbsCert */
#define TBSCERT_SIG_ALG             2 /* Position of SigAlg        in TbsCert */
#define TBSCERT_ISSUER              3 /* Position of IssuerName    in TbsCert */
#define TBSCERT_VALIDITY            4 /* Position of ValidityDates in TbsCert */
#define TBSCERT_SUBJECT             5 /* Position of SubjectName   in TbsCert */
#define TBSCERT_SPKI                6 /* Position of SPKI          in TbsCert */
#define TBSCERT_ISSUER_UID          7 /* Position of IssuerUID     in TbsCert */
#define TBSCERT_SUBJECT_UID         8 /* Position of SubjectUID    in TbsCert */
#define TBSCERT_EXTENSIONS          9 /* Position of Extensions    in TbsCert */

#define SPKI_ALGID           0 /* Position of AlgorithmId in SPKI          */
#define SPKI_SPK             1 /* Position of SPK         in SPKI          */

#define ALGID_ALGORITHM      0 /* Position of Algorithm   in AlgorithmId   */
#define ALGID_PARAMS         1 /* Position of Parameters  in AlgorithmId   */


#define NUM_CERT_FIELDS      3 /* TbsCert, SigAlg, Signature               */
#define NUM_TBSCERT_FIELDS  10 /* Version, SNum, SigAlg,                   */
                               /* IssuerName, Validity, SubjectName, SPKI, */
                               /* IssuerUID, SubjectUID, Extensions        */
#define MIN_NUM_ALGID_FIELDS 1 /* OID only                                 */
#define MAX_NUM_ALGID_FIELDS 2 /* OID + Value                              */

extern CSSM_SPI_MEMORY_FUNCS CLMemFuncs;

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/*                        Exported CL functions                              */
/*****************************************************************************/
CSSM_DATA_PTR CSSMCLI CL_CertGetFirstFieldValue 
                                        (CSSM_CL_HANDLE CLHandle,
                                         const CSSM_DATA_PTR Cert, 
                                         const CSSM_OID_PTR CertField,
                                         CSSM_HANDLE_PTR ResultsHandle,
                                         uint32 *NumberOfMatchedFields);

CSSM_RETURN   CSSMCLI CL_CertAbortQuery (CSSM_CL_HANDLE CLHandle,
                                         CSSM_HANDLE ResultsHandle);

CSSM_KEY_PTR  CSSMCLI CL_CertGetKeyInfo (CSSM_CL_HANDLE CLHandle,
                                         const CSSM_DATA_PTR Cert);

#ifndef CSSM_BIS
CSSM_BOOL     CSSMCLI CL_CertVerify     (CSSM_CL_HANDLE CLHandle,
                                         CSSM_CC_HANDLE CCHandle, 
                                         const CSSM_DATA_PTR SubjectCert, 
                                         const CSSM_DATA_PTR SignerCert, 
                                         const CSSM_FIELD_PTR VerifyScope,
                                         uint32 ScopeSize);

CSSM_DATA_PTR CSSMCLI CL_CertGetNextFieldValue 
                                        (CSSM_CL_HANDLE CLHandle,
                                         CSSM_HANDLE ResultsHandle);

void *        CSSMCLI CL_PassThrough    (CSSM_CL_HANDLE CLHandle,
                                         CSSM_CC_HANDLE CCHandle, 
                                         uint32 PassThroughId, 
                                         const void * InputParams);
#endif /* #ifndef CSSM_BIS */


/*****************************************************************************/
/*                        Internal CL functions                              */
/*****************************************************************************/
CSSM_DATA_PTR cl_CertGetField          (CSSM_CL_HANDLE CLHandle, 
                                        DER_NODE_PTR Cert, 
                                        CSSM_OID_PTR Oid,
                                        uint32 *NumberOfMatchedFields);

CSSM_KEY_PTR cl_SpkiParseTreeToCSSMKey (CSSM_HANDLE CLHandle, 
                                        DER_NODE_PTR SPKI);

/* BER/DER functions */
DER_NODE_PTR  cl_DerDecodeCertificate  (CSSM_HANDLE CLHandle, 
                                        const CSSM_DATA_PTR Cert);
void          cl_FreeCertificate       (CSSM_HANDLE CLHandle, 
                                        DER_NODE_PTR Cert);
CSSM_BOOL cl_IsBadCertificateParseTree (DER_NODE_PTR ParentNode);
CSSM_BOOL cl_IsBadDerNodeChild         (struct der_node_child_struct * Child, 
                                        uint8 Tag);

/* OID => Int functions */
CSSM_ALGORITHMS         cl_AlgorithmOidToAlgId(CSSM_OID_PTR Oid);
INTEL_X509V3_OID_AS_INT cl_CertOidToInt(CSSM_OID_PTR Oid);

/* Time type checking functions */
CSSM_BOOL cl_IsBadUtc             (const uint8* UTCTime, sint32 Length);
CSSM_BOOL cl_IsBadGeneralizedTime (const uint8* GTime,   sint32 Length);

#ifndef CSSM_BIS
/* Time conversion functions */
sint32 cl_UtcToTime               (const char *szTime, sint32 length);
sint32 cl_GeneralizedTimeToTime   (const char *szTime, sint32 length);
#endif

#ifdef __cplusplus
}
#endif

#endif
