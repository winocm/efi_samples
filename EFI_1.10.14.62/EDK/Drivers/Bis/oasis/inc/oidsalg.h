/*-----------------------------------------------------------------------
 *       File:   oidsalg.h
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

#ifndef _OIDSALG_H
#define _OIDSALG_H

#include "cssm.h"
#include "oidsbase.h"

/* From PKCS 1 */
static uint8
OID_MD2[]          = { OID_RSA_HASH, 2 },
OID_MD5[]          = { OID_RSA_HASH, 5 };

static uint8
OID_RSAEncryption[]= { OID_PKCS_1, 1 },
OID_MD2WithRSA[]   = { OID_PKCS_1, 2 },
OID_MD5WithRSA[]   = { OID_PKCS_1, 4 },
OID_SHA1WithRSA[]  = { OID_PKCS_1, 5 };

/* From PKCS 3 */
static uint8    
OID_DHKeyAgreement[] = { OID_PKCS_3, 1 }; 

/* NIST OSE Implementors' Workshop (OIW)
 * Security SIG algorithm identifiers
 * http://nemo.ncsl.nist.gov/oiw/agreements/stable/OSI/12s_9506.w51
 * http://nemo.ncsl.nist.gov/oiw/agreements/working/OSI/12w_9503.w51
 */
static uint8
OID_OIW_DSA[]     = { OID_OIW_ALGORITHM, 12  },     /* From x9.57 */ 
OID_OIW_SHA1[]    = { OID_OIW_ALGORITHM, 26  },     /* From x9.57 */
OID_OIW_DSAWithSHA1[] = { OID_OIW_ALGORITHM, 27  }; /* From x9.57 */


/* Intel OIDs for algorithms that do not have OIDs */
static uint8
DES [] = {INTEL_SEC_ALGS, 13}; /* Data Encryption Standard block cipher */

static CSSM_OID
    CSSMOID_DH      = {OID_PKCS_3_LENGTH+1, OID_DHKeyAgreement},
    CSSMOID_MD2     = {OID_RSA_HASH_LENGTH+1, OID_MD2},
    CSSMOID_MD5     = {OID_RSA_HASH_LENGTH+1, OID_MD5},
    CSSMOID_SHA1    = {OID_OIW_ALGORITHM_LENGTH+1, OID_OIW_SHA1},
    CSSMOID_DES     = {INTEL_SEC_ALGS_LENGTH+1, DES},
    CSSMOID_RSA     = {OID_PKCS_1_LENGTH+1, OID_RSAEncryption}, 
    CSSMOID_DSA     = {OID_OIW_ALGORITHM_LENGTH+1, OID_OIW_DSA}, 
    CSSMOID_MD5WithRSA    = {OID_PKCS_1_LENGTH+1, OID_MD5WithRSA},
    CSSMOID_MD2WithRSA    = {OID_PKCS_1_LENGTH+1, OID_MD2WithRSA},
    CSSMOID_SHA1WithRSA   = {OID_PKCS_1_LENGTH+1, OID_SHA1WithRSA}, 
    CSSMOID_SHA1WithDSA   = {OID_OIW_ALGORITHM_LENGTH+1, OID_OIW_DSAWithSHA1}
;

/****************************************************************************/
/* These definitions have been removed from the size-reduced implementation */
/****************************************************************************/
#if 0

/* From PKCS 1 */
static uint8
OID_MD4[]          = { OID_RSA_HASH, 4 }, 

static uint8
OID_MD4WithRSA[]   = { OID_PKCS_1, 3 },
OID_SETOAEP_RSA[]  = { OID_PKCS_1, 6 };

/* From PKCS 5 */
static uint8    
OID_PBEWithMD2AndDES_CBC[] = { OID_PKCS_5, 1 },
OID_PBEWithMD5AndDES_CBC[] = { OID_PKCS_5, 3 }; 


/* From Microsoft CAPI 2.0 API reference */
//NIST OSE Implementors' Workshop (OIW)
//http://nemo.ncsl.nist.gov/oiw/agreements/stable/OSI/12s_9506.w51
//http://nemo.ncsl.nist.gov/oiw/agreements/working/OSI/12w_9503.w51

