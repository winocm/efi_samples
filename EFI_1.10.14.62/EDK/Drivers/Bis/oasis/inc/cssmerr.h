/* SCCSID: inc/cssmerr.h, dss_cdsa_fwk, fwk_rel1, rel1_level1 1.15 8/4/97 15:02:55 */
/*-----------------------------------------------------------------------
 *      File:   CSSMERR.H
 *
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

#ifndef _CSSMERR_H
#define _CSSMERR_H

#define CSSM_CSP_BASE_ERROR     ( 1000UL )    /* Defined by CSSM */
#define CSSM_CSP_PRIVATE_ERROR  ( 2000UL )    /* Defined by individual CSPs */
#define CSSM_CSP_END_ERROR      ( 2999UL )
#define CSSM_CL_BASE_ERROR      ( 3000UL )    /* Defined by CSSM */
#define CSSM_CL_PRIVATE_ERROR   ( 4000UL )    /* Definted by indiv CLs */
#define CSSM_CL_END_ERROR       ( 4999UL )
#define CSSM_DL_BASE_ERROR      ( 5000UL )    /* Defined by CSSM */
#define CSSM_DL_PRIVATE_ERROR   ( 6000UL )    /* Defined by indiv DBs */
#define CSSM_DL_END_ERROR       ( 6999UL )
#define CSSM_TP_BASE_ERROR      ( 7000UL )    /* Defined by CSSM */
#define CSSM_TP_PRIVATE_ERROR   ( 8000UL )    /* Defined by indiv TPs */
#define CSSM_TP_END_ERROR       ( 8999UL )

#define CSSM_BASE_ERROR         ( 10000UL )
#define CSSM_END_ERROR          ( 19999UL )

#define CSSM_VL_BASE_ERROR		( 20000UL )
#define CSSM_VL_PRIVATE_ERROR   ( 21000UL )
#define CSSM_VL_END_ERROR		( 21999UL )
           
/* General CSP messages and errors */
#define CSSM_CSP_UNKNOWN_ERROR               ( CSSM_CSP_BASE_ERROR +1L )
#define CSSM_CSP_REGISTER_ERROR              ( CSSM_CSP_BASE_ERROR +2L )
#define CSSM_CSP_VERSION_ERROR               ( CSSM_CSP_BASE_ERROR +3L )
#define CSSM_CSP_CONVERSION_ERROR            ( CSSM_CSP_BASE_ERROR +4L )
#define CSSM_CSP_NO_TOKENINFO                ( CSSM_CSP_BASE_ERROR +5L )
#define CSSM_CSP_INTERNAL_ERROR              ( CSSM_CSP_BASE_ERROR +6L )
#define CSSM_CSP_SERIAL_REQUIRED             ( CSSM_CSP_BASE_ERROR +7L )
#define CSSM_CSP_NOT_IMPLEMENTED             ( CSSM_CSP_BASE_ERROR +8L )
#define CSSM_CSP_GET_APIMEMFUNC_ERROR        ( CSSM_CSP_BASE_ERROR +9L )

#define CSSM_CSP_MEMORY_ERROR                ( CSSM_CSP_BASE_ERROR +20L )
#define CSSM_CSP_NOT_ENOUGH_BUFFER           ( CSSM_CSP_BASE_ERROR +21L )
#define CSSM_CSP_ERR_OUTBUF_LENGTH           ( CSSM_CSP_BASE_ERROR +22L )
#define CSSM_CSP_NO_OUTBUF                   ( CSSM_CSP_BASE_ERROR +23L )
#define CSSM_CSP_ERR_INBUF_LENGTH            ( CSSM_CSP_BASE_ERROR +24L )
#define CSSM_CSP_ERR_KEYBUF_LENGTH           ( CSSM_CSP_BASE_ERROR +25L )
#define CSSM_CSP_NO_SLOT                     ( CSSM_CSP_BASE_ERROR +26L )

#define CSSM_CSP_INVALID_CSP_HANDLE          ( CSSM_CSP_BASE_ERROR +30L )
#define CSSM_CSP_INVALID_POINTER             ( CSSM_CSP_BASE_ERROR +31L )
#define CSSM_CSP_INVALID_CONTEXT             ( CSSM_CSP_BASE_ERROR +32L )
#define CSSM_CSP_INVALID_CONTEXT_HANDLE      ( CSSM_CSP_BASE_ERROR +33L )
#define CSSM_CSP_INVALID_CONTEXT_POINTER     ( CSSM_CSP_BASE_ERROR +34L )
#define CSSM_CSP_INVALID_DATA                 ( CSSM_CSP_BASE_ERROR +35L )
#define CSSM_CSP_INVALID_DATA_POINTER        ( CSSM_CSP_BASE_ERROR +36L )
#define CSSM_CSP_INVALID_DATA_COUNT          ( CSSM_CSP_BASE_ERROR +37L )
#define CSSM_CSP_INVALID_KEY                 ( CSSM_CSP_BASE_ERROR +38L )
#define CSSM_CSP_INVALID_KEY_POINTER         ( CSSM_CSP_BASE_ERROR +39L )
#define CSSM_CSP_INVALID_KEY_LENGTH          ( CSSM_CSP_BASE_ERROR +40L )
#define CSSM_CSP_INVALID_CRYPTO_DATA         ( CSSM_CSP_BASE_ERROR +41L )
#define CSSM_CSP_INVALID_CRYPTO_DATA_POINTER ( CSSM_CSP_BASE_ERROR +42L )
#define CSSM_CSP_INVALID_SIGNATURE           ( CSSM_CSP_BASE_ERROR +43L )
#define CSSM_CSP_INVALID_DEVICE_ID           ( CSSM_CSP_BASE_ERROR +44L )
#define CSSM_CSP_INVALID_DIGEST                 ( CSSM_CSP_BASE_ERROR +45L )

#define CSSM_CSP_INVALID_ALGORITHM             ( CSSM_CSP_BASE_ERROR +60L )
#define CSSM_CSP_INVALID_MODE                 ( CSSM_CSP_BASE_ERROR +61L )
#define CSSM_CSP_INVALID_PADDING             ( CSSM_CSP_BASE_ERROR +62L )
#define CSSM_CSP_INVALID_IV_SIZE             ( CSSM_CSP_BASE_ERROR +63L )
#define CSSM_CSP_INVALID_INIT_VECTOR         ( CSSM_CSP_BASE_ERROR +64L )
#define CSSM_CSP_INVALID_KEY_SIZE_IN_BITS    ( CSSM_CSP_BASE_ERROR +65L )
#define CSSM_CSP_INVALID_ROUNDS                 ( CSSM_CSP_BASE_ERROR +66L )
#define CSSM_CSP_INVALID_EFFECTIVE_BITS         ( CSSM_CSP_BASE_ERROR +67L )
#define CSSM_CSP_INVALID_PARAM_LENGTH        ( CSSM_CSP_BASE_ERROR +68L )
#define CSSM_CSP_INVALID_KEYCLASS            ( CSSM_CSP_BASE_ERROR +69L )
#define CSSM_CSP_INVALID_KEYTYPE             ( CSSM_CSP_BASE_ERROR +70L )
#define CSSM_CSP_INVALID_KEYUSAGE            ( CSSM_CSP_BASE_ERROR +71L )
#define CSSM_CSP_INVALID_KEYATTR             ( CSSM_CSP_BASE_ERROR +72L )
#define CSSM_CSP_INVALID_ITERATION_COUNT     ( CSSM_CSP_BASE_ERROR +73L )
#define CSSM_CSP_INVALID_PASSTHROUGH_ID      ( CSSM_CSP_BASE_ERROR +74L )
#define CSSM_CSP_INVALID_PASSTHROUGH_PARAMS  ( CSSM_CSP_BASE_ERROR +75L )
#define CSSM_CSP_INVALID_SALT                 ( CSSM_CSP_BASE_ERROR +76L )
#define CSSM_CSP_INVALID_RANDOM                 ( CSSM_CSP_BASE_ERROR +77L )
#define CSSM_CSP_INVALID_SEED                 ( CSSM_CSP_BASE_ERROR +78L )
#define CSSM_CSP_INVALID_ALG_PARAMS             ( CSSM_CSP_BASE_ERROR +79L )
#define CSSM_CSP_INVALID_LABEL                 ( CSSM_CSP_BASE_ERROR +80L )
#define CSSM_CSP_INVALID_DATE                 ( CSSM_CSP_BASE_ERROR +81L )
#define CSSM_CSP_INVALID_SUBJECT_KEY         ( CSSM_CSP_BASE_ERROR +82L )
#define CSSM_CSP_INVALID_KEYUSAGE_MASK         ( CSSM_CSP_BASE_ERROR +83L )
#define CSSM_CSP_INVALID_KEYATTR_MASK         ( CSSM_CSP_BASE_ERROR +84L )

