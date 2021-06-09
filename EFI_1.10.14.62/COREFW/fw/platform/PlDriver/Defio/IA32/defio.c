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

      These are the read and write primitives for the Device I/O protocol.  They are
      the means by which a driver should issue read and write I/O requests to the 
      platform.  The description of the services can be found in Chapter 6 of the EFI
      Specification.


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
    UINT32          Result;
    PTR             Buffer;
    EFI_STATUS      Status;

    Status = EFI_SUCCESS;

    Address = (UINTN) UserAddress;
    Buffer.buf = (UINT8 *)UserBuffer;
    AlignMask = (1 << Width) - 1;

    if (Address > 0xFFFF) {
        Status = EFI_UNSUPPORTED;
    }
    
    if (Address & AlignMask) {
        Status = EFI_INVALID_PARAMETER;
    }

    //
    // Loop for each iteration and move the data
    //

    while ((Count) && !EFI_ERROR(Status)) {
        switch (Width) {
        case IO_UINT8:
            _asm {
                mov     edx, Address
                in      al, dx
                mov     Result, eax
            };
            *Buffer.ui8 = (UINT8)Result;
            Buffer.ui8++;
            Address += sizeof (UINT8);
            break;

        case IO_UINT16:
            _asm {
                mov     edx, Address
                in      ax, dx
                mov     Result, eax
            };
            *Buffer.ui16 = (UINT16)Result;
            Buffer.ui16++;
            Address += sizeof (UINT16);
            break;

        case IO_UINT32:
            _asm {
                mov     edx, Address
                in      eax, dx
                mov     Result, eax
            };
            *Buffer.ui32 = Result;
            Buffer.ui32++;
            Address += sizeof (UINT32);
            break;

        default:
            Status = EFI_INVALID_PARAMETER;
            break;
        }

        //
        // Next IO
        //

        Count -= 1;
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
    UINT32          Result;
    PTR             Buffer;
    EFI_STATUS      Status;

    Status = EFI_SUCCESS;

    Address = (UINTN) UserAddress;
    Buffer.buf = (UINT8 *)UserBuffer;
    AlignMask = (1 << Width) - 1;

    if (Address > 0xFFFF) {
        Status = EFI_UNSUPPORTED;
    }
    
    if (Address & AlignMask) {
        Status = EFI_INVALID_PARAMETER;
    }

    //
    // Loop for each iteration and move the data
    //

    while ((Count) && !EFI_ERROR(Status)) {

        switch (Width) {
        case IO_UINT8:
            Result = *(UINT32 *)Buffer.ui8;
            _asm {
                mov     edx, Address
                mov     eax, Result
                out     dx, al
            }
            Buffer.ui8++;
            Address += sizeof (UINT8);
            break;

        case IO_UINT16:
            Result = *(UINT32 *)Buffer.ui16;
            _asm {
                mov     edx, Address
                mov     eax, Result
                out     dx, ax
            }
            Buffer.ui16++;
            Address += sizeof (UINT16);
            break;

        case IO_UINT32:
            Result = *Buffer.ui32;
            _asm {
                mov     edx, Address
                mov     eax, Result
                out     dx, eax
            }
            Buffer.ui32++;
            Address += sizeof (UINT32);
            break;

        default:
            Status = EFI_INVALID_PARAMETER;
            break;
        }

        //
        // Next IO
        //

        Count -= 1;
    }

    return Status;
}
