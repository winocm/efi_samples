/*-----------------------------------------------------------------------
 *      File:   sizeof.c
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
 * Name: BER_SizeofObject
 *
 * Description:
 * Return the length of the tag + the length of the length octets + 
 * the length of the contents of the input BER encoded object
 *
 * Parameters: 
 * Octets (input) - A BER or DER-encoded object
 *
 * Returns: 
 * The length of the object
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
uint32 
BER_SizeofObject(const uint8 *Octets)
{
    uint32 tagLength, lenLength, objLength, length;
    sint32 noctets;

    if (!Octets) return 0;

    /* Obtain the length of the tag */
    tagLength = BER_LengthOfTag(Octets);
    Octets += tagLength;

    /* 
     * Compute the length and the number of bytes used to represent the length
     */

    /* If this is the short form of the length, */
    /* the total length equals its TagLength + 1 byte to describe the length */
    /* + whatever is in the length byte */
    if (IS_SHORT_FORM_LEN(*Octets)) 
        return tagLength + 1 + *Octets;

    /* This is the long or indefinite form of the length */
    /* Compute the number of length bytes and move to the next byte */
    noctets = NUM_LENGTH_BYTES(*Octets++);
    if (noctets > sizeof(uint32))  return 0; 
    lenLength = 1 + noctets; 

    /* If this is the long definite form of length, */
    objLength = 0;
    if (noctets > 0) {   
        /* The length = 256 * the inverse concatentation of the length octets */
        while (noctets-- > 0) objLength = 256 * objLength + *Octets++;

    } else {
#ifdef EISL
        return 0;
#endif
        /* This is the constructed indefinite form of the length */
        /* This item ends at the end-of-contents marker 0x00 0x00 */
        while (Octets[0] != INDEF_END_OF_CONTENTS_MARKER || 
               Octets[1] != INDEF_END_OF_CONTENTS_MARKER) { 

            /* Obtain the length of the next object */
            length = BER_SizeofObject(Octets);

            /* Add it to the total content length */
            objLength += length;

            /* Index past this object */
            Octets += length;
        }

        /* Add to the length to account for the end-of-contents marker */
        objLength += INDEF_END_OF_CONTENTS_NUM_BYTES;
    }

    /* Return the TagLength + LengthOfLength + ContentLength */
    return tagLength + lenLength + objLength;
}
