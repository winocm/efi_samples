/* SCCSID: inc/cssmtype.h, dss_cdsa_fwk, fwk_rel1a, dss_cdsa_970916 1.28 9/16/97 16:30:38 */
/*-----------------------------------------------------------------------
 *      File:   CSSMTYPE.H
 *
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

#ifndef _CSSMTYPE_H
#define _CSSMTYPE_H

#include "cssmdefs.h"

/* Operating System Dependent Primitive Declarations */
#ifndef CSSMAPI
	#if defined (WIN32)
		#define CSSMAPI __stdcall
	
	#elif defined (WIN31)
		#ifndef PASCAL
			#define PASCAL __stdcall
		#endif
		#define CSSMAPI PASCAL
	
	#elif defined (UNIX) 
		#define CSSMAPI

	#elif defined (OASIS) 
		#define CSSMAPI
	#endif
#endif

#if defined (UNIX) 
#define CALLBACK
#endif

#ifdef EFI64
typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef          __int16 sint16;
typedef unsigned __int32 uint32;
typedef          __int32 sint32;
#else

/* Basic Types */
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short sint16;
typedef unsigned int uint32;
typedef int sint32;
#endif

#ifndef NULL
#define NULL    ((void *)0)
#endif


#if defined (WIN32)
#pragma warning (disable:4201 4514 4214 4115)
#include <windows.h>
#pragma warning (default:4201 4214 4115)

/* Thread types */
#define CSSM_THREAD __declspec(thread);
#endif /* defined(WIN32) */

#if defined (OASIS)
#define CSSM_MAX_PATH 260
#else
#define CSSM_MAX_PATH _MAX_PATH
#endif

typedef void * CSSM_HANDLE;
typedef CSSM_HANDLE *  CSSM_HANDLE_PTR;
typedef void * CSSM_VL_HANDLE;    /* Verification Library Handle */
typedef void * CSSM_VO_HANDLE;    /* Verifiable Object Handle */
typedef void * CSSM_TP_HANDLE;    /* Trust Policy Handle */
typedef void * CSSM_CL_HANDLE;    /* Certificate Library Handle */
typedef void * CSSM_DL_HANDLE;    /* Database Library Handle */
typedef void * CSSM_DB_HANDLE;    /* Database Handle */
typedef void * CSSM_CC_HANDLE;    /* Cryptographic Context Handle */
typedef void * CSSM_CSP_HANDLE;   /* Cryptographic Service Provider Handle */
typedef void * CSSM_MODULE_HANDLE; /* Service provider
                                      Handle*/
typedef CSSM_MODULE_HANDLE * CSSM_MODULE_HANDLE_PTR;

#define CSSM_INVALID_HANDLE         (0)

typedef uint32 CSSM_TP_ACTION;

typedef enum cssm_tp_stop_on {
    CSSM_TP_STOP_ON_POLICY = 0,
    CSSM_TP_STOP_ON_NONE = 1,
    CSSM_TP_STOP_ON_FIRST_PASS = 2,
    CSSM_TP_STOP_ON_FIRST_FAIL = 3
} CSSM_TP_STOP_ON;

typedef uint32 CSSM_EVENT_TYPE, *CSSM_EVENT_TYPE_PTR;

#define CSSM_EVENT_ATTACH          (0)
#define CSSM_EVENT_DETACH          (1)
#define CSSM_EVENT_INFOATTACH      (2)
#define CSSM_EVENT_INFODETACH      (3)
#define CSSM_EVENT_CREATE_CONTEXT  (4)
#define CSSM_EVENT_DELETE_CONTEXT  (5)

typedef uint32 CSSM_KEY_TYPE, *CSSM_KEY_TYPE_PTR;

typedef uint32 CSSM_BOOL;

#define CSSM_TRUE   1
#define CSSM_FALSE  0

typedef enum cssm_return {
    CSSM_OK = 0,
    CSSM_FAIL = -1
} CSSM_RETURN;

typedef struct cssm_guid {
    uint32 Data1;
    uint16 Data2;
    uint16 Data3;
    uint8 Data4[8];
} CSSM_GUID, *CSSM_GUID_PTR;

typedef struct cssm_error {
    uint32 error;
    CSSM_GUID guid;
} CSSM_ERROR, *CSSM_ERROR_PTR;


/*
    CSSM_VERSION
*/
typedef struct cssm_version {
    uint32 Major;
    uint32 Minor;
} CSSM_VERSION, *CSSM_VERSION_PTR;

/*
 * This structure uniquely identifies a set of behaviors within a subservice 
 * within a CSSM add-in module.
 */
typedef struct cssm_subservice_uid {
    CSSM_GUID Guid;
    CSSM_VERSION Version;
    uint32 SubserviceId;
    uint32 SubserviceFlags;
} CSSM_SUBSERVICE_UID, *CSSM_SUBSERVICE_UID_PTR;

typedef CSSM_RETURN (CSSMAPI *CSSM_NOTIFY_CALLBACK) (
                                                CSSM_MODULE_HANDLE ModuleHandle,
                                                uint32 Application,
                                                uint32 Reason,
                                                void * Param);
typedef enum cssm_revoke_reason { 
    CSSM_REVOKE_CUSTOM = 0,
    CSSM_REVOKE_UNSPECIFIC = 1,
    CSSM_REVOKE_KEYCOMPROMISE = 2,
    CSSM_REVOKE_CACOMPROMISE = 3,
    CSSM_REVOKE_AFFILIATIONCHANGED = 4,
    CSSM_REVOKE_SUPERSEDED = 5,
    CSSM_REVOKE_CESSATIONOFOPERATION = 6,
    CSSM_REVOKE_CERTIFICATEHOLD = 7,
    CSSM_REVOKE_CERTIFICATEHOLDRELEASE = 8,
    CSSM_REVOKE_REMOVEFROMCRL = 9
} CSSM_REVOKE_REASON;

typedef struct cssm_data {
    uint32 Length;            /* in bytes */
    uint8 *Data;
} CSSM_DATA, *CSSM_DATA_PTR;

#define CSSM_MODULE_STRING_SIZE  64
typedef char CSSM_STRING [CSSM_MODULE_STRING_SIZE + 4];

/* Multi service modules */
typedef uint32 CSSM_SERVICE_MASK;
typedef CSSM_SERVICE_MASK CSSM_SERVICE_TYPE;

#define CSSM_SERVICE_CSSM         0x1
#define CSSM_SERVICE_CSP          0x2
#define CSSM_SERVICE_DL           0x4
#define CSSM_SERVICE_CL           0x8
#define CSSM_SERVICE_TP           0x10
#define CSSM_SERVICE_VL           0x40
#define CSSM_SERVICE_LAST CSSM_SERVICE_VL

#define CSSM_DB_DATASTORES_UNKNOWN (0xFFFFFFFF)
#define CSSM_ALL_SUBSERVICES       (0xFFFFFFFF)

/* indicates level of information disclosure by GetModuleInfo func */
typedef enum cssm_info_level {
	CSSM_INFO_LEVEL_MIN = -1,
    CSSM_INFO_LEVEL_MODULE    = 0,
            /* values from XXinfo struct */
    CSSM_INFO_LEVEL_SUBSERVICE    = 1,
            /* values from XXinfo and XXsubservice struct */
    CSSM_INFO_LEVEL_STATIC_ATTR    = 2,
            /* values from XXinfo and XXsubservice and
               all static-valued attributes of a subservice */
    CSSM_INFO_LEVEL_ALL_ATTR    = 3,
            /* values from XXinfo and XXsubservice and
               all attributes, static and dynamic,
               of a subservice */
	CSSM_INFO_LEVEL_MAX
} CSSM_INFO_LEVEL;


/*
 * This bitmask represents the exemptions requested by the calling application
 * process or thread. Exemptions are defined corresponding to built-in checks
 * performed by CSSM and the CSSM Module Managers. 
 */
typedef uint32 CSSM_EXEMPTION_MASK;

#define CSSM_EXEMPT_NONE 0x00000001
#define CSSM_EXEMPT_MULTI_ENCRYPT_CHECK 0x00000002
#define CSSM_EXEMPT_ALL 0xFFFFFFFF


/*  CSSM_USER_AUTHENTICATION_MECHANISM  */ 
typedef enum cssm_user_authentication_mechanism {
    CSSM_AUTHENTICATION_NONE = 0,
    CSSM_AUTHENTICATION_CUSTOM = 1,
    CSSM_AUTHENTICATION_PASSWORD = 2,
    CSSM_AUTHENTICATION_USERID_AND_PASSWORD = 3,
    CSSM_AUTHENTICATION_CERTIFICATE_AND_PASSPHRASE = 4,
    CSSM_AUTHENTICATION_LOGIN_AND_WRAP = 5
} CSSM_USER_AUTHENTICATION_MECHANISM;

typedef CSSM_DATA_PTR (CSSMAPI *CSSM_CALLBACK) (void *allocRef, uint32 ID);

typedef struct cssm_crypto_data {
    CSSM_DATA_PTR Param;
    CSSM_CALLBACK Callback;
    uint32	CallbackID;
} CSSM_CRYPTO_DATA, *CSSM_CRYPTO_DATA_PTR;

/*  CSSM_USER_AUTHENTICATION  */
typedef struct cssm_user_authentication {
    CSSM_DATA_PTR Credential;      /* a cert, a shared secret, other */
    CSSM_CRYPTO_DATA_PTR MoreAuthenticationData;
} CSSM_USER_AUTHENTICATION, *CSSM_USER_AUTHENTICATION_PTR;

typedef CSSM_DATA CSSM_OID, *CSSM_OID_PTR;

/*
 * Structure to encapsulate the name and GUID of an add-in module.
 */
typedef struct cssm_list_item {
    CSSM_SUBSERVICE_UID	SubserviceUid;
    char *Name;
} CSSM_LIST_ITEM, *CSSM_LIST_ITEM_PTR;

typedef struct cssm_list {
    uint32 NumberItems;
    CSSM_LIST_ITEM_PTR Items;
} CSSM_LIST, *CSSM_LIST_PTR;

typedef struct cssm_name_list {
    uint32 NumStrings;
    char** String;
} CSSM_NAME_LIST, *CSSM_NAME_LIST_PTR;


typedef enum cssm_cert_type {
    CSSM_CERT_UNKNOWN =  0x00,
    CSSM_CERT_X_509v1 =  0x01,
    CSSM_CERT_X_509v2 =  0x02,
    CSSM_CERT_X_509v3 =  0x03,
    CSSM_CERT_PGP =      0x04,
    CSSM_CERT_SPKI =     0x05,
    CSSM_CERT_SDSIv1 =   0x06,
    CSSM_CERT_Intel =    0x08,
    CSSM_CERT_X509_ATTRIBUTE = 0x09, /* X.509 attribute cert */
	CSSM_CERT_X9_ATTRIBUTE	= 0x0A, /* X9 attribute cert */
    CSSM_CERT_LAST =     0x7FFF
} CSSM_CERT_TYPE, *CSSM_CERT_TYPE_PTR;

/* Applications wishing to define their own custom certificate 
 * type should create a random uint32 whose value is greater than 
 * the CSSM_CL_CUSTOM_CERT_TYPE */
#define CSSM_CL_CUSTOM_CERT_TYPE  0x08000 

typedef enum cssm_cert_encoding {
    CSSM_CERT_ENCODING_UNKNOWN =   0x00,
    CSSM_CERT_ENCODING_CUSTOM  =   0x01,
    CSSM_CERT_ENCODING_BER     =  0x02,
    CSSM_CERT_ENCODING_DER     =  0x03,
    CSSM_CERT_ENCODING_NDR     =  0x04
} CSSM_CERT_ENCODING, *CSSM_CERT_ENCODING_PTR;


typedef struct cssm_certgroup {
	CSSM_CERT_TYPE CertType; /* Certificate domain/type identifier */
	CSSM_CERT_ENCODING  CertEncoding;		/* certificate encoding */
    uint32 NumCerts;
    CSSM_DATA_PTR CertList;
    void* reserved;
} CSSM_CERTGROUP, *CSSM_CERTGROUP_PTR;

/*
 * This structure represents the type of format used for revocation lists.
 */
typedef enum cssm_crl_type { 
    CSSM_CRL_TYPE_UNKNOWN,
    CSSM_CRL_TYPE_X_509v1,
    CSSM_CRL_TYPE_X_509v2,
} CSSM_CRL_TYPE, *CSSM_CRL_TYPE_PTR; 

/*
 * This structure represents the encoding format used for revocation lists.
 */
typedef enum cssm_crl_encoding { 
    CSSM_CRL_ENCODING_UNKNOWN,
    CSSM_CRL_ENCODING_CUSTOM,
    CSSM_CRL_ENCODING_BER,
    CSSM_CRL_ENCODING_DER,
    CSSM_CRL_ENCODING_BLOOM
} CSSM_CRL_ENCODING, *CSSM_CRL_ENCODING_PTR; 


#define CSSM_EVIDENCE_FORM_UNSPECIFIC 0x0 
#define CSSM_EVIDENCE_FORM_CERT 0x1 
#define CSSM_EVIDENCE_FORM_CRL 0x2 
/*
 * This structure contains certificates, CRLs and other information used as 
 * audit trail evidence.
 */
