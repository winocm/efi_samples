/*-----------------------------------------------------------------------
        File:   X509defs.h
  
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

#ifndef _X509DEFS_H
#define _X509DEFS_H
#include "cssm.h"

#define UTC_TIME_STRLEN 13
#define GENERALIZED_TIME_STRLEN 15

typedef enum cl_der_tag_type {
    DER_TAG_UNKNOWN      = 0x00,
    DER_TAG_BOOLEAN      = 0x01,
	DER_TAG_INTEGER      = 0x02,
    DER_TAG_BIT_STRING   = 0x03,
    DER_TAG_OCTET_STRING = 0x04,
    DER_TAG_NULL         = 0x05,
    DER_TAG_OID          = 0x06,
    /* Unsupported:
    DER_TAG_OBJDESCRIPTOR= 0x07,
    DER_TAG_EXTERNAL     = 0x08,
    DER_TAG_REAL         = 0x09,
    DER_TAG_ENUMERATED   = 0x0A,
    DER_TAG_EMBEDDED_PDV = 0x0B,
    */
    DER_TAG_SEQUENCE     = 0x10,
    DER_TAG_SET          = 0x11,
    /* Unsupported:
    DER_TAG_NUMERIC_STRING = 0x12,
    */
    DER_TAG_PRINTABLE_STRING = 0x13,
    DER_TAG_T61_STRING   = 0x14,
    /* Unsupported:
    DER_TAG_VIDEOTEX_STRING = 0x15,
    */
    DER_TAG_IA5_STRING   = 0x16,
    DER_TAG_UTC_TIME     = 0x17,
    DER_TAG_GENERALIZED_TIME = 0x18,
    /* Unsupported:
    DER_TAG_GRAPHIC_STRING = 0x19,
    DER_TAG_VISIBLE_STRING = 0x1A,
    DER_TAG_GENERAL_STRING = 0x1B,
    DER_TAG_UNIVERSAL_STRING = 0x1C,
    DER_TAG_CHARACTER_STRING = 0x1D,
    DER_TAG_BMP_STRING   = 0x1E,
    */
} CL_DER_TAG_TYPE;

/* the X.509 algorithm identifier */
typedef struct x509_algorithm_identifier {
    CSSM_OID        algorithm;        
	CSSM_DATA       parameters;		    /* optional */
} X509_ALGORITHM_IDENTIFIER, *X509_ALGORITHM_IDENTIFIER_PTR;

/* X509 Distinguished name structure */
typedef struct x509_type_value_pair {
    CSSM_OID    type;
    CL_DER_TAG_TYPE  valueType;   /* The Tag to be used when this value is BER encoded */
    CSSM_DATA   value;
} X509_TYPE_VALUE_PAIR, *X509_TYPE_VALUE_PAIR_PTR;

typedef struct x509_rdn {
    uint32          numberOfPairs;
    X509_TYPE_VALUE_PAIR_PTR AttributeTypeAndValue;
} X509_RDN, *X509_RDN_PTR;

typedef struct x509_name {
    uint32          numberOfRDNs;
    X509_RDN_PTR    RelativeDistinguishedName;
} X509_NAME, *X509_NAME_PTR;

/* Public key info struct */
typedef struct x509_subject_public_key_info {
    X509_ALGORITHM_IDENTIFIER   algorithm;          /* as described above */
    CSSM_DATA                   subjectPublicKey;   
} X509_SUBJECT_PUBLIC_KEY_INFO, *X509_SUBJECT_PUBLIC_KEY_INFO_PTR;

/* Validity struct */
typedef char *X509_TIME;
typedef struct x509_validity {
    X509_TIME notBefore;
    X509_TIME notAfter;
} X509_VALIDITY, *X509_VALIDITY_PTR;

/* Extension structure */
typedef struct x509_extension {
	CSSM_OID				extnId;			/* extension unique ID */
	CSSM_BOOL				critical;       /* Criticality flag:If critical and user */
											/* does not understand, means invalid cert. */
											/* if not critical,user can continue processing */
											/* the cert even if the extension is not understood */
    CL_DER_TAG_TYPE         extnType;       /* The Tag to be used when this extension is BER encoded */
	CSSM_DATA               extnValue;		/* extension Data */
} X509_EXTENSION, *X509_EXTENSION_PTR;

