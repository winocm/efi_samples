/*-----------------------------------------------------------------------
 *       File:   spki_key.c
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
 * This file translates a SPKI parse tree into a CSSM_KEY data structure.
 */

#include "x_fndefs.h"

#define NUM_DSA_COMPONENTS    4 /* p, q, g, y */
#define NUM_DSA_PARAMS_FIELDS 3 /* p, q, g */
#define Y_INDEX 3 /* the y component of the DSA key goes after p, q, and g */

extern CSSM_CSP_HANDLE CL_CSPHandle;

/*-----------------------------------------------------------------------------
 * Name: cl_PackParseTree
 *
 * Description:
 * This function packs a parse tree structure into a DER encoded blob
 *
 * Parameters: 
 * CLHandle (input) : The handle used by CSSM to identify this attach instance.
 * Root     (input) : The root of the tree to be packed
 * Data    (output) : The DER encoded blob will be put in Data->Data.
 *                    Data->Data will need to be freed by the calling app.
 *
 * Return value:
 * A success/failure indicator
 *
 * Error Codes:
 * CSSM_CL_MEMORY_ERROR
 *---------------------------------------------------------------------------*/
CSSM_RETURN cl_PackParseTree (CSSM_HANDLE   CLHandle, 
                              DER_NODE_PTR  Root,
                              CSSM_DATA_PTR Data)
{
    /* Obtain the final size of the DER encoded object */
    Data->Length = DER_SizeofTree(Root);
    if (!Data->Length)
        return CSSM_FAIL;

    /* Allocate a buffer for the DER encoded object */
    Data->Data = CLMemFuncs.malloc_func(CLHandle, Data->Length);
    if (!Data->Data)
    {
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_MEMORY_ERROR);
        return CSSM_FAIL;
    }
    
    /* Create the DER encoded object from the parse tree */
    if (DER_CopyTree(Root, Data->Data) != Data->Length)
    {
        CLMemFuncs.free_func(CLHandle, Data->Data);
        return CSSM_FAIL;
    }

    return CSSM_OK;
}


/*-----------------------------------------------------------------------------
 * Name: cl_DSAKeyRejoin
 *
 * Description:
 * DSA keys are split and stored in 2 fields of a certificate:
 *   The p, q, and g components are stored in SPKI->AlgId->parameters
 *      as a sequence of 3 integers
 *   The y component is stored in SPKI->SPK bit string
        as a DER-encoded integer
 * The 4 DSA components need to be joined into a sequence of 4 integers
 *   in order to be used by CSSM CSPs.
 * This function takes the SPKI parse tree from a certificate and 
 * joins the p, q, g, and y integers into a single DER-encoded sequence.
 *
 * Parameters: 
 * CLHandle (input) : The handle used by CSSM to identify this attach instance.
 * SPKI     (input) : The SPKI parse tree from an X509 certificate
 * KeyData (output) : The newly-formed DSA Key will be put in KeyData->Data
 *                    KeyData->Data will need to be freed by the calling app.
 *
 * Return value:
 * A success/failure indicator
 * 
 * Error Codes:
 * None. The error code is set by the calling routine when this function fails.
 *---------------------------------------------------------------------------*/