typedef struct cssm_evidence {
	uint32 EvidenceForm; /* CSSM_EVIDENCE_FORM_CERT,CSSM_EVIDENCE_FORM_CRL */
	union cssm_format_type {
		CSSM_CERT_TYPE CertType;
		CSSM_CRL_TYPE CrlType;
	} FormatType ;
	union cssm_format_encoding {
		CSSM_CERT_ENCODING CertEncoding;
		CSSM_CRL_ENCODING CrlEncoding;
	} FormatEncoding;

CSSM_DATA_PTR Evidence;	/* Evidence content */
} CSSM_EVIDENCE, *CSSM_EVIDENCE_PTR; 

/*
 * This enumerated list defines the application-level protocols that could be
 * supported by a Certificate Library Module that communicates with Certification 
 * Authorities, Registration Authorities and other services, or by a Data Storage
 * Library Module that communicates with service-based storage and directory 
 * services.
 */
typedef enum cssm_net_protocol {
    CSSM_NET_PROTO_NONE = 0, /* local */
    CSSM_NET_PROTO_CUSTOM = 1, /* proprietary implementation */
    CSSM_NET_PROTO_UNSPECIFIED = 2, /* implementation default */
    CSSM_NET_PROTO_LDAP = 3, /* light weight directory access protocol */
    CSSM_NET_PROTO_LDAPS = 4, /* ldap/ssl where SSL initiates the connection */
    CSSM_NET_PROTO_LDAPNS = 5, /* ldap where ldap negotiates an SSL session */
    CSSM_NET_PROTO_X500DAP = 6, /* x.500 Directory access protocol */
    CSSM_NET_PROTO_FTPDAP = 7, /* file transfer protocol for cert/crl fetch */
    CSSM_NET_PROTO_FTPDAPS = 8, /* ftp/ssl where SSL initiates the connection */
    CSSM_NET_PROTO_NDS = 9, /* Novell directory services */
    CSSM_NET_PROTO_OCSP = 10, /* online certificate status protocol */
    CSSM_NET_PROTO_PKIX3 = 11, /* the cert request protocol in PKIX3 */
    CSSM_NET_PROTO_PKIX3S = 12, /* The ssl/tls derivative of PKIX3 */
    CSSM_NET_PROTO_PKCS_HTTP = 13, /* PKCS client <=> CA protocol over HTTP */
    CSSM_NET_PROTO_PKCS_HTTPS = 14, /* PKCS client <=> CA protocol over HTTPS */
} CSSM_NET_PROTOCOL;

typedef struct cssm_dl_db_handle {
    CSSM_DL_HANDLE DLHandle;
    CSSM_DB_HANDLE DBHandle;
} CSSM_DL_DB_HANDLE, *CSSM_DL_DB_HANDLE_PTR;

typedef struct cssm_dl_db_list {
    uint32 NumHandles;
    CSSM_DL_DB_HANDLE_PTR DLDBHandle;
} CSSM_DL_DB_LIST, *CSSM_DL_DB_LIST_PTR;

/*  CSSM_DB_ATTRIBUTE_NAME_FORMAT  */ 
typedef enum cssm_db_attribute_name_format {
    CSSM_DB_ATTRIBUTE_NAME_AS_STRING = 0, 
    CSSM_DB_ATTRIBUTE_NAME_AS_OID = 1,
	CSSM_DB_ATTRIBUTE_NAME_AS_BLOB = 2
} CSSM_DB_ATTRIBUTE_NAME_FORMAT, *CSSM_DB_ATTRIBUTE_NAME_FORMAT_PTR;

typedef enum cssm_db_attribute_format {
    CSSM_DB_ATTRIBUTE_FORMAT_STRING = 0,
    CSSM_DB_ATTRIBUTE_FORMAT_INTEGER = 1,
    CSSM_DB_ATTRIBUTE_FORMAT_REAL = 2,
    CSSM_DB_ATTRIBUTE_FORMAT_TIME = 3,
    CSSM_DB_ATTRIBUTE_FORMAT_MONEY = 4,
    CSSM_DB_ATTRIBUTE_FORMAT_BLOB = 5,
} CSSM_DB_ATTRIBUTE_FORMAT, *CSSM_DB_ATTRIBUTE_FORMAT_PTR;

/*  CSSM_DB_ATTRIBUTE_INFO  */
typedef struct cssm_db_attribute_info {
    CSSM_DB_ATTRIBUTE_NAME_FORMAT AttributeNameFormat; 
    union cssm_db_attribute_label {
		CSSM_DATA Name;
        char * AttributeName;             /* eg. "record label" */
        CSSM_OID AttributeID;             /* eg. CSSMOID_RECORDLABEL */
	} Label;
	CSSM_DB_ATTRIBUTE_FORMAT AttributeFormat;
} CSSM_DB_ATTRIBUTE_INFO, *CSSM_DB_ATTRIBUTE_INFO_PTR;

/*  CSSM_DB_ATTRIBUTE_DATA  */
typedef struct cssm_db_attribute_data {
    CSSM_DB_ATTRIBUTE_INFO Info; 
    CSSM_DATA Value;
} CSSM_DB_ATTRIBUTE_DATA, *CSSM_DB_ATTRIBUTE_DATA_PTR;


/*  *****  Record-related structures  *****  */

/*  CSSM_DB_RECORDTYPE  */
typedef enum cssm_db_recordtype {
    CSSM_DL_DB_RECORD_GENERIC = 0,
    CSSM_DL_DB_RECORD_CERT = 1,
    CSSM_DL_DB_RECORD_CRL = 2,
    CSSM_DL_DB_RECORD_KEY = 3,
    CSSM_DL_DB_RECORD_POLICY = 4
} CSSM_DB_RECORDTYPE;

/*  CSSM_DB_XXXRECORD_SEMANTICS  */
/* (XXX can be Cert, CRL, Policy, etc.)  */
/* It is expected that there will be certain semantic types associated
   with certs, CRLs, policies, etc.
   Each type is expected to have a bit mask which is capable of describing
   general information about how the record should be used.  CRL, KEY, 
   and POLICY semantic bit masks will be defined as needed .  */

/*  CSSM_DB_CERTRECORD_SEMANTICS  */
/*  Optional semantic information associated with certificate records.  */
#define CSSM_DB_CERT_USE_TRUSTED 0x00000001 /* application-defined as trusted */
#define CSSM_DB_CERT_USE_SYSTEM  0x00000002 /* the CSSM system cert */
#define CSSM_DB_CERT_USE_OWNER   0x00000004 /* private key owned by system user*/
#define CSSM_DB_CERT_USE_REVOKED 0x00000008 /* revoked cert - used w\ CRL APIs */
#define CSSM_DB_CERT_USE_SIGNING 0x00000010 /* use cert for signing only */ 
#define CSSM_DB_CERT_USE_PRIVACY 0x00000020 /* use cert for confidentiality only */

/*  CSSM_DB_RECORD_ATTRIBUTE_INFO  */
typedef struct cssm_db_record_attribute_info {
    CSSM_DB_RECORDTYPE DataRecordType;
    uint32 NumberOfAttributes; 
    CSSM_DB_ATTRIBUTE_INFO_PTR AttributeInfo;
} CSSM_DB_RECORD_ATTRIBUTE_INFO, *CSSM_DB_RECORD_ATTRIBUTE_INFO_PTR;

/*  CSSM_DB_RECORD_ATTRIBUTE_DATA  */
typedef struct cssm_db_record_attribute_data {
    CSSM_DB_RECORDTYPE DataRecordType;
    uint32 SemanticInformation; 
    uint32 NumberOfAttributes; 
    CSSM_DB_ATTRIBUTE_DATA_PTR AttributeData;
} CSSM_DB_RECORD_ATTRIBUTE_DATA, *CSSM_DB_RECORD_ATTRIBUTE_DATA_PTR;

/*  CSSM_DB_RECORD_PARSING_FUNCTION_TABLE  */ 
typedef struct cssm_db_record_parsing_fntable {
	CSSM_DATA_PTR (CSSMAPI *RecordGetFirstFieldValue) 
									(CSSM_HANDLE Handle,
									const CSSM_DATA_PTR Data,
									const CSSM_OID_PTR DataField,
									CSSM_HANDLE_PTR ResultsHandle,
									uint32 *NumberOfMatchedFields);
	CSSM_DATA_PTR (CSSMAPI *RecordGetNextFieldValue) 
									(CSSM_HANDLE Handle,
									CSSM_HANDLE ResultsHandle);
	CSSM_RETURN (CSSMAPI *RecordAbortQuery)
									(CSSM_HANDLE Handle,
									CSSM_HANDLE ResultsHandle);
} CSSM_DB_RECORD_PARSING_FNTABLE, *CSSM_DB_RECORD_PARSING_FNTABLE_PTR; 

/* CSSM_DB_PARSING_INFO */
typedef struct cssm_db_parsing_module_info {
    CSSM_DB_RECORDTYPE RecordType;
    CSSM_SUBSERVICE_UID ModuleSubserviceUid;
} CSSM_DB_PARSING_MODULE_INFO, *CSSM_DB_PARSING_MODULE_INFO_PTR;

/*  CSSM_DB_INDEX_TYPE  */
typedef enum cssm_db_index_type {
    CSSM_DB_INDEX_UNIQUE = 0,
    CSSM_DB_INDEX_NONUNIQUE = 1
} CSSM_DB_INDEX_TYPE;

/*  CSSM_DB_INDEXED_DATA_LOCATION  */ 
typedef enum cssm_db_indexed_data_location {
    CSSM_DB_INDEX_ON_UNKNOWN = 0,
    CSSM_DB_INDEX_ON_ATTRIBUTE = 1,
    CSSM_DB_INDEX_ON_RECORD = 2
} CSSM_DB_INDEXED_DATA_LOCATION;

/*  CSSM_DB_INDEX_INFO  */
typedef struct cssm_db_index_info {
    CSSM_DB_INDEX_TYPE IndexType; 
    CSSM_DB_INDEXED_DATA_LOCATION IndexedDataLocation; 
    CSSM_DB_ATTRIBUTE_INFO Info;
} CSSM_DB_INDEX_INFO, *CSSM_DB_INDEX_INFO_PTR;

/*  CSSM_DB_UNIQUE_RECORD  */
typedef struct cssm_db_unique_record {
    CSSM_DB_INDEX_INFO RecordLocator; 
    CSSM_DATA RecordIdentifier;
} CSSM_DB_UNIQUE_RECORD, *CSSM_DB_UNIQUE_RECORD_PTR;

/*  CSSM_DB_RECORD_INDEX_INFO  */
typedef struct cssm_db_record_indexinfo {
    CSSM_DB_RECORDTYPE DataRecordType; 
    uint32 NumberOfIndexes; 
    CSSM_DB_INDEX_INFO_PTR IndexInfo;
} CSSM_DB_RECORD_INDEX_INFO, *CSSM_DB_RECORD_INDEX_INFO_PTR;


/*  CSSM_DB_ACCESS_TYPE  */
typedef uint32 CSSM_DB_ACCESS_TYPE, *CSSM_DB_ACCESS_TYPE_PTR;

#define CSSM_DB_ACCESS_READ		0x00001
#define CSSM_DB_ACCESS_WRITE		0x00002
#define CSSM_DB_ACCESS_PRIVILEGED	0x00004		/* versus user mode */
#define CSSM_DB_ACCESS_ASYNCHRONOUS	0x00008		/* versus synchronous */

/*  CSSM_DBINFO  */
typedef struct cssm_dbInfo {
/* meta information about each record type stored in this data store 
   including meta information about record attributes and indexes */
    uint32 NumberOfRecordTypes;
    CSSM_DB_PARSING_MODULE_INFO_PTR DefaultParsingModules;
    CSSM_DB_RECORD_ATTRIBUTE_INFO_PTR RecordAttributeNames;
    CSSM_DB_RECORD_INDEX_INFO_PTR RecordIndexes;

    /* access restrictions for opening this data store */
    CSSM_USER_AUTHENTICATION_MECHANISM AuthenticationMechanism;

    /* transparent integrity checking options for this data store */
    CSSM_BOOL RecordSigningImplemented;
    CSSM_DATA SigningCertificate;
    CSSM_SUBSERVICE_UID SigningCspSubserviceUid;

    /* additional information */
    CSSM_BOOL IsLocal;
    char *AccessPath;           /* URL, dir path, etc */
    void *Reserved;
} CSSM_DBINFO, *CSSM_DBINFO_PTR;


/*  CSSM_DB_OPERATOR  */
typedef enum cssm_db_operator {
    CSSM_DB_EQUAL = 0,
    CSSM_DB_NOT_EQUAL = 1,
    CSSM_DB_APPROX_EQUAL = 2,
    CSSM_DB_LESS_THAN = 3,
    CSSM_DB_GREATER_THAN = 4,
    CSSM_DB_EQUALS_INITIAL_SUBSTRING = 5,
    CSSM_DB_EQUALS_ANY_SUBSTRING = 6,
    CSSM_DB_EQUALS_FINAL_SUBSTRING = 7,
    CSSM_DB_EXISTS = 8
} CSSM_DB_OPERATOR, *CSSM_DB_OPERATOR_PTR;

