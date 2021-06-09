/*-----------------------------------------------------------------------
 *      File:   integapi.h
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

/*
**	Integrity verification tool and library	API
*/
#ifndef ISL_API_H
#define ISL_API_H
#include "isltype.h"						   


/* error codes */
#define ISL_BASE_ERROR				(0UL )    /* Defined by CSSM */
#define ISL_NO_ERROR				(0UL )
#define ISL_INVALID_INPUT			(ISL_BASE_ERROR +1L )
#define ISL_NO_MEMORY				(ISL_BASE_ERROR +2L )
#define ISL_FORGERY					(ISL_BASE_ERROR +3L )
#define ISL_MISSING_PART			(ISL_BASE_ERROR +4L )
#define ISL_UNSUPPORTED_ALGORITHM	(ISL_BASE_ERROR +5L )
#define ISL_SIGNER_NOT_FOUND		(ISL_BASE_ERROR +6L )
#define ISL_IO_ERROR				(ISL_BASE_ERROR +7L )
#define ISL_ARCHIVE_INIT_FAILED		(ISL_BASE_ERROR +8L )
#define ISL_SIGNATURE_NOT_FOUND		(ISL_BASE_ERROR +9L )
#define ISL_INVALID_PARAMETER		(ISL_BASE_ERROR +10L )
#define ISL_ALGORITHM_NOT_FOUND		(ISL_BASE_ERROR +11L )
#define ISL_OBJECT_NOT_FOUND		(ISL_BASE_ERROR +12L )
#define ISL_PATH_TOO_LONG			(ISL_BASE_ERROR +13L )
#define ISL_DIGEST_NOT_FOUND		(ISL_BASE_ERROR +14L )
#define ISL_INVALID_SIGNATURE_FILE  (ISL_BASE_ERROR +15L )
#define ISL_UNABLE_TO_OPEN_FILE		(ISL_BASE_ERROR +16L )
#define ISL_INVALID_SIGNATURE		(ISL_BASE_ERROR +17L )
#define ISL_INVALID_CERTIFICATE		(ISL_BASE_ERROR +18L )
#define ISL_INVALID_BER_ENCODING	(ISL_BASE_ERROR +19L )
#define ISL_OBJECT_LIST_ALREADY_DEFINED			(ISL_BASE_ERROR +20L )
#define ISL_SIGNED_LIST_FAILED_TO_INITIALIZE	(ISL_BASE_ERROR +21L )
#define ISL_FAILED_TO_SET_SIGNED_LIST			(ISL_BASE_ERROR +22L )
#define ISL_CERTIFICATE_FAILED_TO_INITIALIZE	(ISL_BASE_ERROR +23L )
#define ISL_INVALID_DIGEST_CONFIG				(ISL_BASE_ERROR +24L )
#define ISL_INVALID_SIGNER_CONTEXT				(ISL_BASE_ERROR +25L )
#define ISL_NO_SIGNED_LIST						(ISL_BASE_ERROR +26L )
#define ISL_INVALID_SIGNED_LIST					(ISL_BASE_ERROR +27L )
#define ISL_INVALID_ARCHIVE						(ISL_BASE_ERROR +28L )
#define ISL_UNIMPLEMENTED						(ISL_BASE_ERROR +999L )

/* temporary definitions for backwards compatibility with previous work */
#define INTEGRITY_OK				ISL_OK						
#define INTEGRITY_INVALID_INPUT		ISL_INVALID_INPUT			
#define INTEGRITY_NO_MEMORY			ISL_NO_MEMORY				
#define INTEGRITY_FORGERY			ISL_FORGERY					
#define INTEGRITY_MISSING_PART		ISL_MISSING_PART			
#define INTEGRITY_UNSUPPORTED_ALGORITHM	ISL_UNSUPPORTED_ALGORITHM
#define INTEGRITY_SIGNER_NOT_FOUND	ISL_SIGNER_NOT_FOUND	
#define INTEGRITY_IO_ERROR			ISL_IO_ERROR			
#define INTEGRITY_UNIMPLEMENTED		ISL_UNIMPLEMENTED
/* end temporary definitions */

/*
**	all services have a common routine to identify themselves,
**	providing a human readable string as well as their signature format-specific algorithm ID
*/
typedef struct isl_service_class_methods {
	void (*id)(
		ISL_SERVICE_CLASS *AlgClass,		/* return service type code */
		ISL_CONST_DATA_PTR AlgID,			/* return archive-specific encoding */
		ISL_CONST_DATA_PTR ServiceName);	/* return human-readable description */
	ISL_CLASS_PTR Class;					/* pointer to class structure */
} ISL_SERVICE_CLASS_METHODS;

