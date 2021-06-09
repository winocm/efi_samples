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


Abstract:




Revision History

--*/
#ifndef _EMULENV_H_
#define _EMULENV_H_


//
// Externs to initialize FW
//

VOID
EFIEntryPoint (
    IN struct _EFI_PLATFORM_TABLE   *PlTable,
    OUT struct _EFI_FIRMWARE_TABLE  **FwTable,
    OUT EFI_SYSTEM_TABLE            **SystemTable
    );

typedef
VOID
(EFIAPI *EFI_MEMORY_MAP_INSTALLED) (
    VOID
    );

typedef
VOID
(EFIAPI *EFI_NV_STORE_INSTALLED) (
    VOID
    );

typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_DEFAULT_IO_DEVICE) (
    IN EFI_DEVICE_PATH          *DevicePath,
    IN UINTN                    MemBase,
    IN UINTN                    IoBase
    );

typedef
VOID
(EFIAPI *EFI_ADD_MEMORY_DESCRIPTOR) (
    IN EFI_MEMORY_TYPE          Type,
    IN EFI_PHYSICAL_ADDRESS     PhsyicalStart,
    IN UINT64                   NumberOfPages,
    IN UINT64                   Attribute
    );

typedef
EFI_STATUS
(EFIAPI *EFI_LOAD_INTERNAL_DRIVER) (
    IN UINTN                        ImageType,    
    IN CHAR16                       *InternalName,
    IN EFI_IMAGE_ENTRY_POINT        ImageEntryPoint OPTIONAL
    );

typedef
VOID
(EFIAPI *EFI_TICK_HANDLE) (
    IN UINTN                        Duration
    );

//
// Externs to environment emulator
//


typedef
EFI_STATUS
(EFIAPI *PL_EMULATE_LOAD_DRIVER) (
    IN CHAR16                   *InternalName,
    IN OUT EFI_LOADED_IMAGE     *ImageInfo,
    OUT EFI_IMAGE_ENTRY_POINT   *ImageEntryPoint
    );

typedef
VOID
(EFIAPI *PL_IDLE_LOOP) (
    IN BOOLEAN          Polling
    );

typedef
BOOLEAN
(EFIAPI *PL_SET_INTERRUPT_STATE) (
    IN BOOLEAN          Enable
    );

typedef
VOID
(EFIAPI *PL_SET_VIRTUAL_ADDRESS_MAP) (
    IN EFI_CONVERT_POINTER          ConvertPointer,
    IN UINTN                        MemoryMapSize,
    IN UINTN                        DescriptorSize,
    IN UINT32                       DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR        *VirtualMap
    );

typedef
VOID
(EFIAPI *PL_STARTIMAGE_HANDOFF_STATE) (
    VOID
    );

typedef
VOID
(EFIAPI *PL_EXITIMAGE_RETURN_STATE) (
    VOID
    );

typedef
VOID
(EFIAPI *PL_FLUSH_CACHE) (
    IN  VOID    *StartAddress,
    IN  VOID    *EndAddress
    );

//
// The firmware table.  Functions that the firmware provides
// to the emulator
//

typedef struct _EFI_FIRMWARE_TABLE {

    //
    // Initialization time functions
    //

    EFI_MEMORY_MAP_INSTALLED        MemoryMapInstalled;
    EFI_NV_STORE_INSTALLED          NvStoreInstalled;

    EFI_ADD_MEMORY_DESCRIPTOR       AddMemoryDescriptor;    

    //
    // Boot service time functions
    //

    EFI_TICK_HANDLE                 TickHandler;
    EFI_LOAD_INTERNAL_DRIVER        LoadInternal;

} EFI_FIRMWARE_TABLE;


//
// Emulator table.  Functions that the emulator provides to the
// firmware.
//

typedef struct _EFI_PLATFORM_TABLE {

    //
    // Misc platform specific helper functions
    //
    PL_STARTIMAGE_HANDOFF_STATE         SI_HandoffState;
    PL_EXITIMAGE_RETURN_STATE           EI_ReturnState;    
    PL_FLUSH_CACHE                      FlushCache;   

    //
    // Boot service time functions
    //

    PL_IDLE_LOOP                        IdleLoop;
    PL_EMULATE_LOAD_DRIVER              EmulateLoad;

    //
    // Runtime functions
    //

    PL_SET_INTERRUPT_STATE              SetInterruptState;
    PL_SET_VIRTUAL_ADDRESS_MAP          SetVirtualAddressMap;

} EFI_PLATFORM_TABLE;

#endif _EMULENV_H_
