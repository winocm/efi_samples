/*-----------------------------------------------------------------------
 *      File:   oidscert.h
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
 * -----------------------------------------------------------------------
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

#ifndef _OIDSCERT_H
#define _OIDSCERT_H

#include "cssm.h"
#include "oidsbase.h"

#define INTEL_X509V3_CERT_R08           INTEL_SEC_FORMATS, 1, 1
#define INTEL_X509V3_CERT_R08_LENGTH    INTEL_SEC_FORMATS_LENGTH + 2

#define INTEL_X509V3_CERT_PRIVATE_EXTENSIONS           INTEL_X509V3_CERT_R08, 50
#define INTEL_X509V3_CERT_PRIVATE_EXTENSIONS_LENGTH    INTEL_X509V3_CERT_R08_LENGTH + 1

#define INTEL_X509V3_SIGN_R08           INTEL_SEC_FORMATS, 3, 2
#define INTEL_X509V3_SIGN_R08_LENGTH    INTEL_SEC_FORMATS_LENGTH + 2


/************************* CERTIFICATE & SIGNATURE OIDS **********************/
static uint8
    X509V1SerialNumber[]                = {INTEL_X509V3_CERT_R08, 3},
    X509V1IssuerName[]                  = {INTEL_X509V3_CERT_R08, 5},   
    X509V1SubjectName[]                 = {INTEL_X509V3_CERT_R08, 8},   
    X509V1SignatureAlgorithm[]          = {INTEL_X509V3_SIGN_R08, 1};     

/* NOTE: INTEL_X509V3_CERT_R08, 50 is reserved for the Extension Contents OID tree */

static CSSM_OID
    CSSMOID_X509V1SerialNumber              = {INTEL_X509V3_CERT_R08_LENGTH+1, X509V1SerialNumber},
    CSSMOID_X509V1IssuerName                = {INTEL_X509V3_CERT_R08_LENGTH+1, X509V1IssuerName},
    CSSMOID_X509V1SubjectName               = {INTEL_X509V3_CERT_R08_LENGTH+1, X509V1SubjectName},
    CSSMOID_X509V1SignatureAlgorithm        = {INTEL_X509V3_SIGN_R08_LENGTH+1, X509V1SignatureAlgorithm};


/**************** Certificate OID -> int mapping **************/
typedef enum intel_x509v3_oid_as_int {
    OIDasINT_X509V3NoMatch = 0,
    OIDasINT_X509V1Version,
    OIDasINT_X509V1SerialNumber,
    OIDasINT_X509V1IssuerName,
    OIDasINT_X509V1ValidityNotBefore,
    OIDasINT_X509V1ValidityNotAfter,
    OIDasINT_X509V1SubjectName,
    OIDasINT_CSSMKeyStruct,
    OIDasINT_X509V1SubjectPublicKeyAlgorithm,
    OIDasINT_X509V1SubjectPublicKeyAlgorithmParameters,
    OIDasINT_X509V1SubjectPublicKey,
    OIDasINT_X509V1CertificateIssuerUniqueId,
    OIDasINT_X509V1CertificateSubjectUniqueId,
    OIDasINT_X509V3CertificateExtensionId,
    OIDasINT_X509V3CertificateExtensionCritical,
    OIDasINT_X509V3CertificateExtensionType,
    OIDasINT_X509V3CertificateExtensionValue,

    OIDasINT_X509V3SignedCertificate,
    OIDasINT_X509V3Certificate,
    OIDasINT_X509V3CertificateExtensionStruct,

    OIDasINT_X509V3CertificateNumberOfExtensions,
    OIDasINT_X509V1SignatureStruct,
    OIDasINT_X509V1SignatureAlgorithm, 
    OIDasINT_X509V1SignatureAlgorithmParameters, 
    OIDasINT_X509V1Signature
} INTEL_X509V3_OID_AS_INT;


/****************************************************************************/
/* These definitions have been removed from the size-reduced implementation */
/****************************************************************************/
#if 0
static uint8
    X509V3SignedCertificate[]           = {INTEL_X509V3_CERT_R08},   
    X509V3Certificate[]                 = {INTEL_X509V3_CERT_R08, 1},
    X509V1Version[]                     = {INTEL_X509V3_CERT_R08, 2},
    X509V1ValidityNotBefore[]           = {INTEL_X509V3_CERT_R08, 6},   
    X509V1ValidityNotAfter[]            = {INTEL_X509V3_CERT_R08, 7},   
    CSSMKeyStruct[]                     = {INTEL_X509V3_CERT_R08, 20},
    X509V1SubjectPublicKeyAlgorithm[]   = {INTEL_X509V3_CERT_R08, 9}, 
    X509V1SubjectPublicKeyAlgorithmParameters[]   = {INTEL_X509V3_CERT_R08, 18},
    X509V1SubjectPublicKey[]            = {INTEL_X509V3_CERT_R08, 10},
    X509V1CertificateIssuerUniqueId[]   = {INTEL_X509V3_CERT_R08, 11},
    X509V1CertificateSubjectUniqueId[]  = {INTEL_X509V3_CERT_R08, 12},

    X509V3CertificateExtensionStruct[]    = {INTEL_X509V3_CERT_R08, 13},    
    X509V3CertificateNumberOfExtensions[] = {INTEL_X509V3_CERT_R08, 14}, 
    X509V3CertificateExtensionId[]        = {INTEL_X509V3_CERT_R08, 15},    
    X509V3CertificateExtensionCritical[]  = {INTEL_X509V3_CERT_R08, 16},
    X509V3CertificateExtensionType[]      = {INTEL_X509V3_CERT_R08, 19},
    X509V3CertificateExtensionValue[]     = {INTEL_X509V3_CERT_R08, 17},

    X509V1SignatureStruct[]             = {INTEL_X509V3_SIGN_R08, 0},
    X509V1SignatureAlgorithmParameters[] = {INTEL_X509V3_SIGN_R08, 3},   
    X509V1Signature[]                   = {INTEL_X509V3_SIGN_R08, 2};     

#endif

#endif
