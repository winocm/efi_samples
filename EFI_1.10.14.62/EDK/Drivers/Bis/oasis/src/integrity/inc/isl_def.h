/*-----------------------------------------------------------------------
 *      File:   ISL_DEF.H
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

#ifndef _ISL_DEF_H
#define _ISL_DEF_H
#include "pkapi.h"

/* Typedef Declarations */

typedef struct isl_manifest					ISL_MANIFEST;
typedef struct isl_const_name_value_pair    ISL_CONST_NAME_VALUE_PAIR;
typedef struct isl_iterator                 ISL_ITERATOR;
//typedef struct isl_name_value               ISL_NAME_VALUE;
typedef struct isl_manifest_section			ISL_MANIFEST_SECTION;
typedef struct isl_signer_context			ISL_SIGNER_CONTEXT;
typedef struct isl_verified_module          ISL_VERIFIED_MODULE;
typedef void								*ISL_NODE_PTR;
typedef struct der_node_struct DER_NODE,	*DER_NODE_PTR;
typedef void								*ISL_METHODS_PTR;
typedef struct isl_error_node				ISL_ERROR_NODE, *ISL_ERROR_NODE_PTR;

/* Typedef Definitions */
typedef struct isl_list {
    void * Node;
    struct isl_list * Next;
} ISL_LIST, *ISL_LIST_PTR;

typedef struct isl_header_section {
	uint16				MajorVersion;
	uint16				MinorVersion;
	uint32				AttributeCount;			/* number of attributes */
	ISL_LIST_PTR	    Attribute;				/* head of list of attributes*/
} ISL_HEADER_SECTION, *ISL_HEADER_SECTION_PTR;

struct isl_manifest {
	ISL_CLASS_PTR				Class;
	ISL_HEADER_SECTION			Header;
	uint32						SectionCount;	/* number of signable objects */
	ISL_LIST_PTR	            Section;		/* head of list of signable objects */
	ISL_CONST_DATA				Image;			/* external representation e.g. mf file */
};

struct isl_archive_context {
	ISL_CLASS_PTR				Class;
//	ISL_ARCHIVE_METHODS_PTR		Methods;
	ISL_CONFIG_PTR				Config;
//	ISL_CONST_DATA				PathName;		/* Root Directory */
	ISL_MANIFEST				Manifest;
	ISL_LIST_PTR	            Certs;			/* head of list of Certs */
	ISL_LIST_PTR                Signatures;		/* head of list of Signature Contexts */
	ISL_MEMORY_CONTEXT_PTR		Memory;		    /* head of list of Allocate Memory Buffers */
	ISL_LMAP                    Map;
//    struct pk_new_archive	   *NewZipArchive;	/* new archive during externalize call, or NULL */
	ISL_ERROR_NODE_PTR			ErrStack;		/* head of stack of error nodes */
};

struct isl_error_node {
	ISL_ERROR_CODE		ErrCode;
	uint8 *				ErrStr;
	ISL_ERROR_NODE_PTR	Next;
};

struct isl_config {
	void *                          AllocRef;
	ISL_MEMORY_METHODS_PTR			MemoryMethods;
	ISL_ALGORITHM_LIST_ITEM_PTR		Algorithms;
};

typedef struct isl_section {
	ISL_CONST_DATA				SectionName;
	ISL_CONST_DATA				Name;			/* archive specific name */
	ISL_CONST_DATA				Image;			/* external rep of signable object */
	uint32						AttributeCount; /* number of attributes */
	ISL_LIST_PTR				Attribute;		/* head of list of attributes*/
	uint32						AlgCount;		/* number of digest algs used */
	ISL_ALG_INFO_PTR			Algorithm;		/* head of list of digest algorithms */
} ISL_SECTION, *ISL_SECTION_PTR;

struct isl_manifest_section {							/* signable object */
	ISL_ARCHIVE_CONTEXT_PTR		Parent;
	ISL_CONST_DATA				Protocol;		/* http, file, module, etc */
	ISL_CONST_DATA				Parameters;
	ISL_GET_DATA_METHODS_PTR	GetDataMethods;
	ISL_SECTION					SectionInfo;
	ISL_MANIFEST_SECTION_PTR	Next;
};

typedef struct isl_algorithm_list_item {
	ISL_ALGORITHM_LIST_ITEM_PTR	Next;
	ISL_SERVICE_CLASS			ServiceType;
	ISL_CONST_DATA				ArchiveName;
	ISL_CONST_DATA				HumanName;
	void *						Methods;
} ISL_ALGORITHM_LIST_ITEM;

typedef struct isl_alg_info {
	ISL_CLASS_PTR				Class;
	ISL_DIGEST_METHODS_PTR		Methods;
	ISL_DATA_PTR				Parameters;
	ISL_MANIFEST_SECTION_PTR	Parent;
	ISL_CONST_DATA				AlgName;		/* archive specific name */
	uint32						HashCount;
	ISL_HASH_INFO_PTR			Hash;			/* head of list */
	uint32						AlgorithmID;
	CSSM_CSP_HANDLE				CSPHandle;
	ISL_ALG_INFO_PTR			Next;		
} ISL_ALG_INFO;

typedef struct isl_hash_info {
	ISL_CONST_DATA				Name;			/* archive specific name */
	ISL_CONST_DATA				Value;			/* internal representation */
	ISL_HASH_INFO_PTR			Next;
} ISL_HASH_INFO;


typedef struct isl_certificate_list {
	ISL_CERTIFICATE				*Cert;			/* certificate pointer (certs are standalone) */
	ISL_CONST_DATA				Name;			/* archive specific name */
	ISL_CERTIFICATE_LIST_PTR	Next;
} ISL_CERTIFICATE_LIST;