/*  CSSM_DB_CONJUNCTIVE  */
typedef enum cssm_db_conjunctive{
    CSSM_DB_NONE = 0,
    CSSM_DB_AND = 1,
    CSSM_DB_OR = 2
} CSSM_DB_CONJUNCTIVE, *CSSM_DB_CONJUNCTIVE_PTR;

/* CSSM_SELECTION_PREDICATE  */
typedef struct cssm_selection_predicate {
    CSSM_DB_OPERATOR DbOperator;
    CSSM_DB_ATTRIBUTE_DATA Attribute;
} CSSM_SELECTION_PREDICATE, *CSSM_SELECTION_PREDICATE_PTR;

#define CSSM_QUERY_TIMELIMIT_NONE   0
#define CSSM_QUERY_SIZELIMIT_NONE   0

/*  CSSM_QUERY_LIMITS  */
typedef struct cssm_query_limits {
    uint32 TimeLimit;         /* in seconds */
    uint32 SizeLimit;         /* max. number of records to return */
} CSSM_QUERY_LIMITS, *CSSM_QUERY_LIMITS_PTR;

typedef uint32 CSSM_QUERY_FLAGS;

#define CSSM_QUERY_RETURN_DATA  0x1  /* On = Output in common data format
                                        Off = Output in DL native format */

/*  CSSM_QUERY  */
typedef struct cssm_query {
    CSSM_DB_RECORDTYPE RecordType;
    CSSM_DB_CONJUNCTIVE Conjunctive;
    uint32 NumSelectionPredicates;
    CSSM_SELECTION_PREDICATE_PTR SelectionPredicate;
    CSSM_QUERY_LIMITS QueryLimits;
    CSSM_QUERY_FLAGS QueryFlags;
} CSSM_QUERY, *CSSM_QUERY_PTR;

/*  CSSM_DLTYPE  */
typedef enum cssm_dltype {
    CSSM_DL_UNKNOWN = 0,
    CSSM_DL_CUSTOM = 1,
    CSSM_DL_LDAP = 2,
    CSSM_DL_ODBC = 3,
    CSSM_DL_PKCS11 = 4,
    CSSM_DL_FFS = 5,          /* flat file system  or fast file system */
    CSSM_DL_MEMORY = 6, 
    CSSM_DL_REMOTEDIR = 7
} CSSM_DLTYPE, *CSSM_DLTYPE_PTR;

/*  CSSM_DL_PKCS11_ATTRIBUTES  */
typedef struct cssm_dl_pkcs11_attributes {
    uint32 DeviceAccessFlags;
} CSSM_DL_PKCS11_ATTRIBUTES, *CSSM_DL_PKCS11_ATTRIBUTES_PTR; 

typedef void *CSSM_DL_CUSTOM_ATTRIBUTES;
typedef void *CSSM_DL_LDAP_ATTRIBUTES;
typedef void *CSSM_DL_ODBC_ATTRIBUTES;
typedef void *CSSM_DL_FFS_ATTRIBUTES;

/*
    CSSM_DL_WRAPPEDPRODUCTINFO
*/

typedef struct cssm_dl_wrappedproductinfo {
    CSSM_VERSION StandardVersion;     /* Version of standard this product conforms to */
    CSSM_STRING StandardDescription;  /* Description of standard this product conforms to */
    CSSM_VERSION ProductVersion;      /* Version of wrapped product/library */
    CSSM_STRING ProductDescription;   /* Description of wrapped product/library */
    CSSM_STRING ProductVendor;        /* Vendor of wrapped product/library */
    uint32 ProductFlags;              /* ProductFlags */
	CSSM_NET_PROTOCOL NetworkProtocol;/* The network protocol supported by a remote storage service */

} CSSM_DL_WRAPPEDPRODUCTINFO, *CSSM_DL_WRAPPEDPRODUCTINFO_PTR;

typedef struct cssm_dlsubservice {
    uint32 SubServiceId;
    CSSM_STRING Description;          /* Description of this sub service */
    CSSM_DLTYPE Type;
    union cssm_dlsubservice_attributes {
        CSSM_DL_CUSTOM_ATTRIBUTES CustomAttributes;
        CSSM_DL_LDAP_ATTRIBUTES LdapAttributes;
        CSSM_DL_ODBC_ATTRIBUTES OdbcAttributes;
        CSSM_DL_PKCS11_ATTRIBUTES_PTR Pkcs11Attributes;
        CSSM_DL_FFS_ATTRIBUTES FfsAttributes;
    } Attributes;

    CSSM_DL_WRAPPEDPRODUCTINFO WrappedProduct;
    CSSM_USER_AUTHENTICATION_MECHANISM AuthenticationMechanism;

    /* meta information about the query support provided by the module */
    uint32 NumberOfRelOperatorTypes;
    CSSM_DB_OPERATOR_PTR RelOperatorTypes;
    uint32 NumberOfConjOperatorTypes;
    CSSM_DB_CONJUNCTIVE_PTR ConjOperatorTypes;
    CSSM_BOOL QueryLimitsSupported;

    /* meta information about the encapsulated data stores (if known) */
    uint32 NumberOfDataStores;
    CSSM_NAME_LIST_PTR DataStoreNames;
    CSSM_DBINFO_PTR DataStoreInfo;

    /* additional information */
    void *Reserved;
} CSSM_DLSUBSERVICE, *CSSM_DLSUBSERVICE_PTR;

/* Static data associated with a data storage library add-in module */

typedef enum cssm_algorithms {
    CSSM_ALGID_NONE      = 0,
    CSSM_ALGID_CUSTOM    = CSSM_ALGID_NONE+1, /* Custom algorithm */
    CSSM_ALGID_DH        = CSSM_ALGID_NONE+2, /* Diffie Hellman key exchange algorithm */
    CSSM_ALGID_PH        = CSSM_ALGID_NONE+3, /* Pohlig Hellman key exchange algorithm */
    CSSM_ALGID_KEA       = CSSM_ALGID_NONE+4, /* Key Exchange Algorithm */
    CSSM_ALGID_MD2       = CSSM_ALGID_NONE+5, /* MD2  hash algorithm  (invented by Ron Rivest) */
    CSSM_ALGID_MD4       = CSSM_ALGID_NONE+6, /* MD4  hash algorithm  (invented by Ron Rivest) */
    CSSM_ALGID_MD5       = CSSM_ALGID_NONE+7, /* MD5  hash algorithm  (invented by Ron Rivest) */
    CSSM_ALGID_SHA1      = CSSM_ALGID_NONE+8, /* Secure Hash Algorithm  (developed by NIST/NSA) */
    CSSM_ALGID_NHASH     = CSSM_ALGID_NONE+9, /* N-Hash algorithm(developed by Nippon Telephone and Telegraph) */
    CSSM_ALGID_HAVAL     = CSSM_ALGID_NONE+10,/* HAVAL  hash algorithm  (MD5 variant) */
    CSSM_ALGID_RIPEMD    = CSSM_ALGID_NONE+11,/* RIPE-MD  (160) hash algorithm  (MD4 variant - developed for the European Community's RIPE project) */
    CSSM_ALGID_IBCHASH   = CSSM_ALGID_NONE+12,/* IBC-Hash (keyed hash algorithm or MAC) */
    CSSM_ALGID_RIPEMAC   = CSSM_ALGID_NONE+13,/* RIPE-MAC (invented by Bart Preneel) */
    CSSM_ALGID_DES       = CSSM_ALGID_NONE+14,/* Data Encryption Standard block cipher */
    CSSM_ALGID_DESX      = CSSM_ALGID_NONE+15,/* DESX block cipher  (DES variant from RSA) */
    CSSM_ALGID_RDES      = CSSM_ALGID_NONE+16,/* RDES block cipher  (DES variant) */
    CSSM_ALGID_3DES_3KEY = CSSM_ALGID_NONE+17,/* Triple-DES block cipher  (with 3 keys) */
    CSSM_ALGID_3DES_2KEY = CSSM_ALGID_NONE+18,/* Triple-DES block cipher  (with 2 keys) */
    CSSM_ALGID_3DES_1KEY = CSSM_ALGID_NONE+19,/* Triple-DES block cipher  (with 1 key) */
    CSSM_ALGID_IDEA      = CSSM_ALGID_NONE+20,/* IDEA block cipher  (invented by Lai and Massey) */
    CSSM_ALGID_RC2       = CSSM_ALGID_NONE+21,/* RC2 block cipher  (invented by Ron Rivest) */
    CSSM_ALGID_RC5       = CSSM_ALGID_NONE+22,/* RC5 block cipher  (invented by Ron Rivest) */
    CSSM_ALGID_RC4       = CSSM_ALGID_NONE+23,/* RC4 stream cipher  (invented by Ron Rivest) */
    CSSM_ALGID_SEAL      = CSSM_ALGID_NONE+24,/* SEAL stream cipher  (invented by Rogaway and Coppersmith) */
    CSSM_ALGID_CAST      = CSSM_ALGID_NONE+25,/* CAST block cipher  (invented by Adams and Tavares) */
    CSSM_ALGID_BLOWFISH  = CSSM_ALGID_NONE+26,/* BLOWFISH block cipher  (invented by Schneier) */
    CSSM_ALGID_SKIPJACK  = CSSM_ALGID_NONE+27,/* Skipjack block cipher  (developed by NSA) */
    CSSM_ALGID_LUCIFER   = CSSM_ALGID_NONE+28,/* Lucifer block cipher  (developed by IBM) */
    CSSM_ALGID_MADRYGA   = CSSM_ALGID_NONE+29,/* Madryga block cipher  (invented by Madryga) */
    CSSM_ALGID_FEAL      = CSSM_ALGID_NONE+30,/* FEAL block cipher  (invented by Shimizu and Miyaguchi) */
    CSSM_ALGID_REDOC     = CSSM_ALGID_NONE+31,/* REDOC 2 block cipher  (invented by Michael Wood) */
    CSSM_ALGID_REDOC3    = CSSM_ALGID_NONE+32,/* REDOC 3 block cipher  (invented by Michael Wood) */
    CSSM_ALGID_LOKI      = CSSM_ALGID_NONE+33,/* LOKI block cipher */
    CSSM_ALGID_KHUFU     = CSSM_ALGID_NONE+34,/* KHUFU block cipher  (invented by Ralph Merkle) */
    CSSM_ALGID_KHAFRE    = CSSM_ALGID_NONE+35,/* KHAFRE block cipher  (invented by Ralph Merkle) */
    CSSM_ALGID_MMB       = CSSM_ALGID_NONE+36,/* MMB block cipher  (IDEA variant) */
    CSSM_ALGID_GOST      = CSSM_ALGID_NONE+37,/* GOST block cipher  (developed by the former Soviet Union) */
    CSSM_ALGID_SAFER     = CSSM_ALGID_NONE+38,/* SAFER K-64 block cipher  (invented by Massey) */
    CSSM_ALGID_CRAB      = CSSM_ALGID_NONE+39,/* CRAB block cipher  (invented by Kaliski and Robshaw) */
    CSSM_ALGID_RSA       = CSSM_ALGID_NONE+40,/* RSA public key cipher */
    CSSM_ALGID_DSA       = CSSM_ALGID_NONE+41,/* Digital Signature Algorithm */
    CSSM_ALGID_MD5WithRSA= CSSM_ALGID_NONE+42,/* MD5/RSA signature algorithm */
    CSSM_ALGID_MD2WithRSA= CSSM_ALGID_NONE+43,/* MD2/RSA signature algorithm */
    CSSM_ALGID_ElGamal   = CSSM_ALGID_NONE+44,/* ElGamal signature algorithm */
    CSSM_ALGID_MD2Random = CSSM_ALGID_NONE+45,/* MD2-based random numbers */
    CSSM_ALGID_MD5Random = CSSM_ALGID_NONE+46,/* MD5-based random numbers */
    CSSM_ALGID_SHARandom = CSSM_ALGID_NONE+47,/* SHA-based random numbers */
    CSSM_ALGID_DESRandom = CSSM_ALGID_NONE+48, /* DES-based random numbers */
    CSSM_ALGID_SHA1WithRSA = CSSM_ALGID_NONE+49, /* SHA1/RSA signature algorithm */
    CSSM_ALGID_RSA_PKCS  = CSSM_ALGID_RSA,     /* RSA as specified in PKCS #1 */
    CSSM_ALGID_RSA_ISO9796 = CSSM_ALGID_NONE+50, /* RSA as specified in ISO 9796 */
    CSSM_ALGID_RSA_RAW   = CSSM_ALGID_NONE+51, /* Raw RSA as assumed in X.509 */
    CSSM_ALGID_CDMF      = CSSM_ALGID_NONE+52, /* CDMF block cipher */
    CSSM_ALGID_CAST3     = CSSM_ALGID_NONE+53, /* Entrust's CAST3 block cipher */
    CSSM_ALGID_CAST5     = CSSM_ALGID_NONE+54, /* Entrust's CAST5 block cipher */
    CSSM_ALGID_GenericSecret = CSSM_ALGID_NONE+55, /* Generic secret operations */
    CSSM_ALGID_ConcatBaseAndKey = CSSM_ALGID_NONE+56, /* Concatenate two keys, base key first */
    CSSM_ALGID_ConcatKeyAndBase = CSSM_ALGID_NONE+57, /* Concatenate two keys, base key last */
    CSSM_ALGID_ConcatBaseAndData = CSSM_ALGID_NONE+58, /* Concatenate base key and random data, key first */
    CSSM_ALGID_ConcatDataAndBase = CSSM_ALGID_NONE+59, /* Concatenate base key and data, data first */
    CSSM_ALGID_XORBaseAndData = CSSM_ALGID_NONE+60, /* XOR a byte string with the base key */
    CSSM_ALGID_ExtractFromKey = CSSM_ALGID_NONE+61, /* Extract a key from base key, starting at arbitrary bit position */
    CSSM_ALGID_SSL3PreMasterGen = CSSM_ALGID_NONE+62, /* Generate a 48 byte SSL 3 pre-master key */
    CSSM_ALGID_SSL3MasterDerive = CSSM_ALGID_NONE+63, /* Derive an SSL 3 key from a pre-master key */
    CSSM_ALGID_SSL3KeyAndMacDerive = CSSM_ALGID_NONE+64, /* Derive the keys and MACing keys for the SSL cipher suite */
    CSSM_ALGID_SSL3MD5_MAC = CSSM_ALGID_NONE+65, /* Performs SSL 3 MD5 MACing */
    CSSM_ALGID_SSL3SHA1_MAC = CSSM_ALGID_NONE+66, /* Performs SSL 3 SHA-1 MACing */
    CSSM_ALGID_MD5_PBE   = CSSM_ALGID_NONE+67, /* Generate key and IV by MD5 hashing a base key */
    CSSM_ALGID_MD2_PBE   = CSSM_ALGID_NONE+68, /* Generate key and IV by MD2 hashing a base key */
    CSSM_ALGID_SHA1_PBE  = CSSM_ALGID_NONE+69, /* Generate key and IV by SHA-1 hashing a base key */
    CSSM_ALGID_WrapLynks = CSSM_ALGID_NONE+70, /* Spyrus LYNKS DES based wrapping scheme w/checksum */
    CSSM_ALGID_WrapSET_OAEP = CSSM_ALGID_NONE+71, /* SET key wrapping */
    CSSM_ALGID_BATON     = CSSM_ALGID_NONE+72, /* Fortezza BATON cipher */
    CSSM_ALGID_ECDSA     = CSSM_ALGID_NONE+73, /* Elliptic Curve DSA */
    CSSM_ALGID_MAYFLY    = CSSM_ALGID_NONE+74, /* Fortezza MAYFLY cipher */
    CSSM_ALGID_JUNIPER   = CSSM_ALGID_NONE+75, /* Fortezza JUNIPER cipher */
    CSSM_ALGID_FASTHASH  = CSSM_ALGID_NONE+76, /* Fortezza FASTHASH */
    CSSM_ALGID_3DES      = CSSM_ALGID_NONE+77, /* Generic 3DES */
    CSSM_ALGID_SSL3MD5   = CSSM_ALGID_NONE+78, /* SSL3MD5 */
    CSSM_ALGID_SSL3SHA1  = CSSM_ALGID_NONE+79, /* SSL3SHA1 */
    CSSM_ALGID_FortezzaTimestamp = CSSM_ALGID_NONE+80, /* FortezzaTimestamp */
    CSSM_ALGID_SHA1WithDSA = CSSM_ALGID_NONE+81, /* SHA1WithDSA */
    CSSM_ALGID_SHA1WithECDSA = CSSM_ALGID_NONE+82, /* SHA1WithECDSA */
    CSSM_ALGID_DSA_BSAFE = CSSM_ALGID_NONE+83,  /* BSafe Key format */
	CSSM_ALGID_ECDH = CSSM_ALGID_NONE+84,                /* Elliptic Curve DiffieHellman Key Exchange algorithm*/
	CSSM_ALGID_ECMQV = CSSM_ALGID_NONE+85,               /* Elliptic Curve MQV key exchange algorithm*/
	CSSM_ALGID_PKCS12_SHA1_PBE = CSSM_ALGID_NONE+86,     /* PKCS12 SHA-1 Password key derivation algorithm*/
	CSSM_ALGID_ECNRA = CSSM_ALGID_NONE+87,               /* Elliptic Curve Nyberg-Rueppel*/
	CSSM_ALGID_SHA1WithECNRA = CSSM_ALGID_NONE+88,      /* SHA-1 with Elliptic Curve Nyberg-Rueppel*/
	CSSM_ALGID_ECES = CSSM_ALGID_NONE+89,               /* Elliptic Curve Encryption Scheme*/
	CSSM_ALGID_ECAES = CSSM_ALGID_NONE+90,               /* Elliptic Curve Authenticate Encryption Scheme*/
	CSSM_ALGID_SHA1HMAC = CSSM_ALGID_NONE+91,           /* SHA1-MAC*/
	CSSM_ALGID_FIPS186Random = CSSM_ALGID_NONE+92,      /* FIPs86Random*/
	CSSM_ALGID_ECC = CSSM_ALGID_NONE+93,                 /* ECC*/
	CSSM_ALGID_MQV = CSSM_ALGID_NONE+94,          /* Discrete-Log MQV key exchange algorithm*/
	CSSM_ALGID_NRA  = CSSM_ALGID_NONE+95,          /* Discrete-Log Nyberg-Rueppel Signature scheme*/
	CSSM_ALGID_LAST = CSSM_ALGID_NONE+96       
} CSSM_ALGORITHMS;

