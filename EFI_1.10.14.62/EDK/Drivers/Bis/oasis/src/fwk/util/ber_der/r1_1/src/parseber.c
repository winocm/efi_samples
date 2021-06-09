/*-----------------------------------------------------------------------
 *      File:   parseber.c
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
 * DISCLAIMER: CODE QUALITY AND DOCUMENTATION
 * Basic Encoding Rules and Distinguished Encoding Rules (BER/DER) routines.
 * This is alpha-quality code and modifications should be expected for the
 * next release.  This code is provided by Intel "as is", and Intel makes no
 * warranties, express, implied or statutory, and expressly disclaims any
 * warranties of merchantability, noninfringement of intellectual property
 * rights, and fitness for a particular purpose.
 */

#include "ber_der.h"
#include "ber_internal.h"

#define AUTO_BER_ITEMS 30 /* Expected maximum number of children */

/*------------------------------------------------------------------------------
 * Name: BER_ParseComplexObject
 *
 * Description:
 * This function allocates and fills in a parse tree that describes the
 * input BER-encoded structure.  The input structure must be constructed
 * ort the function will fail.
 *
 * Parameters: 
 * AppHandle        (input) - The parameter to the memory function.
 * MemoryFunctions  (input) - Memory functions for this library to use 
 *                            when allocating data structures.
 * Type             (input) - A template for the data structure to be parsed,
 *                            or NULL if an untyped parse is acceptable.
 * Item             (input) - The tag, length, and content of the BER object 
 *                            to be parsed. 
 *
 * Returns: 
 * A parse tree of DER_NODE objects is returned. 
 * If an error occurs, a partial tree may be returned.
 * A correct parse is indicated by ??.
 * Checking of the structure by the application may be used to detect errors.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
DER_NODE_PTR 
BER_ParseComplexObject(         /* return parse tree */
    BER_HANDLE AppHandle,
    BER_MEMORY_FUNCS_PTR MemoryFunctions, 
    DER_COMPLEX_TYPE_PTR Type,  /* template for parsing object */
    BER_PARSED_ITEM_PTR  Item)  /* BER object broken into tag, length, content */
{
    DER_NODE        *node;      /* Node returned to the caller */
    BER_PARSED_ITEM *items,     /* Array of child info */
                    autoItems[AUTO_BER_ITEMS]; 
                                /* If numberOfChildren < AUTO_BER_ITEMS,
                                   this array is used to store child info 
                                   rather than performing a memory allocation */
    sint32          nItems;     /* Number of children */
    sint32          count;      /* Number of children expanded into items */
    DER_COMPLEX_TYPE *childType;/* Template for child being processed */
    sint32 i, j;

    /* This function only parses constructed objects. */
    /* Fail if the input is not constructed           */
    if ( !(Item->Tag[0] & BER_CONSTRUCTED) )
        return 0;

    count = 0;

    /* 
     * Obtain the BER_PARSED_ITEM representation of the object's children 
     */

    /* If a template has been provided and this object is a SET... */
    if (Type && Type->TypeTag && 
        (Type->TypeTag[0] & ALL_TAG_VALUES_MASK) == BER_SET) {

        /* Obtain the number of children in the SET */
        nItems = BER_CountItems(Item->Content, Item->ContentLength) + 
                 Type->NumberOfDefaults;

        /* If necessary, allocate space to hold info about the children */
        if (nItems > AUTO_BER_ITEMS) 
            items = MemoryFunctions->malloc_func(AppHandle, 
                          nItems * sizeof(BER_PARSED_ITEM));
        else 
            items = autoItems;

        /* Expand the SET based on the template */
        count = BER_ExpandSet(
            Item->Content,          /* BER-encoded object */
            Item->ContentLength,    /* BER-encoded object length */
            nItems,                 /* Num Children in encoded object */
            Type->NumberOfDefaults, /* Num Children in decoded object */
            Type->Defaults, /* Values for children that might be missing */
            items);         /* Output array of child info */

        /* Continue even if we were not able to parse the SET */

    /* If a template has been provided and this object is a SEQUENCE... */
    } else if (Type && Type->TypeTag && 
               (Type->TypeTag[0] & ALL_TAG_VALUES_MASK) == BER_SEQUENCE) {

        /* Obtain the number of children in the SEQUENCE */
        nItems = Type->NumberOfDefaults;

        /* If necessary, allocate space to hold info about the children */
        if (nItems > AUTO_BER_ITEMS) 
            items = MemoryFunctions->malloc_func(AppHandle, 
                          nItems * sizeof(BER_PARSED_ITEM));
        else 
            items = autoItems;

        /* Expand the SEQUENCE based on the template */
        count = BER_ExpandSequence(
            Item->Content,          /* BER-encoded object */
            Item->ContentLength,    /* BER-encoded object length */
            Type->NumberOfDefaults, /* Num Children in decoded object */
            Type->Choices,          /* Values for variant children */
            Type->ContextTags,      /* Context-specific tag values */
            Type->OriginalTags,     /* Universal tag values */
            Type->Defaults,         /* Default BER_PARSED_ITEM values */
            items);                 /* Output array of child info */

        /* Fail if we were not able to parse the SEQUENCE */
        if (count < Type->NumberOfDefaults) {
            if (nItems > AUTO_BER_ITEMS) 
                MemoryFunctions->free_func(AppHandle, items);
            return 0;
        }

    /* If a template has not been provided... */
    } else {    
        /* Obtain the number of children */
        nItems = BER_CountItems(Item->Content, Item->ContentLength);

        /* If necessary, allocate space to hold info about the children */
        if (nItems > AUTO_BER_ITEMS) 
            items = MemoryFunctions->malloc_func(AppHandle, 
                          nItems * sizeof(BER_PARSED_ITEM));
        else 
            items = autoItems;

        /* Expand this un-typed object as if it were a SET */
        count = BER_ExpandSet(
            Item->Content,           /* BER-encoded object */
            Item->ContentLength,     /* BER-encoded object length */
            nItems,                  /* Num Children in encoded object */
            0,                       /* No known defaults */
            (BER_PARSED_ITEM_PTR) 0, /* No known defaults */
            items);                  /* Output array of child info */

        /* Continue even if we were not able to fully parse the object */

    } /* Done obtaining child info */


    /* Allocate the proper size node, */
    /* now that we know how many children it has */
    node = DER_AllocateNode(AppHandle, MemoryFunctions, count);
    if (!node) {
        if (nItems > AUTO_BER_ITEMS) 
            MemoryFunctions->free_func(AppHandle, items);
        return 0;
    }

    /* Initialize the top-level node based on the inputs */
    node->Tag = Item->Tag;
    node->OriginalTag = Item->Tag;
    /* node->Count was filled in by DER_AllocateNode */
    node->Length = Item->ContentLength;
    node->Type = Type;

    /* For all children, fill in the BER_PARSED_ITEM that was obtained above */
    /* For all constructed children, also obtain their parse trees */
    for (i = 0; i < node->Count; i++) {

        /* Determine whether or not this child is constructed */
        /* It is not constructed 
           - if the child is missing or 
           - if the child does not have the constructed bit (6) set */
        node->Child[i].IsLeaf = (uint8) 
            (items[i].Tag == 0 || (items[i].Tag[0] & BER_CONSTRUCTED) == 0);

        /* Copy the Child info obtained above into the output structure */
        node->Child[i].X.Input = items[i];
        node->Child[i].X.Node = 0;

        /* If the Child is constructed, find its template, if available */
        if (!node->Child[i].IsLeaf) {
            childType = 0;

            /* If this child is a variant, */
            /* get its subtype from the choices structure */
            if (Type && Type->Choices && Type->Choices[i]) {

                DER_EXPLICIT_CHOICES_PTR choice;
                sint32 parent;
                choice = Type->Choices[i];
                parent = Type->Choices[i]->ParentField;

                /* Find out which variant matches the current child */
                for (j=0; j < choice->NumberOfChoices; j++) {

                    /* If the current variant Tag does not match */
                    /* the parent variant tag, keep looking */
                    if (BER_TagCompare(choice->Oids[j]->Tag, 
                           node->Child[parent].X.Input.Tag))
                        continue;

                    /* We have found a matching variant Tag. */
                    /* Now we need to verify that it is the correct match. */
                    /* If the parent field is the same as the current field,
                       or if the content is the same as the parent content,
                       then we have found the match */
                    /* Otherwise, keep looking */
                    if (parent != i) {	 
                        if (choice->Oids[j]->ContentLength != 
                            node->Child[parent].X.Input.ContentLength)
                            continue;
                        if (cssm_memcmp(choice->Oids[j]->Content, 
                            node->Child[parent].X.Input.Content, 
                            choice->Oids[j]->ContentLength)) continue;
                    }

                    /* We have found the correct variant. */
                    /* Set the template and stop looking. */
                    childType = choice->Subtypes[j];
                    break;
                }
                
            /* If this child is not a variant, */
            /* get its template from the input Type structure */
            } else if (Type && Type->Subtypes) {

                /* If this child is a SET and there are context-specific tags, */
                /* find the subtype with the matching Context-specific Tag.    */
                if ((Type->TypeTag[0] & ALL_TAG_VALUES_MASK) == BER_SET &&
                    Type->ContextTags) {
                    for (j = 0; j < Type->NumberOfDefaults; j++) {
                        if (!BER_TagCompare(Type->ContextTags[j], 
                                            node->Child[i].X.Input.Tag)) {
                            childType = Type->Subtypes[j];
                            break;
                        }
                    }
                    /* Added 11/3/97 MEP */
                    /* If we didn't find a matching Context-specific tag, */
                    /* use the current subtype */
                    if (j == Type->NumberOfDefaults)  {   
                        childType = Type->Subtypes[i];
                    }
                } else {
                    /* If this child is anything else except a SET, */
                    /* use the current subtype as its subtype */
                    childType = Type->Subtypes[i];
                }
            } /* Child type template identified */

            /* Obtain this child's parse tree */
            node->Child[i].X.Node = BER_ParseComplexObject(
                AppHandle, MemoryFunctions, /* Functions to alloc tree */
                childType,                  /* Template for this child */
                &items[i]);          /* BER_PARSED_ITEM for this child */

        } /* End if (!node->Child[i].IsLeaf) - Child's parse tree obtained */

    } /* End for (i = 0; i < node->Count; i++) - Child array filled in */

    /* If there was not enough room in our temporary array for */
    /* all the children, then free the buffer that we allocated */
    if (nItems > AUTO_BER_ITEMS) 
        MemoryFunctions->free_func(AppHandle, items);

    return node;
}
