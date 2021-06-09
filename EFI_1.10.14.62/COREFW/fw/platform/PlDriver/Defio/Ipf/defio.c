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

    BugBug: Need to do SAL PROC calls to do PCI config space.
            Cheating and going directly to CF8/CFC

    BugBug: Need to change the name away from IA32

Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "Pldefio.h"
#include "pci22.h"

//
// Might be good to put this in an include file, but people may start
//  using it! They should always access the EFI abstraction that is
//  contained in this file. Just a little information hiding.
//
#define PORT_TO_MEM(_Port) ( ((_Port) & 0xffffffffffff0000) | (((_Port) & 0xfffc) << 10) | ((_Port) & 0x0fff) )
                                                                           
//                                                                  
// Macro's with casts make this much easier to use and read.
//
#define PORT_TO_MEM8(_Port)     (*(UINT8  *)(PORT_TO_MEM(_Port)))
#define PORT_TO_MEM16(_Port)    (*(UINT16 *)(PORT_TO_MEM(_Port)))
#define PORT_TO_MEM32(_Port)    (*(UINT32 *)(PORT_TO_MEM(_Port)))



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
    PTR             Buffer;
    IO_DEVICE       *IoDevice;
    EFI_STATUS      Status;
    UINT16          Data16;
    UINT32          Data32;

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

    if (Dev) {
        IoDevice = CR(Dev, IO_DEVICE, Io, IO_DEVICE_SIGNATURE);

        //
        // Boot Service Time so we are called via a protocol
        //

        Address += IoDevice->IoBase;
    } else {

        //
        // Runtime - we are called with NULL
        //

        Address += RtVirtualIoBase;
    }

    //
    // Loop for each iteration and move the data
    //

    while ((Count)  && !EFI_ERROR(Status)) {
        MEMORY_FENCE();
        switch (Width) {
        case IO_UINT8:
            *Buffer.ui8++ = PORT_TO_MEM8(Address);
            Address += sizeof (UINT8);
            break;

        case IO_UINT16:
            if (Buffer.ui & 0x1) {
                Data16 = PORT_TO_MEM16(Address);
                *Buffer.ui8++ = (UINT8)(Data16 & 0xff);
                *Buffer.ui8++ = (UINT8)((Data16 >> 8) & 0xff);
            } else {
                *Buffer.ui16++ = PORT_TO_MEM16(Address);
            }
            Address += sizeof (UINT16);
            break;

        case IO_UINT32:
            if (Buffer.ui & 0x3) {
                Data32 = PORT_TO_MEM32(Address);
                *Buffer.ui8++ = (UINT8)(Data32 & 0xff);
                *Buffer.ui8++ = (UINT8)((Data32 >> 8) & 0xff);
                *Buffer.ui8++ = (UINT8)((Data32 >> 16) & 0xff);
                *Buffer.ui8++ = (UINT8)((Data32 >> 24) & 0xff);
            } else {
                *Buffer.ui32++ = PORT_TO_MEM32(Address);
            }
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
    PTR             Buffer;
    IO_DEVICE       *IoDevice;
    EFI_STATUS      Status;
    UINT16          Data16;
    UINT32          Data32;

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

    if (Dev) {
        IoDevice = CR(Dev, IO_DEVICE, Io, IO_DEVICE_SIGNATURE);
        //
        // Boot Service Time so we are called via a protocol
        //
        Address += IoDevice->IoBase;
    } else {
        //
        // Runtime - we are called with NULL
        //
        Address += RtVirtualIoBase;
    }
    
    //
    // Loop for each iteration and move the data
    //
    while ((Count) && !EFI_ERROR(Status)) {

        switch (Width) {
        case IO_UINT8:
            PORT_TO_MEM8(Address) = *Buffer.ui8++;
            Address += sizeof (UINT8);
            break;

        case IO_UINT16:
            if (Buffer.ui & 0x1) {
                Data16 = *Buffer.ui8++;
                Data16 = Data16 | (*Buffer.ui8++ << 8);
                PORT_TO_MEM16(Address) = Data16;
            } else {
                PORT_TO_MEM16(Address) = *Buffer.ui16++;
            }
            Address += sizeof (UINT16);
            break;

        case IO_UINT32:
            if (Buffer.ui & 0x3) {
                Data32 = *Buffer.ui8++;
                Data32 = Data32 | (*Buffer.ui8++ << 8);
                Data32 = Data32 | (*Buffer.ui8++ << 16);
                Data32 = Data32 | (*Buffer.ui8++ << 24);
                PORT_TO_MEM32(Address) = Data32;
            } else {
                PORT_TO_MEM32(Address) = *Buffer.ui32++;
            }
            Address += sizeof (UINT32);
            break;

        default:
            Status = EFI_INVALID_PARAMETER;
            break;
        }
        MEMORY_FENCE();

        //
        // Next IO
        //
        Count -= 1;
    }

    return Status;
}
