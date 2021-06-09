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

  ScsiDisk.c

Abstract:

--*/

#include "scsidisk.h"

EFI_STATUS
ScsiDiskDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
ScsiDiskDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );  

EFI_STATUS
ScsiDiskDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  );

EFI_DRIVER_BINDING_PROTOCOL gScsiDiskDriverBinding = {
  ScsiDiskDriverBindingSupported,
  ScsiDiskDriverBindingStart,
  ScsiDiskDriverBindingStop,
  0x10,
  NULL,
  NULL
};  

EFI_DRIVER_ENTRY_POINT(ScsiDiskDriverEntryPoint)

EFI_STATUS
ScsiDiskDriverEntryPoint(
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/                
{
  EFI_STATUS  Status;

  //
  // Initialize the EFI Library
  //
  Status = EfiLibInstallAllDriverProtocols (
                                        ImageHandle, 
                                        SystemTable, 
                                        &gScsiDiskDriverBinding, 
                                        ImageHandle,
                                        &gScsiDiskComponentName,
                                        NULL,
                                        NULL
                                        );
  return Status;
} 

EFI_STATUS
ScsiDiskDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/                
{
  EFI_STATUS              Status;
  EFI_SCSI_IO_PROTOCOL    *ScsiIo;
  UINT8                   DeviceType;
  
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiScsiIoProtocolGuid,  
                  &ScsiIo,
                  This->DriverBindingHandle,     
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = ScsiIo->GetDeviceType (ScsiIo,&DeviceType);
  if (!EFI_ERROR(Status)) {
    if ((DeviceType == EFI_SCSI_TYPE_DISK) || (DeviceType == EFI_SCSI_TYPE_CDROM)) {
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_UNSUPPORTED;
    }
  }
  
  gBS->CloseProtocol (
                Controller,
                &gEfiScsiIoProtocolGuid,
                This->DriverBindingHandle,
                Controller
                );
  return Status;
}   

EFI_STATUS
ScsiDiskDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/
{
  EFI_STATUS                  Status;
  EFI_SCSI_IO_PROTOCOL        *ScsiIo;
  SCSI_DISK_DEV               *ScsiDiskDevice;
  BOOLEAN                     Temp;
  UINT8                       Index;
  UINT8                       MaxRetry;
  BOOLEAN                     NeedRetry;
  
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (SCSI_DISK_DEV),
                  &ScsiDiskDevice
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EfiZeroMem (ScsiDiskDevice,sizeof(SCSI_DISK_DEV));
  
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiScsiIoProtocolGuid,  
                  &ScsiIo,
                  This->DriverBindingHandle,     
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR(Status)) {
    gBS->FreePool (ScsiDiskDevice);
    return Status;
  }
  
  ScsiDiskDevice->Signature = SCSI_DISK_DEV_SIGNATURE;
  ScsiDiskDevice->ScsiIo = ScsiIo;
  ScsiDiskDevice->BlkIo.Media = &ScsiDiskDevice->BlkIoMedia;
  ScsiDiskDevice->BlkIo.Reset = ScsiDiskReset;
  ScsiDiskDevice->BlkIo.ReadBlocks = ScsiDiskReadBlocks;
  ScsiDiskDevice->BlkIo.WriteBlocks = ScsiDiskWriteBlocks;
  ScsiDiskDevice->BlkIo.FlushBlocks = ScsiDiskFlushBlocks;
  ScsiDiskDevice->Handle = Controller;
  
  ScsiIo->GetDeviceType (ScsiIo,&(ScsiDiskDevice->DeviceType));
  switch (ScsiDiskDevice->DeviceType) {
    case EFI_SCSI_TYPE_DISK:
      ScsiDiskDevice->BlkIo.Media->BlockSize = 0x200;
      break;
      
    case EFI_SCSI_TYPE_CDROM:
      ScsiDiskDevice->BlkIo.Media->BlockSize = 0x800;
      break;
  }
  //
  // The Sense Data Array's initial size is 6
  //
  ScsiDiskDevice->SenseDataNumber = 6;
  Status = gBS->AllocatePool (
              EfiBootServicesData,
              sizeof (EFI_SCSI_SENSE_DATA) * ScsiDiskDevice->SenseDataNumber,
              &(ScsiDiskDevice->SenseData)
              );
  if (EFI_ERROR(Status)) {    
    gBS->CloseProtocol (
         Controller, 
         &gEfiScsiIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    gBS->FreePool (ScsiDiskDevice);
    return Status;
  }
  EfiZeroMem (ScsiDiskDevice->SenseData,
              sizeof (EFI_SCSI_SENSE_DATA) * ScsiDiskDevice->SenseDataNumber);
  
  //
  // Retrive device information
  //  
  MaxRetry = 2;
  for (Index = 0 ; Index < MaxRetry; Index ++ ) {
    Status = ScsiDiskInquiryDevice (ScsiDiskDevice,&NeedRetry);
    if (!EFI_ERROR(Status)) {
      break;
    }    
    if (!NeedRetry) {
      gBS->FreePool (ScsiDiskDevice->SenseData);
      gBS->CloseProtocol (
         Controller, 
         &gEfiScsiIoProtocolGuid,
         This->DriverBindingHandle,   
         Controller   
         );
      gBS->FreePool (ScsiDiskDevice);
      return EFI_DEVICE_ERROR;
    }
  }
  
  //
  // The second parameter "TRUE" means must
  // retrieve media capacity 
  //
  Status = ScsiDiskDetectMedia (ScsiDiskDevice,TRUE,&Temp);
  if (!EFI_ERROR(Status)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
        &Controller,
        &gEfiBlockIoProtocolGuid,   &ScsiDiskDevice->BlkIo,
        NULL
        );
  }
  
  if (EFI_ERROR(Status)) {
    gBS->FreePool (ScsiDiskDevice->SenseData);
    gBS->CloseProtocol (
         Controller, 
         &gEfiScsiIoProtocolGuid,
         This->DriverBindingHandle,   
         Controller   
         );
    gBS->FreePool (ScsiDiskDevice);
    return Status;
  }
  
  ScsiDiskDevice->ControllerNameTable = NULL;
  EfiLibAddUnicodeString (
    "eng", 
    gScsiDiskComponentName.SupportedLanguages, 
    &ScsiDiskDevice->ControllerNameTable,
    L"SCSI Disk Device"
  );
  
  return EFI_SUCCESS;
  
}

  
EFI_STATUS
ScsiDiskDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/
{
  EFI_BLOCK_IO_PROTOCOL       *BlkIo;
  SCSI_DISK_DEV               *ScsiDiskDevice;
  EFI_STATUS                  Status;
    
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiBlockIoProtocolGuid,  
                  &BlkIo,
                  This->DriverBindingHandle,     
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  ScsiDiskDevice = SCSI_DISK_DEV_FROM_THIS (BlkIo);
  Status = gBS->UninstallProtocolInterface (
                  Controller, 
                  &gEfiBlockIoProtocolGuid, 
                  &ScsiDiskDevice->BlkIo
                  );
  if (!EFI_ERROR(Status)) {
    gBS->CloseProtocol (
         Controller, 
         &gEfiScsiIoProtocolGuid,
         This->DriverBindingHandle,   
         Controller   
         );
    
    ReleaseScsiDiskDeviceResources (ScsiDiskDevice);

    return EFI_SUCCESS;
  }
  //
  // errors met
  //
  return Status;
}

