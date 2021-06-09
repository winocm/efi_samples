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

  DiskIo.c
  
Abstract:

  DiskIo driver that layers it's self on every Block IO protocol in the system.
  DiskIo converts a block oriented device to a byte oriented device. 

  ReadDisk may have to do reads that are not aligned on sector boundaries. 
  There are three cases:

    UnderRun - The first byte is not on a sector boundary or the read request is
               less than a sector in length. 

    Aligned  - A read of N contiguous sectors.

    OverRun  - The last byte is not on a sector boundary.

--*/

#include "DiskIo.h"

//
// Prototypes
// Driver model protocol interface
//

EFI_STATUS
DiskIoDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
DiskIoDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
DiskIoDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
DiskIoDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// Disk I/O Protocol Interface
//

EFI_STATUS
EFIAPI
DiskIoReadDisk (
  IN EFI_DISK_IO_PROTOCOL  *This,
  IN UINT32                MediaId,
  IN UINT64                Offset,
  IN UINTN                 BufferSize,
  OUT VOID                 *Buffer
  );

EFI_STATUS
EFIAPI
DiskIoWriteDisk (
  IN EFI_DISK_IO_PROTOCOL  *This,
  IN UINT32                MediaId,
  IN UINT64                Offset,
  IN UINTN                 BufferSize,
  IN VOID                  *Buffer
  );

EFI_DRIVER_BINDING_PROTOCOL gDiskIoDriverBinding = {
  DiskIoDriverBindingSupported,
  DiskIoDriverBindingStart,
  DiskIoDriverBindingStop,
  0x10,
  NULL,
  NULL
};

DISK_IO_PRIVATE_DATA gDiskIoPrivateDataTemplate = {
  DISK_IO_PRIVATE_DATA_SIGNATURE,
  {
    EFI_DISK_IO_PROTOCOL_REVISION,
    DiskIoReadDisk,
    DiskIoWriteDisk
  },
  NULL
};


// new add!!

VOID *
EfiLibAllocateCopyPool (
  IN  UINTN   AllocationSize,
  IN  VOID    *Buffer
  )
{
  VOID  *Memory;

  Memory = NULL;
  gBS->AllocatePool (EfiBootServicesData, AllocationSize, &Memory);
  if (Memory != NULL) {
    gBS->CopyMem (Memory, Buffer, AllocationSize);
  }
  return Memory;
}

// new add!


EFI_DRIVER_ENTRY_POINT (DiskIoDriverEntryPoint)

EFI_STATUS
EFIAPI
DiskIoDriverEntryPoint(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:
  Register Driver Binding protocol for this driver.
  
Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 
  EFI_SUCCESS - Driver loaded.
  other       - Driver not loaded.

--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gDiskIoDriverBinding,
           ImageHandle,
           &gDiskIoComponentName,
           NULL,
           NULL
           );
}

EFI_STATUS
EFIAPI
DiskIoDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath  OPTIONAL
  )
/*++

  Routine Description:
    Test to see if this driver supports ControllerHandle. Any ControllerHandle
    than contains a BlockIo protocol can be supported.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to test.
    RemainingDevicePath - Not used.

  Returns:
    EFI_SUCCESS         - This driver supports this device.
    EFI_ALREADY_STARTED - This driver is already running on this device.
    other               - This driver does not support this device.

--*/
{
  EFI_STATUS             Status;
  EFI_BLOCK_IO_PROTOCOL  *BlockIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,  
                  &gEfiBlockIoProtocolGuid, 
                  (VOID **)&BlockIo,
                  This->DriverBindingHandle,   
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test.
  //
  gBS->CloseProtocol (
         ControllerHandle,  
         &gEfiBlockIoProtocolGuid, 
         This->DriverBindingHandle,   
         ControllerHandle   
         );

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DiskIoDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath  OPTIONAL
  )