typedef struct isl_verified_certificate_chain {
	ISL_CERTIFICATE_LIST_PTR	CertList;
	ISL_ARCHIVE_CONTEXT_PTR		Archive;
} ISL_VERIFIED_CERTIFICATE_CHAIN;

struct isl_signer_context {
	ISL_CLASS_PTR				Class;
	ISL_SIGNATURE_CONTEXT_PTR	Parent;				/* signature that this signs */
	ISL_CONST_DATA				SignerID;			/* issuer+s/n (see cert->SignerID()) */
	ISL_DIGEST_METHODS_PTR		DigestMethods;		/* from configuration */
	ISL_CONST_DATA				DigestParameters;	/* digest parameters */
	ISL_SIGN_VERIFY_METHODS_PTR	SignMethods;		/* from configuration */
	ISL_CONST_DATA				SignParameters;		/* signature algorithm parameters */
	ISL_CONST_DATA				Signature;			/* signature bytes */
	uint32						AttributeCount;		/* number of elements in attributes array */
	ISL_NAME_VALUE_PTR			Attributes;			/* head of authenticated attributes list */
	ISL_NAME_VALUE_PTR			Unauthenticated;	/* head of unauthenticated attributes list */
	ISL_CONST_DATA				Image;				/* image after signing */
	ISL_VERIFIED_CERTIFICATE_CHAIN_PTR	CertChain;	/* head of verified certificate chain */
	ISL_SIGNER_CONTEXT_PTR		Next;				/* next signer for multiple signers */
};

struct isl_signature_context {				    /* signature */
	ISL_CONFIG_PTR				Config;
	ISL_CONST_DATA				Name;			/* without suffix */
	ISL_CONST_DATA				Image;			/* pkcs #7 external image */
    ISL_CONST_DATA              SignedImage;
	ISL_MEMORY_CONTEXT_PTR		Memory;		    /* head of allocate memory buffers list*/
	ISL_LIST_PTR		        Signers;		/* head of signer list */
	ISL_LIST_PTR	            Certificates;	/* head of certificate pointers list*/
};

#if 0
typedef struct isl_signature_info {
    ISL_ARCHIVE_CONTEXT_PTR     Parent;
	ISL_SIGNATURE_CONTEXT_PTR	Signature;		/* signature pointer (signatures are standalone) */
	ISL_SIG_SECTION_LIST_PTR	SigSections;	/* signed object list */		
	ISL_CONST_DATA				Name;			/* archive specific name */
} ISL_SIGNATURE_INFO;
#endif

struct isl_sig_section_list {			/* signed object list */
	ISL_ARCHIVE_CONTEXT_PTR     Parent;			/* signature that signs the object list */
    ISL_MEMORY_CONTEXT_PTR      Memory;
	ISL_HEADER_SECTION			Header;
	ISL_DATA					Name;			/* without suffix */
	uint32						SectionCount;   /* number of signed objects */
	ISL_LIST_PTR			    Section;		/* head of list of signed objects*/
	ISL_CONST_DATA				Image;			/* sf external image */
};

struct isl_sig_section {				/* signed object */
	ISL_CLASS_PTR				Class;
	ISL_SIG_SECTION_METHODS_PTR	Methods;
	ISL_SIG_SECTION_LIST_PTR	Parent;
	ISL_MANIFEST_SECTION_PTR	ManSect;	/* signable object */
	ISL_SECTION					SectionInfo;
	ISL_SIG_SECTION_PTR			Next;
};

struct isl_name_value {
	ISL_CONST_DATA				Name;
	ISL_CONST_DATA				Value;
	ISL_NAME_VALUE_PTR			Next;
};


struct isl_verified_module {
	ISL_CLASS_PTR					Class;
	void *							hModule;
	ISL_MANIFEST_SECTION_PTR		ManSect;
	ISL_CONST_DATA					Path;
	struct isl_verify_continuation *Continuation;
};


struct isl_certificate {
	ISL_CLASS_PTR				Class;
	ISL_MEMORY_CONTEXT_PTR Memory;
	ISL_CONST_DATA	theCert;				/* the entire cert */
	ISL_CONST_DATA	certificateInfo;		/* just the part that's signed */
	ISL_CONST_DATA	serialNumber;
	ISL_CONST_DATA	issuer;
	ISL_CONST_DATA	subject;
	ISL_CONST_DATA	pkcs7id;				/* constructed seq issuer, serial number */
	ISL_CONST_DATA	algorithmID;			/* OID for key */
	ISL_CONST_DATA	algorithmParameters;
	CSSM_KEY	    key;					/* entire key */
	uint32			attributeCount;			/* number of elements in attributes array */
	ISL_CONST_NAME_VALUE_PAIR *attributes;	/* array of ISL_CONST_DATA (name, value) pairs */
	ISL_MEMORY_BUFFER	*Buffers;
	ISL_ERROR_NODE_PTR ErrStack;
};

typedef struct isl_parsed_pkcs {
    ISL_ARCHIVE_CONTEXT_PTR pArchive;
    BER_PARSED_ITEM         *Node;
    uint32                  Count;
} ISL_PARSED_PKCS;

typedef struct isl_sig_section_protected_methods {
	ISL_STATUS (*InitializeOld)(	            /* constructor for existing object list */
		ISL_SIG_SECTION_LIST *memory,			/* memory allocated for this instance */
		ISL_SIGNATURE_CONTEXT *Context,		    /* archive which contains signature */
		ISL_CONST_DATA Name,					/* disk file of external representation */
        ISL_CONST_DATA Image);
} ISL_SIG_SECTION_PROTECTED_METHODS;


/* Do not edit or remove the following string. 
 * It should be expanded at check-in by version control. */
//static const char sccs_id_INC_ISL_DEF_H[] = { "Fix ME" }; 

#endif
