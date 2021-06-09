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
    
      defmem.c

Abstract:




Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "Pldefio.h"


STATIC 
EFI_STATUS
DefMemoryRW (
    IN EFI_IO_WIDTH             Width,
    IN UINTN                    Count,
    IN BOOLEAN                  InStrideFlag,
    IN PTR                      In,
    IN BOOLEAN                  OutStrideFlag,
    OUT PTR                     Out
    )
{
    UINTN                       Stride, InStride, OutStride;
    UINTN                       AlignMask;
    EFI_STATUS                  Status;
    BOOLEAN                     Aligned;

    Status = EFI_SUCCESS;

    Stride = 1 << Width;
    InStride = InStrideFlag ? Stride : 0;
    OutStride = OutStrideFlag ? Stride : 0;

    AlignMask = Stride - 1;

    //
    // Make sure IO is aligned
    //

    Aligned = TRUE;
    if ((In.ui & AlignMask) || (Out.ui & AlignMask)) {
        Aligned = FALSE;
    }

    //
    // Loop for each iteration and move the data
    //

    while ((Count) && !EFI_ERROR (Status)) {

        //
        // Perform the IO
        //
        
        MEMORY_FENCE();
        switch (Width) {
        case IO_UINT8:
            *In.ui8 = *Out.ui8;
            break;

        case IO_UINT16:
            if (Aligned) {
                *In.ui16 = *Out.ui16;
            } else {
                *In.ui8     = *Out.ui8;
                *(In.ui8+1) = *(Out.ui8+1);
            }
            break;

        case IO_UINT32:
            if (Aligned) {
                 *In.ui32 = *Out.ui32;
            } else {
                *In.ui8     = *Out.ui8;
                *(In.ui8+1) = *(Out.ui8+1);
                *(In.ui8+2) = *(Out.ui8+2);
                *(In.ui8+3) = *(Out.ui8+3);
            }
            break;

        case IO_UINT64:
            if (Aligned) {
                 *In.ui64 = *Out.ui64;
            } else {
                *In.ui8     = *Out.ui8;
                *(In.ui8+1) = *(Out.ui8+1);
                *(In.ui8+2) = *(Out.ui8+2);
                *(In.ui8+3) = *(Out.ui8+3);
                *(In.ui8+4) = *(Out.ui8+4);
                *(In.ui8+5) = *(Out.ui8+5);
                *(In.ui8+6) = *(Out.ui8+6);
                *(In.ui8+7) = *(Out.ui8+7);
            }
            break;
        default:
            Status = EFI_INVALID_PARAMETER;
            break;
        }
        MEMORY_FENCE();

        //
        // Next IO
        //

        In.buf += InStride;
        Out.buf += OutStride;
        Count -= 1;
    }

    return Status;
}


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
    PTR             in, out;
    IO_DEVICE       *IoDevice;

    IoDevice = CR(Dev, IO_DEVICE, Io, IO_DEVICE_SIGNATURE);
    in.buf = (VOID *) Address;
    in.ui = in.ui + IoDevice->MemBase;
    out.buf = Buffer;
    if (Width >= MMIO_COPY_UINT8) {
        //
        // Convert to IO_UINT8 - IO_UINT64
        //
        Width = Width - MMIO_COPY_UINT8;
        out.ui = out.ui + IoDevice->MemBase;
    }
    return DefMemoryRW (Width, Count, TRUE, out, TRUE, in);
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
    PTR             in, out;
    IO_DEVICE       *IoDevice;

    IoDevice = CR(Dev, IO_DEVICE, Io, IO_DEVICE_SIGNATURE);
    in.buf = Buffer;
    if (Width >= MMIO_COPY_UINT8) {
        //
        // Convert to IO_UINT8 - IO_UINT64
        //
        Width = Width - MMIO_COPY_UINT8;
        in.ui = in.ui + IoDevice->MemBase;
    }
    out.buf = (VOID *) Address;
    out.ui = out.ui + IoDevice->MemBase;
    return DefMemoryRW (Width, Count, TRUE, out, TRUE, in);
}


