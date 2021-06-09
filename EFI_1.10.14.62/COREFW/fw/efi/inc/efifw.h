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

    efifw.h

Abstract:




Revision History

--*/

#ifndef _EFIFW_H_
#define _EFIFW_H_
//
// Include public headers
//

#include "efi.h"
#include "efistdarg.h"


//
// Include library functions
//

#include "efilib.h"


//
// Include shared function with emulator environment
//

#include "emulenv.h"

//
// FW SetJump / LongJump support
//

#include "efijump.h"

//
// Include debug image info data types and structures
//
#include "DebugImageInfo\DebugImageInfo.h"

//
// Initialize functions
//

VOID
FwMemoryMapInstalled (
    VOID
    );

VOID
FwNvStoreInstalled (
    VOID
    );

VOID
FwStartedBootManager (
    VOID
    );

//
// Exported exec functions
//

VOID
InitializeEvent (
    VOID
    );

EFI_STATUS
BOOTSERVICE
CreateEvent (
    IN UINT32               Type,
    IN EFI_TPL              NotifyTpl,
    IN EFI_EVENT_NOTIFY     NotifyFunction,
    IN VOID                 *NotifyContext,
    OUT EFI_EVENT           *pEvent
    );

EFI_STATUS
BOOTSERVICE
SetTimer (
    IN EFI_EVENT            Event,
    IN EFI_TIMER_DELAY      Type,
    IN UINT64               TriggerTime
    );

EFI_STATUS
BOOTSERVICE
SignalEvent (
    IN EFI_EVENT            Event
    );

EFI_STATUS
BOOTSERVICE
WaitForEvent (
    IN UINTN        NumberOfEvents,
    IN EFI_EVENT    *UserEvents,
    OUT UINTN       *UserIndex
    );

EFI_STATUS
BOOTSERVICE
CloseEvent (
    IN EFI_EVENT            Event
    );

EFI_STATUS
BOOTSERVICE
CheckEvent (
    IN EFI_EVENT            Event
    );

VOID
RUNTIMEFUNCTION
RtNotifySignalList (
    IN UINTN                SignalType
    );

VOID
RUNTIMEFUNCTION
RtEventVirtualAddressFixup(
    VOID
    );

VOID
InitializeTimer (
    VOID
    );

VOID
BOOTSERVICE
FwTimerTick (
    IN UINTN        Duration
    );

VOID
InitializeMonotonicCount (
    VOID
    );

//
// Exported loader functions
//

VOID
InitializeLoader (
    VOID
    );

EFI_STATUS
LoadImage (
    IN BOOLEAN                      BootPolicy,
    IN EFI_HANDLE                   ParentImageHandle,
    IN EFI_DEVICE_PATH              *FilePath,
    IN VOID                         *SourceBuffer   OPTIONAL,
    IN UINTN                        SourceSize,
    OUT EFI_HANDLE                  *ImageHandle
    );

EFI_STATUS
LoadBootImage (
    IN BOOLEAN                      BootImage,
    IN EFI_HANDLE                   ParentImageHandle,
    IN EFI_HANDLE                   DeviceHandle,
    IN EFI_DEVICE_PATH              *FilePath,
    IN VOID                         *SourceBuffer   OPTIONAL,
    IN UINTN                        SourceSize,
    OUT EFI_HANDLE                  *ImageHandle
    );

EFI_STATUS
FwLoadInternal (
    IN UINTN                        ImageType,    
    IN CHAR16                       *InternalName,
    IN EFI_IMAGE_ENTRY_POINT        ImageEntryPoint OPTIONAL
    );

EFI_STATUS
StartImage (
    IN EFI_HANDLE                   ImageHandle,
    OUT UINTN                       *ExitDataSize,
    OUT CHAR16                      **ExitData  OPTIONAL
    );

EFI_STATUS
Exit (
    IN EFI_HANDLE                   ImageHandle,
    IN EFI_STATUS                   ExitStatus,
    IN UINTN                        DataSize,
    IN CHAR16                       *ExitDescription
    );

EFI_STATUS
BSUnloadImage (
    IN EFI_HANDLE                   ImageHandle
    );

EFI_STATUS
ExitBootServices (
    IN EFI_HANDLE                   ImageHandle,
    IN UINTN                        MapKey
    );

VOID
RUNTIMEFUNCTION
RtLoaderExitBootServices (
    VOID
    );

VOID
RUNTIMEFUNCTION
RtLoaderVirtualAddressFixup (
    VOID
    );

EFI_STATUS
InstallConfigurationTable (
    IN EFI_GUID *Guid,
    IN VOID     *Table
    );


//
// Exported task proirity level functions
//

EFI_TPL
CurrentTPL (
    VOID
    );

EFI_TPL
BOOTSERVICE
RaiseTPL (
    IN EFI_TPL  NewTpl
    );

VOID
BOOTSERVICE
RestoreTPL (
    IN EFI_TPL  NewTpl
    );


//
// Monotonic counter
//


EFI_STATUS
GetNextMonotonicCount (
    OUT UINT64                  *Count
    );

