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

  Partition.c
  
Abstract:

  Partition driver that produces logical BlockIo devices from a physical 
  BlockIo device. The logical BlockIo devices are based on the format
  of the raw block devices media. Currently "El Torito CD-ROM", Legacy 
  MBR, and GPT partition schemes are supported.

--*/

#include "Partition.h"

//
// Function Prototypes
//

EFI_STATUS
EFIAPI
PartitionEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
EFIAPI
PartitionDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
PartitionDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
PartitionDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

//
// Partition Driver Global Variables
//
EFI_DRIVER_BINDING_PROTOCOL gPartitionDriverBinding = {
  PartitionDriverBindingSupported,
  PartitionDriverBindingStart,
  PartitionDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT(PartitionEntryPoint)

EFI_STATUS
EFIAPI
PartitionEntryPoint (
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
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gPartitionDriverBinding,
           ImageHandle,
           &gPartitionComponentName,
           NULL,
           NULL
           );
}


EFI_STATUS
PartitionDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

  Routine Description:
    Test to see if this driver supports ControllerHandle. Any ControllerHandle 
    than contains a BlockIo and DiskIo protocol can be supported.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to test
    RemainingDevicePath - Not used

  Returns:
    EFI_SUCCES          - This driver supports this device
    EFI_ALREADY_STARTED - This driver is already running on this device
    other               - This driver does not support this device

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;
  EFI_DISK_IO_PROTOCOL      *DiskIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiDevicePathProtocolGuid,  
                  (VOID **)&ParentDevicePath,
                  This->DriverBindingHandle,     
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         ControllerHandle,   
         &gEfiDevicePathProtocolGuid,  
         This->DriverBindingHandle,     
         ControllerHandle   
         );

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
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }
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
PartitionDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

  Routine Description:
    Start this driver on ControllerHandle by opening a Block IO and Disk IO 
    protocol, reading Device Path, and creating a child handle with a 
    Disk IO and device path protocol.

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
  EFI_STATUS                           Status;
  EFI_STATUS                           OpenStatus;
  EFI_BLOCK_IO_PROTOCOL                *BlockIo;
  EFI_DISK_IO_PROTOCOL                 *DiskIo;
  EFI_DEVICE_PATH_PROTOCOL             *ParentDevicePath;

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

  //
  // Get the Device Path Protocol on ControllerHandle's handle
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiDevicePathProtocolGuid,  
                  (VOID **)&ParentDevicePath,
                  This->DriverBindingHandle,     
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
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
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    gBS->CloseProtocol (
           ControllerHandle,   
           &gEfiDevicePathProtocolGuid,  
           This->DriverBindingHandle,     
           ControllerHandle   
           );
    return Status;
  }

  OpenStatus = Status;
  
  //
  // If no media is present, do nothing here.
  //
  Status = EFI_UNSUPPORTED;
  if (BlockIo->Media->MediaPresent) {
    //
    // Try for GPT, then El Torito, and then legacy MBR partition types. If the
    // media supports a given partition type install child handles to represent
    // the partitions described by the media.
    //
    if (PartitionInstallGptChildHandles (This, 
                                         ControllerHandle, 
                                         DiskIo, 
                                         BlockIo, 
                                         ParentDevicePath) || 

        PartitionInstallElToritoChildHandles (This, 
                                              ControllerHandle, 
                                              DiskIo, 
                                              BlockIo, 
                                              ParentDevicePath) || 

        PartitionInstallMbrChildHandles (This, 
                                         ControllerHandle, 
                                         DiskIo, 
                                         BlockIo, 
                                         ParentDevicePath)) {
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_NOT_FOUND;
    }
  }
  
  //
  // In the case that the driver is already started (OpenStatus == EFI_ALREADY_STARTED), 
  // the DevicePathProtocol and the DiskIoProtocol are not actually opened by the 
  // driver. So don't try to close them. Otherwise, we will break the dependency 
  // between the controller and the driver set up before.
  //
  if (EFI_ERROR (Status) && !EFI_ERROR (OpenStatus)) {
    gBS->CloseProtocol (
           ControllerHandle,   
           &gEfiDiskIoProtocolGuid,  
           This->DriverBindingHandle,     
           ControllerHandle
           );

    gBS->CloseProtocol (
           ControllerHandle,   
           &gEfiDevicePathProtocolGuid,  
           This->DriverBindingHandle,     
           ControllerHandle   
           );
  }

  return Status;
}


