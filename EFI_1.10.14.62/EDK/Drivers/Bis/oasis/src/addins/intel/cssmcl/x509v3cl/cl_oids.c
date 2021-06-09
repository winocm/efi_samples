/*-----------------------------------------------------------------------
 *       File:   cl_oids.c
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
 * This file translates input OIDs into their corresponding enum value.
 */

#include "x_fndefs.h"

/*-----------------------------------------------------------------------------
 * Name: cl_IsEqualOid
 *
 * Description:
 * This function checks for the equivalence of the 2 input OIDs
 *
 * Parameters: 
 * Oid1 (input) - The first OID
 * Oid2 (input) - The second OID
 *
 * Return value:
 * A TRUE/FALSE indicator of whether or not these 2 OIDs are equivalent
 *
 * Error Codes:
 * None.
 *---------------------------------------------------------------------------*/
CSSM_BOOL cl_IsEqualOid(CSSM_OID_PTR Oid1, CSSM_OID_PTR Oid2)
{
    if (Oid1->Length == Oid2->Length &&
        cssm_memcmp(Oid1->Data, Oid2->Data, Oid1->Length) == 0)
        return CSSM_TRUE;

    return CSSM_FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: cl_AlgorithmOidToAlgId
 *
 * Description:
 * Obtain the CSSM_ALGORITHMS value equivalent to the input OID
 *
 * Parameters: 
 * Oid (input) - The OID whose CSSM_ALGORITHMS value should be returned 
 *
 * Return value:
 * CSSM_ALGORITHMS value equivalent to the input OID
 *
 * Error Codes:
 * CSSM_CL_INVALID_DATA_POINTER
 *---------------------------------------------------------------------------*/
CSSM_ALGORITHMS cl_AlgorithmOidToAlgId(CSSM_OID_PTR Oid)
{
    if (!Oid)
    {
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_INVALID_DATA_POINTER);
        return CSSM_ALGID_NONE;
    }
    if (!Oid->Data && !Oid->Length)
    {
        return CSSM_ALGID_NONE;
    }
    if (!Oid->Data || !Oid->Length)
    {
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_INVALID_DATA_POINTER);
        return CSSM_ALGID_NONE;
    }

    if (cl_IsEqualOid(&CSSMOID_RSA, Oid)) return CSSM_ALGID_RSA;
    if (cl_IsEqualOid(&CSSMOID_DSA, Oid)) return CSSM_ALGID_DSA;
#ifndef CSSM_BIS
    if (cl_IsEqualOid(&CSSMOID_DH, Oid)) return CSSM_ALGID_DH;
    if (cl_IsEqualOid(&CSSMOID_MD2, Oid)) return CSSM_ALGID_MD2;
    if (cl_IsEqualOid(&CSSMOID_MD5, Oid)) return CSSM_ALGID_MD5;
    if (cl_IsEqualOid(&CSSMOID_SHA1, Oid)) return CSSM_ALGID_SHA1;
    if (cl_IsEqualOid(&CSSMOID_DES, Oid)) return CSSM_ALGID_DES;
    if (cl_IsEqualOid(&CSSMOID_MD5WithRSA, Oid)) return CSSM_ALGID_MD5WithRSA;
    if (cl_IsEqualOid(&CSSMOID_MD2WithRSA, Oid)) return CSSM_ALGID_MD2WithRSA;
    if (cl_IsEqualOid(&CSSMOID_SHA1WithRSA, Oid)) return CSSM_ALGID_SHA1WithRSA;
    if (cl_IsEqualOid(&CSSMOID_SHA1WithDSA, Oid)) return CSSM_ALGID_SHA1WithDSA;
#endif

    return CSSM_ALGID_CUSTOM;
}


/*-----------------------------------------------------------------------------
 * Name: cl_CertOidToInt
 *
 * Description:
 * Obtain the INTEL_X509V3_OID_AS_INT value equivalent to the input OID
 *
 * Parameters: 
 * Oid (input) - The OID whose INTEL_X509V3_OID_AS_INT value should be returned 
 *
 * Return value:
 * INTEL_X509V3_OID_AS_INT value equivalent to the input OID
 *
 * Error Codes:
 * None.
 *---------------------------------------------------------------------------*/
INTEL_X509V3_OID_AS_INT cl_CertOidToInt(CSSM_OID_PTR Oid)
{
    if (cl_IsEqualOid(Oid, &CSSMOID_X509V1SerialNumber)) 
        return OIDasINT_X509V1SerialNumber; 

    if (cl_IsEqualOid(Oid, &CSSMOID_X509V1IssuerName)) 
        return OIDasINT_X509V1IssuerName; 

    if (cl_IsEqualOid(Oid, &CSSMOID_X509V1SubjectName)) 
        return OIDasINT_X509V1SubjectName; 

    if (cl_IsEqualOid(Oid, &CSSMOID_X509V1SignatureAlgorithm)) 
        return OIDasINT_X509V1SignatureAlgorithm; 

    return OIDasINT_X509V3NoMatch;
}

