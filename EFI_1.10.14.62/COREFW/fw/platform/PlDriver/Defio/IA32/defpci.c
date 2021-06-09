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
    
      pci.c

Abstract:




Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "Pldefio.h"
#include "pci22.h"

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
    UINT32                      Stride;
    UINTN                       PciData, PciDataStride;
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
    // To read a byte of PCI config space you load 0xcf8 and 
    //  read 0xcfc, 0xcfd, 0xcfe, 0xcff
    //
    PciDataStride = Defio->Register & 0x03;

    AcquireLock(&IoDevice->PciLock);
    while ((Count) && !EFI_ERROR(Status)) {
        IoDevice->Io.Io.Write (This, IO_UINT32, IoDevice->PciAddress, 1, &Pci);
        PciData = IoDevice->PciData + PciDataStride;
        if (Write) {
            IoDevice->Io.Io.Write (This, Width, PciData, 1, UserBuffer);
        } else {
            IoDevice->Io.Io.Read (This, Width, PciData, 1, UserBuffer);
        }
        UserBuffer = ((UINT8 *)UserBuffer) + Stride;
        PciDataStride = (PciDataStride + Stride) % 4;
        Count -= 1;
        Pci.Reg += Stride;
    }
    ReleaseLock(&IoDevice->PciLock);
    return Status;
}
