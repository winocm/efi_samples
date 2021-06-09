// CR_Start
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
// This software and associated documentation (if any) is furnished
// under a license and may only be used or copied in accordance
// with the terms of the license. Except as permitted by such
// license, no part of this software or documentation may be
// reproduced, stored in a retrieval system, or transmitted in any
// form or by any means without the express written consent of
// Intel Corporation.
//
//
///////////////////////////////////////////////////////////////////////////////
// CR_End

//************************************************************************************************//
// port.c
//
// Description:
//    Functions in this file provide the port from Windows based calls (memcpy and memset)
//    to their EFI implementations. 
//
//************************************************************************************************//

#include <efi.h>
#include <efidriverlib.h>

void* memcpy(void *dest, void *src, UINTN count);
void* memset(void *dest, UINT8 c, UINTN count);

#pragma function(memcpy)
#pragma function(memset)

///////////////////////////////////////////////////////////////////////////////
//
//  Function Name: memcpy
//
//  Description:   copy specified number of bytes from source to destination
//
//  Returns:
//
///////////////////////////////////////////////////////////////////////////////
void* memcpy(void *dest, void *src, UINTN count)
{
    EfiCopyMem(dest, (void *)src, count);

    return dest;
} // End of memcpy()

///////////////////////////////////////////////////////////////////////////////
//
//  Function Name: memset
//
//  Description:   set specified count of buffer to desired value
//
//  Returns:
//
///////////////////////////////////////////////////////////////////////////////
void *  memset(void *dest, UINT8 c, UINTN count)
{
    EfiSetMem(dest, count, c);

    return dest;
} // End of memset()
