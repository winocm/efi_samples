/*-----------------------------------------------------------------------
 *      File:   x_getfld.c
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
 * This file implements the functions that retrieve fields from the certificate.
 */

#include "x_fndefs.h"

/*-----------------------------------------------------------------------------
 * Name: cl_AllocateAndCopyData
 *
 * Description:
 * This function creates a CSSM_DATA structure containing 
 * a copy of the input data.
 *
 * Parameters: 
 * Data   (input) : The data to be copied
 * Length (input) : The length of the input Data buffer
 *
 * Return value:
 * A pointer to a CSSM_DATA structure
 *
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_DATA_PTR cl_AllocateAndCopyData (CSSM_CL_HANDLE CLHandle, 
                                      const uint8* Data, uint32 Length)
{
    CSSM_DATA_PTR return_data_ptr;

    /* Allocate the CSSM_DATA structure */
    return_data_ptr = CLMemFuncs.calloc_func(CLHandle, 1, sizeof(CSSM_DATA));
    if (!return_data_ptr)
        return NULL;

    /* Allocate and Copy the data */
    if (Length)
    {
        return_data_ptr->Length = Length;
        return_data_ptr->Data = 
            CLMemFuncs.calloc_func(CLHandle, 1, return_data_ptr->Length);
        if(!return_data_ptr->Data)
        {
            CLMemFuncs.free_func(CLHandle, return_data_ptr);
            return NULL;
        }
        cssm_memcpy(return_data_ptr->Data, Data, return_data_ptr->Length);
    } 

    /* Return */
    return return_data_ptr;
}


/*-----------------------------------------------------------------------------
 * Name: cl_CertGetField
 *
 * Description:
 * This function gets the value or values of the requested certificate OID.
 * It will allocate any memory necessary to hold the output values using
 * the application's memory allocation routines.
 *
 * Parameters: 
 * Cert  (input)   : The certificate whose field(s) will be returned
 * Oid   (input)   : The OID to be retrieved.
 * NumberOfMatchedFields (output) : 0 if an error occurred, 1 otherwise
 *
 * Return value:
 * The retrieved field value
 * 
 * Error Codes:
 * CSSM_CL_UNKNOWN_TAG
 * CSSM_CL_MEMORY_ERROR
 *---------------------------------------------------------------------------*/
CSSM_DATA_PTR cl_CertGetField (CSSM_CL_HANDLE CLHandle, 
                               DER_NODE_PTR Cert, 
                               CSSM_OID_PTR Oid,
                               uint32 *NumberOfMatchedFields)
{
    CSSM_DATA_PTR return_data_ptr;
    DER_NODE_PTR  tbs_cert_ptr = Cert->Child[CERT_TBSCERT].X.Node;

    switch ( cl_CertOidToInt(Oid) ) {

    case OIDasINT_X509V1SerialNumber :
        return_data_ptr = cl_AllocateAndCopyData(CLHandle, 
            tbs_cert_ptr->Child[TBSCERT_SERIAL_NUM].X.Input.Content, 
            tbs_cert_ptr->Child[TBSCERT_SERIAL_NUM].X.Input.ContentLength);
        break;

    case OIDasINT_X509V1IssuerName :
        return_data_ptr = cl_AllocateAndCopyData(CLHandle, 
            tbs_cert_ptr->Child[TBSCERT_ISSUER].X.Input.Tag, 
            DER_SizeofTree(tbs_cert_ptr->Child[TBSCERT_ISSUER].X.Node));
        break;

    case OIDasINT_X509V1SubjectName :
        return_data_ptr = cl_AllocateAndCopyData(CLHandle, 
            tbs_cert_ptr->Child[TBSCERT_SUBJECT].X.Input.Tag, 
            DER_SizeofTree(tbs_cert_ptr->Child[TBSCERT_SUBJECT].X.Node));
        break;

    case OIDasINT_X509V1SignatureAlgorithm :
        return_data_ptr = cl_AllocateAndCopyData(CLHandle, 
            Cert->Child[CERT_SIG_ALG].X.Node->Child[ALGID_ALGORITHM].X.Input.Content, 
            Cert->Child[CERT_SIG_ALG].X.Node->Child[ALGID_ALGORITHM].X.Input.ContentLength);
        break;

    default:
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_UNKNOWN_TAG);
        return NULL;
    }

    if (return_data_ptr) 
        *NumberOfMatchedFields = 1;
    else  
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_MEMORY_ERROR);

    return return_data_ptr;
};

