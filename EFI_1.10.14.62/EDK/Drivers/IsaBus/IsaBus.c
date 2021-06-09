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

  IsaBus.c
  
Abstract:

  Discovers all the ISA Controllers and their resources by using the ISA PnP 
  Protocol, produces an instance of the ISA I/O Protocol for every ISA 
  Controller found, loads and initializes all ISA Device Drivers, matches ISA
  Device Drivers with their respective ISA Controllers in a deterministic 
  manner, and informs a ISA Device Driver when it is to start managing an ISA
  Controller. 

Revision History:

--*/

#include "IsaBus.h"

//
// ISA Bus Driver Global Variables
//
EFI_DRIVER_BINDING_PROTOCOL gIsaBusControllerDriver = {
  IsaBusControllerDriverSupported,
  IsaBusControllerDriverStart,
  IsaBusControllerDriverStop,
  0x10,
  NULL,
  NULL
};

//
// ISA Bus Driver Entry Point
//
EFI_DRIVER_ENTRY_POINT(IsaBusControllerDriverEntryPoint)

EFI_STATUS
IsaBusControllerDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

  Routine Description:
    The entry point for an ISA Bus Driver simply allocates space for an  
    EFI_DRIVER_BINDING_PROTOCOL protocol instance, initializes its contents, 
    and attaches this protocol instance to the image handle of the ISA Bus
    Driver. It also attaches the bus identifier protocol to the same handle.  
    For an ISA Bus Driver for a PCI to ISA Bridge controller, 
    this would be the EFI_PCI_BUS_IDENTIFIER_PROTOCOL.  
  
  Arguments:
    ImageHandle   EFI_HANDLE: The firmware allocated handle for 
                  the EFI Driver(PCI/ISA Bridge) image.
    SystemTable   EFI_SYSTEM_TABLE: A pointer to the EFI system table        

  Returns:
    EFI_SUCCESS:            The ISA bus driver was initialized.
    EFI_ALREADY_STARTED:    The driver has already been initialized.
    EFI_INVALID_PARAMETER:  One of the parameters has an invalid value.
    EFI_OUT_OF_RESOURCES:   The request could not be completed due to 
                            a lack of resources.
 
--*/          
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gIsaBusControllerDriver, 
           ImageHandle,
           &gIsaBusComponentName,
           NULL,
           NULL
           );
}

EFI_STATUS
IsaBusControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

  Routine Description:
    This function checks to see if a controller can be managed by the ISA Bus 
    Driver. This is done by checking to see if the controller supports the 
    EFI_PCI_IO_PROTOCOL protocol, and then looking at the PCI Configuration 
    Header to see if the device is a PCI to ISA bridge. The class code of 
    PCI to ISA bridge: Base class 06h, Sub class 01h Interface 00h 
  
  Arguments:
    EFI_DRIVER_BINDING_PROTOCOL *This The EFI_DRIVER_BINDING_PROTOCOL instance.
    EFI_HANDLE                  Controller    The handle of the device to check.

  Returns:
    EFI_SUCCESS:   The device is supported by this driver.
    EFI_UNSUPPORTED: The device is not supported by this driver.

