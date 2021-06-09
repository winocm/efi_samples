/*-----------------------------------------------------------------------
 *      File:   allocnod.c
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
 * Name: DER_AllocateNode
 *
 * Description:
 * This function allocates memory for a DER_NODE with the specified number of 
 * subordinate components.
 *
 * Parameters: 
 * AppHandle        (input) - The parameter to the memory function.
 * MemoryFunctions  (input) - Memory functions for this library to use 
 *                            when allocating data structures.
 * NumberOfChildren (input) - The number of subordinate DER components. 
 *
 * Returns: 
 * A pointer to the node is returned, or NULL if the memory allocation failed.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
DER_NODE_PTR 
DER_AllocateNode(BER_HANDLE AppHandle,
                 BER_MEMORY_FUNCS_PTR MemoryFunctions,
                 sint32 NumberOfChildren)
{
    DER_NODE_PTR result;

    /* Allocate the node and its subordinate components */
    result = (DER_NODE_PTR) MemoryFunctions->calloc_func(AppHandle, 1,
             sizeof(DER_NODE) + sizeof(DER_NODE_CHILD) * NumberOfChildren );

    /* Initialize the NumberOfChildren and pointer to the ChildArray */
    if (result)
    {
        result->Count = NumberOfChildren;
        result->Child = (DER_NODE_CHILD_PTR) (((uint8 *) result) + sizeof(DER_NODE));
    }

    return result;
}