EFI_STATUS
GetNextHighMonotonicCount (
    OUT UINT32                  *HighCount
    );

//
// Handle & Protocol functions
//

VOID
InitializeHandle (
    VOID
    );

EFI_STATUS
BOOTSERVICE
InstallProtocolInterface (
    IN OUT EFI_HANDLE           *UserHandle,
    IN EFI_GUID                 *Protocol,
    IN EFI_INTERFACE_TYPE       InterfaceType,
    IN VOID                     *Interface
    );

EFI_STATUS
BOOTSERVICE
ReinstallProtocolInterface (
    IN EFI_HANDLE               UserHandle,
    IN EFI_GUID                 *Protocol,
    IN VOID                     *OldInterface,
    IN VOID                     *NewInterface
    );

EFI_STATUS
BOOTSERVICE
UninstallProtocolInterface (
    IN EFI_HANDLE               UserHandle,
    IN EFI_GUID                 *Protocol,
    IN VOID                     *Interface
    );

EFI_STATUS
BOOTSERVICE
HandleProtocol (
    IN EFI_HANDLE               Handle,
    IN EFI_GUID                 *Protocol,
    OUT VOID                    **Interface
    );

EFI_STATUS
BOOTSERVICE
RegisterProtocolNotify (
    IN EFI_GUID                 *Protocol,
    IN EFI_EVENT                Event,
    IN VOID                     **Registration
    );

EFI_STATUS
BOOTSERVICE
LocateHandle (
    IN EFI_LOCATE_SEARCH_TYPE   SearchType,
    IN EFI_GUID                 *Protocol OPTIONAL,
    IN VOID                     *SearchKey OPTIONAL,
    IN OUT UINTN                *BufferSize,
    OUT EFI_HANDLE              *Buffer
    );

EFI_STATUS
BOOTSERVICE
LocateDevicePath (
    IN EFI_GUID             *Protocol,
    IN OUT EFI_DEVICE_PATH  **FilePath,
    OUT EFI_HANDLE          *Device
    );

//
// Memory functions
//

VOID
InitializeMemoryMap (
    VOID
    );

VOID
InitializeMemoryMapWatermarks (
  VOID
  );

VOID
FwAddMemoryDescriptor (
    IN EFI_MEMORY_TYPE          Type,
    IN EFI_PHYSICAL_ADDRESS     Start,
    IN UINT64                   NoPages,
    IN UINT64                   Attribute
    );

EFI_STATUS
BOOTSERVICE
BootServiceAllocatePages (
    IN EFI_ALLOCATE_TYPE            Type,
    IN EFI_MEMORY_TYPE              MemoryType,
    IN UINTN                        NoPages,
    OUT EFI_PHYSICAL_ADDRESS        *Memory
    );

EFI_STATUS
AllocatePages (
    IN EFI_ALLOCATE_TYPE            Type,
    IN EFI_MEMORY_TYPE              MemoryType,
    IN UINTN                        NoPages,
    OUT EFI_PHYSICAL_ADDRESS        *Memory
    );

EFI_STATUS
GetMemoryMap (
    IN OUT UINTN                    *MemoryMapSize,
    IN OUT EFI_MEMORY_DESCRIPTOR    *MemoryMap,
    OUT UINTN                       *MapKey,
    OUT UINTN                       *DescriptorSize,
    OUT UINT32                      *DescriptorVersion
    );

EFI_STATUS 
BOOTSERVICE
FreePages (
    IN EFI_PHYSICAL_ADDRESS         Memory,
    IN UINTN                        NoPages
    );

EFI_STATUS
BOOTSERVICE
BootServiceAllocatePool (
    IN EFI_MEMORY_TYPE              PoolType,
    IN UINTN                        Size,
    OUT VOID                        **Buffer
    );

EFI_STATUS
BOOTSERVICE
BSAllocatePool (
    IN EFI_MEMORY_TYPE              PoolType,
    IN UINTN                        Size,
    OUT VOID                        **Buffer
    );

EFI_STATUS
BOOTSERVICE
BSFreePool (
    IN VOID                         *Buffer
    );

EFI_STATUS
SetCodeSection (
    IN EFI_PHYSICAL_ADDRESS         Base,
    IN UINTN                        NoPages,
    IN EFI_MEMORY_TYPE              MemoryType
    );

    
EFI_STATUS
TerminateMemoryMap (
    IN UINTN                        MapKey
    );

EFI_STATUS
RUNTIMEFUNCTION
RtSetVirtualAddressMap (
    IN UINTN                        MemoryMapSize,
    IN UINTN                        DescriptorSize,
    IN UINT32                       DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR        *VirtualMap
    );

EFI_STATUS
RUNTIMEFUNCTION
RtConvertPointer (
    IN UINTN                        DebugDisposition,    
    IN OUT VOID                     **Address
    );

//
// Variable functions
//

VOID
InitializeVariableStore (
    VOID
    );

VOID
InitializeBootServiceVariableStore (
    VOID
    );