typedef enum cssm_encrypt_mode {
    CSSM_ALGMODE_NONE       = 0,
    CSSM_ALGMODE_CUSTOM     = CSSM_ALGMODE_NONE+1, /* Custom mode */
    CSSM_ALGMODE_ECB        = CSSM_ALGMODE_NONE+2, /* Electronic Code Book */
    CSSM_ALGMODE_ECBPad     = CSSM_ALGMODE_NONE+3, /* ECB with padding */
    CSSM_ALGMODE_CBC        = CSSM_ALGMODE_NONE+4, /* Cipher Block Chaining */
    CSSM_ALGMODE_CBC_IV8    = CSSM_ALGMODE_NONE+5, /* CBC with Initialization Vector of 8 bytes */
    CSSM_ALGMODE_CBCPadIV8  = CSSM_ALGMODE_NONE+6, /* CBC with padding and Initialization Vector of 8 bytes */
    CSSM_ALGMODE_CFB        = CSSM_ALGMODE_NONE+7, /* Cipher FeedBack */
    CSSM_ALGMODE_CFB_IV8    = CSSM_ALGMODE_NONE+8, /* CFB with Initialization Vector of 8 bytes */
    CSSM_ALGMODE_CFBPadIV8  = CSSM_ALGMODE_NONE+9, /* CFB used in MS CAPI 2.0 */
    CSSM_ALGMODE_OFB        = CSSM_ALGMODE_NONE+10, /* Output FeedBack */
    CSSM_ALGMODE_OFB_IV8    = CSSM_ALGMODE_NONE+11, /* OFB with Initialization Vector of 8 bytes */
    CSSM_ALGMODE_OFBPadIV8  = CSSM_ALGMODE_NONE+12, /* OFB used in MS CAPI 2.0 */
    CSSM_ALGMODE_COUNTER    = CSSM_ALGMODE_NONE+13, /* Counter */
    CSSM_ALGMODE_BC         = CSSM_ALGMODE_NONE+14, /* Block Chaining */
    CSSM_ALGMODE_PCBC       = CSSM_ALGMODE_NONE+15, /* Propagating CBC */
    CSSM_ALGMODE_CBCC       = CSSM_ALGMODE_NONE+16, /* CBC with Checksum */
    CSSM_ALGMODE_OFBNLF     = CSSM_ALGMODE_NONE+17, /* OFB with NonLinear Function */
    CSSM_ALGMODE_PBC        = CSSM_ALGMODE_NONE+18, /* Plaintext Block Chaining */
    CSSM_ALGMODE_PFB        = CSSM_ALGMODE_NONE+19, /* Plaintext FeedBack */
    CSSM_ALGMODE_CBCPD      = CSSM_ALGMODE_NONE+20, /* CBC of Plaintext Difference */
    CSSM_ALGMODE_PUBLIC_KEY = CSSM_ALGMODE_NONE+21, /* Encrypt with the public key */
    CSSM_ALGMODE_PRIVATE_KEY= CSSM_ALGMODE_NONE+22, /* Encrypt with the private key */
    CSSM_ALGMODE_SHUFFLE    = CSSM_ALGMODE_NONE+23, /* Fortezza shuffle mode */
    CSSM_ALGMODE_ECB64      = CSSM_ALGMODE_NONE+24,
    CSSM_ALGMODE_CBC64      = CSSM_ALGMODE_NONE+25,
    CSSM_ALGMODE_OFB64      = CSSM_ALGMODE_NONE+26,
    CSSM_ALGMODE_CFB64      = CSSM_ALGMODE_NONE+27,
    CSSM_ALGMODE_CFB32      = CSSM_ALGMODE_NONE+28,
    CSSM_ALGMODE_CFB16      = CSSM_ALGMODE_NONE+29,
    CSSM_ALGMODE_CFB8       = CSSM_ALGMODE_NONE+30,
    CSSM_ALGMODE_WRAP       = CSSM_ALGMODE_NONE+31,
    CSSM_ALGMODE_PRIVATE_WRAP = CSSM_ALGMODE_NONE+32,
    CSSM_ALGMODE_RELAYX     = CSSM_ALGMODE_NONE+33,
    CSSM_ALGMODE_ECB128     = CSSM_ALGMODE_NONE+34,
    CSSM_ALGMODE_ECB96      = CSSM_ALGMODE_NONE+35,
    CSSM_ALGMODE_CBC128     = CSSM_ALGMODE_NONE+36,
    CSSM_ALGMODE_OAEP_HASH  = CSSM_ALGMODE_NONE+37,
    CSSM_ALGMODE_LAST		= CSSM_ALGMODE_NONE+38
} CSSM_ENCRYPT_MODE;

typedef enum cssm_context_type {
    CSSM_ALGCLASS_NONE      = 0,
    CSSM_ALGCLASS_CUSTOM    = CSSM_ALGCLASS_NONE+1,
    CSSM_ALGCLASS_KEYXCH    = CSSM_ALGCLASS_NONE+2,
    CSSM_ALGCLASS_SIGNATURE = CSSM_ALGCLASS_NONE+3,
    CSSM_ALGCLASS_SYMMETRIC = CSSM_ALGCLASS_NONE+4,
    CSSM_ALGCLASS_DIGEST    = CSSM_ALGCLASS_NONE+5,
    CSSM_ALGCLASS_RANDOMGEN = CSSM_ALGCLASS_NONE+6,
    CSSM_ALGCLASS_UNIQUEGEN = CSSM_ALGCLASS_NONE+7,
    CSSM_ALGCLASS_MAC       = CSSM_ALGCLASS_NONE+8,
    CSSM_ALGCLASS_ASYMMETRIC= CSSM_ALGCLASS_NONE+9,
    CSSM_ALGCLASS_KEYGEN    = CSSM_ALGCLASS_NONE+10,
    CSSM_ALGCLASS_DERIVEKEY = CSSM_ALGCLASS_NONE+11
}CSSM_CONTEXT_TYPE;

/* Attribute data type tags */
#define CSSM_ATTRIBUTE_DATA_NONE        0x00000000
#define CSSM_ATTRIBUTE_DATA_UINT32      0x10000000
#define CSSM_ATTRIBUTE_DATA_CSSM_DATA   0x20000000
#define CSSM_ATTRIBUTE_DATA_CRYPTO_DATA 0x30000000
#define CSSM_ATTRIBUTE_DATA_KEY         0x40000000
#define CSSM_ATTRIBUTE_DATA_STRING      0x50000000
#define CSSM_ATTRIBUTE_DATA_DATE        0x60000000
#define CSSM_ATTRIBUTE_DATA_RANGE       0x70000000
#define CSSM_ATTRIBUTE_DATA_VERSION     0x01000000

#define CSSM_ATTRIBUTE_TYPE_MASK        0xFF000000

