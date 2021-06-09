/*-----------------------------------------------------------------------
 *      File:   flattree.c
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
 * Name: DER_CopyTree
 *
 * Description:
 * This function flattens the parse tree according to the info in the tree nodes.
 * It outputs the octets that represent the tree's DER encoding. 
 * The user should have first called DER_SizeofTree to prepare the parse tree.
 * The output buffer should be at least as long as the value returned 
 * by DER_SizeofTree.
 *
 * (The spliting of the DER output into these two functions was done purely 
 * to avoid memory allocation issues. It is strongly suggested that the 
 * DER_Generate function be used when a memory allocator is available.)
 *
 * Parameters: 
 * Node (input/output) - A pointer to the parse tree to be flattened.
 * Octets     (output) - A pointer to the output buffer.  The buffer must be of 
 *                       sufficient length to contain the DER encoding of 
 *                       the parse tree (as determined by DER_SizeofTree).
 *
 * Returns: 
 * The number of octets in the DER-encoding of the parse tree. 
 * A zero length or a length not equal to that returned by DER_SizeofTree 
 * indicates an error.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
uint32 
DER_CopyTree(
    DER_NODE_PTR Node,  /* tree to flatten */
    uint8 *Octets)      /* output buffer */
{
    const uint8 *source = Octets;
    sint32 i;

    /* Check inputs */
    if (!Node || !Octets) return 0;

    /* Output our tag and length */
    Octets += BER_OutputTag(Node->Tag, Octets);
    Octets += BER_OutputLength(Node->Length, Octets);

    /* Output our children as our Content */
    for (i = 0; i < Node->Count; i++) {
        /* If this child does not have a subtree to flatten, */
        /* copy its components to the output buffer */
        if (Node->Child[i].IsLeaf) { 
            /* If this child is a missing optional component, skip it */
            if (!Node->Child[i].X.Input.Tag) continue;  
#ifdef FIX_SORT_SET_BUG
            Node->Child[i].X.Input.Tag = Octets;	/* for sort */
#endif
            /* Output tag and length */
            Octets += BER_OutputTag(Node->Child[i].X.Input.Tag, Octets);
            Octets += BER_OutputLength(Node->Child[i].X.Input.ContentLength, 
                                       Octets);

            /* Output content */
            cssm_memcpy(Octets, 
                        Node->Child[i].X.Input.Content, 
                        Node->Child[i].X.Input.ContentLength);
            Octets += Node->Child[i].X.Input.ContentLength;

        } else {
            /* This child does have a subtree to flatten */
            /* If this is a missing optional component, skip it */
            if (!Node->Child[i].X.Node) {
#ifdef FIX_SORT_SET_BUG
                Node->Child[i].X.Input.Tag = 0;
#endif
                continue;
            }

            /* Obtain the DER-encoded sub-tree */
#ifdef FIX_SORT_SET_BUG
            Node->Child[i].X.Input.Tag = Octets;	/* for sort */
#endif
            Octets += DER_CopyTree(Node->Child[i].X.Node, Octets);
        }
    }

#ifdef FIX_SORT_SET_BUG
    /* SET entries should already be sorted by tag and then length. */
    /* Now sort by contents. */
    if (Node->OriginalTag[0] == BER_CONSTRUCTED_SET) {
        /* use a simple bubble sort */
        int j, test;
        unsigned int k, minlen, lenj;
        unsigned char *valuei, *valuej;
        unsigned char temp;
        for (i = 0; i < Node->Count; i++) {
            if (!Node->Child[i].X.Input.Tag) 
                continue;  /* optional field is missing */
            len = BER_SizeofObject(Node->Child[i].X.Input.Tag);
            for (j = i + 1; j < Node->Count; j++) {
                valuei = (unsigned char *) Node->Child[i].X.Input.Tag;
                valuej = (unsigned char *) Node->Child[j].X.Input.Tag;
                if (!valuej) 
                    continue;  /* optional field is missing */
                lenj = BER_SizeofObject(Node->Child[j].X.Input.Tag);
                minlen = len < lenj ? len : lenj;
                test = 0;
                for (k = 0; k < minlen; k++) {
                    test = *valuei++ - *valuej++;
                    if (test != 0) 
                        break;
                }
                if (test > 0) {  /* exchange */
                    if (len != lenj)
                        return Octets - source;
                    valuei = (unsigned char *) Node->Child[i].X.Input.Tag;
                    valuej = (unsigned char *) Node->Child[j].X.Input.Tag;
                    for (k = 0; k < len; k++) {
                        temp = *valuei;
                        *valuei++ = *valuej;
                        *valuej++ = temp;
                    }
                }
            }
        }
    }
#endif

    return (uint32)(Octets - source);
}
