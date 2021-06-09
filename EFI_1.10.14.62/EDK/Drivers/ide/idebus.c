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

    idebus.c
    
Abstract: 
    

Revision History
--*/

#include "idebus.h"

#define PCI_CLASS_MASS_STORAGE    0x01
#define PCI_SUB_CLASS_IDE         0x01

//
// IDE registers' fixed address
//
static IDE_BASE_REGISTERS   IdeIoPortRegisters[2][2] = { 
    { 
        {0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5, 0x1f6, 0x1f7, 0x3f6, 0x3f7, 1}, 
        {0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5, 0x1f6, 0x1f7, 0x3f6, 0x3f7, 0}, 
    }, {
        {0x170, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x177, 0x376, 0x377, 1},
        {0x170, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x177, 0x376, 0x377, 0},
    } 
};

//
// IDE Bus Driver GUID
//
EFI_GUID gIDEBusDriverGuid = {
  0xe778c047, 0x2121, 0x4ad1, 0x82, 0x36, 0x9, 0x7, 0x40, 0x61, 0x9f, 0x5
};

//
// IDE Bus Driver Binding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gIDEBusDriverBinding = {
  IDEBusDriverBindingSupported,
  IDEBusDriverBindingStart,
  IDEBusDriverBindingStop,
  0x10,
  NULL,
  NULL
};  


EFI_DRIVER_ENTRY_POINT (IDEBusControllerDriverEntryPoint)

//***********************************************************************************
// IDEBusControllerDriverEntryPoint
//***********************************************************************************
EFI_STATUS
IDEBusControllerDriverEntryPoint(
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/                
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gIDEBusDriverBinding,
           ImageHandle,
           &gIDEBusComponentName,
           &gIDEBusDriverConfiguration,
           &gIDEBusDriverDiagnostics
           );

} 

