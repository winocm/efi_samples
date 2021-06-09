/*-----------------------------------------------------------------------
 *      File:   ISLTYPE.H
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
 *  INTEL CONFIDENTIAL
 *  This file, software, or program is supplied under the terms
 *  of a licence agreement or nondisclosure agreement with
 *  Intel Corporation and may not be copied or disclosed except
 *  in accordance with the terms of that agreement. This file,
 *  software, or program contains copyrighted material and/or
 *  trade secret information of Intel Corporation, and must be
 *  treated as such. Intel reserves all rights in this material,
 *  except as the licence agreement or nondisclosure agreement
 *  specifically indicate.
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

#ifndef _ISLTYPE_H
#define _ISLTYPE_H
//#include <stdio.h>

#include "cssm.h"

#ifdef WIN32
#pragma warning(disable:4201 4514 4214 4115)
#include <windows.h>
#pragma warning(disable:4201 4514 4214 4115)
#endif

//#ifndef _SIZE_T_DEFINED
//typedef unsigned int size_t;
//#define _SIZE_T_DEFINED
//#endif

#ifndef _FILE_DEFINED
struct _iobuf {
        char *_ptr;
        int   _cnt;
        char *_base;
        int   _flag;
        int   _file;
        int   _charbuf;
        int   _bufsiz;
        char *_tmpfname;
        };
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif

/* Basic Types */
typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef          __int16 sint16;
typedef unsigned __int32 uint32;
typedef          __int32 sint32;

/*
**      public data structures
*/
typedef long    ISL_ERROR_CODE;
typedef unsigned int  ISL_SIZE;

typedef void (*ISL_FUNCTION_PTR)(void);

typedef struct isl_data {
        uint32 Length;
        uint8  *Data;
} ISL_DATA, *ISL_DATA_PTR;

typedef struct isl_const_data {
        uint32 Length;
        const uint8 *Data;
} ISL_CONST_DATA, *ISL_CONST_DATA_PTR;

typedef enum isl_status {
    ISL_OK = 0,
    ISL_FAIL = -1
} ISL_STATUS;

/* codes for class of service */
typedef enum {
        ISL_ServiceGetData                              = 0,
        ISL_ServiceDigest                               = 2,
        ISL_ServiceSignVerify                   = 3,
        ISL_ServiceParseCertificate             = 4,
        ISL_ServiceFindVerify                   = 5,
        ISL_ServiceParseSignature               = 6
} ISL_SERVICE_CLASS;
/*
**      class structure
**
**      This is probably what the method vector should be (perhaps next API?).
**      For now, we'll just add a pointer to the class structure in each method
**      vector amd live with it.
**      Notable additions are the ability to define class variables in the
**  the class context.  The super link may be useful for inheritance.
*/
typedef struct isl_class {
        void *ClassMethods;                                     /* pointer to class methods */
        void *ClassProtectedMethods;            /* private methods */
        void *ClassContext;                                     /* pointer to class variables */
        struct isl_class *SuperClass;           /* optional in case we want to use inheritance */
        char *ClassName;                                        /* optional human-readable class name */
        char **MethodNames;                                     /* optional method name vector */
} ISL_CLASS, *ISL_CLASS_PTR;
/*
**      memory allocation service class methods
*/
typedef CSSM_MEMORY_FUNCS ISL_MEMORY_METHODS;

typedef struct isl_file_methods{
    FILE * (*fopen)(const char *,
        const char *);
    unsigned int (*fread)(void *,
        unsigned int,
        unsigned int,
        FILE *);
    int (*fclose)(FILE *);
} ISL_FILE_METHODS;

