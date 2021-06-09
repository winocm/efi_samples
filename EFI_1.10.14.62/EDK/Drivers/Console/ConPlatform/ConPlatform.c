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

    ConPlatform.c
    
Abstract:

--*/


#include "ConPlatform.h"

EFI_DRIVER_BINDING_PROTOCOL gConPlatformTextInDriverBinding = {
  ConPlatformTextInDriverBindingSupported,
  ConPlatformTextInDriverBindingStart,
  ConPlatformDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_BINDING_PROTOCOL gConPlatformTextOutDriverBinding = {
  ConPlatformTextOutDriverBindingSupported,
  ConPlatformTextOutDriverBindingStart,
  ConPlatformDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT (ConPlatformDriverEntry)

EFI_STATUS
ConPlatformDriverEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 
  EFI_STATUS

--*/
{
  EFI_STATUS Status;

  //
  // Install driver binding protocols and component name protocols
  //
  
  Status = EfiLibInstallAllDriverProtocols (
             ImageHandle, 
             SystemTable, 
             &gConPlatformTextOutDriverBinding, 
             ImageHandle,
             &gConPlatformComponentName,
             NULL,
             NULL
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gConPlatformTextInDriverBinding, 
           NULL,
           &gConPlatformComponentName,
           NULL,
           NULL
           );
}

STATIC
EFI_STATUS
EFIAPI
ConPlatformTextInDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:
  Supported 

Arguments:
  (Standard DriverBinding Protocol Supported() function)

Returns:

  None

--*/
{
  EFI_STATUS                   Status;
  EFI_SIMPLE_TEXT_IN_PROTOCOL  *TextIn; 

  //
  // Test to see if this is a physical device by checking to see 
  // if it has a Device Path Protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiDevicePathProtocolGuid, 
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle, 
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Test to see if this device supports the Simple Input Protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiSimpleTextInProtocolGuid, 
                  (VOID **)&TextIn,
                  This->DriverBindingHandle,
                  ControllerHandle, 
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
         ControllerHandle, 
         &gEfiSimpleTextInProtocolGuid, 
         This->DriverBindingHandle,
         ControllerHandle 
         );

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ConPlatformTextOutDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:
  Supported 

Arguments:
  (Standard DriverBinding Protocol Supported() function)

Returns:

  None

--*/
{
  EFI_STATUS                    Status;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *TextOut;

  //
  // Test to see if this is a physical device by checking to see if 
  // it has a Device Path Protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiDevicePathProtocolGuid, 
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle, 
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Test to see if this device supports the Simple Text Output Protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiSimpleTextOutProtocolGuid, 
                  (VOID **)&TextOut,
                  This->DriverBindingHandle,
                  ControllerHandle, 
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
         ControllerHandle, 
         &gEfiSimpleTextOutProtocolGuid, 
         This->DriverBindingHandle,
         ControllerHandle 
         );

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ConPlatformTextInDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
/*++

Routine Description:


Arguments:
  (Standard DriverBinding Protocol Start() function)

Returns:


--*/
{
  EFI_STATUS                    Status;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath; 
  EFI_SIMPLE_TEXT_IN_PROTOCOL   *TextIn; 

  //
  // Get the Device Path Protocol so the environment variables can be updated
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiDevicePathProtocolGuid, 
                  (VOID **)&DevicePath,
                  This->DriverBindingHandle,
                  ControllerHandle, 
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open the Simple Input Protocol BY_DRIVER
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiSimpleTextInProtocolGuid, 
                  (VOID **)&TextIn,
                  This->DriverBindingHandle,
                  ControllerHandle, 
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Check the device handle, if it is a hot plug device,
  // do not put the device path into ConInDev, and install
  // gEfiConsoleInDeviceGuid to the device handle directly.
  // The policy is, make hot plug device plug in and play immediately.
  //
  if (IsHotPlugDevice (This->DriverBindingHandle,ControllerHandle)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &ControllerHandle,
                    &gEfiConsoleInDeviceGuid, NULL,
                    NULL
                    );
  } else {
    //
    // Append the device path to the ConInDev environment variable
    //
    Status = ConPlatformUpdateDeviceVariable (VarConsoleInpDev,DevicePath,TRUE);
  
    //
    // If the device path is an instance in the ConIn environment variable, 
    // then install EfiConsoleInDeviceGuid onto ControllerHandle
    //
    Status = ConPlatformCheckVariable (VarConsoleInp, DevicePath);
    if (!EFI_ERROR (Status)) {
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &ControllerHandle,
                      &gEfiConsoleInDeviceGuid, NULL,
                      NULL
                      );
    } else {
      gBS->CloseProtocol (
                  ControllerHandle, 
                  &gEfiSimpleTextInProtocolGuid, 
                  This->DriverBindingHandle,
                  ControllerHandle 
                  );
    }
  }
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ConPlatformTextOutDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
/*++

Routine Description:


Arguments:
  (Standard DriverBinding Protocol Start() function)

Returns:


--*/
{
  EFI_STATUS                    Status;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath; 
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *TextOut;
  
  BOOLEAN       NeedClose;
  
  NeedClose = TRUE;

  //
  // Get the Device Path Protocol so the environment variables can be updated
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiDevicePathProtocolGuid, 
                  (VOID **)&DevicePath,
                  This->DriverBindingHandle,
                  ControllerHandle, 
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open the Simple Text Output Protocol BY_DRIVER
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiSimpleTextOutProtocolGuid, 
                  (VOID **)&TextOut,
                  This->DriverBindingHandle,
                  ControllerHandle, 
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Check the device handle, if it is a hot plug device,
  // do not put the device path into ConOutDev and StdErrDev,
  // and install gEfiConsoleOutDeviceGuid to the device handle directly.
  // The policy is, make hot plug device plug in and play immediately.
  //
  if (IsHotPlugDevice (This->DriverBindingHandle,ControllerHandle)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                      &ControllerHandle,
                      &gEfiConsoleOutDeviceGuid, NULL,
                      NULL
                      );
  } else {
    //
    // Append the device path to the ConOutDev environment variable
    //
    Status = ConPlatformUpdateDeviceVariable (
                                  VarConsoleOutDev, 
                                  DevicePath, 
                                  TRUE
                                  );  
    //
    // Append the device path to the StdErrDev environment variable
    //
    Status = ConPlatformUpdateDeviceVariable (VarErrorOutDev, DevicePath, TRUE);
  
    //
    // If the device path is an instance in the ConOut environment variable, 
    // then install EfiConsoleOutDeviceGuid onto ControllerHandle
    //
    Status = ConPlatformCheckVariable (VarConsoleOut, DevicePath);
    if (!EFI_ERROR (Status)) {
      NeedClose = FALSE;
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &ControllerHandle,
                      &gEfiConsoleOutDeviceGuid, NULL,
                      NULL
                      );
    }
    
    //
    // If the device path is an instance in the StdErr environment variable,
    // then install EfiStandardErrorDeviceGuid onto ControllerHandle
    //
    Status = ConPlatformCheckVariable (VarErrorOut, DevicePath);
    if (!EFI_ERROR (Status)) {
      NeedClose = FALSE;
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &ControllerHandle,
                      &gEfiStandardErrorDeviceGuid, NULL,
                      NULL
                      );
    }
    
    if (NeedClose) {
      gBS->CloseProtocol (
                  ControllerHandle, 
                  &gEfiSimpleTextOutProtocolGuid, 
                  This->DriverBindingHandle,
                  ControllerHandle 
                  );
    }    
  }
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
ConPlatformDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS                    Status;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  
  //
  // hot plug device is not included into the console associated variables,
  // so no need to check variable for those hot plug devices.
  //
  if (!IsHotPlugDevice (This->DriverBindingHandle,ControllerHandle)) {

    //
    // Get the Device Path Protocol so the environment variables can be updated
    //
    Status = gBS->OpenProtocol (
                    ControllerHandle, 
                    &gEfiDevicePathProtocolGuid, 
                    (VOID **)&DevicePath,
                    This->DriverBindingHandle,
                    ControllerHandle, 
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {
      //
      // Remove DevicePath from ConInDev, ConOutDev, and StdErrDev
      //
      Status = ConPlatformUpdateDeviceVariable (
                                  VarConsoleInpDev,
                                  DevicePath,
                                  FALSE
                                  );
      Status = ConPlatformUpdateDeviceVariable (
                                  VarConsoleOutDev,
                                  DevicePath,
                                  FALSE
                                  );
      Status = ConPlatformUpdateDeviceVariable (
                                  VarErrorOutDev,
                                  DevicePath,
                                  FALSE
                                  );
    }
  }
  
  //
  // Uninstall the Console Device GUIDs from Controller Handle
  //
  Status = gBS->OpenProtocol (
                ControllerHandle, 
                &gEfiConsoleInDeviceGuid, 
                NULL,
                This->DriverBindingHandle,
                ControllerHandle, 
                EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                );
  if (!EFI_ERROR (Status)) {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                    ControllerHandle,
                    &gEfiConsoleInDeviceGuid, NULL,
                    NULL
                    );
  }

  Status = gBS->OpenProtocol (
                ControllerHandle, 
                &gEfiConsoleOutDeviceGuid, 
                NULL,
                This->DriverBindingHandle,
                ControllerHandle, 
                EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                );
  if (!EFI_ERROR (Status)) {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                    ControllerHandle,
                    &gEfiConsoleOutDeviceGuid, NULL,
                    NULL
                    );
  }

  Status = gBS->OpenProtocol (
                ControllerHandle, 
                &gEfiStandardErrorDeviceGuid, 
                NULL,
                This->DriverBindingHandle,
                ControllerHandle, 
                EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                );
  if (!EFI_ERROR (Status)) {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                    ControllerHandle,
                    &gEfiStandardErrorDeviceGuid, NULL,
                    NULL
                    );
  }

  //
  // Close the Simple Input and Simple Text Output Protocols
  //
  Status = gBS->CloseProtocol (
                  ControllerHandle, 
                  &gEfiSimpleTextInProtocolGuid, 
                  This->DriverBindingHandle,
                  ControllerHandle 
                  );

  Status = gBS->CloseProtocol (
                  ControllerHandle, 
                  &gEfiSimpleTextOutProtocolGuid, 
                  This->DriverBindingHandle,
                  ControllerHandle 
                  );

  return EFI_SUCCESS;
}

