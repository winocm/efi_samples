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

    Note these functions are platform specific and are   
    examples for PC-AT style support.  They are only used if they 
    are initialized.



Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "Pldefio.h"

//
//
//

typedef struct {
    EFI_IO_OPERATION_TYPE Operation;
    UINTN                 NumberOfBytes;
    UINTN                 NumberOfPages;
    EFI_PHYSICAL_ADDRESS  HostAddress;
    EFI_PHYSICAL_ADDRESS  MappedHostAddress;
} MAP_INFO;

//
//
//

EFI_STATUS
PciDevicePath (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This,
    IN UINT64                           Address,
    EFI_DEVICE_PATH                     **PciDevicePath
    );

VOID
STATIC 
RUNTIMEFUNCTION
RtDeviceIoSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    );

EFI_STATUS
DefMap (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This,
    IN EFI_IO_OPERATION_TYPE            Operation,
    IN EFI_PHYSICAL_ADDRESS             *HostAddress,
    IN OUT UINTN                        *NumberOfBytes,
    OUT EFI_PHYSICAL_ADDRESS            *DeviceAddress,
    OUT VOID                            **Mapping
    );

EFI_STATUS
DefUnmap (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This,
    IN VOID                             *Mapping
    );

EFI_STATUS
DefAllocateBuffer (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This,
    IN EFI_ALLOCATE_TYPE                Type,
    IN EFI_MEMORY_TYPE                  MemoryType,
    IN UINTN                            Pages,
    IN OUT EFI_PHYSICAL_ADDRESS         *HostAddress
    );

EFI_STATUS
DefFlush (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This
    );

EFI_STATUS
DefFreeBuffer (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This,
    IN UINTN                            Pages,
    OUT EFI_PHYSICAL_ADDRESS            HostAddress
    );

//
//
//

#pragma BEGIN_RUNTIME_DATA()
UINT64  RtVirtualIoBase = 0;
UINT64  RtVirtualFlashBase = 0;

//
//
//


EFI_STATUS
PlInstallDefaultIoDevice (
    IN EFI_DEVICE_PATH          *DevicePath,
    IN UINTN                    MemBase,
    IN UINTN                    IoBase
    )
{
    IO_DEVICE                   *IoDevice;
    EFI_HANDLE                  Handle;
    EFI_STATUS                  Status;
    EFI_EVENT                   Event;

    LegacyDevicePath = NULL;

    IoDevice = AllocateZeroPool (sizeof(IO_DEVICE));
    ASSERT (IoDevice);

    IoDevice->Signature = IO_DEVICE_SIGNATURE;
    IoDevice->DevicePath = DevicePath;

    //
    // Assume we are in Physical Mode
    //
    IoDevice->MemBase = MemBase;
    IoDevice->Io.Mem.Read  = DefMemoryRead;
    IoDevice->Io.Mem.Write = DefMemoryWrite;

    //
    // The Runtime FLASH base is physical until the
    //  OS calls back and registers a virtual address
    //
    // BugBug: The MMIO space can be fragmented by the OS
    //         So this mapping is only good for the FLASH.
    //         NEED to drive this code from a list of 
    //         addresses.
    //
    RtVirtualFlashBase = MemBase | 0xff000000;
    
    IoDevice->IoBase =  IoBase;
    IoDevice->Io.Io.Read = RtDefIoRead;
    IoDevice->Io.Io.Write = RtDefIoWrite;

    //
    // The Runtime IO base is physical until the OS callbacks and
    //  registers a virtual address space for the call.
    //
    RtVirtualIoBase = IoBase;


    InitializeLock (&IoDevice->PciLock, TPL_HIGH_LEVEL);
    IoDevice->PciAddress = 0xCF8;
    IoDevice->PciData    = 0xCFC;

    IoDevice->Io.Pci.Read       = DefPciRead;
    IoDevice->Io.Pci.Write      = DefPciWrite;
    IoDevice->Io.PciDevicePath  = PciDevicePath;
    IoDevice->Io.Map            = DefMap;
    IoDevice->Io.Unmap          = DefUnmap;
    IoDevice->Io.AllocateBuffer = DefAllocateBuffer;
    IoDevice->Io.Flush          = DefFlush;
    IoDevice->Io.FreeBuffer     = DefFreeBuffer;

    Handle = NULL;
    Status = LibInstallProtocolInterfaces (
                &Handle,
                &DeviceIoProtocol,      &IoDevice->Io,
                &DevicePathProtocol,    IoDevice->DevicePath,
                NULL
                );

    ASSERT (!EFI_ERROR(Status));

    //
    // This a runtime nvram device, we need to update any internal
    // pointers for the device if the OS sets the environment into
    // virtual mode
    //

    Status = BS->CreateEvent(
                    EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                    TPL_NOTIFY,
                    RtDeviceIoSetVirtualMapping,
                    NULL,
                    &Event
                    );
    return Status;
}

