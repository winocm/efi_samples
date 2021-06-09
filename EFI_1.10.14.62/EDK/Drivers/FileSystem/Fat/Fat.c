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

  Fat.c
  
Abstract:

  Fat File System driver routines that support EFI driver model
  
--*/

#include "Fat.h"


EFI_STATUS
FatEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
FatUnload (
  IN EFI_HANDLE         ImageHandle
  );

EFI_STATUS
EFIAPI
FatDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
FatDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
FatDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );


//
// DriverBinding protocol instance
//
EFI_DRIVER_BINDING_PROTOCOL gFatDriverBinding = {
  FatDriverBindingSupported,
  FatDriverBindingStart,
  FatDriverBindingStop,
  0x10,
  NULL,
  NULL
};


EFI_DRIVER_ENTRY_POINT (FatEntryPoint)

EFI_STATUS
EFIAPI
FatEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Register Driver Binding protocol for this driver.
  
Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 
  EFI_SUCCESS - Driver loaded
  other       - Driver not loaded

--*/
{
  EFI_STATUS                 Status;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;

  //
  // Initialize the EFI Driver Library
  //
  Status = EfiLibInstallAllDriverProtocols (
             ImageHandle, 
             SystemTable, 
             &gFatDriverBinding,
             ImageHandle,
             &gFatComponentName,
             NULL,
             NULL
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Fill in the Unload() function
  //
  Status = gBS->OpenProtocol (
                  ImageHandle, 
                  &gEfiLoadedImageProtocolGuid,  
                  (VOID **)&LoadedImage,
                  ImageHandle,   
                  ImageHandle,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  LoadedImage->Unload = FatUnload;

  return Status;
}


EFI_STATUS
EFIAPI
FatUnload (
  IN EFI_HANDLE  ImageHandle
  )
/*++

Routine Description:
  Unload function for this image. Uninstall DriverBinding protocol.
  
Arguments:
  Standart unload function arguments.
  
Returns:
  Status code.
  
--*/
{
  EFI_STATUS  Status;
  EFI_HANDLE  *DeviceHandleBuffer;
  UINTN       DeviceHandleCount;
  UINTN       Index;

  Status = gBS->LocateHandleBuffer (
             AllHandles,
             NULL,
             NULL,
             &DeviceHandleCount,
             &DeviceHandleBuffer
             );
  if (!EFI_ERROR(Status)) {
    for (Index = 0; Index < DeviceHandleCount; Index++) {
      Status = gBS->DisconnectController (DeviceHandleBuffer[Index],
                                          ImageHandle,
                                          NULL
                                          );
    }
    if (DeviceHandleBuffer != NULL) {
      gBS->FreePool(DeviceHandleBuffer);
    }
  }

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  ImageHandle,
                  &gEfiDriverBindingProtocolGuid, &gFatDriverBinding,
                  &gEfiComponentNameProtocolGuid, &gFatComponentName,
                  NULL
                  );

  return Status;
}


EFI_STATUS
EFIAPI
FatDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

  Routine Description:
    Test to see if this driver can add a file system to ControllerHandle.
    ControllerHandle must support both Disk IO and Block IO protocols.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to test
    RemainingDevicePath - Not used.

  Returns:
    EFI_SUCCES          - This driver supports this device
    EFI_ALREADY_STARTED - This driver is already running on this device
    other               - This driver does not support this device

--*/
{
  EFI_STATUS                Status;
  EFI_DISK_IO_PROTOCOL      *DiskIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiDiskIoProtocolGuid,  
                  (VOID **)&DiskIo,
                  This->DriverBindingHandle,     
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         ControllerHandle,   
         &gEfiDiskIoProtocolGuid,  
         This->DriverBindingHandle,     
         ControllerHandle   
         );

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiBlockIoProtocolGuid,  
                  NULL,
                  This->DriverBindingHandle,     
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  return Status;
}

EFI_STATUS
EFIAPI
FatDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

  Routine Description:
    Start this driver on ControllerHandle by opening a Block IO and Disk IO 
    protocol, reading Device Path. Add a Simple File System protocol to
    ControllerHandle if the media contains a valid file system.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to bind driver to
    RemainingDevicePath - Not used

  Returns:
    EFI_SUCCES          - This driver is added to DeviceHandle
    EFI_ALREADY_STARTED - This driver is already running on DeviceHandle
    other               - This driver does not support this device

