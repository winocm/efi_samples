/*-----------------------------------------------------------------------
 *      File:   xpandset.c
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
 * Name: BER_ExpandSet
 *
 * Description:
 * BER_ParseComplexObject uses this function when a SET is encountered. 
 * The function fills in the output array of BER_PARSED_ITEM elements. 
 * It detects missing fields and variant data structures using the input template.
 *
 * Parameters: 
 * Octets       (input) - A pointer to the octets to be parsed.
 * InputLength  (input) - Number of octets to be parsed.
 * MaxCount     (input) - The maximum number of output components to be parsed.  
 * NumberOfTypes(input) - The rank of the Defaults arrays.
 * Defaults     (input) - A pointer to an array of pointers to BER-encoded 
 *                        default values for each component. If there are no  
 *                        default values, the pointer is NULL. Components that 
 *                        do not have default values are indicated by 
 *                        a corresponding NULL array element.  Default values 
 *                        may be referenced by the output BER_PARSED_ITEM values, 
 *                        and therefore must have a longer lifetime 
 *                        than the output.
 * Output      (output) - This parameter points to an array that is filled in  
 *                        with the components of the set. The array must be 
 *                        of sufficient size to hold the output.  
 *
 * Returns: 
 * The number of components successfully parsed is returned.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
sint32 
BER_ExpandSet(                      /* returns the number of components parsed */
    const uint8  *Octets,           /* input BER to parse                      */
    uint32 InputLength,             /* length of BER to parse                  */
    sint32 MaxCount,                /* maximum number of components to parse   */
    sint32 NumberOfTypes,           /* length of the Defaults array            */
    const BER_PARSED_ITEM *Defaults,/* default values                          */
    BER_PARSED_ITEM_PTR Output)     /* output array of parsed items            */
{
    const uint8 *currentPosition;
    uint32 remainingInput;
    sint32 i, j;

    /* Initialize variables */
    currentPosition = Octets;
    remainingInput = InputLength;

    /* Expand each component in the SET                 */
    /* into its Tag, Length and Content representation. */
    for (i = 0; i < MaxCount; i++) {
        currentPosition = 
            BER_ExpandItem(currentPosition, remainingInput, &Output[i]);
        if (!currentPosition) break;
        remainingInput = InputLength - (uint32)(currentPosition - Octets);
    }

    /* Fill in the default values. */
    if (Defaults) {
        for (j = 0; j < NumberOfTypes; j++) {

            /* Replace the current value with its default value */
            if (Defaults[j].Tag) Output[i++] = Defaults[j];

            /* If the output is too small, FAIL */
            if (i >= MaxCount) return 0;    
        }
    }

    return i;
}
