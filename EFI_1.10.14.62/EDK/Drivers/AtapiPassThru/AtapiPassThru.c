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

    AtapiPassThru.c
    
Abstract: 
    

Revision History
--*/

#include "AtapiPassThru.h"

EFI_STATUS
AtapiScsiPassThruDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
AtapiScsiPassThruDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
AtapiScsiPassThruDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  );
  
//
// IDE registers' fixed address
//
static IDE_BASE_REGISTERS   AtapiIoPortRegisters[2] = { 
        {0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5, 0x1f6, 0x1f7, 0x3f6, 0x3f7},
        {0x170, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x177, 0x376, 0x377},
};

static SCSI_COMMAND_SET EndTable = {0xff,0xff};

//
// This table contains all the supported ATAPI commands.
//
static SCSI_COMMAND_SET  SupportedATAPICommands[] = {
    OP_INQUIRY,                     DataIn,
    OP_LOAD_UNLOAD_CD,              NoData,
    OP_MECHANISM_STATUS,            DataIn,
    OP_MODE_SELECT_10,              DataOut,
    OP_MODE_SENSE_10,               DataIn,
    OP_PAUSE_RESUME,                NoData,
    OP_PLAY_AUDIO_10,               DataIn,
    OP_PLAY_AUDIO_MSF,              DataIn,
    OP_PLAY_CD,                     DataIn,
    OP_PLAY_CD_MSF,                 DataIn,
    OP_PREVENT_ALLOW_MEDIUM_REMOVAL,NoData,
    OP_READ_10,                     DataIn,
    OP_READ_12,                     DataIn,
    OP_READ_CAPACITY,               DataIn,
    OP_READ_CD,                     DataIn,
    OP_READ_CD_MSF,                 DataIn,
    OP_READ_HEADER,                 DataIn,
    OP_READ_SUB_CHANNEL,            DataIn,
    OP_READ_TOC,                    DataIn,
    OP_REQUEST_SENSE,               DataIn,
    OP_SCAN,                        NoData,
    OP_SEEK_10,                     NoData,
    OP_SET_CD_SPEED,                DataOut,
    OP_STOPPLAY_SCAN,               NoData,
    OP_START_STOP_UNIT,             NoData,
    OP_TEST_UNIT_READY,             NoData,
    OP_FORMAT_UNIT,                 DataOut,
    OP_READ_FORMAT_CAPACITIES,      DataIn,
    OP_VERIFY,                      DataOut,
    OP_WRITE_10,                    DataOut,
    OP_WRITE_12,                    DataOut,
    OP_WRITE_AND_VERIFY,            DataOut,
    0xff,                           0xff
  };
  
static CHAR16 ControllerNameString[] = L"ATAPI Controller";
static CHAR16 AtapiChannelString[] = L"ATAPI Channel";
    
EFI_DRIVER_BINDING_PROTOCOL gAtapiScsiPassThruDriverBinding = {
  AtapiScsiPassThruDriverBindingSupported,
  AtapiScsiPassThruDriverBindingStart,
  AtapiScsiPassThruDriverBindingStop,
  0x10,
  NULL,
  NULL
};  
    
EFI_DRIVER_ENTRY_POINT(AtapiScsiPassThruDriverEntryPoint)
    
EFI_STATUS
AtapiScsiPassThruDriverEntryPoint(
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  ) 
/*++
  
  Routine Description:
    Driver Entry Point.
        
  Arguments:
    (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)
  
  Returns:
    EFI_STATUS
--*/                
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gAtapiScsiPassThruDriverBinding,
           ImageHandle,
           &gAtapiScsiPassThruComponentName,
           NULL,
           NULL
           );
} 

EFI_STATUS
AtapiScsiPassThruDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Supported.
    
  Arguments:
    (Standard DriverBinding Protocol Supported() function)
    
  Returns:
    EFI_STATUS
  
--*/                
{
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  PCI_TYPE00                Pci;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,           
                  &gEfiPciIoProtocolGuid, 
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
      
  
  //
  // Use the PCI I/O Protocol to see if Controller is a IDE Controller that
  // can be managed by this driver.  Read the PCI Configuration Header 
  // for this device.
  //
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, 
                            sizeof(Pci) / sizeof (UINT32), &Pci);
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
         Controller,  
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    return EFI_UNSUPPORTED;
  }
  
  if(Pci.Hdr.ClassCode[2] != PCI_CLASS_MASS_STORAGE || 
      Pci.Hdr.ClassCode[1] != PCI_CLASS_IDE) {
    
    Status = EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
         Controller,  
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
         
  return Status;    
}   

EFI_STATUS
AtapiScsiPassThruDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Create handles for IDE channels specified by RemainingDevicePath.
    Install SCSI Pass Thru Protocol onto each created handle.
  
  Arguments:
    (Standard DriverBinding Protocol Start() function)
    
  Returns:
    EFI_STATUS
    
--*/                
{
  EFI_STATUS                  Status;
  EFI_PCI_IO_PROTOCOL         *PciIo;

  PciIo = NULL;
  Status = gBS->OpenProtocol (
                  Controller, 
                  &gEfiPciIoProtocolGuid, 
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationEnable,
                    EFI_PCI_IO_ATTRIBUTE_IDE_PRIMARY_IO | EFI_PCI_IO_ATTRIBUTE_IDE_SECONDARY_IO | EFI_PCI_DEVICE_ENABLE, 
                    NULL
                    );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Create SCSI Pass Thru instance for the IDE channel.
  //
  Status = RegisterAtapiScsiPassThru (This, Controller, PciIo);

Done:
  if (EFI_ERROR (Status)) {
    if (PciIo) {
      PciIo->Attributes (
               PciIo, 
               EfiPciIoAttributeOperationDisable, 
               EFI_PCI_IO_ATTRIBUTE_IDE_PRIMARY_IO | EFI_PCI_IO_ATTRIBUTE_IDE_SECONDARY_IO | EFI_PCI_DEVICE_ENABLE, 
               NULL
               );
    }
    gBS->CloseProtocol (
           Controller, 
           &gEfiPciIoProtocolGuid, 
           This->DriverBindingHandle,   
           Controller   
           );
  }

  return Status;
}

EFI_STATUS
AtapiScsiPassThruDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++
  
  Routine Description:
    Stop.
  
  Arguments:
    (Standard DriverBinding Protocol Stop() function)
  
  Returns:
    EFI_STATUS
  
