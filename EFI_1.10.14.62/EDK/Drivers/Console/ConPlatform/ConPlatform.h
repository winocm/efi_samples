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

    ConPlatform.h
    
Abstract:

--*/
#ifndef CON_MANAGE_H_
#define CON_MANAGE_H_


#include "Efi.h"
#include "EfiDriverLib.h"

//
// Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)
#include EFI_GUID_DEFINITION     (GlobalVariable)

//
// Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_GUID_DEFINITION     (ConsoleInDevice)
#include EFI_GUID_DEFINITION     (ConsoleOutDevice)
#include EFI_GUID_DEFINITION     (StandardErrorDevice)
#include EFI_GUID_DEFINITION     (HotPlugDevice)

//
//
//
#define VarConsoleInpDev L"ConInDev"
#define VarConsoleInp    L"ConIn"
#define VarConsoleOutDev L"ConOutDev"
#define VarConsoleOut    L"ConOut"
#define VarErrorOutDev   L"ErrOutDev"
#define VarErrorOut      L"ErrOut"

//
// Global variables
//

extern EFI_DRIVER_BINDING_PROTOCOL gConPlatformTextInDriverBinding;
extern EFI_DRIVER_BINDING_PROTOCOL gConPlatformTextOutDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gConPlatformComponentName;

//
// Function Prototypes
//
EFI_STATUS
ConPlatformDriverEntry (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

STATIC
EFI_STATUS
EFIAPI
ConPlatformTextInDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

STATIC
EFI_STATUS
EFIAPI
ConPlatformTextOutDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

STATIC
EFI_STATUS
EFIAPI
ConPlatformTextInDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  );

STATIC
EFI_STATUS
EFIAPI
ConPlatformTextOutDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  );

STATIC
EFI_STATUS
EFIAPI
ConPlatformDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

VOID *
ConPlatformGetVariableAndSize (
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize
  );
  
EFI_STATUS
ConPlatformMatchDevicePaths (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Multi,
  IN  EFI_DEVICE_PATH_PROTOCOL  *Single,
  IN  EFI_DEVICE_PATH_PROTOCOL  **NewDevicePath  OPTIONAL,
  IN  BOOLEAN                   Delete
  );

EFI_STATUS
ConPlatformCheckVariable (
  IN  CHAR16                    *VariableName,
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  );

EFI_STATUS
ConPlatformUpdateDeviceVariable (
  IN  CHAR16                    *VariableName,
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  IN  BOOLEAN                   Append
  );
  
BOOLEAN
IsHotPlugDevice (
  EFI_HANDLE    DriverBindingHandle,
  EFI_HANDLE    ControllerHandle
  );

#endif
