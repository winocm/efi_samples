/*-----------------------------------------------------------------------
 *      File:   pkcs7oid.h
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
    
#include "oidsBase.h"    

/*
**  intel cdsa object identifiers for pkcs#7 signature format extensions
*/
#define RSAOID              0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d
#define RSALEN              6

#define PKCSOID             RSAOID, 1
#define PKCSLEN             RSALEN + 1

#define INTELOID            0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x4d
#define INTELLEN            7

#define INTELCDSAOID        INTELOID, 2
#define INTELCDSALEN        INTELLEN + 1

#define INTELCERTFORMATOID  INTELCDSAOID, 1, 1
#define INTELCERTFORMATLEN  INTELCDSALEN + 2

#define INTELAGGSIGOID      INTELCDSAOID, 1, 3, 1
#define INTELAGGSIGLEN      INTELCDSALEN + 3

#define INTELGETDATAALGOID  INTELAGGSIGOID, 1
#define INTELGETDATAALGLEN  INTELAGGSIGLEN + 1

#define INTELGETCERTALGOID  INTELAGGSIGOID, 2
#define INTELGETCERTALGLEN  INTELAGGSIGLEN + 1

#define INTELSIGFORMATOID   INTELAGGSIGOID, 3
#define INTELSIGFORMATLEN   INTELAGGSIGLEN + 1

#define INTELDIGESTALGOID   INTELAGGSIGOID, 4
#define INTELDIGESTALGLEN   INTELAGGSIGLEN + 1

#define INTELSIGALGOID      INTELAGGSIGOID, 5
#define INTELSIGALGLEN      INTELAGGSIGLEN + 1

/* get data algorithms */
#define INTELSIGGETDATAEMBEDOID     INTELGETDATAALGOID, 1
#define INTELSIGGETDATAEMBEDLEN     INTELGETDATAALGLEN + 1

#define INTELSIGGETDATAFILEOID      INTELGETDATAALGOID, 2
#define INTELSIGGETDATAFILELEN      INTELGETDATAALGLEN + 1

#define INTELSIGGETDATAURLOID       INTELGETDATAALGOID, 3
#define INTELSIGGETDATAURLLEN       INTELGETDATAALGLEN + 1

#define INTELSIGGETDATAFILEREFOID   INTELGETDATAALGOID, 4
#define INTELSIGGETDATAFILEREFLEN   INTELGETDATAALGLEN + 1

#define INTELSIGGETDATAURLREFOID    INTELGETDATAALGOID, 5
#define INTELSIGGETDATAURLREFLEN    INTELGETDATAALGLEN + 1

/* From PKCS 7 */
static uint8 
OID_data[]          = { OID_PKCS_7, 1  },
OID_signedData[]    = { OID_PKCS_7, 2  },
OID_envelopedData[] = { OID_PKCS_7, 3  },
OID_signEnvData[]   = { OID_PKCS_7, 4  },
OID_digestedData[]  = { OID_PKCS_7, 5  },
OID_encryptedData[] = { OID_PKCS_7, 6  };

static CSSM_OID
    CSSMOID_Pkcs7Data                    = {OID_PKCS_7_LENGTH+1, OID_data},
    CSSMOID_Pkcs7SignedData              = {OID_PKCS_7_LENGTH+1, OID_signedData},
    CSSMOID_Pkcs7EnvelopedData           = {OID_PKCS_7_LENGTH+1, OID_envelopedData},
    CSSMOID_Pkcs7SignedAndEnvelopedData  = {OID_PKCS_7_LENGTH+1, OID_signEnvData},
    CSSMOID_Pkcs7DigestedData            = {OID_PKCS_7_LENGTH+1, OID_digestedData},
    CSSMOID_Pkcs7EncryptedData           = {OID_PKCS_7_LENGTH+1, OID_encryptedData}
;
