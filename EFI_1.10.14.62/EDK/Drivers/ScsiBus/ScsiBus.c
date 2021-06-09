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

    scsibus.c
    
Abstract: 
    

Revision History
--*/

#include "scsibus.h"
#include "scsilib.h"

EFI_STATUS
SCSIBusDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
SCSIBusDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );  

EFI_STATUS
SCSIBusDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  );

EFI_DRIVER_BINDING_PROTOCOL gSCSIBusDriverBinding = {
  SCSIBusDriverBindingSupported,
  SCSIBusDriverBindingStart,
  SCSIBusDriverBindingStop,
  0x10,
  NULL,
  NULL
};  

EFI_DRIVER_ENTRY_POINT(ScsiBusControllerDriverEntryPoint)

EFI_STATUS
ScsiBusControllerDriverEntryPoint(
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
                                        &gSCSIBusDriverBinding, 
                                        ImageHandle,
                                        &gScsiBusComponentName,
                                        NULL,
                                        NULL
                                        );
  return Status;
} 

EFI_STATUS
SCSIBusDriverBindingSupported (
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
  EFI_SCSI_PASS_THRU_PROTOCOL *ScsiPassThru;

  //
  // check for the existence of SCSI Pass Thru Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,   
                  &gEfiScsiPassThruProtocolGuid,  
                  (VOID **)&ScsiPassThru,
                  This->DriverBindingHandle,     
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && (Status != EFI_ALREADY_STARTED)) {
    return EFI_UNSUPPORTED;
  }

  Status = EFI_UNSUPPORTED;
  if (ScsiPassThru->Mode->Attributes & EFI_SCSI_PASS_THRU_ATTRIBUTES_LOGICAL) {
    Status = EFI_SUCCESS;
  }

  gBS->CloseProtocol (
         Controller, 
         &gEfiScsiPassThruProtocolGuid, 
         This->DriverBindingHandle, 
         Controller
         );

  return Status;
}   

