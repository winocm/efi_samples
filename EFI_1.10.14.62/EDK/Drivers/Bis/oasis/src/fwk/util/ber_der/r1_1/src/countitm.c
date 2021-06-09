/*-----------------------------------------------------------------------
 *      File:   countitm.c
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
 * Name: BER_CountItems
 *
 * Description:
 * This function counts the number of subordinate BER objects in 
 * the input BER object.  The input BER object must be definite length constructed.
 *
 * Parameters: 
 * Octets (input) - The pointer to the octets to be examined.
 * Length (input) - The number of octets to be examined.
 *
 * Returns: 
 * The count of the number of BER-encoded items in the buffer. 
 * Zero indicates an error.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
sint32
BER_CountItems(const uint8 *Octets, uint32 Length)
{
    sint32 result;
    uint32 len_remaining, len_object;

    len_remaining = Length;
    for (result = 0; len_remaining > 0; result++) {

        /* Obtain the length of the next item */
        len_object = BER_SizeofObject(Octets);

        /* If the length of the next item is greater than the */
        /* number of remaining bytes, FAIL */
        if (len_object > len_remaining) return 0;

        /* Index past this object to the next one */
        Octets += len_object;
        len_remaining -= len_object;

        /* If the input was a constructed indefinite object, */
        /* then we have finished when there are 2 bytes remaining. */
        /* The value of those 2 bytes would be 0x00 0x00 */
        if (len_remaining == INDEF_END_OF_CONTENTS_NUM_BYTES && 
            Octets[0] == INDEF_END_OF_CONTENTS_MARKER && 
            Octets[1] == INDEF_END_OF_CONTENTS_MARKER)
            len_remaining -= INDEF_END_OF_CONTENTS_NUM_BYTES;
    }

    return result;
}
