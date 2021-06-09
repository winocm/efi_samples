/*-----------------------------------------------------------------------
 *      File:   x509v3cl.c
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
 * This file implements the CL SPI.  
 */

#include "x_fndefs.h"

/*-----------------------------------------------------------------------------
 * Name: CL_CertGetFirstFieldValue
 *
 * Description:
 * This function returns the value of the designated certificate field.
 * None of the supported OIDs have more than one field in the certificate,
 * so the ResultsHandle will always be 0 and the NumberOfMatchedFields
 * will always be 0 or 1.
 *
 * NOTE: ClearError() is not necessary because it is performed by CSSM.
 *
 * Parameters: 
 * Cert (input)           : The certificate from which to obtain the field value
 * CertField (input)      : An OID that identifies the field value 
 *                          to be extracted from the Cert.
 * ResultsHandle (output) : A handle that the application can use 
 *                          to obtain additional matched elements.
 *                          This will always be 0 for this implementation.
 * NumberOfMatchedFields  : An output that indicates the number of 
 *               (output)   matching elements in the Cert
 *                          This will always be 0 or 1 for this implementation.
 *
 * Return value:
 * The value of the requested field
 * This buffer and its encapsulated Data->Data will need to be freed 
 * by the calling application.
 *
 * Error Codes:
 * CSSM_CL_INVALID_CERT_POINTER
 * CSSM_CL_INVALID_DATA_POINTER
 * CSSM_CL_UNKNOWN_FORMAT
 * CSSM_CL_UNKNOWN_TAG
 * CSSM_CL_MEMORY_ERROR
 *---------------------------------------------------------------------------*/
CSSM_DATA_PTR CSSMCLI CL_CertGetFirstFieldValue 
                                    (CSSM_CL_HANDLE CLHandle,
                                     const CSSM_DATA_PTR Cert, 
                                     const CSSM_OID_PTR CertField,
                                     CSSM_HANDLE_PTR ResultsHandle,
                                     uint32 *NumberOfMatchedFields)
{
    DER_NODE_PTR   cert_ptr;
    CSSM_DATA_PTR  results_ptr;

    /* Check inputs */
    /* CLHandle is checked by CSSM */
    if ( !Cert || !Cert->Data || !Cert->Length)
    {
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_INVALID_CERT_POINTER);
        return NULL;
    }
    if (!CertField || !CertField->Data || !CertField->Length || 
        !ResultsHandle || !NumberOfMatchedFields)
    {
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_INVALID_DATA_POINTER);
        return NULL;
    }
    *ResultsHandle = 0;
    *NumberOfMatchedFields = 0;

    /* Obtain the parse tree representation of the Cert */
    cert_ptr = cl_DerDecodeCertificate(CLHandle, Cert);
    if (!cert_ptr)
    {
        if ((CSSM_GetError())->error != CSSM_CL_MEMORY_ERROR)
            CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_UNKNOWN_FORMAT);
        return NULL;
    }

    /* Get the field value */
    results_ptr = cl_CertGetField(CLHandle, cert_ptr, 
                                  CertField, NumberOfMatchedFields);

    /* Cleanup & Return */
    cl_FreeCertificate(CLHandle, cert_ptr);
    return results_ptr;
};

/*-----------------------------------------------------------------------------
 * Name: CL_CertAbortQuery
 *
 * Description:
 * This function removes the results of a previous query from the query table.
 * However, none of the supported OIDs have more than one field 
 * in the certificate, so this function does nothing.
 *
 * NOTE: ClearError() is not necessary because it is performed by CSSM.
 *
 * Parameters: 
 * ResultsHandle (input) ): The handle used to identify the query 
 *
 * Return value:
 * A success/fail indicator
 *
 * Error Codes:
 * CSSM_CL_INVALID_RESULTS_HANDLE
 *---------------------------------------------------------------------------*/
CSSM_RETURN CSSMCLI CL_CertAbortQuery (CSSM_CL_HANDLE CLHandle,
                                       CSSM_HANDLE ResultsHandle)
{
    CLHandle;

    if (!ResultsHandle) 
        return CSSM_OK;
    else {
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_INVALID_RESULTS_HANDLE);
        return CSSM_FAIL;
    }
}


/*-----------------------------------------------------------------------------
 * Name: CL_CertGetKeyInfo
 *
 * Description:
 * This function obtains the certificate's public key in a CSSM_KEY format. 
 * This CSSM_KEY contains the fields necessary to create 
 * a cryptographic context.
 *
 * NOTE: ClearError() is not necessary because it is performed by CSSM.
 *
 * Parameters: 
 * Cert (input) : The certificate from which to extract the public key.
 *
 * Return value:
 * The public key in CSSM_KEY format
 * This buffer and its encapsulated KeyData->Data will need to be freed 
 * by the calling application.
 * 
 * Error Codes:
 * CSSM_CL_INVALID_CERT_POINTER
 * CSSM_CL_UNKNOWN_FORMAT
 * CSSM_CL_MEMORY_ERROR
 *---------------------------------------------------------------------------*/
CSSM_KEY_PTR CSSMCLI CL_CertGetKeyInfo (CSSM_CL_HANDLE CLHandle,
                                        const CSSM_DATA_PTR Cert)
{
    DER_NODE_PTR cert_ptr;
    CSSM_KEY_PTR key_ptr;

    /* Check inputs */
    /* CLHandle is checked by CSSM */
    if (!Cert || !Cert->Data || !Cert->Length) 
    {
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_INVALID_CERT_POINTER);
        return NULL;
    }

    /* Obtain the parse tree representation of the Cert */
    cert_ptr = cl_DerDecodeCertificate(CLHandle, Cert);
    if (!cert_ptr)
    {
        if ((CSSM_GetError())->error != CSSM_CL_MEMORY_ERROR)
            CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_UNKNOWN_FORMAT);
        return NULL;
    }

    /* Translate the key from its SPKI format in the certificate */
    /* to a CSSM_KEY format */
    key_ptr = cl_SpkiParseTreeToCSSMKey(CLHandle, 
              cert_ptr->Child[CERT_TBSCERT].X.Node->Child[TBSCERT_SPKI].X.Node);

    /* Cleanup & Return */
    cl_FreeCertificate(CLHandle, cert_ptr);
    return key_ptr;
};