EFI_STATUS
EFIAPI
PartitionDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  )
/*++

  Routine Description:
    Stop this driver on ControllerHandle. Support stoping any child handles 
    created by this driver.

  Arguments:
    This              - Protocol instance pointer.
    DeviceHandle      - Handle of device to stop driver on 
    NumberOfChildren  - Number of Children in the ChildHandleBuffer
    ChildHandleBuffer - List of handles for the children we need to stop.

  Returns:
    EFI_SUCCES          - This driver is removed DeviceHandle
    other               - This driver was not removed from this device

--*/
{
  EFI_STATUS                    Status;
  UINTN                         Index;
  EFI_BLOCK_IO_PROTOCOL         *BlockIo;
  BOOLEAN                       AllChildrenStopped;
  PARTITION_PRIVATE_DATA        *Private;
  EFI_DISK_IO_PROTOCOL          *DiskIo;
  
  if (NumberOfChildren == 0) {
    //
    // Close the bus driver
    //
    gBS->CloseProtocol (
           ControllerHandle,   
           &gEfiDiskIoProtocolGuid,  
           This->DriverBindingHandle,     
           ControllerHandle
           );

    gBS->CloseProtocol (
           ControllerHandle,   
           &gEfiDevicePathProtocolGuid,  
           This->DriverBindingHandle,     
           ControllerHandle   
           );

    return EFI_SUCCESS;
  }


  AllChildrenStopped = TRUE;
  for (Index = 0; Index < NumberOfChildren; Index++) {
    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index],   
                    &gEfiBlockIoProtocolGuid,  
                    (VOID **)&BlockIo,
                    This->DriverBindingHandle,             
                    ControllerHandle,   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {

      Private = PARTITION_DEVICE_FROM_BLOCK_IO_THIS (BlockIo);

      //
      // All Software protocols have be freed from the handle so remove it.
      //
      BlockIo->FlushBlocks (BlockIo);

      Status = gBS->CloseProtocol (
                      ControllerHandle,   
                      &gEfiDiskIoProtocolGuid,  
                      This->DriverBindingHandle,     
                      ChildHandleBuffer[Index]   
                      );

      Status = gBS->UninstallMultipleProtocolInterfaces (
                     ChildHandleBuffer[Index], 
                     &gEfiDevicePathProtocolGuid,      Private->DevicePath,
                     &gEfiBlockIoProtocolGuid,         &Private->BlockIo,
                     Private->EspGuid,                 NULL,
                     NULL
                     );
      if (EFI_ERROR (Status)) {
        gBS->OpenProtocol (
               ControllerHandle,
               &gEfiDiskIoProtocolGuid,  
               (VOID **)&DiskIo,
               This->DriverBindingHandle,     
               ChildHandleBuffer[Index],
               EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
               );
      } else {
        gBS->FreePool (Private->DevicePath);
        gBS->FreePool (Private);
      }

    }

    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
PartitionReset (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN BOOLEAN                ExtendedVerification
  )
/*++

  Routine Description:
    Reset the parent Block Device.

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES            - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could 
                            not be reset.

--*/
{
  PARTITION_PRIVATE_DATA  *Private;

  Private = PARTITION_DEVICE_FROM_BLOCK_IO_THIS (This);

  return Private->ParentBlockIo->Reset (
                                    Private->ParentBlockIo, 
                                    ExtendedVerification
                                    );
}


EFI_STATUS
PartitionReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,
  OUT VOID                  *Buffer
  )
