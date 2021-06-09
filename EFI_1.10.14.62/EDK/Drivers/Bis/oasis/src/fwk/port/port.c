/*-----------------------------------------------------------------------
 *      File:   port.c
 *
Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

 *
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

#include <efi.h>
#include <efidriverlib.h>
#include "cssm.h"
#include "cssmport.h"

/*-----------------------------------------------------------------------------
 * Name: cssm_srand
 *
 * Description  : Seeds the random number generator (RNG)
 * 
 * Parameters: 
 * Seed (input) : Value to use to seed the RNG
 *
 * Returns      : None
 * 
 * Error Codes  : None
 *---------------------------------------------------------------------------*/
static uint32 randseed = 1L;
void   CSSMAPI cssm_srand(uint32 Seed)
{
    randseed = Seed;
}

/*-----------------------------------------------------------------------------
 * Name: cssm_rand
 *
 * Description: Generates a pseudorandom integer
 * 
 * Parameters : None
 *
 * Returns    : A pseudorandom integer
 * 
 * Error Codes: None
 *---------------------------------------------------------------------------*/
sint32 CSSMAPI cssm_rand(void)
{
    return(((randseed = randseed * 214013L + 2531011L) >> 16) & 0x7fff);
}

/*-----------------------------------------------------------------------------
 * Name: cssm_memset
 *
 * Description:  Initializes all elements of a memory block to a value
 * 
 * Parameters: 
 * ptr (input)       : Pointer to memory block
 * value (input)     : Value to initialize memory to
 * num_bytes (input) : Number of bytes of memory to initialize
 *
 * Returns           : Ptr to memory (same as pointer passed in)
 * 
 * Error Codes       : None
 *---------------------------------------------------------------------------*/
void * cssm_memset (void *ptr, sint32 value, uint32 num_bytes )
{
	//Use EFI method. Avoid unaligned data faults.
	EfiSetMem( ptr, num_bytes, (uint8) value);
	
	#if 0
    /* This code does not attempt to align the source or destination */
    /* which may be important on non Intel architectures             */ 
    /* It does use DWORD copies instead of character copies to speed */
    /* things up a bit.                                              */

    if (ptr != NULL)
    {
        uint8 *Temp;
        uint32 val32, x;

        value &= 0xFF;  /* this is dealt with as a character */
        val32 = value | (value << 8) | (value << 16) | (value << 24);
        Temp = (uint8 *)ptr;

        for (x = 0; x < (num_bytes >> 2); x++)
        {
            *(uint32 *)Temp = val32;
            Temp += sizeof(uint32);
        }
        for (x = 0; x < (num_bytes % 4); x++)
        {
            *Temp++ = (uint8) value;
        }
    }
    #endif

    return(ptr);
}

/*-----------------------------------------------------------------------------
 * Name: cssm_memcpy
 *
 * Description:  Copies contents of one memory block to another
 * 
 * Parameters: 
 * dest_ptr (input)  : Pointer to destination memory block
 * src_ptr (input)   : Pointer to source memory block
 * num_bytes (input) : Number of bytes of memory to copy
 *
 * Returns           : Ptr to memory (same as pointer passed in)
 * 
 * Error Codes       : None
 *---------------------------------------------------------------------------*/
void * cssm_memcpy (void *dest_ptr, const void *src_ptr, uint32 num_bytes )
{
	EfiCopyMem( dest_ptr, (void*)src_ptr, num_bytes);
#if 0
    /* This code does not attempt to align the source or destination */
    /* which may be important on non Intel architectures             */ 
    /* It does use DWORD copies instead of character copies to speed */
    /* things up a bit.                                              */

    if (dest_ptr != NULL && src_ptr != NULL)
    {
        uint8 *TempSrc;
        uint8 *TempDst;
        uint32 x;

        TempSrc = (uint8 *)src_ptr;
        TempDst = (uint8 *)dest_ptr;
        for (x = 0; x < (num_bytes >> 2); x++)
        {
            *(uint32 *)TempDst = *(uint32 *)TempSrc;
            TempDst += sizeof(uint32);
            TempSrc += sizeof(uint32);
        }
        for (x = 0; x < (num_bytes % 4); x++)
        {
            *TempDst++ = *TempSrc++;
        }
    }
  #endif
    return (dest_ptr);
}

/*-----------------------------------------------------------------------------
 * Name: cssm_memmove
 *
 * Description:  Copies contents of one memory block to another -
 *               handles overlapping memory regions
 * 
 * Parameters: 
 * dest_ptr (input)  : Pointer to destination memory block
 * src_ptr (input)   : Pointer to source memory block
 * num_bytes (input) : Number of bytes of memory to copy
 *
 * Returns           : Ptr to memory (same as pointer passed in)
 * 
 * Error Codes       : None
 *---------------------------------------------------------------------------*/
void * cssm_memmove (void *dest_ptr, const void *src_ptr, uint32 num_bytes )
{
    void * Temp = dest_ptr;

    if (dest_ptr <= src_ptr || (char *)dest_ptr >= ((char *)src_ptr + num_bytes)) 
    {
        /* Buffers do not overlap copy from lower higher addresses */
        while (num_bytes--) 
        {
            *(char *)dest_ptr = *(char *)src_ptr;
            dest_ptr = (char *)dest_ptr + 1;
            src_ptr = (char *)src_ptr + 1;
        }
    } else {
        /* Buffer overlap copy from higher to lower addresses */
        dest_ptr = (char *)dest_ptr + num_bytes - 1;
        src_ptr = (char *)src_ptr + num_bytes - 1;

        while (num_bytes--) 
        {
            *(char *)dest_ptr = *(char *)src_ptr;
            dest_ptr = (char *)dest_ptr - 1;
            src_ptr = (char *)src_ptr - 1;
        }
    }
    return(Temp);
}

/*-----------------------------------------------------------------------------
 * Name: cssm_memcmp
 *
 * Description:  Compares the contents of two memory blocks
 * 
 * Parameters: 
 * ptr1 (input)  : Pointer to first memory block
 * ptr2 (input)  : Pointer to second memory block
 * count (input) : Number of bytes of memory to compare
 *
 * Returns:
 * < 0             ptr1 less than ptr2
 * 0               ptr1 identical to ptr2
 * > 0             ptr1 greater than ptr2
 * 
 * Error Codes   : None
 *---------------------------------------------------------------------------*/
sint32 cssm_memcmp (const void *ptr1, const void *ptr2, uint32  count)
{
    if (count != 0)
    {
        while ( --count && *(char *)ptr1 == *(char *)ptr2 ) 
        {
            ptr1 = (char *)ptr1 + 1;
            ptr2 = (char *)ptr2 + 1;
        }
    } else 
        return (0);

    return(*((uint8 *)ptr1) - *((uint8 *)ptr2) );
}

/*-----------------------------------------------------------------------------
 * Name: cssm_strlen
 *
 * Description : Returns the length of a zero terminated string
 * 
 * Parameters: 
 * str (input) : Pointer to a string
 *
 * Returns     : Length of the input string
 * 
 * Error Codes : None
 *---------------------------------------------------------------------------*/
uint32 CSSMAPI cssm_strlen (const char * str)
{
    uint32 len = 0;
    if (str)
        while (*str++) len++;
    return(len);
}
