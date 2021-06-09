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

  PciVgaMiniPort.c
    
Abstract: 
    

Revision History
--*/

#include "PciVgaMiniPort.h"

//
// EFI Driver Binding Protocol Instance
//
//   This driver has a version value of 0x00000000.  This is the
//   lowest possible priority for a driver.  This is done on purpose to help
//   the developers of UGA drivers.  This driver can bind if no UGA driver 
//   is present, so a console is available.  Then, when a UGA driver is loaded
//   this driver can be disconnected, and the UGA driver can be connected.
//   As long as the UGA driver has a version value greater than 0x00000000, it
//   will be connected first and will block this driver from connecting.
//
EFI_DRIVER_BINDING_PROTOCOL gPciVgaMiniPortDriverBinding = {
  PciVgaMiniPortDriverBindingSupported,
  PciVgaMiniPortDriverBindingStart,
  PciVgaMiniPortDriverBindingStop,
  0x00000000,
  NULL,
  NULL
};  

//
// Driver Entry Point
//  

EFI_DRIVER_ENTRY_POINT(PciVgaMiniPortDriverEntryPoint)
    
EFI_STATUS
PciVgaMiniPortDriverEntryPoint(
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
           &gPciVgaMiniPortDriverBinding,
           ImageHandle,
           &gPciVgaMiniPortComponentName,
           NULL,
           NULL
           );
} 

EFI_STATUS
PciVgaMiniPortDriverBindingSupported (
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
  EFI_STATUS           Status;
  EFI_PCI_IO_PROTOCOL  *PciIo;
  PCI_TYPE00           Pci;

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
  // See if this is a PCI VGA Controller by looking at the Command register and 
  // Class Code Register
  //
  Status = PciIo->Pci.Read (
                        PciIo, 
                        EfiPciIoWidthUint32, 
                        0, 
                        sizeof (Pci) / sizeof (UINT32), 
                        &Pci
                        );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  
  Status = EFI_UNSUPPORTED;
  //
  // See if the device is an enabled VGA device.
  // Most systems can only have on VGA device on at a time.
  //
  if (((Pci.Hdr.Command & 0x03) == 0x03) && IS_PCI_VGA (&Pci)) {
    Status = EFI_SUCCESS;
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

EFI_STATUS
PciVgaMiniPortDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Install VGA Mini Port Protocol onto VGA device handles
  
  Arguments:
    (Standard DriverBinding Protocol Start() function)
    
  Returns:
    EFI_STATUS
    
--*/                
{
  EFI_STATUS             Status;
  EFI_PCI_IO_PROTOCOL    *PciIo;
  PCI_VGA_MINI_PORT_DEV  *PciVgaMiniPortPrivate;

  PciVgaMiniPortPrivate = NULL;
  PciIo = NULL;
  //
  // Open the IO Abstraction(s) needed 
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
    goto Done;
  }

  //
  // Allocate the private device structure
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(PCI_VGA_MINI_PORT_DEV),
                  &PciVgaMiniPortPrivate
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  EfiZeroMem (PciVgaMiniPortPrivate, sizeof (PCI_VGA_MINI_PORT_DEV)) ;
  
  //
  // Initialize the private device structure
  //
  PciVgaMiniPortPrivate->Signature = PCI_VGA_MINI_PORT_DEV_SIGNATURE;
  PciVgaMiniPortPrivate->Handle    = Controller;
  PciVgaMiniPortPrivate->PciIo     = PciIo;

  PciVgaMiniPortPrivate->VgaMiniPort.SetMode                   = PciVgaMiniPortSetMode;
  PciVgaMiniPortPrivate->VgaMiniPort.VgaMemoryOffset           = 0xb8000;
  PciVgaMiniPortPrivate->VgaMiniPort.CrtcAddressRegisterOffset = 0x3d4;
  PciVgaMiniPortPrivate->VgaMiniPort.CrtcDataRegisterOffset    = 0x3d5;
  PciVgaMiniPortPrivate->VgaMiniPort.VgaMemoryBar              = EFI_PCI_IO_PASS_THROUGH_BAR;
  PciVgaMiniPortPrivate->VgaMiniPort.CrtcAddressRegisterBar    = EFI_PCI_IO_PASS_THROUGH_BAR;
  PciVgaMiniPortPrivate->VgaMiniPort.CrtcDataRegisterBar       = EFI_PCI_IO_PASS_THROUGH_BAR;
  PciVgaMiniPortPrivate->VgaMiniPort.MaxMode                   = 1;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiVgaMiniPortProtocolGuid, &PciVgaMiniPortPrivate->VgaMiniPort,
                  NULL
                  );
Done:
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
           Controller, 
           &gEfiPciIoProtocolGuid, 
           This->DriverBindingHandle, 
           Controller
           );
    if (PciVgaMiniPortPrivate) {
      gBS->FreePool (PciVgaMiniPortPrivate);
    }
  }

  return Status;
}

EFI_STATUS
PciVgaMiniPortDriverBindingStop (
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
  EFI_STATUS                  Status;
  EFI_VGA_MINI_PORT_PROTOCOL  *VgaMiniPort;
  PCI_VGA_MINI_PORT_DEV       *PciVgaMiniPortPrivate;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiVgaMiniPortProtocolGuid,  
                  (VOID **)&VgaMiniPort,
                  This->DriverBindingHandle,             
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  PciVgaMiniPortPrivate = PCI_VGA_MINI_PORT_DEV_FROM_THIS (VgaMiniPort);

  Status = gBS->UninstallProtocolInterface (
                  Controller, 
                  &gEfiVgaMiniPortProtocolGuid, &PciVgaMiniPortPrivate->VgaMiniPort
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
    
  gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle, 
         Controller
         );
    
  gBS->FreePool (PciVgaMiniPortPrivate);

  return EFI_SUCCESS;
}

//
// VGA Mini Port Protocol Functions
//

EFI_STATUS 
EFIAPI
PciVgaMiniPortSetMode (
  IN  EFI_VGA_MINI_PORT_PROTOCOL  *This,
  IN  UINTN                       ModeNumber
  )
{
  if (ModeNumber > This->MaxMode) {
    return EFI_UNSUPPORTED;
  }
  return EFI_SUCCESS;
}

