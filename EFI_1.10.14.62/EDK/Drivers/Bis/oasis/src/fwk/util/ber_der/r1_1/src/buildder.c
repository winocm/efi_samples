/*-----------------------------------------------------------------------
 *      File:   buildder.c
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
 * Name: DER_ConstructItem
 *
 * Description:
 * This function encodes the constructed DER item from its components and the tag. 
 * It returns the number of octets that it used to encode the item.
 *
 * Parameters: 
 * BerTag  (input) - A pointer to the tag for the constructed item 
 *                   (generally a SET or SEQUENCE).
 * Count   (input) - The number of components in the constructed item.
 * Inputs  (input) - A pointer to the array of parsed items  
 *                   (tag, length, contents) to be concatenated.
 * Octets (output) - The output buffer for the encoding. 
 *                   It must be sufficiently large to contain the output.
 *
 * Returns: 
 * A pointer to the node is returned, or NULL if the memory allocation failed.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
uint32 
DER_ConstructItem(
    const uint8 *BerTag,        /* top-level tag to output */
    sint32 Count,               /* number of parsed items */
    BER_PARSED_ITEM_PTR Inputs, /* array of parsed items */
    uint8 *Octets)              /* target memory for the output */
{
    const uint8 *source = Octets;
    uint32 lengthContent;
    sint32 i;

    /* Compute the length of the final content */
    lengthContent = 0;
    for (i = 0; i < Count; i++) {
        lengthContent += BER_LengthOfTag(Inputs[i].Tag) +
            Inputs[i].ContentLength + 
            BER_LengthOfLength(Inputs[i].ContentLength);
    }

    /* Copy the DER-encoded Tag and Length to the output buffer */
    Octets += BER_OutputTag(BerTag, Octets);
    Octets += BER_OutputLength(lengthContent, Octets);

    /* Concatentate the input parsed items into */
    /* the content portion of the output buffer */
    for (i = 0; i < Count; i++) {

        /* Copy the DER-encoded Tag and Length to the output buffer */
        Octets += BER_OutputTag(Inputs[i].Tag, Octets);
        Octets += BER_OutputLength(Inputs[i].ContentLength, Octets);

        /* Copy the Content to the output buffer */
        cssm_memcpy(Octets, Inputs[i].Content, Inputs[i].ContentLength);
        Octets += Inputs[i].ContentLength;
    }

    return (uint32)(Octets - source);
}
