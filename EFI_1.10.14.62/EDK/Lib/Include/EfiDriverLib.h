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

  EfiDriverLib.h

Abstract:

  Light weight lib to support EFI drivers.

--*/

#ifndef _EFI_DRIVER_LIB_H_
#define _EFI_DRIVER_LIB_H_

#include "EfiCommonLib.h"
#include "LinkedList.h"
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (DriverConfiguration)
#include EFI_PROTOCOL_DEFINITION (DriverDiagnostics)


typedef struct {
  CHAR8   *Language;
  CHAR16  *UnicodeString;
} EFI_UNICODE_STRING_TABLE;


//
// Macros for EFI Driver Library Functions that are really EFI Boot Services
//
#define EfiCopyMem(_Destination, _Source, _Length)  gBS->CopyMem ((_Destination), (_Source), (_Length))
#define EfiSetMem(_Destination, _Length, _Value)   gBS->SetMem  ((_Destination), (_Length), (_Value))
#define EfiZeroMem(_Destination, _Length)          gBS->SetMem  ((_Destination), (_Length), 0)

//
// Driver Lib Globals.
//
extern EFI_BOOT_SERVICES         *gBS;
extern EFI_RUNTIME_SERVICES      *gRT;
extern EFI_SYSTEM_TABLE          *gST;
extern UINTN                     gErrorLevel;

EFI_STATUS
EfiInitializeDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EfiLibInstallDriverBinding (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding,
  IN EFI_HANDLE                   DriverBindingHandle
  );

EFI_STATUS
EfiLibInstallAllDriverProtocols (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL        *DriverBinding,
  IN EFI_HANDLE                         DriverBindingHandle,
  IN EFI_COMPONENT_NAME_PROTOCOL        *ComponentName,
  IN EFI_DRIVER_CONFIGURATION_PROTOCOL  *DriverConfiguration,
  IN EFI_DRIVER_DIAGNOSTICS_PROTOCOL    *DriverDiagnostics
  );

EFI_STATUS
EfiLibGetSystemConfigurationTable (
  IN EFI_GUID *TableGuid,
  IN OUT VOID **Table
  );

//
// DevicePath.c
//

BOOLEAN
EfiIsDevicePathMultiInstance (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  );

EFI_DEVICE_PATH_PROTOCOL *
EfiDevicePathInstance (
  IN OUT EFI_DEVICE_PATH_PROTOCOL   **DevicePath,
  OUT UINTN                         *Size
  );

UINTN
EfiDevicePathSize (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevPath
  );

EFI_DEVICE_PATH_PROTOCOL *
EfiAppendDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *Src1,
  IN EFI_DEVICE_PATH_PROTOCOL  *Src2
  );

EFI_DEVICE_PATH_PROTOCOL *
EfiDevicePathFromHandle (
    IN EFI_HANDLE       Handle
    );

EFI_DEVICE_PATH_PROTOCOL *
EfiDuplicateDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevPath
  );

EFI_DEVICE_PATH_PROTOCOL *
EfiAppendDevicePathNode (
  IN EFI_DEVICE_PATH_PROTOCOL  *Src1,
  IN EFI_DEVICE_PATH_PROTOCOL  *Src2
  );

EFI_DEVICE_PATH_PROTOCOL *
EfiFileDevicePath (
  IN EFI_HANDLE       Device  OPTIONAL,
  IN CHAR16           *FileName
  );

EFI_DEVICE_PATH_PROTOCOL * 
EfiAppendDevicePathInstance (
  IN EFI_DEVICE_PATH_PROTOCOL  *Src,
  IN EFI_DEVICE_PATH_PROTOCOL  *Instance
  );

//
// Lock.c
//

typedef struct _EFI_LOCK {
  EFI_TPL     Tpl;
  EFI_TPL     OwnerTpl;
  UINTN       Lock;
} EFI_LOCK;

#define EFI_INITIALIZE_LOCK_VARIABLE(Tpl) {Tpl,0,0}

VOID
EfiInitializeLock (
  IN OUT EFI_LOCK *Lock,
  IN EFI_TPL      Priority
  );

VOID
EfiAcquireLock (
  IN EFI_LOCK *Lock
  );

EFI_STATUS
EfiAcquireLockOrFail (
  IN EFI_LOCK  *Lock
  );

VOID
EfiReleaseLock (
  IN EFI_LOCK *Lock
  );

VOID  *
EfiLibAllocatePool (
  IN  UINTN   AllocationSize
  );

VOID  *
EfiLibAllocateZeroPool (
  IN  UINTN   AllocationSize
  );

//
// Protocol.c
//

EFI_EVENT
EfiLibCreateProtocolNotifyEvent (
  IN EFI_GUID             *ProtocolGuid,
  IN EFI_TPL              NotifyTpl,
  IN EFI_EVENT_NOTIFY     NotifyFunction,
  IN VOID                 *NotifyContext,
  OUT VOID                *Registration
  );

//
// Debug.c init
//

EFI_STATUS
EfiDebugAssertInit (
  VOID
  );


//
// Unicode String Support
//
EFI_STATUS
EfiLibLookupUnicodeString (
  CHAR8                     *Language,
  CHAR8                     *SupportedLanguages,
  EFI_UNICODE_STRING_TABLE  *UnicodeStringTable,
  CHAR16                    **UnicodeString
  );

EFI_STATUS
EfiLibAddUnicodeString (
  CHAR8                     *Language,
  CHAR8                     *SupportedLanguages,
  EFI_UNICODE_STRING_TABLE  **UnicodeStringTable,
  CHAR16                    *UnicodeString
  );
 
EFI_STATUS
EfiLibFreeUnicodeStringTable (
  EFI_UNICODE_STRING_TABLE  *UnicodeStringTable
  );

#endif