/* password message and errors */
#define CSSM_CSP_PASSPHRASE_INCORRECT        ( CSSM_CSP_BASE_ERROR +100L)
#define CSSM_CSP_PASSPHRASE_SAME             ( CSSM_CSP_BASE_ERROR +101L)
#define CSSM_CSP_PASSPHRASE_LENGTH_ERROR     ( CSSM_CSP_BASE_ERROR +102L)
#define CSSM_CSP_PASSPHRASE_INVALID          ( CSSM_CSP_BASE_ERROR +103L)
#define CSSM_CSP_PASSPHRASE_VERIFY_FAILED    ( CSSM_CSP_BASE_ERROR +104L)

#define CSSM_CSP_INVALID_CONTEXT_ATTRIBUTE   ( CSSM_CSP_BASE_ERROR +120L )
#define CSSM_CSP_INVALID_ATTR_KEY             ( CSSM_CSP_BASE_ERROR +121L )
#define CSSM_CSP_INVALID_ATTR_INIT_VECTOR     ( CSSM_CSP_BASE_ERROR +122L )
#define CSSM_CSP_INVALID_ATTR_SALT             ( CSSM_CSP_BASE_ERROR +123L )
#define CSSM_CSP_INVALID_ATTR_PADDING        ( CSSM_CSP_BASE_ERROR +124L )
#define CSSM_CSP_INVALID_ATTR_RANDOM         ( CSSM_CSP_BASE_ERROR +125L )
#define CSSM_CSP_INVALID_ATTR_SEED             ( CSSM_CSP_BASE_ERROR +126L )
#define CSSM_CSP_INVALID_ATTR_PASSPHRASE     ( CSSM_CSP_BASE_ERROR +127L )
#define CSSM_CSP_INVALID_ATTR_KEY_LENGTH     ( CSSM_CSP_BASE_ERROR +128L )
#define CSSM_CSP_INVALID_ATTR_OUTPUT_SIZE     ( CSSM_CSP_BASE_ERROR +129L )
#define CSSM_CSP_INVALID_ATTR_ROUNDS         ( CSSM_CSP_BASE_ERROR +130L )
#define CSSM_CSP_INVALID_ATTR_ALG_PARAMS     ( CSSM_CSP_BASE_ERROR +131L )
#define CSSM_CSP_INVALID_ATTR_MODE             ( CSSM_CSP_BASE_ERROR +132L )
#define CSSM_CSP_INVALID_ATTR_START_DATE     ( CSSM_CSP_BASE_ERROR +133L )
#define CSSM_CSP_INVALID_ATTR_END_DATE         ( CSSM_CSP_BASE_ERROR +134L )
#define CSSM_CSP_INVALID_ATTR_INTERATION_COUNT ( CSSM_CSP_BASE_ERROR +135L )
#define CSSM_CSP_INVALID_ATTR_KEY_TYPE         ( CSSM_CSP_BASE_ERROR +136L )

#define CSSM_CSP_INPUT_LENGTH_OVERSIZE       ( CSSM_CSP_BASE_ERROR +150L )
#define CSSM_CSP_INPUT_LENGTH_ERROR          ( CSSM_CSP_BASE_ERROR +151L )
#define CSSM_CSP_INPUT_DATA_ERROR            ( CSSM_CSP_BASE_ERROR +152L )

#define CSSM_CSP_EXCLUSIVE_UNAVAILABLE       ( CSSM_CSP_BASE_ERROR +160L )
#define CSSM_CSP_UPDATE_WITHOUT_INIT         ( CSSM_CSP_BASE_ERROR +161L )
#define CSSM_CSP_CALLBACK_FAILED             ( CSSM_CSP_BASE_ERROR +166L )

#define CSSM_CSP_FILE_NOT_EXISTS             ( CSSM_CSP_BASE_ERROR +180L )
#define CSSM_CSP_FILE_NOT_OPEN               ( CSSM_CSP_BASE_ERROR +181L )
#define CSSM_CSP_FILE_OPEN_FAILED            ( CSSM_CSP_BASE_ERROR +182L )
#define CSSM_CSP_FILE_CREATE_FAILED          ( CSSM_CSP_BASE_ERROR +183L )
#define CSSM_CSP_FILE_READ_FAILED            ( CSSM_CSP_BASE_ERROR +184L )
#define CSSM_CSP_FILE_WRITE_FAILED           ( CSSM_CSP_BASE_ERROR +185L )
#define CSSM_CSP_FILE_CLOSE_FAILED           ( CSSM_CSP_BASE_ERROR +186L )
#define CSSM_CSP_FILE_COPY_FAILED            ( CSSM_CSP_BASE_ERROR +187L )
#define CSSM_CSP_FILE_DELETE_FAILED          ( CSSM_CSP_BASE_ERROR +188L )
#define CSSM_CSP_FILE_FORMAT_ERROR           ( CSSM_CSP_BASE_ERROR +189L)

#define CSSM_CSP_OPERATION_UNSUPPORTED       ( CSSM_CSP_BASE_ERROR +200L )
#define CSSM_CSP_OPERATION_FAILED            ( CSSM_CSP_BASE_ERROR +201L )
#define CSSM_CSP_OPERATION_IN_PROGRESS       ( CSSM_CSP_BASE_ERROR +201L )

#define CSSM_CSP_STAGED_OPERATION_UNSUPPORTED ( CSSM_CSP_BASE_ERROR+210L )
#define CSSM_CSP_STAGED_OPERATION_FAILED      ( CSSM_CSP_BASE_ERROR+211L )
#define CSSM_CSP_STAGED_OPERATION_IN_PROGRESS ( CSSM_CSP_BASE_ERROR+212L )

#define CSSM_CSP_VECTOROFBUFS_UNSUPPORTED    ( CSSM_CSP_BASE_ERROR +220L )
#define CSSM_CSP_KEYUSAGE_MASK_UNSUPPORTED   ( CSSM_CSP_BASE_ERROR +221L )
#define CSSM_CSP_KEYATTR_MASK_UNSUPPORTED    ( CSSM_CSP_BASE_ERROR +222L )

#define CSSM_CSP_QUERY_SIZE_UNKNOWN          ( CSSM_CSP_BASE_ERROR +250L )
#define CSSM_CSP_QUERY_KEYSIZEINBITS_UNKNOWN ( CSSM_CSP_BASE_ERROR +251L )
#define CSSM_CSP_GET_STAGED_INFO_ERROR       ( CSSM_CSP_BASE_ERROR +252L )

/* key, key management messages and errors */
#define CSSM_CSP_PRIKEY_LOAD_ERROR           ( CSSM_CSP_BASE_ERROR +330L )
#define CSSM_CSP_PRIKEY_NOT_FOUND            ( CSSM_CSP_BASE_ERROR +231L )
#define CSSM_CSP_PRIKEY_ALREADY_EXIST        ( CSSM_CSP_BASE_ERROR +232L )
#define CSSM_CSP_PRIKEY_ERROR                 ( CSSM_CSP_BASE_ERROR +233L )
#define CSSM_CSP_PRIKEY_PUBKEY_INCONSISTENT  ( CSSM_CSP_BASE_ERROR +234L )

#define CSSM_CSP_KEY_ALGID_MISMATCH          ( CSSM_CSP_BASE_ERROR +350L )
#define CSSM_CSP_KEY_BLOBTYPE_INCORRECT      ( CSSM_CSP_BASE_ERROR +351L )
#define CSSM_CSP_KEY_DELETE_FAILED           ( CSSM_CSP_BASE_ERROR +352L )
#define CSSM_CSP_KEY_USAGE_INCORRECT         ( CSSM_CSP_BASE_ERROR +353L )
#define CSSM_CSP_KEY_FORMAT_INCORRECT        ( CSSM_CSP_BASE_ERROR +354L )
#define CSSM_CSP_KEY_KEYHEADER_INCONSISTENT  ( CSSM_CSP_BASE_ERROR +355L )
#define CSSM_CSP_KEY_PROTECTED               ( CSSM_CSP_BASE_ERROR +356L )
#define CSSM_CSP_KEY_INCOMPATIBLE_VERSION    ( CSSM_CSP_BASE_ERROR +357L )

