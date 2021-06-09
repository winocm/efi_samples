/*-----------------------------------------------------------------------------
 *      File:   context.c
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *-----------------------------------------------------------------------------
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
 * This file contains the internal functions for managing the crypto context
 */
/*
 * The CSSM_BIS implementation of this code was inspected 8/19/98.
 */

#include "internal.h"
#include "context.h"

cssm_CONTEXT_NODE_PTR ContextHead = NULL;

/*---------------------------------------------------------------
 *Name: cssm_IsBadCryptoDataPtr
 *
 *Description:
 *  Determines whether this is a valid CryptoData structure.
 *  A valid CryptoData structure must contain either a Callback
 *  function pointer or a Param structure.
 *
 *Parameters: 
 *  Data (input) - CryptoData structure to be checked
 *
 *Returns:
 *  CSSM_FALSE - valid crypto data structure
 *  CSSM_TRUE  - invalid crypto data structure
 *
 *-------------------------------------------------------------*/
CSSM_BOOL cssm_IsBadCryptoDataPtr (CSSM_CRYPTO_DATA_PTR Data)
{
    if (!Data)
        return CSSM_TRUE;

    if (!Data->Callback) {
        if (!Data->Param)
            return CSSM_TRUE;

        if (!Data->Param->Length || !Data->Param->Data)
            return CSSM_TRUE;
    }

    return CSSM_FALSE;
}


/*---------------------------------------------------------------
 *Name: cssm_CopyDataStruct
 *
 *Description:
 *   Create a duplicate CSSM_DATA structure 
 *
 *Parameters: 
 *   Data (input) - pointer to the CSSM_DATA structure to copy
 *
 *Returns:
 *   NULL - unable to allocate sufficient memory
 *   non NULL - pointer to the new CSSM_DATA structure
 *
 *-------------------------------------------------------------*/
CSSM_DATA_PTR cssm_CopyDataStruct (CSSM_DATA_PTR Data)
{
    CSSM_DATA_PTR Dest;

    if (!Data) return NULL;

    /* Allocate the CSSM_DATA struct */
    if ((Dest = cssm_malloc (sizeof (CSSM_DATA), 0)) == NULL)
        return NULL;

    /* Allocate the actual data pointer */
    if ((Dest->Data = cssm_malloc (Data->Length, 0)) == NULL) {
        cssm_free (Dest, 0);
        return NULL;
    }

    /* Copy the data */
    Dest->Length = Data->Length;
    cssm_memcpy(Dest->Data, Data->Data, Data->Length);

    return Dest;
}


/*---------------------------------------------------------------
 *Name: cssm_CreateContext
 *
 *Description:
 *   Create and initialize a context structure.
 *   Initialization includes allocating the attributes array 
 *   and adding the CSPHandle attribute.
 *
 *Parameters: 
 *   CSPHandle (input)   - the CSP that will use this context
 *   Class (input)       - class of context
 *   AlgorithmID (input) - algorithm for the crypto operation
 *   NumberAttributes (input) - the number of attributes to allocate.
 *       An additional attribute is allocated for the CSPHandle.
 *
 *Returns:
 *   NULL - unable to create the context
 *   non NULL - pointer to the new context
 *
 *-------------------------------------------------------------*/
