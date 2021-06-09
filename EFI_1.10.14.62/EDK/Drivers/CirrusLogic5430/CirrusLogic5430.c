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

  CirrusLogic5430.c
    
Abstract:

  Cirrus Logic 5430 Controller Driver.
  This driver is a sample implementation of the UGA Draw Protocol for the
  Cirrus Logic 5430 family of PCI video controllers.  This driver is only
  usable in the EFI pre-boot environment.  This sample is intended to show
  how the UGA Draw Protocol is able to function.  The UGA I/O Protocol is not
  implemented in this sample.  A fully compliant EFI UGA driver requires both
  the UGA Draw and the UGA I/O Protocol.  Please refer to Microsoft's
  documentation on UGA for details on how to write a UGA driver that is able
  to function both in the EFI pre-boot environment and from the OS runtime.

Revision History:

--*/

//
// Cirrus Logic 5430 Controller Driver
//

#include "CirrusLogic5430.h"

EFI_DRIVER_BINDING_PROTOCOL gCirrusLogic5430DriverBinding = {
  CirrusLogic5430ControllerDriverSupported,
  CirrusLogic5430ControllerDriverStart,
  CirrusLogic5430ControllerDriverStop,
  0x10,
  NULL,
  NULL
};

//
// Cirrus Logic 5430 Driver Entry point
//

EFI_DRIVER_ENTRY_POINT(CirrusLogic5430UgaDrawDriverEntryPoint)

EFI_STATUS
CirrusLogic5430UgaDrawDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
                
/*++

Routine Description:

Arguments:

Returns:

    None

--*/

{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gCirrusLogic5430DriverBinding,
           ImageHandle,
           &gCirrusLogic5430ComponentName,
           NULL,
           NULL
           );
}

EFI_STATUS
CirrusLogic5430ControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

    None

--*/

{
  EFI_STATUS           Status; 
  EFI_PCI_IO_PROTOCOL  *PciIo;
  PCI_TYPE00           Pci;

  //
  // Open the PCI I/O Protocol
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
  // Read the PCI Configuration Header from the PCI Device
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
  // See if the I/O enable is on.  Most systems only allow one VGA device to be turned on
  // at a time, so see if this is one that is turned on.
  //
//  if (((Pci.Hdr.Command & 0x01) == 0x01)) {
    //
    // See if this is a Cirrus Logic PCI controller
    //
    if (Pci.Hdr.VendorId == CIRRUS_LOGIC_VENDOR_ID) {
      //
      // See if this is a 5430 or a 5446 PCI controller
      //
      if (Pci.Hdr.DeviceId == CIRRUS_LOGIC_5430_DEVICE_ID) {
        Status = EFI_SUCCESS;
      }
      if (Pci.Hdr.DeviceId == CIRRUS_LOGIC_5430_ALTERNATE_DEVICE_ID) {
        Status = EFI_SUCCESS;
      }
      if (Pci.Hdr.DeviceId == CIRRUS_LOGIC_5446_DEVICE_ID) {
        Status = EFI_SUCCESS;
      }
    }
//  }

Done:
  //
  // Close the PCI I/O Protocol
  //
  gBS->CloseProtocol (
         Controller,       
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
  
  return Status;
}

EFI_STATUS
CirrusLogic5430ControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

    None

--*/

{
  EFI_STATUS                      Status;
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private;

  //
  // Allocate Private context data for UGA Draw inteface.
  //
  Private = NULL;
  Status = gBS->AllocatePool (
                  EfiBootServicesData, 
                  sizeof(CIRRUS_LOGIC_5430_PRIVATE_DATA), 
                  (VOID **)&Private
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  EfiZeroMem (Private, sizeof (Private));

  //
  // Set up context record
  //
  Private->Signature = CIRRUS_LOGIC_5430_PRIVATE_DATA_SIGNATURE;
  Private->Handle    = Controller;

  //
  // Open PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                      Private->Handle, 
                      &gEfiPciIoProtocolGuid, 
                      (VOID **)&Private->PciIo,
                      This->DriverBindingHandle,   
                      Private->Handle,   
                      EFI_OPEN_PROTOCOL_BY_DRIVER 
                      );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  Status = Private->PciIo->Attributes ( 
                             Private->PciIo, 
                             EfiPciIoAttributeOperationEnable, 
                             EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                             NULL
                             );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // Start the UGA Draw software stack.
  //
  Status = CirrusLogic5430UgaDrawConstructor (Private);
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // Publish the UGA Draw interface to the world
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,         
                  &gEfiUgaDrawProtocolGuid, &Private->UgaDraw,
                  NULL
                  );

Error:
  if (EFI_ERROR (Status)) {
    if (Private) {
      if (Private->PciIo) {
        Private->PciIo->Attributes ( 
                          Private->PciIo, 
                          EfiPciIoAttributeOperationDisable, 
                          EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                          NULL
                          );
      }
    }

    //
    // Close the PCI I/O Protocol
    //
    gBS->CloseProtocol (
           Private->Handle, 
           &gEfiPciIoProtocolGuid, 
           This->DriverBindingHandle, 
           Private->Handle
           );
    if (Private) {
      gBS->FreePool (Private);
    }
  }
  return Status;
}

EFI_STATUS
CirrusLogic5430ControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:

Returns:

    None

--*/

{
  EFI_UGA_DRAW_PROTOCOL           *UgaDraw;
  EFI_STATUS                      Status;
  CIRRUS_LOGIC_5430_PRIVATE_DATA  *Private;

  Status = gBS->OpenProtocol (
                  Controller,           
                  &gEfiUgaDrawProtocolGuid,  
                  (VOID **)&UgaDraw,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    //
    // If the UGA Draw interface does not exist the driver is not started
    //
    return Status;
  }

  //
  // Get our private context information
  //
  Private = CIRRUS_LOGIC_5430_PRIVATE_DATA_FROM_UGA_DRAW_THIS (UgaDraw);

  //
  // Remove the UGA Draw interface from the system
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Private->Handle, 
                  &gEfiUgaDrawProtocolGuid, &Private->UgaDraw,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Shutdown the hardware
  //
  CirrusLogic5430UgaDrawDestructor (Private);

  Private->PciIo->Attributes ( 
                    Private->PciIo, 
                    EfiPciIoAttributeOperationDisable, 
                    EFI_PCI_DEVICE_ENABLE | EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY | EFI_PCI_IO_ATTRIBUTE_VGA_IO,
                    NULL
                    );

  //
  // Close the PCI I/O Protocol
  //
  gBS->CloseProtocol (
         Controller, 
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle, 
         Controller
         );

  //
  // Free our instance data
  //
  gBS->FreePool (Private);

  return EFI_SUCCESS;
}

