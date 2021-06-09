/*-----------------------------------------------------------------------
 *      File:   treesize.c
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


/*------------------------------------------------------------------------------
 * Name: ber_ContentCompare
 *
 * Description:
 * This function compares the length and contents of two BER parsed items.
 * It returns a signed integer to indicate the relationship.
 *
 * Parameters: 
 * A (input) - A pointer to the first BER_PARSED_ITEM
 * B (input) - A pointer to the second BER_PARSED_ITEM
 *
 * Returns: 
 * If the content length and content of A equals 
 * the content length and content of B, zero is returned. 
 * Otherwise, a negative number is returned.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
sint32 ber_ContentCompare(BER_PARSED_ITEM_PTR A, BER_PARSED_ITEM_PTR B)
{
    if (A->ContentLength == B->ContentLength &&
        cssm_memcmp(A->Content, B->Content, A->ContentLength) == 0)
        return 0;

    return -1;
}


/*------------------------------------------------------------------------------
 * Name: DER_SizeofTree
 *
 * Description:
 * This function modifies the parse tree according to the types in the nodes.
 * For example, it eliminate instances of default values for sequence and set 
 * nodes and sorts sets by their tags and lengths. It recursively calculates 
 * the number of octets necessary to DER-encode the tree 
 * 
 * After allocating a buffer of the required length, it is expected that a call 
 * to DER_CopyTree will be made to complete the conversion. 
 *
 * (The spliting of the DER output into these two functions was done 
 * purely to avoid memory allocation issues. It is strongly suggested that 
 * the DER_Generate function be used when a memroy allocator is available.)
 *
 * Parameters: 
 * Node (input/output) - The pointer to the parse tree to be prepared 
 *                       for DER output.
 *
 * Returns: 
 * The number of octets necessary to represent the DER-encoded parse tree.
 * A zero length indicates an error.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
uint32 
DER_SizeofTree(DER_NODE_PTR Node)
{
    uint32 length;
    sint32 i, j;

    /* Check inputs */
    if (!Node) return 0;

    /* Compute the length of each child */
    length = 0;
    for (i = 0; i < Node->Count; i++) {

        /* If the child is constructed,
        /*   - Remove any items that equal their default value
        /*   - Insert context-specific tags where appropriate
        /*   - Calculate the encoded size
         */
        if (!Node->Child[i].IsLeaf) {

            /* If a parse template exists 
            /*   - Remove any items that equal their default value
            /*   - Insert context-specific tags where appropriate
             */
            if (Node->Type) {
              switch (Node->OriginalTag[0] & ALL_TAG_VALUES_MASK) {

              case BER_SET:
                /* Omit child if content equals any default value for the set */
                if (Node->Type->Defaults) {
                    for (j = 0; j < Node->Type->NumberOfDefaults; j++) {    
                        if (!BER_TagCompare(Node->Child[i].X.Input.Tag, 
                                            Node->Type->ContextTags[j]) &&
                            !ber_ContentCompare(&Node->Child[i].X.Input,
                                            &Node->Type->Defaults[j])) {
                            Node->Child[i].X.Input.Tag = 0;
                            break;
                        }
                    }
                }
                break;

              case BER_SEQUENCE:
                /* Omit child if content equals its default value */
                if (Node->Type->Defaults &&  Node->Type->Defaults[i].Tag &&
                    !ber_ContentCompare(&Node->Child[i].X.Input,
                                        &Node->Type->Defaults[i]) ){
                    Node->Child[i].X.Input.Tag = 0;
                } else {    
                /* Substitute context-sensitive tag, if present */
                    if (Node->Type->ContextTags && Node->Type->ContextTags[i]) {
                        if (Node->Child[i].IsLeaf) {
                            if (Node->Child[i].X.Input.Tag)  /* optional */
                                Node->Child[i].X.Input.Tag = 
                                      Node->Type->ContextTags[i];
                        } else {
                            if (Node->Child[i].X.Node)  /* optional */
                                Node->Child[i].X.Node->Tag = 
                                      Node->Type->ContextTags[i];
                        }
                    }
                }
                break;
              } /* end switch */
            } /* End if (Node->Type)

            /* Add the size of this child to the total node size */
            length += DER_SizeofTree(Node->Child[i].X.Node);

        } else {    
            /* The child is not constructed */

            /* If this is an absent optional field, go to the next child */
            if (!Node->Child[i].X.Input.Tag) continue;

            /* If this is a sequence and this child has a context-specific tag, 
             * replace the original tag with the context-sensitive tag */
            if (Node->OriginalTag && 
                (Node->OriginalTag[0] & ALL_TAG_VALUES_MASK) == BER_SEQUENCE && 
                Node->Type && Node->Type->ContextTags && 
                Node->Type->ContextTags[i]) 
            {
#ifdef FIX_CONTEXT_TAG_BUG
                Node->Child[i].X.Input.Tag = Node->Type->ContextTags[i];
#else
                /* Gary's original code
                Node->Child[i].X.Input.Tag = Node->Type->ContextTags[i];
                */
                /* Marie's replacement code */
                /* If the OriginalTag is at least as long as */
                /* the context-specific tag, copy it.    */
                /* This will be true in almost all cases */
                if ( BER_LengthOfTag(Node->Child[i].X.Input.Tag) >= 
                     BER_LengthOfTag(Node->Type->ContextTags[i]) )
                    cssm_memcpy(
                        (void*) Node->Child[i].X.Input.Tag, 
                        Node->Type->ContextTags[i], 
                        BER_LengthOfTag(Node->Type->ContextTags[i]) );
                else {
                /* If the original tag is shorter than the ctx-specific tag, */
                /* return an error. */
                /* In 1.2, this function should be updated to take MemoryFuncs */
                /* and a re-alloc should be performed */
                    return 0;
                }
#endif
            }

            /* Add the size of this child to the total node size */
            length += BER_LengthOfTag(Node->Child[i].X.Input.Tag);
            length += BER_LengthOfLength(Node->Child[i].X.Input.ContentLength);
            length += Node->Child[i].X.Input.ContentLength;

        } /* End if constructed */
    } /* End for each child */