typedef enum cssm_attribute_type {
    CSSM_ATTRIBUTE_NONE         = 0,
    CSSM_ATTRIBUTE_CUSTOM       = (CSSM_ATTRIBUTE_DATA_CSSM_DATA | 1),
    CSSM_ATTRIBUTE_DESCRIPTION  = (CSSM_ATTRIBUTE_DATA_STRING | 2),
    CSSM_ATTRIBUTE_KEY          = (CSSM_ATTRIBUTE_DATA_KEY | 3),
    CSSM_ATTRIBUTE_INIT_VECTOR  = (CSSM_ATTRIBUTE_DATA_CSSM_DATA | 4),
    CSSM_ATTRIBUTE_SALT         = (CSSM_ATTRIBUTE_DATA_CSSM_DATA | 5),
    CSSM_ATTRIBUTE_PADDING      = (CSSM_ATTRIBUTE_DATA_UINT32 | 6),
    CSSM_ATTRIBUTE_RANDOM       = (CSSM_ATTRIBUTE_DATA_CSSM_DATA | 7),
    CSSM_ATTRIBUTE_SEED         = (CSSM_ATTRIBUTE_DATA_CRYPTO_DATA | 8),
    CSSM_ATTRIBUTE_PASSPHRASE   = (CSSM_ATTRIBUTE_DATA_CRYPTO_DATA | 9),
    CSSM_ATTRIBUTE_KEY_LENGTH   = (CSSM_ATTRIBUTE_DATA_UINT32 | 10),
    CSSM_ATTRIBUTE_KEY_LENGTH_RANGE = (CSSM_ATTRIBUTE_DATA_RANGE | 11),
    CSSM_ATTRIBUTE_BLOCK_SIZE   = (CSSM_ATTRIBUTE_DATA_UINT32 | 12),
    CSSM_ATTRIBUTE_OUTPUT_SIZE  = (CSSM_ATTRIBUTE_DATA_UINT32 | 13),
    CSSM_ATTRIBUTE_ROUNDS       = (CSSM_ATTRIBUTE_DATA_UINT32 | 14),
    CSSM_ATTRIBUTE_IV_SIZE        = (CSSM_ATTRIBUTE_DATA_UINT32 | 15),
    CSSM_ATTRIBUTE_ALG_PARAMS    = (CSSM_ATTRIBUTE_DATA_CSSM_DATA | 16),
    CSSM_ATTRIBUTE_LABEL        = (CSSM_ATTRIBUTE_DATA_CSSM_DATA | 17),
    CSSM_ATTRIBUTE_KEY_TYPE        = (CSSM_ATTRIBUTE_DATA_UINT32 | 18),
    CSSM_ATTRIBUTE_MODE        = (CSSM_ATTRIBUTE_DATA_UINT32 | 19),
    CSSM_ATTRIBUTE_EFFECTIVE_BITS = (CSSM_ATTRIBUTE_DATA_UINT32 | 20),
    CSSM_ATTRIBUTE_START_DATE     = (CSSM_ATTRIBUTE_DATA_DATE | 21),
    CSSM_ATTRIBUTE_END_DATE       = (CSSM_ATTRIBUTE_DATA_DATE | 22),
    CSSM_ATTRIBUTE_KEYUSAGE     = (CSSM_ATTRIBUTE_DATA_UINT32 | 23),
    CSSM_ATTRIBUTE_KEYATTR      = (CSSM_ATTRIBUTE_DATA_UINT32 | 24),
    CSSM_ATTRIBUTE_VERSION      = (CSSM_ATTRIBUTE_DATA_VERSION | 25),
    CSSM_ATTRIBUTE_PRIME        = (CSSM_ATTRIBUTE_DATA_CSSM_DATA | 26),
    CSSM_ATTRIBUTE_BASE         = (CSSM_ATTRIBUTE_DATA_CSSM_DATA | 27),
    CSSM_ATTRIBUTE_SUBPRIME     = (CSSM_ATTRIBUTE_DATA_CSSM_DATA | 28),
    CSSM_ATTRIBUTE_ALG_ID       = (CSSM_ATTRIBUTE_DATA_UINT32 | 29),
    CSSM_ATTRIBUTE_ITERATION_COUNT = (CSSM_ATTRIBUTE_DATA_UINT32 | 30),
    CSSM_ATTRIBUTE_ROUNDS_RANGE     = (CSSM_ATTRIBUTE_DATA_RANGE | 31),
    CSSM_ATTRIBUTE_CSP_HANDLE        = (CSSM_ATTRIBUTE_DATA_UINT32 | 34)
} CSSM_ATTRIBUTE_TYPE;

typedef enum cssm_padding { 
    CSSM_PADDING_NONE           = 0,
    CSSM_PADDING_CUSTOM         = CSSM_PADDING_NONE+1,
    CSSM_PADDING_ZERO           = CSSM_PADDING_NONE+2,
    CSSM_PADDING_ONE            = CSSM_PADDING_NONE+3,
    CSSM_PADDING_ALTERNATE      = CSSM_PADDING_NONE+4,
    CSSM_PADDING_FF             = CSSM_PADDING_NONE+5,
    CSSM_PADDING_PKCS5          = CSSM_PADDING_NONE+6,
    CSSM_PADDING_PKCS7          = CSSM_PADDING_NONE+7,
    CSSM_PADDING_CipherStealing = CSSM_PADDING_NONE+8,
    CSSM_PADDING_RANDOM         = CSSM_PADDING_NONE+9
} CSSM_PADDING;


/*
    CSSM_KEY & CSSM_KEYHEADER Data Structures  
*/

/*
    Blob Type
*/

#define CSSM_KEYBLOB_RAW                0            /* The blob is a clear, raw key */
#define CSSM_KEYBLOB_RAW_BERDER            1            /* The blob is a clear key, DER encoded */
#define CSSM_KEYBLOB_REFERENCE            2            /* The blob is a reference to a key */
#define CSSM_KEYBLOB_WRAPPED            3            /* The blob is a wrapped RAW key */
#define CSSM_KEYBLOB_WRAPPED_BERDER        4            /* The blob is a wrapped DER encoded key */
#define CSSM_KEYBLOB_OTHER                0xFFFFFFFF  /* The blob is a wrapped DER encoded key */

/*
    Raw Format 
*/

#define CSSM_KEYBLOB_RAW_FORMAT_NONE      0            /* No further conversion need to be done */
#define CSSM_KEYBLOB_RAW_FORMAT_PKCS1     1            /* RSA PKCS1 V1.5 */
#define CSSM_KEYBLOB_RAW_FORMAT_PKCS3     2            /* RSA PKCS3 V1.5 */
#define CSSM_KEYBLOB_RAW_FORMAT_MSCAPI    3            /* Microsoft CAPI V2.0 */
#define CSSM_KEYBLOB_RAW_FORMAT_PGP       4            /* PGP V??? */
#define CSSM_KEYBLOB_RAW_FORMAT_FIPS186   5            /* US Gov. FIPS 186 - DSS V??? */
#define CSSM_KEYBLOB_RAW_FORMAT_BSAFE     6            /* RSA Bsafe V3.0 */
#define CSSM_KEYBLOB_RAW_FORMAT_PKCS11    7            /* RSA PKCS11 V2.0 */
#define CSSM_KEYBLOB_RAW_FORMAT_CDSA      8            /* CDSA */
#define CSSM_KEYBLOB_RAW_FORMAT_CCA       9            /* CCA clear public key blob */
#define CSSM_KEYBLOB_RAW_FORMAT_PKCS8     10		   /* RSA PKCS8 V1.2 */
#define CSSM_KEYBLOB_RAW_FORMAT_OTHER     0xFFFFFFFF   /* Other, CSP defined */

/*
    Wrapped Format 
*/
#define CSSM_KEYBLOB_WRAPPED_FORMAT_NONE  0				/* No further conversion need to be done */
#define CSSM_KEYBLOB_WRAPPED_FORMAT_PKCS8 1				/* RSA PKCS8 V1.2 */
#define CSSM_KEYBLOB_WRAPPED_FORMAT_OTHER 0xFFFFFFFF    /* Other, CSP defined */

/*
    Reference Format
*/

#define CSSM_KEYBLOB_REF_FORMAT_INTEGER     0            /* Reference is a number or handle */
#define CSSM_KEYBLOB_REF_FORMAT_STRING      1           /* Reference is a string or name */
#define CSSM_KEYBLOB_REF_FORMAT_CCA         2           /* Reference is a CCA key token */
#define CSSM_KEYBLOB_REF_FORMAT_OTHER       0xFFFFFFFF  /* Other, CSP defined */


/*
    Key Class
*/

#define CSSM_KEYCLASS_PUBLIC_KEY     0                    /* Key is public key */
#define CSSM_KEYCLASS_PRIVATE_KEY    1                    /* Key is private key */
#define CSSM_KEYCLASS_SESSION_KEY    2                    /* Key is session or symmetric key */
#define CSSM_KEYCLASS_SECRET_PART    3                    /* Key is part of secret key */ 
#define CSSM_KEYCLASS_OTHER          0xFFFFFFFF            /* Other */ 


/*
    Key Use Flags
*/

#define CSSM_KEYUSE_ANY                 0x80000000
#define CSSM_KEYUSE_ENCRYPT             0x00000001
#define CSSM_KEYUSE_DECRYPT             0x00000002
#define CSSM_KEYUSE_SIGN                0x00000004
#define CSSM_KEYUSE_VERIFY              0x00000008
#define CSSM_KEYUSE_SIGN_RECOVER        0x00000010
#define CSSM_KEYUSE_VERIFY_RECOVER      0x00000020
#define CSSM_KEYUSE_WRAP                0x00000040
#define CSSM_KEYUSE_UNWRAP              0x00000080
#define CSSM_KEYUSE_DERIVE              0x00000100



/*
    CSSM_DATE
*/

typedef struct cssm_date {
    uint8 Year[4];                  /* 1997-9999, year 10000 problem!!! */
                                    /* For 1997 - Year[0] = '1'; Year[1] = '9'; 
                                       Year[2] = '9'; Year[3] = '7'; */
    uint8 Month[2];                 /* 01-12 */
    uint8 Day[2];                   /* 01-31 */
} CSSM_DATE, *CSSM_DATE_PTR;

/*
    CSSM_DATE_AND_TIME
*/

typedef struct cssm_date_and_time {
    sint32 Century;               /* 00-256 */
    sint32 Year;                  /* 00-99 */
    sint32 Month;                 /* 01-12 */
    sint32 Day;                   /* 01-31 */
    sint32 Hour;                  /* 00-23 */
    sint32 Minute;                /* 00-59 */
    sint32 Second;                /* 00-59 */
} CSSM_DATE_AND_TIME, *CSSM_DATE_AND_TIME_PTR;


/*
    CSSM_RANGE
*/
typedef struct cssm_range {
    uint32 Min;          /* inclusive minimium value */
    uint32 Max;          /* inclusive maximium value */
} CSSM_RANGE, *CSSM_RANGE_PTR;

typedef uint32 CSSM_HEADERVERSION;
/*
    CSSM_KEYHEADER
*/

typedef struct cssm_keyheader {
    CSSM_HEADERVERSION HeaderVersion;   /* Key header version */
    CSSM_GUID CspId;                    /* GUID of CSP generating the key */
    uint32 BlobType;                    /* See BlobType #define's */
    uint32 Format;                      /* Raw or Reference format */
    uint32 AlgorithmId;                 /* Algoritm ID of key */
    uint32 KeyClass;                    /* Public/Private/Secret etc. */
    uint32 EffectiveKeySizeInBits;      /* Size of actual key/modulus/prime in bits */
    uint32 KeyAttr;                     /* Attribute flags */
    uint32 KeyUsage;                    /* Key use flags */
    CSSM_DATE StartDate;                /* Effective date of key */
    CSSM_DATE EndDate;                  /* Expiration date of key */
    uint32 WrapAlgorithmId;             /* == CSSM_ALGID_NONE if clear key */
    uint32 WrapMode;                    /* if alg supports multiple wrapping modes */
    uint32 Reserved;
} CSSM_KEYHEADER, *CSSM_KEYHEADER_PTR;


/*
    CSSM_KEY
*/

typedef struct cssm_key    {
    CSSM_KEYHEADER KeyHeader;       /* Key header which is of fixed length */
    CSSM_DATA KeyData;              /* Key data which is of variable length */
} CSSM_KEY, *CSSM_KEY_PTR;

typedef CSSM_KEY CSSM_WRAP_KEY, *CSSM_WRAP_KEY_PTR;

#define CSSM_KEYHEADER_VERSION    (2)

typedef struct cssm_key_size {
    uint32 KeySizeInBits;            /* Key size in bits */
    uint32 EffectiveKeySizeInBits;   /* Effective key size in bits */
} CSSM_KEY_SIZE, *CSSM_KEY_SIZE_PTR;

typedef struct cssm_query_size_data {
    uint32 SizeInputBlock;
    uint32 SizeOutputBlock;
} CSSM_QUERY_SIZE_DATA, *CSSM_QUERY_SIZE_DATA_PTR;

typedef struct cssm_context_attribute{
    uint32 AttributeType;    /* one of the defined CSSM_ATTRIBUTE_TYPEs */
    uint32 AttributeLength;  /* length of attribute */
    union cssm_context_attribute_value {
        char *String;
        uint32 Uint32;
        CSSM_CRYPTO_DATA_PTR Crypto;
        CSSM_KEY_PTR Key;
        CSSM_DATA_PTR Data;
        CSSM_DATE_PTR Date;
        CSSM_RANGE_PTR Range;
        CSSM_VERSION_PTR Version;
    } Attribute;            /* data that describes attribute */
} CSSM_CONTEXT_ATTRIBUTE, *CSSM_CONTEXT_ATTRIBUTE_PTR;