--*/

{
  EFI_STATUS                      Status;
  EFI_BLOCK_IO_PROTOCOL           *BlockIo;
  EFI_DISK_IO_PROTOCOL            *DiskIo;
  LC_ISO_639_2                    *LangCode;
  UINTN                           Size;

  //
  // Initialize unicode support
  //
  
  gUnicodeCollationInterface = NULL;
  
  //
  // Find current LangCode from Lang NV Variable
  //
  Size = 0;
  LangCode = NULL;
  Status = gRT->GetVariable (L"Lang",
                             &gEfiGlobalVariableGuid,
                             NULL,
                             &Size,
                             LangCode);

  if (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Allocate the buffer to return
    //
    LangCode = EfiLibAllocatePool (Size);
    
    if (LangCode == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Read variable into the allocated buffer.
    //
    Status = gRT->GetVariable (L"Lang",
                               &gEfiGlobalVariableGuid,
                               NULL,
                               &Size,
                               LangCode);
  }

  //
  // If cannot get language code from Lang Variable,
  // set language code to the default language English.
  //
  if (EFI_ERROR(Status)) {
    Status = FatInitUnicodeCollationSupport (This,
                                             "eng",
                                             &gUnicodeCollationInterface
                                             );
    DEBUG ((EFI_D_ERROR, "Failed to get language code, so use the default!\n"));
  } else {
    Status = FatInitUnicodeCollationSupport (This,
                                         LangCode,
                                         &gUnicodeCollationInterface
                                         );
  }
  
  gBS->FreePool (LangCode);
  
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  //
  // Open our required BlockIo and DiskIo
  //
  
  Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiBlockIoProtocolGuid,  
                  (VOID **)&BlockIo,
                  This->DriverBindingHandle,     
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiDiskIoProtocolGuid,  
                  (VOID **)&DiskIo,
                  This->DriverBindingHandle,     
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Allocate Volume structure. In FatAllocateVolume(), Resources 
  // are allocated with protocol installed and cached initialized
  //  
  Status = EFI_UNSUPPORTED;
  if (BlockIo->Media->MediaPresent) {
    Status = FatAllocateVolume (ControllerHandle, DiskIo, BlockIo);
  }

  //
  // Leave the device open even if we did not add a Simple File System 
  // protocol.
  // This will prevent re-probing of the same device over and over. When the 
  // media changes on a device it will Reinstall the BlockIo interaface. This
  // will cause a call to our Stop(), and a subsequent call to our Start().
  //
  if (EFI_ERROR (Status)) {
    Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiSimpleFileSystemProtocolGuid,  
                  NULL,
                  This->DriverBindingHandle,     
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
    if (EFI_ERROR(Status)) {
    gBS->CloseProtocol (
           ControllerHandle, 
           &gEfiDiskIoProtocolGuid, 
           This->DriverBindingHandle,   
           ControllerHandle
           );
    }
  }
  return Status;
}


EFI_STATUS
EFIAPI
FatDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  )
/*++

  Routine Description:
    Stop this driver on ControllerHandle. 

  Arguments:
    This              - Protocol instance pointer.
    DeviceHandle      - Handle of device to stop driver on 
    NumberOfChildren  - Not used
    ChildHandleBuffer - Not used

  Returns:
    EFI_SUCCES        - This driver is removed DeviceHandle
    other             - This driver was not removed from this device

--*/
{
  EFI_STATUS                      Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  FAT_VOLUME                      *Vol;

  //
  // Get our context back
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiSimpleFileSystemProtocolGuid,  
                  (VOID **)&FileSystem,
                  This->DriverBindingHandle,   
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (!EFI_ERROR (Status)) {
    Vol = CR (FileSystem, FAT_VOLUME, VolInterface, FAT_VOLUME_SIGNATURE);    
    Status = FatAbandonVolume (Vol);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = gBS->CloseProtocol (
         ControllerHandle, 
         &gEfiDiskIoProtocolGuid, 
         This->DriverBindingHandle,   
         ControllerHandle
         );

  return Status;
}