#ifdef FIX_SORT_SET_BUG
    /* Sort set entries by tag and then length */
    /* Defer sort of contents until CopyDERTree--not flat yet */
    if (Node->OriginalTag[0] == BER_CONSTRUCTED_SET) {
        /* use a simple bubble sort */
        sint32 test;
        for (i = 0; i < Node->Count; i++) {
            if (!Node->Child[i].X.Input.Tag) continue; /* optional */
                
            for (j = i + 1; j < Node->Count; j++) {
                if (!Node->Child[j].X.Input.Tag) continue; /* optional */ 

                test = BER_TagCompare(Node->Child[i].X.Input.Tag, 
                                      Node->Child[j].X.Input.Tag);
                if (test == 0) 
                    test = 
                        ((Node->Child[i].IsLeaf || !Node->Child[i].X.Node) ? 
                          Node->Child[i].X.Input.ContentLength :
                          Node->Child[i].X.Node->Length) - 
                        ((Node->Child[j].IsLeaf || !Node->Child[j].X.Node) ? 
                          Node->Child[j].X.Input.ContentLength  : 
                          Node->Child[j].X.Node->Length);
                if (test > 0) {
                    DER_NODE_CHILD temp;
                    temp = Node->Child[i];
                    Node->Child[i] = Node->Child[j];
                    Node->Child[j] = temp;
                }
            }
        }
    }
#endif

    /* Now we have the length of the aggregated contents */
    Node->Length = length;
    length += BER_LengthOfLength(length);
    length += BER_LengthOfTag(Node->Tag);

    return length;  /* return total length */
}
