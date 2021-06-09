/*-----------------------------------------------------------------------
        File:   X509err.h
  
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

#ifndef _X509ERR_H
#define _X509ERR_H

#define CSSM_CL_INVALID_ENCODE_ALGORITHM_INPUT_DATA     CSSM_CL_PRIVATE_ERROR + 40
#define CSSM_CL_INVALID_ALGORITHM_DATA                  CSSM_CL_PRIVATE_ERROR + 41
#define CSSM_CL_INVALID_PARAMETER_DATA                  CSSM_CL_PRIVATE_ERROR + 42
#define CSSM_CL_ENCODE_ALGORITHM_FAIL                   CSSM_CL_PRIVATE_ERROR + 43
#define CSSM_CL_INVALID_ENCODE_NAME_INPUT_DATA          CSSM_CL_PRIVATE_ERROR + 44
#define CSSM_CL_ENCODE_NAME_FAIL                        CSSM_CL_PRIVATE_ERROR + 45
#define CSSM_CL_INVALID_ENCODE_KEY_INPUT_DATA           CSSM_CL_PRIVATE_ERROR + 46
#define CSSM_CL_ENCODE_KEY_FAIL                         CSSM_CL_PRIVATE_ERROR + 47
#define CSSM_CL_INVALID_ENCODE_EXTENSION_INPUT_DATA     CSSM_CL_PRIVATE_ERROR + 48
#define CSSM_CL_ENCODE_EXTENSION_FAIL                   CSSM_CL_PRIVATE_ERROR + 49
#define CSSM_CL_INVALID_ENCODE_EXTENSIONS_INPUT_DATA    CSSM_CL_PRIVATE_ERROR + 50
#define CSSM_CL_ENCODE_EXTENSIONS_FAIL                  CSSM_CL_PRIVATE_ERROR + 51
#define CSSM_CL_INVALID_ENCODE_TBSCERT_INPUT_DATA       CSSM_CL_PRIVATE_ERROR + 52
#define CSSM_CL_ENCODE_TBSCERT_FAIL                     CSSM_CL_PRIVATE_ERROR + 53
#define CSSM_CL_INVALID_ENCODE_SIGNATURE_INPUT_DATA     CSSM_CL_PRIVATE_ERROR + 54
#define CSSM_CL_ENCODE_SIGNATURE_FAIL                   CSSM_CL_PRIVATE_ERROR + 55
#define CSSM_CL_INVALID_ENCODE_CERT_INPUT_DATA          CSSM_CL_PRIVATE_ERROR + 56
#define CSSM_CL_ENCODE_CERT_FAIL                        CSSM_CL_PRIVATE_ERROR + 57

#define CSSM_CL_INVALID_DECODE_ALGORITHM_INPUT_DATA     CSSM_CL_PRIVATE_ERROR + 58
#define CSSM_CL_DECODE_ALGORITHM_FAIL                   CSSM_CL_PRIVATE_ERROR + 59
#define CSSM_CL_INVALID_DECODE_NAME_INPUT_DATA          CSSM_CL_PRIVATE_ERROR + 60
#define CSSM_CL_DECODE_NAME_FAIL                        CSSM_CL_PRIVATE_ERROR + 61
#define CSSM_CL_INVALID_DECODE_SPKI_INPUT_DATA          CSSM_CL_PRIVATE_ERROR + 101
#define CSSM_CL_DECODE_SPKI_FAIL                        CSSM_CL_PRIVATE_ERROR + 102
#define CSSM_CL_INVALID_DECODE_EXTENSION_INPUT_DATA     CSSM_CL_PRIVATE_ERROR + 62
#define CSSM_CL_DECODE_EXTENSION_FAIL                   CSSM_CL_PRIVATE_ERROR + 63
#define CSSM_CL_INVALID_DECODE_EXTENSIONS_INPUT_DATA    CSSM_CL_PRIVATE_ERROR + 64
#define CSSM_CL_DECODE_EXTENSIONS_FAIL                  CSSM_CL_PRIVATE_ERROR + 65
#define CSSM_CL_INVALID_DECODE_TBSCERT_INPUT_DATA       CSSM_CL_PRIVATE_ERROR + 66
#define CSSM_CL_DECODE_VALIDITY_FAIL                    CSSM_CL_PRIVATE_ERROR + 67
#define CSSM_CL_DECODE_KEY_FAIL                         CSSM_CL_PRIVATE_ERROR + 68
#define CSSM_CL_DECODE_TBSCERT_FAIL                     CSSM_CL_PRIVATE_ERROR + 69
#define CSSM_CL_DECODE_SIGNATURE_FAIL                   CSSM_CL_PRIVATE_ERROR + 70
#define CSSM_CL_INVALID_DECODE_CERT_INPUT_DATA          CSSM_CL_PRIVATE_ERROR + 71
#define CSSM_CL_DECODE_CERT_FAIL                        CSSM_CL_PRIVATE_ERROR + 72
#define CSSM_CL_INVALID_DECODE_S_OBJECT_INPUT_DATA      CSSM_CL_PRIVATE_ERROR + 103
#define CSSM_CL_DECODE_S_OBJECT_FAIL                    CSSM_CL_PRIVATE_ERROR + 104
#define CSSM_CL_INVALID_DECODE_OBJECT_INPUT_DATA        CSSM_CL_PRIVATE_ERROR + 105
#define CSSM_CL_DECODE_OBJECT_FAIL                      CSSM_CL_PRIVATE_ERROR + 106
#define CSSM_CL_INVALID_DECODE_CERT_REQ_INPUT_DATA      CSSM_CL_PRIVATE_ERROR + 107
#define CSSM_CL_DECODE_CERT_REQ_FAIL                    CSSM_CL_PRIVATE_ERROR + 108

/* New in 1.2 */
#define CSSM_CL_INVALID_ENCODE_REVOKED_ENTRY_INPUT_DATA CSSM_CL_PRIVATE_ERROR + 73
#define CSSM_CL_ENCODE_REVOKED_ENTRY_FAIL               CSSM_CL_PRIVATE_ERROR + 74
#define CSSM_CL_INVALID_ENCODE_REVOKED_LIST_INPUT_DATA  CSSM_CL_PRIVATE_ERROR + 75
#define CSSM_CL_ENCODE_REVOKED_LIST_FAIL                CSSM_CL_PRIVATE_ERROR + 76
#define CSSM_CL_INVALID_ENCODE_TBSCERTLIST_INPUT_DATA   CSSM_CL_PRIVATE_ERROR + 77
#define CSSM_CL_INVALID_ENCODE_TBSCERTLIST_FAIL         CSSM_CL_PRIVATE_ERROR + 78
#define CSSM_CL_INVALID_ENCODE_SIGNED_CRL_INPUT_DATA    CSSM_CL_PRIVATE_ERROR + 79
#define CSSM_CL_INVALID_ENCODE_CERT_GROUP_INPUT_DATA    CSSM_CL_PRIVATE_ERROR + 109
#define CSSM_CL_ENCODE_CERT_GROUP_FAIL                  CSSM_CL_PRIVATE_ERROR + 110
#define CSSM_CL_INVALID_ENCODE_PKCS7_INPUT_DATA         CSSM_CL_PRIVATE_ERROR + 111
#define CSSM_CL_ENCODE_PKCS7_FAIL                       CSSM_CL_PRIVATE_ERROR + 112
#define CSSM_CL_INVALID_ENCODE_CRL_GROUP_INPUT_DATA     CSSM_CL_PRIVATE_ERROR + 113
#define CSSM_CL_ENCODE_CRL_GROUP_FAIL                   CSSM_CL_PRIVATE_ERROR + 114
#define CSSM_CL_INVALID_ENCODE_ARRAY_INPUT_DATA         CSSM_CL_PRIVATE_ERROR + 115
#define CSSM_CL_ENCODE_ARRAY_FAIL                       CSSM_CL_PRIVATE_ERROR + 116
#define CSSM_CL_INVALID_ENCODE_SIGNER_INFO_INPUT_DATA   CSSM_CL_PRIVATE_ERROR + 117
#define CSSM_CL_ENCODE_SIGNER_INFO_FAIL                 CSSM_CL_PRIVATE_ERROR + 118
#define CSSM_CL_INVALID_ENCODE_INPUT_DATA               CSSM_CL_PRIVATE_ERROR + 119 
#define CSSM_CL_ENCODE_FAIL                             CSSM_CL_PRIVATE_ERROR + 120 