/* X509V3 certificate structure */
typedef struct x509_certificate {
	CSSM_DATA  					version;			/* Certificate Version optional [0], type DER Integer */
	CSSM_DATA  					serialNumber;		/* Serial number of certificate, type DER Integer */
    X509_ALGORITHM_IDENTIFIER   signature;          /* The Signature algorithm */
    X509_NAME					issuer;			    /* Distinguished name fields of issuer */
    X509_VALIDITY               validity;           /* Validity date of certificate */
    X509_NAME                   subject;    		/* Distinguished name fields of subject */     
													
    X509_SUBJECT_PUBLIC_KEY_INFO  subjectPublicKeyInfo;  /* Public key of subject */
    CSSM_DATA					issuerUniqueIdentifier;  /* Issuer's  ID optional [1] */
    CSSM_DATA					subjectUniqueIdentifier; /* Subject's ID optional [2] */
    uint32                      numberOfExtensions;
    X509_EXTENSION_PTR          extensions;         /* Sequence of extensions optional [3] */
}X509_CERTIFICATE, *X509_CERTIFICATE_PTR;
          
/* Signature structure */
typedef struct x509_signature {
    X509_ALGORITHM_IDENTIFIER   algorithmIdentifier;
    CSSM_DATA                   encrypted;
} X509_SIGNATURE, *X509_SIGNATURE_PTR;
     

/* Signed certificate structure
   This structure is passed in most CSSM functions which require a certificate */
typedef struct x509_signed_certificate {  
	X509_CERTIFICATE		certificate;    /* x.509 info */
	X509_SIGNATURE          signature;	    /* the signature */
} X509_SIGNED_CERTIFICATE, *X509_SIGNED_CERTIFICATE_PTR;

#define INTEL_X509V3_PASSTHROUGH_LAST 42

/* PassThrough Operation Ids */
typedef enum intel_x509v3_passthrough_operation_id {
/*    INTEL_X509V3_PASSTHROUGH_ENCODE_CERTIFICATE	= 1, Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DECODE_CERTIFICATE	= 2, Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_FREE_CERTIFICATE   = 3, Not in PreOS implementation */ 

/*    INTEL_X509V3_PASSTHROUGH_CREATE_ENCODED_NAME = 14,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_ENCODE_NAME        = 4, Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DECODE_NAME    	= 5, Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_FREE_NAME          = 6, Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_TRANSLATE_DERNAME_TO_STRING = 7,Not in PreOS implementation */

/*    INTEL_X509V3_PASSTHROUGH_ENCODE_EXTENSION	= 8, Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DECODE_EXTENSION   = 9, Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_ENCODE_EXTENSIONS 	= 10,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DECODE_EXTENSIONS	= 11,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_FREE_EXTENSIONS    = 12, Not in PreOS implementation */

/*    INTEL_X509V3_PASSTHROUGH_ALGID_TO_ALGOID    = 15, Not in PreOS implementation */
    INTEL_X509V3_PASSTHROUGH_ALGOID_TO_ALGID    = 16,
/*    INTEL_X509V3_PASSTHROUGH_ENCODE_ALGID       = 33,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DECODE_ALGID       = 34,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_FREE_ALGID         = 35,Not in PreOS implementation */

/*    INTEL_X509V3_PASSTHROUGH_OPEN_FILE          = 36,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_CLOSE_FILE         = 37,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_WRITE_CERT_TO_FILE = 17,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_READ_CERT_FROM_FILE= 18,Not in PreOS implementation */
    
/*    INTEL_X509V3_PASSTHROUGH_ENCODE_SIGNED_CRL  = 19,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DECODE_SIGNED_CRL  = 20,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_FREE_SIGNED_CRL    = 21,Not in PreOS implementation */

/*    INTEL_X509V3_PASSTHROUGH_ENCODE_TBS_CERTLIST    = 22,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DECODE_TBS_CERTLIST    = 23,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_FREE_TBS_CERTLIST_DATA = 24,Not in PreOS implementation */

/*    INTEL_X509V3_PASSTHROUGH_ENCODE_REVOKED_CERTLIST= 25,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DECODE_REVOKED_CERTLIST= 26,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_FREE_REVOKED_CERTLIST  = 27,Not in PreOS implementation */

/*    INTEL_X509V3_PASSTHROUGH_ENCODE_REVOKED_CERT_ENTRY  = 28,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DECODE_REVOKED_CERT_ENTRY  = 29,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_FREE_REVOKED_CERT_ENTRY    = 30,Not in PreOS implementation */

    /* INTEL_X509V3_PASSTHROUGH_CERT_REQUEST  = 31, Made obsolete by 2.0 APIs */
    /* INTEL_X509V3_PASSTHROUGH_CERT_RETRIEVE = 32, Made obsolete by 2.0 APIs */
    /* INTEL_X509V3_PASSTHROUGH_CERT_CREATE_TEMPLATE = 41,  Not in PreOS implementation */
    /* INTEL_X509V3_PASSTHROUGH_CERT_SIGN = 42,             Not in PreOS implementation */

/*    INTEL_X509V3_PASSTHROUGH_FIND_SUPPORTING_CSP = 38,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_CSSMKEY_TO_SPKI     = 39,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_SPKI_TO_CSSMKEY     = 40,Not in PreOS implementation */

    /* PassThrough to Ber/Der primitives */
/*    INTEL_X509V3_PASSTHROUGH_DER_LIBRARY_ATTACH = 50,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DER_LIBRARY_DETACH = 51,Not in PreOS implementation */

/*    INTEL_X509V3_PASSTHROUGH_DER_ENCODE_OBJECT  = 52,Not in PreOS implementation */
    
/*    INTEL_X509V3_PASSTHROUGH_DER_CREATE_LEAF    = 53,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DER_CREATE_PARENT  = 54,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DER_PACK_TREE      = 55,Not in PreOS implementation */

/*    INTEL_X509V3_PASSTHROUGH_DER_DELETE_LEAF    = 56,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DER_DELETE_PARENT  = 57,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DER_DELETE_TREE    = 58,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DER_DELETE_TYPE    = 59,Not in PreOS implementation */

/*    INTEL_X509V3_PASSTHROUGH_DER_PARSE_OBJECT   = 60,Not in PreOS implementation */
/*    INTEL_X509V3_PASSTHROUGH_DER_DELETE_PARSED_OBJECT  = 61,Not in PreOS implementation */
} INTEL_X509V3_PASSTHROUGH_OPERATION_ID;