//***********************************************************************************
// IDEBusDriverBindingSupported
//***********************************************************************************
EFI_STATUS
IDEBusDriverBindingSupported (
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
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  PCI_TYPE00                Pci;
  EFI_DEV_PATH              *Node;  

  if (RemainingDevicePath != NULL) {
    Node = (EFI_DEV_PATH *)RemainingDevicePath;
    if (Node->DevPath.Type != MESSAGING_DEVICE_PATH ||
        Node->DevPath.SubType != MSG_ATAPI_DP ||
        DevicePathNodeLength(&Node->DevPath) != sizeof(ATAPI_DEVICE_PATH)) {
      return EFI_UNSUPPORTED;
    }
  }

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller      ,   
                  &gEfiDevicePathProtocolGuid,  
                  (VOID**)&ParentDevicePath,
                  This->DriverBindingHandle,     
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }
 
  //
  // Clsoe protocol, don't use device path protocol in the .Support() function
  //
  gBS->CloseProtocol (
         Controller,           
         &gEfiDevicePathProtocolGuid, 
         This->DriverBindingHandle,   
         Controller
         );

  Status = gBS->OpenProtocol (
                  Controller,           
                  &gEfiPciIoProtocolGuid, 
                  (VOID**)&PciIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }
      
  
  //
  // Use the PCI I/O Protocol to see if Controller is a IDE Controller that
  // can be managed by this driver.  Read the PCI Configuration Header 
  // for this device.
  //
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0, sizeof(Pci), &Pci);
  if (EFI_ERROR (Status)) {
    Status = EFI_UNSUPPORTED;
    goto Done;
  }
  
  if(Pci.Hdr.ClassCode[2] != PCI_CLASS_MASS_STORAGE || 
      Pci.Hdr.ClassCode[1] != PCI_SUB_CLASS_IDE) {
    
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

Done:
  gBS->CloseProtocol (
         Controller,  
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
         
  return Status;    
}   

//***********************************************************************************
// IDEBusDriverBindingStart
//***********************************************************************************
EFI_STATUS
IDEBusDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/                
{
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;
  EFI_DEV_PATH              *Node;
  EFI_IDE_CHANNEL           IdeChannel,BeginningIdeChannel,EndIdeChannel;
  EFI_IDE_DEVICE            IdeDevice,BeginningIdeDevice,EndIdeDevice;
  IDE_BLK_IO_DEV            *IdeBlkIoDevice;
  EFI_DEV_PATH              NewNode;
  UINT8                     ConfigurationOptions;
  UINTN                     DataSize;
  UINT32                    Attributes;
  UINTN                     StringIndex;
  CHAR16                    ModelName[41];
  
  IDE_BUS_DRIVER_PRIVATE_DATA   *IdeBusDriverPrivateData = NULL;
  
  Status = gBS->OpenProtocol (
                  Controller,   
                  &gEfiDevicePathProtocolGuid,  
                  (VOID**)&ParentDevicePath,
                  This->DriverBindingHandle,     
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if ((EFI_ERROR(Status)) && (Status != EFI_ALREADY_STARTED)) {
    return Status;
  }                   
  //
  // Consume PCI I/O protocol.
  //
  Status = gBS->OpenProtocol (
                  Controller, 
                  &gEfiPciIoProtocolGuid, 
                  (VOID**)&PciIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if ((EFI_ERROR (Status)) && Status != EFI_ALREADY_STARTED) {
    goto ErrorExit;
  }

  if (Status != EFI_ALREADY_STARTED) {
    Status = gBS->AllocatePool (
                      EfiBootServicesData,
                      sizeof (IDE_BUS_DRIVER_PRIVATE_DATA),
                      (VOID**)&IdeBusDriverPrivateData
                      );
    if (EFI_ERROR(Status)) {
      goto ErrorExit;
    }

    EfiZeroMem (IdeBusDriverPrivateData, sizeof (IDE_BUS_DRIVER_PRIVATE_DATA));
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Controller,
                    &gIDEBusDriverGuid, IdeBusDriverPrivateData,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      goto ErrorExit;
    }

  } else {
    Status = gBS->OpenProtocol (
                  Controller, 
                  &gIDEBusDriverGuid, 
                  (VOID**)&IdeBusDriverPrivateData,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR(Status)) {
      IdeBusDriverPrivateData = NULL;
      goto ErrorExit;
    }
  }
  
  Status = PciIo->Attributes (
                    PciIo, 
                    EfiPciIoAttributeOperationEnable, 
                    EFI_PCI_IO_ATTRIBUTE_IDE_PRIMARY_IO | EFI_PCI_IO_ATTRIBUTE_IDE_SECONDARY_IO | EFI_PCI_DEVICE_ENABLE, 
                    NULL
                    );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  //
  // Read the environment variable that contains the IDEBus Driver's 
  // Config options that were set by the Driver Configuration Protocol
  //
  DataSize = sizeof (ConfigurationOptions);
  Status = gRT->GetVariable (
                  L"Configuration", 
                  &gIDEBusDriverGuid, 
                  &Attributes, 
                  &DataSize, 
                  &ConfigurationOptions
                  );
  if (EFI_ERROR (Status)) {
    ConfigurationOptions = 0x0f;
  }
  
  //
  // Parsing RemainingDevicePath parameter;
  // if RemainingDevicePath is NULL, scan IDE bus for each device;
  // if RemainingDevicePath is not NULL, only scan the specified device.
  //
  if (RemainingDevicePath == NULL) {
    BeginningIdeChannel = IdePrimary;
    EndIdeChannel       = IdeSecondary;
    BeginningIdeDevice  = IdeMaster;
    EndIdeDevice        = IdeSlave;
  } else {
    Node = (EFI_DEV_PATH*)RemainingDevicePath;
    
    BeginningIdeChannel = Node->Atapi.PrimarySecondary;
    EndIdeChannel       = BeginningIdeChannel;
    BeginningIdeDevice  = Node->Atapi.SlaveMaster;
    EndIdeDevice        = BeginningIdeDevice;    
  }

  for (IdeChannel = BeginningIdeChannel;IdeChannel <= EndIdeChannel;IdeChannel++) {
    
    for (IdeDevice = BeginningIdeDevice;  IdeDevice <= EndIdeDevice; IdeDevice++) {
      //
      // See if the configuration options allow this device
      //
      if (!(ConfigurationOptions & (1 << (IdeChannel * 2 + IdeDevice)))) {
        continue;
      }

      //
      // The device has been scanned in another Start(), No need to scan it again
      // for perf optimization.
      //
      if (IdeBusDriverPrivateData->HaveScannedDevice[IdeChannel * 2 + IdeDevice]) {
        continue;
      }
      
      //
      // create child handle for the detected device.
      //
      Status = gBS->AllocatePool (
                      EfiBootServicesData,
                      sizeof (IDE_BLK_IO_DEV),
                      (VOID**)&IdeBlkIoDevice
                      );
      if (EFI_ERROR (Status)) {
        continue;
      }

      EfiZeroMem (IdeBlkIoDevice, sizeof (IDE_BLK_IO_DEV));
  
      IdeBlkIoDevice->Signature = IDE_BLK_IO_DEV_SIGNATURE;
      IdeBlkIoDevice->Channel   = IdeChannel;
      IdeBlkIoDevice->Device    = IdeDevice;
  
      //
      // initialize Block IO interface's Media pointer
      //
      IdeBlkIoDevice->BlkIo.Media = &IdeBlkIoDevice->BlkMedia;
      IdeBlkIoDevice->IoPort      = &IdeIoPortRegisters[IdeBlkIoDevice->Channel][IdeBlkIoDevice->Device]; 
      IdeBlkIoDevice->PciIo       = PciIo;      
      IdeBlkIoDevice->IdeBusDriverPrivateData = IdeBusDriverPrivateData;
      //
      // the device has been scanned in another Start(),
      // no need to scan it again for perf optimization.
      //
      if (IdeBusDriverPrivateData->HaveScannedDevice[IdeChannel * 2 + IdeDevice]) {
        ReleaseIdeResources (IdeBlkIoDevice);
        continue;
      }
      
      Status = DiscoverIdeDevice (IdeBlkIoDevice);      
      IdeBusDriverPrivateData->HaveScannedDevice[IdeChannel * 2 + IdeDevice] = TRUE;

      if (!EFI_ERROR (Status)) {
        //
        // Set Device Path
        //    
        EfiZeroMem (&NewNode, sizeof(NewNode));
        NewNode.DevPath.Type    = MESSAGING_DEVICE_PATH;
        NewNode.DevPath.SubType = MSG_ATAPI_DP;
        SetDevicePathNodeLength (&NewNode.DevPath, sizeof(ATAPI_DEVICE_PATH));

        NewNode.Atapi.PrimarySecondary = (UINT8)IdeBlkIoDevice->Channel;
        NewNode.Atapi.SlaveMaster      = (UINT8)IdeBlkIoDevice->Device;
        NewNode.Atapi.Lun              = IdeBlkIoDevice->Lun;
        IdeBlkIoDevice->DevicePath = EfiAppendDevicePathNode (ParentDevicePath,
                                                              &NewNode.DevPath);
        if (IdeBlkIoDevice->DevicePath == NULL) {
          ReleaseIdeResources (IdeBlkIoDevice);
          continue;
        }

        //
        // Add Component Name for the IDE/ATAPI device that was discovered.
        // Use the ModelName data from the Inquiry command to build an English 
        // Component Name Unicode String.
        //
        IdeBlkIoDevice->ControllerNameTable = NULL;
        for (StringIndex = 0; StringIndex < 41; StringIndex++) {
          ModelName[StringIndex] = IdeBlkIoDevice->ModelName[StringIndex];
        }
        EfiLibAddUnicodeString (
          "eng", 
          gIDEBusComponentName.SupportedLanguages, 
          &IdeBlkIoDevice->ControllerNameTable, 
          ModelName
          );

        Status = gBS->InstallMultipleProtocolInterfaces (
                        &IdeBlkIoDevice->Handle,           
                        &gEfiDevicePathProtocolGuid, IdeBlkIoDevice->DevicePath,
                        &gEfiBlockIoProtocolGuid,    &IdeBlkIoDevice->BlkIo,
                        NULL
                        );

        if (!EFI_ERROR (Status)) {
          gBS->OpenProtocol (
                 Controller,   
                 &gEfiPciIoProtocolGuid,
                 (VOID**)&PciIo,
                 This->DriverBindingHandle,
                 IdeBlkIoDevice->Handle,
                 EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                 );
        }
      }
     
      if (EFI_ERROR (Status)) {
        ReleaseIdeResources (IdeBlkIoDevice);
      }

    }     
  }
  
  return EFI_SUCCESS;

ErrorExit:  
  
  gBS->UninstallMultipleProtocolInterfaces (
           Controller,
           &gIDEBusDriverGuid, IdeBusDriverPrivateData,
           NULL
           );
  
  if (IdeBusDriverPrivateData != NULL) {
    gBS->FreePool (IdeBusDriverPrivateData);
  }
           
  gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );

  gBS->CloseProtocol (
         Controller, 
         &gEfiDevicePathProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
  
  return Status;
  
}
  
//***********************************************************************************
// IDEBusDriverBindingStop
//***********************************************************************************
EFI_STATUS
IDEBusDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/                
{
  EFI_STATUS           Status;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  BOOLEAN              AllChildrenStopped;
  UINTN                Index;
  
  IDE_BUS_DRIVER_PRIVATE_DATA   *IdeBusDriverPrivateData = NULL;
  
  if (NumberOfChildren == 0) {

    Status = gBS->OpenProtocol (
                    Controller, 
                    &gEfiPciIoProtocolGuid, 
                    (VOID**)&PciIo,
                    This->DriverBindingHandle,   
                    Controller,   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {
      PciIo->Attributes (
               PciIo, 
               EfiPciIoAttributeOperationDisable, 
               EFI_PCI_IO_ATTRIBUTE_IDE_PRIMARY_IO | EFI_PCI_IO_ATTRIBUTE_IDE_SECONDARY_IO | EFI_PCI_DEVICE_ENABLE, 
               NULL
               );
    }
    
    gBS->OpenProtocol (
                  Controller, 
                  &gIDEBusDriverGuid, 
                  (VOID**)&IdeBusDriverPrivateData,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
                  
    gBS->UninstallMultipleProtocolInterfaces (
           Controller,
           &gIDEBusDriverGuid, IdeBusDriverPrivateData,
           NULL
           );
           
    if (IdeBusDriverPrivateData != NULL) {
      gBS->FreePool (IdeBusDriverPrivateData);
    }
    //
    // Close the bus driver
    //
    gBS->CloseProtocol (
           Controller, 
           &gEfiPciIoProtocolGuid, 
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
    
    Status = DeRegisterIdeDevice (This, Controller, ChildHandleBuffer[Index]) ;
    
    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }
  
  return EFI_SUCCESS;
}

//***********************************************************************************
// DeRegisterIdeDevice
//***********************************************************************************
EFI_STATUS 
DeRegisterIdeDevice ( 
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  EFI_HANDLE                     Handle
  ) 
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                    Status;
  EFI_BLOCK_IO_PROTOCOL         *BlkIo;
  IDE_BLK_IO_DEV                *IdeBlkIoDevice;
  EFI_PCI_IO_PROTOCOL           *PciIo;
  UINTN                         Index;

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiBlockIoProtocolGuid,  
                  (VOID**)&BlkIo,
                  This->DriverBindingHandle,             
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  IdeBlkIoDevice = IDE_BLOCK_IO_DEV_FROM_THIS (BlkIo);

  //
  // Close the child handle
  //
  Status = gBS->CloseProtocol (
                  Controller, 
                  &gEfiPciIoProtocolGuid, 
                  This->DriverBindingHandle, 
                  Handle
                  );

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Handle, 
                  &gEfiDevicePathProtocolGuid, IdeBlkIoDevice->DevicePath,
                  &gEfiBlockIoProtocolGuid,    &IdeBlkIoDevice->BlkIo,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    gBS->OpenProtocol (
           Controller,   
           &gEfiPciIoProtocolGuid,  
           (VOID**)&PciIo,
           This->DriverBindingHandle,   
           Handle,
           EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
           );
    return Status;
  }
  
  //
  // Release allocated resources
  //
  Index = IdeBlkIoDevice->Channel * 2 + IdeBlkIoDevice->Device;
  IdeBlkIoDevice->IdeBusDriverPrivateData->HaveScannedDevice[Index] = FALSE;

  ReleaseIdeResources (IdeBlkIoDevice);
  
  return EFI_SUCCESS;
}

//***********************************************************************************
// IDEBlkIoReset
//***********************************************************************************
EFI_STATUS
IDEBlkIoReset(
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  BOOLEAN                 ExtendedVerification
  )
{
  IDE_BLK_IO_DEV  *IdeBlkIoDevice;
  EFI_STATUS      Status;
    
  IdeBlkIoDevice = IDE_BLOCK_IO_DEV_FROM_THIS (This);
  
  //
  // for ATA device, using ATA reset method
  //    
  if(IdeBlkIoDevice->Type == IdeHardDisk) {
    return AtaSoftReset(IdeBlkIoDevice);
  }

  if(IdeBlkIoDevice->Type == IdeUnknown) {
    return EFI_DEVICE_ERROR;
  }
  
  //
  // for ATAPI device, using ATAPI reset method
  //
  Status = AtapiSoftReset(IdeBlkIoDevice);
  if (ExtendedVerification) {
    Status = AtaSoftReset(IdeBlkIoDevice);
  }
  return Status;
}   
  
EFI_STATUS
IDEBlkIoReadBlocks(
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  OUT VOID                    *Buffer
  )
{
  IDE_BLK_IO_DEV  *IdeBlkIoDevice;
    
  IdeBlkIoDevice = IDE_BLOCK_IO_DEV_FROM_THIS (This);
  
  //
  // for ATA device, using ATA read block's mechanism
  //  
  if(IdeBlkIoDevice->Type == IdeHardDisk) {        

    return AtaBlkIoReadBlocks(
                  IdeBlkIoDevice,
                  MediaId,
                  LBA,
                  BufferSize,
                  Buffer
                  );
  }

  if(IdeBlkIoDevice->Type == IdeUnknown) {        
    return EFI_DEVICE_ERROR;
  }
  
  //
  // for ATAPI device, using ATAPI read block's mechanism
  //
  return AtapiBlkIoReadBlocks(
                  IdeBlkIoDevice,
                  MediaId,
                  LBA,
                  BufferSize,
                  Buffer
                  );
 
}   
  
  
EFI_STATUS
IDEBlkIoWriteBlocks(
  IN  EFI_BLOCK_IO_PROTOCOL   *This,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  IN  VOID                    *Buffer
  )
{
  IDE_BLK_IO_DEV  *IdeBlkIoDevice;
    
  IdeBlkIoDevice = IDE_BLOCK_IO_DEV_FROM_THIS (This);
  
  //
  // for ATA device, using ATA write block's mechanism
  //
  if(IdeBlkIoDevice->Type == IdeHardDisk) {        
    return AtaBlkIoWriteBlocks(
                  IdeBlkIoDevice,
                  MediaId,
                  LBA,
                  BufferSize,
                  Buffer
                  );
  }

  if(IdeBlkIoDevice->Type == IdeUnknown) {        
    return EFI_DEVICE_ERROR;
  }
  
  //
  // for ATAPI device, using ATAPI write block's mechanism
  //
  return AtapiBlkIoWriteBlocks(
                  IdeBlkIoDevice,
                  MediaId,
                  LBA,
                  BufferSize,
                  Buffer
                  );
}     


//***********************************************************************************
// IDEBlkIoFlushBlocks
//***********************************************************************************
EFI_STATUS
IDEBlkIoFlushBlocks(
  IN  EFI_BLOCK_IO_PROTOCOL   *This
  )
{
  //
  // return directly
  //
  return EFI_SUCCESS;
}

