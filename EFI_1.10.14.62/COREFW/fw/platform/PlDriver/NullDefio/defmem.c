/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:
    
      defia32.c

Abstract:




Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "Pldefio.h"

EFI_STATUS
INTERNAL
DefMemoryRead (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   Address,
    IN UINTN                    Count,
    IN OUT VOID                 *Buffer
    )
{
    if (Width < 0 || Width > IO_UINT64) {
        return EFI_INVALID_PARAMETER;
    }
    SetMem(Buffer,Count*(1<<Width),0x00);
    return EFI_SUCCESS;
}



EFI_STATUS
INTERNAL
DefMemoryWrite (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   Address,
    IN UINTN                    Count,
    IN OUT VOID                 *Buffer
    )
{
    if (Width < 0 || Width > IO_UINT64) {
        return EFI_INVALID_PARAMETER;
    }
    return EFI_SUCCESS;
}