CSSM_RETURN cl_DSAKeyRejoin (CSSM_CL_HANDLE CLHandle, 
                             DER_NODE_PTR   SPKI,
                             CSSM_DATA_PTR  KeyData)
{
    DER_NODE_PTR dsa_key_tree;
    struct der_node_child_struct 
                 *p_q_g_array,
                 p_q_g_y_array[NUM_DSA_COMPONENTS];
    CSSM_RETURN  rRtn;
    sint32       i;

    /* Verify that SPKI->AlgId->parameters exists */
    if (SPKI->Child[SPKI_ALGID].X.Node->Count != MAX_NUM_ALGID_FIELDS)
        return CSSM_FAIL;

    /* Verify that SPKI->AlgId->parameters is a SEQUENCE */
    if (cl_IsBadDerNodeChild(&SPKI->Child[SPKI_ALGID].X.Node->Child[ALGID_PARAMS], 
                             BER_CONSTRUCTED_SEQUENCE))
        return CSSM_FAIL;

    /* Verify that SPKI->AlgId->parameters has 3 children (p,q,g) */
    dsa_key_tree = SPKI->Child[SPKI_ALGID].X.Node->Child[ALGID_PARAMS].X.Node;
    if (dsa_key_tree->Count != NUM_DSA_PARAMS_FIELDS)
        return CSSM_FAIL;

    /* Verify that each SPKI->AlgId->parameters child is an integer */
    for (i=0; i < dsa_key_tree->Count; i++)
    {
        if (cl_IsBadDerNodeChild(&dsa_key_tree->Child[i], BER_INTEGER))
            return CSSM_FAIL;
    }

    /* Index past the first byte of the SPKI->SPK bit string and         */
    /* expand the DER-encoded object stored at that location             */
    /* into its BER-parsed-item components.                              */
    /* Store the BER-parsed-item into the y-index of the p_q_g_y_array   */
    BER_ExpandItem(SPKI->Child[SPKI_SPK].X.Input.Content+1, 
                   SPKI->Child[SPKI_SPK].X.Input.ContentLength-1, 
                   &p_q_g_y_array[Y_INDEX].X.Input);

    /* Verify that the DER-encoded object in the SPKI->SPK is an integer */
    /* If so, we've found the y-component of the DSA key and placed it   */
    /* into the correct position in the p_q_g_y_array                    */
    p_q_g_y_array[Y_INDEX].IsLeaf = CSSM_TRUE;
    p_q_g_y_array[Y_INDEX].X.Node = NULL;
    if (cl_IsBadDerNodeChild(&p_q_g_y_array[Y_INDEX], BER_INTEGER))
        return CSSM_FAIL;

    /* Copy the p, q, and g integers into the p_q_g_y_array */
    p_q_g_array = dsa_key_tree->Child;
    cssm_memcpy(p_q_g_y_array, 
                p_q_g_array, 
                NUM_DSA_PARAMS_FIELDS * sizeof(struct der_node_child_struct));

    /* Modify the SPKI->AlgId->parameters parse tree so that it contains    */
    /* the p, q, g, and y integers instead of just the p, q, and g integers */
    dsa_key_tree->Child = p_q_g_y_array;
    dsa_key_tree->Count = NUM_DSA_COMPONENTS;

    /* Pack the modified SPKI->AlgId->parameters parse tree to obtain       */
    /* a DER-encoded SEQUENCE of the p, q, g, and y integers                */
    rRtn = cl_PackParseTree(CLHandle, dsa_key_tree, KeyData);

    /* Restore the SPKI->AlgId->parameters parse tree to its original state */
    dsa_key_tree->Count = NUM_DSA_PARAMS_FIELDS;
    dsa_key_tree->Child = p_q_g_array;

    /* Return */
    return rRtn;
}



/*-----------------------------------------------------------------------------
 * Name: cl_SpkiParseTreeToCSSMKey
 *
 * Description:
 * This function translates the input SPKI parse tree into a CSSM_KEY structure
 *
 * Parameters: 
 * CLHandle (input) : The handle used by CSSM to identify this attach instance.
 * SPKI     (input) : The SPKI parse tree to be translated
 *
 * Return value:
 * A CSSM_KEY representation of the SPKI parse tree
 * This buffer and its encapsulated KeyData->Data will need to be freed 
 * by the calling application.
 * 
 * Error Codes:
 * CSSM_CL_MEMORY_ERROR
 * CSSM_CL_UNKNOWN_KEY_FORMAT
 *---------------------------------------------------------------------------*/
CSSM_KEY_PTR cl_SpkiParseTreeToCSSMKey (CSSM_HANDLE CLHandle, 
                                        DER_NODE_PTR SPKI)
{
    CSSM_KEY_PTR  key_ptr;
    CSSM_DATA     kpg_alg;
    CSSM_KEY_SIZE key_size_struct = {0,0};

    /* Allocate the new CSSM_KEY structure */
    key_ptr = CLMemFuncs.calloc_func(CLHandle, 1, sizeof(CSSM_KEY));
    if (!key_ptr)
    {
        CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_MEMORY_ERROR);
        return NULL;
    }
