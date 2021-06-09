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

  BiosVgaMiniPort.c
    
Abstract:

  ConsoleOut Routines that speak VGA.

Revision History

--*/

#include "BiosVgaMiniPort.h"

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
EFI_DRIVER_BINDING_PROTOCOL gBiosVgaMiniPortDriverBinding = {
  BiosVgaMiniPortDriverBindingSupported,
  BiosVgaMiniPortDriverBindingStart,
  BiosVgaMiniPortDriverBindingStop,
  0x00000000,
  NULL,
  NULL
};  

//
// Driver Entry Point
//  

EFI_DRIVER_ENTRY_POINT(BiosVgaMiniPortDriverEntryPoint)
    
EFI_STATUS
BiosVgaMiniPortDriverEntryPoint(
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
           &gBiosVgaMiniPortDriverBinding,
           ImageHandle,
           &gBiosVgaMiniPortComponentName,
           NULL,
           NULL
           );
} 

EFI_STATUS
BiosVgaMiniPortDriverBindingSupported (
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
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
  if (EFI_ERROR (Status)) {
    Status = EFI_UNSUPPORTED;
    goto Done;
  }
  
  Status = EFI_UNSUPPORTED;
#ifdef SOFT_SDV
    if(Pci.Hdr.ClassCode[2] == 1 && Pci.Hdr.ClassCode[1] == 1) {
      Status = EFI_SUCCESS;
    }
#else
  if ((Pci.Hdr.Command & 0x03) == 0x03) {
    if (IS_PCI_VGA (&Pci)) {
      Status = EFI_SUCCESS;
    }
  }
#endif

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
BiosVgaMiniPortDriverBindingStart (
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
  BIOS_VGA_MINI_PORT_DEV  *BiosVgaMiniPortPrivate;

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
    return Status;
  }
      
  //
  // Allocate the private device structure
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(BIOS_VGA_MINI_PORT_DEV),
                  &BiosVgaMiniPortPrivate
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  EfiZeroMem (BiosVgaMiniPortPrivate, sizeof (BIOS_VGA_MINI_PORT_DEV)) ;
  
  //
  // Initialize the private device structure
  //
  BiosVgaMiniPortPrivate->Signature = BIOS_VGA_MINI_PORT_DEV_SIGNATURE;
  BiosVgaMiniPortPrivate->Handle    = Controller;
  BiosVgaMiniPortPrivate->PciIo     = PciIo;

  BiosVgaMiniPortPrivate->VgaMiniPort.SetMode                   = BiosVgaMiniPortSetMode;
  BiosVgaMiniPortPrivate->VgaMiniPort.VgaMemoryOffset           = 0xb8000;
  BiosVgaMiniPortPrivate->VgaMiniPort.CrtcAddressRegisterOffset = 0x3d4;
  BiosVgaMiniPortPrivate->VgaMiniPort.CrtcDataRegisterOffset    = 0x3d5;
  BiosVgaMiniPortPrivate->VgaMiniPort.VgaMemoryBar              = EFI_PCI_IO_PASS_THROUGH_BAR;
  BiosVgaMiniPortPrivate->VgaMiniPort.CrtcAddressRegisterBar    = EFI_PCI_IO_PASS_THROUGH_BAR;
  BiosVgaMiniPortPrivate->VgaMiniPort.CrtcDataRegisterBar       = EFI_PCI_IO_PASS_THROUGH_BAR;

  if (Int86Available()) {
    BiosVgaMiniPortPrivate->Int86Available                      = TRUE;
    BiosVgaMiniPortPrivate->VgaMiniPort.MaxMode                 = 2;
  } else {
    BiosVgaMiniPortPrivate->Int86Available                      = FALSE;
    BiosVgaMiniPortPrivate->VgaMiniPort.MaxMode                 = 1;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiVgaMiniPortProtocolGuid, &BiosVgaMiniPortPrivate->VgaMiniPort,
                  NULL
                  );

  return Status;
}

EFI_STATUS
BiosVgaMiniPortDriverBindingStop (
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
  BIOS_VGA_MINI_PORT_DEV       *BiosVgaMiniPortPrivate;

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

  BiosVgaMiniPortPrivate = BIOS_VGA_MINI_PORT_DEV_FROM_THIS (VgaMiniPort);

  Status = gBS->UninstallProtocolInterface (
                  Controller, 
                  &gEfiVgaMiniPortProtocolGuid, &BiosVgaMiniPortPrivate->VgaMiniPort
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
    
  //
  // Release PCI I/O and VGA Mini Port Protocols on the controller handle.
  //
  gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle, 
         Controller
         );
    
  gBS->FreePool (BiosVgaMiniPortPrivate);

  return EFI_SUCCESS;
}

//
// BIOS VGA Mini Port Protocol Functions
//
EFI_STATUS 
EFIAPI
BiosVgaMiniPortSetMode (
  IN  EFI_VGA_MINI_PORT_PROTOCOL  *This,
  IN  UINTN                       ModeNumber
  )
{
  BIOS_VGA_MINI_PORT_DEV  *BiosVgaMiniPortPrivate;
  IA32_RegisterSet_t      Regs;

  //
  // Make sure the ModeNumber is a valid value
  //
  if (ModeNumber >= This->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  //
  // Get the device structure for this device
  //
  BiosVgaMiniPortPrivate = BIOS_VGA_MINI_PORT_DEV_FROM_THIS (This);

  //
  // If INT calls are not available, then only 1 mode is supported, so just return.
  //
  if (!BiosVgaMiniPortPrivate->Int86Available) {
    return EFI_SUCCESS;
  }

  switch (ModeNumber) {
  case 0:
    //
    // Set the 80x25 Text VGA Mode
    //
    Regs.h.AH = 0x00;
    Regs.h.AL = 0x83;
    Int86(0x10, &Regs);

    Regs.h.AH = 0x11;
    Regs.h.AL = 0x14;
    Regs.h.BL = 0;
    Int86(0x10, &Regs);
    break;
  case 1:
    //
    // Set the 80x50 Text VGA Mode
    //
    Regs.h.AH = 0x00;
    Regs.h.AL = 0x83;
    Int86(0x10, &Regs);

    Regs.h.AH = 0x11;
    Regs.h.AL = 0x12;
    Regs.h.BL = 0;
    Int86(0x10, &Regs);
    break;
  default:
    return EFI_UNSUPPORTED;
  }
  return EFI_SUCCESS;
}