/*++

  Routine Description:
    Read by using the Disk IO protocol on the parent device. Lba addresses
    must be converted to byte offsets.

  Arguments:
    This       - Protocol instance pointer.
    MediaId    - Id of the media, changes every time the media is replaced.
    Lba        - The starting Logical Block Address to read from
    BufferSize - Size of Buffer, must be a multiple of device block size.
    Buffer     - Buffer containing read data

  Returns:
    EFI_SUCCES            - The data was read correctly from the device.
    EFI_DEVICE_ERROR      - The device reported an error while performing the read.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHANGED     - The MediaId does not matched the current device.
    EFI_BAD_BUFFER_SIZE   - The Buffer was not a multiple of the block size of the 
                            device.
    EFI_INVALID_PARAMETER - The read request contains device addresses that are not 
                            valid for the device.

--*/
{
  PARTITION_PRIVATE_DATA  *Private;
  EFI_BLOCK_IO_MEDIA  *Media;
  UINT64                  Offset;


  Private = PARTITION_DEVICE_FROM_BLOCK_IO_THIS (This);
  
  //
  //Check buffer alignment
  //
  Media = Private->BlockIo.Media;
  if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
    return EFI_INVALID_PARAMETER;
  }
  
  if (BufferSize % Private->BlockSize != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }
  
  Offset = DriverLibMultU64x32 (Lba, Private->BlockSize) + Private->Start;
  if (Offset + BufferSize > Private->End) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Because some kinds of partition have different block size from their parent
  // device, we call the Disk IO protocol on the parent device, not the Block IO
  // protocol
  //
  return Private->DiskIo->ReadDisk (Private->DiskIo, MediaId, Offset, BufferSize, Buffer);
}


EFI_STATUS
PartitionWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,
  OUT VOID                  *Buffer
  )
/*++

  Routine Description:
    Write by using the Disk IO protocol on the parent device. Lba addresses
    must be converted to byte offsets.

  Arguments:
    This       - Protocol instance pointer.
    MediaId    - Id of the media, changes every time the media is replaced.
    Lba        - The starting Logical Block Address to read from
    BufferSize - Size of Buffer, must be a multiple of device block size.
    Buffer     - Buffer containing read data

  Returns:
    EFI_SUCCES            - The data was written correctly to the device.
    EFI_WRITE_PROTECTED   - The device can not be written to.
    EFI_DEVICE_ERROR      - The device reported an error while performing the write.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHNAGED     - The MediaId does not matched the current device.
    EFI_BAD_BUFFER_SIZE   - The Buffer was not a multiple of the block size of the 
                            device.
    EFI_INVALID_PARAMETER - The write request contains a LBA that is not 
                            valid for the device.

--*/
{
  PARTITION_PRIVATE_DATA  *Private;
  EFI_BLOCK_IO_MEDIA  *Media;
  UINT64                  Offset;


  Private = PARTITION_DEVICE_FROM_BLOCK_IO_THIS (This);


  //
  //Check buffer alignment
  //
  Media = Private->BlockIo.Media;
  if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
    return EFI_INVALID_PARAMETER;
  }
  
  if (BufferSize % Private->BlockSize != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  Offset = DriverLibMultU64x32 (Lba, Private->BlockSize) + Private->Start;
  if (Offset + BufferSize > Private->End) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Because some kinds of partition have different block size from their parent
  // device, we call the Disk IO protocol on the parent device, not the Block IO
  // protocol
  //
  return Private->DiskIo->WriteDisk (Private->DiskIo, MediaId, Offset, BufferSize, Buffer);
}


EFI_STATUS
PartitionFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
/*++

  Routine Description:
    Flush the parent Block Device.

  Arguments:
    This             - Protocol instance pointer.

  Returns:
    EFI_SUCCES       - All outstanding data was written to the device
    EFI_DEVICE_ERROR - The device reported an error while writing back the data
    EFI_NO_MEDIA     - There is no media in the device.

--*/
{
  PARTITION_PRIVATE_DATA  *Private;

  Private = PARTITION_DEVICE_FROM_BLOCK_IO_THIS (This);

  return Private->ParentBlockIo->FlushBlocks (Private->ParentBlockIo);
}