EFI_STATUS
SCSIBusDriverBindingStart (
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
  EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath;
  EFI_SCSI_PASS_THRU_PROTOCOL *ScsiPassThru;
  UINT32                      StartPun = 0;
  UINT64                      StartLun = 0;
  UINT32                      Pun;
  UINT64                      Lun;
  BOOLEAN                     ScanOtherPuns;
  
  Status = gBS->OpenProtocol (
                  Controller,   
                  &gEfiDevicePathProtocolGuid,  
                  (VOID **)&ParentDevicePath,
                  This->DriverBindingHandle,     
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    return Status;
  }
  
  //
  // Consume SCSI Pass Thru protocol.
  //
  Status = gBS->OpenProtocol (
                  Controller, 
                  &gEfiScsiPassThruProtocolGuid, 
                  (VOID **)&ScsiPassThru,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    gBS->CloseProtocol (
         Controller, 
         &gEfiDevicePathProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    return Status;
  }
  
  if (RemainingDevicePath == NULL) {
    StartPun = 0xFFFFFFFF;
    StartLun = 0;
  } else {
    ScsiPassThru->GetTargetLun (ScsiPassThru,RemainingDevicePath,&StartPun,&StartLun);
  }
  
  for (Pun = StartPun,ScanOtherPuns = TRUE;ScanOtherPuns == TRUE;) {
  
    if (StartPun == 0xFFFFFFFF) {
    //
    // Remaining Device Path is NULL, scan all the possible Puns in the
    // SCSI Channel.
    //
      Status = ScsiPassThru->GetNextDevice (ScsiPassThru,&Pun,&Lun);
      if (EFI_ERROR(Status)) {
        //
        // no legal Pun and Lun found any more
        //
        break;
      }
    } else {
    //
    // Remaining Device Path is not NULL, only scan the specified Pun.
    //
      Pun = StartPun;
      Lun = StartLun;
      ScanOtherPuns = FALSE;
    }
    
    //
    // Avoid creating handle for the host adapter.
    //
    if (Pun == ScsiPassThru->Mode->AdapterId) {
      continue;
    }
    
    //
    // Scan for the scsi device, if it attaches to the scsi bus,
    // then create handle and install scsi i/o protocol.
    //
    Status = ScsiScanCreateDevice (This,Controller,Pun,Lun,ScsiPassThru,ParentDevicePath);
  }
  
  return Status;
}

EFI_STATUS
SCSIBusDriverBindingStop (
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
  EFI_STATUS                    Status;
  BOOLEAN                       AllChildrenStopped;
  UINTN                         Index;
  EFI_SCSI_IO_PROTOCOL          *ScsiIo;
  SCSI_IO_DEV                   *ScsiIoDevice;
  EFI_SCSI_PASS_THRU_PROTOCOL   *ScsiPassThru;

  if (NumberOfChildren == 0) {
    //
    // Close the bus driver
    //
    gBS->CloseProtocol (
           Controller, 
           &gEfiScsiPassThruProtocolGuid, 
           This->DriverBindingHandle, 
           Controller
           );
    gBS->CloseProtocol (
           Controller, 
           &gEfiDevicePathProtocolGuid, 
           This->DriverBindingHandle,   
           Controller   
           );    
    
    return EFI_SUCCESS;
  }
  
  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {
    
    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index],
                    &gEfiScsiIoProtocolGuid,  
                    (VOID **)&ScsiIo,
                    This->DriverBindingHandle,             
                    Controller,   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
      continue;
    }
    
    ScsiIoDevice = SCSI_IO_DEV_FROM_THIS(ScsiIo);
    //
    // Close the child handle
    //
    Status = gBS->CloseProtocol (
                        Controller, 
                        &gEfiScsiPassThruProtocolGuid, 
                        This->DriverBindingHandle, 
                        ChildHandleBuffer[Index]
                       );
  
    Status = gBS->UninstallMultipleProtocolInterfaces (
                   ChildHandleBuffer[Index], 
                   &gEfiDevicePathProtocolGuid,      ScsiIoDevice->DevicePath,
                   &gEfiScsiIoProtocolGuid,          &ScsiIoDevice->ScsiIo,
                   NULL
                   );
    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
      gBS->OpenProtocol (
           Controller,   
           &gEfiScsiPassThruProtocolGuid,  
           &ScsiPassThru,
           This->DriverBindingHandle,   
           ChildHandleBuffer[Index],
           EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
           );
    } else {
      gBS->FreePool (ScsiIoDevice);
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
ScsiGetDeviceType (
  IN  EFI_SCSI_IO_PROTOCOL     *This,
  OUT UINT8                    *DeviceType
  )
/*++

  Routine Description:
    Retrieves the device type information of the SCSI Controller.
    
  Arguments:
    This                  - Protocol instance pointer.
    DeviceType            - A pointer to the device type information
                            retrieved from the SCSI Controller. 

  Returns:
    EFI_SUCCESS           - Retrieves the device type information successfully.
    EFI_INVALID_PARAMETER - The DeviceType is NULL.
--*/
{
  SCSI_IO_DEV       *ScsiIoDevice;
  
  if (DeviceType == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (This);
  *DeviceType = ScsiIoDevice->ScsiDeviceType;
  return EFI_SUCCESS;
}

EFI_STATUS
ScsiGetDeviceLocation (
  IN  EFI_SCSI_IO_PROTOCOL    *This,
  OUT UINT32                  *Target,
  OUT UINT64                  *Lun
  )
/*++
  Routine Description:
    Retrieves the device location in the SCSI channel.
    
  Arguments:
    This                  - Protocol instance pointer.
    Target                - A pointer to the Target ID of a SCSI device 
                            on the SCSI channel. 
    Lun                   - A pointer to the LUN of the SCSI device on 
                            the SCSI channel.

  Returns:
    EFI_SUCCESS           - Retrieves the device location successfully.
    EFI_INVALID_PARAMETER - The Target or Lun is NULL.
--*/
{
  SCSI_IO_DEV       *ScsiIoDevice;
  
  if (Target == NULL || Lun == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (This);
  
  *Target = ScsiIoDevice->Pun;
  *Lun = ScsiIoDevice->Lun;
  
  return EFI_SUCCESS;
}

EFI_STATUS
ScsiResetBus (
  IN  EFI_SCSI_IO_PROTOCOL     *This
  )
/*++

  Routine Description:
    Resets the SCSI Bus that the SCSI Controller is attached to.
    
  Arguments:
    This                  - Protocol instance pointer.

  Returns:
    EFI_SUCCESS           - The SCSI bus is reset successfully.
    EFI_DEVICE_ERROR      - Errors encountered when resetting the SCSI bus.
    EFI_UNSUPPORTED       - The bus reset operation is not supported by the
                            SCSI Host Controller.
    EFI_TIMEOUT           - A timeout occurred while attempting to reset 
                            the SCSI bus.
--*/
{
  SCSI_IO_DEV     *ScsiIoDevice;
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (This);
  
  return ScsiIoDevice->ScsiPassThru->ResetChannel (ScsiIoDevice->ScsiPassThru);

}

EFI_STATUS
ScsiResetDevice (
  IN  EFI_SCSI_IO_PROTOCOL     *This
  )
/*++

  Routine Description:
    Resets the SCSI Controller that the device handle specifies.
    
  Arguments:
    This                  - Protocol instance pointer.
    

  Returns:
    EFI_SUCCESS           - Reset the SCSI controller successfully.
    EFI_DEVICE_ERROR      - Errors are encountered when resetting the
                            SCSI Controller.
    EFI_UNSUPPORTED       - The SCSI bus does not support a device 
                            reset operation.
    EFI_TIMEOUT           - A timeout occurred while attempting to 
                            reset the SCSI Controller.
--*/
{
  SCSI_IO_DEV     *ScsiIoDevice;
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (This);
  
  return ScsiIoDevice->ScsiPassThru->ResetTarget (ScsiIoDevice->ScsiPassThru,
                                                  ScsiIoDevice->Pun,
                                                  ScsiIoDevice->Lun
                                                  );
}

EFI_STATUS
ScsiExecuteSCSICommand (
  IN  EFI_SCSI_IO_PROTOCOL                  *This,
  IN OUT  EFI_SCSI_IO_SCSI_REQUEST_PACKET   *Packet,
  IN EFI_EVENT                              Event  OPTIONAL
  )
/*++

  Routine Description:
    Sends a SCSI Request Packet to the SCSI Controller for execution.
    
  Arguments:
    This                  - Protocol instance pointer.
    Packet                - The SCSI request packet to send to the SCSI 
                            Controller specified by the device handle.
    Event                 - If the SCSI bus where the SCSI device is attached
                            does not support non-blocking I/O, then Event is 
                            ignored, and blocking I/O is performed.  
                            If Event is NULL, then blocking I/O is performed.
                            If Event is not NULL and non-blocking I/O is 
                            supported, then non-blocking I/O is performed,
                            and Event will be signaled when the SCSI Request
                            Packet completes.
  Returns:
    EFI_SUCCESS           - The SCSI Request Packet was sent by the host 
                            successfully, and TransferLength bytes were 
                            transferred to/from DataBuffer.See 
                            HostAdapterStatus, TargetStatus, 
                            SenseDataLength, and SenseData in that order
                            for additional status information.
    EFI_WARN_BUFFER_TOO_SMALL - The SCSI Request Packet was executed, 
                            but the entire DataBuffer could not be transferred.
                            The actual number of bytes transferred is returned
                            in TransferLength. See HostAdapterStatus, 
                            TargetStatus, SenseDataLength, and SenseData in 
                            that order for additional status information.
    EFI_NOT_READY         - The SCSI Request Packet could not be sent because 
                            there are too many SCSI Command Packets already 
                            queued.The caller may retry again later.
    EFI_DEVICE_ERROR      - A device error occurred while attempting to send 
                            the SCSI Request Packet. See HostAdapterStatus, 
                            TargetStatus, SenseDataLength, and SenseData in 
                            that order for additional status information.
    EFI_INVALID_PARAMETER - The contents of CommandPacket are invalid.  
                            The SCSI Request Packet was not sent, so no 
                            additional status information is available.
    EFI_UNSUPPORTED       - The command described by the SCSI Request Packet
                            is not supported by the SCSI initiator(i.e., SCSI 
                            Host Controller). The SCSI Request Packet was not
                            sent, so no additional status information is 
                            available.
    EFI_TIMEOUT           - A timeout occurred while waiting for the SCSI 
                            Request Packet to execute. See HostAdapterStatus,
                            TargetStatus, SenseDataLength, and SenseData in 
                            that order for additional status information.
--*/
{
  SCSI_IO_DEV       *ScsiIoDevice;
  EFI_STATUS        Status;
  
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET    *RequestPacket;
  
  if (Packet == NULL) {
    return EFI_INVALID_PARAMETER;
  }  
  
  ScsiIoDevice = SCSI_IO_DEV_FROM_THIS (This);
  
  RequestPacket = (EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET*)Packet;
  
  Status = ScsiIoDevice->ScsiPassThru->PassThru (
                                        ScsiIoDevice->ScsiPassThru,
                                        ScsiIoDevice->Pun,
                                        ScsiIoDevice->Lun,
                                        RequestPacket,
                                        Event
                                        );
  return Status;
}

EFI_STATUS
ScsiScanCreateDevice (
  EFI_DRIVER_BINDING_PROTOCOL   *This,
  EFI_HANDLE                    Controller,  
  UINT32                        Pun,
  UINT64                        Lun,
  EFI_SCSI_PASS_THRU_PROTOCOL   *ScsiPassThru,
  EFI_DEVICE_PATH_PROTOCOL      *ParentDevicePath
  )
{
  EFI_STATUS                  Status;
  SCSI_IO_DEV                 *ScsiIoDevice;
  EFI_DEVICE_PATH_PROTOCOL    *ScsiDevicePath;
  
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(SCSI_IO_DEV),
                  (VOID **)&ScsiIoDevice);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  EfiZeroMem (ScsiIoDevice,sizeof(SCSI_IO_DEV));
  
  ScsiIoDevice->Signature = SCSI_IO_DEV_SIGNATURE;
  ScsiIoDevice->ScsiPassThru = ScsiPassThru;
  ScsiIoDevice->Pun = Pun;
  ScsiIoDevice->Lun = Lun;
  
  ScsiIoDevice->ScsiIo.GetDeviceType      = ScsiGetDeviceType;
  ScsiIoDevice->ScsiIo.GetDeviceLocation  = ScsiGetDeviceLocation;
  ScsiIoDevice->ScsiIo.ResetBus           = ScsiResetBus;
  ScsiIoDevice->ScsiIo.ResetDevice        = ScsiResetDevice;
  ScsiIoDevice->ScsiIo.ExecuteSCSICommand = ScsiExecuteSCSICommand;
  
  if (!DiscoverScsiDevice (ScsiIoDevice)) {
    gBS->FreePool (ScsiIoDevice);
    return EFI_SUCCESS;
  }
  
  //
  // Set Device Path
  //
  Status = ScsiIoDevice->ScsiPassThru->BuildDevicePath (
                                                ScsiIoDevice->ScsiPassThru,
                                                ScsiIoDevice->Pun,
                                                ScsiIoDevice->Lun,
                                                &ScsiDevicePath
                                                );
  if (Status == EFI_OUT_OF_RESOURCES) {
    gBS->FreePool (ScsiIoDevice);
    return Status;
  }
  ScsiIoDevice->DevicePath = EfiAppendDevicePathNode (
                                                ParentDevicePath, 
                                                ScsiDevicePath);  
  //
  // The memory space for ScsiDevicePath is allocated in 
  // ScsiPassThru->BuildDevicePath() function; It is no longer used
  // after EfiAppendDevicePathNode,so free the memory it occupies.
  //
  gBS->FreePool (ScsiDevicePath);
  
  if (ScsiIoDevice->DevicePath == NULL) {
    gBS->FreePool (ScsiIoDevice);
    return EFI_OUT_OF_RESOURCES;
  }
  
  Status = gBS->InstallMultipleProtocolInterfaces (
        &ScsiIoDevice->Handle,           
        &gEfiDevicePathProtocolGuid,       ScsiIoDevice->DevicePath,
        &gEfiScsiIoProtocolGuid,           &ScsiIoDevice->ScsiIo,
        NULL
        );
  if (EFI_ERROR(Status)) {
    gBS->FreePool (ScsiIoDevice);
  } else {
    gBS->OpenProtocol (
                Controller,   
                &gEfiScsiPassThruProtocolGuid,  
                (VOID **)&ScsiPassThru,
                This->DriverBindingHandle,   
                ScsiIoDevice->Handle,
                EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                );
  }
  return EFI_SUCCESS;
}

BOOLEAN
DiscoverScsiDevice (
  SCSI_IO_DEV   *ScsiIoDevice
  )
{
  EFI_STATUS              Status;
  EFI_SCSI_INQUIRY_DATA   InquiryData;
  UINT32                  InquiryDataLength;
  EFI_SCSI_SENSE_DATA     SenseData;
  UINT8                   SenseDataLength;
  UINT8                   HostAdapterStatus = 0;
  UINT8                   TargetStatus = 0;
  
  //
  // Using Inquiry command to scan for the device
  //
  InquiryDataLength = sizeof (EFI_SCSI_INQUIRY_DATA);
  SenseDataLength = sizeof (EFI_SCSI_SENSE_DATA);
  
  Status = SubmitInquiryCommand (&ScsiIoDevice->ScsiIo,
                                  EfiScsiStallSeconds(1),
                                  (VOID*)&SenseData,
                                  &SenseDataLength,
                                  &HostAdapterStatus,
                                  &TargetStatus,
                                  (VOID*)&InquiryData,
                                  &InquiryDataLength,
                                  FALSE
                                  );
  if (EFI_ERROR(Status)) {
//    ParseSenseData (&SenseData,SenseDataLength);
    return FALSE;
  }
  
  //
  // Retrieved inquiry data successfully
  //
  if ((InquiryData.Peripheral_Qualifier != 0) &&
      (InquiryData.Peripheral_Qualifier != 3)) {
    return FALSE;
  }
  
  if (InquiryData.Peripheral_Qualifier == 3) {
    if (InquiryData.Peripheral_Type != 0x1f) {
      return FALSE;
    }
  }
  
  if (0x1e >= InquiryData.Peripheral_Type >= 0xa) {
    return FALSE;
  }
  
  //
  // valid device type and peripheral qualifier combination.
  //
  ScsiIoDevice->ScsiDeviceType = InquiryData.Peripheral_Type;
  ScsiIoDevice->RemovableDevice = InquiryData.RMB;
  if (InquiryData.Version == 0) {
    ScsiIoDevice->ScsiVersion = 0;
  } else {
    //
    // ANSI-approved version
    //
    ScsiIoDevice->ScsiVersion = (UINT8)(InquiryData.Version & 0x03);
  }
  
  return TRUE;
}



