/*-----------------------------------------------------------------------
 *      File: pkcs7sig.c  
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
* Implementation of ISL_SIGNATURE_METHODS class for the PKCS#7: Cryptographic
* Message Syntax Standard Version 1.5 Revised Novemeber 1, 1993
* 
*/
#include "integapi.h"
#include "isl_internal.h"
#include "cssm.h"
#include "pkcs7oid.h"


extern ISL_SIG_SECTION_METHODS SignedListMethods;
extern ISL_SIGNATURE_METHODS PKCS7SignatureMethods;
extern ISL_CERTIFICATE_METHODS X509CertMethods;
extern ISL_SIG_SECTION_PROTECTED_METHODS SignedListProtectedMethods;


static unsigned char bernull [] =				{ BER_NULL, 0 };
static ISL_CONST_DATA defaultParameters = { sizeof(bernull), bernull };
static
ISL_STATUS
isl_InitializeSignerFromImage(
    ISL_SIGNER_CONTEXT_PTR      SignerPtr,
    ISL_SIGNATURE_CONTEXT_PTR   SigContextPtr,
    ISL_CONST_DATA              Image);

/* class methods */
/*-----------------------------------------------------------------------------
 * Name: SizeofObject_isl_pkcs7sig
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static	ISL_SIZE SizeofObject_isl_pkcs7sig()			/* returns sizeof object */
{
	return sizeof(ISL_SIGNATURE_CONTEXT);
}

	/* object methods */

/*-----------------------------------------------------------------------------
 * Name: isl_InitializeSignatureFromImage
 *
 * Description: Initializes an ISL_SIGNATURE_CONTEXT from a PKCS#7 message
 * loaded into memory.
 *
 * Parameters:
 * memory (output) - memory allocated for this instance
 * configContext (input) - my algorithm<->code extension configuration
 * Archive (input) - archive which contains signature
 * Name (input) - disk file of external representation
 * Image (input) - image of the external representation
 *
 * Return value:
 * 
 * Error Codes:
 *
 *---------------------------------------------------------------------------*/