--*/                
{
  EFI_STATUS                    Status;
  EFI_SCSI_PASS_THRU_PROTOCOL   *ScsiPassThru;
  ATAPI_SCSI_PASS_THRU_DEV      *AtapiScsiPrivate;

  Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiScsiPassThruProtocolGuid,  
                    (VOID **)&ScsiPassThru,
                    This->DriverBindingHandle,             
                    Controller,   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (ScsiPassThru);

  Status = gBS->UninstallProtocolInterface (
                  Controller, 
                  &gEfiScsiPassThruProtocolGuid,&AtapiScsiPrivate->ScsiPassThru
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
    
  //
  // Release Pci Io protocol on the controller handle.
  //
  AtapiScsiPrivate->PciIo->Attributes (
                             AtapiScsiPrivate->PciIo, 
                             EfiPciIoAttributeOperationDisable, 
                             EFI_PCI_IO_ATTRIBUTE_IDE_PRIMARY_IO | EFI_PCI_IO_ATTRIBUTE_IDE_SECONDARY_IO | EFI_PCI_DEVICE_ENABLE, 
                             NULL
                             );

  gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle, 
         Controller
         );
    
  gBS->FreePool (AtapiScsiPrivate);

  return EFI_SUCCESS;
}


EFI_STATUS
RegisterAtapiScsiPassThru (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_PCI_IO_PROTOCOL         *PciIo
  )
/*++
  
  Routine Description:
    Attaches SCSI Pass Thru Protocol for specified IDE channel.
    
  Arguments:
    Controller:       Parent device handle to the IDE channel.    
    PciIo:            PCI I/O protocol attached on the "Controller".                        
  
  Returns:
    Always return EFI_SUCCESS unless installing SCSI Pass Thru Protocol failed.
--*/    
{
  EFI_STATUS                  Status;
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate;
  UINT64                      Attributes;
  
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(ATAPI_SCSI_PASS_THRU_DEV),
                  (VOID **)&AtapiScsiPrivate
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EfiZeroMem (AtapiScsiPrivate,sizeof (ATAPI_SCSI_PASS_THRU_DEV)) ;
  
  Attributes = EFI_PCI_IO_ATTRIBUTE_IDE_PRIMARY_IO | EFI_PCI_IO_ATTRIBUTE_IDE_SECONDARY_IO | EFI_PCI_DEVICE_ENABLE;
  EfiCopyMem (AtapiScsiPrivate->ChannelName,AtapiChannelString,sizeof(AtapiChannelString));

  //
  // Enable channel
  //
  PciIo->Attributes ( PciIo, EfiPciIoAttributeOperationSet, Attributes, NULL ) ;
  
  AtapiScsiPrivate->Signature = ATAPI_SCSI_PASS_THRU_DEV_SIGNATURE;
  AtapiScsiPrivate->Handle    = Controller ;
  
  //
  // will reset the IoPort inside each API function.
  //
  AtapiScsiPrivate->IoPort      = AtapiIoPortRegisters;
  AtapiScsiPrivate->PciIo       = PciIo;

  //
  // initialize SCSI Pass Thru Protocol interface
  //
  AtapiScsiPrivate->ScsiPassThru.Mode             = &AtapiScsiPrivate->ScsiPassThruMode;
  AtapiScsiPrivate->ScsiPassThru.PassThru         = AtapiScsiPassThruFunction; 
  AtapiScsiPrivate->ScsiPassThru.GetNextDevice    = AtapiScsiPassThruGetNextDevice;
  AtapiScsiPrivate->ScsiPassThru.BuildDevicePath  = AtapiScsiPassThruBuildDevicePath;
  AtapiScsiPrivate->ScsiPassThru.GetTargetLun     = AtapiScsiPassThruGetTargetLun;
  AtapiScsiPrivate->ScsiPassThru.ResetChannel     = AtapiScsiPassThruResetChannel;
  AtapiScsiPrivate->ScsiPassThru.ResetTarget      = AtapiScsiPassThruResetTarget;
  
  //
  // Set Mode
  //
  EfiCopyMem (AtapiScsiPrivate->ControllerName,ControllerNameString,sizeof(ControllerNameString));
  
  AtapiScsiPrivate->ScsiPassThruMode.ControllerName = AtapiScsiPrivate->ControllerName;
  AtapiScsiPrivate->ScsiPassThruMode.ChannelName = AtapiScsiPrivate->ChannelName;
  AtapiScsiPrivate->ScsiPassThruMode.AdapterId = 4;
  //
  // non-RAID SCSI controllers should set both physical and logical attributes
  //
  AtapiScsiPrivate->ScsiPassThruMode.Attributes = EFI_SCSI_PASS_THRU_ATTRIBUTES_PHYSICAL 
                                                  | EFI_SCSI_PASS_THRU_ATTRIBUTES_LOGICAL;
  AtapiScsiPrivate->ScsiPassThruMode.IoAlign = 0;
  
  //
  // Initialize the LatestTargetId to 0xFFFFFFFF (for the GetNextDevice() call).
  //
  AtapiScsiPrivate->LatestTargetId = 0xFFFFFFFF;
  AtapiScsiPrivate->LatestLun      = 0;
  
  Status = gBS->InstallProtocolInterface (
        &Controller,
        &gEfiScsiPassThruProtocolGuid,
        EFI_NATIVE_INTERFACE,
        &AtapiScsiPrivate->ScsiPassThru
        );
  return Status;
}

EFI_STATUS
AtapiScsiPassThruFunction (
  IN EFI_SCSI_PASS_THRU_PROTOCOL                  *This,
  IN UINT32                                       Target,
  IN UINT64                                       Lun,
  IN OUT EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET   *Packet,
  IN EFI_EVENT                                    Event OPTIONAL
  )
/*++

  Routine Description:
    Implements EFI_SCSI_PASS_THRU_PROTOCOL.PassThru() function.
  
  Arguments:
    This:     The EFI_SCSI_PASS_THRU_PROTOCOL instance.
    Target:   The Target ID of the ATAPI device to send the SCSI 
              Request Packet. To ATAPI devices attached on an IDE
              Channel, Target ID 0 indicates Master device;Target
              ID 1 indicates Slave device.
    Lun:      The LUN of the ATAPI device to send the SCSI Request
              Packet. To the ATAPI device, Lun is always 0.
    Packet:   The SCSI Request Packet to send to the ATAPI device 
              specified by Target and Lun.
    Event:    If non-blocking I/O is not supported then Event is ignored, 
              and blocking I/O is performed.
              If Event is NULL, then blocking I/O is performed.
              If Event is not NULL and non blocking I/O is supported, 
              then non-blocking I/O is performed, and Event will be signaled 
              when the SCSI Request Packet completes.      
    
  Returns:  

--*/  
{
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate;
  EFI_STATUS                      Status;
  
  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (This);
  
  //
  // Target is not allowed beyond MAX_TARGET_ID
  //
  if (Target > MAX_TARGET_ID) {
    return EFI_INVALID_PARAMETER;
  }  
  
  //
  // check the data fields in Packet parameter.
  //
  Status = CheckSCSIRequestPacket (Packet);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // If Request Packet targets at the IDE channel itself,
  // do nothing.
  //
  if (Target == This->Mode->AdapterId) {
    Packet->TransferLength = 0;
    return EFI_SUCCESS;
  }
  
  //
  // According to Target ID, reset the Atapi I/O Register mapping
  // (Target Id in [0,1] area, using AtapiIoPortRegisters[0],
  //  Target Id in [2,3] area, using AtapiIoPortRegisters[1]
  //
  if ((Target / 2) == 0 ) {
    AtapiScsiPrivate->IoPort = &AtapiIoPortRegisters[0];
  } else {
    AtapiScsiPrivate->IoPort = &AtapiIoPortRegisters[1];
  }
  
  //
  // the ATAPI SCSI interface does not support non-blocking I/O
  // ignore the Event parameter
  //
  // Performs blocking I/O.
  //
  Status = SubmitBlockingIoCommand (AtapiScsiPrivate,Target,Packet);  
  return Status;
}    