VOID *
ConPlatformGetVariableAndSize (
  IN  CHAR16    *Name,
  IN  EFI_GUID  *VendorGuid,
  OUT UINTN     *VariableSize
  )
/*++

Routine Description:
  Read the EFI variable (VendorGuid/Name) and return a dynamically allocated
  buffer, and the size of the buffer. On failure return NULL.

Arguments:
  Name       - String part of EFI variable name

  VendorGuid - GUID part of EFI variable name

  VariableSize - Returns the size of the EFI variable that was read

Returns:
  Dynamically allocated memory that contains a copy of the EFI variable.
  Caller is repsoncible freeing the buffer.

  NULL - Variable was not read
  
--*/
{
  EFI_STATUS  Status;
  VOID        *Buffer;

  //
  // Allocate a small buffer to test if the variable exists.
  //
  *VariableSize = 1;
  Status = gBS->AllocatePool (EfiBootServicesData, *VariableSize, &Buffer); 
  if (EFI_ERROR (Status)) {
    *VariableSize = 0;
    return NULL;
  }
  
  //
  // Test to see if the variable exists.  If it doesn't then free Buffer
  // and return NULL.
  //
  Status = gRT->GetVariable (Name, VendorGuid, NULL, VariableSize, Buffer);
  
  if (Status == EFI_SUCCESS) {
    return Buffer;
  
  } else if (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Allocate the buffer to return
    //
    gBS->FreePool (Buffer);
    Status = gBS->AllocatePool (EfiBootServicesData, *VariableSize, &Buffer); 
    if (EFI_ERROR (Status)) {
      *VariableSize = 0;
      return NULL;
    }

    //
    // Read variable into the allocated buffer.
    //
    Status = gRT->GetVariable (Name, VendorGuid, NULL, VariableSize, Buffer);
    if (EFI_ERROR (Status)) {
      *VariableSize = 0;
      gBS->FreePool (Buffer);
      return NULL;
    }
    
    return Buffer;
    
  } else {
    //
    // Variable not found or other errors met.
    //
    *VariableSize = 0;
    gBS->FreePool (Buffer);
    return NULL;
  }
}