CSSM_CONTEXT_PTR cssm_CreateContext (CSSM_CSP_HANDLE CSPHandle,
                                     CSSM_CONTEXT_TYPE Class, 
                                     uint32 AlgorithmID,
                                     uint32 NumberAttributes)
{
    CSSM_CONTEXT_PTR Context;

    /* CheckInit & ClearError are done in cssm_GetModuleRecord */

    /* Make sure that the CSP handle is valid */
    if (cssm_GetModuleRecord (CSPHandle, 0, NULL) == NULL) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_CSP_HANDLE);
        return NULL;
    }

    /* Allocate memory for the structure */
    if ((Context = cssm_calloc (1, sizeof (CSSM_CONTEXT), 0)) == NULL) {
        CSSM_SetError (&CssmGUID, CSSM_MEMORY_ERROR);
        return NULL;
    }

    /* Initialize the context structure */
    Context->ContextType = Class;
    Context->AlgorithmType = AlgorithmID;
    Context->CSPHandle = CSPHandle;
    Context->NumberOfAttributes = NumberAttributes+1; /* +1 for the CSPHandle */
    Context->ContextAttributes = cssm_calloc(NumberAttributes +1,
                                             sizeof(CSSM_CONTEXT_ATTRIBUTE), 0);
    if (!Context->ContextAttributes) {
        cssm_free (Context, 0);
        CSSM_SetError (&CssmGUID, CSSM_MEMORY_ERROR);
        return NULL;
    }

    /* Add the CSPHandle attribute */
    /* There are no failure cases for CSSM_ATTRIBUTE_CSP_HANDLE AddAttribute */
    cssm_AddAttribute (&Context->ContextAttributes[NumberAttributes], 
                       CSSM_ATTRIBUTE_CSP_HANDLE, (void*) CSPHandle);

    return Context;
}


/*---------------------------------------------------------------
 *Name: cssm_InsertContext
 *
 *Description:
 *   Insert the context into the list of context nodes
 *
 *Parameters: 
 *   Context (input) - pointer to the context to be inserted
 *
 *Returns:
 *   CSSM_INVALID_HANDLE - memory could not be allocated
 *   non 0 - crypto handle associated with this context
 *
 *-------------------------------------------------------------*/
CSSM_CC_HANDLE cssm_InsertContext (CSSM_CONTEXT_PTR Context)
{
    cssm_CONTEXT_NODE_PTR TempContext;

    /* Allocate the context node */
    TempContext = cssm_malloc(sizeof (cssm_CONTEXT_NODE), 0);
    if (!TempContext) {
        CSSM_SetError (&CssmGUID, CSSM_MEMORY_ERROR);
        return CSSM_INVALID_HANDLE;
    }
    
    /* Initialize the context node */
    TempContext->ContextHandle = cssm_GetHandle();
    TempContext->Context = Context;

    /* Add this node to the list of context nodes */
    TempContext->Next = ContextHead;
    ContextHead = TempContext;

#ifndef CSSM_BIS
    {
    cssm_ATTACHED_MODULE_NODE_PTR ModuleInfo;

    /* Callback to csp to notify of context creation */
    ModuleInfo = cssm_GetModuleRecord (TempContext->Context->CSPHandle, 0, NULL);
    if (ModuleInfo && ModuleInfo->AddInJT)
        if (ModuleInfo->AddInJT->EventNotify)
            ModuleInfo->AddInJT->EventNotify (TempContext->Context->CSPHandle, 
                                              CSSM_EVENT_CREATE_CONTEXT,
                                              TempContext->ContextHandle);
    }
#endif

    return TempContext->ContextHandle;
}


/*---------------------------------------------------------------
 *Name: cssm_AddAttribute
 *
 *Description:
 *   Initialize the input attribute with a copy of the input data
 *
 *Parameters: 
 *   Attribute (input/output) - pointer to attribute to be initialized
 *   Type (input) - type of attribute data
 *   Data (input) - pointer to attribute data
 *
 *Returns:
 *   CSSM_FAIL - sufficient memory could not be allocated
 *   CSSM_OK - Attribute was successfully initialized
 *
 *-------------------------------------------------------------*/
