/*-----------------------------------------------------------------------
        File:   oidsBase.h
  
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

  -----------------------------------------------------------------------
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

#ifndef _OIDSBASE_H
#define _OIDSBASE_H

#include "cssm.h"

/* From X.680 */

#define OID_ITU_RECOMMENDATION 0
#define OID_ITU_QUESTION 1
#define OID_ITU_ADMIN    2
#define OID_ITU_NETWORK_OP 3
#define OID_ITU_IDENTIFIED_ORG 4

#define OID_ISO_STANDARD       40
#define OID_ISO_MEMBER         42                      /* Also in PKCS */
#define OID_US                 OID_ISO_MEMBER, 134, 72 /* Also in PKCS */
#define OID_CA                 OID_ISO_MEMBER, 124

#define OID_ISO_IDENTIFIED_ORG 43
#define OID_OSINET             OID_ISO_IDENTIFIED_ORG, 4
#define OID_GOSIP              OID_ISO_IDENTIFIED_ORG, 5
#define OID_DOD                OID_ISO_IDENTIFIED_ORG, 6
#define OID_OIW                OID_ISO_IDENTIFIED_ORG, 14 /* Also in x9.57 */

#define OID_ISO_CCITT_DIR_SERVICE 85
#define OID_ISO_CCITT_COUNTRY     96
#define OID_COUNTRY_US            OID_ISO_CCITT_COUNTRY, 134, 72
#define OID_COUNTRY_CA            OID_ISO_CCITT_COUNTRY, 124
#define OID_COUNTRY_US_ORG        OID_COUNTRY_US, 1
#define OID_COUNTRY_US_MHS_MD     OID_COUNTRY_US, 2
#define OID_COUNTRY_US_STATE      OID_COUNTRY_US, 3

/* From the PKCS Standards */
#define OID_ISO_MEMBER_LENGTH 1
#define OID_US_LENGTH         OID_ISO_MEMBER_LENGTH + 2

#define OID_RSA               OID_US, 134, 247, 13
#define OID_RSA_LENGTH        OID_US_LENGTH + 3

#define OID_RSA_HASH          OID_RSA, 2
#define OID_RSA_HASH_LENGTH   OID_RSA_LENGTH + 1

#define OID_RSA_ENCRYPT       OID_RSA, 3
#define OID_RSA_ENCRYPT_LENGTH OID_RSA_LENGTH + 1

#define OID_PKCS              OID_RSA, 1
#define OID_PKCS_LENGTH       OID_RSA_LENGTH +1

#define OID_PKCS_1          OID_PKCS, 1 
#define OID_PKCS_1_LENGTH   OID_PKCS_LENGTH +1

#define OID_PKCS_2          OID_PKCS, 2 
#define OID_PKCS_3          OID_PKCS, 3 
#define OID_PKCS_3_LENGTH   OID_PKCS_LENGTH +1

#define OID_PKCS_4          OID_PKCS, 4 
#define OID_PKCS_5          OID_PKCS, 5 
#define OID_PKCS_5_LENGTH   OID_PKCS_LENGTH +1
#define OID_PKCS_6          OID_PKCS, 6 
#define OID_PKCS_7          OID_PKCS, 7 
#define OID_PKCS_7_LENGTH   OID_PKCS_LENGTH +1

#define OID_PKCS_8          OID_PKCS, 8 
#define OID_PKCS_9          OID_PKCS, 9
#define OID_PKCS_9_LENGTH   OID_PKCS_LENGTH +1
#define OID_PKCS_10         OID_PKCS, 10


#define OID_DS              OID_ISO_CCITT_DIR_SERVICE /* Also in X.501 */
#define OID_DS_LENGTH       1

#define OID_ATTR_TYPE        OID_DS, 4                /* Also in X.501 */
#define OID_ATTR_TYPE_LENGTH OID_DS_LENGTH +1

#define OID_DSALG            OID_DS, 8                /* Also in X.501 */
#define OID_DSALG_LENGTH     OID_DS_LENGTH +1

#define OID_EXTENSION        OID_DS, 29               /* Also in X.501 */
#define OID_EXTENSION_LENGTH OID_DS_LENGTH +1

/* From PKIX 1 */
// last digit unknown, using 0 as placeholder 
// #define OID_PKIX             43, 6, 1, 5, ???
#define OID_PKIX             43, 6, 1, 5, 0
#define OID_PKIX_LENGTH      5

#define OID_APPL_TCP_PROTO   43, 6, 1, 2, 1, 27, 4
#define OID_APPL_TCP_PROTO_LENGTH   8

#define OID_DAP              OID_DS, 3, 1
#define OID_DAP_LENGTH       OID_DS_LENGTH +2

/* From x9.57 */
#define OID_OIW_LENGTH       2

#define OID_OIW_SECSIG        OID_OIW, 3
#define OID_OIW_SECSIG_LENGTH OID_OIW_LENGTH +1

#define OID_OIW_ALGORITHM    OID_OIW_SECSIG, 2
#define OID_OIW_ALGORITHM_LENGTH   OID_OIW_SECSIG_LENGTH +1

#define OID_OIWDIR           OID_OIW, 7, 2
#define OID_OIWDIR_LENGTH    OID_OIW_LENGTH +2

#define OID_OIWDIR_CRPT      OID_OIWDIR, 1

#define OID_OIWDIR_HASH      OID_OIWDIR, 2
#define OID_OIWDIR_HASH_LENGTH OID_OIWDIR_LENGTH +1

#define OID_OIWDIR_SIGN      OID_OIWDIR, 3
#define OID_OIWDIR_SIGN_LENGTH OID_OIWDIR_LENGTH +1

//#define OID_X9CM           OID_US, 10040 
//This DER encoding for 10040 should be re-verified
#define OID_X9CM             OID_US, 206, 38
#define OID_X9CM_MODULE      OID_X9CM, 1
#define OID_X9CM_INSTRUCTION OID_X9CM, 2
#define OID_X9CM_ATTR        OID_X9CM, 3


/* Intel CSSM */
#define INTEL 96, 134, 72, 1, 134, 248, 77 
#define INTEL_LENGTH 7

#define INTEL_CDSASECURITY         INTEL, 2
#define INTEL_CDSASECURITY_LENGTH  INTEL_LENGTH+1

#define INTEL_SEC_FORMATS           INTEL_CDSASECURITY, 1
#define INTEL_SEC_FORMATS_LENGTH    INTEL_CDSASECURITY_LENGTH + 1

#define INTEL_SEC_ALGS              INTEL_CDSASECURITY, 2, 5  
#define INTEL_SEC_ALGS_LENGTH       INTEL_CDSASECURITY_LENGTH + 2

#define INTEL_SEC_OBJECT_BUNDLE         INTEL_SEC_FORMATS, 4
#define INTEL_SEC_OBJECT_BUNDLE_LENGTH  INTEL_SEC_FORMATS_LENGTH + 1

#define INTEL_CERT_AND_PRIVATE_KEY_2_0  INTEL_SEC_OBJECT_BUNDLE, 1
#define INTEL_CERT_AND_PRIVATE_KEY_2_0_LENGTH  INTEL_SEC_OBJECT_BUNDLE_LENGTH + 1

#endif