#pragma warning (disable:4213)
    /* The SPKI structure was verified as part of certificate decode */
    /* The key pair generation algorithm OID is being placed into a  */
    /* CSSM_DATA structure so that it can be translated into a       */
    /* CSSM_ALGORITHMS value                                         */
    kpg_alg.Length = SPKI->Child[SPKI_ALGID].X.Node->Child[ALGID_ALGORITHM].X.Input.ContentLength;
    (const unsigned char*)kpg_alg.Data   = SPKI->Child[SPKI_ALGID].X.Node->Child[ALGID_ALGORITHM].X.Input.Content;
#pragma warning (default:4213)

    /*                              */
    /* Fill in the KeyHeader fields */
    /*                              */
    key_ptr->KeyHeader.HeaderVersion = CSSM_KEYHEADER_VERSION;
    /* key_ptr->KeyHeader.CspId is unknown (remains 0) */
    key_ptr->KeyHeader.BlobType = CSSM_KEYBLOB_RAW_BERDER;
    key_ptr->KeyHeader.AlgorithmId = cl_AlgorithmOidToAlgId(&kpg_alg);
#ifdef CSSM_BIS
    if (key_ptr->KeyHeader.AlgorithmId != CSSM_ALGID_DSA)
    {
        CLMemFuncs.free_func(CLHandle, key_ptr);
        return NULL;
    }
#endif
    switch ( key_ptr->KeyHeader.AlgorithmId ) {
    case CSSM_ALGID_DSA :
        key_ptr->KeyHeader.Format = CSSM_KEYBLOB_RAW_FORMAT_FIPS186;
        break;
    case CSSM_ALGID_RSA :
        key_ptr->KeyHeader.Format = CSSM_KEYBLOB_RAW_FORMAT_PKCS1;
        break;
    default:
        key_ptr->KeyHeader.Format = CSSM_KEYBLOB_RAW_FORMAT_OTHER;
    }
    key_ptr->KeyHeader.KeyClass = CSSM_KEYCLASS_PUBLIC_KEY;
    /* key_ptr->KeyHeader.KeyAttr remains 0 */
    /* key_ptr->KeyHeader.KeyUsage is unknown  (set to any usage) */
    key_ptr->KeyHeader.KeyUsage = CSSM_KEYUSE_ANY;
    /* key_ptr->KeyHeader.StartDate is unknown  (remains 0) */
    /* key_ptr->KeyHeader.EndDate is unknown  (remains 0) */
    key_ptr->KeyHeader.WrapAlgorithmId = CSSM_ALGID_NONE;
    key_ptr->KeyHeader.WrapMode = CSSM_ALGMODE_NONE;

    /*                                                  */
    /* Fill in the KeyData based on the SPKI parse tree */
    /*                                                  */
#ifdef CSSM_BIS
    if (cl_DSAKeyRejoin(CLHandle, SPKI, &key_ptr->KeyData) != CSSM_OK)
#else
    if (cl_GetKeyFromSpki(CLHandle, key_ptr->KeyHeader.AlgorithmId,
                          SPKI, &key_ptr->KeyData) != CSSM_OK)
#endif
    {
        CLMemFuncs.free_func(CLHandle, key_ptr);
        if (CSSM_GetError()->error != CSSM_CL_MEMORY_ERROR)
            CSSM_SetError(&intel_preos_clm_guid, CSSM_CL_UNKNOWN_KEY_FORMAT);
        return NULL;
    }

    /*                                                     */
    /* Fill in the effective key size based on the KeyData */
    /*                                                     */
    if (CSSM_QueryKeySizeInBits (CL_CSPHandle, 0, key_ptr, &key_size_struct) 
                                                                   == CSSM_OK)
    {
        key_ptr->KeyHeader.EffectiveKeySizeInBits = 
                 key_size_struct.EffectiveKeySizeInBits;
        CSSM_ClearError(); /* Remove any errors set when obtaining EffKeySize */
    } else {
        CLMemFuncs.free_func(CLHandle, key_ptr->KeyData.Data);
        CLMemFuncs.free_func(CLHandle, key_ptr);
        return NULL;
    }

    /* Return */
    return key_ptr;
}