/* opaque Types */
typedef struct isl_signer_context                               *ISL_VERIFIED_SIGNATURE_ROOT_PTR;
typedef struct isl_verified_certificate_chain   *ISL_VERIFIED_CERTIFICATE_CHAIN_PTR;
typedef struct isl_certificate                                  *ISL_VERIFIED_CERTIFICATE_PTR;
typedef struct isl_manifest_section                             *ISL_MANIFEST_SECTION_PTR;
typedef struct isl_verified_module                              *ISL_VERIFIED_MODULE_PTR;
typedef struct isl_loaded_module                                *ISL_LOADED_MODULE_PTR;
typedef struct isl_verified_module                              *ISL_MODULE_PTR;
typedef struct isl_iterator                                             *ISL_ITERATOR_PTR;
typedef struct isl_signer_context                               *ISL_SIGNER_CONTEXT_PTR;
typedef void *                                                                  ISL_KEY_BLOCK, ** ISL_KEY_BLOCK_PTR;
typedef struct isl_certificate                                  ISL_CERTIFICATE, *ISL_CERTIFICATE_PTR;
typedef struct isl_config                                               ISL_CONFIG;
typedef struct isl_archive_context                              ISL_ARCHIVE_CONTEXT;
typedef struct isl_signature_context                    ISL_SIGNATURE_CONTEXT;
typedef struct isl_get_data_service_context             ISL_GET_DATA_SERVICE_CONTEXT;
typedef struct isl_digest_service_context               ISL_DIGEST_SERVICE_CONTEXT;
typedef struct isl_sign_service_context                 ISL_SIGN_SERVICE_CONTEXT;
typedef struct isl_digest_methods                               ISL_DIGEST_METHODS;
typedef struct isl_sign_verify_methods                  ISL_SIGN_VERIFY_METHODS;
typedef struct isl_certificate_methods                  ISL_CERTIFICATE_METHODS;
typedef struct isl_service_methods                              ISL_SERVICE_METHODS;
typedef struct isl_sig_section                                  ISL_SIG_SECTION;
typedef struct isl_archive_methods                              ISL_ARCHIVE_METHODS;
typedef struct isl_sig_section_list                     ISL_SIG_SECTION_LIST;
typedef struct isl_archive_context                          *ISL_ARCHIVE_CONTEXT_PTR;
typedef struct isl_config                                               *ISL_CONFIG_PTR;
typedef struct isl_archive_methods                          *ISL_ARCHIVE_METHODS_PTR;
typedef struct isl_signature_context                *ISL_SIGNATURE_CONTEXT_PTR;
typedef struct isl_signer_context                           *ISL_SIGNATURE_ROOT_PTR;
typedef struct isl_signature_methods                *ISL_SIGNATURE_METHODS_PTR;
typedef struct isl_manifest                                         *ISL_MANIFEST_PTR;
typedef struct isl_manifest_section                         *ISL_MANIFEST_SECTION_PTR;
typedef struct isl_manifest_section_methods     *ISL_MANIFEST_SECTION_METHODS_PTR;
typedef struct isl_alg_info                                         *ISL_ALG_INFO_PTR;
typedef struct isl_hash_info                                *ISL_HASH_INFO_PTR;
typedef struct isl_sig_section_list                         *ISL_SIG_SECTION_LIST_PTR;
typedef struct isl_sig_section                              *ISL_SIG_SECTION_PTR;
typedef struct isl_sig_section_methods              *ISL_SIG_SECTION_METHODS_PTR;
typedef struct isl_siginfo                                          *ISL_SIGINFO_PTR;
typedef struct isl_signers                                          *ISL_SIGNERS_PTR;
typedef struct isl_memory_buffer                            *ISL_MEMORY_BUFFER_PTR;
typedef struct isl_digest_methods                           *ISL_DIGEST_METHODS_PTR;
typedef struct isl_get_data_methods                         *ISL_GET_DATA_METHODS_PTR;
typedef struct isl_sign_verify_methods              *ISL_SIGN_VERIFY_METHODS_PTR;
typedef struct isl_algorithm_list_item              *ISL_ALGORITHM_LIST_ITEM_PTR;
typedef struct isl_config_methods                               *ISL_CONFIG_METHODS_PTR;
typedef ISL_MEMORY_METHODS                                              *ISL_MEMORY_METHODS_PTR;
typedef struct isl_certificate_list                         *ISL_CERTIFICATE_LIST_PTR;
typedef struct isl_signature_list                               *ISL_SIGNATURE_LIST_PTR;
typedef struct isl_memory_context               *ISL_MEMORY_CONTEXT_PTR;

/* class context structures */
typedef struct isl_csp_handles {
    uint32 NumberOfHandles;                                             /* Number of CSP handle in list */
    CSSM_MODULE_HANDLE *Handles;                                /* List of CSP handles */
} ISL_CSP_HANDLES, *ISL_CSP_HANDLES_PTR;

