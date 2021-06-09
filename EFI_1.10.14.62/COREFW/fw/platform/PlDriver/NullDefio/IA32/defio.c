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
    
      defio.c

Abstract:




Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "PlDefio.h"
#include "pci22.h"

//
//
//

#pragma RUNTIME_CODE(RtDefIoRead)
EFI_STATUS
RUNTIMEFUNCTION
RtDefIoRead (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   UserAddress,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    )
{
    UINTN           Address, AlignMask;
    EFI_STATUS      Status;

    Status = EFI_SUCCESS;

    Address = (UINTN) UserAddress;
    AlignMask = (1 << Width) - 1;

    if (Address > 0xFFFF) {
        Status = EFI_UNSUPPORTED;
    }
    
    if (Address & AlignMask) {
        Status = EFI_INVALID_PARAMETER;
    }

    if (!EFI_ERROR(Status)) {
        switch (Width) {
            case IO_UINT8:
            case IO_UINT16:
            case IO_UINT32:
                SetMem(UserBuffer,Count*(1<<Width),0x00);
                break;
            default:
                Status = EFI_INVALID_PARAMETER;
                break;
        }
    }
    return Status;
}

#pragma RUNTIME_CODE(RtDefIoWrite)
EFI_STATUS
RUNTIMEFUNCTION
RtDefIoWrite (
    IN EFI_DEVICE_IO_INTERFACE  *Dev,
    IN EFI_IO_WIDTH             Width,
    IN UINT64                   UserAddress,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    )
{
    UINTN           Address, AlignMask;
    EFI_STATUS      Status;

    Status = EFI_SUCCESS;

    Address = (UINTN) UserAddress;
    AlignMask = (1 << Width) - 1;

    if (Address > 0xFFFF) {
        Status = EFI_UNSUPPORTED;
    }
    
    if (Address & AlignMask) {
        Status = EFI_INVALID_PARAMETER;
    }

    if (!EFI_ERROR(Status)) {
        switch (Width) {
            case IO_UINT8:
            case IO_UINT16:
            case IO_UINT32:
                break;
            default:
                Status = EFI_INVALID_PARAMETER;
                break;
        }
    }
    return Status;
}
