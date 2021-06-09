/*-----------------------------------------------------------------------
 *       File:   is_bad.c
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
 * This file verifies that the parse tree input to cl_IsBadCertificateParseTree
 * corresponds to the parse tree expected for an X.509 certificate.
 */

#include "x_fndefs.h"

#define NUM_OPTIONAL_TBSCERT_FIELDS 4 /* A TbsCert has 4 optional fields,     */
                                      /* Version. issuerUID, subjectUID, extns*/
#define NUM_TBSCERT_VERSION_FIELDS  1 /* The Version sequence has one field,  */
                                      /* the version itself.                  */
#define NUM_TBSCERT_VALIDITY_FIELDS 2 /* The Validity sequence has two fields,*/
                                      /* the startDate and endDate            */
#define NUM_TBSCERT_SPKI_FIELDS     2 /* The SPKI sequence has two fields,    */
                                      /* the KPG AlgorithmId and the SPK.     */


#define TBSCERT_VERSION_LENGTH      1 /* The TbsCert Version is of length = 1 */


/*-----------------------------------------------------------------------------
 * Name: cl_IsBadDerNodeChild
 *
 * Description:
 * For the input DerNodeChild, this function checks
 * 1) whether its BER parsed item representation contains  
 *    the input universal Tag and some content
 * 2) whether the IsLeaf flag and Node pointer are consistent with the Tag
 *
 * Parameters: 
 * Child (input) : The DerNodeChild to be checked
 * Tag   (input) : The Tag value that should be in the DerNodeChild
 *
 * Return value:
 * A TRUE/FALSE indicator of whether this is a valid DerNodeChild
 * 
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_BOOL cl_IsBadDerNodeChild(struct der_node_child_struct * Child, uint8 Tag)
{
    /* Child existence is verified by the calling function */

    /* Check that the input DerNodeChild contains  */
    /* the input universal Tag and some Content    */
    if (BER_LengthOfTag(Child->X.Input.Tag) != BER_UNIVERSAL_TAG_LENGTH || 
        *Child->X.Input.Tag != Tag ||
        !Child->X.Input.ContentLength || 
        !Child->X.Input.Content)
        return CSSM_TRUE;

    /* Check that the IsLeaf flag and Node pointer are consistent with the Tag */
    if (Tag & BER_CONSTRUCTED) {               /* If this is a constructed Tag,*/
        if (Child->IsLeaf || !Child->X.Node)   /* Verify that the Node exists  */
            return CSSM_TRUE;
    } else {                                   /* Otherwise,                   */
        if (!Child->IsLeaf || Child->X.Node)   /* Verify that IsLeaf == TRUE   */
            return CSSM_TRUE;
    }

    return CSSM_FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: cl_IsBadOidValueParseTree
 *
 * Description:
 * This function checks whether this is a valid OID/value parse tree,
 * such as an AlgorithmId parse tree or an Attribute type/value parse tree
 *
 * AlgorithmIdentifier       ::= SEQUENCE {
 *     algorithm    ALGORITHM.&id ({SupportedAlgorithms}),
 *     parameters   ALGORITHM.&Type ({SupportedAlgorithms}{ @algorithm}) OPTIONAL }
 *
 * AttributeTypeAndValue     ::= SEQUENCE
 *     type         ATTRIBUTE.&id ({SupportedAttributes}),
 *     value        ({ATTRIBUTE.&Type ({SupportedAttributes}{@type})}
 *
 * Parameters: 
 * ParentNode (input) : The top node in the parse tree to be checked
 *
 * Return value:
 * A TRUE/FALSE indicator of whether this is a valid OID/value parse tree
 * 
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_BOOL cl_IsBadOidValueParseTree(DER_NODE_PTR ParentNode)
{    
    /* ParentNode existence as a SEQUENCE is checked by the calling function */

    /* Verify that it has either 1 child (the OID) or 2 children (OID+Value) */
    if (ParentNode->Count < MIN_NUM_ALGID_FIELDS ||
        ParentNode->Count > MAX_NUM_ALGID_FIELDS)
        return CSSM_TRUE;

    /* Verify that the first child is an OID */
    return cl_IsBadDerNodeChild(
              &ParentNode->Child[ALGID_ALGORITHM], BER_OBJECT_IDENTIFIER);
}


/*-----------------------------------------------------------------------------
 * Name: cl_IsBadNameParseTree
 *
 * Description:
 * This function checks whether this parse tree conforms
 * to the Name structure defined by the X.509 specification.
 *
 * Name                      ::= SEQUENCE OF RelativeDistinguishedName
 * RelativeDistinguishedName ::= SET SIZE (1..MAX) OF AttributeTypeAndValue
 * AttributeTypeAndValue     ::= SEQUENCE
 *
 * Parameters: 
 * ParentNode (input) : The top node in the parse tree to be checked
 *
 * Return value:
 * A TRUE/FALSE indicator of whether this is a valid Name parse tree
 * 
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_BOOL cl_IsBadNameParseTree(DER_NODE_PTR ParentNode)
{
    DER_NODE_PTR current_rdn;
    sint32 i,j;

    /* ParentNode existence as a SEQUENCE is checked by the calling function */

    /* Verify that each RDN is a SET */
    for (i=0; i < ParentNode->Count; i++)
    {
        if (cl_IsBadDerNodeChild(&ParentNode->Child[i], BER_CONSTRUCTED_SET))
            return CSSM_TRUE;

        /* Verify that each type/value pair is an OID/value SEQUENCE */
        current_rdn = ParentNode->Child[i].X.Node;
        for (j=0; j < current_rdn->Count; j++)
        {
            if (cl_IsBadDerNodeChild(&current_rdn->Child[j], 
                                     BER_CONSTRUCTED_SEQUENCE))
                return CSSM_TRUE;

            if (cl_IsBadOidValueParseTree(current_rdn->Child[j].X.Node))
                return CSSM_TRUE;

        } /* Loop to next type/value pair */

    } /* Loop to next RDN */

    return CSSM_FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: cl_DetermineOptionalFieldExistence
 *
 * Description:
 * In X.509 certificates, the version, issuerUID, subjectUID, and extensions
 * fields do not necessarily appear in the DER-encoded representation.
 *
 * This function fills in an array that indicates which optional fields 
 * are present.  A field that is not present will not have any values for
 * the Tag, ContentLength, Content, and Node. Otherwise, it is assumed present.
 * 
 * For required fields, this function assumes that they are present.  
 *
 * The Tag, ContentLength, Content, Node, and subtree values will be checked 
 * for all present fields as part of cl_IsBadTbsCertParseTree.
 *
 * Parameters: 
 * ChildrenArray  (input) : The TbsCert ChildArray
 * Existence     (output) : The array to be filled in with Boolean indicators
 *                          of whether or not a particular child exists
 *
 * Return value:
 * None
 * 
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
void cl_DetermineOptionalFieldExistence(
                                     struct der_node_child_struct * ChildrenArray,
                                     CSSM_BOOL *Existence)
{
    uint32 i, opt_fields[] = {TBSCERT_VERSION,
                              TBSCERT_ISSUER_UID,
                              TBSCERT_SUBJECT_UID,
                              TBSCERT_EXTENSIONS};

    /* Set all the fields to existence != FALSE */
    cssm_memset(Existence, 0xFF, NUM_TBSCERT_FIELDS * sizeof(CSSM_BOOL));

    /* Check for the absence of optional fields */
    for (i=0; i < NUM_OPTIONAL_TBSCERT_FIELDS; i++)
    {
        if (!ChildrenArray[opt_fields[i]].X.Input.Tag &&
            !ChildrenArray[opt_fields[i]].X.Input.ContentLength &&
            !ChildrenArray[opt_fields[i]].X.Input.Content &&
            !ChildrenArray[opt_fields[i]].X.Node)
            Existence[opt_fields[i]] = CSSM_FALSE;
    }
}

/*-----------------------------------------------------------------------------
 * Name: cl_IsBadTbsCertParseTree
 *
 * Description:
 * This function checks whether this parse tree conforms
 * to the tbsCert structure defined by the X.509 specification.
 *
 * tbsCert				::=	SEQUENCE {
 * version                  [0]	Version DEFAULT v1,
 * serialNumber             CertificateSerialNumber,
 * signature                AlgorithmIdentifier,
 * issuer                   Name,
 * validity                 Validity,
 * subject                  Name,
 * subjectPublicKeyInfo     SubjectPublicKeyInfo,
 * issuerUniqueIdentifier   [1]	IMPLICIT UniqueIdentifier OPTIONAL,
 *                              -- if present, version must be v2or v3
 * subjectUniqueIdentifier  [2]	IMPLICIT UniqueIdentifier OPTIONAL
 *                              -- if present, version must be v2 or v3
 * extensions               [3]	Extensions OPTIONAL
 *                              -- if present, version must be v3}
 *
 * Version                  ::= INTEGER { v1(0), v2(1), v3(2) }
 * CertificateSerialNumber  ::= INTEGER
 * AlgorithmIdentifier      ::= SEQUENCE 
 * Name                     ::= SEQUENCE OF RelativeDistinguishedName
 * Validity                 ::= SEQUENCE {
 *     notBefore        UTCTime,
 *     notAfter	        UTCTime }
 * SubjectPublicKeyInfo     ::= SEQUENCE {
 *     algorithm        AlgorithmIdentifier,
 *     subjectPublicKey BIT STRING }
 * Extensions               ::= SEQUENCE  of Extension
 *     Extension        ::=	SEQUENCE {
 *         extnId           EXTENSION.&id({Extension Set}),
 *         critical         BOOLEAN DEFAULT FALSE,
 *         extnValue        OCTET STRING 
 *
 * Parameters: 
 * ParentNode (input) : The top node in the parse tree to be checked
 *
 * Return value:
 * A TRUE/FALSE indicator of whether this is a valid TbsCert parse tree
 * 
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_BOOL cl_IsBadTbsCertParseTree (DER_NODE_PTR ParentNode)
{
                  /* The order of these tags is determined by */
                  /* the X.509 structure shown above */
    const uint8   field_tags[NUM_TBSCERT_FIELDS] = {
                  BER_CONSTRUCTED_SEQUENCE, /* Version as EXPLICIT */
                  BER_INTEGER,              /* SerialNumber */
                  BER_CONSTRUCTED_SEQUENCE, /* SigningAlg */
                  BER_CONSTRUCTED_SEQUENCE, /* IssuerName */
                  BER_CONSTRUCTED_SEQUENCE, /* Validity */
                  BER_CONSTRUCTED_SEQUENCE, /* SubjectName */
                  BER_CONSTRUCTED_SEQUENCE, /* SPKI */
                  BER_BIT_STRING,           /* IssuerUID */
                  BER_BIT_STRING,           /* SubjectUID */
                  BER_CONSTRUCTED_SEQUENCE  /* Extensions */
                  };
    CSSM_BOOL     existence[NUM_TBSCERT_FIELDS];
    DER_NODE_PTR  tree_version, tree_validity, tree_key_info,                  
                  tree_expl_extns, tree_extensions, current_extn;
    sint32        i;

    /* Verify that the parse tree has 10 children, */
    /* one for each field in the TbsCert structure */
    if (ParentNode->Count != NUM_TBSCERT_FIELDS)
        return CSSM_TRUE;

    /* Verify that the Tag is correct for each field */
    cl_DetermineOptionalFieldExistence(ParentNode->Child, existence);
    for (i=0; i < NUM_TBSCERT_FIELDS; i++)
    {
        if (existence[i])
            if (cl_IsBadDerNodeChild(&ParentNode->Child[i], field_tags[i]))
                return CSSM_TRUE;
    }

    /* Error checking on the version subtree */
    if (existence[TBSCERT_VERSION])
    {
        /* Version should have a single child, the version itself */
        tree_version = ParentNode->Child[TBSCERT_VERSION].X.Node;
        if (tree_version->Count != NUM_TBSCERT_VERSION_FIELDS) 
            return CSSM_TRUE;

        /* The Version child should be an integer of length = 1 */
        if (cl_IsBadDerNodeChild(&tree_version->Child[0], BER_INTEGER) ||
            tree_version->Child->X.Input.ContentLength != TBSCERT_VERSION_LENGTH)
            return CSSM_TRUE;
    }

    /* Error checking on the signature algorithm subtree */
    if (cl_IsBadOidValueParseTree(ParentNode->Child[TBSCERT_SIG_ALG].X.Node))
        return CSSM_TRUE;

    /* Error checking on the issuer and subject subtrees */
    if (cl_IsBadNameParseTree(ParentNode->Child[TBSCERT_ISSUER].X.Node) ||
        cl_IsBadNameParseTree(ParentNode->Child[TBSCERT_SUBJECT].X.Node))
        return CSSM_TRUE;

    /* Error checking on the validity subtree */
    /* The Validity subtree should have 2 children */
    tree_validity = ParentNode->Child[TBSCERT_VALIDITY].X.Node;
    if (tree_validity->Count != NUM_TBSCERT_VALIDITY_FIELDS)
        return CSSM_TRUE;

    /* The Validity's children should be UTCTime or GeneralizedTime */
    for (i=0; i < tree_validity->Count; i++)
    {
        if (!tree_validity->Child[i].IsLeaf ||
            BER_LengthOfTag(
                tree_validity->Child[i].X.Input.Tag) != BER_UNIVERSAL_TAG_LENGTH)
            return CSSM_TRUE;

        if (*tree_validity->Child[i].X.Input.Tag == BER_UTCTIME)
        {
            /* Verify that this is a valid UTCTime string */
            if (cl_IsBadUtc (tree_validity->Child[i].X.Input.Content, 
                             tree_validity->Child[i].X.Input.ContentLength))
                return CSSM_TRUE;
        }
        else if (*tree_validity->Child[i].X.Input.Tag == BER_GENTIME)
        {
            /* Verify that this is a valid Generalized Time string */
            if (cl_IsBadGeneralizedTime (tree_validity->Child[i].X.Input.Content,
                                  tree_validity->Child[i].X.Input.ContentLength))
                return CSSM_TRUE;
        }
        else /* This is an invalid Tag for a validity date. */
            return CSSM_TRUE;
    }

    /* Error checking on SPKI subtree */
    /* SPKI should have 2 children */
    tree_key_info = ParentNode->Child[TBSCERT_SPKI].X.Node;
    if (tree_key_info->Count != NUM_TBSCERT_SPKI_FIELDS) 
        return CSSM_TRUE;

    /* The first child should be an AlgId structure */
    if (cl_IsBadDerNodeChild(
            &tree_key_info->Child[SPKI_ALGID], BER_CONSTRUCTED_SEQUENCE) ||
        cl_IsBadOidValueParseTree(tree_key_info->Child[SPKI_ALGID].X.Node))
        return CSSM_TRUE;

    /* The second child should be a bit string */
    if (cl_IsBadDerNodeChild(&tree_key_info->Child[SPKI_SPK], BER_BIT_STRING))
        return CSSM_TRUE;

    /* Error checking on extensions subtree */
    if (existence[TBSCERT_EXTENSIONS])
    {
        /* Verify that extensions is a SEQUENCE */
        tree_expl_extns = ParentNode->Child[TBSCERT_EXTENSIONS].X.Node;
        if (tree_expl_extns->Count != 1 ||
            cl_IsBadDerNodeChild(&tree_expl_extns->Child[0], 
                                 BER_CONSTRUCTED_SEQUENCE))
                return CSSM_TRUE;

        tree_extensions = tree_expl_extns->Child[0].X.Node;

        /* For each extension */
        for (i=0; i < tree_extensions->Count; i++)
        {
            /* Verify that the extension is a SEQUENCE */
            if (cl_IsBadDerNodeChild(&tree_extensions->Child[i], 
                                     BER_CONSTRUCTED_SEQUENCE))
                return CSSM_TRUE;

            /* Verify that the extension has either 2 or 3 children.         */
            /* It will have 2 if extension->critical was FALSE because,      */
            /* in DER-encoding, values that equal their default are removed. */
            /* (extension->critical is defined as BOOLEAN DEFAULT FALSE)     */
            current_extn = tree_extensions->Child[i].X.Node;
            if (current_extn->Count != 2 && current_extn->Count != 3)
                return CSSM_TRUE;

            /* Verify that the first field (extension->Id) is an OID */
            if (cl_IsBadDerNodeChild(&current_extn->Child[0],
                                     BER_OBJECT_IDENTIFIER))
                return CSSM_TRUE;

            /* If available, verify that the second field (extension->critical) */
            /* is a BOOLEAN of length 1 */
            if (current_extn->Count == 3)
                if (cl_IsBadDerNodeChild(&current_extn->Child[1], BER_BOOLEAN) ||
                    current_extn->Child[1].X.Input.ContentLength != 1)
                    return CSSM_TRUE;

            /* Verify that the last field (extension->value) is an OCTET STRING */
            if (cl_IsBadDerNodeChild(&current_extn->Child[current_extn->Count-1],
                                     BER_OCTET_STRING))
                return CSSM_TRUE;
        }
    }
    
    return CSSM_FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: cl_IsBadCertificateParseTree
 *
 * Description:
 * This function checks whether this parse tree conforms
 * to the certificate structure defined by the X.509 specification.
 * Certificate ::= SIGNED{ tbsCert }
 *
 * Parameters: 
 * ParentNode (input) : The top node in the parse tree to be checked
 *
 * Return value:
 * A TRUE/FALSE indicator of whether this is a valid Cert parse tree
 * 
 * Error Codes:
 * None
 *---------------------------------------------------------------------------*/
CSSM_BOOL cl_IsBadCertificateParseTree(DER_NODE_PTR ParentNode)
{
    uint8  field_tags[NUM_CERT_FIELDS] = {
           BER_CONSTRUCTED_SEQUENCE, /* TbsCert */
           BER_CONSTRUCTED_SEQUENCE, /* SigningAlg */
           BER_BIT_STRING            /* Signature */
           };
    uint32 i;

    /* A certificate should be a SEQUENCE with 3 children: 
     * TbsCert, SigAlg, and signature */
    if (!ParentNode) 
        return CSSM_TRUE;
    if (BER_LengthOfTag(ParentNode->Tag) != BER_UNIVERSAL_TAG_LENGTH ||
        *ParentNode->Tag != BER_CONSTRUCTED_SEQUENCE)
        return CSSM_TRUE;
    if (ParentNode->Count != NUM_CERT_FIELDS) 
        return CSSM_TRUE;

    /* Verify that the Tag is correct for each field */
    for (i=0; i < NUM_CERT_FIELDS; i++)
    {
        if (cl_IsBadDerNodeChild(&ParentNode->Child[i], field_tags[i]))
            return CSSM_TRUE;
    }

    /* Verify that the TbsCert parse tree is valid */
    if (cl_IsBadTbsCertParseTree(ParentNode->Child[CERT_TBSCERT].X.Node)) 
        return CSSM_TRUE;

    /* Verify that the SigAlg parse tree is valid */
    if (cl_IsBadOidValueParseTree(ParentNode->Child[CERT_SIG_ALG].X.Node)) 
        return CSSM_TRUE;

    return CSSM_FALSE;
}
