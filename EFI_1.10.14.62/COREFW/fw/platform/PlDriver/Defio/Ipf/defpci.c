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
#include "SalProc.h"
#include "Pldefio.h"
#include "pci22.h"

#define EFI_PCI_ADDRESS_IA64(_seg, _bus,_dev,_func,_reg) \
    ( (UINT64) ( (((UINTN)_seg) << 24) + (((UINTN)_bus) << 16) + (((UINTN)_dev) << 11) + (((UINTN)_func) << 8) + ((UINTN)_reg)) )

EFI_STATUS
INTERNAL
Ia64PCIRead(
    IN EFI_IO_WIDTH             Width,
    IN PCI_CONFIG_ACCESS_CF8    *Pci,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    ); 

EFI_STATUS
INTERNAL
Ia64PCIWrite(
    IN EFI_IO_WIDTH             Width,
    IN PCI_CONFIG_ACCESS_CF8    *Pci,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    ); 


EFI_STATUS
DefPciRead (
    IN  EFI_DEVICE_IO_INTERFACE     *This,
    IN  EFI_IO_WIDTH                Width,
    IN  UINT64                      UserAddress,
    IN  UINTN                       Count,
    IN  OUT VOID                    *UserBuffer
    )
{
    return ReadWritePciConfigSpace (This, FALSE, Width, UserAddress, Count, UserBuffer);
}

EFI_STATUS
DefPciWrite (
    IN  EFI_DEVICE_IO_INTERFACE     *This,
    IN  EFI_IO_WIDTH                Width,
    IN  UINT64                      UserAddress,
    IN  UINTN                       Count,
    IN  OUT VOID                    *UserBuffer
    )
{
    return ReadWritePciConfigSpace (This, TRUE, Width, UserAddress, Count, UserBuffer);
}

EFI_STATUS
INTERNAL
ReadWritePciConfigSpace (
    IN  EFI_DEVICE_IO_INTERFACE     *This,
    IN  BOOLEAN                     Write,
    IN  EFI_IO_WIDTH                Width,
    IN  UINT64                      UserAddress,
    IN  UINTN                       Count,
    IN  OUT VOID                    *UserBuffer
    )
{
    PCI_CONFIG_ACCESS_CF8       Pci;
    DEFIO_PCI_ADDR              *Defio;
    UINT8                       Stride;
    IO_DEVICE                   *IoDevice;
    EFI_STATUS                  Status;

    if (Width < 0 || Width > IO_UINT32) {
      return EFI_INVALID_PARAMETER;
    }

    Status = EFI_SUCCESS;

    IoDevice = IO_DEVICE_FROM_THIS(This);

    This = &IoDevice->Io;
    Stride = 1 << Width;

    Defio = (DEFIO_PCI_ADDR *)&UserAddress;
    Pci.Reg = Defio->Register;
    Pci.Func = Defio->Function;
    Pci.Dev = Defio->Device;
    Pci.Bus = Defio->Bus;
    Pci.Reserved = 0;
    Pci.Enable = 1;

    if ((Pci.Func > PCI_MAX_FUNC) || (Pci.Dev > PCI_MAX_DEVICE)) {
        Status = EFI_UNSUPPORTED;
    }

    //
    // PCI Config access are all 32-bit alligned, but by accessing the
    //  CONFIG_DATA_REGISTER (0xcfc) with different widths more cycle types
    //  are possible on PCI.
    //
    // SalProc takes care of reading the proper register depending on stride
    //

    AcquireLock(&IoDevice->PciLock);
    while ((Count) && !EFI_ERROR(Status)) {
        if(!Write) {
            Status = Ia64PCIRead (Width, &Pci, 1, UserBuffer);
            if(EFI_ERROR(Status)) {
                DEBUG((D_ERROR, "ReadWritePciConfigSpace: Error reading PCI register\n"));
                goto Done;
            }
        } else {
            Status = Ia64PCIWrite (Width, &Pci, 1, UserBuffer);
            if(EFI_ERROR(Status)) {
                DEBUG((D_ERROR, "ReadWritePciConfigSpace: Error writing PCI register\n"));
                goto Done;
            }
        }

        UserBuffer = ((UINT8 *)UserBuffer) + Stride;
        Count -= 1;
        Pci.Reg += Stride;
    }

Done:
    ReleaseLock(&IoDevice->PciLock);
    return Status;
}