typedef struct isl_digest_sign_method {
        ISL_DIGEST_METHODS *DigestMethods;
        ISL_SIGN_VERIFY_METHODS *SignMethods;
}ISL_DIGEST_SIGN_METHOD, *ISL_DIGEST_SIGN_METHOD_PTR;

typedef struct isl_sign_methods {
        uint32 NumberOfMethods;
        ISL_DIGEST_SIGN_METHOD *Methods;
}ISL_SIGN_METHODS, *ISL_SIGN_METHOD_PTR;

typedef struct isl_sign_verify_class_context{
        CSSM_API_MEMORY_FUNCS_PTR MemoryFuncs;          /* memory functions that will be used by CSSM */
        ISL_CSP_HANDLES CSPList;
        ISL_SIGN_METHODS SignatureMethodList;
} ISL_SIGN_VERIFY_CLASS_CONTEXT, *ISL_SIGN_VERIFY_CLASS_CONTEXT_PTR;

typedef struct isl_digest_class_context{
        ISL_CSP_HANDLES CSPList;
} ISL_DIGEST_CLASS_CONTEXT, *ISL_DIGEST_CLASS_CONTEXT_PTR;

typedef struct isl_lmap_entry {
        ISL_GET_DATA_METHODS_PTR GetDataMethod;
        ISL_CONST_DATA GetParameters;
        ISL_CONST_DATA JoinName;
} ISL_LMAP_ENTRY, *ISL_LMAP_ENTRY_PTR;

typedef struct isl_lmap {
        uint32 NumberOfEntries;
        ISL_LMAP_ENTRY_PTR MapEntries;
} ISL_LMAP, *ISL_LMAP_PTR;

typedef struct isl_signature_info {
    ISL_CONST_DATA Name;
    ISL_SIGNATURE_CONTEXT_PTR SignaturePtr;
    ISL_SIG_SECTION_LIST_PTR SignedListPtr;
} ISL_SIGNATURE_INFO, *ISL_SIGNATURE_INFO_PTR;

typedef struct isl_signature_group {
    uint32 NumberOfSignatures;
    ISL_SIGNATURE_INFO_PTR  SignatureInfoPtr;
} ISL_SIGNATURE_INFO_GROUP, *ISL_SIGNATURE_INFO_GROUP_PTR;

typedef struct isl_name_value_pair {
        ISL_CONST_DATA  Name;
        ISL_CONST_DATA  Value;
} ISL_NAME_VALUE, *ISL_NAME_VALUE_PTR;

typedef struct isl_attribute_group {
    uint32 NumberOfAttributes;
    ISL_NAME_VALUE_PTR AttributesPtr;
} ISL_ATTRIBUTE_GROUP, *ISL_ATTRIBUTE_GROUP_PTR;

typedef struct isl_signer_group {
    uint32 NumberOfSigners;
    ISL_SIGNER_CONTEXT_PTR *Signers;
} ISL_SIGNER_GROUP, *ISL_SIGNER_GROUP_PTR;

typedef struct isl_certificate_group {
    uint32 NumberOfCertificates;
    ISL_CERTIFICATE_PTR *Certificates;
} ISL_CERTIFICATE_GROUP, *ISL_CERTIFICATE_GROUP_PTR;

typedef struct isl_sig_section_group {
    uint32 NumberOfSignedObjects;
    ISL_SIG_SECTION_PTR *SignedObjects;
} ISL_SIG_SECTION_GROUP, *ISL_SIG_SECTION_GROUP_PTR;

typedef struct isl_memory_buffer {
        ISL_MEMORY_BUFFER_PTR           Next;
        void                                            *Buffer;
} ISL_MEMORY_BUFFER;

typedef struct isl_memory_context{
    ISL_MEMORY_BUFFER_PTR Buffers;
    ISL_MEMORY_METHODS_PTR MemoryMethods;
    void * AllocRef;
}ISL_MEMORY_CONTEXT, *ISL_MEMORY_CONTEXT_PTR;

/* Do not edit or remove the following string.
 * It should be expanded at check-in by version control. */

#ifdef SCCS_ID
static const char sccs_id_INC_ISLTYPE_H[] = { "Fix ME" };
#endif

#endif