typedef struct cssm_context {
    uint32 ContextType;        /* one of the defined CSSM_CONTEXT_TYPEs */
    uint32 AlgorithmType;      /* one of the defined CSSM_ALGORITHMSs */
    uint32 Reserve;            /* reserved for future use */
    uint32 NumberOfAttributes; /* number of attributes associated with context */
    CSSM_CONTEXT_ATTRIBUTE_PTR ContextAttributes;    /* pointer to attributes */
	CSSM_CSP_HANDLE CSPHandle;	/* TODO: Missing in API doc */    
} CSSM_CONTEXT, *CSSM_CONTEXT_PTR;

/* --- Key Recovery Policy API Flags --- */
#define KR_INDIV            0x01
#define KR_ENT              0x02
#define KR_LE               KR_LE_MAN | KR_LE_USE    
#define KR_LE_MAN           0x08
#define KR_LE_USE           0x10
#define KR_ALL              0x20
#define KR_OPTIMIZE         0x40
#define KR_DROP_WORKFACTOR  0x80


typedef CSSM_CONTEXT CSSM_CONTEXT_INFO;

/*
    CSSM_CSP_CAPABILITY
*/

typedef CSSM_CONTEXT CSSM_CSP_CAPABILITY, *CSSM_CSP_CAPABILITY_PTR;


/*
    CspType
*/
typedef enum cssm_csptype {
	CSSM_CSP_SOFTWARE	= 1, 
	CSSM_CSP_HARDWARE	= CSSM_CSP_SOFTWARE+1,
	CSSM_CSP_HYBRID = CSSM_CSP_SOFTWARE+2,
}CSSM_CSPTYPE; 

/*
    Software Csp SubService Info
*/

typedef struct cssm_softwarecspsubserviceinfo {
    uint32 NumberOfCapabilities;                /* Number of capabilities in list */
    CSSM_CSP_CAPABILITY_PTR CapabilityList;     /* List of capabilitites */
    void* Reserved;                             /* Reserved field */
} CSSM_SOFTWARE_CSPSUBSERVICE_INFO, *CSSM_SOFTWARE_CSPSUBSERVICE_INFO_PTR;


/*
    Hardware Csp SubService Info
*/

/* ReaderFlags */

#define CSSM_CSP_RDR_TOKENPRESENT  0x00000001    /* Token is present in reader/slot */
#define CSSM_CSP_RDR_EXISTS        0x00000002    /* Device is a reader with removable token */
#define CSSM_CSP_RDR_HW            0x00000004    /* Slot is a hardware slot */


/* TokenFlags */

#define CSSM_CSP_TOK_RNG                    0x00000001    /* Token has random number generator */ 
#define CSSM_CSP_TOK_WRITE_PROTECTED        0x00000002    /* Token is write protected */
#define CSSM_CSP_TOK_LOGIN_REQUIRED         0x00000004    /* User must login to access private obj */
#define CSSM_CSP_TOK_USER_PIN_INITIALIZED   0x00000008    /* User's PIN has been initialized */
#define CSSM_CSP_TOK_EXCLUSIVE_SESSION      0x00000010    /* An exclusive session currently exists */
#define CSSM_CSP_TOK_CLOCK_EXISTS           0x00000040    /* Token has built in clock */
#define CSSM_CSP_TOK_ASYNC_SESSION          0x00000080    /* Token supports asynchronous operations */
#define CSSM_CSP_TOK_PROT_AUTHENTICATION    0x00000100    /* Token has protected authentication path */
#define CSSM_CSP_TOK_DUAL_CRYPTO_OPS        0x00000200    /* Token supports dual crypto ops */
 

typedef struct cssm_hardwarecspsubserviceinfo {
    uint32 NumberOfCapabilities;                       /* Number of capabilities in list */
    CSSM_CSP_CAPABILITY_PTR CapabilityList;            /* List of capabilitites */
    void* Reserved;                                    /* Reserved field */

    /* Reader/Slot Info */
    CSSM_STRING ReaderDescription;
    CSSM_STRING ReaderVendor;
    CSSM_STRING ReaderSerialNumber;
    CSSM_VERSION ReaderHardwareVersion;
    CSSM_VERSION ReaderFirmwareVersion;
    uint32 ReaderFlags;                                /* See ReaderFlags #define's */
    uint32 ReaderCustomFlags;

    /* 
        Token Info, may not be available if reader supports removable device 
        AND device is not present.
    */
    CSSM_STRING TokenDescription;   
    CSSM_STRING TokenVendor;            
    CSSM_STRING TokenSerialNumber;    
    CSSM_VERSION TokenHardwareVersion;    
    CSSM_VERSION TokenFirmwareVersion;    
    
    uint32 TokenFlags;                                /* See TokenFlags #defines's */
    uint32 TokenCustomFlags;
    uint32 TokenMaxSessionCount;
    uint32 TokenOpenedSessionCount;
    uint32 TokenMaxRWSessionCount;
    uint32 TokenOpenedRWSessionCount;
    uint32 TokenTotalPublicMem;
    uint32 TokenFreePublicMem;
    uint32 TokenTotalPrivateMem;
    uint32 TokenFreePrivateMem;
    uint32 TokenMaxPinLen;
    uint32 TokenMinPinLen;
    char TokenUTCTime[16];

    /*
        User Info, may not be available if reader supports removable device 
        AND device is not present.
    */
    CSSM_STRING UserLabel;
    CSSM_DATA UserCACertificate;                    /* Certificate of CA */ 

} CSSM_HARDWARE_CSPSUBSERVICE_INFO, *CSSM_HARDWARE_CSPSUBSERVICE_INFO_PTR;

typedef CSSM_HARDWARE_CSPSUBSERVICE_INFO
				CSSM_HYBRID_CSPSUBSERVICE_INFO, * CSSM_HYBRID_CSPSUBSERVICE_INFO_PTR;


/*
    CSSM_CSP_WRAPPEDPRODUCTINFO
*/

/* (Wrapped)ProductFlags */
/* None curently defined */

typedef struct cssm_csp_wrappedproductinfo {
    CSSM_VERSION StandardVersion;                           /* Version of standard this product conforms to */
    CSSM_STRING StandardDescription;  /* Description of standard this product conforms to */
    CSSM_VERSION ProductVersion;                            /* Version of wrapped product/library */
    CSSM_STRING ProductDescription;   /* Description of wrapped product/library */
    CSSM_STRING ProductVendor;        /* Vendor of wrapped product/library */
    uint32 ProductFlags;              /* ProductFlags */
	uint32 ProductCustomFlags;
} CSSM_CSP_WRAPPEDPRODUCTINFO, *CSSM_CSP_WRAPPEDPRODUCTINFO_PTR;

/*
    CSSM_CSPINFO
*/

/* CspFlags */
typedef uint32 CSSM_CSP_FLAGS;
#define CSSM_CSP_STORES_PRIVATE_KEYS    0x00000001    /* CSP can store private keys */
#define CSSM_CSP_STORES_PUBLIC_KEYS     0x00000002    /* CSP can store public keys */
#define CSSM_CSP_STORES_SESSION_KEYS    0x00000004    /* CSP can store session/secret keys */

typedef struct cssm_cspsubservice {
    uint32 SubServiceId;
    CSSM_STRING Description;
    CSSM_CSP_FLAGS CspFlags;       /* General flags defined by CSSM for CSPs */   
    uint32 CspCustomFlags;         /* Flags defined by individual CSP */  
    uint32 AccessFlags;            /* Access Flags used by CSP */
    CSSM_CSPTYPE CspType;          /* CSP type number for dereferencing CspInfo*/
    union cssm_subservice_info {   /* info struct of type defined by CspType */
        CSSM_SOFTWARE_CSPSUBSERVICE_INFO SoftwareCspSubService;
        CSSM_HARDWARE_CSPSUBSERVICE_INFO HardwareCspSubService;
		CSSM_HYBRID_CSPSUBSERVICE_INFO HybridCspSubService;
    } SubServiceInfo;
    CSSM_CSP_WRAPPEDPRODUCTINFO WrappedProduct;    /* Pointer to wrapped product info */
} CSSM_CSPSUBSERVICE, *CSSM_CSPSUBSERVICE_PTR;

#define CSSM_VALUE_NOT_AVAILABLE    (0xFFFFFFFF)

/* 
    Key Attribute Flags
*/
/* Valid only during call to an API. Will never be valid when set in a key header */
#define CSSM_KEYATTR_RETURN_DEFAULT   0x00000000
#define CSSM_KEYATTR_RETURN_DATA      0x10000000
#define CSSM_KEYATTR_RETURN_REF       0x20000000
#define CSSM_KEYATTR_RETURN_NONE      0x40000000

/* Valid during an API call and in a key header */
#define CSSM_KEYATTR_PERMANENT        0x00000001
#define CSSM_KEYATTR_PRIVATE          0x00000002
#define CSSM_KEYATTR_MODIFIABLE       0x00000004
#define CSSM_KEYATTR_SENSITIVE        0x00000008
#define CSSM_KEYATTR_EXTRACTABLE      0x00000020

/* Valid only in a key header generated by a CSP, not valid during an API call */
#define CSSM_KEYATTR_ALWAYS_SENSITIVE 0x00000010
#define CSSM_KEYATTR_NEVER_EXTRACTABLE 0x00000040

/* The effects of specifying the EXTRACTABLE & SENSATIVE bits in an API call
 * is summarized in this table.
 * SENSATIVE  EXTRACTABLE    Effect
 * ---------- ----------------- --------------------------------------------
 * FALSE      TRUE                Key extractable wrapped or plaintext
 * TRUE       TRUE                Key extractable only when wrapped
 *                                *This mode is an error condition for Cryptoki v1.0
 * TRUE/FALSE FALSE                Key NEVER extractable in any form
 *                                *Sensative forced to TRUE for Cryptoki v1.0
 */

#define CSSM_ESTIMATED_TIME_UNKNOWN	-1

typedef struct cssm_field {
    CSSM_OID FieldOid;
    CSSM_DATA FieldValue;
} CSSM_FIELD, *CSSM_FIELD_PTR;

/*
 * This data structure contains parameters useful in verifying certificate groups,
 * certificate revocation lists and other forms of signed document.
 */
typedef struct cssm_verify_context {
	CSSM_FIELD_PTR  PolicyIdentifiers;
	uint32 NumberofPolicyIdentifiers;
	CSSM_TP_STOP_ON VerificationAbortOn; 
	CSSM_USER_AUTHENTICATION_PTR UserAuthentication;
	CSSM_DATA_PTR AnchorCerts;
	uint32 NumberofAnchorCerts;
	CSSM_FIELD_PTR VerifyScope;
	uint32 ScopeSize;
	CSSM_TP_ACTION Action;
	CSSM_NOTIFY_CALLBACK CallbackWithVerifiedCert;
	CSSM_DATA_PTR ActionData;
	CSSM_EVIDENCE_PTR *Evidence;
	uint32 *NumberOfEvidences;
} CSSM_VERIFYCONTEXT, *CSSM_VERIFYCONTEXT_PTR; 


typedef struct cssm_tp_wrappedproductinfo {
    CSSM_VERSION StandardVersion;                           /* Version of standard this product conforms to */
    CSSM_STRING StandardDescription;  /* Description of standard this product conforms to */
    CSSM_STRING ProductVendor;        /* Vendor of wrapped product/library */
    uint32 ProductFlags;                                    /* ProductFlags */
} CSSM_TP_WRAPPEDPRODUCTINFO, *CSSM_TP_WRAPPEDPRODUCTINFO_PTR;

typedef struct cssm_tpsubservice {
    uint32 SubServiceId;
    CSSM_STRING Description; /* Description of this sub service */
    CSSM_CERT_TYPE CertType;       /* Type of certificate accepted by the TP */
	CSSM_CERT_ENCODING CertEncoding; 	/* Encoding of cert accepted by TP */
    CSSM_USER_AUTHENTICATION_MECHANISM AuthenticationMechanism; 
    uint32 NumberOfPolicyIdentifiers;    
    CSSM_FIELD_PTR PolicyIdentifiers;
    CSSM_TP_WRAPPEDPRODUCTINFO WrappedProduct;  /* Pointer to wrapped product info */
} CSSM_TPSUBSERVICE, *CSSM_TPSUBSERVICE_PTR;

/*
 * Structure to describe the attributes of the CSSM infrastructure.
 */
typedef struct cssm_cssminfo {
    CSSM_VERSION Version; 
    CSSM_STRING Description;	/* Description of CSSM */ 
    CSSM_STRING Vendor;		/* Vendor of CSSM */
    CSSM_BOOL ThreadSafe;
    char Location[CSSM_MAX_PATH];
    CSSM_GUID CssmGUID;
	CSSM_GUID InterfaceGUID;	/* opt GUID defining supported interface */
}CSSM_CSSMINFO, *CSSM_CSSMINFO_PTR;


/*
 * This enumerated type lists the signed certificate aggregates that are 
 * considered to be certificate bundles.
 */
typedef enum cssm_cert_bundle_type {
	CSSM_CERT_BUNDLE_UNKNOWN =  0x00,
	CSSM_CERT_BUNDLE_CUSTOM  =  0x01,
	CSSM_CERT_BUNDLE_PKCS7_SIGNED_DATA =  0x02,
	CSSM_CERT_BUNDLE_PKCS7_SIGNED_ENVELOPED_DATA =  0x03,
	CSSM_CERT_BUNDLE_PKCS12 =  0x04,
	CSSM_CERT_BUNDLE_PFX =  0x05,
	CSSM_CERT_BUNDLE_LAST = 0x7FFF
} CSSM_CERT_BUNDLE_TYPE;