#pragma RUNTIME_CODE(RtDeviceIoSetVirtualMapping)
VOID
STATIC 
RUNTIMEFUNCTION
RtDeviceIoSetVirtualMapping (
    IN EFI_EVENT            Event,
    IN VOID                 *Context
    )
{
    //
    // Clear BIT63 since it is just set to force non cachable accesses
    //
    RtVirtualIoBase &= 0x7fffffffffffffff;
    RT->ConvertPointer (EFI_INTERNAL_PTR, (VOID **) &RtVirtualIoBase);    

//    RtVirtualFlashBase &= 0x7fffffffffffffff;
//    RT->ConvertPointer (EFI_INTERNAL_PTR, (VOID **) &RtVirtualFlashBase);    
}

EFI_STATUS
DefMap (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This,
    IN EFI_IO_OPERATION_TYPE            Operation,
    IN EFI_PHYSICAL_ADDRESS             *HostAddress,
    IN OUT UINTN                        *NumberOfBytes,
    OUT EFI_PHYSICAL_ADDRESS            *DeviceAddress,
    OUT VOID                            **Mapping
    )

{
    EFI_STATUS Status;
    MAP_INFO   *MapInfo;

    if (Operation < 0 || Operation > EfiBusMasterCommonBuffer) {
        return EFI_INVALID_PARAMETER;
    }

    *Mapping = NULL;
    if ((*HostAddress + *NumberOfBytes) > 0x100000000) {

        if (Operation == EfiBusMasterCommonBuffer) {
            return EFI_UNSUPPORTED;
        }

        Status = BS->AllocatePool(EfiBootServicesData, sizeof(MAP_INFO), &MapInfo);
        if (EFI_ERROR(Status)) {
            *NumberOfBytes = 0;
            return Status;
        }
        *Mapping = MapInfo;

        MapInfo->Operation         = Operation;
        MapInfo->NumberOfBytes     = *NumberOfBytes;
        MapInfo->NumberOfPages     = EFI_SIZE_TO_PAGES(*NumberOfBytes);
        MapInfo->HostAddress       = *HostAddress;
        MapInfo->MappedHostAddress = 0x00000000ffffffff;

        Status = BS->AllocatePages (AllocateMaxAddress, 
                                    EfiBootServicesData, 
                                    MapInfo->NumberOfPages,
                                    &MapInfo->MappedHostAddress);

        if (EFI_ERROR(Status)) {
            BS->FreePool(MapInfo);
            *NumberOfBytes = 0;
            return Status;
        }

        if (Operation == EfiBusMasterRead) {
            CopyMem((VOID *)MapInfo->MappedHostAddress, 
                    (VOID *)MapInfo->HostAddress,
                    MapInfo->NumberOfBytes);
        }

        *DeviceAddress = MapInfo->MappedHostAddress;

    } else {

        *DeviceAddress = *HostAddress;

    }
    return EFI_SUCCESS;
}

EFI_STATUS
DefUnmap (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This,
    IN VOID                             *Mapping
    )

{
    EFI_STATUS Status;
    MAP_INFO   *MapInfo;

    if (Mapping != NULL) {
        MapInfo = (MAP_INFO *)Mapping;
        if (MapInfo->Operation == EfiBusMasterWrite) {
            CopyMem((VOID *)MapInfo->HostAddress, 
                    (VOID *)MapInfo->MappedHostAddress,
                    MapInfo->NumberOfBytes);
        }
        Status = BS->FreePages(MapInfo->MappedHostAddress, MapInfo->NumberOfPages);
        Status = BS->FreePool(Mapping);
    }
    return EFI_SUCCESS;
}

EFI_STATUS
DefAllocateBuffer (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This,
    IN EFI_ALLOCATE_TYPE                Type,
    IN EFI_MEMORY_TYPE                  MemoryType,
    IN UINTN                            Pages,
    IN OUT EFI_PHYSICAL_ADDRESS         *HostAddress
    )

{
    EFI_STATUS Status;

    if (MemoryType != EfiBootServicesData && MemoryType != EfiRuntimeServicesData) {
      return EFI_INVALID_PARAMETER;
    }

    Status = EFI_SUCCESS;

    //
    // Limit allocations to memory below 4GB
    //

    if (Type == AllocateAnyPages)  {
        Type = AllocateMaxAddress;
        *HostAddress = 0x00000000ffffffff;
    }

    if (Type == AllocateMaxAddress) {
        if (*HostAddress > 0x00000000ffffffff) {
            *HostAddress = 0x00000000ffffffff;
        }
    }

    if (Type == AllocateAddress) {
        if (*HostAddress > 0x00000000ffffffff) {
            Status = EFI_UNSUPPORTED;
        }
    }

    if (!EFI_ERROR(Status)) {
        Status = BS->AllocatePages (Type, MemoryType, Pages, HostAddress);
    }
    return Status;
}

EFI_STATUS
DefFlush (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This
    )

{
    return EFI_SUCCESS;
}

EFI_STATUS
DefFreeBuffer (
    IN struct _EFI_DEVICE_IO_INTERFACE  *This,
    IN UINTN                            Pages,
    IN EFI_PHYSICAL_ADDRESS             HostAddress
    )

{
    EFI_STATUS Status;

    Status = BS->FreePages (HostAddress, Pages);
    return Status;
}