/* login messages and errors */
#define CSSM_CSP_LOGIN_FAILED                ( CSSM_CSP_BASE_ERROR +400L )
#define CSSM_CSP_NOT_LOGGED_IN               ( CSSM_CSP_BASE_ERROR +401L )
#define CSSM_CSP_ALREADY_LOGGED_IN           ( CSSM_CSP_BASE_ERROR +402L )

#define CSSM_CSP_DEVICE_ERROR                ( CSSM_CSP_BASE_ERROR +470L )
#define CSSM_CSP_DEVICE_MEMORY_ERROR         ( CSSM_CSP_BASE_ERROR +471L )
#define CSSM_CSP_DEVICE_REMOVED              ( CSSM_CSP_BASE_ERROR +472L )
#define CSSM_CSP_DEVICE_NOT_PRESENT          ( CSSM_CSP_BASE_ERROR +473L )
#define CSSM_CSP_DEVICE_UNKNOWN              ( CSSM_CSP_BASE_ERROR +474L )

#define CSSM_CSP_PERMISSIONS_READ_ONLY       ( CSSM_CSP_BASE_ERROR +490L )
#define CSSM_CSP_PERMISSIONS_WRITE_PROTECT   ( CSSM_CSP_BASE_ERROR +491L )
#define CSSM_CSP_PERMISSIONS_NOT_EXCLUSIVE   ( CSSM_CSP_BASE_ERROR +492L )


/* Porting Library Error Codes */

/* Memory allocation, pointers, strings */
#define CSSM_MALLOC_FAILED                   ( CSSM_BASE_ERROR + 1 )
#define CSSM_CALLOC_FAILED                   ( CSSM_BASE_ERROR + 2 )
#define CSSM_REALLOC_FAILED                  ( CSSM_BASE_ERROR + 3 )

/* File I/O */
#define CSSM_FWRITE_FAILED                   ( CSSM_BASE_ERROR + 10 )
#define CSSM_FREAD_FAILED                    ( CSSM_BASE_ERROR + 11 )
#define CSSM_CANT_FSEEK                      ( CSSM_BASE_ERROR + 12 )
#define CSSM_INVALID_FILE_PTR                ( CSSM_BASE_ERROR + 13 )
#define CSSM_END_OF_FILE                     ( CSSM_BASE_ERROR + 14 )
#define CSSM_FOPEN_FAILED                    ( CSSM_BASE_ERROR + 15 ) /* Added 10/16/97 MEP */
#define CSSM_FCLOSE_FAILED                   ( CSSM_BASE_ERROR + 16 ) /* Added 10/16/97 MEP */
#define CSSM_INVALID_FILENAME                ( CSSM_BASE_ERROR + 17 ) /* Added 10/16/97 MEP */
#define CSSM_INVALID_MODE                    ( CSSM_BASE_ERROR + 18 ) /* Added 10/16/97 MEP */

/* Misc Errors */
#define CSSM_CANT_GET_USER_NAME              ( CSSM_BASE_ERROR + 20 )
#define CSSM_GETCWD_FAILED                   ( CSSM_BASE_ERROR + 21 )
#define CSSM_ENV_VAR_NOT_FOUND               ( CSSM_BASE_ERROR + 22 )

/* Dynamic Library */
#define CSSM_FREE_LIBRARY_FAILED             ( CSSM_BASE_ERROR + 30 )
#define CSSM_LOAD_LIBRARY_FAILED             ( CSSM_BASE_ERROR + 31 )
#define CSSM_CANT_GET_PROC_ADDR              ( CSSM_BASE_ERROR + 32 )
#define CSSM_CANT_GET_MODULE_HANDLE          ( CSSM_BASE_ERROR + 33 )
#define CSSM_CANT_GET_MODULE_FILE_NAME       ( CSSM_BASE_ERROR + 34 )
#define CSSM_INVALID_LIB_HANDLE              ( CSSM_BASE_ERROR + 35 )
#define CSSM_BAD_MODULE_HANDLE               ( CSSM_BASE_ERROR + 36 )

/* Registry errors */
#define CSSM_CANT_CREATE_KEY                 ( CSSM_BASE_ERROR + 40 )
#define CSSM_CANT_SET_VALUE                  ( CSSM_BASE_ERROR + 41 )
#define CSSM_CANT_GET_VALUE                  ( CSSM_BASE_ERROR + 42 )
#define CSSM_CANT_DELETE_SECTION             ( CSSM_BASE_ERROR + 43 )
#define CSSM_CANT_DELETE_KEY                 ( CSSM_BASE_ERROR + 44 )
#define CSSM_CANT_ENUM_KEY                   ( CSSM_BASE_ERROR + 45 )
#define CSSM_CANT_OPEN_KEY                   ( CSSM_BASE_ERROR + 46 )
#define CSSM_CANT_QUERY_KEY                  ( CSSM_BASE_ERROR + 47 )

/* Mutex/Synchronization Errors */
#define CSSM_CANT_CREATE_OBJECT               ( CSSM_BASE_ERROR + 50 )
#define CSSM_LOCK_OBJECT_FAILED               ( CSSM_BASE_ERROR + 51 )
#define CSSM_TRYLOCK_OBJECT_FAILED            ( CSSM_BASE_ERROR + 52 )
#define CSSM_UNLOCK_OBJECT_FAILED             ( CSSM_BASE_ERROR + 53 )
#define CSSM_CANT_CLOSE_OBJECT                ( CSSM_BASE_ERROR + 54 )
#define CSSM_INVALID_OBJECT_PTR               ( CSSM_BASE_ERROR + 55 )

/* Shared Memory File Errors */
#define CSSM_CANT_CREATE_SHARED_MEMORY_FILE  ( CSSM_BASE_ERROR + 60 )
#define CSSM_CANT_OPEN_SHARED_MEMORY_FILE    ( CSSM_BASE_ERROR + 61 )
#define CSSM_CANT_MAP_SHARED_MEMORY_FILE     ( CSSM_BASE_ERROR + 62 )
#define CSSM_CANT_UNMAP_SHARED_MEMORY_FILE   ( CSSM_BASE_ERROR + 63 )
#define CSSM_CANT_FLUSH_SHARED_MEMORY_FILE   ( CSSM_BASE_ERROR + 64 )
#define CSSM_CANT_CLOSE_SHARED_MEMORY_FILE   ( CSSM_BASE_ERROR + 65 )
#define CSSM_INVALID_PERMS                   ( CSSM_BASE_ERROR + 66 )
#define CSSM_BAD_FILE_HANDLE                 ( CSSM_BASE_ERROR + 67 )
#define CSSM_BAD_FILE_ADDR                   ( CSSM_BASE_ERROR + 68 )


/* General */
#define CSSM_BAD_PTR_PASSED                  ( CSSM_BASE_ERROR + 100 )

/* CSSM API Errors */

#define CSSM_INVALID_POINTER                 ( CSSM_BASE_ERROR + 301L )
#define CSSM_EXPIRED                         ( CSSM_BASE_ERROR + 302L )
#define CSSM_MEMORY_ERROR                    ( CSSM_BASE_ERROR + 303L )
#define CSSM_INVALID_ATTRIBUTE               ( CSSM_BASE_ERROR + 304L )
#define CSSM_NOT_INITIALIZE                  ( CSSM_BASE_ERROR + 305L )
#define CSSM_INSTALL_FAIL                    ( CSSM_BASE_ERROR + 306L )
#define CSSM_REGISTRY_ERROR                  ( CSSM_BASE_ERROR + 307L )
#define CSSM_INVALID_CONTEXT_HANDLE          ( CSSM_BASE_ERROR + 308L )
#define CSSM_INVALID_CSP_HANDLE              ( CSSM_BASE_ERROR + 309L )
#define CSSM_INVALID_TP_HANDLE               ( CSSM_BASE_ERROR + 310L )
#define CSSM_INVALID_CL_HANDLE               ( CSSM_BASE_ERROR + 311L )
#define CSSM_INVALID_DL_HANDLE               ( CSSM_BASE_ERROR + 312L )
#define CSSM_INCOMPATIBLE_VERSION            ( CSSM_BASE_ERROR + 313L )
#define CSSM_ATTACH_FAIL                     ( CSSM_BASE_ERROR + 314L )
#define CSSM_NO_ADDIN                        ( CSSM_BASE_ERROR + 315L )
#define CSSM_FUNCTION_NOT_IMPLEMENTED        ( CSSM_BASE_ERROR + 316L )
#define CSSM_INVALID_CONTEXT_POINTER         ( CSSM_BASE_ERROR + 317L )
#define CSSM_INVALID_MANIFEST_ATTRIB_POINTER ( CSSM_BASE_ERROR + 318L )
#define CSSM_MODE_UNSUPPORTED            ( CSSM_BASE_ERROR + 319L )
#define CSSM_KEY_LENGTH_UNSUPPORTED          ( CSSM_BASE_ERROR + 320L )
#define CSSM_IV_SIZE_UNSUPPORTED             ( CSSM_BASE_ERROR + 321L )
#define CSSM_PADDING_UNSUPPORTED             ( CSSM_BASE_ERROR + 322L )
#define CSSM_KEY_MODULUS_UNSUPPORTED         ( CSSM_BASE_ERROR + 323L )
#define CSSM_PARAM_NO_KEY                    ( CSSM_BASE_ERROR + 324L )