STATIC	
ISL_STATUS 
isl_InitializeSignatureFromImage(
		ISL_SIGNATURE_CONTEXT *memory,			
		ISL_CONFIG_PTR configContext,
		ISL_MEMORY_CONTEXT *MemoryPtr,
		ISL_CONST_DATA Name,
        ISL_CONST_DATA Image)
{
    ISL_PARSED_PKCS             BerParsedImage;
    ISL_CONST_DATA              *pCertImage;
    uint32                      CertCount;
    ISL_CONST_DATA_PTR          pSignerImage;
    uint32                      SignerCount;
    uint32                      i;

	if (memory == NULL || 
		MemoryPtr == NULL ||
		configContext == NULL ||
		Name.Data == NULL || 
		Image.Data == NULL) 
	{
		return ISL_FAIL;
	}

	cssm_memset(memory, 0, sizeof(ISL_SIGNATURE_CONTEXT));
    memory->Config = configContext;
    memory->Memory = MemoryPtr;
//    memory->Methods = &PKCS7SignatureMethods;
	memory->Name.Length = Name.Length;
	memory->Name.Data = isl_CopyMemory(MemoryPtr, (void*)Name.Data, Name.Length);
	if (memory->Name.Data == NULL) {
		return ISL_FAIL;
	}
	memory->Image.Length = Image.Length;
#ifndef CSSM_BIS
	memory->Image.Data = isl_CopyMemory(MemoryPtr, (void*)Image.Data, Image.Length);
    if (memory->Image.Data == NULL){
		return ISL_FAIL;
	}
#else
    memory->Image.Data = Image.Data;
#endif
    if (ISL_OK != isl_ParsePKCS7(MemoryPtr, memory->Image, &BerParsedImage))
    {
		return ISL_FAIL;
	}

    /* parse Certificates */
    if (ISL_OK != isl_GetCertsFromPKCS(
                    MemoryPtr, 
                    BerParsedImage, 
                    &pCertImage, 
                    &CertCount))
    {
		return ISL_FAIL;		
	}

	if (CertCount != 0)
	{
        ISL_LIST_PTR EndListPtr = NULL;


		for (i=0; i < CertCount; i++)
		{
		    ISL_CERTIFICATE_PTR CertPtr;
		    ISL_LIST_PTR CertListPtr;

		    CertPtr = isl_AllocateMemory(
                MemoryPtr,
                sizeof(ISL_CERTIFICATE));
		    if (CertPtr == NULL) {
			    return ISL_FAIL;
		    }

		    CertListPtr = isl_AllocateMemory(
                MemoryPtr, 
                sizeof(ISL_CERTIFICATE_LIST));
		    if (CertListPtr == NULL) 
		    {
			    return ISL_FAIL;
		    }
			if (ISL_OK != X509CertMethods.Initialize(
								CertPtr,	
							    MemoryPtr,
								pCertImage[i]))
			{
				return ISL_FAIL;
			}

			CertListPtr->Node = CertPtr;
			CertListPtr->Next = NULL;
			if (memory->Certificates == NULL)
            {
                memory->Certificates = CertListPtr;
            }
            if (EndListPtr)
            {
                EndListPtr->Next = CertListPtr;
                EndListPtr = CertListPtr;
            }
            else
            {
                EndListPtr = CertListPtr;
            }
		}
	}
    
    if (ISL_OK != isl_GetSignedContentFromPKCS(
        BerParsedImage, 
        &memory->SignedImage))
	{
		return ISL_FAIL;
	}

    /* parse Signers */
    if (ISL_OK != isl_GetSignersFromPKCS(
                    MemoryPtr, 
                    BerParsedImage, 
                    &pSignerImage, 
                    &SignerCount))
	{
		return ISL_FAIL;
	}

	if (SignerCount != 0)
	{
        ISL_LIST_PTR EndListPtr = NULL;

		for(i=0; i < SignerCount; i++)
		{
		    ISL_SIGNER_CONTEXT_PTR SignerPtr;
            ISL_LIST_PTR SignerNodePtr;

	        SignerPtr = isl_AllocateMemory(
                MemoryPtr, 
                sizeof(ISL_SIGNER_CONTEXT));
	        if (SignerPtr == NULL) return ISL_FAIL;

            SignerNodePtr = isl_AllocateMemory(
                MemoryPtr,
                sizeof(ISL_LIST));
            if (SignerNodePtr == NULL) return ISL_FAIL;

            SignerNodePtr->Node = SignerPtr;
			if (ISL_OK != isl_InitializeSignerFromImage(
								SignerPtr,
								memory, 
								pSignerImage[i]))
			{
				return ISL_FAIL;
			}
			
            if (EndListPtr)
            {
                EndListPtr->Next = SignerNodePtr;
                EndListPtr = SignerNodePtr;
            }
            else
            {
                memory->Signers = SignerNodePtr;
                EndListPtr = SignerNodePtr;
            }
		}
	}
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: isl_InitializeSignerFromImage
 *
 * Description: Decodes a SignerInfo block
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_STATUS
isl_InitializeSignerFromImage(
    ISL_SIGNER_CONTEXT_PTR      SignerPtr,
    ISL_SIGNATURE_CONTEXT_PTR   SigContextPtr,
    ISL_CONST_DATA              Image)
{
    ISL_CONFIG *ConfigPtr;
    sint32 ItemCount;
    sint32 Count;
    sint32 i;
    ISL_CONST_DATA AlgID;
    ISL_SERVICE_CLASS_METHODS *methods;

    BER_PARSED_ITEM_PTR SignerInfoPtr;		/* output: array of parsed items */
    BER_PARSED_ITEM Digest;
    BER_PARSED_ITEM Signature;
    
    if (SigContextPtr == NULL) return ISL_FAIL;
    cssm_memset(SignerPtr, 0, sizeof(ISL_SIGNER_CONTEXT));

    ConfigPtr = SigContextPtr->Config;
    SignerPtr->Parent = SigContextPtr;
    ItemCount = BER_CountItems((uint8 *)Image.Data, Image.Length);
    if (ItemCount <= 2) return ISL_FAIL;

    SignerInfoPtr = isl_AllocateMemory(
        SignerPtr->Parent->Memory,
        sizeof(BER_PARSED_ITEM)*ItemCount);

    if (SignerInfoPtr == NULL) {
		return ISL_FAIL;
	}
    Count = BER_ExpandSequence(     /* return count of items parsed */
	            Image.Data,			/* input BER to parse */
	            Image.Length,		/* length of BER to parse */
	            ItemCount,		    /* input number of components */
	            NULL,               /* explicit tag choices for each field */
	            NULL,	            /* implicit context-specific tags */
	            NULL,	            /* original tags for each component */
	            NULL,	            /* default values */
                SignerInfoPtr);

    SignerPtr->Parent = SigContextPtr;	/* signature that this signs */

    /* issuer+s/n (see cert->SignerID()) */
	SignerPtr->SignerID.Data = SignerInfoPtr[1].Tag; 
    SignerPtr->SignerID.Length = BER_SizeofObject(SignerPtr->SignerID.Data);

    /* Decode Digest Algorithm Block */
    Digest = SignerInfoPtr[2];
    AlgID.Data = Digest.Content;
    AlgID.Length = BER_SizeofObject(AlgID.Data);
    methods = ArchiveConfigMethods.FindAlgorithm(ConfigPtr, AlgID);
	SignerPtr->DigestMethods = (ISL_DIGEST_METHODS *)methods;               
    SignerPtr->DigestParameters.Data = Digest.Content + AlgID.Length;
	SignerPtr->DigestParameters.Length = Digest.ContentLength - AlgID.Length;

     /* Decode Signature Algorithm Block */
    for (i= 3; i < Count; i++) {
        if (SignerInfoPtr[i].Tag[0] == (BER_CONSTRUCTED | BER_SEQUENCE) )
            break;
    }
    if (i == Count) return ISL_FAIL;

    Signature = SignerInfoPtr[i];
    AlgID.Data = Signature.Content;
    AlgID.Length = BER_SizeofObject(AlgID.Data);
    methods = ArchiveConfigMethods.FindAlgorithm(ConfigPtr, AlgID);
	SignerPtr->SignMethods = (ISL_SIGN_VERIFY_METHODS *)methods;
    SignerPtr->SignParameters.Data = Signature.Content + AlgID.Length;    
	SignerPtr->SignParameters.Length = Signature.ContentLength - AlgID.Length;
    
    /* Decode Signature Block */
    i++;
    SignerPtr->Signature.Data = SignerInfoPtr[i].Content;
	SignerPtr->Signature.Length = SignerInfoPtr[i].ContentLength;
	SignerPtr->Image = Image;
    return ISL_OK;
}


/*
**	signer context methods
*/
/*-----------------------------------------------------------------------------
 * Name: GetSignerID
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS GetSignerID_isl_pkcs7sig(
		ISL_SIGNER_CONTEXT_PTR Context,			/* signer context */
		ISL_CONST_DATA *SignerID)				/* updated issuer and serial number */
{
	if (Context == NULL ||
		SignerID == NULL)
		return ISL_FAIL;

	*SignerID = Context->SignerID;
	return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: VerifyUsingCert
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_STATUS VerifyUsingCert_isl_pkcs7sig(
		ISL_SIGNER_CONTEXT *Context,					/* archive context */
		const ISL_CERTIFICATE_METHODS *CertMethods,		/* configuration methods for certificate format */
		ISL_CERTIFICATE *Cert)							/* the certificate */
{
	ISL_KEY_BLOCK key;
	CSSM_KEY_PTR KeyPtr;
	CSSM_CSP_HANDLE hCSP;
	CSSM_CC_HANDLE hCC;
	CSSM_BOOL cssmstatus;
	CSSM_DATA Signature;
	CSSM_DATA Image;

	if (Context == NULL ||
		Context->Parent == NULL ||
		Context->SignMethods == NULL ||
		CertMethods == NULL ||
		CertMethods->GetKey == NULL ||
		Cert == NULL) 
	{
		return ISL_FAIL;
	}
	
	Signature.Data = (uint8 *)Context->Signature.Data;
	Signature.Length = Context->Signature.Length;
	
	Image.Data = (uint8 *)Context->Parent->SignedImage.Data;
	Image.Length = Context->Parent->SignedImage.Length;

	if (ISL_OK != CertMethods->GetKey(Cert, &key)) return ISL_FAIL;

	KeyPtr = (CSSM_KEY_PTR) key;
	hCSP = Context->SignMethods->CSPHandle;
	if (hCSP == 0) return ISL_FAIL;
	hCC = CSSM_CSP_CreateSignatureContext(hCSP, CSSM_ALGID_SHA1WithDSA, NULL, KeyPtr);
	if (hCC == 0) return ISL_FAIL;
	cssmstatus = CSSM_VerifyData(hCC, &Image, 1, &Signature);
	CSSM_DeleteContext(hCC);
	if (cssmstatus != CSSM_TRUE) {
		CSSM_ERROR_PTR ErrorPtr;
		ErrorPtr = CSSM_GetError();
		return ISL_FAIL; //FIXME:change error code;
	}

    return ISL_OK;
}


/*-----------------------------------------------------------------------------
 * Name: FindSignerCertificate
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_CERTIFICATE *FindSignerCertificate_isl_pkcs7sig(	/* return found object or NULL */
		ISL_SIGNER_CONTEXT *Context)
{
	ISL_LIST_PTR pCertList = NULL;
	ISL_CERTIFICATE *pCertificate = NULL;
	
	if (!Context)
		return NULL;

	if (Context->CertChain)
    {
		/* the signer's cert is the first cert in the cert chain */
		pCertificate = Context->CertChain->CertList->Cert;
    }
	else if (Context->Parent->Certificates) {
		/* search through all certificates in the signature for signer's cert */
		for(pCertList= Context->Parent->Certificates;
		    pCertList;
            pCertList = pCertList->Next) 
        {
            ISL_CERTIFICATE_PTR CurrCertPtr;

            CurrCertPtr = pCertList->Node;
			if (IS_EQUAL(CurrCertPtr->pkcs7id, Context->SignerID))
            {
				pCertificate = CurrCertPtr;
				break;
			}
		}
	}
	
	return pCertificate;

}

