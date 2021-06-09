/*-----------------------------------------------------------------------
 *      File:   expand.c
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
 * Name: ber_LengthOfBerLength
 *
 * Description:
 * This function computes the number of octets used to represent 
 * the BER/DER length field.
 *
 * Parameters: 
 * Octets (input) - A pointer to a BER-encoded length.
 *
 * Returns: 
 * The number of octets used to represent the length.
 * If an error occurs, 0 is returned.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
uint32 
ber_LengthOfBerLength(const uint8 *Octets)
{
    uint32 length =0; /* number of length bytes past the first */

    /* If this is not a short-form length,  */
    /* Obtain the number of aditional bytes */
    if (!IS_SHORT_FORM_LEN(*Octets)) {
        length = NUM_LENGTH_BYTES(*Octets);
        if (length > sizeof(uint32))  return 0; 
    }

    return 1 + length; /* the first byte plus any additional bytes */
}


/*------------------------------------------------------------------------------
 * Name: BER_ExpandItem
 *
 * Description:
 * This function expands a BER-encoded object into its tag, length, and content.  
 * It returns a pointer to the octet immediately following the component. 
 *
 * Parameters: 
 * Octets    (input) - A pointer to the component to be expanded.
 * MaxLength (input) - The maximum number of octets in the component. 
 *                     It is used to help detect errors when garbage input 
 *                     would otherwise cause the parse to exceed the buffer. 
 * Output   (output) - A pointer to the BER_PARSED_ITEM to be filled in 
 *                     with a tag ptr, the content length, and a content ptr.
 *                     The tag and content pointers point to the original octets.
 *
 * Returns: 
 * A pointer to the octet following the last byte of content. 
 * If an error occurs, 0 is returned.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
const uint8 *
BER_ExpandItem(                 
    const uint8 *Octets,        /* input BER to parse */
    uint32 MaxLength,           /* max length of BER to parse */
    BER_PARSED_ITEM_PTR Output) /* the output structure */
{
    uint32 obj_length, tag_length, len_length;

    /* Check outputs */
    if (!Output) return 0;

    /* Initialize outputs */
    Output->Tag = 0;
    Output->ContentLength = 0;
    Output->Content = 0;

    /* Check inputs */
    if (!Octets) return 0;

    /* Obtain tag, length and total sizes for the input BER object */
    tag_length = BER_LengthOfTag(Octets);
    len_length = ber_LengthOfBerLength(Octets + tag_length);
    obj_length = BER_SizeofObject(Octets);
    if (!tag_length || !len_length || !obj_length || 
        obj_length > MaxLength || obj_length < (tag_length+len_length))
        return 0;

    /* Set outputs & return */
    Output->Tag = Octets;
    Output->ContentLength = obj_length - tag_length - len_length;
    Output->Content = Octets + tag_length + len_length;
    return Octets + obj_length;
}