CSSM_RETURN cssm_AddAttribute (CSSM_CONTEXT_ATTRIBUTE_PTR Attribute,
                               CSSM_ATTRIBUTE_TYPE Type,
                               void *Data)
{
    Attribute->AttributeType = Type;

    switch (Type & CSSM_ATTRIBUTE_TYPE_MASK) {
        case CSSM_ATTRIBUTE_DATA_UINT32 :{
            Attribute->AttributeLength = sizeof(uint32);
            // The following line generates a warning when compiled as
            // 64-bit code, on the grounds that the 64-bit "Data" gets
            // truncated down to 32 bits.  This is benign in this case
            // since we know the data is actually 32 bits stuffed into
            // a 64-bit variable.
            Attribute->Attribute.Uint32 = (uint32) Data;
        }
        break;

#ifndef CSSM_BIS
        case CSSM_ATTRIBUTE_DATA_CSSM_DATA : {
            Attribute->AttributeLength = sizeof(CSSM_DATA);
            if ((Attribute->Attribute.Data = cssm_CopyDataStruct(Data)) == NULL)
                goto ADD_ATTR_FAIL;
        }
        break;
#endif /* #ifndef CSSM_BIS */

        case CSSM_ATTRIBUTE_DATA_CRYPTO_DATA : {
            CSSM_CRYPTO_DATA_PTR Source = Data;
            CSSM_CRYPTO_DATA_PTR Dest;

            Attribute->AttributeLength = sizeof(CSSM_CRYPTO_DATA); 

            if ((Dest = cssm_malloc(Attribute->AttributeLength, 0)) == NULL)
                goto ADD_ATTR_FAIL;

            *Dest = *Source;

            if (Source->Param) {
                if ((Dest->Param = cssm_CopyDataStruct(Source->Param)) == NULL) {
                    cssm_free (Dest, 0);
                    goto ADD_ATTR_FAIL;
                }
            }

            Attribute->Attribute.Crypto = Dest;
        }
        break;

        case CSSM_ATTRIBUTE_DATA_KEY: {
            CSSM_KEY_PTR Source = Data;
            CSSM_KEY_PTR Dest;

            Attribute->AttributeLength = sizeof(CSSM_KEY);

            if ((Dest = cssm_malloc (Attribute->AttributeLength, 0)) == NULL)
                goto ADD_ATTR_FAIL;

            *Dest = *Source;

            Dest->KeyData.Data = cssm_malloc(Source->KeyData.Length, 0);
            if (!Dest->KeyData.Data) {
                cssm_free (Dest, 0);
                goto ADD_ATTR_FAIL;
            }
            cssm_memcpy (Dest->KeyData.Data, 
                         Source->KeyData.Data, 
                         Source->KeyData.Length);

            Attribute->Attribute.Key = Dest;                
        }
        break;

#ifndef CSSM_BIS
        case CSSM_ATTRIBUTE_DATA_STRING : {
            char *Source = Data;
            char *Dest;

            Attribute->AttributeLength = cssm_strlen(Source);
            
            if ((Dest = cssm_malloc(Attribute->AttributeLength+1, 0)) == NULL)
                goto ADD_ATTR_FAIL;
            cssm_strcpy (Dest, Source);

            Attribute->Attribute.String = Dest;
        }
        break;

        case CSSM_ATTRIBUTE_DATA_DATE: {
            CSSM_DATE_PTR Source = Data;
            CSSM_DATE_PTR Dest;

            Attribute->AttributeLength = sizeof(CSSM_DATE);

            if ((Dest = cssm_malloc (Attribute->AttributeLength, 0)) == NULL)
                goto ADD_ATTR_FAIL;
            cssm_memcpy(Dest, Source, Attribute->AttributeLength);

            Attribute->Attribute.Date = Dest;
        }
        break;

        case CSSM_ATTRIBUTE_DATA_RANGE: {
            CSSM_RANGE_PTR Source = Data;
            CSSM_RANGE_PTR Dest;

            Attribute->AttributeLength = sizeof(CSSM_RANGE);

            if (!Source)
            {
                CSSM_SetError(&CssmGUID, CSSM_INVALID_ATTRIBUTE);
                return CSSM_FAIL;
            }

            if ((Dest = cssm_malloc (Attribute->AttributeLength, 0)) == NULL)
                goto ADD_ATTR_FAIL;
            *Dest = *Source;

            Attribute->Attribute.Range = Dest;
        }
        break;

        case CSSM_ATTRIBUTE_DATA_VERSION: {
            CSSM_VERSION_PTR Source = Data;
            CSSM_VERSION_PTR Dest;

            Attribute->AttributeLength = sizeof(CSSM_VERSION);

            if (!Source)
            {
                CSSM_SetError(&CssmGUID, CSSM_INVALID_ATTRIBUTE);
                return CSSM_FAIL;
            }

            if ((Dest = cssm_malloc (Attribute->AttributeLength, 0)) == NULL)
                goto ADD_ATTR_FAIL;
            *Dest = *Source;

            Attribute->Attribute.Version = Dest;
        }
        break;
#endif /* #ifndef CSSM_BIS */
    }

    return CSSM_OK;

ADD_ATTR_FAIL:
    Attribute->AttributeType = 0;
    Attribute->AttributeLength = 0;
    CSSM_SetError(&CssmGUID, CSSM_MEMORY_ERROR);
    return CSSM_FAIL;
}