//
// Block I/O Protocol Interface
//

EFI_STATUS
ScsiDiskReset (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  BOOLEAN                 ExtendedVerification
  )
{
  SCSI_DISK_DEV       *ScsiDiskDevice;
  EFI_STATUS          Status;
  
  ScsiDiskDevice = SCSI_DISK_DEV_FROM_THIS (This);
  
  Status = ScsiDiskDevice->ScsiIo->ResetDevice (ScsiDiskDevice->ScsiIo);
  
  if (!ExtendedVerification) {
    return Status;
  }
  
  Status = ScsiDiskDevice->ScsiIo->ResetBus (ScsiDiskDevice->ScsiIo);
  
  return Status;
}  

EFI_STATUS
ScsiDiskReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  OUT VOID                    *Buffer
  )
{
  SCSI_DISK_DEV         *ScsiDiskDevice;
  EFI_BLOCK_IO_MEDIA    *Media;
  EFI_STATUS            Status;
  UINTN                 BlockSize;
  UINTN                 NumberOfBlocks;
  BOOLEAN               MediaChange = FALSE;
  
  ScsiDiskDevice = SCSI_DISK_DEV_FROM_THIS (This);
  
  if (!IsDeviceFixed (ScsiDiskDevice)) {
    
    Status = ScsiDiskDetectMedia (ScsiDiskDevice,FALSE,&MediaChange);
    if (EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR;
    }
    if (MediaChange) {
      gBS->ReinstallProtocolInterface (ScsiDiskDevice->Handle,
                                        &gEfiBlockIoProtocolGuid,
                                        &ScsiDiskDevice->BlkIo,
                                        &ScsiDiskDevice->BlkIo
                                        );  
    }
  }
  
  //
  //Get the intrinsic block size
  //
  Media = ScsiDiskDevice->BlkIo.Media ;
  BlockSize = Media->BlockSize ;

  NumberOfBlocks = BufferSize / BlockSize ;

  //
  //Check buffer alignment
  //
  if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
    return EFI_INVALID_PARAMETER;
  }

  if ( !(Media -> MediaPresent)) {
    return EFI_NO_MEDIA;
  }
  
  if (MediaId != Media->MediaId) {
    return EFI_MEDIA_CHANGED;
  }
  
  if (LBA > Media->LastBlock) {    
    return EFI_INVALID_PARAMETER;
  }
  
  if ((LBA + NumberOfBlocks - 1) > Media->LastBlock) {    
    return EFI_INVALID_PARAMETER;
  }
  
  if (BufferSize % BlockSize != 0) {    
    return EFI_BAD_BUFFER_SIZE;
  }
  
  if (Buffer == NULL) {    
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0 ) {    
    return EFI_SUCCESS;
  }
  
  //
  // if all the parameters are valid, then perform read sectors command
  // to transfer data from device to host.
  //
  Status = ScsiDiskReadSectors (ScsiDiskDevice,Buffer,LBA,NumberOfBlocks);
  
  return Status;
}


EFI_STATUS
ScsiDiskWriteBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  IN  VOID                    *Buffer
  )
{
  SCSI_DISK_DEV         *ScsiDiskDevice;
  EFI_BLOCK_IO_MEDIA    *Media;
  EFI_STATUS            Status;
  UINTN                 BlockSize;
  UINTN                 NumberOfBlocks;
  BOOLEAN               MediaChange = FALSE;
  
  ScsiDiskDevice = SCSI_DISK_DEV_FROM_THIS (This);
  
  if (!IsDeviceFixed (ScsiDiskDevice)) {
    
    Status = ScsiDiskDetectMedia (ScsiDiskDevice,FALSE,&MediaChange);
    if (EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR;
    }
    if (MediaChange) {
      gBS->ReinstallProtocolInterface (ScsiDiskDevice->Handle,
                                        &gEfiBlockIoProtocolGuid,
                                        &ScsiDiskDevice->BlkIo,
                                        &ScsiDiskDevice->BlkIo
                                        );  
    }
  }
  
  //
  //Get the intrinsic block size
  //
  Media = ScsiDiskDevice->BlkIo.Media ;
  BlockSize = Media->BlockSize ;

  NumberOfBlocks = BufferSize / BlockSize ;

  //
  //Check buffer alignment
  //
  if((Media->IoAlign > 1) && ((UINTN)Buffer & (UINTN)(Media->IoAlign -1 ))){
    return EFI_INVALID_PARAMETER;
  }
  

  if ( !(Media -> MediaPresent)) {
    return EFI_NO_MEDIA;
  }
  
  if (MediaId != Media->MediaId) {
     return EFI_MEDIA_CHANGED;
  }

  if (Media->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }
  
  if (LBA > Media->LastBlock) {    
    return EFI_INVALID_PARAMETER;
  }
  
  if ((LBA + NumberOfBlocks - 1) > Media->LastBlock) {    
    return EFI_INVALID_PARAMETER;
  }
  
  if (BufferSize % BlockSize != 0) {    
    return EFI_BAD_BUFFER_SIZE;
  }
  
  if (Buffer == NULL) {    
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0 ) {    
    return EFI_SUCCESS;
  }
  
  //
  // if all the parameters are valid, then perform read sectors command
  // to transfer data from device to host.
  //
  Status = ScsiDiskWriteSectors (ScsiDiskDevice,Buffer,LBA, NumberOfBlocks);
  
  return Status;
}

EFI_STATUS
ScsiDiskFlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL   *This
  )
{
  //
  // return directly
  //
  return EFI_SUCCESS;
}