//NIST OSE Implementors' Workshop (OIW) Security SIG algorithm //identifiers
static uint8
OID_OIW_md4RSA[]  = { OID_OIW_ALGORITHM, 2  },
OID_OIW_md5RSA[]  = { OID_OIW_ALGORITHM, 3  },
OID_OIW_md4RSA2[] = { OID_OIW_ALGORITHM, 4  },
OID_OIW_desECB[]  = { OID_OIW_ALGORITHM, 6  },
OID_OIW_desCBC[]  = { OID_OIW_ALGORITHM, 7  },
OID_OIW_desOFB[]  = { OID_OIW_ALGORITHM, 8  },
OID_OIW_desCFB[]  = { OID_OIW_ALGORITHM, 9  },
OID_OIW_desMAC[]  = { OID_OIW_ALGORITHM, 10  },
OID_OIW_RSASignature[] = { OID_OIW_ALGORITHM, 11  },/* From x9.57 */
OID_OIW_DSAWithSHA[]  = { OID_OIW_ALGORITHM, 13  }, /* From x9.57 */
OID_OIW_mdc2RSA[] = { OID_OIW_ALGORITHM, 14  },
OID_OIW_shaRSA[]  = { OID_OIW_ALGORITHM, 15  },
OID_OIW_dhCommMod[]  = { OID_OIW_ALGORITHM, 16  },
OID_OIW_desEDE[]  = { OID_OIW_ALGORITHM, 17  },
OID_OIW_SHA[]     = { OID_OIW_ALGORITHM, 18  },     /* From x9.57 */
OID_OIW_mdc2[]    = { OID_OIW_ALGORITHM, 19  },
OID_OIW_dsaComm[] = { OID_OIW_ALGORITHM, 20  },
OID_OIW_dsaCommSHA[] = { OID_OIW_ALGORITHM, 21  },
OID_OIW_rsaXchg[] = { OID_OIW_ALGORITHM, 22  },
OID_OIW_keyHashSeal[]= { OID_OIW_ALGORITHM, 23  },
OID_OIW_md2RSASign[] = { OID_OIW_ALGORITHM, 24  },
OID_OIW_md5RSASign[] = { OID_OIW_ALGORITHM, 25  },
OID_OIW_dsaCommSHA1[]= { OID_OIW_ALGORITHM, 28  },
OID_OIW_sha1RSASign[]= { OID_OIW_ALGORITHM, 29  };

//NIST OSE Implementors' Workshop (OIW) Directory SIG algorithm //identifiers
static uint8
OID_OIWDIR_md2[]     = { OID_OIWDIR_HASH, 1 },
OID_OIWDIR_md2RSA[]  = { OID_OIWDIR_SIGN, 1 }; 

/* ITU-T UsefulDefinitions */
static uint8
OID_DSALG_CRPT[]  = { OID_DSALG, 1  },
OID_DSALG_HASH[]  = { OID_DSALG, 2  },
OID_DSALG_SIGN[]  = { OID_DSALG, 3  },
OID_DSALG_RSA[]   = { OID_DSALG, 1, 1 };

/* From Microsoft CAPI 2.0 API reference */
static uint8 
OID_RC2CBC[]  = { OID_RSA_ENCRYPT, 2  },
OID_RC4[]     = { OID_RSA_ENCRYPT, 4  },
OID_RC5[]     = { OID_RSA_ENCRYPT, 5  };  /* Note: This OID is assumed */