typedef struct x501_any_value {
    CL_DER_TAG_TYPE  valueTag;   /* The Tag to be used when this value is BER encoded */
    CSSM_DATA        value;
} X501_ANY_VALUE, *X501_ANY_VALUE_PTR;

typedef struct x501_attribute {
    CSSM_OID            type;
    uint32              numberOfValues;
    X501_ANY_VALUE_PTR  values;
} X501_ATTRIBUTE, *X501_ATTRIBUTE_PTR;

typedef struct x501_attributes {
    uint32              numberOfAttributes;
    X501_ATTRIBUTE_PTR  attributes;
} X501_ATTRIBUTES, *X501_ATTRIBUTES_PTR;


typedef struct cssm_crlgroup {
    uint32 NumCrls;
    CSSM_DATA_PTR CrlList;
    void* reserved;
} CSSM_CRLGROUP, *CSSM_CRLGROUP_PTR;

typedef X509_ALGORITHM_IDENTIFIER_PTR PKSC7_CONTENT_INFO_PTR;
typedef X509_ALGORITHM_IDENTIFIER     PKSC7_CONTENT_INFO;

typedef struct pkcs7_issuer_and_serial_number {
    X509_NAME issuer;
    CSSM_DATA serialNumber;
} PKCS7_ISSUER_AND_SERIAL_NUMBER, *PKCS7_ISSUER_AND_SERIAL_NUMBER_PTR;

typedef struct pkcs7_signer_info {
    CSSM_DATA                      version;              /* signer info version, default =1, type DER Integer */
    PKCS7_ISSUER_AND_SERIAL_NUMBER issuerAndSerialNumber;
    X509_ALGORITHM_IDENTIFIER      digestAlgId;
    X501_ATTRIBUTES                authenticatedAttrs;   /* Set, Implicit, Optional, CtxTag = 0 */
    X509_ALGORITHM_IDENTIFIER      digestEncryptionAlgId;
    CSSM_DATA                      encryptedDigest;      /* Octet string */
    X501_ATTRIBUTES                unauthenticatedAttrs; /* Set, Implicit, Optional, CtxTag = 0 */
} PKCS7_SIGNER_INFO, *PKCS7_SIGNER_INFO_PTR;

typedef struct pkcs7_signed_data {
    CSSM_DATA                     version;      /* pkcs#7 version, default =1, type DER Integer */
    uint32                        numberOfDigestAlgIds;
    X509_ALGORITHM_IDENTIFIER_PTR digestAlgIds; /* Set */
    PKSC7_CONTENT_INFO            contentInfo;
    CSSM_CERTGROUP                certificates; /* Set, Implicit, Optional, CtxTag = 0 */
    CSSM_CRLGROUP                 crls;         /* Set, Implicit, Optional, CtxTag = 1 */
    uint32                        numberOfSignerInfos;
    PKCS7_SIGNER_INFO_PTR         signerInfos;  /* Set */
} PKCS7_SIGNED_DATA, *PKCS7_SIGNED_DATA_PTR;


typedef struct cl_der_leaf_node {
    CL_DER_TAG_TYPE    Tag;
    CSSM_DATA		   Data;
} CL_DER_LEAF_NODE, *CL_DER_LEAF_NODE_PTR;


#endif /* _X509DEFS_H */