/*-----------------------------------------------------------------------------
 * Name: GetSignature
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
ISL_STATUS GetSignature_isl_pkcs7sig(
    ISL_SIGNER_CONTEXT_PTR Context,
    ISL_CONST_DATA *Signature)
{
    if (Context == NULL ||
        Signature == NULL)
    {
        return ISL_FAIL;
    }

    *Signature = Context->Signature;
    return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: FindSigner
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static ISL_SIGNER_CONTEXT_PTR FindSigner_isl_pkcs7sig(
		ISL_SIGNATURE_CONTEXT_PTR Context,
		ISL_CERTIFICATE_PTR CertificatePtr)
{
    ISL_LIST_PTR SignerNodePtr;

	for(SignerNodePtr = Context->Signers;
		SignerNodePtr != NULL;
		SignerNodePtr = SignerNodePtr->Next)
	{
	    ISL_SIGNER_CONTEXT_PTR SignerObjectPtr;
        SignerObjectPtr = SignerNodePtr->Node;
        if (SignerObjectPtr == NULL) return NULL;

		if (IS_EQUAL(CertificatePtr->pkcs7id, SignerObjectPtr->SignerID))
		{
			return SignerObjectPtr;
		}
	}
	return NULL;
}

#define gsDSA "DSA"
/*-----------------------------------------------------------------------------
 * Name: id_isl_pkcs7sig
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static void id_isl_pkcs7sig(
	ISL_SERVICE_CLASS *algClass,				
	ISL_CONST_DATA *algID,				
	ISL_CONST_DATA *serviceName)
{
	*algClass = ISL_ServiceParseSignature;
	algID->Data = (const uint8 *)gsDSA;
	algID->Length = sizeof(gsDSA) - 1;
	serviceName->Data = (const uint8 *)gsDSA;
	serviceName->Length = sizeof(gsDSA) -1;
}


/*-----------------------------------------------------------------------------
 * Name: GetSignerGroup
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_SIGNER_GROUP_PTR 
GetSignerGroup_isl_pkcs7sig(
    ISL_SIGNATURE_CONTEXT *Context)
{
    ISL_SIGNER_GROUP_PTR SignerGrpPtr = NULL;

    if (Context == NULL) return NULL;

    {
        ISL_SIGNER_CONTEXT_PTR *SignerObjectPtr;
        ISL_LIST_PTR SignerNodePtr;

        uint32 NumberOfSigners;

        SignerGrpPtr = isl_AllocateMemory(
            Context->Memory,
            sizeof(ISL_SIGNER_GROUP));
        if (SignerGrpPtr == NULL) goto FAIL;

        NumberOfSigners = isl_CountItemsInList(Context->Signers);
        if (NumberOfSigners == 0) return SignerGrpPtr;
        SignerGrpPtr->NumberOfSigners = NumberOfSigners;

        SignerObjectPtr = isl_AllocateMemory(
            Context->Memory,
            sizeof(ISL_SIGNER_CONTEXT_PTR) * NumberOfSigners);
        if (SignerObjectPtr == NULL) goto FAIL;
        SignerGrpPtr->Signers = SignerObjectPtr;

        for(SignerNodePtr = Context->Signers;
            SignerNodePtr != NULL;
            SignerNodePtr = SignerNodePtr->Next)
        {
            *SignerObjectPtr = SignerNodePtr->Node;
            SignerObjectPtr++;
        }
        return SignerGrpPtr;
    }
FAIL:
    {
        if (SignerGrpPtr)
        {
            isl_FreeMemory(
                Context->Memory,
                SignerGrpPtr->Signers);
        }
        isl_FreeMemory(
            Context->Memory,
            SignerGrpPtr);
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: FreeSignerGroup
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *---------------------------------------------------------------------------*/
static
ISL_STATUS 
FreeSignerGroup_isl_pkcs7sig(
    ISL_SIGNATURE_CONTEXT *Context,
    ISL_SIGNER_GROUP_PTR SignerGrpPtr)
{
    if (Context == NULL) return ISL_FAIL;

    if (SignerGrpPtr)
    {
        isl_FreeMemory(
            Context->Memory,
            SignerGrpPtr->Signers);
    }
    isl_FreeMemory(
        Context->Memory,
        SignerGrpPtr);

    return ISL_OK;
}

