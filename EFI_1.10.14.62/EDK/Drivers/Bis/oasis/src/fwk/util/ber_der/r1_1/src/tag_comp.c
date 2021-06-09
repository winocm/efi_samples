/*-----------------------------------------------------------------------
 *      File:   tag_comp.c
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
 * Name: BER_TagCompare
 *
 * Description:
 * This function lexigraphically compares two BER/DER tags.
 * It returns a signed integer to indicate the relationship.
 *
 * Parameters: 
 * A (input) - A pointer to the first tag
 * B (input) - A pointer to the second tag
 *
 * Returns: 
 * If A < B, a negative number is returned. 
 * If A == B, zero is returned. 
 * If A > B, a positive number is returned.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
sint32 
BER_TagCompare(const uint8 *A, const uint8 *B)
{
    /* Check inputs */
    if (!A && !B) return  0;
    if (!A)       return -1;
    if (!B)       return  1;

    /* If the first byte is not the same, return *A - *B */
    if (*A != *B) return *A - *B;

    /* The first tag byte is the same */
    /* If this is a short tag, return 0 indicating equality */
    if (!IS_LONG_TAG(*A)) return 0;  

    /* This is a long tag 
    /*
    /* Loop until the first non-equal byte is found or 
    /* the last byte in A is encountered 
    /*
    /* If B is shorter than A, the loop will terminate with a
    /* positive number because *A will be greater than *B for 
    /* that last byte because bit 8 ==1 in *A and ==0 in *B 
    /*
    /* If A is shorter than B, the loop will terminate with a
    /* negative number because *A will be less than *B for 
    /* that last byte because bit 8 ==0 in *A and ==1 in *B 
    /* 
    /* If the end of A is reached and we have not terminated, 
    /* then the strings must be equal.  return 0.
     */
    for (;;) {
        if (*++A != *++B) return *A - *B;
        if ((*A & 128) == 0) break;
    }
    return 0;   /* long equal tags */
}