--*/  
{
  EFI_STATUS                      Status;
  EFI_DEVICE_PATH_PROTOCOL        *ParentDevicePath;
  EFI_ISA_ACPI_PROTOCOL           *IsaAcpi;
  EFI_DEV_PATH_PTR                Node;
  
  if (RemainingDevicePath != NULL) {
    Node.DevPath = RemainingDevicePath;
    if (Node.DevPath->Type != ACPI_DEVICE_PATH ||
        Node.DevPath->SubType != ACPI_DP ||
        DevicePathNodeLength(Node.DevPath) != sizeof(ACPI_HID_DEVICE_PATH)) {
      return EFI_UNSUPPORTED;
    }
  }

  //
  // Test the existence of EFI_DEVICE_PATH protocol
  //
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
  if (Status != EFI_ALREADY_STARTED) {
    gBS->CloseProtocol (
         Controller,
         &gEfiDevicePathProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );
  }

  //
  // Get the Isa Acpi protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,       
                  &gEfiIsaAcpiProtocolGuid, 
                  (VOID **)&IsaAcpi,
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
  
  gBS->CloseProtocol (
         Controller,       
         &gEfiIsaAcpiProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
  
  return Status;
}

EFI_STATUS
IsaBusControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

  Routine Description:
    This function tells the ISA Bus Driver to start managing a PCI to ISA 
    Bridge controller. 
  
  Arguments:
    EFI_DRIVER_BINDING_PROTOCOL *This The EFI_DRIVER_BINDING_PROTOCOL instance.
    EFI_HANDLE                  Controller A handle to the device being started. 

  Returns:
    EFI_SUCCESS:          The device was started.
    EFI_UNSUPPORTED:      The device is not supported.
    EFI_DEVICE_ERROR:     The device could not be started due to a device error.
    EFI_ALREADY_STARTED:  The device has already been started.
    EFI_INVALID_PARAMETER:One of the parameters has an invalid value.
    EFI_OUT_OF_RESOURCES: The request could not be completed due to a lack of 
                          resources.
  
--*/  
{
  EFI_STATUS                  Status;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath;
  EFI_ISA_ACPI_PROTOCOL       *IsaAcpi;
  EFI_ISA_ACPI_DEVICE_ID      *IsaDevice;
  EFI_ISA_ACPI_RESOURCE_LIST  *ResourceList;
  
  //
  // Open Device Path Protocol
  //
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
  // Open Pci IO Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,       
                  &gEfiPciIoProtocolGuid, 
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    //
    // Close opened protocol
    //
    gBS->CloseProtocol (
         Controller,       
         &gEfiDevicePathProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    return Status;
  }

  //
  // Open ISA Acpi Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,       
                  &gEfiIsaAcpiProtocolGuid, 
                  (VOID **)&IsaAcpi,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER 
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    //
    // Close opened protocol
    //
    gBS->CloseProtocol (
         Controller,       
         &gEfiDevicePathProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    gBS->CloseProtocol (
         Controller,       
         &gEfiPciIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    return Status;
  }
  
  //
  // first init ISA interface
  //
  IsaAcpi->InterfaceInit (IsaAcpi);
  
  //
  // Create each ISA device handle in this ISA bus
  //
  IsaDevice = NULL;
  do {
    Status = IsaAcpi->DeviceEnumerate (IsaAcpi, &IsaDevice);
    if (EFI_ERROR (Status)) {
      break;
    }

    //
    // Get current resource of this ISA device
    //
    ResourceList = NULL;
    Status = IsaAcpi->GetCurResource(IsaAcpi, IsaDevice, &ResourceList);
    if (EFI_ERROR (Status)) {
      continue;
    }

    //
    // Create handle for this ISA device
    //
    Status = IsaCreateDevice (
                This, 
                Controller,
                PciIo, 
                ParentDevicePath, 
                ResourceList
                );
                              
    if (EFI_ERROR (Status)) {
      continue;
    }

    //
    // Initialize ISA device
    //
    IsaAcpi->InitDevice(IsaAcpi, IsaDevice);

    //
    // Set resources for this ISA device
    //
    IsaAcpi->SetResource (IsaAcpi, IsaDevice, ResourceList);

    //
    // Set power for this ISA device
    //
    IsaAcpi->SetPower (IsaAcpi, IsaDevice, TRUE);

    //
    // Enable this ISA device
    //
    IsaAcpi->EnableDevice (IsaAcpi, IsaDevice, TRUE);

  } while (TRUE);
 
  return EFI_SUCCESS;
}

EFI_STATUS
IsaBusControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

  Routine Description:
    This function tells the ISA Bus Driver to stop managing a PCI to ISA 
    Bridge controller. 
     
  Arguments:
    EFI_DRIVER_BINDING_PROTOCOL *This The EFI_DRIVER_BINDING_PROTOCOL instance.
    EFI_HANDLE   Controller   A handle to the device being stopped.

  Returns:
    EFI_SUCCESS  The device was stopped.
    EFI_DEVICE_ERROR The device could not be stopped due to a device error.
    EFI_NOT_STARTED  The device has not been started.
    EFI_INVALID_PARAMETER  One of the parameters has an invalid value.
    EFI_OUT_OF_RESOURCES The request could not be completed due to a lack of 
                         resources.

--*/
{
  EFI_STATUS           Status;
  UINTN                Index;
  BOOLEAN              AllChildrenStopped;
  ISA_IO_DEVICE        *IsaIoDevice;
  EFI_ISA_IO_PROTOCOL  *IsaIo;
  EFI_PCI_IO_PROTOCOL  *PciIo;

  if (NumberOfChildren == 0) {
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
           
    gBS->CloseProtocol (
           Controller, 
           &gEfiIsaAcpiProtocolGuid, 
           This->DriverBindingHandle, 
           Controller
           );      

    return EFI_SUCCESS;
  }

  //
  // Complete all outstanding transactions to Controller.
  // Don't allow any new transaction to Controller to be started.
  //

  //
    // Stop all the children
  // Find all the ISA devices that were discovered on this PCI to ISA Bridge 
  // with the Start() function.
  //

  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {

    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index],   
                    &gEfiIsaIoProtocolGuid,  
                    (VOID **)&IsaIo,
                    This->DriverBindingHandle,             
                    Controller,   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {

      IsaIoDevice = ISA_IO_DEVICE_FROM_ISA_IO_THIS(IsaIo);

      //
      // Close the child handle
      //
      gBS->CloseProtocol (
             Controller, 
             &gEfiPciIoProtocolGuid, 
             This->DriverBindingHandle, 
             ChildHandleBuffer[Index]
             );

      Status = gBS->UninstallMultipleProtocolInterfaces (
                 ChildHandleBuffer[Index], 
                 &gEfiDevicePathProtocolGuid,      IsaIoDevice->DevicePath,
                 &gEfiIsaIoProtocolGuid,           &IsaIoDevice->IsaIo,
                 NULL
                 );
      if (EFI_ERROR (Status)) {
        gBS->OpenProtocol (
               Controller,       
               &gEfiPciIoProtocolGuid, 
               (VOID **)&PciIo,
               This->DriverBindingHandle,   
               ChildHandleBuffer[Index],   
               EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
               );
      } else {
        gBS->FreePool (IsaIoDevice);
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

//
// Internal Function
//
EFI_STATUS
IsaCreateDevice (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_PCI_IO_PROTOCOL          *PciIo,
  IN EFI_DEVICE_PATH_PROTOCOL     *ParentDevicePath,
  IN EFI_ISA_ACPI_RESOURCE_LIST   *IsaDeviceResourceList
  )
/*++

  Routine Description:
    Create ISA device found by IsaPnpProtocol 

  Arguments:
    EFI_HANDLE Controller: The handle of ISA bus controller(PCI to ISA bridge)
    EFI_PCI_IO_PROTOCOL *PciIo: Used to implement EFI_ISA_IO_PROTOCOL
    EFI_DEVICE_PATH_PROTOCOL *ParentDevicePath: Device path of the ISA bus
                                                controller
    EFI_ISA_PNP_RESOURCE *IsaDevice: The resource list of the ISA device

  Returns:
    EFI_SUCCESS:   
    EFI_OUT_OF_RESOURCES: 

--*/
{
  EFI_STATUS     Status;
  ISA_IO_DEVICE  *IsaIoDevice;
  EFI_DEV_PATH   Node;

  //
  // Initialize the PCI_IO_DEVICE structure
  //
  IsaIoDevice = EfiLibAllocateZeroPool (sizeof (ISA_IO_DEVICE));
  if (IsaIoDevice == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  IsaIoDevice->Signature  = ISA_IO_DEVICE_SIGNATURE;
  IsaIoDevice->Handle     = NULL;
  IsaIoDevice->PciIo      = PciIo;

  //
  // Initialize the ISA I/O instance structure
  //
  Status = InitializeIsaIoInstance (IsaIoDevice, IsaDeviceResourceList);

  //
  // Build the child device path
  //
  Node.DevPath.Type    = ACPI_DEVICE_PATH;
  Node.DevPath.SubType = ACPI_DP;
  SetDevicePathNodeLength (&Node.DevPath, sizeof(ACPI_HID_DEVICE_PATH));
  Node.Acpi.HID = IsaDeviceResourceList->Device.HID;
  Node.Acpi.UID = IsaDeviceResourceList->Device.UID;

  IsaIoDevice->DevicePath = EfiAppendDevicePathNode (
                               ParentDevicePath, 
                               &Node.DevPath
                               );
  
  if (IsaIoDevice->DevicePath == NULL) {
    gBS->FreePool (IsaIoDevice);
    return EFI_DEVICE_ERROR;
  }                             

  //
  // Create a child handle and attach the DevicePath, 
  // PCI I/O, and Controller State
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &IsaIoDevice->Handle,              
                  &gEfiDevicePathProtocolGuid, IsaIoDevice->DevicePath,
                  &gEfiIsaIoProtocolGuid,      &IsaIoDevice->IsaIo,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (IsaIoDevice);
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Controller,       
                  &gEfiPciIoProtocolGuid, 
                  (VOID **)&PciIo,
                  This->DriverBindingHandle,   
                  IsaIoDevice->Handle,   
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  return Status;
}