#define CSSM_INVALID_SERVICE_MASK            ( CSSM_BASE_ERROR + 325L )    
#define CSSM_INVALID_SUBSERVICEID            ( CSSM_BASE_ERROR + 326L )
#define CSSM_INVALID_INFO_LEVEL              ( CSSM_BASE_ERROR + 327L )
#define CSSM_CONTEXT_FILTER_FAILED           ( CSSM_BASE_ERROR + 329L )
#define CSSM_MANIFEST_ATTRIBUTES_NOT_FOUND   ( CSSM_BASE_ERROR + 330L )
#define CSSM_REGISTER_SERVICES_FAIL          ( CSSM_BASE_ERROR + 331L )
#define CSSM_DEREGISTER_SERVICES_FAIL        ( CSSM_BASE_ERROR + 332L )
#define CSSM_INVALID_MODULE_INFO             ( CSSM_BASE_ERROR + 333L )
#define CSSM_INVALID_NET_ADDRESS			 ( CSSM_BASE_ERROR + 334L )

#define CSSM_MODULE_MANIFEST_VERIFY_FAIL				   ( CSSM_BASE_ERROR + 335L )
#define CSSM_MODULE_MANIFEST_ATTRIBUTE_RETRIEVE_FAIL       ( CSSM_BASE_ERROR + 336L )
#define CSSM_MODULE_REQUIRES_APP_MANIFEST                  ( CSSM_BASE_ERROR + 337L )
#define CSSM_NO_APP_AUTHEN_KEY_IN_MODULE_MANIFEST          ( CSSM_BASE_ERROR + 338L )
#define CSSM_APP_MANIFEST_VERIFY_FAIL                      ( CSSM_BASE_ERROR + 339L )
#define CSSM_APP_MANIFEST_ATTRIBUTE_RETRIEVE_FAIL          ( CSSM_BASE_ERROR + 340L )
#define CSSM_NO_MODULE_AUTHEN_KEY_IN_APP_MANIFEST          ( CSSM_BASE_ERROR + 341L )
#define CSSM_MODULE_VERIFICATION_USING_EMBEDDED_KEY_FAILED ( CSSM_BASE_ERROR + 342L )
#define CSSM_MODULE_VERIFICATION_USING_APP_KEY_FAILED      ( CSSM_BASE_ERROR + 343L ) 
#define CSSM_APP_VERIFICATION_USING_EMBEDDED_KEY_FAILED    ( CSSM_BASE_ERROR + 344L )
#define CSSM_APP_VERIFICATION_USING_MODULE_KEY_FAILED      ( CSSM_BASE_ERROR + 345L )

/* CSSM errors, new */
#define CSSM_INVALID_ADDIN_HANDLE            ( CSSM_BASE_ERROR + 500L )
/* internal error for when a plug-in handle of any type is passed
 * to the plug-in manager.
 */
#define CSSM_INVALID_GUID                    ( CSSM_BASE_ERROR + 501L )
/* ill-formed or unmatchable GUID */
#define CSSM_MEM_FUNCS_NOT_MATCHING         ( CSSM_BASE_ERROR + 502L)
#define CSSM_ALREADY_INITIALIZED            ( CSSM_BASE_ERROR + 503L)
#define CSSM_ADDIN_ALREADY_REGISTERED       ( CSSM_BASE_ERROR + 504L)


/* Certificate Library Errors */
/* This list contains CSSM_CL_BASE_ERROR + 1 - 84, as defined for CSSM 2.0 */

#define CSSM_CL_INVALID_CERT_POINTER         ( CSSM_CL_BASE_ERROR + 6 )
#define CSSM_CL_INVALID_CERTGROUP_POINTER    ( CSSM_CL_BASE_ERROR + 48 ) /* Added in 2.0 */
#define CSSM_CL_INVALID_BUNDLE_POINTER       ( CSSM_CL_BASE_ERROR + 49 ) /* Added in 2.0 */
#define CSSM_CL_INVALID_BUNDLE_INFO          ( CSSM_CL_BASE_ERROR + 50 ) /* Added in 2.0 */
#define CSSM_CL_INVALID_CRL_PTR              ( CSSM_CL_BASE_ERROR + 28 )
#define CSSM_CL_INVALID_SIGNER_CERTIFICATE   ( CSSM_CL_BASE_ERROR + 26 )
#define CSSM_CL_INVALID_FIELD_POINTER        ( CSSM_CL_BASE_ERROR + 7 )
#define CSSM_CL_INVALID_TEMPLATE             ( CSSM_CL_BASE_ERROR + 8 )
#define CSSM_CL_INVALID_DATA_POINTER         ( CSSM_CL_BASE_ERROR + 9 )
#define CSSM_CL_INVALID_DATA                 ( CSSM_CL_BASE_ERROR + 40 )
#define CSSM_CL_INVALID_IDENTIFIER_PTR       ( CSSM_CL_BASE_ERROR + 51 ) /* Added in 2.0 */
#define CSSM_CL_INVALID_IDENTIFIER           ( CSSM_CL_BASE_ERROR + 52 ) /* Added in 2.0 */
#define CSSM_CL_INVALID_RESULTS_HANDLE       ( CSSM_CL_BASE_ERROR + 44 ) /* Added in 1.2 */
#define CSSM_CL_INVALID_HANDLE               ( CSSM_CL_BASE_ERROR + 53 ) /* Added in 2.0 */
#define CSSM_CL_INVALID_INDEX                ( CSSM_CL_BASE_ERROR + 54 ) /* Added in 2.0 */
#define CSSM_CL_INVALID_SCOPE                ( CSSM_CL_BASE_ERROR + 10 )
#define CSSM_CL_INVALID_ADDR                 ( CSSM_CL_BASE_ERROR + 55 ) /* Added in 2.0 */
#define CSSM_CL_INVALID_CSP                  ( CSSM_CL_BASE_ERROR + 56 ) /* Added in 2.0 */

#define CSSM_CL_UNKNOWN_FORMAT               ( CSSM_CL_BASE_ERROR + 1 )
#define CSSM_CL_UNKNOWN_TAG                  ( CSSM_CL_BASE_ERROR + 2 )
#define CSSM_CL_UNKNOWN_KEY_FORMAT           ( CSSM_CL_BASE_ERROR + 57 ) /* Added in 2.0 */
#define CSSM_CL_NO_FIELD_VALUES              ( CSSM_CL_BASE_ERROR + 27 )
#define CSSM_CL_CERT_EXPIRED                 ( CSSM_CL_BASE_ERROR + 47 ) /* Added in 1.2 */
#define CSSM_CL_AUTHENTICATION_FAIL          ( CSSM_CL_BASE_ERROR + 58 ) /* Added in 2.0 */
#define CSSM_CL_INSUFFICIENT_CREDENTIALS     ( CSSM_CL_BASE_ERROR + 59 ) /* Added in 2.0 */
#define CSSM_CL_SIGN_SCOPE_NOT_SUPPORTED     ( CSSM_CL_BASE_ERROR + 45 ) /* Added in 1.2 */
#define CSSM_CL_VERIFY_SCOPE_NOT_SUPPORTED   ( CSSM_CL_BASE_ERROR + 46 ) /* Added in 1.2 */
#define CSSM_CL_UNSUPPORTED_ADDR_TYPE        ( CSSM_CL_BASE_ERROR + 60 ) /* Added in 2.0 */
#define CSSM_CL_UNSUPPORTED_SERVICE          ( CSSM_CL_BASE_ERROR + 61 ) /* Added in 2.0 */
#define CSSM_CL_EXTRA_SERVICE_FAIL           ( CSSM_CL_BASE_ERROR + 62 ) /* Added in 2.0 */
#define CSSM_CL_PRIVATE_KEY_STORE_FAIL       ( CSSM_CL_BASE_ERROR + 63 ) /* Added in 2.0 */
#define CSSM_CL_ADDITIONAL_TIME_REQD         ( CSSM_CL_BASE_ERROR + 64 ) /* Added in 2.0 */
#define CSSM_CL_INVALID_RA                   ( CSSM_CL_BASE_ERROR + 65 ) /* Added in 2.0 */
#define CSSM_CL_NO_DEFAULT_RA                ( CSSM_CL_BASE_ERROR + 66 ) /* Added in 2.0 */
#define CSSM_CL_RA_REJECTED_FORM             ( CSSM_CL_BASE_ERROR + 67 ) /* Added in 2.0 */
#define CSSM_CL_CA_REJECTED_FORM             ( CSSM_CL_BASE_ERROR + 68 ) /* Added in 2.0 */

