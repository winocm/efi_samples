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

    PlatformLib.c

Abstract:

Revision History

--*/

#include "efi.h"
#include "efilib.h"

#include EFI_GUID_DEFINITION (PrimaryConsoleInDevice)
#include EFI_GUID_DEFINITION (PrimaryConsoleOutDevice)
#include EFI_GUID_DEFINITION (PrimaryStandardErrorDevice)


EFI_STATUS
ConnectDevicePath (
  EFI_DEVICE_PATH  *DevicePathToConnect
  )

{
  EFI_STATUS       Status;
  EFI_DEVICE_PATH  *DevicePath;
  EFI_DEVICE_PATH  *OriginalDevicePath;
  EFI_DEVICE_PATH  *Instance;
  EFI_DEVICE_PATH  *RemainingDevicePath;
  EFI_DEVICE_PATH  *Next;
  EFI_HANDLE       Handle;
  EFI_HANDLE       PreviousHandle;
  UINTN            Size;

  if (DevicePathToConnect == NULL) {
    return EFI_SUCCESS;
  }

  DevicePath         = DuplicateDevicePath (DevicePathToConnect);
  OriginalDevicePath = DevicePath;

  if (DevicePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  do {

    Instance = DevicePathInstance (&DevicePath, &Size);

    Next = Instance;
    while (!IsDevicePathEndType(Next)) {
      Next = NextDevicePathNode(Next);
    }
    SetDevicePathEndNode(Next);

    PreviousHandle = NULL;
    do {
      //
      // Find the handle that best matches the Device Path. If it is only a 
      // partial match the remaining part of the device path is returned in
      // RemainingDevicePath.
      //
      RemainingDevicePath = Instance;
      Status = BS->LocateDevicePath (
                      &DevicePathProtocol,
                      &RemainingDevicePath,
                      &Handle
                      );
      if (!EFI_ERROR(Status)) {

        if (Handle == PreviousHandle) {
          Status = EFI_NOT_FOUND;
        } else {
          PreviousHandle = Handle;
          //
          // Connect all drivers that apply to Handle and RemainingDevicePath
          // If no drivers are connected Handle, then return EFI_NOT_FOUND
          // The Recursive flag is FALSE so only one level will be expanded.
          //
          Status = BS->ConnectController (
                          Handle, 
                          NULL, 
                          RemainingDevicePath, 
                          FALSE
                          );
        }
      }

      //
      // Loop until RemainingDevicePath is an empty device path
      //
    } while (!EFI_ERROR (Status) && !IsDevicePathEnd (RemainingDevicePath));

  } while (DevicePath != NULL);

  if (OriginalDevicePath != NULL) {
    FreePool(OriginalDevicePath);
  }

  //
  // A handle with DevicePath exists in the handle database
  //
  return EFI_SUCCESS;
}

VOID
ConnectConsole (
  CHAR16      *VariableName
  )
{
  EFI_DEVICE_PATH  *DevicePath;

  DevicePath = LibGetVariable (VariableName, &EfiGlobalVariable);
  if (DevicePath != NULL) {
    ConnectDevicePath (DevicePath);
    FreePool (DevicePath);
  }
}

VOID
ConnectAllDriversToAllControllers (
    VOID
    )

{
  EFI_STATUS  Status;
  UINTN       AllHandleCount;
  EFI_HANDLE  *AllHandleBuffer;
  UINTN       Index;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINT32      *HandleType;
  UINTN       HandleIndex;
  BOOLEAN     Parent;
  BOOLEAN     Device;

  Status = LibLocateHandle (
             AllHandles,
             NULL,
             NULL,
             &AllHandleCount,
             &AllHandleBuffer
             );
  if (EFI_ERROR (Status)) {
    return;
  }

  for (Index = 0; Index < AllHandleCount; Index++) {

    //
    // Scan the handle database
    //
    Status = LibScanHandleDatabase (
               NULL,
               NULL,
               AllHandleBuffer[Index],
               NULL,
               &HandleCount, 
               &HandleBuffer, 
               &HandleType
               );
    if (EFI_ERROR (Status)) {
      return;
    }

    Device = TRUE;
    if (HandleType[Index] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE) {
      Device = FALSE;
    }
    if (HandleType[Index] & EFI_HANDLE_TYPE_IMAGE_HANDLE) {
      Device = FALSE;
    }

    if (Device) {
      Parent = FALSE;
      for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
          Parent = TRUE;
        }
      }

      if (!Parent) {
        if (HandleType[Index] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
	        Status = BS->ConnectController (
			  		             AllHandleBuffer[Index],
				  	             NULL,
					               NULL,
					               TRUE
					               );
        }
      }
    }

    BS->FreePool (HandleBuffer);
    BS->FreePool (HandleType);
  }

  BS->FreePool (AllHandleBuffer);
}

VOID
DisconnectAll (
    VOID
    )

{
  EFI_STATUS  Status;
  UINTN       AllHandleCount;
  EFI_HANDLE  *AllHandleBuffer;
  UINTN       Index;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINT32      *HandleType;
  UINTN       HandleIndex;
  BOOLEAN     Parent;
  BOOLEAN     Device;

  Status = LibLocateHandle (
             AllHandles,
             NULL,
             NULL,
             &AllHandleCount,
             &AllHandleBuffer
             );
  if (EFI_ERROR (Status)) {
    return;
  }

  for (Index = 0; Index < AllHandleCount; Index++) {

    //
    // Scan the handle database
    //
    Status = LibScanHandleDatabase (
               NULL,
               NULL,
               AllHandleBuffer[Index],
               NULL,
               &HandleCount, 
               &HandleBuffer, 
               &HandleType
               );
    if (EFI_ERROR (Status)) {
      return;
    }

    Device = TRUE;
    if (HandleType[Index] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE) {
      Device = FALSE;
    }
    if (HandleType[Index] & EFI_HANDLE_TYPE_IMAGE_HANDLE) {
      Device = FALSE;
    }

    if (Device) {
      Parent = FALSE;
      for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
          Parent = TRUE;
        }
      }

      if (!Parent) {
        if (HandleType[Index] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
	        Status = BS->DisconnectController (
			  		             AllHandleBuffer[Index],
				  	             NULL,
					               NULL
					               );
        }
      }
    }

    BS->FreePool (HandleBuffer);
    BS->FreePool (HandleType);
  }

  BS->FreePool (AllHandleBuffer);
}

VOID
ConnectAllConsoles (
    VOID
    )

{
  BOOLEAN          Connect;
  EFI_DEVICE_PATH  *DevicePath;

  Connect = FALSE;

  DevicePath = LibGetVariable (VarConsoleOut, &EfiGlobalVariable);
  if (DevicePath != NULL) {
    FreePool (DevicePath);
  } else {
    Connect = TRUE;
  }

  DevicePath = LibGetVariable (VarConsoleInp, &EfiGlobalVariable);
  if (DevicePath != NULL) {
    FreePool (DevicePath);
  } else {
    Connect = TRUE;
  }

  if (Connect) {
    ConnectAllDriversToAllControllers ();
  }

  ConnectConsole (VarErrorOut);

  ConnectConsole (VarConsoleOut);

  ConnectConsole (VarConsoleInp);
}

EFI_STATUS
ConnectAll (
    VOID
    )

{
  ConnectAllDriversToAllControllers();
  ConnectAllConsoles();
  return EFI_SUCCESS;
}