#define CSSM_CL_INVALID_DECODE_CRL_INPUT_DATA           CSSM_CL_PRIVATE_ERROR + 80
#define CSSM_CL_DECODE_CRL_FAIL                         CSSM_CL_PRIVATE_ERROR + 81
#define CSSM_CL_INVALID_DECODE_TBSCERTLIST_INPUT_DATA   CSSM_CL_PRIVATE_ERROR + 82
#define CSSM_CL_DECODE_TBS_CERTLIST_FAIL                CSSM_CL_PRIVATE_ERROR + 83
#define CSSM_CL_INVALID_DECODE_REVOKED_LIST_INPUT_DATA  CSSM_CL_PRIVATE_ERROR + 84
#define CSSM_CL_INVALID_INPUT_DATA                      CSSM_CL_PRIVATE_ERROR + 85
#define CSSM_CL_INVALID_DECODE_REVOKED_ENTRY_INPUT_DATA CSSM_CL_PRIVATE_ERROR + 86
#define CSSM_CL_DECODE_REVOKED_ENTRY_FAIL               CSSM_CL_PRIVATE_ERROR + 87
#define CSSM_CL_DECODE_REVOKED_LIST_FAIL                CSSM_CL_PRIVATE_ERROR + 88
#define CSSM_CL_INVALID_DECODE_INPUT_DATA               CSSM_CL_PRIVATE_ERROR + 121 
#define CSSM_CL_DECODE_FAIL                             CSSM_CL_PRIVATE_ERROR + 122 
#define CSSM_CL_PKCS7_DECODE_FAIL                       CSSM_CL_PRIVATE_ERROR + 123 