#define CSSM_CL_INITIALIZE_FAIL              ( CSSM_CL_BASE_ERROR + 41 )
#define CSSM_CL_UNINITIALIZE_FAIL            ( CSSM_CL_BASE_ERROR + 42 ) /* Added in 1.2 */
#define CSSM_CL_CERT_REQUEST_FAIL            ( CSSM_CL_BASE_ERROR + 69 ) /* Added in 2.0 */
#define CSSM_CL_CERT_RETRIEVE_FAIL           ( CSSM_CL_BASE_ERROR + 70 ) /* Added in 2.0 */
#define CSSM_CL_CERT_CREATE_FAIL             ( CSSM_CL_BASE_ERROR + 12 )
#define CSSM_CL_CERT_SIGN_FAIL               ( CSSM_CL_BASE_ERROR + 22 )
#define CSSM_CL_FORM_RETRIEVE_FAIL           ( CSSM_CL_BASE_ERROR + 71 ) /* Added in 2.0 */
#define CSSM_CL_FORM_SUBMIT_FAIL             ( CSSM_CL_BASE_ERROR + 72 ) /* Added in 2.0 */
#define CSSM_CL_MULTISIGN_REQUEST_FAIL       ( CSSM_CL_BASE_ERROR + 73 ) /* Added in 2.0 */
#define CSSM_CL_MULTISIGN_RETRIEVE_FAIL      ( CSSM_CL_BASE_ERROR + 74 ) /* Added in 2.0 */
#define CSSM_CL_RECOVERY_REQUEST_FAIL        ( CSSM_CL_BASE_ERROR + 75 ) /* Added in 2.0 */
#define CSSM_CL_RECOVERY_RETRIEVE_FAIL       ( CSSM_CL_BASE_ERROR + 76 ) /* Added in 2.0 */
#define CSSM_CL_CERT_RECOVER_FAIL            ( CSSM_CL_BASE_ERROR + 77 ) /* Added in 2.0 */
#define CSSM_CL_KEY_RECOVER_FAIL             ( CSSM_CL_BASE_ERROR + 78 ) /* Added in 2.0 */
#define CSSM_CL_ABORT_RECOVERY_FAIL          ( CSSM_CL_BASE_ERROR + 79 ) /* Added in 2.0 */
#define CSSM_CL_CERT_VERIFY_FAIL             ( CSSM_CL_BASE_ERROR + 24 )
#define CSSM_CL_CERT_GET_FIELD_VALUE_FAIL    ( CSSM_CL_BASE_ERROR + 14 )
#define CSSM_CL_CERT_ABORT_QUERY_FAIL        ( CSSM_CL_BASE_ERROR + 29 )
#define CSSM_CL_CERT_GET_KEY_INFO_FAIL       ( CSSM_CL_BASE_ERROR + 15 )
#define CSSM_CL_CERT_GET_ALL_FIELDS_FAIL     ( CSSM_CL_BASE_ERROR + 80 ) /* Added in 2.0 */
#define CSSM_CL_CERTGROUP_TO_BUNDLE_FAIL     ( CSSM_CL_BASE_ERROR + 81 ) /* Added in 2.0 */
#define CSSM_CL_CERTGROUP_FROM_BUNDLE_FAIL   ( CSSM_CL_BASE_ERROR + 82 ) /* Added in 2.0 */
#define CSSM_CL_CERT_IMPORT_FAIL             ( CSSM_CL_BASE_ERROR + 16 )
#define CSSM_CL_CERT_EXPORT_FAIL             ( CSSM_CL_BASE_ERROR + 17 )
#define CSSM_CL_CERT_DESCRIBE_FORMAT_FAIL    ( CSSM_CL_BASE_ERROR + 19 )

#define CSSM_CL_CRL_CREATE_FAIL              ( CSSM_CL_BASE_ERROR + 30 )
#define CSSM_CL_CRL_SET_FIELDS_FAIL          ( CSSM_CL_BASE_ERROR + 31 )
#define CSSM_CL_CRL_REQUEST_FAIL             ( CSSM_CL_BASE_ERROR + 83 ) /* Added in 2.0 */
#define CSSM_CL_CRL_RETRIEVE_FAIL            ( CSSM_CL_BASE_ERROR + 84 ) /* Added in 2.0 */
#define CSSM_CL_CRL_ADD_CERT_FAIL            ( CSSM_CL_BASE_ERROR + 32 )
#define CSSM_CL_CRL_REMOVE_CERT_FAIL         ( CSSM_CL_BASE_ERROR + 33 )
#define CSSM_CL_CRL_SIGN_FAIL                ( CSSM_CL_BASE_ERROR + 34 )
#define CSSM_CL_CRL_VERIFY_FAIL              ( CSSM_CL_BASE_ERROR + 35 )
#define CSSM_CL_IS_CERT_IN_CRL_FAIL          ( CSSM_CL_BASE_ERROR + 43 ) /* Added in 1.2 */ 
#define CSSM_CL_CRL_GET_FIELD_VALUE_FAIL     ( CSSM_CL_BASE_ERROR + 36 )
#define CSSM_CL_CRL_ABORT_QUERY_FAIL         ( CSSM_CL_BASE_ERROR + 37 )
#define CSSM_CL_CRL_DESCRIBE_FORMAT_FAIL     ( CSSM_CL_BASE_ERROR + 38 )
#define CSSM_CL_PASS_THROUGH_FAIL            ( CSSM_CL_BASE_ERROR + 18 )

#define CSSM_CL_MEMORY_ERROR                 ( CSSM_CL_BASE_ERROR + 21 )
#define CSSM_CL_UNSUPPORTED_OPERATION        ( CSSM_CL_BASE_ERROR + 20 )

/* #define CSSM_CL_INVALID_CONTEXT              ( CSSM_CL_BASE_ERROR + 3 ) obsoloete in 2.0. context is checked by CSSM 2/22/98 MEP */
/* #define CSSM_CL_INVALID_CL_HANDLE            ( CSSM_CL_BASE_ERROR + 4 ) obsoloete in 2.0. handle  is checked by CSSM 2/22/98 MEP */
/* #define CSSM_CL_INVALID_CC_HANDLE            ( CSSM_CL_BASE_ERROR + 5 ) obsoloete in 2.0. context is checked by CSSM 2/22/98 MEP */
/* #define CSSM_CL_INVALID_POINTER              ( CSSM_CL_BASE_ERROR + 39 ) removed 9/19/97 MEP */
/* #define CSSM_CL_CERT_VIEW_FAIL               ( CSSM_CL_BASE_ERROR + 13 ) removed 9/19/97 MEP */
/* #define CSSM_CL_CERT_UNSIGN_FAIL             ( CSSM_CL_BASE_ERROR + 23 ) removed 9/19/97 MEP */
/* #define CSSM_CL_RESULTS_HANDLE               ( CSSM_CL_BASE_ERROR + 25 ) removed 9/19/97 MEP */