/*---------------------------------------------------------------
 *Name: cssm_FreeContext
 *
 *Description:
 *   Free the memory that was allocated for this context
 *
 *Parameters: 
 *   Context (input) - the context to be freed
 *
 *Returns:
 *   None
 *
 *-------------------------------------------------------------*/
void cssm_FreeContext (CSSM_CONTEXT_PTR Context)
{
    CSSM_CONTEXT_ATTRIBUTE_PTR Attributes = Context->ContextAttributes;
    uint32 i;

    /* For each attribute */
    for (i=0; i < Context->NumberOfAttributes; i++, Attributes++) 
    {
        /* If there is something to free */
        if ((Attributes->AttributeType & CSSM_ATTRIBUTE_TYPE_MASK) != 
                                                CSSM_ATTRIBUTE_DATA_UINT32)
        {
            /* Free any buffers inside of the attribute */
            switch (Attributes->AttributeType & CSSM_ATTRIBUTE_TYPE_MASK)
            {
                case CSSM_ATTRIBUTE_DATA_CRYPTO_DATA: 
                    if (Attributes->Attribute.Crypto->Param) 
                    {
                        cssm_free(Attributes->Attribute.Crypto->Param->Data, 0);
                        cssm_free(Attributes->Attribute.Crypto->Param, 0);
                    }
                    break;

                case CSSM_ATTRIBUTE_DATA_KEY : 
                    cssm_free(Attributes->Attribute.Key->KeyData.Data, 0);
                    break;

#ifndef CSSM_BIS
                case CSSM_ATTRIBUTE_DATA_CSSM_DATA:
                    cssm_free(Attributes->Attribute.Data->Data, 0);
                    break;
#endif
            }

            /* Free the attribute */
            cssm_free(Attributes->Attribute.Data, 0);

        } /* endif */
    } /* endfor */

    /* Free the attributes array and context */
    cssm_free (Context->ContextAttributes, 0);
    cssm_free (Context, 0);
}


/*---------------------------------------------------------------
 *Name: cssm_GetContext
 *
 *Description:
 *  Given a context handle, find the context 
 *
 *Parameters: 
 *  CCHandle (input) - the context handle
 *
 *Returns:
 *  NULL - unable to get context
 *  non NULL - pointer to context
 *
 *-------------------------------------------------------------*/
CSSM_CONTEXT_PTR cssm_GetContext (CSSM_CC_HANDLE CCHandle)
{
    cssm_CONTEXT_NODE_PTR TempContext;

    /* Make sure that app has done a CSSM_Init */
    if (cssm_CheckInit () == CSSM_FAIL)
        return NULL;

    /* Clear the error */
    CSSM_ClearError ();

    /* Find the handle in the context list */
    TempContext = ContextHead;
    while (TempContext) {
        if (TempContext->ContextHandle == CCHandle)
            break;
        TempContext = TempContext->Next;
    }

    /* If the handle wasn't found, return an error */
    if (!TempContext) {
        CSSM_SetError (&CssmGUID, CSSM_INVALID_CONTEXT_HANDLE);
        return NULL;
    }

    /* Return */
    return TempContext->Context;
}


