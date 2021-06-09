/*-----------------------------------------------------------------------
 *      File:   outtag.c
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
 * Name: BER_OutputTag
 *
 * Description:
 * This function copies the BER/DER tag from the source to the output buffer.
 * It returns the number of octets copied.
 *
 * Parameters: 
 * Source  (input) - The tag to be copied.
 * Octets (output) - The buffer into which the tag will be copied.
 *
 * Returns: 
 * The number of octets in the BER/DER tag.
 *
 * Error Codes:
 * None.
 *----------------------------------------------------------------------------*/
uint32 
BER_OutputTag(           /* return length of copied tag */
    const uint8 *Source, /* input BER/DER tag */
    uint8 *Octets)       /* output buffer */
{
    uint32 length;

    /* Obtain the tag length */
    length = BER_LengthOfTag(Source);

    /* Copy the correct number of bytes to the output buffer */
    cssm_memcpy(Octets, Source, length);

    /* Return the tag length */
    return length;
}