EFI_STATUS
PartitionInstallChildHandle (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ParentHandle,
  IN  EFI_DISK_IO_PROTOCOL         *ParentDiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL        *ParentBlockIo,
  IN  EFI_DEVICE_PATH_PROTOCOL     *ParentDevicePath,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN  EFI_LBA                      Start,
  IN  EFI_LBA                      End,
  IN  UINT32                       BlockSize,
  IN  BOOLEAN                      InstallEspGuid
  )
/*++

Routine Description:
  Create a child handle for a logical block device that represents the
  bytes Start to End of the Parent Block IO device.

Arguments:       
  ParentHandle     - Parent Handle for new child
  ParentDiskIo     - Parent DiskIo interface
  ParentBlockIo    - Parent BlockIo interface
  ParentDevicePath - Parent Device Path
  DevicePathNode   - Child Device Path node
  Start            - Start Block
  End              - End Block
  BlockSize        - Child block size
  InstallEspGuid   - Flag to install EFI System Partition GUID on handle

Returns:
  EFI_SUCCESS - If a child handle was added
  other       - A child handle was not added

--*/
{
  EFI_STATUS              Status;
  PARTITION_PRIVATE_DATA  *Private;
  UINTN                   Dummy;

  Private = EfiLibAllocateZeroPool (sizeof(PARTITION_PRIVATE_DATA));
  if (Private == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Private->Signature = PARTITION_PRIVATE_DATA_SIGNATURE;

  Private->Start          = DriverLibMultU64x32 (Start, ParentBlockIo->Media->BlockSize);
  Private->End            = DriverLibMultU64x32 (End + 1, ParentBlockIo->Media->BlockSize);

  Private->BlockSize      = BlockSize;
  Private->ParentBlockIo  = ParentBlockIo;
  Private->DiskIo   = ParentDiskIo;

  Private->BlockIo.Revision = EFI_BLOCK_IO_PROTOCOL_REVISION;

  Private->BlockIo.Media = &Private->Media;
  EfiCopyMem (Private->BlockIo.Media, ParentBlockIo->Media, sizeof(EFI_BLOCK_IO_MEDIA));
  Private->Media.LogicalPartition = TRUE;
  Private->Media.LastBlock        = 
    DriverLibDivU64x32 (
      DriverLibMultU64x32 (End - Start + 1, ParentBlockIo->Media->BlockSize),
      (UINTN)BlockSize,
      &Dummy
      ) - 1;

  Private->Media.BlockSize        = (UINT32)BlockSize;

  Private->BlockIo.Reset        = PartitionReset;
  Private->BlockIo.ReadBlocks   = PartitionReadBlocks;
  Private->BlockIo.WriteBlocks  = PartitionWriteBlocks;
  Private->BlockIo.FlushBlocks  = PartitionFlushBlocks;

  Private->DevicePath = EfiAppendDevicePathNode (ParentDevicePath, DevicePathNode);
  
  if (Private->DevicePath == NULL) {
    gBS->FreePool (Private);
    return EFI_OUT_OF_RESOURCES;
  }
  
  if (InstallEspGuid) {
    Private->EspGuid = &gEfiPartTypeSystemPartGuid;
  } else {
    //
    // If NULL InstallMultipleProtocolInterfaces will ignore it.
    //
    Private->EspGuid = NULL;
  }

  //
  // Create the new handle
  //
  Private->Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,                
                  &gEfiDevicePathProtocolGuid,      Private->DevicePath,
                  &gEfiBlockIoProtocolGuid,         &Private->BlockIo,
                  Private->EspGuid,                 NULL,
                  NULL
                  );

  if (!EFI_ERROR (Status)) {
    //
    // Open the Parent Handle for the child
    //

    Status = gBS->OpenProtocol (
                    ParentHandle,   
                    &gEfiDiskIoProtocolGuid,  
                    (VOID **)&ParentDiskIo,
                    This->DriverBindingHandle,     
                    Private->Handle,
                    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                    );
  } else {
    gBS->FreePool (Private->DevicePath);
    gBS->FreePool (Private);
  }  

  return Status;
}