EFI_STATUS
AtapiScsiPassThruGetNextDevice (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN OUT UINT32                      *Target,
  IN OUT UINT64                      *Lun
  )
/*++

  Routine Description:
    Used to retrieve the list of legal Target IDs for SCSI devices 
    on a SCSI channel.

  Arguments:
    This                  - Protocol instance pointer.
    Target                - On input, a pointer to the Target ID of a SCSI 
                            device present on the SCSI channel.  On output, 
                            a pointer to the Target ID of the next SCSI device
                             present on a SCSI channel.  An input value of 
                             0xFFFFFFFF retrieves the Target ID of the first 
                             SCSI device present on a SCSI channel.
    Lun                   - On input, a pointer to the LUN of a SCSI device
                            present on the SCSI channel. On output, a pointer
                            to the LUN of the next SCSI device present on 
                            a SCSI channel.
    
  Returns:
    EFI_SUCCESS           - The Target ID and Lun of the next SCSI device 
                            on the SCSI channel was returned in Target and Lun.
    EFI_NOT_FOUND         - There are no more SCSI devices on this SCSI channel.
    EFI_INVALID_PARAMETER - Target is not 0xFFFFFFFF,and Target and Lun were not
                            returned on a previous call to GetNextDevice().
--*/
{
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate;
  
  //
  // Retrieve Device Private Data Structure.
  //
  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (This);
  
  //
  // Check whether Target is valid.
  //
  
  if (Target == NULL || Lun == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  if ((*Target != 0xFFFFFFFF) &&
      ((*Target != AtapiScsiPrivate->LatestTargetId) ||
      (*Lun != AtapiScsiPrivate->LatestLun))) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (*Target == MAX_TARGET_ID) {
    return EFI_NOT_FOUND;
  }
  
  if (*Target == 0xFFFFFFFF) {
    *Target = 0;
  } else {
    *Target = AtapiScsiPrivate->LatestTargetId + 1;
  }
  *Lun = 0;
  
  //
  // Update the LatestTargetId.
  //
  AtapiScsiPrivate->LatestTargetId = *Target;
  AtapiScsiPrivate->LatestLun = *Lun;
  
  return EFI_SUCCESS;
  
}

EFI_STATUS
AtapiScsiPassThruBuildDevicePath (
  IN     EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN     UINT32                         Target,
  IN     UINT64                         Lun,
  IN OUT EFI_DEVICE_PATH_PROTOCOL       **DevicePath
  )
/*++

  Routine Description:
    Used to allocate and build a device path node for a SCSI device 
    on a SCSI channel. Would not build device path for a SCSI Host Controller.

  Arguments:
    This                  - Protocol instance pointer.
    Target                - The Target ID of the SCSI device for which
                            a device path node is to be allocated and built.
    Lun                   - The LUN of the SCSI device for which a device 
                            path node is to be allocated and built.
    DevicePath            - A pointer to a single device path node that 
                            describes the SCSI device specified by 
                            Target and Lun. This function is responsible 
                            for allocating the buffer DevicePath with the boot
                            service AllocatePool().  It is the caller's 
                            responsibility to free DevicePath when the caller
                            is finished with DevicePath.    
  Returns:
    EFI_SUCCESS           - The device path node that describes the SCSI device
                            specified by Target and Lun was allocated and 
                            returned in DevicePath.
    EFI_NOT_FOUND         - The SCSI devices specified by Target and Lun does
                            not exist on the SCSI channel.
    EFI_INVALID_PARAMETER - DevicePath is NULL.
    EFI_OUT_OF_RESOURCES  - There are not enough resources to allocate 
                            DevicePath.
--*/
{
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate;
  EFI_DEV_PATH                *Node;
  EFI_STATUS                  Status;
  
  //
  // Retrieve Device Private Data Structure.
  //
  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (This);

  //
  // Validate parameters passed in.
  //
  
  if (DevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // can not build device path for the SCSI Host Controller.
  //
  if ((Target > (MAX_TARGET_ID - 1)) || (Lun != 0)) {
    return EFI_NOT_FOUND;
  }
  
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(EFI_DEV_PATH),
                  (VOID **)&Node);
  if (EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  EfiZeroMem (Node, sizeof(EFI_DEV_PATH));
  
  Node->DevPath.Type     = MESSAGING_DEVICE_PATH;
  Node->DevPath.SubType  = MSG_ATAPI_DP;
  SetDevicePathNodeLength (&Node->DevPath, sizeof(ATAPI_DEVICE_PATH));
  
  Node->Atapi.PrimarySecondary = (UINT8)(Target / 2);
  Node->Atapi.SlaveMaster      = (UINT8)(Target % 2);
  Node->Atapi.Lun              = (UINT16)Lun;
    
  *DevicePath = (EFI_DEVICE_PATH_PROTOCOL*)Node;
  
  return EFI_SUCCESS;
}    

EFI_STATUS
AtapiScsiPassThruGetTargetLun (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN  EFI_DEVICE_PATH_PROTOCOL       *DevicePath,
  OUT UINT32                         *Target,
  OUT UINT64                         *Lun
  )
/*++

  Routine Description:
    Used to translate a device path node to a Target ID and LUN.

  Arguments:
    This                  - Protocol instance pointer.
    DevicePath            - A pointer to the device path node that 
                            describes a SCSI device on the SCSI channel.
    Target                - A pointer to the Target ID of a SCSI device 
                            on the SCSI channel. 
    Lun                   - A pointer to the LUN of a SCSI device on 
                            the SCSI channel.    
  Returns:
    EFI_SUCCESS           - DevicePath was successfully translated to a 
                            Target ID and LUN, and they were returned 
                            in Target and Lun.
    EFI_INVALID_PARAMETER - DevicePath is NULL.
    EFI_INVALID_PARAMETER - Target is NULL.
    EFI_INVALID_PARAMETER - Lun is NULL.
    EFI_UNSUPPORTED       - This driver does not support the device path 
                            node type in DevicePath.
    EFI_NOT_FOUND         - A valid translation from DevicePath to a 
                            Target ID and LUN does not exist.
--*/
{
  EFI_DEV_PATH    *Node;
  
  //
  // Validate parameters passed in.
  //
  if (DevicePath == NULL || Target == NULL || Lun == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Check whether the DevicePath belongs to SCSI_DEVICE_PATH
  //
  if ((DevicePath->Type != MESSAGING_DEVICE_PATH) ||
      (DevicePath->SubType != MSG_ATAPI_DP) ||
      (DevicePathNodeLength(DevicePath) != sizeof(ATAPI_DEVICE_PATH))) {
    return EFI_UNSUPPORTED;
  }

  Node = (EFI_DEV_PATH*)DevicePath;
  
  *Target = Node->Atapi.PrimarySecondary * 2 + Node->Atapi.SlaveMaster;
  *Lun = Node->Atapi.Lun;
  
  if (*Target > (MAX_TARGET_ID - 1) || *Lun != 0) {
    return EFI_NOT_FOUND;
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
AtapiScsiPassThruResetChannel (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL   *This
  )
/*++

  Routine Description:
    Resets a SCSI channel.This operation resets all the 
    SCSI devices connected to the SCSI channel.

  Arguments:
    This                  - Protocol instance pointer.
    
  Returns:
    EFI_SUCCESS           - The SCSI channel was reset.
    EFI_UNSUPPORTED       - The SCSI channel does not support 
                            a channel reset operation.
    EFI_DEVICE_ERROR      - A device error occurred while 
                            attempting to reset the SCSI channel.
    EFI_TIMEOUT           - A timeout occurred while attempting 
                            to reset the SCSI channel.
--*/
{
  UINT8                     DeviceControlValue;
  ATAPI_SCSI_PASS_THRU_DEV  *AtapiScsiPrivate;
  UINT8                     Index;
  
  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (This);
  
  //
  // Reset both Primary channel and Secondary channel.
  // so, the IoPort pointer must point to the right I/O Register group
  //
  for (Index = 0; Index < 2; Index ++ ) {
    
    //
    // Reset 
    //
    AtapiScsiPrivate->IoPort = &AtapiIoPortRegisters[Index];
    
    DeviceControlValue = 0;
    DeviceControlValue |= SRST;      // set SRST bit to initiate soft reset
    DeviceControlValue |= bit(1);      // disable Interrupt
    
    WritePortB (AtapiScsiPrivate->PciIo,
                AtapiScsiPrivate->IoPort->Alt.DeviceControl, 
                DeviceControlValue);
   
    gBS->Stall(10);     // Wait 10us 
   
    //
    // Clear SRST bit
    //
    DeviceControlValue &= 0xfb ;     // 0xfb:1111,1011
    WritePortB (AtapiScsiPrivate->PciIo,AtapiScsiPrivate->IoPort->Alt.DeviceControl, DeviceControlValue);
  
    //
    // slave device needs at most 31s to clear BSY 
    //
    if ( StatusWaitForBSYClear (AtapiScsiPrivate, 31000) == EFI_TIMEOUT) {    
      return EFI_DEVICE_ERROR;
    }
  }
  
  return EFI_SUCCESS;
}    

EFI_STATUS
AtapiScsiPassThruResetTarget (
  IN EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN UINT32                         Target,
  IN UINT64                         Lun
  )
/*++

  Routine Description:
    Resets a SCSI device that is connected to a SCSI channel.

  Arguments:
    This                  - Protocol instance pointer.
    Target                - The Target ID of the SCSI device to reset. 
    Lun                   - The LUN of the SCSI device to reset.
        
  Returns:
    EFI_SUCCESS           - The SCSI device specified by Target and 
                            Lun was reset.
    EFI_UNSUPPORTED       - The SCSI channel does not support a target
                            reset operation.
    EFI_INVALID_PARAMETER - Target or Lun are invalid.
    EFI_DEVICE_ERROR      - A device error occurred while attempting 
                            to reset the SCSI device specified by Target 
                            and Lun.
    EFI_TIMEOUT           - A timeout occurred while attempting to reset 
                            the SCSI device specified by Target and Lun.
--*/
{
  ATAPI_SCSI_PASS_THRU_DEV  *AtapiScsiPrivate;  
  UINT8                     Command;
  UINT8                     DeviceSelect;
  
  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (This);
  
  if (Target > MAX_TARGET_ID) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Directly return EFI_SUCCESS if want to reset the host controller
  //
  if (Target == This->Mode->AdapterId) {
    return EFI_SUCCESS;
  }
  
  //
  // According to Target ID, reset the Atapi I/O Register mapping
  // (Target Id in [0,1] area, using AtapiIoPortRegisters[0],
  //  Target Id in [2,3] area, using AtapiIoPortRegisters[1]
  //
  if ((Target / 2) == 0 ) {
    AtapiScsiPrivate->IoPort = &AtapiIoPortRegisters[0];
  } else {
    AtapiScsiPrivate->IoPort = &AtapiIoPortRegisters[1];
  }
  
  //
  // for ATAPI device, no need to wait DRDY ready after device selecting.
  //
  // bit7 and bit5 are both set to 1 for backward compatibility
  //
  DeviceSelect = (UINT8)(((bit(7) | bit(5)) | (Target << 4))); 
  WritePortB (AtapiScsiPrivate->PciIo,AtapiScsiPrivate->IoPort->Head, DeviceSelect);
  
  Command = ATAPI_SOFT_RESET_CMD;
  WritePortB (AtapiScsiPrivate->PciIo,AtapiScsiPrivate->IoPort->Reg.Command, Command);
 
  //
  // BSY clear is the only status return to the host by the device 
  // when reset is complete.
  // slave device needs at most 31s to clear BSY 
  //
  if (EFI_ERROR(StatusWaitForBSYClear (AtapiScsiPrivate, 31000))) {
    return EFI_DEVICE_ERROR;
  }
  
  //
  // stall 5 seconds to make the device status stable
  //
  gBS->Stall(5000000);
  
  return EFI_SUCCESS;
}

    
EFI_STATUS
CheckSCSIRequestPacket (
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET      *Packet
  )
/*++
  
  Checks the parameters in the SCSI Request Packet to make sure
  they are valid for a SCSI Pass Thru request.

--*/  
{
  if (Packet == NULL) {
    return EFI_INVALID_PARAMETER;
  }  

  if (!ValidCdbLength (Packet->CdbLength)) {
    return EFI_INVALID_PARAMETER;
  }
  
  if ( Packet->Cdb == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Checks whether the request command is supported.
  //
  if (!IsCommandValid (Packet)) {
    return EFI_UNSUPPORTED;
  }
  
  return EFI_SUCCESS;
}  

BOOLEAN
IsCommandValid (
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET   *Packet
  ) 
/*++
  
  Checks the requested SCSI command: 
    Is it supported by this driver?
    Is the Data transfer direction reasonable?

--*/  
{
  UINT8     Index;
  UINT8     *OpCode;
  
  OpCode = (UINT8*)(Packet->Cdb);
  
  for (Index = 0; EfiCompareMem (&SupportedATAPICommands[Index],&EndTable,sizeof(SCSI_COMMAND_SET)); Index ++) {

    if ( *OpCode == SupportedATAPICommands[Index].OpCode) {
      //
      // Check whether the requested Command is supported by this driver
      //
      
      if (Packet->DataDirection == DataIn) {
        //
        // Check whether the requested data direction conforms to
        // what it should be.
        //
        if (SupportedATAPICommands[Index].Direction == DataOut) {
          return FALSE;
        }
      }
      
      if (Packet->DataDirection == DataOut) {
        //
        // Check whether the requested data direction conforms to
        // what it should be.
        //
        if (SupportedATAPICommands[Index].Direction == DataIn) {
          return FALSE;
        }
      }
      
      return TRUE;
    }
  }
  
  return FALSE;
}    

EFI_STATUS
SubmitBlockingIoCommand (
  ATAPI_SCSI_PASS_THRU_DEV                  *AtapiScsiPrivate,
  UINT32                                    Target,
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET    *Packet  
  )
/*++

Routine Description:
    Performs blocking I/O request.
    
  Arguments:
    AtapiScsiPrivate:   Private data structure for the specified channel.
    Target:             The Target ID of the ATAPI device to send the SCSI 
                        Request Packet. To ATAPI devices attached on an IDE
                        Channel, Target ID 0 indicates Master device;Target
                        ID 1 indicates Slave device.
    Packet:             The SCSI Request Packet to send to the ATAPI device 
                        specified by Target.
  
  Returns:
  
--*/  
{
  UINT8           PacketCommand[12];
  UINT64          TimeoutInMicroSeconds;
  EFI_STATUS      PacketCommandStatus;
  UINTN           Remainder;
  
  //
  // Fill ATAPI Command Packet according to CDB
  //
  EfiZeroMem (&PacketCommand,12);  
  EfiCopyMem (&PacketCommand,Packet->Cdb,Packet->CdbLength);  
  
  //
  // Timeout is 100ns unit, convert it to 1000ns (1us) unit.
  //
  TimeoutInMicroSeconds = DriverLibDivU64x32 (Packet->Timeout, (UINT32)10,&Remainder);
  
  //
  // Submit ATAPI Command Packet
  //
  PacketCommandStatus = AtapiPacketCommand (AtapiScsiPrivate,
                                           Target,
                                           PacketCommand,
                                           Packet->DataBuffer, 
                                           &(Packet->TransferLength), 
                                           Packet->DataDirection, 
                                           TimeoutInMicroSeconds
                                           );
  if (!EFI_ERROR(PacketCommandStatus) ||
      (Packet->SenseData == NULL)) {
    Packet->SenseDataLength = 0;
    return PacketCommandStatus;
  }
  
  //
  // Return SenseData if PacketCommandStatus matches 
  // the following return codes.
  //
  if ((PacketCommandStatus == EFI_WARN_BUFFER_TOO_SMALL) ||
      (PacketCommandStatus == EFI_DEVICE_ERROR) ||
      (PacketCommandStatus == EFI_TIMEOUT)) {
    
    //
    // avoid submit request sense command continuously.
    //
    if (PacketCommand[0] == OP_REQUEST_SENSE) {
      Packet->SenseDataLength = 0;
      return PacketCommandStatus;
    }
    
    RequestSenseCommand (AtapiScsiPrivate,Target,Packet->Timeout,
                         Packet->SenseData,&Packet->SenseDataLength
                         );
  }
  
  return PacketCommandStatus;  
}    

EFI_STATUS
RequestSenseCommand(
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT32                      Target,
  UINT64                      Timeout,
  VOID                        *SenseData,
  UINT8                       *SenseDataLength
  )
{
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET    Packet;
  UINT8                                     Cdb[12];
  EFI_STATUS                                Status;
  
  EfiZeroMem (&Packet,sizeof (EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET));
  EfiZeroMem (Cdb,12);
  
  Cdb[0] = OP_REQUEST_SENSE;
  Cdb[4] = (UINT8)(*SenseDataLength);

  Packet.Timeout = Timeout;
  Packet.DataBuffer = SenseData;
  Packet.SenseData = NULL;
  Packet.Cdb = Cdb;
  Packet.TransferLength = *SenseDataLength;
  Packet.CdbLength = 12;
  Packet.DataDirection = DataIn;
  
  Status = SubmitBlockingIoCommand (AtapiScsiPrivate,Target,&Packet);
  *SenseDataLength = (UINT8)(Packet.TransferLength);
  return Status;
}

EFI_STATUS
AtapiPacketCommand (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT32                      Target,
  UINT8                       *PacketCommand,
  VOID                        *Buffer,   
  UINT32                      *ByteCount,
  DATA_DIRECTION              Direction,
  UINT64                      TimeoutInMicroSeconds
  )
/*++

  Routine Description:
    Submits ATAPI command packet to the specified ATAPI device.
    
  Arguments:
    AtapiScsiPrivate:   Private data structure for the specified channel.
    Target:             The Target ID of the ATAPI device to send the SCSI 
                        Request Packet. To ATAPI devices attached on an IDE
                        Channel, Target ID 0 indicates Master device;Target
                        ID 1 indicates Slave device.
    PacketCommand:      Points to the ATAPI command packet.
    Buffer:             Points to the transferred data.
    ByteCount:          When input,indicates the buffer size; when output,
                        indicates the actually transferred data size.
    Direction:          Indicates the data transfer direction. 
    TimeoutInMicroSeconds:
                        The timeout, in micro second units, to use for the 
                        execution of this ATAPI command.
                        A TimeoutInMicroSeconds value of 0 means that 
                        this function will wait indefinitely for the ATAPI 
                        command to execute.
                        If TimeoutInMicroSeconds is greater than zero, then 
                        this function will return EFI_TIMEOUT if the time 
                        required to execute the ATAPI command is greater 
                        than TimeoutInMicroSeconds.
  
  Returns:


--*/  
{

  UINT16          *CommandIndex;
  UINT8           Count;
  EFI_STATUS      Status;
  
  //
  // Set all the command parameters by fill related registers.
  // Before write to all the following registers, BSY and DRQ must be 0.
  //
  Status = StatusDRQClear (AtapiScsiPrivate, TimeoutInMicroSeconds);
  if (EFI_ERROR(Status)) {
    if (Status == EFI_ABORTED) {
      Status = EFI_DEVICE_ERROR;
    }
    *ByteCount = 0;
    return Status;
  }

  //
  // Select device via Device/Head Register.
  // "Target = 0" indicates device 0; "Target = 1" indicates device 1
  //
  WritePortB (AtapiScsiPrivate->PciIo,
              AtapiScsiPrivate->IoPort->Head, 
              (UINT8)((Target << 4) | DEFAULT_CMD)    // DEFAULT_CMD: 0xa0 (1010,0000)
              );  
  
  //
  // No OVL; No DMA (by setting feature register)
  //
  WritePortB (AtapiScsiPrivate->PciIo,
              AtapiScsiPrivate->IoPort->Reg1.Feature, 
              0x00
              );             

  //
  // set the transfersize to MAX_ATAPI_BYTE_COUNT to let the device
  // determine how much data should be transfered.
  //
  WritePortB (AtapiScsiPrivate->PciIo,
              AtapiScsiPrivate->IoPort->CylinderLsb, 
              (UINT8)(MAX_ATAPI_BYTE_COUNT & 0x00ff)
              ); 
  WritePortB (AtapiScsiPrivate->PciIo,
              AtapiScsiPrivate->IoPort->CylinderMsb, 
              (UINT8)(MAX_ATAPI_BYTE_COUNT >> 8)
              );  

  //
  //  DEFAULT_CTL:0x0a (0000,1010)
  //  Disable interrupt
  //
  WritePortB (AtapiScsiPrivate->PciIo,
             AtapiScsiPrivate->IoPort->Alt.DeviceControl, 
             DEFAULT_CTL
             );       
  
  //
  // Send Packet command to inform device 
  // that the following data bytes are command packet.
  //
  WritePortB (AtapiScsiPrivate->PciIo,
              AtapiScsiPrivate->IoPort->Reg.Command, 
              PACKET_CMD
              );   
  
  //
  // Before data transfer, BSY should be 0 and DRQ should be 1.
  // if they are not in specified time frame,
  // retrieve Sense Key from Error Register before return.
  //
  Status = StatusDRQReady(AtapiScsiPrivate, TimeoutInMicroSeconds);
  if (EFI_ERROR(Status)) {
    if (Status == EFI_ABORTED) {
      Status = EFI_DEVICE_ERROR;
    }
    *ByteCount = 0;
    return Status;
  }

  //
  // Send out command packet
  //
  CommandIndex = (UINT16*)PacketCommand; 
  for (Count = 0; Count < 6; Count++, CommandIndex++) {    
    WritePortW (AtapiScsiPrivate->PciIo,AtapiScsiPrivate->IoPort->Data, *CommandIndex);
  }

  //
  // call AtapiPassThruPioReadWriteData() function to get 
  // requested transfer data form device.
  //
  return AtapiPassThruPioReadWriteData(AtapiScsiPrivate, 
                                       Buffer, 
                                       ByteCount, 
                                       Direction, 
                                       TimeoutInMicroSeconds
                                       );
}

EFI_STATUS 
AtapiPassThruPioReadWriteData (       
  ATAPI_SCSI_PASS_THRU_DEV  *AtapiScsiPrivate, 
  UINT16                    *Buffer,   
  UINT32                    *ByteCount,
  DATA_DIRECTION            Direction,
  UINT64                    TimeoutInMicroSeconds
  )
/*++

  Routine Description:
    Performs data transfer between ATAPI device and host after the
    ATAPI command packet is sent.
    
  Arguments:
    AtapiScsiPrivate:   Private data structure for the specified channel.    
    Buffer:             Points to the transferred data.
    ByteCount:          When input,indicates the buffer size; when output,
                        indicates the actually transferred data size.
    Direction:          Indicates the data transfer direction. 
    TimeoutInMicroSeconds:
                        The timeout, in micro second units, to use for the 
                        execution of this ATAPI command.
                        A TimeoutInMicroSeconds value of 0 means that 
                        this function will wait indefinitely for the ATAPI 
                        command to execute.
                        If TimeoutInMicroSeconds is greater than zero, then 
                        this function will return EFI_TIMEOUT if the time 
                        required to execute the ATAPI command is greater 
                        than TimeoutInMicroSeconds.

  Returns:


--*/               
{                                                      
  UINT32        Index;   
  UINT32        RequiredWordCount; 
  UINT32        ActualWordCount;
  
  UINT32        WordCount;
  EFI_STATUS    Status ;
  UINT16        *ptrBuffer;

  Status      = EFI_SUCCESS ;
  
  //
  // Non Data transfer request is also supported.
  //
  if (*ByteCount == 0 || Buffer == NULL) {
    *ByteCount = 0;
    if (EFI_ERROR(StatusWaitForBSYClear (AtapiScsiPrivate,TimeoutInMicroSeconds))) {
      return EFI_DEVICE_ERROR;
    }
  }
  
  ptrBuffer = Buffer;
  RequiredWordCount = *ByteCount / 2;
 
  //
  // ActuralWordCount means the word count of data really transfered.
  //
  ActualWordCount = 0;

  while (ActualWordCount < RequiredWordCount) { 
    
    //
    // before each data transfer stream, the host should poll DRQ bit ready,
    // which indicates device's ready for data transfer .
    //
    Status = StatusDRQReady (AtapiScsiPrivate, TimeoutInMicroSeconds);
    if (EFI_ERROR(Status)) {            
      *ByteCount = ActualWordCount * 2;        
      
      AtapiPassThruCheckErrorStatus (AtapiScsiPrivate);
      
      if (ActualWordCount == 0) {
        return EFI_DEVICE_ERROR;
      }
      //
      // ActualWordCount > 0
      //
      if (ActualWordCount < RequiredWordCount) {
        return EFI_WARN_BUFFER_TOO_SMALL;
      }
    }
    
    //
    // get current data transfer size from Cylinder Registers.
    //
    WordCount = ((ReadPortB (AtapiScsiPrivate->PciIo,AtapiScsiPrivate->IoPort->CylinderMsb) << 8) | 
                  ReadPortB (AtapiScsiPrivate->PciIo,AtapiScsiPrivate->IoPort->CylinderLsb)) & 0xffff;                  
    WordCount /= 2;

    //
    // perform a series data In/Out.
    //
    for (Index = 0; (Index < WordCount) && (ActualWordCount < RequiredWordCount); Index ++, ActualWordCount++) {
      
      if (Direction == DataIn) {
        
        *ptrBuffer = ReadPortW (AtapiScsiPrivate->PciIo,AtapiScsiPrivate->IoPort->Data);
      } else {
        
        WritePortW (AtapiScsiPrivate->PciIo,AtapiScsiPrivate->IoPort->Data, *ptrBuffer);
      }
      
      ptrBuffer++;

    }
  }
  //
  // After data transfer is completed, normally, DRQ bit should clear.
  //
  StatusDRQClear(AtapiScsiPrivate, TimeoutInMicroSeconds);
  
  //
  // read status register to check whether error happens.
  //
  Status = AtapiPassThruCheckErrorStatus (AtapiScsiPrivate);    
  
  *ByteCount = ActualWordCount * 2;
  
  return Status;
} 

STATIC
UINT8
ReadPortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  )
/*++
  Read one byte from a specified I/O port.
--*/  
{
  UINT8         Data = 0;

  PciIo->Io.Read(PciIo,
                 EfiPciIoWidthUint8,
                 EFI_PCI_IO_PASS_THROUGH_BAR,
                 (UINT64)Port,
                 1,
                 &Data
                 );
  return Data;                 
}   

STATIC
UINT16
ReadPortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  )
/*++
  Read one word from a specified I/O port.
--*/  
{
  UINT16          Data = 0;
  
  PciIo->Io.Read(PciIo,
                 EfiPciIoWidthUint16,
                 EFI_PCI_IO_PASS_THROUGH_BAR,
                 (UINT64)Port,
                 1,
                 &Data
                 );
    return Data;      
}   

STATIC
VOID
WritePortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINT8                 Data
  )
/*++
  Write one byte to a specified I/O port.
--*/  
{

  
  PciIo->Io.Write(PciIo,
                  EfiPciIoWidthUint8,
                  EFI_PCI_IO_PASS_THROUGH_BAR,
                  (UINT64)Port,
                  1,
                  &Data
                  );  

}   

STATIC
VOID
WritePortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port, 
  IN  UINT16                Data
  )
/*++
  Write one word to a specified I/O port.
--*/    
{
  
  PciIo->Io.Write(PciIo,
                  EfiPciIoWidthUint16,
                  EFI_PCI_IO_PASS_THROUGH_BAR,
                  (UINT64)Port,
                  1,
                  &Data
                  );
}      

EFI_STATUS  
StatusDRQClear (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeoutInMicroSeconds
  )
/*++
  Check whether DRQ is clear in the Status Register. (BSY must also be cleared)
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRQ clear. Otherwise, it will return EFI_TIMEOUT when specified time is 
  elapsed.
--*/  
{
  UINT64        Delay; 
  UINT8         StatusRegister,ErrRegister;
  UINTN         Remainder;
  
  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DriverLibDivU64x32 (TimeoutInMicroSeconds, (UINT32)30,&Remainder) + 1;
  }

  do {
    
    StatusRegister = ReadPortB(AtapiScsiPrivate->PciIo,
                               AtapiScsiPrivate->IoPort->Reg.Status
                               );
    
    //
    // wait for BSY == 0 and DRQ == 0
    //
    if ( (StatusRegister & (DRQ | BSY) ) == 0) {
      break;
    }
    //
    // check whether the command is aborted by the device
    //  
    if ((StatusRegister & (BSY | ERR)) == ERR ) {
      
     ErrRegister =  ReadPortB(AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Reg1.Error
                              ); 
     if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {
        
      return EFI_ABORTED;
     }
    }

    //
    //  Stall for 30 us
    //
    gBS->Stall(30);
        
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }
    
    Delay --;
  } while (Delay);
  
  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

EFI_STATUS  
AltStatusDRQClear (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeoutInMicroSeconds
  )
/*++
  Check whether DRQ is clear in the Alternate Status Register. 
  (BSY must also be cleared).
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRQ clear. Otherwise, it will return EFI_TIMEOUT when specified time is 
  elapsed.
--*/  
{
  UINT64        Delay; 
  UINT8         AltStatusRegister,ErrRegister;
  UINTN         Remainder;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DriverLibDivU64x32 (TimeoutInMicroSeconds, (UINT32)30,&Remainder) + 1;
  }

  do {
    
    AltStatusRegister = ReadPortB(AtapiScsiPrivate->PciIo,
                               AtapiScsiPrivate->IoPort->Alt.AltStatus
                               );
    
    //
    // wait for BSY == 0 and DRQ == 0
    //
    if ( (AltStatusRegister & (DRQ |  BSY) ) == 0) {
    break;
    }
  
    if ((AltStatusRegister & (BSY | ERR)) == ERR ) {
      
     ErrRegister =  ReadPortB(AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Reg1.Error
                              ); 
     if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {
        
      return EFI_ABORTED;
     }
    }

    //
    //  Stall for 30 us
    //
    gBS->Stall(30);
    
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }
    
    Delay --;
  } while (Delay);
  
  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