/*
**	General methods for any object (not in the methods vector--we may not have the methods!)
*/
ISL_CLASS_PTR ISL_GetClass(void * Context);		/* returns class structure for any class */

/* 
**	ISL_ARCHIVE_CONFIGuration class public methods (configuration)
*/
typedef struct isl_config_methods{
	ISL_SERVICE_CLASS_METHODS ServiceMethods;

	/* class methods */
	ISL_SIZE (*SizeofObject)();				/* returns sizeof object */
	/* object methods */
	ISL_STATUS (*Initialize)(				/* ISL_ARCHIVE_CONTEXT constructor */
		ISL_CONFIG *Context,				/* pointer to configuration context */
		ISL_MEMORY_METHODS *MemoryMethods);	/* malloc, free methods */
	
	ISL_STATUS (*AddAlgorithm)(
		ISL_CONFIG *Context,						/* configuration context */
		const ISL_SERVICE_CLASS_METHODS *Methods);	/* vector of methods */

	ISL_SERVICE_CLASS_METHODS *(*FindAlgorithm)(	/* return a pointer to the service methods vector */
		ISL_CONFIG *Context,						/* pointer to configuration context */
		ISL_CONST_DATA AlgID);						/* given an algorithm ID or HumanName */
	
	ISL_STATUS (*Recycle)(					/* destructor for IntegrityContext */
		ISL_CONFIG *Context);				/* configuration context */

} ISL_CONFIG_METHODS;
/*
**	ISL_ARCHIVE_CONTEXT class public methods
*/
typedef struct isl_archive_methods{
	ISL_SERVICE_CLASS_METHODS ServiceMethods;

	/* class methods */
	ISL_SIZE (*SizeofObject)();			/* returns sizeof object */
	
	/* object methods */
	
	ISL_STATUS (*InitializeOld)(			/* constructor for existing archive */
		ISL_ARCHIVE_CONTEXT *Memory,		/* memory allocated for this instance */
		ISL_CONFIG *ConfigContext, 			/* my algorithm<->code extension configuration */
		ISL_CONST_DATA InMemoryImage);		/* external representation */
			
	ISL_STATUS (*Recycle)(					/* destructor for archive */
		ISL_ARCHIVE_CONTEXT *Context);	    /* archive context */
	
	ISL_MANIFEST_SECTION_PTR (*FindSignableObject)(		/* return found object or NULL */
		ISL_ARCHIVE_CONTEXT *Context,					/* context */
		ISL_CONST_DATA Name);							/* archive-unique name */

    ISL_STATUS (*Verify)(
        ISL_ARCHIVE_CONTEXT_PTR ArchivePtr,             /* archive context */
	    ISL_SIGNATURE_CONTEXT_PTR SignaturePtr,			/* signature context */
	    ISL_CERTIFICATE_METHODS *CertMethods,		    /* configuration methods for certificate format */
	    ISL_CERTIFICATE *Cert);							/* the certificate */
    
    ISL_LMAP_PTR (*GetLocationMap)(
	    ISL_ARCHIVE_CONTEXT_PTR ArchivePtr);

    ISL_STATUS (*SetLocationMap)(
        ISL_ARCHIVE_CONTEXT_PTR ArchivePtr,
	    ISL_LMAP Map);

    ISL_STATUS (*FindAttributeValue)(		/* find metadata in section */
		ISL_ARCHIVE_CONTEXT_PTR Context,		/* archive context */
		ISL_CONST_DATA Attribute,			/* attribute "name" */
		ISL_CONST_DATA_PTR Value);			/* fill in Value if found */

    ISL_SIGNATURE_INFO_GROUP_PTR (*GetSignatureGroup)(
        ISL_ARCHIVE_CONTEXT_PTR Context);

    ISL_STATUS (*FreeSignatureGroup)(
        ISL_ARCHIVE_CONTEXT_PTR Context,
        ISL_SIGNATURE_INFO_GROUP_PTR SignatureInfoGroupPtr);

    ISL_CERTIFICATE_GROUP_PTR (*GetCertificateGroup)(
        ISL_ARCHIVE_CONTEXT_PTR Context);

    ISL_STATUS (*FreeCertificateGroup)(
        ISL_ARCHIVE_CONTEXT_PTR Context,
        ISL_CERTIFICATE_GROUP_PTR CertificateGroupPtr);

} ISL_ARCHIVE_METHODS;