EFI_STATUS
ConPlatformMatchDevicePaths (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Multi,
  IN  EFI_DEVICE_PATH_PROTOCOL  *Single,
  IN  EFI_DEVICE_PATH_PROTOCOL  **NewDevicePath  OPTIONAL,
  IN  BOOLEAN                   Delete
  )
/*++

Routine Description:
  Function compares a device path data structure to that of all the nodes of a
  second device path instance.

Arguments:
  Multi        - A pointer to a multi-instance device path data structure.

  Single       - A pointer to a single-instance device path data structure.
  
  NewDevicePath - If Delete is TRUE, this parameter must not be null, and it
                  points to the remaining device path data structure. 
                  (remaining device path = Multi - Single.)
  
  Delete        - If TRUE, means removing Single from Multi.
                  If FALSE, the routine just check whether Single matches 
                  with any instance in Multi.

Returns:

  The function returns EFI_SUCCESS if the Single is contained within Multi.  
  Otherwise, EFI_NOT_FOUND is returned.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathInst;
  UINTN                     Size;
  
  //
  // if performing Delete operation, the NewDevicePath must not be NULL.
  //
  if (Delete) {
    if (NewDevicePath == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    *NewDevicePath = NULL;
  }
  
  if (!Multi || !Single) {
    return EFI_NOT_FOUND;
  }

  DevicePath = Multi;
  DevicePathInst = EfiDevicePathInstance (&DevicePath, &Size);
  //
  // search for the match of 'Single' in 'Multi'
  //
  while (DevicePathInst != NULL) {
    if (EfiCompareMem (Single, DevicePathInst, Size) == 0) {
      if (!Delete) {
        return EFI_SUCCESS;
      }
    } else {
      if (Delete) {
        TempDevicePath = EfiAppendDevicePathInstance (*NewDevicePath, 
                                                      DevicePathInst);          
        if (*NewDevicePath) {
          gBS->FreePool (*NewDevicePath);
        }
        *NewDevicePath = TempDevicePath;
      }
    }
    gBS->FreePool(DevicePathInst);
    DevicePathInst = EfiDevicePathInstance (&DevicePath, &Size);
  }

  if (Delete) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
ConPlatformCheckVariable (
  IN  CHAR16                    *VariableName,
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
/*++

Routine Description:
  

Arguments:

Returns:

  None

--*/  
{
  UINT8                     *Variable;
  UINTN                     VariableSize;
  EFI_DEVICE_PATH_PROTOCOL  *VariableDevicePath;
  EFI_STATUS                Status;
  
  Variable = NULL;
  
  //
  // Get Variable and its size according to variable name.
  // The memory for Variable is allocated within ConPlatformGetVaribleAndSize(),
  // it is the caller's responsibility to free the memory before return. 
  //
  Variable = ConPlatformGetVariableAndSize (
               VariableName,
               &gEfiGlobalVariableGuid,
               &VariableSize  
               );
  //
  // The Variable was not found
  //
  if (Variable == NULL) {
    return EFI_SUCCESS;
  }
  
  //
  // Variable is found.
  //
  VariableDevicePath = (EFI_DEVICE_PATH_PROTOCOL*)Variable;
  
  //
  // Check whether the specified DevicePath is contained in the Variable
  //
  Status = ConPlatformMatchDevicePaths (VariableDevicePath, 
                                        DevicePath, 
                                        NULL, 
                                        FALSE);
  if (Variable) {
    gBS->FreePool (Variable);
  }
  return Status;
}               