EFI_STATUS  
StatusDRQReady (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeoutInMicroSeconds
  )
/*++
  Check whether DRQ is ready in the Status Register. (BSY must also be cleared)
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRQ ready. Otherwise, it will return EFI_TIMEOUT when specified time is 
  elapsed.
--*/  
{
  UINT64      Delay; 
  UINT8       StatusRegister;
  UINT8       ErrRegister;
  UINTN       Remainder;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DriverLibDivU64x32 (TimeoutInMicroSeconds, (UINT32)30,&Remainder) + 1;
  }

  do {
    //
    //  read Status Register will clear interrupt
    //
    StatusRegister = ReadPortB(AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Reg.Status
                              );   
    
    //
    //  BSY==0,DRQ==1
    //
    if ( (StatusRegister & (BSY | DRQ )) == DRQ) {
      break;
    }
    
    if ((StatusRegister & (BSY | ERR)) == ERR ) {
      
     ErrRegister =  ReadPortB(AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Reg1.Error
                              ); 
     if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {        
      return EFI_ABORTED;
     }
    }
   
    gBS->Stall(30); // Stall for 30 us
    
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }
    
    Delay --;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

EFI_STATUS  
AltStatusDRQReady (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeoutInMicroSeconds
  )
/*++
  Check whether DRQ is ready in the Alternate Status Register. 
  (BSY must also be cleared)
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRQ ready. Otherwise, it will return EFI_TIMEOUT when specified time is 
  elapsed.
--*/  
{
  UINT64      Delay; 
  UINT8       AltStatusRegister;
  UINT8       ErrRegister;
  UINTN       Remainder;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DriverLibDivU64x32 (TimeoutInMicroSeconds, (UINT32)30,&Remainder) + 1;
  }
  
  do {
    //
    //  read Status Register will clear interrupt
    //
    AltStatusRegister = ReadPortB(AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Alt.AltStatus
                              );      
    //
    //  BSY==0,DRQ==1
    //
    if ((AltStatusRegister & (BSY | DRQ )) == DRQ) {
      break;
    }
    
    if ((AltStatusRegister & (BSY | ERR)) == ERR ) {
      
     ErrRegister =  ReadPortB(AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Reg1.Error
                              ); 
     if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {        
      return EFI_ABORTED;
     }
    }
   
    gBS->Stall(30); // Stall for 30 us
    
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }
    
    Delay --;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