/* #define CSSM_CL_SIG_NOT_IN_CERT              ( CSSM_CL_BASE_ERROR + 100 ) removed 9/19/97 MEP */
/* #define CSSM_CL_INVALID_REVOKER_CERT_PTR     ( CSSM_CL_BASE_ERROR + 101 ) removed 9/19/97 MEP */
/* #define CSSM_CL_NO_REVOKED_CERTS_IN_CRL      ( CSSM_CL_BASE_ERROR + 102 ) removed 9/19/97 MEP */
/* #define CSSM_CL_CERT_NOT_FOUND_IN_CRL        ( CSSM_CL_BASE_ERROR + 103 ) removed 9/19/97 MEP */
/* #define CSSM_CL_CRL_SIGNSCOPE_NOT_SUPPORTED  ( CSSM_CL_BASE_ERROR + 104 ) removed 9/19/97 MEP */
/* #define CSSM_CL_CRL_VERIFYSCOPE_NOT_SUPPORTED( CSSM_CL_BASE_ERROR + 105 ) removed 9/19/97 MEP */
/* #define CSSM_CL_CRL_NOT_SIGNEDBY_SIGNER      ( CSSM_CL_BASE_ERROR + 106 ) removed 9/19/97 MEP */
/* #define CSSM_CL_CRL_NO_FIELD_OID             ( CSSM_CL_BASE_ERROR + 107 ) removed 9/19/97 MEP */
/* #define CSSM_CL_INVALID_REVOKED_CERT_PTR     ( CSSM_CL_BASE_ERROR + 108 ) removed 9/19/97 MEP */
/* #define CSSM_CL_INVALID_INPUT_PTR            ( CSSM_CL_BASE_ERROR + 109 ) removed 9/19/97 MEP */


/* Trust Policy Errors */
#define CSSM_TP_NOT_LOADED                   ( CSSM_TP_BASE_ERROR + 1 )
#define CSSM_TP_INVALID_TP_HANDLE            ( CSSM_TP_BASE_ERROR + 2 )
#define CSSM_TP_INVALID_CL_HANDLE            ( CSSM_TP_BASE_ERROR + 3 )
#define CSSM_TP_INVALID_DL_HANDLE            ( CSSM_TP_BASE_ERROR + 4 )
#define CSSM_TP_INVALID_DB_HANDLE            ( CSSM_TP_BASE_ERROR + 5 )
#define CSSM_TP_INVALID_CC_HANDLE            ( CSSM_TP_BASE_ERROR + 6 )
#define CSSM_TP_INVALID_CERTIFICATE          ( CSSM_TP_BASE_ERROR + 7 )
#define CSSM_TP_NOT_SIGNER                   ( CSSM_TP_BASE_ERROR + 8 )
#define CSSM_TP_NOT_TRUSTED                  ( CSSM_TP_BASE_ERROR + 9 )
#define CSSM_TP_CERT_VERIFY_FAIL             ( CSSM_TP_BASE_ERROR + 10 )
#define CSSM_TP_CERTIFICATE_CANT_OPERATE     ( CSSM_TP_BASE_ERROR + 11 )
#define CSSM_TP_MEMORY_ERROR                 ( CSSM_TP_BASE_ERROR + 12 )
#define CSSM_TP_CERT_SIGN_FAIL               ( CSSM_TP_BASE_ERROR + 13 )
#define CSSM_TP_INVALID_CRL                  ( CSSM_TP_BASE_ERROR + 14 )
#define CSSM_TP_CERT_REVOKE_FAIL             ( CSSM_TP_BASE_ERROR + 15 )
#define CSSM_TP_CRL_VERIFY_FAIL              ( CSSM_TP_BASE_ERROR + 16 )
#define CSSM_TP_CRL_SIGN_FAIL                ( CSSM_TP_BASE_ERROR + 17 )
#define CSSM_TP_APPLY_CRL_TO_DB_FAIL         ( CSSM_TP_BASE_ERROR + 18 )
#define CSSM_TP_INVALID_GUID                 ( CSSM_TP_BASE_ERROR + 19 )
#define CSSM_TP_INCOMPATIBLE_VERSION         ( CSSM_TP_BASE_ERROR + 21 )
#define CSSM_TP_INVALID_ACTION               ( CSSM_TP_BASE_ERROR + 22 )
#define CSSM_TP_VERIFY_ACTION_FAIL           ( CSSM_TP_BASE_ERROR + 23 )
#define CSSM_TP_INVALID_DATA_POINTER         ( CSSM_TP_BASE_ERROR + 24 )
#define CSSM_TP_INVALID_ID                   ( CSSM_TP_BASE_ERROR + 25 )
#define CSSM_TP_PASS_THROUGH_FAIL            ( CSSM_TP_BASE_ERROR + 26 )
/* new r1.2 errors */
#define CSSM_TP_INVALID_CERTGROUP_PTR        ( CSSM_TP_BASE_ERROR + 27 )
#define CSSM_TP_INVALID_VERIFY_CONTEXT_PTR   ( CSSM_TP_BASE_ERROR + 28 )
#define CSSM_TP_CERTGROUP_INCOMPLETE         ( CSSM_TP_BASE_ERROR + 29 )
#define CSSM_TP_INVALID_CSP_HANDLE           ( CSSM_TP_BASE_ERROR + 30 )
#define CSSM_TP_CERTGROUP_CONSTRUCT_FAIL     ( CSSM_TP_BASE_ERROR + 31 )
#define CSSM_TP_CERTGROUP_ANCHOR_NOT_FOUND   ( CSSM_TP_BASE_ERROR + 32 )
#define CSSM_TP_INITIALIZE_FAIL              ( CSSM_TP_BASE_ERROR + 33 )
#define CSSM_TP_INVALID_POLICY_IDENTIFIERS   ( CSSM_TP_BASE_ERROR + 34 )
#define CSSM_TP_INVALID_STOP_ON_POLICY       ( CSSM_TP_BASE_ERROR + 35 )
#define CSSM_TP_ANCHOR_CERTS_UNSUPPORTED     ( CSSM_TP_BASE_ERROR + 36 )
#define CSSM_TP_VERIFY_SCOPE_UNSUPPORTED     ( CSSM_TP_BASE_ERROR + 37 )
#define CSSM_TP_SIGN_SCOPE_UNSUPPORTED       ( CSSM_TP_BASE_ERROR + 38 )
#define CSSM_TP_INVALID_DBLIST               ( CSSM_TP_BASE_ERROR + 39 )

/* Obsoleted TP errors */
/* #define CSSM_TP_UNISTALL_FAIL                ( CSSM_TP_BASE_ERROR + 20 ) removed 971014 SNR */


/* Database Errors */
/* This list contains CSSM_DL_BASE_ERROR + 1 - 90, as defined for CSSM 2.0 */
#define CSSM_DL_INVALID_DB_HANDLE            ( CSSM_DL_BASE_ERROR + 6 ) 
#define CSSM_DL_INVALID_POINTER              ( CSSM_DL_BASE_ERROR + 36 )
#define CSSM_DL_INVALID_CERTIFICATE_PTR      ( CSSM_DL_BASE_ERROR + 13 )
#define CSSM_DL_INVALID_CRL_PTR              ( CSSM_DL_BASE_ERROR + 24 )
#define CSSM_DL_INVALID_FIELD_INFO           ( CSSM_DL_BASE_ERROR + 39 )
#define CSSM_DL_INVALID_PASSTHROUGH_ID       ( CSSM_DL_BASE_ERROR + 34 )

#define CSSM_DL_DB_OPEN_FAIL                 ( CSSM_DL_BASE_ERROR + 5 ) 
#define CSSM_DL_DB_CLOSE_FAIL                ( CSSM_DL_BASE_ERROR + 7 ) 
#define CSSM_DL_DB_CREATE_FAIL               ( CSSM_DL_BASE_ERROR + 8 ) 
#define CSSM_DL_DB_DELETE_FAIL               ( CSSM_DL_BASE_ERROR + 9 ) 
#define CSSM_DL_DB_IMPORT_FAIL               ( CSSM_DL_BASE_ERROR + 11 )
#define CSSM_DL_DB_EXPORT_FAIL               ( CSSM_DL_BASE_ERROR + 12 )

#define CSSM_DL_PASS_THROUGH_FAIL            ( CSSM_DL_BASE_ERROR + 35 )
#define CSSM_DL_MEMORY_ERROR                 ( CSSM_DL_BASE_ERROR + 4 ) 
#define CSSM_DL_INCOMPATIBLE_VERSION         ( CSSM_DL_BASE_ERROR + 38 )
#define CSSM_DL_DATASTORE_NOT_EXISTS         ( CSSM_DL_BASE_ERROR + 3 ) 
#define CSSM_DL_DATASTORE_DOES_NOT_EXIST     CSSM_DL_DATASTORE_NOT_EXISTS