/*
**	ISL_SIG_SECTION_LIST class public methods
*/
typedef struct isl_sig_section_methods {
	ISL_SERVICE_CLASS_METHODS ServiceMethods;

	/* class methods */
	ISL_SIZE (*SizeofObject)();				/* returns sizeof object */
	
	/* object methods */
	
	ISL_SIG_SECTION_PTR (*FindSignedObject)(	/* return found object or NULL */
		ISL_SIG_SECTION_LIST *Context,					/* context */
		ISL_CONST_DATA Name);						/* name of section to be found */

	ISL_STATUS (*FindAttributeValue)(		/* find metadata in section */
		ISL_SIG_SECTION_LIST_PTR Context,		/* archive context */
		ISL_CONST_DATA Attribute,			/* attribute "name" */
		ISL_CONST_DATA_PTR Value);			/* fill in Value if found */

    ISL_SIG_SECTION_GROUP_PTR (*GetSignedObjectInfos)(
        ISL_SIG_SECTION_LIST_PTR Context);

    ISL_STATUS (*FreeSignedObjectInfos)(
        ISL_SIG_SECTION_LIST_PTR Context,
        ISL_SIG_SECTION_GROUP_PTR SignedObjectInfoPtr);

} ISL_SIG_SECTION_METHODS;

/*
**	ISL_SIGNATURE_CONTEXT class public methods
*/
typedef struct isl_signature_methods{	
	ISL_SERVICE_CLASS_METHODS ServiceMethods;

	/* class methods */
	ISL_SIZE (*SizeofObject)();					/* returns sizeof object */
	
	/* object methods */
    ISL_STATUS (*InitializeFromImage)(			/* constructor for existing archive */
		ISL_SIGNATURE_CONTEXT   *Context,		/* memory allocated for this instance */
		ISL_CONFIG				*ConfigContext,	/* my algorithm<->code extension configuration */
		ISL_MEMORY_CONTEXT_PTR  Memory,		/* archive which contains signature */
		ISL_CONST_DATA          Name,			/* disk file of external representation */
	    ISL_CONST_DATA          Image);         /* image of external representation */
	
	ISL_SIGNER_CONTEXT_PTR (*FindSigner)(
		ISL_SIGNATURE_CONTEXT *Context,
		ISL_CERTIFICATE *Certificate);

    ISL_SIGNER_GROUP_PTR (*GetSignerGroup)(
        ISL_SIGNATURE_CONTEXT *Context);

    ISL_STATUS (*FreeSignerGroup)(
        ISL_SIGNATURE_CONTEXT *Context,
        ISL_SIGNER_GROUP_PTR SignerGroupPtr);

    ISL_CERTIFICATE_GROUP_PTR (*GetCertificateGroup)(
        ISL_SIGNATURE_CONTEXT *Context);

    ISL_STATUS (*FreeCertificateGroup)(
        ISL_SIGNATURE_CONTEXT *Context,
        ISL_CERTIFICATE_GROUP_PTR CertificatePtr);

} ISL_SIGNATURE_METHODS;
/*
**	Signer Class
*/
typedef struct isl_signer_methods {
	ISL_SERVICE_CLASS_METHODS ServiceMethods;

	ISL_STATUS (*GetSignerID)(
		ISL_SIGNER_CONTEXT_PTR Context,			/* signer context */
		ISL_CONST_DATA *SignerID);				/* updated issuer and serial number */

	ISL_STATUS (*VerifyUsingCert)(
		ISL_SIGNER_CONTEXT_PTR Context,				/* signer context */
		ISL_CERTIFICATE_METHODS *CertMethods,	/* configuration methods for certificate format */
		ISL_CERTIFICATE *Cert);				/* the certificate */

	ISL_CERTIFICATE *(*FindSignerCertificate)(	/* return found object or NULL */
		ISL_SIGNER_CONTEXT_PTR Context);

    ISL_STATUS (*GetSignature)(
        ISL_SIGNER_CONTEXT_PTR Context,
        ISL_CONST_DATA *Signature);

} ISL_SIGNER_METHODS;
/*
**	Signable Object (Manifest Section) Class
*/
typedef struct isl_manifest_section_methods {
	ISL_SERVICE_CLASS_METHODS ServiceMethods;

	/* class methods */
	ISL_SIZE (*SizeofObject)();			/* returns sizeof object */
	
	/* object methods */

	ISL_STATUS (*Verify)(
		ISL_MANIFEST_SECTION_PTR Context);		/* context */
	
	ISL_STATUS (*FindAttributeValue)(			/* find metadata in section */
		ISL_MANIFEST_SECTION_PTR Context,		/* archive context */
		ISL_CONST_DATA Attribute,				/* attribute "name" */
		ISL_CONST_DATA_PTR Value);				/* fill in Value and Length if found */

    ISL_ATTRIBUTE_GROUP_PTR (*GetAttributeGroup)(
        ISL_MANIFEST_SECTION_PTR Context);

    ISL_STATUS (*FreeAttributeGroup)(
        ISL_MANIFEST_SECTION_PTR Context,
        ISL_ATTRIBUTE_GROUP_PTR AttributeInfoPtr);

} ISL_MANIFEST_SECTION_METHODS;

