/*-----------------------------------------------------------------------
 *      File:   tal_cntx.c
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
 * This file is part of the Token Adaptation Layer (TAL) source code base.
 * The TAL code makes it easier for CSP venders to develop CSPs that plug
 * into the Intel CDSA infrastructure.
 * This file contains the token adaptation layer context functions.
 */

#ifndef _TAL_CNTX_C
#define _TAL_CNTX_C

#include "cssmerr.h"
#include "cssmport.h"
#include "tal_cntx.h"
#include "tal_mem.h"
#include "tal_glob.h"
#include "tal_defs.h"

/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/*                      Context and Attribute functions                     */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*-----------------------------------------------------------------------------
 *Name: TAL_GetContextAttributeFromContext(...)
 *
 *Description:
 *	Get a attribute which matchs the requirements from the supplied context.
 *
 *Parameters:
 *  Context_ptr(input) - A pointer to a context structure to search all its
 *                    attributes.
 *AttributeType(input) - one of the CSSM_ATTRIBUTE_TYPE.
 *
 *Returns:
 *  CSSM_CONTEXT_ATTRIBUTE_PTR: The found attribute in the context.
 *  NULL:                         Can not find the required attribute 
 *                                from the context	
 *---------------------------------------------------------------------------*/
CSSM_CONTEXT_ATTRIBUTE_PTR
TAL_GetContextAttrFromContext(const CSSM_CONTEXT_PTR Context_ptr,
                              uint32 AttributeType)
{
    uint32 i;

    if (Context_ptr == NULL)
        return NULL;

    for (i=0; i<Context_ptr->NumberOfAttributes; i++)
    {
        if (Context_ptr->ContextAttributes[i].AttributeType == AttributeType)
            return &Context_ptr->ContextAttributes[i];
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
 *Name: TAL_GetAttributeFromContext(...)
 *
 *Description:
 *	Get a attribute value for the AttributeType from the supplied context.
 *
 *Parameters:
 *  Context_ptr(input) - A pointer to a context structure to search all its
 *                    attributes.
 *	AttributeType(input) - one of the CSSM_ATTRIBUTE_TYPE.
 *
 *Returns:
 *    void* - Pointer to the found attribute value in the context.
 *    NULL  - Can not find the required AttributeType from the context	
 *---------------------------------------------------------------------------*/
void* TAL_GetAttributeFromContext(const CSSM_CONTEXT_PTR Context_ptr,
                                  uint32 AttributeType)
{
    CSSM_CONTEXT_ATTRIBUTE_PTR pAttr = NULL;

    pAttr = TAL_GetContextAttrFromContext(Context_ptr, AttributeType);
    if (pAttr == NULL)
        return NULL;

    switch (pAttr->AttributeType & CSSM_ATTRIBUTE_TYPE_MASK)
    {
    case CSSM_ATTRIBUTE_DATA_UINT32:     return &pAttr->Attribute.Uint32;
    case CSSM_ATTRIBUTE_DATA_KEY:        return pAttr->Attribute.Key;
    case CSSM_ATTRIBUTE_DATA_CSSM_DATA:	 return pAttr->Attribute.Data;
    case CSSM_ATTRIBUTE_DATA_STRING:     return pAttr->Attribute.String;

/*########################### CSSM_BIS ###########################*/
#ifndef CSSM_BIS
    case CSSM_ATTRIBUTE_DATA_CRYPTO_DATA:return pAttr->Attribute.Crypto;
    case CSSM_ATTRIBUTE_DATA_DATE:       return pAttr->Attribute.Date;
    case CSSM_ATTRIBUTE_DATA_RANGE:      return pAttr->Attribute.Range; 
    case CSSM_ATTRIBUTE_DATA_VERSION:    return pAttr->Attribute.Version;
    case CSSM_ATTRIBUTE_DATA_NONE:
#endif
/*########################### CSSM_BIS ###########################*/

    default:                             break;
    }
    return NULL;
}

/*-----------------------------------------------------------------------------
 *Name: TAL_GetKeyFromContext(...)
 *
 *Description:
 *	Get the key from the supplied context, and check the key class.
 *
 *Parameters:
 *  Context_ptr(input) - A pointer to a context structure to search all 
 *                    its attributes.
 *
 *Returns:
 *  CSSM_KEY_PTR - A pointer to the private key.
 *  NULL - No private key in the context
 *---------------------------------------------------------------------------*/
CSSM_KEY_PTR TAL_GetKeyFromContext(const CSSM_CONTEXT_PTR Context_ptr,
                                   uint32 KeyClass)
{
    CSSM_CONTEXT_ATTRIBUTE_PTR	pAttr=NULL;
    CSSM_KEY_PTR                pKey=NULL;

    if ((pAttr = TAL_GetContextAttrFromContext(Context_ptr, CSSM_ATTRIBUTE_KEY))
                 == NULL)
        return NULL;
    if ((pKey = pAttr->Attribute.Key) == NULL)
        return NULL;
    if (pKey->KeyHeader.KeyClass != KeyClass) 
        return NULL;

    return pKey;
}

/*-----------------------------------------------------------------------------
 *Name: TAL_CheckContext(...)
 *
 *Description:
 *	Check the context structure and the key class for enc/dec, sign/verify,
 *	mac, and derive context
 *
 *Parameters:
 *  Context_ptr(input) - a module handle
 *  nSessionType(input) - a pointer to the source attribute
 *
 *Returns:
 *  CSSM_OK:   call is successful.
 *  CSSM_FAIL: memory error.
 *---------------------------------------------------------------------------*/
CSSM_RETURN TAL_CheckContext(const CSSM_CONTEXT_PTR Context_ptr,
                             uint32 nSessionType)
{
    CSSM_KEY_PTR                pKey = NULL;

    /* Verify the context pointer */
    if (Context_ptr == NULL)
    {
        TAL_SetError(CSSM_CSP_INVALID_CONTEXT_POINTER);
        return CSSM_FAIL;
    }
    /* Get and verify the key attribute in the context */
    if ((nSessionType == CSP_CRYPTO_ENC) ||
        (nSessionType == CSP_CRYPTO_DEC) ||
        (nSessionType == CSP_CRYPTO_SIGN) ||
        (nSessionType == CSP_CRYPTO_VERIFY) ||
        (nSessionType == CSP_CRYPTO_WRAP) ||
        (nSessionType == CSP_CRYPTO_UNWRAP) ||
        (nSessionType == CSP_CRYPTO_GEN_MAC) ||
        (nSessionType == CSP_CRYPTO_VERIFY_MAC))
    {
        CSSM_CONTEXT_ATTRIBUTE_PTR	pAttr = NULL;

        /* Get and verify the key in the context */
        if (((pAttr = TAL_GetContextAttrFromContext(
                            Context_ptr, CSSM_ATTRIBUTE_KEY)) == NULL) ||
            ((pKey = pAttr->Attribute.Key) == NULL) )
        {
            TAL_SetError(CSSM_CSP_INVALID_ATTR_KEY);
                return CSSM_FAIL;
        }
        if ((!pKey) || (!pKey->KeyData.Data))
            return CSSM_FAIL;
    }
    /* Verify the context type, key usage and key class */
    switch (nSessionType)
    {
    case CSP_CRYPTO_VERIFY:
            if (Context_ptr->ContextType != CSSM_ALGCLASS_SIGNATURE)
            {
                TAL_SetError(CSSM_CSP_INVALID_CONTEXT);
                return CSSM_FAIL;
            }
            /* Verify Key Class */
            if (pKey->KeyHeader.KeyClass != CSSM_KEYCLASS_PUBLIC_KEY)
            {
                TAL_SetError(CSSM_CSP_INVALID_KEYCLASS);
                return CSSM_FAIL;
            }
            /* Verify that it has a valid key usage */
            if ((pKey->KeyHeader.KeyUsage & CSSM_KEYUSE_ANY) == 0)
            {
                if ((pKey->KeyHeader.KeyUsage & CSSM_KEYUSE_VERIFY ) == 0)
                {
                    TAL_SetError(CSSM_CSP_KEY_USAGE_INCORRECT);
                    return CSSM_FAIL;
                }
            }
            break;

    /*########################### CSSM_BIS ###########################*/
#ifndef CSSM_BIS

    case CSP_CRYPTO_SIGN:
            if (Context_ptr->ContextType != CSSM_ALGCLASS_SIGNATURE)
            {
                TAL_SetError(CSSM_CSP_INVALID_CONTEXT);
                return CSSM_FAIL;
            }
            /* Verify Key Class */
            if (pKey->KeyHeader.KeyClass != CSSM_KEYCLASS_PRIVATE_KEY)
            {
                TAL_SetError(CSSM_CSP_INVALID_KEYCLASS);
                return CSSM_FAIL;
            }
            /* Verify that it has a valid key usage */
            if ((pKey->KeyHeader.KeyUsage & CSSM_KEYUSE_ANY) == 0)
            {
                if (TAL_ValidateKeyUsage(pKey->KeyHeader.KeyUsage, 
                    CSSM_KEYUSE_SIGN | CSSM_KEYUSE_SIGN_RECOVER) != CSSM_OK)
                    return CSSM_FAIL;
            }
            break;

    case CSP_CRYPTO_WRAP:
    case CSP_CRYPTO_ENC:
            /* Verify Context type, and Key Class */
            /*  Only verify Key BlobType for Wrap function */
            if (Context_ptr->ContextType == CSSM_ALGCLASS_ASYMMETRIC)
            {
                if ((pKey->KeyHeader.KeyClass != CSSM_KEYCLASS_PUBLIC_KEY) &&
                    (pKey->KeyHeader.KeyClass != CSSM_KEYCLASS_PRIVATE_KEY))
                {
                    TAL_SetError(CSSM_CSP_INVALID_KEYCLASS);
                    return CSSM_FAIL;
                }
            }
            else if (Context_ptr->ContextType == CSSM_ALGCLASS_SYMMETRIC)
            {
                if (pKey->KeyHeader.KeyClass != CSSM_KEYCLASS_SESSION_KEY)
                {
                    TAL_SetError(CSSM_CSP_INVALID_KEYCLASS);
                    return CSSM_FAIL;
                }
            }
            else
            {
                TAL_SetError(CSSM_CSP_INVALID_CONTEXT);
                return CSSM_FAIL;
            }
            /*Verify that it has a valid key usage */
            if ((pKey->KeyHeader.KeyUsage & CSSM_KEYUSE_ANY) == 0)
            {
                uint32 nValidUsage = (nSessionType == CSP_CRYPTO_ENC)
                                     ? CSSM_KEYUSE_ENCRYPT: CSSM_KEYUSE_WRAP;
                if (TAL_ValidateKeyUsage(pKey->KeyHeader.KeyUsage, 
                                         nValidUsage) != CSSM_OK)
                    return CSSM_FAIL;
            }
            break;

    case CSP_CRYPTO_UNWRAP:
    case CSP_CRYPTO_DEC:
            /* Verify Context type and Key Class */
            if (Context_ptr->ContextType == CSSM_ALGCLASS_ASYMMETRIC)
            {
                if ((pKey->KeyHeader.KeyClass != CSSM_KEYCLASS_PUBLIC_KEY) &&
                    (pKey->KeyHeader.KeyClass != CSSM_KEYCLASS_PRIVATE_KEY))
                {
                    TAL_SetError(CSSM_CSP_INVALID_KEYCLASS);
                    return CSSM_FAIL;
                }
            }
            else if (Context_ptr->ContextType == CSSM_ALGCLASS_SYMMETRIC)
            {
                if (pKey->KeyHeader.KeyClass != CSSM_KEYCLASS_SESSION_KEY)
                {
                    TAL_SetError(CSSM_CSP_INVALID_KEYCLASS);
                    return CSSM_FAIL;
                }
            }
            else
            {
                TAL_SetError(CSSM_CSP_INVALID_CONTEXT);
                return CSSM_FAIL;
            }
            /* Verify that it has a valid key usage */
            if ((pKey->KeyHeader.KeyUsage & CSSM_KEYUSE_ANY) == 0)
            {
                uint32 nValidUsage = (nSessionType == CSP_CRYPTO_DEC)
                                     ? CSSM_KEYUSE_DECRYPT: CSSM_KEYUSE_UNWRAP;
                if (TAL_ValidateKeyUsage(pKey->KeyHeader.KeyUsage, 
                                         nValidUsage) != CSSM_OK)
                    return CSSM_FAIL;
            }
            break;

    case CSP_CRYPTO_DERIVE:
            if (Context_ptr->ContextType != CSSM_ALGCLASS_DERIVEKEY)
            {
                TAL_SetError(CSSM_CSP_INVALID_CONTEXT);
                return CSSM_FAIL;
            }
            break;

    case CSP_CRYPTO_GEN_MAC:
    case CSP_CRYPTO_VERIFY_MAC:

#endif /*CSSM_BIS*/
/*###################### end not CSSM_BIS #########################*/

    default:if (Context_ptr->ContextType != nSessionType)
            {
                TAL_SetError(CSSM_CSP_INVALID_CONTEXT);
                return CSSM_FAIL;
            }
            break;
    }
    return CSSM_OK;
}

#endif