#define CSSM_DL_INVALID_ACCESS_REQUEST          ( CSSM_DL_BASE_ERROR + 40 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_CREDENTIAL              ( CSSM_DL_BASE_ERROR + 41 ) /* Added in 1.2 */
#define CSSM_DL_INSUFFICIENT_CREDENTIALS        CSSM_DL_INVALID_CREDENTIAL
#define CSSM_DL_INVALID_AUTHENTICATION_DATA     ( CSSM_DL_BASE_ERROR + 42 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_DB_NAME                 ( CSSM_DL_BASE_ERROR + 43 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_DB_OPEN_PARAMETERS      ( CSSM_DL_BASE_ERROR + 44 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_DB_INFO                 ( CSSM_DL_BASE_ERROR + 45 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_RECORD_PARSING_MODULE   ( CSSM_DL_BASE_ERROR + 46 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_ATTRIBUTE               ( CSSM_DL_BASE_ERROR + 47 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_ATTRIBUTES_POINTER      ( CSSM_DL_BASE_ERROR + 49 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_INDEX_INFO              ( CSSM_DL_BASE_ERROR + 50 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_INDEXED_DATA_LOCATION   ( CSSM_DL_BASE_ERROR + 84 ) /* Added in 2.0 */
#define CSSM_DL_INVALID_RECORD_PARSING_FUNCTIONS ( CSSM_DL_BASE_ERROR + 51 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_DATA_POINTER            ( CSSM_DL_BASE_ERROR + 52 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_QUERY_POINTER           ( CSSM_DL_BASE_ERROR + 53 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_SELECTION_PRED          ( CSSM_DL_BASE_ERROR + 54 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_SELECTION_PRED_POINTER  ( CSSM_DL_BASE_ERROR + 55 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_RESULTS_HANDLE          ( CSSM_DL_BASE_ERROR + 56 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_RESULTS_HANDLE_POINTER  ( CSSM_DL_BASE_ERROR + 57 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_ENDOFDATASTORE_POINTER  ( CSSM_DL_BASE_ERROR + 58 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_OUTPUT_DATA_POINTER     ( CSSM_DL_BASE_ERROR + 59 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_ADDR                    ( CSSM_DL_BASE_ERROR + 86 ) /* Added in 2.0 */
#define CSSM_DL_INVALID_STRING                  ( CSSM_DL_BASE_ERROR + 97 ) /* Added in 2.0 */

#define CSSM_DL_INVALID_RECORD_UID              ( CSSM_DL_BASE_ERROR + 80 ) /* Added in 1.2 */
#define CSSM_DL_INVALID_RECORD_UID_POINTER      ( CSSM_DL_BASE_ERROR + 81 ) /* Added in 1.2 */
#define CSSM_DL_UNKNOWN_RECORD_ID               ( CSSM_DL_BASE_ERROR + 77 ) /* Added in 1.2 */
#define CSSM_DL_UNKNOWN_RECORD_UID              CSSM_DL_UNKNOWN_RECORD_ID

#define CSSM_DL_DB_UNSUPPORTED_RECORD_TYPE      ( CSSM_DL_BASE_ERROR + 60 ) /* Added in 1.2 */
#define CSSM_DL_DB_UNSUPPORTED_ATTRIBUTE        ( CSSM_DL_BASE_ERROR + 61 ) /* Added in 1.2 */
#define CSSM_DL_DB_UNSUPPORTED_CONJUNCTIVE      ( CSSM_DL_BASE_ERROR + 62 ) /* Added in 1.2 */
#define CSSM_DL_DB_UNSUPPORTED_OPERATOR         ( CSSM_DL_BASE_ERROR + 63 ) /* Added in 1.2 */
#define CSSM_DL_UNSUPPORTED_ADDR_TYPE           ( CSSM_DL_BASE_ERROR + 88 ) /* Added in 2.0 */
#define CSSM_DL_DB_RECORD_SIGNING_UNSUPPORTED   ( CSSM_DL_BASE_ERROR + 78 ) /* Added in 1.2 */
#define CSSM_DL_DB_REMOTE_LOCALITY_UNSUPPORTED  ( CSSM_DL_BASE_ERROR + 79 ) /* Added in 1.2 */

#define CSSM_DL_AUTHENTICATE_FAIL               ( CSSM_DL_BASE_ERROR + 64 ) /* Added in 1.2 */
#define CSSM_DL_DB_SET_PARSING_FUNCTIONS_FAIL   ( CSSM_DL_BASE_ERROR + 65 ) /* Added in 1.2 */
#define CSSM_DL_DB_GET_PARSING_FUNCTIONS_FAIL   ( CSSM_DL_BASE_ERROR + 66 ) /* Added in 1.2 */
#define CSSM_DL_GET_DB_NAME_FAIL                ( CSSM_DL_BASE_ERROR + 67 ) /* Added in 1.2 */
#define CSSM_DL_INITIALIZE_FAIL                 ( CSSM_DL_BASE_ERROR + 68 ) /* Added in 1.2 */
#define CSSM_DL_UNINITIALIZE_FAIL               ( CSSM_DL_BASE_ERROR + 69 ) /* Added in 1.2 */

#define CSSM_DL_DATA_INSERT_FAIL                ( CSSM_DL_BASE_ERROR + 70 ) /* Added in 1.2 */ 
#define CSSM_DL_DATA_DELETE_FAIL                ( CSSM_DL_BASE_ERROR + 71 ) /* Added in 1.2 */
#define CSSM_DL_DATA_MODIFY_FAIL                ( CSSM_DL_BASE_ERROR + 89 ) /* Added in 2.0 */
#define CSSM_DL_DATA_GET_FIRST_FAIL             ( CSSM_DL_BASE_ERROR + 72 ) /* Added in 1.2 */
#define CSSM_DL_DATA_GET_NEXT_FAIL              ( CSSM_DL_BASE_ERROR + 73 ) /* Added in 1.2 */
#define CSSM_DL_DATA_ABORT_QUERY_FAIL           ( CSSM_DL_BASE_ERROR + 74 ) /* Added in 1.2 */
#define CSSM_DL_DATA_GET_FROM_RECORD_UID_FAIL   ( CSSM_DL_BASE_ERROR + 90 ) /* Added in 2.0 */
#define CSSM_DL_FREE_RECORD_ID_FAIL             ( CSSM_DL_BASE_ERROR + 75 ) /* Added in 1.2 */
#define CSSM_DL_FREE_RECORD_UID_FAIL            CSSM_DL_FREE_RECORD_ID_FAIL

#define CSSM_DL_NO_DATASOURCES_AVAILABLE        ( CSSM_DL_BASE_ERROR + 76 ) /* Added in 1.2 */
#define CSSM_DL_NO_DATASTORES_AVAILABLE         CSSM_DL_NO_DATASOURCES_AVAILABLE
#define CSSM_DL_DATASTORE_ALREADY_EXISTS        ( CSSM_DL_BASE_ERROR + 82 ) /* Added in 1.2 */
#define CSSM_DL_DATASTORE_IS_OPEN               ( CSSM_DL_BASE_ERROR + 83 ) /* Added in 2.0 */

#define CSSM_DL_NEEDED_ATTRIBUTE_MISSING        ( CSSM_DL_BASE_ERROR + 94 ) /* Added in 2.0 */
#define CSSM_DL_ATTRIBUTE_SPECIFIED_MULTIPLE    ( CSSM_DL_BASE_ERROR + 96 ) /* Added in 2.0 */
#define CSSM_DL_ATTRIBUTE_NOT_FOUND             ( CSSM_DL_BASE_ERROR + 95 ) /* Added in 2.0 */
#define CSSM_DL_INDEX_NOT_FOUND                 ( CSSM_DL_BASE_ERROR + 91 ) /* Added in 2.0 */

#define CSSM_DL_NO_MORE_RECORDS                 ( CSSM_DL_BASE_ERROR + 92 ) /* Added in 2.0 */
#define CSSM_DL_NO_DATA_FOUND                   ( CSSM_DL_BASE_ERROR + 93 ) /* Added in 2.0 */
#define CSSM_DL_EXISTING_RECORD_MODIFIED        ( CSSM_DL_BASE_ERROR + 98 ) /* Added in 2.0 */

/* Note: CSSM_DL_BASE_ERROR + 87, 85, 99 are avalable */