/*++

  Routine Description:
    Start this driver on ControllerHandle by opening a Block IO protocol and 
    installing a Disk IO protocol on ControllerHandle.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to bind driver to.
    RemainingDevicePath - Not used, always produce all possible children.

  Returns:
    EFI_SUCCESS         - This driver is added to ControllerHandle.
    EFI_ALREADY_STARTED - This driver is already running on ControllerHandle.
    other               - This driver does not support this device.

--*/
{
  EFI_STATUS              Status;
  DISK_IO_PRIVATE_DATA    *Private;

  Private = NULL;

  //
  // Connect to the Block IO interface on ControllerHandle.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiBlockIoProtocolGuid, 
                  (VOID **)&gDiskIoPrivateDataTemplate.BlockIo,
                  This->DriverBindingHandle,   
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Initialize the Disk IO device instance.
  //

  Private = EfiLibAllocateCopyPool (sizeof (DISK_IO_PRIVATE_DATA), &gDiskIoPrivateDataTemplate);
  if (Private == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }
 
  //
  // Install protocol interfaces for the Disk IO device.
  //
  Status = gBS->InstallProtocolInterface (
                  &ControllerHandle, 
                  &gEfiDiskIoProtocolGuid, 
                  EFI_NATIVE_INTERFACE, 
                  &Private->DiskIo
                  );

ErrorExit:
  if (EFI_ERROR (Status)) {

    if (Private != NULL) {
      gBS->FreePool(Private);
    }

    gBS->CloseProtocol (
           ControllerHandle, 
           &gEfiBlockIoProtocolGuid, 
           This->DriverBindingHandle,   
           ControllerHandle   
           );
  }

  return Status;
}


EFI_STATUS
EFIAPI
DiskIoDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
/*++

  Routine Description:
    Stop this driver on ControllerHandle by removing Disk IO protocol and closing 
    the Block IO protocol on ControllerHandle.

  Arguments:
    This              - Protocol instance pointer.
    ControllerHandle  - Handle of device to stop driver on.
    NumberOfChildren  - Not used.
    ChildHandleBuffer - Not used.

  Returns:
    EFI_SUCCESS         - This driver is removed ControllerHandle.
    other               - This driver was not removed from this device.

--*/
{
  EFI_STATUS            Status;
  EFI_DISK_IO_PROTOCOL  *DiskIo;
  DISK_IO_PRIVATE_DATA           *Private;

  //
  // Get our context back.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiDiskIoProtocolGuid,  
                  (VOID **)&DiskIo,
                  This->DriverBindingHandle,   
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Private = DISK_IO_PRIVATE_DATA_FROM_THIS (DiskIo);

  Status = gBS->UninstallProtocolInterface (
                  ControllerHandle, 
                  &gEfiDiskIoProtocolGuid, 
                  &Private->DiskIo
                  );
  if (!EFI_ERROR (Status)) {

    Status = gBS->CloseProtocol (
                    ControllerHandle, 
                    &gEfiBlockIoProtocolGuid, 
                    This->DriverBindingHandle,   
                    ControllerHandle
                    );
  }
  
  if (!EFI_ERROR (Status)) {
    gBS->FreePool (Private);
  }

  return Status;
}

EFI_STATUS
EFIAPI
DiskIoReadDisk (
  IN EFI_DISK_IO_PROTOCOL  *This,
  IN UINT32                MediaId,
  IN UINT64                Offset,
  IN UINTN                 BufferSize,
  OUT VOID                 *Buffer
  )
