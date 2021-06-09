/*-----------------------------------------------------------------------
 *      File:   freetree.c
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

/*------------------------------------------------------------------------------
 * Name: DER_RecycleTree
 *
 * Description:
 * This function releases the memory used for the input parse tree structure.
 * This includes the DER_NODE structure and DER_NODE_CHILD array, which
 * were allocated together in a single contiguous buffer.
 *
 * Parameters: 
 * AppHandle        (input) - The parameter to the memory function.
 * MemoryFunctions  (input) - Memory functions for this library to use 
 *                            when freeing data structures.
 * Node             (input) - The pointer to the parse tree to be recycled.
 *
 * Returns: 
 * None.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
void 
DER_RecycleTree(BER_HANDLE AppHandle,
                BER_MEMORY_FUNCS_PTR MemoryFunctions,
                DER_NODE_PTR Node)
{
    sint32 i;

    /* Check inputs */
    if (!Node || !MemoryFunctions) return;

    /* Free any subtrees */
    for (i = 0; i < Node->Count; i++) {
        if (!Node->Child[i].IsLeaf) 
            DER_RecycleTree(AppHandle, MemoryFunctions, Node->Child[i].X.Node);
    }

    /* Free the top-level node and its children array */
    MemoryFunctions->free_func(AppHandle, Node);
}