EFI_STATUS
ConPlatformUpdateDeviceVariable (
  IN  CHAR16                    *VariableName,
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  IN  BOOLEAN                   Append
  )
/*++

Routine Description:
  

Arguments:

Returns:

  None

--*/  
{
  EFI_STATUS                Status;
  UINT8                     *Variable;
  UINTN                     VariableSize;
  EFI_DEVICE_PATH_PROTOCOL  *VariableDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *NewVariableDevicePath;

  if (DevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Variable = NULL;
  NewVariableDevicePath = NULL;
  
  //
  // Get Variable and its size according to variable name.
  // The memory for Variable is allocated within ConPlatformGetVaribleAndSize(),
  // it is the caller's responsibility to free the memory before return.
  //
  Variable = ConPlatformGetVariableAndSize (
               VariableName,
               &gEfiGlobalVariableGuid,
               &VariableSize  
               );
  VariableDevicePath = (EFI_DEVICE_PATH_PROTOCOL*)Variable;

  if (Append) {
    
    Status = ConPlatformMatchDevicePaths (VariableDevicePath, 
                                          DevicePath, 
                                          &NewVariableDevicePath,
                                          FALSE);
    
    if (!EFI_ERROR(Status)) {
      //
      // The device path has already in the variable
      //
      
      if (Variable) {
        gBS->FreePool (Variable);
      }
      if (NewVariableDevicePath) {
        gBS->FreePool (NewVariableDevicePath);
      }
      return EFI_SUCCESS;
    }
      
    //
    // Append DevicePath to the environment variable that 
    // is a multi-instance device path.
    //
    NewVariableDevicePath = EfiAppendDevicePathInstance (VariableDevicePath,
                                                          DevicePath);
    if (NewVariableDevicePath == NULL) {
      if (Variable) {
        gBS->FreePool (Variable);
      }
      return EFI_OUT_OF_RESOURCES;
    }

  } else {
    //
    // Remove DevicePath from the environment variable that 
    // is a multi-instance device path.
    //
    Status = ConPlatformMatchDevicePaths (VariableDevicePath, 
                                          DevicePath, 
                                          &NewVariableDevicePath,
                                          TRUE);
    if (EFI_ERROR (Status)) {
      if (Variable) {
        gBS->FreePool (Variable);
      }
      return Status;
    }
  }

  if (NewVariableDevicePath) {
    VariableSize = EfiDevicePathSize (NewVariableDevicePath);
  } else {
    VariableSize = 0;
  }

  Status = gRT->SetVariable (
                  VariableName, 
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  VariableSize, 
                  NewVariableDevicePath
                  );

  if (NewVariableDevicePath) {
    gBS->FreePool (NewVariableDevicePath);
  }

  if (Variable) {
    gBS->FreePool (Variable);
  }

  return Status;
}

BOOLEAN
IsHotPlugDevice (
  EFI_HANDLE    DriverBindingHandle,
  EFI_HANDLE    ControllerHandle
  )
{
  EFI_STATUS    Status;
  
  //
  // HotPlugDeviceGuid indicates ControllerHandle stands for a hot plug device.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiHotPlugDeviceGuid, 
                  NULL,
                  DriverBindingHandle,
                  ControllerHandle, 
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR(Status)) {
    return FALSE;
  }
  
  return TRUE;
}