/*++

  Routine Description:
    Read BufferSize bytes from Offset into Buffer.

    Reads may support reads that are not aligned on 
    sector boundaries. There are three cases:

      UnderRun - The first byte is not on a sector boundary or the read request is
                 less than a sector in length. 

      Aligned  - A read of N contiguous sectors.

      OverRun  - The last byte is not on a sector boundary.


  Arguments:
    This       - Protocol instance pointer.
    MediaId    - Id of the media, changes every time the media is replaced.
    Offset     - The starting byte offset to read from.
    BufferSize - Size of Buffer.
    Buffer     - Buffer containing read data.

  Returns:
    EFI_SUCCESS           - The data was read correctly from the device.
    EFI_DEVICE_ERROR      - The device reported an error while performing the read.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHNAGED     - The MediaId does not matched the current device.
    EFI_INVALID_PARAMETER - The read request contains device addresses that are not 
                            valid for the device.

--*/
{
  EFI_STATUS            Status;
  DISK_IO_PRIVATE_DATA  *Private;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;
  EFI_BLOCK_IO_MEDIA    *Media;
  UINT32                BlockSize;
  UINT64                Lba;
  UINT64                OverRunLba;
  UINTN                 UnderRun;
  UINTN                 OverRun;
  BOOLEAN               TransactionComplete;
  UINTN                 WorkingBufferSize;
  UINT8                 *WorkingBuffer;
  UINTN                 Length;
  UINT8                 *Data;
  UINT8                 *PreData;
  UINTN                 IsBufferAligned;
  UINTN                 DataBufferSize;
  BOOLEAN               LastRead;

  Private = DISK_IO_PRIVATE_DATA_FROM_THIS (This);

  BlockIo   = Private->BlockIo;
  Media     = BlockIo->Media;
  BlockSize = Media->BlockSize;

  if (Media->MediaId != MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  WorkingBuffer = Buffer;
  WorkingBufferSize = BufferSize;

  //
  // Allocate a temporary buffer for operation
  // 

  DataBufferSize = BlockSize * DATA_BUFFER_BLOCK_NUM;

  if (Media->IoAlign > 1) {
    PreData = EfiLibAllocatePool (DataBufferSize + Media->IoAlign);    
    Data = PreData - ((UINTN)PreData & (Media->IoAlign - 1)) + Media->IoAlign;
  } else {
    PreData = EfiLibAllocatePool (DataBufferSize);
    Data = PreData;
  }
  
  if (PreData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Lba = DriverLibDivU64x32 (Offset, BlockSize, &UnderRun);
 
  Length = BlockSize - UnderRun;
  TransactionComplete = FALSE;

  Status = EFI_SUCCESS;
  if (UnderRun != 0) {
    //
    // Offset starts in the middle of an Lba, so read the entire block.
    //
    Status = BlockIo->ReadBlocks (
                        BlockIo, 
                        MediaId, 
                        Lba, 
                        BlockSize, 
                        Data
                        );

    if (EFI_ERROR (Status)) {
      goto Done;
    }
    
    if (Length > BufferSize) {
      Length = BufferSize;
      TransactionComplete = TRUE;
    }

    EfiCopyMem (WorkingBuffer, Data + UnderRun, Length);
    
    WorkingBuffer += Length;
    
    WorkingBufferSize  -= Length;
    if (WorkingBufferSize == 0) {
      //
      // BugBug: Goto is not clean but I was getting bad warnings.
      //
      goto Done;
    }

    Lba += 1;
  }


  OverRunLba = Lba + DriverLibDivU64x32 (WorkingBufferSize, BlockSize, &OverRun);

  if (!TransactionComplete && WorkingBufferSize >= BlockSize) {
    //
    // If the DiskIo maps directly to a BlockIo device do the read.
    //
    if (OverRun != 0) {
      WorkingBufferSize -= OverRun;
    }

    //
    // Check buffer alignment
    //
    IsBufferAligned = (UINTN)WorkingBuffer & (UINTN)(Media->IoAlign - 1);

    if (Media->IoAlign <= 1 || IsBufferAligned == 0) {

      //
      // Alignment is satisfied, so read them together
      //      

      Status = BlockIo->ReadBlocks (
                        BlockIo, 
                        MediaId, 
                        Lba, 
                        WorkingBufferSize, 
                        WorkingBuffer
                        );

      if (EFI_ERROR (Status)) {
        goto Done;
      }

      WorkingBuffer += WorkingBufferSize;

    } else {

      //
      // Use the allocated buffer instead of the original buffer
      // to avoid alignment issue.
      // Here, the allocated buffer (8-byte align) can satisfy the alignment
      //

      LastRead = FALSE;
      do {
        if (WorkingBufferSize <= DataBufferSize) {

          //
          // It is the last calling to readblocks in this loop
          //
          DataBufferSize = WorkingBufferSize;          
          LastRead       = TRUE;            
        } 

        Status = BlockIo->ReadBlocks (
                        BlockIo, 
                        MediaId, 
                        Lba, 
                        DataBufferSize, 
                        Data
                        );
        if (EFI_ERROR (Status)) {
          goto Done;
        }
        EfiCopyMem (WorkingBuffer, Data, DataBufferSize);
        WorkingBufferSize -= DataBufferSize;
        WorkingBuffer     += DataBufferSize;
        Lba               += DATA_BUFFER_BLOCK_NUM;
      } while (!LastRead);
    } 
  }


  if (!TransactionComplete && OverRun != 0) {
    //
    // Last read is not a complete block.
    //
    Status = BlockIo->ReadBlocks (
                        BlockIo, 
                        MediaId, 
                        OverRunLba, 
                        BlockSize, 
                        Data
                        );

    if (EFI_ERROR (Status)) {
      goto Done;
    }

    EfiCopyMem (WorkingBuffer, Data, OverRun);    
  }

Done:
  if (PreData != NULL) {
    gBS->FreePool (PreData);
  }
  return Status;
}

EFI_STATUS
EFIAPI
DiskIoWriteDisk (
  IN EFI_DISK_IO_PROTOCOL  *This,
  IN UINT32                MediaId,
  IN UINT64                Offset,
  IN UINTN                 BufferSize,
  IN VOID                  *Buffer
  )
/*++

  Routine Description:
    Read BufferSize bytes from Offset into Buffer.

    Writes may require a read modify write to support writes that are not 
    aligned on sector boundaries. There are three cases:
    
      UnderRun - The first byte is not on a sector boundary or the write request
                 is less than a sector in length. Read modify write is required.

      Aligned  - A write of N contiguous sectors.

      OverRun  - The last byte is not on a sector boundary. Read modified write 
                 required.

  Arguments:
    This       - Protocol instance pointer.
    MediaId    - Id of the media, changes every time the media is replaced. 
    Offset     - The starting byte offset to read from.
    BufferSize - Size of Buffer.
    Buffer     - Buffer containing read data.

  Returns:
    EFI_SUCCESS           - The data was written correctly to the device.
    EFI_WRITE_PROTECTED   - The device can not be written to.
    EFI_DEVICE_ERROR      - The device reported an error while performing the write.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHNAGED     - The MediaId does not matched the current device.
    EFI_INVALID_PARAMETER - The write request contains device addresses that are not 
                            valid for the device.

--*/
{
  EFI_STATUS            Status;
  DISK_IO_PRIVATE_DATA  *Private;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;
  EFI_BLOCK_IO_MEDIA    *Media;
  UINT32                BlockSize;
  UINT64                Lba;
  UINT64                OverRunLba;
  UINTN                 UnderRun;
  UINTN                 OverRun;
  BOOLEAN               TransactionComplete;
  UINTN                 WorkingBufferSize;
  UINT8                 *WorkingBuffer;
  UINTN                 Length;
  UINT8                 *Data;
  UINT8                 *PreData;
  UINTN                 IsBufferAligned;
  UINTN                 DataBufferSize;
  BOOLEAN               LastWrite;

  Private = DISK_IO_PRIVATE_DATA_FROM_THIS (This);

  BlockIo   = Private->BlockIo;
  Media     = BlockIo->Media;
  BlockSize = Media->BlockSize;

  if (Media->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }

  if (Media->MediaId != MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  DataBufferSize = BlockSize * DATA_BUFFER_BLOCK_NUM;

  if (Media->IoAlign > 1) {
    PreData = EfiLibAllocatePool (DataBufferSize + Media->IoAlign);
    Data = PreData - ((UINTN)PreData & (Media->IoAlign - 1)) + Media->IoAlign;
  } else {
    PreData = EfiLibAllocatePool (DataBufferSize);
    Data = PreData;
  }

  if (PreData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  WorkingBuffer = Buffer;
  WorkingBufferSize = BufferSize;

  Lba = DriverLibDivU64x32 (Offset, BlockSize, &UnderRun);

 
  Length = BlockSize - UnderRun;
  TransactionComplete = FALSE;

  Status = EFI_SUCCESS;
  if (UnderRun != 0) {
    //
    // Offset starts in the middle of an Lba, so do read modify write.
    //
    Status = BlockIo->ReadBlocks (
                        BlockIo, 
                        MediaId, 
                        Lba, 
                        BlockSize, 
                        Data
                        );

    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (Length > BufferSize) {
      Length = BufferSize;
      TransactionComplete = TRUE;
    }

    EfiCopyMem (Data + UnderRun, WorkingBuffer, Length);

    Status = BlockIo->WriteBlocks (
                        BlockIo, 
                        MediaId, 
                        Lba, 
                        BlockSize, 
                        Data
                        );
    if (EFI_ERROR (Status)) {
      goto Done;
    }
    
    
    WorkingBuffer += Length;
    WorkingBufferSize  -= Length;
    if (WorkingBufferSize == 0) {
      //
      // BugBug: Goto is not clean but I was getting bad warnings.
      //
      goto Done;
    }

    Lba += 1;
  }

  OverRunLba = Lba + DriverLibDivU64x32 (WorkingBufferSize, BlockSize, &OverRun);

  if (!TransactionComplete && WorkingBufferSize >= BlockSize) {
    //
    // If the DiskIo maps directly to a BlockIo device do the write.
    //
    if (OverRun != 0) {
      WorkingBufferSize -= OverRun;
    }
    
    //
    // Check buffer alignment
    //
    IsBufferAligned = (UINTN)WorkingBuffer & (UINTN)(Media->IoAlign - 1);

    if (Media->IoAlign <= 1 || IsBufferAligned == 0) {

      //
      // Alignment is satisfied, so write them together
      //

      Status = BlockIo->WriteBlocks (
                        BlockIo, 
                        MediaId, 
                        Lba, 
                        WorkingBufferSize, 
                        WorkingBuffer
                        );

      if (EFI_ERROR (Status)) {
        goto Done;
      }

      WorkingBuffer += WorkingBufferSize;

    } else {
      //
      // The buffer parameter is not aligned with the request
      // So use the allocated instead.
      // It can fit almost all the cases.
      //
      LastWrite = FALSE;
      do {
        if (WorkingBufferSize <= DataBufferSize) {

          //
          // It is the last calling to writeblocks in this loop
          //

          DataBufferSize = WorkingBufferSize;          
          LastWrite      = TRUE;  
        }
        
        EfiCopyMem (Data, WorkingBuffer, DataBufferSize);
        Status = BlockIo->WriteBlocks (
                        BlockIo, 
                        MediaId, 
                        Lba, 
                        DataBufferSize, 
                        Data
                        );
        if (EFI_ERROR (Status)) {
          goto Done;
        }
        
        WorkingBufferSize -= DataBufferSize;
        WorkingBuffer     += DataBufferSize;
        Lba               += DATA_BUFFER_BLOCK_NUM;
      } while (!LastWrite);
    } 
  }

  if (!TransactionComplete && OverRun != 0) {
    //
    // Last bit is not a complete block, so do a read modify write.
    //
    Status = BlockIo->ReadBlocks (
                        BlockIo, 
                        MediaId, 
                        OverRunLba, 
                        BlockSize, 
                        Data
                        );

    if (EFI_ERROR (Status)) {
      goto Done;
    }

    EfiCopyMem (Data, WorkingBuffer, OverRun);    

    Status = BlockIo->WriteBlocks (
                        BlockIo, 
                        MediaId, 
                        OverRunLba, 
                        BlockSize, 
                        Data
                        );
    if (EFI_ERROR (Status)) {
      goto Done;
    }
  }

Done:
  if (PreData != NULL) {
    gBS->FreePool (PreData);
  }  
  return Status;
}