EFI_STATUS  
StatusWaitForBSYClear (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  )
/*++
  Check whether BSY is clear in the Status Register.
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  BSY clear. Otherwise, it will return EFI_TIMEOUT when specified time is 
  elapsed.
--*/  
{
  UINT64      Delay; 
  UINT8       StatusRegister;
  UINTN       Remainder;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DriverLibDivU64x32 (TimeoutInMicroSeconds, (UINT32)30,&Remainder) + 1;
  }
  
  do {

    StatusRegister = ReadPortB(AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Reg.Status); 
    if ( (StatusRegister & BSY) == 0x00) {
      break;
    }
    
    gBS->Stall(30); // Stall for 30 us 
    
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }  
    
    Delay --;
  } while (Delay);

  if (Delay == 0){
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

EFI_STATUS  
AltStatusWaitForBSYClear  (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  )
/*++
  Check whether BSY is clear in the Alternate Status Register.
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  BSY clear. Otherwise, it will return EFI_TIMEOUT when specified time is 
  elapsed.
--*/    
{
  UINT64      Delay; 
  UINT8       AltStatusRegister;
  UINTN       Remainder;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DriverLibDivU64x32 (TimeoutInMicroSeconds, (UINT32)30,&Remainder) + 1;
  }
  
  do {

    AltStatusRegister = ReadPortB(AtapiScsiPrivate->PciIo,
                                  AtapiScsiPrivate->IoPort->Alt.AltStatus); 
    if ((AltStatusRegister & BSY) == 0x00) {
      break;
    }
    
    gBS->Stall(30); // Stall for 30 us   
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    } 
    
    Delay --;
  } while (Delay);

  if (Delay == 0){
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

EFI_STATUS
StatusDRDYReady (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                       TimeoutInMicroSeconds
  )
/*++
  Check whether DRDY is ready in the Status Register. 
  (BSY must also be cleared)
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRDY ready. Otherwise, it will return EFI_TIMEOUT when specified time is 
  elapsed.
--*/  
{
  UINT64      Delay; 
  UINT8       StatusRegister,ErrRegister;
  UINTN       Remainder;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DriverLibDivU64x32 (TimeoutInMicroSeconds, (UINT32)30,&Remainder) + 1;
  }
  
  do {
    StatusRegister = ReadPortB(AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Reg.Status); 
    //
    //  BSY == 0 , DRDY == 1
    //
    if ( (StatusRegister & (DRDY | BSY) ) == DRDY) {
      break;
    }
    
    if ((StatusRegister & (BSY | ERR)) == ERR ) {
        
      ErrRegister = ReadPortB(AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Reg1.Error
                              ); 
      if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {         
        return EFI_ABORTED;
      }
    }

    gBS->Stall(30); // Stall for 30 us
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    } 
    
    Delay --;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

EFI_STATUS
AltStatusDRDYReady (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                       TimeoutInMicroSeconds
  )
/*++
  Check whether DRDY is ready in the Alternate Status Register. 
  (BSY must also be cleared)
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRDY ready. Otherwise, it will return EFI_TIMEOUT when specified time is 
  elapsed.
--*/  
{
  UINT64      Delay; 
  UINT8       AltStatusRegister,ErrRegister;
  UINTN       Remainder;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DriverLibDivU64x32 (TimeoutInMicroSeconds, (UINT32)30,&Remainder) + 1;
  }
  
  do {
    AltStatusRegister = ReadPortB(AtapiScsiPrivate->PciIo,
                                  AtapiScsiPrivate->IoPort->Alt.AltStatus); 
    //
    //  BSY == 0 , DRDY == 1
    //
    if ((AltStatusRegister & (DRDY | BSY) ) == DRDY) {
      break;
    }
    
    if ((AltStatusRegister & (BSY | ERR)) == ERR ) {
        
      ErrRegister = ReadPortB(AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Reg1.Error
                              ); 
      if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {         
        return EFI_ABORTED;
      }
    }

    gBS->Stall(30); // Stall for 30 us
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    } 
    
    Delay --;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  } 

  return EFI_SUCCESS;
}