/* 
 * Applications wishing to define their own custom certificate 
 * BUNDLE type should create a random uint32 whose value
 * is greater than the CSSM_CL_CUSTOM_CERT_BUNDLE_TYPE 
 */
#define CSSM_CL_CUSTOM_CERT_BUNDLE_TYPE  0x8000 

/*
 * This enumerated type lists the encoding methods applied to the signed 
 * certificate aggregates that are considered to be certificate bundles.
 */

typedef enum cssm_cert_bundle_encoding {
    CSSM_CERT_BUNDLE_ENCODING_UNKNOWN =  0x00,
	CSSM_CERT_BUNDLE_ENCODING_CUSTOM  =  0x01,
	CSSM_CERT_BUNDLE_ENCODING_BER     =  0x02,
	CSSM_CERT_BUNDLE_ENCODING_DER     =  0x03
} CSSM_CERT_BUNDLE_ENCODING;

/*
 * This structure defines a bundle header, which describes the type and encoding
 * of a certificate bundle.
 */
typedef struct cssm_cert_bundle_header {
	CSSM_CERT_BUNDLE_TYPE BundleType;
	CSSM_CERT_BUNDLE_ENCODING BundleEncoding;
} CSSM_CERT_BUNDLE_HEADER, *CSSM_CERT_BUNDLE_HEADER_PTR;

/*
 * This structure defines a certificate bundle, which consists of a descriptive 
 * header and a pointer to the opaque bundle. The bundle itself is a signed opaque 
 * aggregate of certificates.
 */
 typedef struct cssm_cert_bundle {
	CSSM_CERT_BUNDLE_HEADER BundleHeader;
	CSSM_DATA Bundle;
} CSSM_CERT_BUNDLE, *CSSM_CERT_BUNDLE_PTR;


typedef uint32 CSSM_CA_SERVICES;
/*  bit masks for additional CA services at cert enroll  */
#define    CSSM_CA_KEY_ARCHIVE        0x0001
#define    CSSM_CA_CERT_PUBLISH        0x0002
#define    CSSM_CA_CERT_NOTIFY_RENEW    0x0004
#define	CSSM_CA_CERT_DIR_UPDATE	0x0008	/* multi-signed cert to dir svc */
#define	CSSM_CA_CRL_DISTRIBUTE	0x0010 /* push CRL to everyone */ 

/*
    CSSM_CL_WRAPPEDPRODUCTINFO
*/

/* CL_CA_ProductInfo */
typedef struct cssm_cl_ca_cert_classinfo {
    CSSM_STRING CertClassName;        /* Name of a cert class issued by this CA */
    CSSM_DATA CACert;                 /* CA Certificate for this cert class */
} CSSM_CL_CA_CERT_CLASSINFO, *CSSM_CL_CA_CERT_CLASSINFO_PTR;

typedef struct cssm_cl_ca_productinfo {
    CSSM_VERSION StandardVersion;     /* Version of standard this product conforms to */
    CSSM_STRING StandardDescription;  /* Description of standard this product conforms to */
    CSSM_VERSION ProductVersion;      /* Version of wrapped product/library */
    CSSM_STRING ProductDescription;   /* Description of wrapped product/library */
    CSSM_STRING ProductVendor;        /* Vendor of wrapped product/library */
	CSSM_NET_PROTOCOL NetworkProtocol;/* The network protocol supported by the CA service */

    CSSM_CERT_TYPE CertType;		  /* Type of certs and CRLs supported by the CA */
    CSSM_CERT_ENCODING CertEncoding; /* Encoding of certs supported by CA */
    CSSM_CRL_TYPE CrlType; /* Type of CRLs supported by CA */
    CSSM_CRL_ENCODING CrlEncoding; /* Encoding of CRLs supported by CA */

    CSSM_CA_SERVICES AdditionalServiceFlags;/* Mask of additional services a caller can request */
    uint32 NumberOfCertClasses;             /* Number of different cert types or classes the CA can issue */
    CSSM_CL_CA_CERT_CLASSINFO_PTR CertClasses;                                  
} CSSM_CL_CA_PRODUCTINFO, *CSSM_CL_CA_PRODUCTINFO_PTR;

/* CL_Encoder_ProductInfo */
typedef struct cssm_cl_encoder_productinfo {
    CSSM_VERSION StandardVersion;       /* Version of standard this product conforms to */
    CSSM_STRING StandardDescription;  /* Description of standard this product conforms to */
    CSSM_VERSION ProductVersion;        /* Version of wrapped product/library */
    CSSM_STRING ProductDescription; /* Description of wrapped product/library */
    CSSM_STRING ProductVendor;    /* Vendor of wrapped product/library */       
    CSSM_CERT_TYPE CertType;            /* Type of certs and CRLs supported by the CA */
	CSSM_CRL_TYPE CrlType; /* Type of crls supported by encoder */
    uint32 ProductFlags;                /* Mask of selectable encoder features actually used by the CL */
} CSSM_CL_ENCODER_PRODUCTINFO, *CSSM_CL_ENCODER_PRODUCTINFO_PTR;

typedef struct cssm_cl_wrappedproductinfo {
    /* List of encode/decode/parse libraries embedded in the CL module */
    CSSM_CL_ENCODER_PRODUCTINFO_PTR EmbeddedEncoderProducts;    /* library product description */
    uint32 NumberOfEncoderProducts;     /* number of encode/decode/parse libraries used in CL */

    /* List of CAs accessible to the CL module */
    CSSM_CL_CA_PRODUCTINFO_PTR AccessibleCAProducts;            /* CA product description*/
    uint32 NumberOfCAProducts;    /* Number of accessible CAs */
} CSSM_CL_WRAPPEDPRODUCTINFO, *CSSM_CL_WRAPPEDPRODUCTINFO_PTR;


typedef struct cssm_clsubservice {
	uint32 SubServiceId;
	CSSM_STRING Description;  
	CSSM_CERT_TYPE CertType;
	CSSM_CERT_ENCODING CertEncoding;
	uint32 NumberOfBundleInfos; 
	CSSM_CERT_BUNDLE_HEADER_PTR BundleInfo;   /* first is default value */
	CSSM_USER_AUTHENTICATION_MECHANISM AuthenticationMechanism;
	uint32 NumberOfTemplateFields;
	CSSM_OID_PTR CertTemplate;
	uint32 NumberOfTranslationTypes;
	CSSM_CERT_TYPE_PTR CertTranslationTypes;
	CSSM_CL_WRAPPEDPRODUCTINFO WrappedProduct;
} CSSM_CLSUBSERVICE, *CSSM_CLSUBSERVICE_PTR;


#define CSSM_NOTIFY_SURRENDER           0
#define CSSM_NOTIFY_COMPLETE            1
#define CSSM_NOTIFY_DEVICE_REMOVED      2
#define CSSM_NOTIFY_DEVICE_INSERTED     3


#define CSSM_CSP_SESSION_EXCLUSIVE        0x0001 
#define CSSM_CSP_SESSION_READWRITE        0x0002
#define CSSM_CSP_SESSION_SERIAL           0x0004


#define CSSM_DL_STORE_ACCESS_SERIAL       CSSM_CSP_SESSION_SERIAL
#define CSSM_DL_STORE_ACCESS_EXCLUSIVE    CSSM_CSP_SESSION_EXCLUSIVE
#define CSSM_DL_STROE_ACCESS_READWRITE    CSSM_CSP_SESSION_READWRITE


/*
 * VL Data types 
 */
typedef CSSM_GUID CSSM_VOBUNDLE_UID, *CSSM_VOBUNDLE_UID_PTR;
typedef CSSM_GUID CSSM_VO_UID, *CSSM_VO_UID_PTR;
typedef uint32    CSSM_VL_VERIFICATION_HANDLE, *CSSM_VL_VERIFICATION_HANDLE_PTR;

typedef struct cssm_vo_uid_binding {
	CSSM_VO_UID VoIdentifier;
	CSSM_VO_HANDLE VoHandle;
} CSSM_VO_UID_BINDING, *CSSM_VO_UID_BINDING_PTR;

typedef struct cssm_vo_uid_binding_group {
	uint32 NumberOfBindings;
	CSSM_VO_UID_BINDING_PTR Bindings;
} CSSM_VO_UID_BINDING_GROUP, *CSSM_VO_UID_BINDING_GROUP_PTR;

/* VL Locations */
typedef enum cssm_vl_media_type {
    CSSM_VL_MEDIA_TYPE_UNKNOWN = 0, 
    CSSM_VL_MEDIA_TYPE_CUSTOM = 1, 
    CSSM_VL_MEDIA_TYPE_VODIRECTORY = 2,
    CSSM_VL_MEDIA_TYPE_DLM = 3,
    CSSM_VL_MEDIA_TYPE_FILE = 4,
    CSSM_VL_MEDIA_TYPE_REMOTE = 5,
    CSSM_VL_MEDIA_TYPE_MEMORY = 6,
	CSSM_VL_MEDIA_TYPE_EMBEDDED = 7
} CSSM_VL_MEDIA_TYPE;

#define CSSM_VL_CUSTOM_MEDIA_ACCESS_SIZE_ARBITRARY 0
#define CSSM_VL_CUSTOM_MEDIA_ACCESS_GET 0x01
#define CSSM_VL_CUSTOM_MEDIA_ACCESS_PUT 0x02

typedef struct cssm_vl_location_custom  {
	CSSM_STRING ByteSourceName;
	uint32 ByteBlockSize;
	CSSM_HANDLE (*OpenMedia) (CSSM_STRING ByteSourceName, uint32 AccessMode);
	uint32      (*GetBytes)  (CSSM_HANDLE MediaHandle, CSSM_DATA_PTR ByteBuffer);
	uint32      (*PutBytes)  (CSSM_HANDLE MediaHandle, CSSM_DATA_PTR ByteBuffer);	
    CSSM_RETURN (*CloseMedia)(CSSM_HANDLE MediaHandle);
} CSSM_VL_LOCATION_CUSTOM, *CSSM_VL_LOCATION_CUSTOM_PTR;

typedef CSSM_VOBUNDLE_UID CSSM_VL_LOCATION_VODIRECTORY, *CSSM_VL_LOCATION_VODIRECTORY_PTR;

typedef struct cssm_vl_location_dlm {
    CSSM_SUBSERVICE_UID DLSubserviceUID;
    CSSM_STRING DbName;
    CSSM_DB_ATTRIBUTE_DATA DbPrimaryKeyValue;
} CSSM_VL_LOCATION_DLM, *CSSM_VL_LOCATION_DLM_PTR;

typedef struct cssm_vl_location_file {
    char *PathName;
    char *FileName;
} CSSM_VL_LOCATION_FILE, *CSSM_VL_LOCATION_FILE_PTR;

/*
 * This enumerated type defines representations for specifying the location 
 * of a service.
 */
typedef enum cssm_net_address_type {
	CSSM_ADDR_NONE = 0,
	CSSM_ADDR_CUSTOM = 1,
	CSSM_ADDR_URL = 2, /* char* */
	CSSM_ADDR_SOCKADDR = 3,
	CSSM_ADDR_NAME = 4 /* char* - qualified by access method */
} CSSM_NET_ADDRESS_TYPE;

/*
 * This structure holds the address of a service. Typically the service is remote, 
 * but the value of the address field may resolve to the local system. The 
 * AddressType field defines how the Address field should be interpreted.
 */
typedef struct cssm_net_address {
    CSSM_NET_ADDRESS_TYPE AddressType;
    CSSM_DATA Address;
} CSSM_NET_ADDRESS, *CSSM_NET_ADDRESS_PTR;

typedef struct cssm_vl_location_remote {
    CSSM_NET_ADDRESS	 RemoteAddress;  
    void		*RemoteRequestParameters;
} CSSM_VL_LOCATION_REMOTE, *CSSM_VL_LOCATION_REMOTE_PTR;

typedef CSSM_DATA CSSM_VL_LOCATION_MEMORY, *CSSM_VL_LOCATION_MEMORY_PTR;

typedef CSSM_DATA CSSM_VL_LOCATION_EMBEDDED, *CSSM_VL_LOCATION_EMBEDDED_PTR;

typedef struct cssm_vl_location {
    CSSM_VL_MEDIA_TYPE MediaType;
    union {
		CSSM_VL_LOCATION_CUSTOM CustomRef;
        CSSM_VL_LOCATION_VODIRECTORY VORef;
		CSSM_VL_LOCATION_DLM DLMRef; 
		CSSM_VL_LOCATION_FILE FileRef;
		CSSM_VL_LOCATION_REMOTE RemoteRef;
		CSSM_VL_LOCATION_MEMORY MemoryRef;
		CSSM_VL_LOCATION_EMBEDDED EmbeddedRef;
    } Location;
} CSSM_VL_LOCATION, *CSSM_VL_LOCATION_PTR;