#define CSSM_CL_INVALID_ENCODE_ATTRS_INPUT_DATA         CSSM_CL_PRIVATE_ERROR + 89
#define CSSM_CL_ENCODE_ATTRIBUTES_FAIL                  CSSM_CL_PRIVATE_ERROR + 90

#define CSSM_CL_INVALID_LISTID                          CSSM_CL_PRIVATE_ERROR + 91
#define CSSM_CL_INVALID_LLIST_HANDLE                    CSSM_CL_PRIVATE_ERROR + 92
#define CSSM_CL_STORE_LIST_FAIL                         CSSM_CL_PRIVATE_ERROR + 93
#define CSSM_CL_ADD_NODE_FAIL                           CSSM_CL_PRIVATE_ERROR + 94
#define CSSM_CL_RETRIEVE_LIST_FAIL                      CSSM_CL_PRIVATE_ERROR + 95

#define CSSM_CL_OPERATION_FAIL                          CSSM_CL_PRIVATE_ERROR + 96

#define CSSM_CL_KEY_TRANFORMATION_FAIL                  CSSM_CL_PRIVATE_ERROR + 98
#define CSSM_CL_ENCODE_OBJECT_FAIL                      CSSM_CL_PRIVATE_ERROR + 100
#define CSSM_CLUTIL_INVALID_INITIALIZATION              CSSM_CL_PRIVATE_ERROR + 126 

/* New in PreOS environment */
#define CSSM_CL_NO_MORE_FIELDS             ( CSSM_CL_PRIVATE_ERROR + 150 ) 


#endif