EFI_STATUS
AtapiPassThruCheckErrorStatus (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate
  )
/*++  
  Check Error Register for Error Information. 
--*/  
{
  UINT8   StatusRegister;

#ifdef EFI_DEBUG

  UINT8   ErrorRegister;

#endif

  StatusRegister = ReadPortB (AtapiScsiPrivate->PciIo,
                              AtapiScsiPrivate->IoPort->Reg.Status
                              ); 
  DEBUG_CODE (
    
    if (StatusRegister & DWF)  {
      DEBUG((EFI_D_BLKIO, "AtapiPassThruCheckErrorStatus()-- %02x : Error : Write Fault\n", 
             StatusRegister));
    }
    
    if (StatusRegister & CORR) {
      DEBUG((EFI_D_BLKIO,"AtapiPassThruCheckErrorStatus()-- %02x : Error : Corrected Data\n",
             StatusRegister));
    }
  
    if (StatusRegister & ERR) {
      ErrorRegister = ReadPortB (AtapiScsiPrivate->PciIo,AtapiScsiPrivate->IoPort->Reg1.Error);
  
      if (ErrorRegister & BBK_ERR) {
        DEBUG((EFI_D_BLKIO, "AtapiPassThruCheckErrorStatus()-- %02x : Error : Bad Block Detected\n",
               ErrorRegister));
      }
      
      if (ErrorRegister & UNC_ERR) {
        DEBUG((EFI_D_BLKIO, "AtapiPassThruCheckErrorStatus()-- %02x : Error : Uncorrectable Data\n",
                ErrorRegister));
      }
      
      if (ErrorRegister & MC_ERR) {
        DEBUG((EFI_D_BLKIO, "AtapiPassThruCheckErrorStatus()-- %02x : Error : Media Change\n", 
                ErrorRegister));
      }
      
      if (ErrorRegister & ABRT_ERR) {
        DEBUG((EFI_D_BLKIO, "AtapiPassThruCheckErrorStatus()-- %02x : Error : Abort\n",
               ErrorRegister));
      }
      
      if (ErrorRegister & TK0NF_ERR) {
        DEBUG((EFI_D_BLKIO, "AtapiPassThruCheckErrorStatus()-- %02x : Error : Track 0 Not Found\n",
               ErrorRegister));
      }
      
      if (ErrorRegister & AMNF_ERR) {
        DEBUG((EFI_D_BLKIO, "AtapiPassThruCheckErrorStatus()-- %02x : Error : Address Mark Not Found\n",
              ErrorRegister));
      }
  
    }
  )

  if ( (StatusRegister & (ERR | DWF | CORR) ) == 0 ) {
    
    return EFI_SUCCESS;  
  } 
  
  return EFI_DEVICE_ERROR;
  
}