static uint8
DH[]       OID_DHKeyAgreement     /*Diffie Hellman key exchange algorithm */
PH []       = {INTEL_SEC_ALGS, 2},  /*Pohlig Hellman key exchange algorithm */
KEA[]       = {INTEL_SEC_ALGS, 3},  /*Key Exchange Algorithm */
MD2[]     OID_MD2
MD4[]     OID_MD4
MD5[]     OID_MD5
SHA1[]    OID_OIW_SHA1            /*Secure Hash Algorithm  (developed by NIST/NSA) */
NHASH[]     = {INTEL_SEC_ALGS, 8},  /*N-Hash algorithm(developed by Nippon Telephone and Telegraph) */
HAVAL[]     = {INTEL_SEC_ALGS, 9},  /*HAVAL  hash algorithm  (MD5 variant) */
RIPEMD []   = {INTEL_SEC_ALGS, 10}, /*RIPE-MD  hash algorithm  (MD4 variant - developed for the European Community's RIPE project) */
IBCHASH []  = {INTEL_SEC_ALGS, 11}, /*IBC-Hash (keyed hash algorithm or MAC) */
RIPEMAC[]   = {INTEL_SEC_ALGS, 12}, /*RIPE-MAC (invented by Bart Preneel) */
DESX []     = {INTEL_SEC_ALGS, 14}, /*DESX block cipher  (DES variant from RSA) */
RDES []     = {INTEL_SEC_ALGS, 15}, /*RDES block cipher  (DES variant) */
ALG_3DES_3KEY[] = {INTEL_SEC_ALGS, 16}, /*Triple-DES block cipher  (with 3 keys) */
ALG_3DES_2KEY[] = {INTEL_SEC_ALGS, 17}, /*Triple-DES block cipher  (with 2 keys) */
ALG_3DES_1KEY[] = {INTEL_SEC_ALGS, 18}, /*Triple-DES block cipher  (with 1 key) */
IDEA []     = {INTEL_SEC_ALGS, 19}, /*IDEA block cipher  (invented by Lai and Massey) */
RC2[]     OID_RC2CBC
RC5[]     OID_RC5
RC4[]     OID_RC4
SEAL[]      = {INTEL_SEC_ALGS, 23}, /*SEAL stream cipher  (invented by Rogaway and Coppersmith) */
CAST[]      = {INTEL_SEC_ALGS, 24}, /*CAST block cipher  (invented by Adams and Tavares) */
BLOWFISH [] = {INTEL_SEC_ALGS, 25}, /*BLOWFISH block cipher  (invented by Schneier) */
SKIPJACK[]  = {INTEL_SEC_ALGS, 26}, /*Skipjack block cipher  (developed by NSA) */
LUCIFER []  = {INTEL_SEC_ALGS, 27}, /*Lucifer block cipher  (developed by IBM) */
MADRYGA []  = {INTEL_SEC_ALGS, 28}, /*Madryga block cipher  (invented by Madryga) */
FEAL []     = {INTEL_SEC_ALGS, 29}, /*FEAL block cipher  (invented by Shimizu and Miyaguchi) */
REDOC []    = {INTEL_SEC_ALGS, 30}, /*REDOC 2 block cipher  (invented by Michael Wood) */
REDOC3[]    = {INTEL_SEC_ALGS, 31}, /*REDOC 3 block cipher  (invented by Michael Wood) */
LOKI[]      = {INTEL_SEC_ALGS, 32}, /*LOKI block cipher */
KHUFU []    = {INTEL_SEC_ALGS, 33}, /*KHUFU block cipher  (invented by Ralph Merkle) */
KHAFRE []   = {INTEL_SEC_ALGS, 34}, /*KHAFRE block cipher  (invented by Ralph Merkle) */
MMB[]       = {INTEL_SEC_ALGS, 35}, /*MMB block cipher  (IDEA variant) */
GOST[]      = {INTEL_SEC_ALGS, 36}, /*GOST block cipher  (developed by the former Soviet Union) */
SAFER[]     = {INTEL_SEC_ALGS, 37}, /*SAFER K-64 block cipher  (invented by Massey) */
CRAB[]      = {INTEL_SEC_ALGS, 38}, /*CRAB block cipher  (invented by Kaliski and Robshaw) */
RSA[]     OID_RSAEncryption
DSA[]     OID_OIW_DSA     /*Digital Signature Algorithm */
MD5WithRSA[]  OID_MD5WithRSA
MD2WithRSA[]  OID_MD2WithRSA
ElGamal[]   = {INTEL_SEC_ALGS, 43}, /* ElGamal signature algorithm */
MD2Random[] = {INTEL_SEC_ALGS, 44}, /*MD2-based random numbers */
MD5Random[] = {INTEL_SEC_ALGS, 45}, /*MD5-based random numbers */
SHARandom[] = {INTEL_SEC_ALGS, 46}, /*SHA-based random numbers */
DESRandom[] = {INTEL_SEC_ALGS, 47},  /*DES-based random numbers */
SHA1WithRSA[] OID_SHA1WithRSA      /* SHA1/RSA signature algorithm */
RSA_PKCS[]    OID_RSAEncryption    /* RSA as specified in PKCS #1 */
RSA_ISO9796[] = {INTEL_SEC_ALGS, 49}, /* RSA as specified in ISO 9796 */
RSA_RAW[]   = {INTEL_SEC_ALGS, 50}, /* Raw RSA as assumed in X.509 */
CDMF[]      = {INTEL_SEC_ALGS, 51}, /* CDMF block cipher */
CAST3[]     = {INTEL_SEC_ALGS, 52}, /* Entrust's CAST3 block cipher */
CAST5[]     = {INTEL_SEC_ALGS, 53}, /* Entrust's CAST5 block cipher */
GenericSecret[]     = {INTEL_SEC_ALGS, 54}, /* Generic secret operations */
ConcatBaseAndKey[]  = {INTEL_SEC_ALGS, 55}, /* Concatenate two keys, base key first */
ConcatKeyAndBase[]  = {INTEL_SEC_ALGS, 56}, /* Concatenate two keys, base key last */
ConcatBaseAndData[] = {INTEL_SEC_ALGS, 57}, /* Concatenate base key and random data, key first */
ConcatDataAndBase[] = {INTEL_SEC_ALGS, 58}, /* Concatenate base key and data, data first */
XORBaseAndData[]    = {INTEL_SEC_ALGS, 59}, /* XOR a byte string with the base key */
ExtractFromKey[]    = {INTEL_SEC_ALGS, 60}, /* Extract a key from base key, starting at arbitrary bit position */
SSL3PreMasterGen[]  = {INTEL_SEC_ALGS, 61}, /* Generate a 48 byte SSL 3 pre-master key */
SSL3MasterDerive[]  = {INTEL_SEC_ALGS, 62}, /* Derive an SSL 3 key from a pre-master key */
SSL3KeyAndMacDerive[] = {INTEL_SEC_ALGS, 63}, /* Derive the keys and MACing keys for the SSL cipher suite */
SSL3MD5_MAC[]       = {INTEL_SEC_ALGS, 64}, /* Performs SSL 3 MD5 MACing */
SSL3SHA1_MAC[]      = {INTEL_SEC_ALGS, 65}, /* Performs SSL 3 SHA-1 MACing */
MD5Derive[]         = {INTEL_SEC_ALGS, 66}, /* Generate key by MD5 hashing a base key */
MD2Derive[]         = {INTEL_SEC_ALGS, 67}, /* Generate key by MD2 hashing a base key */
SHA1Derive[]        = {INTEL_SEC_ALGS, 68}, /* Generate key by SHA-1 hashing a base key */
WrapLynks[]         = {INTEL_SEC_ALGS, 69}, /* Spyrus LYNKS DES based wrapping scheme w/checksum */
WrapSET_OAEP[] OID_SETOAEP_RSA
BATON[]             = {INTEL_SEC_ALGS, 71}, /* Fortezza BATON cipher */
ECDSA[]             = {INTEL_SEC_ALGS, 72}, /* Elliptic Curve DSA */
MAYFLY[]            = {INTEL_SEC_ALGS, 73}, /* Fortezza MAYFLY cipher */
JUNIPER[]           = {INTEL_SEC_ALGS, 74}, /* Fortezza JUNIPER cipher */
FASTHASH[]          = {INTEL_SEC_ALGS, 75}, /* Fortezza FASTHASH */
ALG_3DES[]	        = {INTEL_SEC_ALGS, 76}, /* Generic 3DES */
SSL3MD5[]	        = {INTEL_SEC_ALGS, 77}, /* SSL3MD5 */
SSL3SHA1[]	        = {INTEL_SEC_ALGS, 78}, /* SSL3SHA1 */
FortezzaTimestamp[] = {INTEL_SEC_ALGS, 79}, /* FortezzaTimestamp */
SHA1WithDSA[]     OID_OIW_DSAWithSHA1 /* SHA1WithDSA */
SHA1WithECDSA[]     = {INTEL_SEC_ALGS, 81}, /* SHA1WithECDSA */
DSA_BSAFE[]         = {INTEL_SEC_ALGS, 82},
ECDH[]              = {INTEL_SEC_ALGS, 83}, /* Elliptic Curve DiffieHellman Key Exchange algorithm*/
ECMQV[]             = {INTEL_SEC_ALGS, 84}, /* Elliptic Curve MQV key exchange algorithm*/
PKCS12_SHA1_PBE[]   = {INTEL_SEC_ALGS, 85}, /* PKCS12 SHA-1 Password key derivation algorithm*/
ECNRA[]             = {INTEL_SEC_ALGS, 86}, /* Elliptic Curve Nyberg-Rueppel*/
SHA1WithECNRA[]     = {INTEL_SEC_ALGS, 87}, /* SHA-1 with Elliptic Curve Nyberg-Rueppel*/
ECES[]              = {INTEL_SEC_ALGS, 88}, /* Elliptic Curve Encryption Scheme*/
ECAES[]             = {INTEL_SEC_ALGS, 89}, /* Elliptic Curve Authenticate Encryption Scheme*/
SHA1HMAC[]          = {INTEL_SEC_ALGS, 90}, /* SHA1-MAC*/
FIPS186Random[]     = {INTEL_SEC_ALGS, 91}, /* FIPs86Random*/
ECC[]               = {INTEL_SEC_ALGS, 92};  /* ECC*/

#endif

#endif