typedef struct isl_signed_section_methods {
	ISL_SERVICE_CLASS_METHODS ServiceMethods;

	/* class methods */
	ISL_SIZE (*SizeofObject)();			/* returns sizeof object */
	
	/* object methods */

	ISL_STATUS (*Verify)(
		ISL_SIG_SECTION_PTR Context);		/* context */

    ISL_MANIFEST_SECTION_PTR (*GetManifestSection)(
        ISL_SIG_SECTION_PTR Context);
	
	ISL_STATUS (*FindAttributeValue)(			/* find metadata in section */
		ISL_SIG_SECTION_PTR Context,		/* archive context */
		ISL_CONST_DATA Attribute,				/* attribute "name" */
		ISL_CONST_DATA_PTR Value);				/* fill in Value and Length if found */

    ISL_ATTRIBUTE_GROUP_PTR (*GetAttributeGroup)(
        ISL_SIG_SECTION_PTR Context);

    ISL_STATUS (*FreeAttributeGroup)(
        ISL_SIG_SECTION_PTR Context,
        ISL_ATTRIBUTE_GROUP_PTR AttributeInfoPtr);
} ISL_SIGNED_SECTION_METHODS;

/*
**	Certificate object methods
*/
typedef struct isl_certificate_methods {
	ISL_SERVICE_CLASS_METHODS		ServiceMethods;

	/* class methods */
	ISL_SIZE (*SizeofObject)();			/* returns sizeof object */
	
	/* object methods */
	ISL_STATUS (*Initialize)(
		ISL_CERTIFICATE *ContextPtr,			/* memory for internal rep of cert */
		ISL_MEMORY_CONTEXT_PTR MemoryPtr,		/* memory allocation methods */
		ISL_CONST_DATA Cert);					/* external rep of certificate */

	ISL_STATUS (*Recycle)(
		ISL_CERTIFICATE *Cert);

	ISL_STATUS (*GetID)(
		ISL_CERTIFICATE *Cert,
		ISL_CONST_DATA_PTR SignerID);

	ISL_STATUS (*GetKey)(
		ISL_CERTIFICATE *Cert,
		ISL_KEY_BLOCK_PTR Key);

	ISL_STATUS (*GetAlgID)(
		ISL_CERTIFICATE *Cert,
		ISL_CONST_DATA *AlgID);	

    ISL_STATUS (*Flatten)(
        ISL_CERTIFICATE *Cert,
        ISL_CONST_DATA *theCert);

} ISL_CERTIFICATE_METHODS;
/*
**	Integrity Service Provider Interfaces
**	Each service provider may be specific to a signature format, archive format, etc.
*/


/* 
**	get signed data service
*/
typedef struct isl_get_data_methods{
	ISL_SERVICE_CLASS_METHODS ServiceMethods;

	ISL_SIZE (*SizeofObject)();

	ISL_STATUS (*Initialize)(
		ISL_GET_DATA_SERVICE_CONTEXT *Memory,
		ISL_CONST_DATA Parameters);				/* archive-specific-encoded parameters */
		// void *ClassContext,							/* class context */
		// ISL_MANIFEST_SECTION_PTR *Section);			/* manifest section, if any */	

	ISL_ERROR_CODE (*Update)(
		ISL_GET_DATA_SERVICE_CONTEXT *Context,
		ISL_CONST_DATA_PTR Data);					/* location, length of data returned */ 

	ISL_ERROR_CODE (*Recycle)(
		ISL_GET_DATA_SERVICE_CONTEXT *Context);

	ISL_STATUS (*InitializeWithClass)(
		ISL_GET_DATA_SERVICE_CONTEXT *Memory,
		ISL_CONST_DATA Parameters,					/* archive-specific-encoded parameters */
		ISL_CLASS_PTR Class,						/* class structure */
		ISL_MANIFEST_SECTION_PTR Section);			/* manifest section, if any */
		
} ISL_GET_DATA_METHODS;

/*
**	digest function service
*/
typedef struct isl_digest_methods {
	ISL_SERVICE_CLASS_METHODS ServiceMethods;
	CSSM_CSP_HANDLE CSPHandle;
	uint32 AlgorithmID;
} ISL_DIGEST_METHODS;


/*
**	signature service: digest encrypt/decrypt, no context
*/
typedef struct isl_sign_verify_methods {
	ISL_SERVICE_CLASS_METHODS ServiceMethods;
	CSSM_CSP_HANDLE CSPHandle;
	uint32 AlgorithmID;
} ISL_SIGN_VERIFY_METHODS;


void ISL_SetError(uint32 ErrCode);
uint32 ISL_GetError();

#endif /* ifndef ISL_API_H */