EFI_STATUS
RUNTIMESERVICE
RtGetVariable (
    IN CHAR16                       *VariableName,
    IN EFI_GUID                     *VendorGuid,
    OUT UINT32                      *Attributes OPTIONAL,
    IN OUT UINTN                    *DataSize,
    OUT VOID                        *Data
    );

EFI_STATUS
RUNTIMESERVICE
RtGetNextVariableName (
    IN OUT UINTN                    *VariableNameSize,
    IN OUT CHAR16                   *VariableName,
    IN OUT EFI_GUID                 *VendorGuid
    );

EFI_STATUS
RUNTIMESERVICE
RtSetVariable (
    IN CHAR16                       *VariableName,
    IN EFI_GUID                     *VendorGuid,
    IN UINT32                       Attributes,
    IN UINTN                        VariableSize,
    IN VOID                         *Variable
    );

//
// EFI 1.1 Services
//

EFI_STATUS 
EFIAPI
ConnectController (
  IN  EFI_HANDLE                ControllerHandle,
  IN  EFI_HANDLE                *DriverImageHandle    OPTIONAL,
  IN  EFI_DEVICE_PATH           *RemainingDevicePath  OPTIONAL,
  IN  BOOLEAN                   Recursive
  );

EFI_STATUS 
DisconnectController (
  IN  EFI_HANDLE  ControllerHandle,
  IN  EFI_HANDLE  DriverImageHandle, OPTIONAL
  IN  EFI_HANDLE  ChildHandle        OPTIONAL
  );

EFI_STATUS
BOOTSERVICE
OpenProtocol (
  IN  EFI_HANDLE                UserHandle,
  IN  EFI_GUID                  *Protocol,
  OUT VOID                      **Interface,
  IN  EFI_HANDLE                ImageHandle,
  IN  EFI_HANDLE                ControllerHandle,
  IN  UINT32                    Attributes
  );

EFI_STATUS
BOOTSERVICE
CloseProtocol (
  IN  EFI_HANDLE                UserHandle,
  IN  EFI_GUID                  *Protocol,
  IN  EFI_HANDLE                ImageHandle,
  IN  EFI_HANDLE                ControllerHandle  OPTIONAL
  );

EFI_STATUS
BOOTSERVICE
OpenProtocolInformation (
  IN  EFI_HANDLE                          UserHandle,
  IN  EFI_GUID                            *Protocol,
  IN  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer,
  OUT UINTN                               *EntryCount
  );

EFI_STATUS
BOOTSERVICE
ProtocolsPerHandle (
  IN EFI_HANDLE       UserHandle,
  OUT EFI_GUID        ***ProtocolBuffer,
  OUT UINTN           *ProtocolBufferCount
  );

EFI_STATUS
BOOTSERVICE
LocateHandleBuffer (
  IN EFI_LOCATE_SEARCH_TYPE       SearchType,
  IN EFI_GUID                     *Protocol OPTIONAL,
  IN VOID                         *SearchKey OPTIONAL,
  IN OUT UINTN                    *NumberHandles,
  OUT EFI_HANDLE                  **Buffer
  );

EFI_STATUS
BOOTSERVICE
LocateProtocol (
  EFI_GUID  *Protocol,
  VOID      *Registration, OPTIONAL
  VOID      **Interface
  );

EFI_STATUS
BOOTSERVICE
InstallMultipleProtocolInterfaces (
  IN OUT EFI_HANDLE           *Handle,
  ...
  );

EFI_STATUS
BOOTSERVICE
UninstallMultipleProtocolInterfaces (
  IN EFI_HANDLE           Handle,
  ...
  );

EFI_STATUS
RUNTIMEFUNCTION
RtCalculateCrc32 (
    VOID   *pt,
    UINTN  Size,
    UINT32 *Crc
    );

VOID
BOOTSERVICE
EfiCoreCopyMem (
  VOID *Dest,
  VOID *Src,
  UINTN Length
  );

VOID
BOOTSERVICE
EfiCoreSetMem (
  VOID *Dest,
  UINTN Length,
  UINT8 Value
  );

UINT64
CoreGetHandleDatabaseKey (
  );

VOID
CoreConnectHandlesByKey (
  UINT64  Key
  );

//
// Default IO driver
//

EFI_STATUS
FwInstallDefaultIoDevice (
    IN EFI_DEVICE_PATH          *DevicePath,
    IN UINTN                    MemBase,
    IN UINTN                    IoBase
    );

//
// Externs
//

extern BOOLEAN EfiAtRuntime;
extern BOOLEAN EfiVirtualMode;
extern EFI_PLATFORM_TABLE   *PL;
extern INTERNAL PL_SET_INTERRUPT_STATE EfiSetInterruptState;

//
// Extern from lib
//

extern BOOLEAN LibFwInstance;
extern SIMPLE_TEXT_OUTPUT_INTERFACE    *LibRuntimeDebugOut;
extern EFI_RAISE_TPL                    LibRuntimeRaiseTPL;
extern EFI_RESTORE_TPL                  LibRuntimeRestoreTPL;

#endif _EFIFW_H_