EFI_STATUS
INTERNAL
Ia64PCIRead(
    IN EFI_IO_WIDTH             Width,
    IN PCI_CONFIG_ACCESS_CF8    *Pci,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    )
{
    UINT64          Address;
    rArg            Return;
    PTR             Buffer;
    UINT64          Stride;
    UINT16          Data16;
    UINT32          Data32;

    ASSERT (Count == 1);

    Buffer.buf  = (UINT8 *)UserBuffer;
    Address     = EFI_PCI_ADDRESS_IA64(0,Pci->Bus, Pci->Dev, Pci->Func, Pci->Reg);

    Stride = 1 << Width;
    SalProc ((UINT64) SAL_PCI_CONFIG_READ,Address,Stride,0,0,0,0,0,&Return);

    if(!Return.p0) {
        switch (Width) {
            case IO_UINT8:
                *Buffer.ui8 = (UINT8)Return.p1;
                break;

            case IO_UINT16:
                if (Buffer.ui & 0x1) {
                    Data16 = (UINT16)Return.p1;
                    *Buffer.ui8++ = Data16 & 0xff;
                    *Buffer.ui8++ = (Data16 >> 8) & 0xff;
                } else {
                    *Buffer.ui16 = (UINT16)Return.p1;
                }
                break;

            case IO_UINT32:
                if (Buffer.ui & 0x3) {
                    Data32 = (UINT32)Return.p1;
                    *Buffer.ui8++ = (UINT8)(Data32 & 0xff);
                    *Buffer.ui8++ = (UINT8)((Data32 >> 8) & 0xff);
                    *Buffer.ui8++ = (UINT8)((Data32 >> 16) & 0xff);
                    *Buffer.ui8++ = (UINT8)((Data32 >> 24) & 0xff);
                } else {
                    *Buffer.ui32 = (UINT32)Return.p1;
                }
                break;

            default:
                ASSERT(FALSE);
                break;
        } // end of switch

        return EFI_SUCCESS;
    } else {
        //
        // BugBug: need to map error returned properly
        //
        return EFI_UNSUPPORTED;
    }
}

EFI_STATUS
INTERNAL
Ia64PCIWrite(
    IN EFI_IO_WIDTH             Width,
    IN PCI_CONFIG_ACCESS_CF8    *Pci,
    IN UINTN                    Count,
    IN OUT VOID                 *UserBuffer
    )
{
    UINT64          Address;
    rArg            Return;
    UINT64          Stride;
    UINT32          Data32;
    UINT8           *Buffer;

    ASSERT (Count == 1);

    Address     = EFI_PCI_ADDRESS_IA64(0, Pci->Bus, Pci->Dev, Pci->Func, Pci->Reg);
    Stride = 1 << Width;

    if ((UINTN)UserBuffer & 0x3) {
        Buffer = (UINT8 *)UserBuffer;
        Data32 = *Buffer++;
        Data32 = Data32 | (*Buffer++ << 8);
        Data32 = Data32 | (*Buffer++ << 16);
        Data32 = Data32 | (*Buffer++ << 24);
    } else {
        Data32 = *(UINT32 *)UserBuffer;
    }

    SalProc ((UINT64) SAL_PCI_CONFIG_WRITE,Address,Stride,Data32,0,0,0,0,&Return);
    
    if(!Return.p0) {
        return EFI_SUCCESS;
    } else  {
        //
        // BugBug: need to map error returned properly
        //
        return EFI_UNSUPPORTED;
    }
}
