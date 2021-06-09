/*-----------------------------------------------------------------------
 *       File:   cert_dec.c
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
 * This file contains routines necessary to create a parse tree for 
 * a DER-encoded X509 certificate
 */ 

#include "x_fndefs.h"

/* 
 * These are globals because they may be inserted in the certificate 
 * parse tree in place of context-specific tags and missing values
 * for the Version, SubjectUID, IssuerUID, and Extensions.
 */
const uint8 uint8_der_tag_int      = (BER_INTEGER);
const uint8 uint8_der_tag_bit_str  = (BER_BIT_STRING);
const uint8 uint8_der_tag_sequence = (BER_CONSTRUCTED | BER_SEQUENCE);


/*-----------------------------------------------------------------------------
 * Name: cl_DerDecodeCertificate
 *
 * Description:
 * This function creates a parse tree for a DER-encoded X509 certificate.
 *
 * All variables describing the the X.509 certificate parse tree have been
 * declared as 'const'.  These variables are organized in such a way as to 
 * accurately describe an X.509 certificate.  Unless the specification changes,
 * this structure will not change.
 *
 * Warnings C4221 & C4204 (nonstandard extension used) have been disabled 
 * in order to allow for static initialization of the parse tree structure.  
 * The Microsoft compiler allows for initialization of an aggregate type 
 * with the address of a local variable or a non-constant value.  
 * Other compilers may not.  In this case, static initialization of the structures 
 * takes precendence over code portability to other compilers because of the
 * size savings realized by using static initialization.
 * 
 * Parameters: 
 * Cert (input) : the certificate to be decoded
 *
 * Return value:
 * A pointer to the parse tree for this DER-encoded X509 certificate
 * 
 * Error Codes:
 * None.  The error code is set by the calling routine when this function fails.
 *---------------------------------------------------------------------------*/
#pragma warning (disable:4221 4204) 
DER_NODE_PTR cl_DerDecodeCertificate (CSSM_HANDLE CLHandle, 
                                      const CSSM_DATA_PTR Cert)
{
    /* Universal tags for the TbsCert fields */
    const uint8* tbscert_orig_tags[NUM_TBSCERT_FIELDS] =  
    {
        &uint8_der_tag_sequence, /* Version, since it's EXPLICIT, is a SEQUENCE */
        &uint8_der_tag_int,      /* SerialNumber */
        &uint8_der_tag_sequence, /* Signature algorithm */
        &uint8_der_tag_sequence, /* IssuerName */
        &uint8_der_tag_sequence, /* Validity dates */
        &uint8_der_tag_sequence, /* SubjectName */
        &uint8_der_tag_sequence, /* SPKI */
        &uint8_der_tag_bit_str,  /* IssuerUniqueIdentifier */
        &uint8_der_tag_bit_str,  /* SubjectUniqueIdentifier */
        &uint8_der_tag_sequence  /* Extensions */
    };

    /* Actual tags for the TbsCert fields */
    const uint8 ctx_spec_version     = 0xA0; /* Actual tag for version */
    const uint8 ctx_spec_issuer_uid  = 0x81; /* Actual tag for issuerUID */
    const uint8 ctx_spec_subject_uid = 0x82; /* Actual tag for subjectUID */
    const uint8 ctx_spec_extn        = 0xA3; /* Actual tag for extensions */
    const uint8* tbscert_ctx_tags[NUM_TBSCERT_FIELDS] =
    {
        &ctx_spec_version,       /* Version */
        &uint8_der_tag_int,      /* SerialNumber */
        &uint8_der_tag_sequence, /* Signature algorithm */
        &uint8_der_tag_sequence, /* IssuerName */
        &uint8_der_tag_sequence, /* Validity dates */
        &uint8_der_tag_sequence, /* SubjectName */
        &uint8_der_tag_sequence, /* SPKI */
        &ctx_spec_issuer_uid,    /* IssuerUniqueIdentifier */
        &ctx_spec_subject_uid,   /* SubjectUniqueIdentifier */
        &ctx_spec_extn           /* Extensions */
    };

    /* Fill in the structure to guide TbsCert decode */
    DER_COMPLEX_TYPE tbscert_subtype = {  
        &uint8_der_tag_sequence, /* TbsCert Tag */
        NUM_TBSCERT_FIELDS,      /* Number of TbsCert fields */
        NULL,                    /* No Variants in the TbsCert structure */
        tbscert_ctx_tags,        /* Context-specific tags */
        tbscert_orig_tags,       /* Universal tags */
        NULL,                    /* No need for defaults */
        NULL,                    /* No need for TbsCert subtypes */
        NULL                     /* No need for a debugging name */
    };

    /* Fill in the list of structures to guide Cert substructure decode */
    DER_COMPLEX_TYPE_PTR cert_subtypes[NUM_CERT_FIELDS] =
    {
        &tbscert_subtype,        /* For the TbsCert */
        NULL,                    /* For the signature algorithm */
        NULL                     /* For the signature */
    };

    /* Fill in the structure to guide Cert decode */
    DER_COMPLEX_TYPE cert_type = {
        &uint8_der_tag_sequence, /* Cert Tag */
        NUM_CERT_FIELDS,         /* Number of Cert fields */
        NULL,                    /* No Variants in the Cert structure */
        NULL,                    /* No Context-specific tags in Cert */
        NULL,                    /* No need for Universal tags */
        NULL,                    /* No field defaults in Cert */
        cert_subtypes,           /* TbsCert subtypes */
        NULL                     /* No need for a debugging name */
    };

    DER_NODE_PTR    parsed_tree; /* The returned parse tree */
    BER_PARSED_ITEM cert_object; /* The BER-parsed item format for Cert */


    /* Create the BER-parsed item format for Cert */
    if (BER_ExpandItem(Cert->Data, Cert->Length, &cert_object) == 0)
        return NULL;

    /* Create the parse tree for Cert */
    parsed_tree = BER_ParseComplexObject(CLHandle, 
                                        (BER_MEMORY_FUNCS_PTR) &CLMemFuncs, 
                                         &cert_type, 
                                         &cert_object);

    /* Verify that the parse tree for Cert corresponds to 
     * an X.509 certiifcate parse tree */
    if (cl_IsBadCertificateParseTree(parsed_tree))
    {
        DER_RecycleTree(CLHandle, 
                       (BER_MEMORY_FUNCS_PTR) &CLMemFuncs, parsed_tree);
        return NULL;
    }

    /* parsed_tree->Type is a pointer to the cert_type structure    */
    /* initialized in this function.                                */
    /* The cert_type structure is not needed outside this function, */
    /* so parsed_tree->Type has been set to NULL.                   */
    parsed_tree->Type = NULL;

    /* Return the parse tree pointer */
    return parsed_tree;
}
#pragma warning (default:4221 4204) 


/*-----------------------------------------------------------------------------
 * Name: cl_FreeCertificate
 *
 * Description:
 * This function frees the Cert parse tree
 *
 * Parameters: 
 * Cert (input) : the Cert parse tree to be freed
 *
 * Return value:
 * None
 * 
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
void cl_FreeCertificate (CSSM_HANDLE CLHandle, 
                         DER_NODE_PTR Cert)
{
    DER_RecycleTree(CLHandle, (BER_MEMORY_FUNCS_PTR) &CLMemFuncs, Cert);
}