EFI_STATUS
ScsiDiskDetectMedia (
  SCSI_DISK_DEV   *ScsiDiskDevice,
  BOOLEAN         MustReadCapacity,
  BOOLEAN         *MediaChange
  )
{
  EFI_STATUS              Status;
  EFI_STATUS              ReadCapacityStatus;
  EFI_SCSI_SENSE_DATA     *SenseData;
  UINTN                   NumberOfSenseKeys;
  BOOLEAN                 NeedRetry;
  BOOLEAN                 NeedReadCapacity;
  UINT8                   Index;
  UINT8                   MaxRetry;
  EFI_BLOCK_IO_MEDIA      OldMedia;  
  UINTN                   Action;
  
  Status            = EFI_SUCCESS;
  ReadCapacityStatus = EFI_SUCCESS;
  SenseData         = NULL;
  NumberOfSenseKeys = 0;
  NeedReadCapacity  = FALSE;
  OldMedia          = *(ScsiDiskDevice->BlkIo.Media);
  
  *MediaChange      = FALSE;
  
  MaxRetry = 3;
  for (Index = 0 ; Index < MaxRetry; Index ++ ) {
    Status = ScsiDiskTestUnitReady (ScsiDiskDevice,
                                    &NeedRetry,
                                    &SenseData,
                                    &NumberOfSenseKeys
                                    );
    if (!EFI_ERROR(Status)) {
      break;
    }
    
    if (!NeedRetry) {
      return Status;
    }
  }
  
  if ((Index == MaxRetry) && EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
  
  Status = DetectMediaParsingSenseKeys (ScsiDiskDevice,SenseData,
                                        NumberOfSenseKeys,&Action);
  if (EFI_ERROR(Status)){
    return Status;
  }
  
  //
  // ACTION_NO_ACTION: need not read capacity
  // other action code: need read capacity 
  //
  if (Action == ACTION_NO_ACTION) {
    NeedReadCapacity = FALSE;
  } else {
    NeedReadCapacity = TRUE;
  }    
  
  //
  // either NeedReadCapacity is TRUE, or MustReadCapacity is TRUE,
  // retrieve capacity via Read Capacity command
  //
  if (NeedReadCapacity || MustReadCapacity) {
    
    //
    // retrieve media information
    //
    MaxRetry = 3;
    for (Index = 0; Index < MaxRetry; Index ++) {
      
      ReadCapacityStatus = ScsiDiskReadCapacity (ScsiDiskDevice,
                                                 &NeedRetry,
                                                 &SenseData,
                                                 &NumberOfSenseKeys
                                                 );
      if (EFI_ERROR(ReadCapacityStatus) && !NeedRetry) {
        return EFI_DEVICE_ERROR;
      }
      //
      // analyze sense key to action
      //
      Status = DetectMediaParsingSenseKeys (ScsiDiskDevice,
                                             SenseData,
                                             NumberOfSenseKeys,
                                             &Action
                                             );
      //
      // if Status is error, it may indicate crisis error,
      // so return without retry.
      //
      if (EFI_ERROR(Status)) {
        return Status;
      }
      switch (Action) {
        case ACTION_NO_ACTION:
          //
          // no retry
          //
          Index = MaxRetry;
          break;
          
        case ACTION_RETRY_COMMAND_LATER:
          //
          // retry the ReadCapacity later and continuously, until the condition
          // no longer emerges.
          // stall time is 100000us, or say 0.1 second.
          //
          gBS->Stall (100000);
          Index = 0;
          break;
          
        default:
          //
          // other cases, just retry the command
          //
          break;          
      }
    }

    if ( (Index == MaxRetry) && EFI_ERROR(ReadCapacityStatus)) {
      return EFI_DEVICE_ERROR;
    }
  } 

  if (ScsiDiskDevice->BlkIo.Media->MediaId != OldMedia.MediaId) {
    //
    // Media change information got from the device
    //
    *MediaChange = TRUE;
  }
  
  if (ScsiDiskDevice->BlkIo.Media->ReadOnly != OldMedia.ReadOnly) {
    *MediaChange = TRUE;
    ScsiDiskDevice->BlkIo.Media->MediaId += 1;    
  }
  
  if (ScsiDiskDevice->BlkIo.Media->BlockSize != OldMedia.BlockSize) {    
    *MediaChange = TRUE;
    ScsiDiskDevice->BlkIo.Media->MediaId += 1;    
  }
  
  if (ScsiDiskDevice->BlkIo.Media->LastBlock != OldMedia.LastBlock) {    
    *MediaChange = TRUE;
    ScsiDiskDevice->BlkIo.Media->MediaId += 1;    
  }
  
  if (ScsiDiskDevice->BlkIo.Media->MediaPresent != OldMedia.MediaPresent) {
    if (ScsiDiskDevice->BlkIo.Media->MediaPresent) {
      //
      // when change from no media to media present, reset the MediaId to 1.
      //
      ScsiDiskDevice->BlkIo.Media->MediaId = 1 ;
    } else {
      //
      // when no media, reset the MediaId to zero.
      //
      ScsiDiskDevice->BlkIo.Media->MediaId = 0 ;
    }
    *MediaChange = TRUE;
  }
    
  return EFI_SUCCESS;
}    

EFI_STATUS
ScsiDiskInquiryDevice (
  SCSI_DISK_DEV   *ScsiDiskDevice,
  BOOLEAN         *NeedRetry
  )
{
  UINT32                  InquiryDataLength;
  UINT8                   SenseDataLength;
  UINT8                   HostAdapterStatus;
  UINT8                   TargetStatus;
  EFI_SCSI_SENSE_DATA     *SenseDataArray;
  UINTN                   NumberOfSenseKeys;
  EFI_STATUS              Status;
  UINT8                   MaxRetry;
  UINT8                   Index;  
  
  InquiryDataLength = sizeof (EFI_SCSI_INQUIRY_DATA);
  SenseDataLength = 0;
  
  Status = SubmitInquiryCommand (
                      ScsiDiskDevice->ScsiIo,
                      EfiScsiStallSeconds(1),
                      NULL,
                      &SenseDataLength,
                      &HostAdapterStatus,
                      &TargetStatus,
                      (VOID*)&(ScsiDiskDevice->InquiryData),
                      &InquiryDataLength,
                      FALSE
                      );
  switch (Status) {
    //
    // no need to check HostAdapterStatus and TargetStatus
    //
    case EFI_SUCCESS:
    case EFI_WARN_BUFFER_TOO_SMALL:
      ParseInquiryData (ScsiDiskDevice);
      return EFI_SUCCESS;
      
    case EFI_NOT_READY:
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
      
    case EFI_INVALID_PARAMETER:
    case EFI_UNSUPPORTED:
      *NeedRetry = FALSE;
      return EFI_DEVICE_ERROR;
    
    //
    // go ahead to check HostAdapterStatus and TargetStatus
    // (EFI_TIMEOUT, EFI_DEVICE_ERROR)
    //
    default:
      break;
  }
  
  Status = CheckHostAdapterStatus (HostAdapterStatus);
  switch (Status) {
    case EFI_SUCCESS:
      break;
      
    case EFI_TIMEOUT:
    case EFI_NOT_READY:
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
      
    case EFI_DEVICE_ERROR:
      //
      // reset the scsi channel
      //
      ScsiDiskDevice->ScsiIo->ResetBus (ScsiDiskDevice->ScsiIo);
      *NeedRetry = FALSE;
      return EFI_DEVICE_ERROR;
  }
  
  Status = CheckTargetStatus (TargetStatus);
  switch (Status) {
    case EFI_SUCCESS:
      break;
      
    case EFI_NOT_READY:
      //
      // reset the scsi device
      //
      ScsiDiskDevice->ScsiIo->ResetDevice (ScsiDiskDevice->ScsiIo);
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
      
    case EFI_DEVICE_ERROR:
      *NeedRetry = FALSE;
      return EFI_DEVICE_ERROR;
  }
  
  //
  // if goes here, meant SubmitInquiryCommand() failed. 
  // if ScsiDiskRequestSenseKeys() succeeds at last, 
  // better retry SubmitInquiryCommand(). (by setting *NeedRetry = TRUE)
  //
  MaxRetry = 3;
  for (Index = 0; Index < MaxRetry; Index ++) {
    
    Status = ScsiDiskRequestSenseKeys (ScsiDiskDevice,
                                        NeedRetry,
                                        &SenseDataArray,
                                        &NumberOfSenseKeys,
                                        TRUE);
    if (!EFI_ERROR(Status)) {
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
    }
    
    if (*NeedRetry == FALSE) {
      return EFI_DEVICE_ERROR;
    }
  }
  //
  // ScsiDiskRequestSenseKeys() failed after several rounds of retry.
  // set *NeedRetry = FALSE to avoid the outside caller try again.
  //
  *NeedRetry = FALSE;
  return EFI_DEVICE_ERROR;
}


EFI_STATUS
ScsiDiskTestUnitReady (
  SCSI_DISK_DEV         *ScsiDiskDevice,
  BOOLEAN               *NeedRetry,
  EFI_SCSI_SENSE_DATA   **SenseDataArray,
  UINTN                 *NumberOfSenseKeys
  )
/*
  When Test Unit Ready command succeeds,
  retrieve Sense Keys via Request Sense;
  When Test Unit Ready command encounters any error caused by host adapter or
  target, return error without retrieving Sense Keys.
*/
{
  EFI_STATUS      Status;
  UINT8           SenseDataLength;
  UINT8           HostAdapterStatus;
  UINT8           TargetStatus;
  UINT8           Index;
  UINT8           MaxRetry;
  
  SenseDataLength = 0;
  *NumberOfSenseKeys = 0;

  //
  // Parameter 3 and 4: do not require sense data, retrieve it when needed.
  //
  Status = SubmitTestUnitReadyCommand (
                                  ScsiDiskDevice->ScsiIo,
                                  EfiScsiStallSeconds(1),
                                  NULL,
                                  &SenseDataLength,
                                  &HostAdapterStatus,
                                  &TargetStatus
                                  );
  switch (Status) {
    //
    // no need to check HostAdapterStatus and TargetStatus
    //
    case EFI_NOT_READY:
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
      
    case EFI_INVALID_PARAMETER:
    case EFI_UNSUPPORTED:
      *NeedRetry = FALSE;
      return EFI_DEVICE_ERROR;
    
    //
    // go ahead to check HostAdapterStatus and TargetStatus
    //
    default:
      break;
  }
  
  Status = CheckHostAdapterStatus (HostAdapterStatus);
  switch (Status) {
    case EFI_SUCCESS:
      break;
      
    case EFI_TIMEOUT:
    case EFI_NOT_READY:
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
      
    case EFI_DEVICE_ERROR:
      //
      // reset the scsi channel
      //
      ScsiDiskDevice->ScsiIo->ResetBus (ScsiDiskDevice->ScsiIo);
      *NeedRetry = FALSE;
      return EFI_DEVICE_ERROR;
  }
  
  Status = CheckTargetStatus (TargetStatus);
  switch (Status) {
    case EFI_SUCCESS:
      break;
      
    case EFI_NOT_READY:
      //
      // reset the scsi device
      //
      ScsiDiskDevice->ScsiIo->ResetDevice (ScsiDiskDevice->ScsiIo);
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
      
    case EFI_DEVICE_ERROR:
      *NeedRetry = FALSE;
      return EFI_DEVICE_ERROR;
  }
  
  MaxRetry = 3;
  for (Index = 0; Index < MaxRetry; Index ++) {
    
    Status = ScsiDiskRequestSenseKeys (ScsiDiskDevice,
                                        NeedRetry,
                                        SenseDataArray,
                                        NumberOfSenseKeys,
                                        FALSE
                                        );
    if (!EFI_ERROR(Status)) {
      return EFI_SUCCESS;
    }
    if (*NeedRetry == FALSE) {
      return EFI_DEVICE_ERROR;
    }
  }
  //
  // ScsiDiskRequestSenseKeys() failed after several rounds of retry.
  // set *NeedRetry = FALSE to avoid the outside caller try again.
  //
  *NeedRetry = FALSE;
  return EFI_DEVICE_ERROR;
}

EFI_STATUS
DetectMediaParsingSenseKeys (
  SCSI_DISK_DEV           *ScsiDiskDevice,
  EFI_SCSI_SENSE_DATA     *SenseData,
  UINTN                   NumberOfSenseKeys,
  UINTN                   *Action
  )
{
  BOOLEAN                 RetryLater;
  
  //
  // Default is to read capacity, unless..
  //
  *Action = ACTION_READ_CAPACITY;
  
  if (NumberOfSenseKeys == 0) {
    *Action = ACTION_NO_ACTION;
    return EFI_SUCCESS;
  }
  
  if (!ScsiDiskHaveSenseKey (SenseData,NumberOfSenseKeys)) {
    //
    // No Sense Key returned from last submitted command
    //
    *Action = ACTION_NO_ACTION;
    return EFI_SUCCESS;
  }
  
  if (ScsiDiskIsNoMedia (SenseData,NumberOfSenseKeys)) {    
    ScsiDiskDevice->BlkIo.Media->MediaPresent = FALSE;
    ScsiDiskDevice->BlkIo.Media->LastBlock = 0;
    *Action = ACTION_NO_ACTION;
    return EFI_SUCCESS;
  }
  
  if (ScsiDiskIsMediaChange (SenseData,NumberOfSenseKeys)) {
    ScsiDiskDevice->BlkIo.Media->MediaId++;
    return EFI_SUCCESS;
  }
  
  if (ScsiDiskIsMediaError (SenseData,NumberOfSenseKeys)) {
    ScsiDiskDevice->BlkIo.Media->MediaPresent = FALSE;
    ScsiDiskDevice->BlkIo.Media->LastBlock = 0;
    return EFI_DEVICE_ERROR;
  }
  
  if (ScsiDiskIsHardwareError (SenseData,NumberOfSenseKeys)) {
    return EFI_DEVICE_ERROR;
  }
  
  if (!ScsiDiskIsDriveReady (SenseData,NumberOfSenseKeys,&RetryLater)) {
    if (RetryLater) {
      *Action = ACTION_RETRY_COMMAND_LATER;
      return EFI_SUCCESS;
    } 
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}    


EFI_STATUS
ScsiDiskReadCapacity (
  SCSI_DISK_DEV           *ScsiDiskDevice,
  BOOLEAN                 *NeedRetry,
  EFI_SCSI_SENSE_DATA     **SenseDataArray,
  UINTN                   *NumberOfSenseKeys
  )
{
  EFI_SCSI_DISK_CAPACITY_DATA     CapacityData;
  UINT32                          DataLength;
  UINT8                           HostAdapterStatus;
  UINT8                           TargetStatus;
  EFI_STATUS                      CommandStatus;
  EFI_STATUS                      Status;
  UINT8                           Index;
  UINT8                           MaxRetry;
  UINT8                           SenseDataLength; 
  
  SenseDataLength = 0;
  EfiZeroMem (&CapacityData,sizeof (EFI_SCSI_DISK_CAPACITY_DATA));
  DataLength = sizeof(EFI_SCSI_DISK_CAPACITY_DATA);
  
  *NumberOfSenseKeys = 0;
  *NeedRetry = FALSE;
  //
  // submit Read Capacity Command. in this call,not request sense data
  //
  CommandStatus = SubmitReadCapacityCommand (
                            ScsiDiskDevice->ScsiIo,
                            EfiScsiStallSeconds(1),
                            NULL,
                            &SenseDataLength,
                            &HostAdapterStatus,
                            &TargetStatus,
                            (VOID*)&CapacityData,
                            &DataLength,
                            FALSE
                            );
  switch (CommandStatus) {
    //
    // no need to check HostAdapterStatus and TargetStatus
    //
    case EFI_SUCCESS:
      GetMediaInfo (ScsiDiskDevice,&CapacityData);      
      return EFI_SUCCESS;
      
    case EFI_NOT_READY:
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
      
    case EFI_INVALID_PARAMETER:
    case EFI_UNSUPPORTED:
      *NeedRetry = FALSE;
      return EFI_DEVICE_ERROR;
    
    //
    // go ahead to check HostAdapterStatus and TargetStatus
    // (EFI_TIMEOUT, EFI_DEVICE_ERROR, EFI_WARN_BUFFER_TOO_SMALL)
    //
    default:
      break;
  }
  
  Status = CheckHostAdapterStatus (HostAdapterStatus);
  switch (Status) {
    case EFI_SUCCESS:
      break;
      
    case EFI_TIMEOUT:
    case EFI_NOT_READY:
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
      
    case EFI_DEVICE_ERROR:
      //
      // reset the scsi channel
      //
      ScsiDiskDevice->ScsiIo->ResetBus (ScsiDiskDevice->ScsiIo);
      *NeedRetry = FALSE;
      return EFI_DEVICE_ERROR;
  }
  
  Status = CheckTargetStatus (TargetStatus);
  switch (Status) {
    case EFI_SUCCESS:
      break;
      
    case EFI_NOT_READY:
      //
      // reset the scsi device
      //
      ScsiDiskDevice->ScsiIo->ResetDevice (ScsiDiskDevice->ScsiIo);
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
      
    case EFI_DEVICE_ERROR:
      *NeedRetry = FALSE;
      return EFI_DEVICE_ERROR;
  }
  
  //
  // if goes here, meant SubmitReadCapacityCommand() failed. 
  // if ScsiDiskRequestSenseKeys() succeeds at last, 
  // better retry SubmitReadCapacityCommand(). (by setting *NeedRetry = TRUE)
  //
  MaxRetry = 3;
  for (Index = 0; Index < MaxRetry; Index ++) {
    
    Status = ScsiDiskRequestSenseKeys (ScsiDiskDevice,
                                        NeedRetry,
                                        SenseDataArray,
                                        NumberOfSenseKeys,
                                        TRUE);
    if (!EFI_ERROR(Status)) {
      *NeedRetry = TRUE;
      return EFI_DEVICE_ERROR;
    }
    if (*NeedRetry == FALSE) {
      return EFI_DEVICE_ERROR;
    }
  }
  //
  // ScsiDiskRequestSenseKeys() failed after several rounds of retry.
  // set *NeedRetry = FALSE to avoid the outside caller try again.
  //
  *NeedRetry = FALSE;
  return EFI_DEVICE_ERROR;
}

EFI_STATUS
CheckHostAdapterStatus (
  UINT8   HostAdapterStatus
  )
{
  switch (HostAdapterStatus) {
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_OK:
      return EFI_SUCCESS;
      
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_SELECTION_TIMEOUT:
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_TIMEOUT:
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_TIMEOUT_COMMAND:
      return EFI_TIMEOUT;
      
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_MESSAGE_REJECT:
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_PARITY_ERROR:
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_REQUEST_SENSE_FAILED:
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_DATA_OVERRUN_UNDERRUN:
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_BUS_RESET:
      return EFI_NOT_READY;
      
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_BUS_FREE:
    case EFI_SCSI_IO_STATUS_HOST_ADAPTER_PHASE_ERROR:
      return EFI_DEVICE_ERROR;
      
    default:
      return EFI_SUCCESS;
  }
}

EFI_STATUS
CheckTargetStatus (
  UINT8   TargetStatus
  )
{
  switch (TargetStatus) {
    case EFI_SCSI_IO_STATUS_TARGET_GOOD:
    case EFI_SCSI_IO_STATUS_TARGET_CHECK_CONDITION:
    case EFI_SCSI_IO_STATUS_TARGET_CONDITION_MET:
      return EFI_SUCCESS;
    
    case EFI_SCSI_IO_STATUS_TARGET_INTERMEDIATE:
    case EFI_SCSI_IO_STATUS_TARGET_INTERMEDIATE_CONDITION_MET:
    case EFI_SCSI_IO_STATUS_TARGET_BUSY:
    case EFI_SCSI_IO_STATUS_TARGET_COMMOND_TERMINATED:
    case EFI_SCSI_IO_STATUS_TARGET_QUEUE_FULL:
      return EFI_NOT_READY;
      
    case EFI_SCSI_IO_STATUS_TARGET_RESERVATION_CONFLICT:
      return EFI_DEVICE_ERROR;
      break;
    
    default:
      return EFI_SUCCESS;   
  }
}

EFI_STATUS
ScsiDiskRequestSenseKeys (
  SCSI_DISK_DEV           *ScsiDiskDevice,
  BOOLEAN                 *NeedRetry,
  EFI_SCSI_SENSE_DATA     **SenseDataArray,
  UINTN                   *NumberOfSenseKeys,
  BOOLEAN                 AskResetIfError
  )
/*
  Retrieve all sense keys from the device.
  When encountering error during the process,
  if retrieve sense keys before error encounterred,
  return the sense keys with return status set to EFI_SUCCESS,
  and NeedRetry set to FALSE; otherwize, return the proper return
  status.
*/  
{
  EFI_SCSI_SENSE_DATA     *PtrSenseData;
  UINT8                   SenseDataLength;
  BOOLEAN                 SenseReq;
  EFI_STATUS              Status;
  EFI_STATUS              FallStatus;
  UINT8                   HostAdapterStatus;
  UINT8                   TargetStatus;
  
  FallStatus = EFI_SUCCESS;
  SenseDataLength = sizeof (EFI_SCSI_SENSE_DATA);
  
  EfiZeroMem (ScsiDiskDevice->SenseData, 
              sizeof(EFI_SCSI_SENSE_DATA)*(ScsiDiskDevice->SenseDataNumber)
              );
  
  *NumberOfSenseKeys = 0;
  *SenseDataArray = ScsiDiskDevice->SenseData;
  PtrSenseData = ScsiDiskDevice->SenseData;
  
  for (SenseReq = TRUE; SenseReq == TRUE;) {
    
    Status = SubmitRequestSenseCommand (ScsiDiskDevice->ScsiIo,
                                        EfiScsiStallSeconds(2),
                                        PtrSenseData,
                                        &SenseDataLength,
                                        &HostAdapterStatus,
                                        &TargetStatus
                                        );    
    switch (Status) {
      
      case EFI_SUCCESS:
      case EFI_WARN_BUFFER_TOO_SMALL: // fall through
        FallStatus = EFI_SUCCESS;
        break;
        
      case EFI_TIMEOUT:      
      case EFI_NOT_READY: // fall through
        *NeedRetry = TRUE;
        FallStatus = EFI_DEVICE_ERROR;
        break;
        
      case EFI_INVALID_PARAMETER:
      case EFI_UNSUPPORTED:
        *NeedRetry = FALSE;
        FallStatus = EFI_DEVICE_ERROR;
        break;
      
      case EFI_DEVICE_ERROR:
        if (AskResetIfError) {
          ScsiDiskDevice->ScsiIo->ResetDevice (ScsiDiskDevice->ScsiIo);
        }
        FallStatus = EFI_DEVICE_ERROR;
        break;
    }
    
    if (EFI_ERROR(FallStatus)) {
      if (*NumberOfSenseKeys != 0) {
        *NeedRetry = FALSE;
        return EFI_SUCCESS;
      } else {
        return EFI_DEVICE_ERROR;
      }
    }
  
    (*NumberOfSenseKeys) += 1;
    
    //
    // no more sense key or number of sense keys exceeds predefined,
    // skip the loop.
    //
    if ((PtrSenseData->Sense_Key == EFI_SCSI_SK_NO_SENSE)
        || (*NumberOfSenseKeys == ScsiDiskDevice->SenseDataNumber)) {
      SenseReq = FALSE;
    }
    
    PtrSenseData += 1;
    
  }
  
  return EFI_SUCCESS;
}

VOID
GetMediaInfo (
  SCSI_DISK_DEV                 *ScsiDiskDevice,
  EFI_SCSI_DISK_CAPACITY_DATA   *Capacity
  )
{
  ScsiDiskDevice->BlkIo.Media->LastBlock =  (Capacity->LastLba3 << 24) |
                                            (Capacity->LastLba2 << 16) |
                                            (Capacity->LastLba1 << 8)  |
                                             Capacity->LastLba0;
      
  ScsiDiskDevice->BlkIo.Media->MediaPresent = TRUE;
  ScsiDiskDevice->BlkIo.Media->BlockSize = (Capacity->BlockSize3 << 24) |
                                           (Capacity->BlockSize2 << 16) | 
                                           (Capacity->BlockSize1 << 8)  |
                                            Capacity->BlockSize0;
  if (ScsiDiskDevice->DeviceType == EFI_SCSI_TYPE_DISK) {
    ScsiDiskDevice->BlkIo.Media->BlockSize = 0x200;
  }
  
  if (ScsiDiskDevice->DeviceType == EFI_SCSI_TYPE_CDROM) {
    ScsiDiskDevice->BlkIo.Media->BlockSize = 0x800;
  }
}

VOID
ParseInquiryData (
  SCSI_DISK_DEV   *ScsiDiskDevice
  )
{
  ScsiDiskDevice->FixedDevice = (BOOLEAN)(ScsiDiskDevice->InquiryData.RMB ? 0 : 1);
  ScsiDiskDevice->BlkIoMedia.RemovableMedia = (BOOLEAN)(!ScsiDiskDevice->FixedDevice);
}

EFI_STATUS
ScsiDiskReadSectors (
  SCSI_DISK_DEV     *ScsiDiskDevice,
  VOID              *Buffer,
  EFI_LBA           Lba,
  UINTN             NumberOfBlocks
  )
{
  UINTN                 BlocksRemaining;
  UINT32                Lba32;
  UINT8                 *PtrBuffer;
  UINT32                BlockSize;
  UINT32                ByteCount;
  UINT32                MaxBlock;
  UINT32                SectorCount;
  UINT64                Timeout;
  EFI_STATUS            Status;
  UINT8                 Index;
  UINT8                 MaxRetry;
  BOOLEAN               NeedRetry;
  EFI_SCSI_SENSE_DATA   *SenseData;
  UINT8                 SenseDataLength;
  UINTN                 NumberOfSenseKeys;
  
  SenseData = NULL;
  SenseDataLength = 0;
  NumberOfSenseKeys = 0;
  
  Status = EFI_SUCCESS;
  
  BlocksRemaining = NumberOfBlocks;
  BlockSize = ScsiDiskDevice->BlkIo.Media->BlockSize;
  //
  // limit the data bytes that can be transferred by one Read(10) Command
  //
  MaxBlock = 65536;
  
  PtrBuffer = Buffer;
  Lba32 = (UINT32)Lba;
  
  while (BlocksRemaining > 0) {
    
    if (BlocksRemaining <= MaxBlock) {
      
      SectorCount = (UINT16)BlocksRemaining ;
    } else {
      
      SectorCount = MaxBlock;
    }
    
    ByteCount = SectorCount * BlockSize;
    Timeout = EfiScsiStallSeconds(2);
    
    MaxRetry = 2;
    for (Index = 0; Index < MaxRetry; Index ++) {
      
      Status = ScsiDiskRead10 (ScsiDiskDevice,
                               &NeedRetry,
                               &SenseData,
                               &NumberOfSenseKeys,
                               Timeout,
                               PtrBuffer,
                               &ByteCount,
                               Lba32,
                               SectorCount
                               );
      if (!EFI_ERROR(Status)) {
        break;
      }
      
      if (!NeedRetry) {
        return EFI_DEVICE_ERROR;
      }

    }
    
    if ((Index == MaxRetry) && (Status != EFI_SUCCESS)) {
      return EFI_DEVICE_ERROR;
    }
    
    //
    // actual transferred sectors
    //
    SectorCount = ByteCount / BlockSize;
    
    Lba32 += SectorCount;
    PtrBuffer = PtrBuffer + SectorCount * BlockSize ;
    BlocksRemaining -= SectorCount;
  }
  
  return EFI_SUCCESS;
}    


EFI_STATUS
ScsiDiskWriteSectors (
  SCSI_DISK_DEV     *ScsiDiskDevice,
  VOID              *Buffer,
  EFI_LBA           Lba,
  UINTN             NumberOfBlocks
  )
{
  UINTN                 BlocksRemaining;
  UINT32                Lba32;
  UINT8                 *PtrBuffer;
  UINT32                BlockSize, ByteCount;
  UINT32                MaxBlock,SectorCount;
  UINT64                Timeout;
  EFI_STATUS            Status;
  UINT8                 Index;
  UINT8                 MaxRetry;
  BOOLEAN               NeedRetry;
  EFI_SCSI_SENSE_DATA   *SenseData;
  UINT8                 SenseDataLength;
  UINTN                 NumberOfSenseKeys;
  
  SenseData = NULL;
  SenseDataLength = 0;
  NumberOfSenseKeys = 0;
  
  Status = EFI_SUCCESS;
    
  BlocksRemaining = NumberOfBlocks;
  BlockSize = ScsiDiskDevice->BlkIo.Media->BlockSize;
  //
  // limit the data bytes that can be transferred by one Write(10) Command
  //
  MaxBlock = 65536;
  
  PtrBuffer = Buffer;
  Lba32 = (UINT32)Lba;
  
  while (BlocksRemaining > 0) {
    
    if (BlocksRemaining <= MaxBlock) {
      
      SectorCount = (UINT16)BlocksRemaining ;
    } else {
      
      SectorCount = MaxBlock ;
    }
    
    ByteCount = SectorCount * BlockSize;
    Timeout = EfiScsiStallSeconds (2);
    MaxRetry = 2;
    for (Index = 0; Index < MaxRetry; Index ++) {
      Status = ScsiDiskWrite10 (ScsiDiskDevice,
                                &NeedRetry,
                                &SenseData,
                                &NumberOfSenseKeys,
                                Timeout,
                                PtrBuffer,
                                &ByteCount,
                                Lba32,
                                SectorCount
                                );
      if (!EFI_ERROR(Status)) {
        break;
      }
      
      if (!NeedRetry) {
        return EFI_DEVICE_ERROR;
      }
    }    
    
    if ((Index == MaxRetry) && (Status != EFI_SUCCESS)) {
      return EFI_DEVICE_ERROR;
    }
    //
    // actual transferred sectors
    //
    SectorCount = ByteCount / BlockSize;
   
    Lba32 += SectorCount;
    PtrBuffer = PtrBuffer + SectorCount * BlockSize ;
    BlocksRemaining -= SectorCount;
  }
  
  return EFI_SUCCESS;
}    

EFI_STATUS
ScsiDiskRead10 (
  SCSI_DISK_DEV         *ScsiDiskDevice,
  BOOLEAN               *NeedRetry,
  EFI_SCSI_SENSE_DATA   **SenseDataArray,
  UINTN                 *NumberOfSenseKeys,
  UINT64                Timeout,
  UINT8                 *DataBuffer,
  UINT32                *DataLength,
  UINT32                StartLba,
  UINT32                SectorSize
  )
{
  UINT8                 SenseDataLength;
  EFI_STATUS            Status;
  UINT8                 HostAdapterStatus;
  UINT8                 TargetStatus;
  
  *NeedRetry = FALSE;
  *NumberOfSenseKeys = 0;
  SenseDataLength = 0;
  Status = SubmitRead10Command (ScsiDiskDevice->ScsiIo,
                                Timeout,
                                NULL,
                                &SenseDataLength,
                                &HostAdapterStatus,
                                &TargetStatus,
                                DataBuffer,
                                DataLength,
                                StartLba,
                                SectorSize
                                );
  return Status;
}

EFI_STATUS
ScsiDiskWrite10 (
  SCSI_DISK_DEV         *ScsiDiskDevice,
  BOOLEAN               *NeedRetry,
  EFI_SCSI_SENSE_DATA   **SenseDataArray,
  UINTN                 *NumberOfSenseKeys,
  UINT64                Timeout,
  UINT8                 *DataBuffer,
  UINT32                *DataLength,
  UINT32                StartLba,
  UINT32                SectorSize
  )
{
  EFI_STATUS            Status;
  UINT8                 SenseDataLength;
  UINT8                 HostAdapterStatus;
  UINT8                 TargetStatus;
  
  *NeedRetry = FALSE;
  *NumberOfSenseKeys = 0;
  SenseDataLength = 0;  
  Status = SubmitWrite10Command (ScsiDiskDevice->ScsiIo,
                                  Timeout,
                                  NULL,
                                  &SenseDataLength,
                                  &HostAdapterStatus,
                                  &TargetStatus,
                                  DataBuffer,
                                  DataLength,
                                  StartLba,
                                  SectorSize
                                  );
  return Status;
}

BOOLEAN
ScsiDiskIsNoMedia(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  EFI_SCSI_SENSE_DATA   *SensePtr;
  UINTN                 Index;
  BOOLEAN               IsNoMedia;

  IsNoMedia = FALSE;
  SensePtr = SenseData;

  for (Index = 0; Index < SenseCounts; Index ++) {
    
    //
    // Sense Key is EFI_SCSI_SK_NOT_READY (0x2),
    // Additional Sense Code is ASC_NO_MEDIA (0x3A)
    //
    if ((SensePtr->Sense_Key == EFI_SCSI_SK_NOT_READY)
        && (SensePtr->Addnl_Sense_Code == EFI_SCSI_ASC_NO_MEDIA)) {
      IsNoMedia = TRUE;
    }
    
    SensePtr ++ ;
  }

  return IsNoMedia;
}


BOOLEAN
ScsiDiskIsMediaError(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  EFI_SCSI_SENSE_DATA *SensePtr;
  UINTN               Index;
  BOOLEAN             IsError;

  IsError = FALSE;
  SensePtr = SenseData ;

  for (Index = 0; Index < SenseCounts; Index ++) {
  
    switch (SensePtr->Sense_Key) {
      
      case EFI_SCSI_SK_MEDIUM_ERROR:
        //
        // Sense Key is EFI_SCSI_SK_MEDIUM_ERROR (0x3)
        //
        switch (SensePtr->Addnl_Sense_Code) {
          case EFI_SCSI_ASC_MEDIA_ERR1:  // fall through
          case EFI_SCSI_ASC_MEDIA_ERR2:  // fall through
          case EFI_SCSI_ASC_MEDIA_ERR3:  // fall through
          case EFI_SCSI_ASC_MEDIA_ERR4:
            IsError = TRUE;
            break;
          
          default:
            break;
        }
        
        break;

      case EFI_SCSI_SK_NOT_READY:
        //
        // Sense Key is EFI_SCSI_SK_NOT_READY (0x2)
        //
        switch (SensePtr->Addnl_Sense_Code) {
          //
          // Additional Sense Code is ASC_MEDIA_UPSIDE_DOWN (0x6)
          //
          case EFI_SCSI_ASC_MEDIA_UPSIDE_DOWN:
            IsError = TRUE ;
            break;

          default:
            break ;
        }
        break;

      default:
        break; 
    }
            
    SensePtr ++ ;
  }

  return IsError;
}


BOOLEAN
ScsiDiskIsHardwareError(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  EFI_SCSI_SENSE_DATA   *SensePtr;
  UINTN                 Index;
  BOOLEAN               IsError;

  IsError = FALSE;
  SensePtr = SenseData;

  for (Index = 0; Index < SenseCounts; Index ++) {
    
    //
    // Sense Key is EFI_SCSI_SK_HARDWARE_ERROR (0x4)
    //
    if (SensePtr->Sense_Key == EFI_SCSI_SK_HARDWARE_ERROR) {
      IsError = TRUE;
    }

    SensePtr ++ ;
  }

  return IsError;
}


BOOLEAN
ScsiDiskIsMediaChange(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  EFI_SCSI_SENSE_DATA   *SensePtr;
  UINTN                 Index;
  BOOLEAN               IsMediaChanged;

  IsMediaChanged = FALSE;
  SensePtr = SenseData;

  for (Index = 0; Index < SenseCounts; Index ++) {
    
    //
    // Sense Key is EFI_SCSI_SK_UNIT_ATTENTION (0x6),
    // Additional sense code is EFI_SCSI_ASC_MEDIA_CHANGE (0x28)
    //
    if ((SensePtr->Sense_Key == EFI_SCSI_SK_UNIT_ATTENTION)
        && (SensePtr->Addnl_Sense_Code == EFI_SCSI_ASC_MEDIA_CHANGE)){
      IsMediaChanged = TRUE;
    }
  
    SensePtr ++ ;
  }

  return IsMediaChanged;
}


BOOLEAN
ScsiDiskIsResetBefore(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  EFI_SCSI_SENSE_DATA   *SensePtr;
  UINTN                 Index;
  BOOLEAN               IsResetBefore;

  IsResetBefore = FALSE;
  SensePtr = SenseData;

  for (Index = 0; Index < SenseCounts; Index ++) {
    
    //
    // Sense Key is EFI_SCSI_SK_UNIT_ATTENTION (0x6)
    // Additional Sense Code is EFI_SCSI_ASC_RESET (0x29)
    //
    if ((SensePtr->Sense_Key == EFI_SCSI_SK_UNIT_ATTENTION)
        && (SensePtr->Addnl_Sense_Code == EFI_SCSI_ASC_RESET)) {
      IsResetBefore = TRUE;
    }
  
    SensePtr ++ ;
  }

  return IsResetBefore;
}


BOOLEAN
ScsiDiskIsDriveReady(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts,
  OUT BOOLEAN               *RetryLater
  )
{
  EFI_SCSI_SENSE_DATA   *SensePtr;
  UINTN                 Index;
  BOOLEAN               IsReady;

  IsReady     = TRUE;
  *RetryLater = FALSE;
  SensePtr    = SenseData ;

  for (Index = 0; Index < SenseCounts; Index ++) {
  
    switch (SensePtr->Sense_Key) {
      
      case EFI_SCSI_SK_NOT_READY:
        //
        // Sense Key is EFI_SCSI_SK_NOT_READY (0x2)
        //
        switch (SensePtr->Addnl_Sense_Code) {
          case EFI_SCSI_ASC_NOT_READY: 
          //
          // Additional Sense Code is EFI_SCSI_ASC_NOT_READY (0x4)
          //
            switch (SensePtr->Addnl_Sense_Code_Qualifier) {
              case EFI_SCSI_ASCQ_IN_PROGRESS:
                //
                // Additional Sense Code Qualifier is 
                // EFI_SCSI_ASCQ_IN_PROGRESS (0x1)
                //
                IsReady = FALSE ;
                *RetryLater = TRUE ;
                break;
                
              default:
                IsReady = FALSE ;
                *RetryLater = FALSE ;
                break;
            }           
            break;

          default:
            break;
        }
        break;

      default:
        break; 
    }
            
    SensePtr ++ ;
  }

  return IsReady;
}


BOOLEAN
ScsiDiskHaveSenseKey(
  IN  EFI_SCSI_SENSE_DATA   *SenseData,
  IN  UINTN                 SenseCounts
  )
{
  EFI_SCSI_SENSE_DATA   *SensePtr;
  UINTN                 Index;
  BOOLEAN               HaveSenseKey;
  
  if (SenseCounts == 0) {
    HaveSenseKey = FALSE;
  } else {
    HaveSenseKey = TRUE;
  }
  
  SensePtr = SenseData;

  for ( Index = 0; Index < SenseCounts; Index ++ ) {
    
    //
    // Sense Key is SK_NO_SENSE (0x0)
    // 
    if ((SensePtr->Sense_Key == EFI_SCSI_SK_NO_SENSE)
        && (Index == 0)) {
      HaveSenseKey = FALSE;
    }
    
    SensePtr ++ ;
  }

  return HaveSenseKey;
}

VOID
ReleaseScsiDiskDeviceResources (
  IN  SCSI_DISK_DEV   *ScsiDiskDevice
  )
{
  if (ScsiDiskDevice == NULL) {
    return;
  }
  
  if (ScsiDiskDevice->SenseData != NULL) {
    gBS->FreePool (ScsiDiskDevice->SenseData);
    ScsiDiskDevice->SenseData = NULL;
  }
  
  if (ScsiDiskDevice->ControllerNameTable != NULL) {
    EfiLibFreeUnicodeStringTable (ScsiDiskDevice->ControllerNameTable);
    ScsiDiskDevice->ControllerNameTable = NULL;
  }
  
  gBS->FreePool (ScsiDiskDevice); 
  
  ScsiDiskDevice = NULL;    
}