/*-----------------------------------------------------------------------------
 * Name: GetCertificateGroup
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *
 * Notes:
 * Use FreeCertificateGroup to free ISL_CERTIFICATE_GROUP_PTR
 *---------------------------------------------------------------------------*/
static
ISL_CERTIFICATE_GROUP_PTR 
GetCertificateGroup_isl_pkcs7sig(
    ISL_SIGNATURE_CONTEXT *Context)
{
    if (Context == NULL) return NULL;

    return isl_BuildCertificateGroup(
        Context->Certificates,
        Context->Memory);
}

/*-----------------------------------------------------------------------------
 * Name: FreeCertificateGroup
 *
 * Description:
 *
 * Parameters: 
 *
 * Return value:
 * 
 * Error Codes:
 *
 *---------------------------------------------------------------------------*/
static
ISL_STATUS 
FreeCertificateGroup_isl_pkcs7sig(
    ISL_SIGNATURE_CONTEXT *Context,
    ISL_CERTIFICATE_GROUP_PTR CertificateGrpPtr)
{
    if (Context == NULL) return ISL_FAIL;

    return isl_FreeCertificateGroup(
        CertificateGrpPtr,
        Context->Memory);
}
/*
**	export vector of methods
*/
ISL_SIGNATURE_METHODS PKCS7SignatureMethods = {
	{id_isl_pkcs7sig,0},
	SizeofObject_isl_pkcs7sig,						//synonym		
    isl_InitializeSignatureFromImage,
	FindSigner_isl_pkcs7sig,
    GetSignerGroup_isl_pkcs7sig,
    FreeSignerGroup_isl_pkcs7sig,
    GetCertificateGroup_isl_pkcs7sig,
    FreeCertificateGroup_isl_pkcs7sig
};



ISL_SIGNER_METHODS PKCS7SignerMethods = {
	{0, 0},
	GetSignerID_isl_pkcs7sig,
	VerifyUsingCert_isl_pkcs7sig,
	FindSignerCertificate_isl_pkcs7sig,
    GetSignature_isl_pkcs7sig
};