/* DO definitions */
typedef enum cssm_vl_do_type {
	CSSM_VL_DO_TYPE_UNKNOWN_FORMAT,
	CSSM_VL_DO_TYPE_CUSTOM_FORMAT,
	CSSM_VL_DO_TYPE_VO_UNKNOWN_FORMAT,
	CSSM_VL_DO_TYPE_VO_SIGNED_MANIFEST,
	CSSM_VL_DO_TYPE_VO_PKCS7_DER,
	CSSM_VL_DO_TYPE_VO_PKCS7v1_2_DER,
	CSSM_VL_DO_TYPE_VO_PKCS12,
	CSSM_VL_DO_TYPE_VO_MSFT_ACODEv1_0,
  	CSSM_VL_DO_TYPE_VO_MSFT_ACODEv1_2, 
	CSSM_VL_DO_TYPE_EXECUTABLE_UNKNOWN_FORMAT,
	CSSM_VL_DO_TYPE_EXECUTABLE_COFF,
	CSSM_VL_DO_TYPE_EXECUTABLE_ELF,
	CSSM_VL_DO_TYPE_EXECUTABLE_REX,
	CSSM_VL_DO_TYPE_EXECUTABLE_PE,
	CSSM_VL_DO_TYPE_BLOB_BYTE_DATA_UNKNOWN_FORMAT,
	CSSM_VL_DO_TYPE_BLOB_BYTE_DATA_ASCII,
	CSSM_VL_DO_TYPE_BLOB_BYTE_DATA_UNICODE, 
	CSSM_VL_DO_TYPE_BLOB_WORD_LITTLE_ENDIAN,
	CSSM_VL_DO_TYPE_BLOB_WORD_BIG_ENDIAN
} CSSM_VL_DO_TYPE;

typedef CSSM_DB_ATTRIBUTE_DATA CSSM_VL_ATTRIBUTE, *CSSM_VL_ATTRIBUTE_PTR;

typedef struct cssm_vl_attribute_group {
	uint32 NumberOfAttributes;
	CSSM_VL_ATTRIBUTE_PTR Attributes;
} CSSM_VL_ATTRIBUTE_GROUP, *CSSM_VL_ATTRIBUTE_GROUP_PTR;

typedef struct cssm_vl_do_uname {
	CSSM_STRING JoinName;
	CSSM_VO_HANDLE VoHandle;
} CSSM_VL_DO_UNAME, *CSSM_VL_DO_UNAME_PTR;

typedef struct cssm_vl_do_info {
	CSSM_STRING JoinName;
	CSSM_VO_HANDLE VoHandle;
	CSSM_VL_DO_TYPE DoType;
	CSSM_VL_ATTRIBUTE_GROUP BundleWideAttributes;
	CSSM_VL_ATTRIBUTE_GROUP VoSpecificAttributes;
} CSSM_VL_DO_INFO, *CSSM_VL_DO_INFO_PTR;

typedef struct cssm_vl_do_lmap_entry {
    CSSM_VOBUNDLE_UID VoBundleIdentifier;
    CSSM_STRING JoinName;
    CSSM_VL_LOCATION MapEntry;
} CSSM_VL_DO_LMAP_ENTRY, *CSSM_VL_DO_LMAP_ENTRY_PTR;

typedef struct cssm_vl_do_lmap {
    uint32 NumberOfMapEntries;
    CSSM_VL_DO_LMAP_ENTRY_PTR MapEntries;
} CSSM_VL_DO_LMAP, *CSSM_VL_DO_LMAP_PTR;


/* Signature definitions */
typedef struct cssm_vl_signature_info {
    CSSM_ALGORITHMS SigningAlgorithm;
    CSSM_DATA Signature;
    CSSM_CERTGROUP_PTR SignerCertGroup;  
	CSSM_VL_ATTRIBUTE_GROUP SignedAttributes;
    CSSM_VL_ATTRIBUTE_GROUP UnsignedAttributes;
} CSSM_VL_SIGNATURE_INFO, *CSSM_VL_SIGNATURE_INFO_PTR;


/* VO definitions */
typedef enum cssm_vo_type {
	CSSM_VL_VO_TYPE_PKCS7_DER,
	CSSM_VL_VO_TYPE_PKCS7v1_2_DER,
	CSSM_VL_VO_TYPE_PKCS12
} CSSM_VL_VO_TYPE;

typedef struct cssm_vl_vo_registry_info {
    CSSM_VO_UID Id;
    CSSM_STRING Name;
 	CSSM_VL_VO_TYPE Type;
    uint32 NumberOfVoInfos;
    struct cssm_vl_vo_registry_info *VoInfos;
} CSSM_VL_VO_REGISTRY_INFO, *CSSM_VL_VO_REGISTRY_INFO_PTR;


/* VOBundle definitions */
typedef enum cssm_vl_vobundle_type {
	CSSM_VOBUNDLE_TYPE_SIGNED_MANIFEST_CUSTOM,
	CSSM_VOBUNDLE_TYPE_PKCS7_DER,
	CSSM_VOBUNDLE_TYPE_PKCS7v1_2_DER,
	CSSM_VOBUNDLE_TYPE_PKCS12
} CSSM_VL_VOBUNDLE_TYPE;

typedef struct cssm_vl_vobundle_registry_info {
	CSSM_VOBUNDLE_UID Id;
	CSSM_STRING Name;
	CSSM_VL_VOBUNDLE_TYPE Type;
	uint32 NumberOfVoInfos;
	CSSM_VL_VO_REGISTRY_INFO_PTR VoInfos;
	uint32 ArchiveSize;
	CSSM_VL_LOCATION Location;  
} CSSM_VL_VOBUNDLE_REGISTRY_INFO, *CSSM_VL_VOBUNDLE_REGISTRY_INFO_PTR;


/* Other VL structures */
typedef uint32 CSSM_VL_VO_SERVICE;
#define CSSM_VL_VO_SERVICE_MULTIVO       0x00000001
#define CSSM_VL_VO_SERVICE_MULTISIGNER   0x00000002
#define CSSM_VL_VO_SERVICE_NESTEDOBJECTS 0x00000004

typedef struct cssm_vlsubservice {
    uint32 SubServiceId;
    CSSM_STRING Description;
    uint32 NumberOfVOBundleTypes;
    CSSM_VL_VOBUNDLE_TYPE *VOBundleTypes;
    uint32 NumberOfDoTypes;
    CSSM_VL_DO_TYPE *DoTypes;
    uint32 NumberOfMediaTypes;
    CSSM_VL_MEDIA_TYPE *MediaTypes;
    CSSM_VL_VO_SERVICE VlVoServiceMask;
} CSSM_VLSUBSERVICE, *CSSM_VLSUBSERVICE_PTR;  

#define CSSM_VL_MAXIMUM_NESTED_VO_DEPTH	0xFFFFFFFF
#define CSSM_VL_ALGID_ANY	            0xFFFFFFFF

typedef struct cssm_vl_do_containment_location {
	CSSM_VL_DO_UNAME DoUniqueName;
	CSSM_VL_LOCATION ContainedLocation;
} CSSM_VL_DO_CONTAINMENT_LOCATION, *CSSM_VL_DO_CONTAINMENT_LOCATION_PTR;

typedef struct cssm_vl_preferred_csp {
	uint32 AlgorithmId;
	CSSM_CSP_HANDLE CSPHandle;
} CSSM_VL_PREFERRED_CSP, *CSSM_VL_PREFERRED_CSP_PTR;



/* structure for passing a memory function table to cssm */
typedef struct cssm_memory_funcs {
    void *(*malloc_func) (uint32 Size, void *AllocRef);
    void (*free_func) (void *MemPtr, void *AllocRef);
    void *(*realloc_func) (void *MemPtr, uint32 Size, void *AllocRef);
    void *(*calloc_func) (uint32 Num, uint32 Size, void *AllocRef);
    void *AllocRef;
} CSSM_MEMORY_FUNCS, *CSSM_MEMORY_FUNCS_PTR;

typedef CSSM_MEMORY_FUNCS CSSM_API_MEMORY_FUNCS;
typedef CSSM_API_MEMORY_FUNCS *CSSM_API_MEMORY_FUNCS_PTR;

/* structure for passing a memory function table to an add-in */
typedef struct cssm_spi_func_tbl {
    void *(*malloc_func) (CSSM_HANDLE AddInHandle, uint32 Size);
    void (*free_func) (CSSM_HANDLE AddInHandle, void *MemPtr);
    void  *(*realloc_func) (CSSM_HANDLE AddInHandle, void *MemPtr, uint32 Size);
    void  *(*calloc_func) (CSSM_HANDLE AddInHandle, uint32 Num, uint32 Size);
} CSSM_SPI_MEMORY_FUNCS, *CSSM_SPI_MEMORY_FUNCS_PTR;

typedef uint32 CSSM_SERVICE_FLAGS;

#define CSSM_SERVICE_ISWRAPPEDPRODUCT 0x1

/*
    Service Info
*/

typedef struct cssm_serviceinfo {
    CSSM_STRING Description;	/* Service description */
    CSSM_SERVICE_TYPE Type;		/* Service type */    
    CSSM_SERVICE_FLAGS Flags;	/* Service flags */  
    uint32 NumberOfSubServices;	/* Number of sub services in SubServiceList */ 
    union cssm_subservice_list {/* List of sub services */
        void *SubServiceList;                        
        CSSM_CSPSUBSERVICE_PTR CspSubServiceList;
        CSSM_DLSUBSERVICE_PTR DlSubServiceList;
        CSSM_CLSUBSERVICE_PTR ClSubServiceList;
        CSSM_TPSUBSERVICE_PTR TpSubServiceList;
    } SubServiceList;
    void* Reserved;
} CSSM_SERVICE_INFO, *CSSM_SERVICE_INFO_PTR;

/* 
    Module Flags 
*/

typedef uint32 CSSM_MODULE_FLAGS;

#define CSSM_MODULE_THREADSAFE				0x1	/* Module is threadsafe */
#define CSSM_MODULE_EXPORTABLE				0x2	/* Module can be exported */
#define CSSM_MODULE_CALLER_AUTHENTOCSSM		0x04/* CSSM authenticates the caller
												 * based on CSSM-known points of
												 * trust
												 */
#define CSSM_MODULE_CALLER_AUTHENTOMODULE	0x08/* CSSM authenticates the caller
												 * based on module-supplied points
												 * of trust
												 */
 
/*
 * Module Info 
 */

typedef struct cssm_moduleinfo {
    CSSM_VERSION Version;			/* Module version */
    CSSM_VERSION CompatibleCSSMVersion;	/* Module written for CSSM version */
	CSSM_GUID_PTR InterfaceGUID;	/* opt GUID defining supported interface */
    CSSM_STRING Description;		/* Module description */
    CSSM_STRING Vendor;				/* Vendor name, etc */
    CSSM_MODULE_FLAGS Flags;		/* Flags to describe and control module use */
	CSSM_DATA_PTR AppAuthenRootKeys; /* Module-specific keys to authenticate apps */
    uint32 NumberOfAppAuthenRootKeys;/* Number of module-specific root keys */
    CSSM_SERVICE_MASK ServiceMask;	/* Bit mask of supported services */
    uint32 NumberOfServices;		/* Num of services in ServiceList */
    CSSM_SERVICE_INFO_PTR ServiceList;	/* Pointer to list of service infos */
    void* Reserved;
} CSSM_MODULE_INFO, *CSSM_MODULE_INFO_PTR;

typedef uint32 CSSM_APP_SERVICE_FLAGS;

#define CSSM_APP_SERVICE_AUTHENTOAPP  0x1 /* CSSM authenticates the service module
										   * based on application-supplied points of
										   * trust 
										   */
/* 
 * This structure aggregates the roots of trust for authenticating a particular 
 * add-in service module during module attach.
 */
typedef struct cssm_app_keys {
    CSSM_DATA_PTR ModuleAuthenRootKeys;	/* Application-specified keys to authen
										 * service modules */
    uint32 NumberOfModuleAuthenRootKeys;/* Number of application-specified root keys
										 */
} CSSM_APP_KEYS, *CSSM_APP_KEYS_PTR;

/*
 * This structure aggregates all information required by CSSM to perform additional
 * authentication of add-in service modules on behalf of an application during 
 * module attach processing.
 */
typedef struct cssm_app_service_info {
    CSSM_SUBSERVICE_UID_PTR ModuleList;  /* List of module service ID structs */
    uint32 NumberOfModules;       		/* Number of modules to authenticate */
    CSSM_APP_SERVICE_FLAGS Flags;	/* Flags selecting CSSM or app-specified roots 
									of trust */
    CSSM_APP_KEYS_PTR Keys;/* Application-specified keys to authenticate modules */
    void *Reserved;
} CSSM_APP_SERVICE_INFO, *CSSM_APP_SERVICE_INFO_PTR; 

typedef CSSM_RETURN (*STATIC_PROC_ADDR)(const char* cssmCredentialPath, const char* cssmSection,
                                  const char* AppCredential, const char* AppSection);  
typedef struct ADDINAUTHTBLKEY
{
    CSSM_GUID         guid;
    STATIC_PROC_ADDR  procaddress;
} ADD_IN_AUTH_TBL, *PADD_IN_AUTH_TBL;


#endif /* _CSSMTYPE_H */