/* Obsolete error codes */
/* #define CSSM_DL_NOT_LOADED                   ( CSSM_DL_BASE_ERROR + 1 ) removed 9/19/97 MEP */
/* #define CSSM_DL_INVALID_DL_HANDLE            ( CSSM_DL_BASE_ERROR + 2 ) obsolete in 2.0. handle  is checked by CSSM 2/22/98 MEP */
/* #define CSSM_DL_INVALID_PTR                  ( CSSM_DL_BASE_ERROR + 10 ) redundant with CSSM_DL_INVALID_POINTER 9/19/97 MEP */
/* #define CSSM_DL_CERTIFICATE_NOT_IN_DB        ( CSSM_DL_BASE_ERROR + 15 ) removed 9/19/97 MEP */
/* #define CSSM_DL_CERT_REVOKE_FAIL             ( CSSM_DL_BASE_ERROR + 17 ) removed 9/19/97 MEP */
/* #define CSSM_DL_INVALID_SELECTION_PTR        ( CSSM_DL_BASE_ERROR + 18 ) removed 9/19/97 MEP */
/* #define CSSM_DL_NO_MORE_CERTS                ( CSSM_DL_BASE_ERROR + 21 ) removed 9/19/97 MEP */
/* #define CSSM_DL_CRL_NOT_IN_DB                ( CSSM_DL_BASE_ERROR + 26 ) removed 9/19/97 MEP */
/* #define CSSM_DL_GET_DB_NAMES_FAIL            ( CSSM_DL_BASE_ERROR + 33 ) removed 9/19/97 MEP */
/* #define CSSM_DL_NO_DATASOURCES               ( CSSM_DL_BASE_ERROR + 37 ) removed 9/19/97 MEP */
/* #define CSSM_DL_INVALID_DATA_POINTER         ( CSSM_DL_BASE_ERROR + 100 ) removed 9/19/97 MEP */
/* #define CSSM_DL_INVALID_DLINFO_POINTER       ( CSSM_DL_BASE_ERROR + 101 ) removed 9/19/97 MEP */
/* #define CSSM_DL_INSTALL_FAIL                 ( CSSM_DL_BASE_ERROR + 102 ) removed 9/19/97 MEP */
/* #define CSSM_DL_INVALID_GUID                 ( CSSM_DL_BASE_ERROR + 103 ) removed 9/19/97 MEP */
/* #define CSSM_DL_UNINSTALL_FAIL               ( CSSM_DL_BASE_ERROR + 104 ) removed 9/19/97 MEP */
/* #define CSSM_DL_LIST_MODULES_FAIL            ( CSSM_DL_BASE_ERROR + 105 ) removed 9/19/97 MEP */
/* #define CSSM_DL_ATTACH_FAIL                  ( CSSM_DL_BASE_ERROR + 107 ) removed 9/19/97 MEP */
/* #define CSSM_DL_DETACH_FAIL                  ( CSSM_DL_BASE_ERROR + 108 ) removed 9/19/97 MEP */
/* #define CSSM_DL_GET_INFO_FAIL                ( CSSM_DL_BASE_ERROR + 109 ) removed 9/19/97 MEP */
/* #define CSSM_DL_FREE_INFO_FAIL               ( CSSM_DL_BASE_ERROR + 110 ) removed 9/19/97 MEP */
/* #define CSSM_DL_INVALID_DLINFO_PTR           ( CSSM_DL_BASE_ERROR + 111 ) removed 9/19/97 MEP */
/* #define CSSM_DL_INVALID_CL_HANDLE            ( CSSM_DL_BASE_ERROR + 112 ) removed 9/19/97 MEP */
/* #define CSSM_DL_INVALID_CERTIFICATE_PTR      ( CSSM_DL_BASE_ERROR + 113 ) redundant 9/19/97 MEP */
/* #define CSSM_DL_INVALID_CRL                  ( CSSM_DL_BASE_ERROR + 114 ) removed 9/19/97 MEP */
/* #define CSSM_DL_INVALID_CRL_POINTER          ( CSSM_DL_BASE_ERROR + 115 ) removed 9/19/97 MEP */

/* Further Obsolete Error codes - 2/13/98 */
/* #define CSSM_DL_CERT_INSERT_FAIL             ( CSSM_DL_BASE_ERROR + 14 ) removed 2/13/98 GBM */
/* #define CSSM_DL_CERT_DELETE_FAIL             ( CSSM_DL_BASE_ERROR + 16 ) removed 2/13/98 GBM */
/* #define CSSM_DL_CERT_GETFIRST_FAIL           ( CSSM_DL_BASE_ERROR + 20 ) removed 2/13/98 GBM */
/* #define CSSM_DL_CERT_GET_NEXT_FAIL           ( CSSM_DL_BASE_ERROR + 22 ) removed 2/13/98 GBM */
/* #define CSSM_DL_CERT_ABORT_QUERY_FAIL        ( CSSM_DL_BASE_ERROR + 23 ) removed 2/13/98 GBM */
/* #define CSSM_DL_NO_CERTIFICATE_FOUND         ( CSSM_DL_BASE_ERROR + 19 ) removed 2/13/98 GBM */

/* #define CSSM_DL_CRL_INSERT_FAIL              ( CSSM_DL_BASE_ERROR + 25 ) removed 2/13/98 GBM */
/* #define CSSM_DL_CRL_DELETE_FAIL              ( CSSM_DL_BASE_ERROR + 27 ) removed 2/13/98 GBM */
/* #define CSSM_DL_CRL_GET_FIRST_FAIL           ( CSSM_DL_BASE_ERROR + 29 ) removed 2/13/98 GBM */
/* #define CSSM_DL_CRL_GET_NEXT_FAIL            ( CSSM_DL_BASE_ERROR + 31 ) removed 2/13/98 GBM */
/* #define CSSM_DL_CRL_ABORT_QUERY_FAIL         ( CSSM_DL_BASE_ERROR + 32 ) removed 2/13/98 GBM */
/* #define CSSM_DL_NO_CRL_FOUND                 ( CSSM_DL_BASE_ERROR + 28 ) removed 2/13/98 GBM */
/* #define CSSM_DL_NO_MORE_CRLS                 ( CSSM_DL_BASE_ERROR + 30 ) removed 2/13/98 GBM */

#define CSSM_VL_INVALID_BUNDLE					( CSSM_VL_BASE_ERROR + 1  )
#define CSSM_VL_NOT_IN_BUNDLE					( CSSM_VL_BASE_ERROR + 2  )
#define CSSM_VL_INVALID_VO_HANDLE				( CSSM_VL_BASE_ERROR + 3  )
#define CSSM_VL_INVALID_MAP_ENTRY				( CSSM_VL_BASE_ERROR + 4  )
#define CSSM_VL_INVALID_VOBUNDLE_IDENTIFIER		( CSSM_VL_BASE_ERROR + 5  )
#define CSSM_VL_INVALID_INFO_POINTER			( CSSM_VL_BASE_ERROR + 6  )
#define CSSM_VL_INVALID_VO_IDENTIFIER			( CSSM_VL_BASE_ERROR + 7  )
#define CSSM_VL_INVALID_VO_INFO					( CSSM_VL_BASE_ERROR + 8  )
#define CSSM_VL_INVALID_ATTRIBUTE				( CSSM_VL_BASE_ERROR + 9  )
#define CSSM_VL_UNKNOWN_NAME					( CSSM_VL_BASE_ERROR + 10 )
#define CSSM_VL_INVALID_INFO_BLOCK				( CSSM_VL_BASE_ERROR + 11 )
#define CSSM_VL_INVALID_DO_INFO					( CSSM_VL_BASE_ERROR + 12 )
#define CSSM_VL_INVALID_PREFERENCES				( CSSM_VL_BASE_ERROR + 13 )
#define CSSM_VL_INVALID_LOCATION				( CSSM_VL_BASE_ERROR + 14 )
#define CSSM_VL_INVALID_JOIN_NAME				( CSSM_VL_BASE_ERROR + 15 )
#define CSSM_VL_INVALID_CERTIFICATE				( CSSM_VL_BASE_ERROR + 16 )
#define CSSM_VL_SIGNER_NOT_FOUND				( CSSM_VL_BASE_ERROR + 17 )
#define CSSM_VL_CREDENTIALS_VERIFY_FAIL			( CSSM_VL_BASE_ERROR + 18 )
#define CSSM_VL_DATA_VERIFY_FAIL				( CSSM_VL_BASE_ERROR + 19 )
#define CSSM_VL_CONTAINMENT_NOT_SUPPORTED		( CSSM_VL_BASE_ERROR + 20 )


#endif /* _CSSMERR_H */
