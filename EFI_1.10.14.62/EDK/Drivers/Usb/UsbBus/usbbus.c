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

    UsbBus.c

  Abstract:

    USB Bus Driver

  Revision History

--*/

//
// USB Bus Controller Driver
//
#include "Efi.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(DriverBinding)
#include EFI_PROTOCOL_DEFINITION(UsbIo)
#include EFI_PROTOCOL_DEFINITION(DevicePath)

#include "usb.h"
#include EFI_PROTOCOL_DEFINITION(UsbHostController)
#include "usbbus.h"
#include "usblib.h"
#include "usbutil.h"
#include "hub.h"

//
// USB bus entry point
//
EFI_STATUS
UsbBusDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// EFI_DRIVER_BINDING_PROTOCOL Protocol Interface
//
EFI_STATUS
UsbBusControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
UsbBusControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
UsbBusControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
  );


//
// Supported function
//
VOID
InitializeUsbIoInstance (
  IN USB_IO_CONTROLLER_DEVICE     *UsbIoController
  );


STATIC
USB_IO_CONTROLLER_DEVICE *
CreateUsbIoControllerDevice (
  VOID
);

STATIC
EFI_STATUS
InitUsbIoController (
  IN USB_IO_CONTROLLER_DEVICE     *UsbIoController
  );

//
// USB Device Configuration / Deconfiguration
//
STATIC
EFI_STATUS
UsbDeviceConfiguration(
  IN  USB_IO_CONTROLLER_DEVICE    *ParentHubController,
  IN  EFI_HANDLE                  HostController,
  IN  UINT8                       ParentPort,
  IN  USB_IO_DEVICE               *UsbIoDevice
  );

STATIC
EFI_STATUS
UsbDeviceDeConfiguration(
  IN  USB_IO_DEVICE  *UsbIoDevice
  );

//
// Usb Bus enumeration function
//
STATIC
VOID
UsbEnumeration (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  );

EFI_DRIVER_ENTRY_POINT(UsbBusDriverEntryPoint)


EFI_DRIVER_BINDING_PROTOCOL gUsbBusDriverBinding = {
  UsbBusControllerDriverSupported,
  UsbBusControllerDriverStart,
  UsbBusControllerDriverStop,
  0x10,
  NULL,
  NULL
};

VOID
ResetRootPort(
  IN UINT8                  PortNum,
  IN EFI_USB_HC_PROTOCOL    *UsbHCInterface
  );

VOID
ClearRootPortConnectionChangeStatus (
  IN UINT8                  PortNum,
  IN EFI_USB_HC_PROTOCOL    *UsbHCInterface
);

STATIC
EFI_STATUS
ParentPortReset (
  IN USB_IO_CONTROLLER_DEVICE    *UsbIoController,
  IN BOOLEAN                     ReConfigure
  );

//
// Following are address allocate and free functions
//
STATIC
UINT8 UsbAllocateAddress(
  IN UINT8    *AddressPool
)
{
  UINT8 ByteIndex;
  UINT8 BitIndex;

  for (ByteIndex = 0; ByteIndex < 16 ; ByteIndex ++) {
    for (BitIndex = 0; BitIndex < 8 ; BitIndex ++) {
      if ((AddressPool[ByteIndex] & (1 << BitIndex)) == 0) {
         //
         // Found one, covert to address, and mark it use
         //
         AddressPool[ByteIndex] |= (1 << BitIndex);
         return (UINT8)(ByteIndex * 8 + BitIndex);
      }
    }
  }

  return 0;

}

STATIC
VOID UsbFreeAddress(
  IN UINT8    DevAddress,
  IN UINT8    *AddressPool
)
{
  UINT8 WhichByte;
  UINT8 WhichBit;

  //
  // Locate the position
  // 
  WhichByte = (UINT8)(DevAddress / 8);
  WhichBit = (UINT8)(DevAddress % 8);

  AddressPool[WhichByte] &= (~(1 << WhichBit));
}

#define MICROSECOND     10000
#define ONESECOND       (1000 * MICROSECOND)

//
// USB Bus Driver Entry point
//
EFI_STATUS
UsbBusDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

  Routine Description:
    Entry point for EFI drivers.

  Arguments:
    (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

  Returns:
    EFI_SUCCESS
    others

--*/

{
  EFI_STATUS  Status;

  //
  // Install driver binding protocol
  //
  Status = EfiLibInstallAllDriverProtocols (
            ImageHandle, 
            SystemTable, 
            &gUsbBusDriverBinding, 
            ImageHandle,
            &gUsbBusComponentName,
            NULL,
            NULL
           );

  return Status;

}


EFI_STATUS
UsbBusControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

  Routine Description:
    Test to see if this driver supports ControllerHandle. Any ControllerHandle
    that has UsbHcProtocol installed will be supported.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to test
    RemainingDevicePath - Not used

  Returns:
    EFI_SUCCESS         - This driver supports this device.
    EFI_UNSUPPORTED     - This driver does not support this device.

--*/
{
  EFI_STATUS                 OpenStatus;

  //
  // Check whether USB Host Controller Protocol is already
  // installed on this handle. If it is installed, we can start
  // USB Bus Driver now.
  //
  OpenStatus = gBS->OpenProtocol (
                      Controller,
                      &gEfiUsbHcProtocolGuid,
                      NULL,
                      This->DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                 );

  if (EFI_ERROR (OpenStatus) && (OpenStatus != EFI_ALREADY_STARTED)) {
    return EFI_UNSUPPORTED;
  } 

  return OpenStatus;
}

//
// The UsbBusProtocol is just used to locate USB_BUS_CONTROLLER
// structure in the UsbBusDriverControllerDriverStop(). Then we can
// Close all opened protocols and release this structure.
//
STATIC EFI_GUID mUsbBusProtocolGuid = EFI_USB_BUS_PROTOCOL_GUID;

EFI_STATUS
UsbBusControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

  Routine Description:
    Starting the Usb Bus Driver

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to test
    RemainingDevicePath - Not used

  Returns:
    EFI_SUCCESS         - This driver supports this device.
    EFI_UNSUPPORTED     - This driver does not support this device.
    EFI_DEVICE_ERROR    - This driver cannot be started due to device
                          Error
    EFI_OUT_OF_RESOURCES

--*/
{
  EFI_STATUS                     Status;
  EFI_STATUS                     OpenStatus;
  USB_BUS_CONTROLLER_DEVICE      *UsbBusDev;
  USB_IO_DEVICE                  *RootHub;
  USB_IO_CONTROLLER_DEVICE       *RootHubController;
  EFI_USB_HC_PROTOCOL            *UsbHCInterface;
  EFI_TPL                        OldTPL;

  OldTPL = gBS->RaiseTPL(EFI_TPL_CALLBACK);

  //
  // Allocate USB_BUS_CONTROLLER_DEVICE structure
  //
  UsbBusDev = NULL;
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(USB_BUS_CONTROLLER_DEVICE),
                  (VOID **)&UsbBusDev
                );

  if (EFI_ERROR(Status)) {
    gBS->RestoreTPL(OldTPL);
    return EFI_OUT_OF_RESOURCES;
  }

  EfiZeroMem(UsbBusDev, sizeof(USB_BUS_CONTROLLER_DEVICE));

  UsbBusDev->Signature = USB_BUS_DEVICE_SIGNATURE;
  UsbBusDev->AddressPool[0] = 1;

  //
  // Get the Device Path Protocol on Controller's handle
  //
  OpenStatus = gBS->OpenProtocol(
                      Controller,
                      &gEfiDevicePathProtocolGuid,
                      (VOID **)&UsbBusDev->BusControllerDevicePath,
                      This->DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_BY_DRIVER
                    );

  if (EFI_ERROR(OpenStatus)) {
    gBS->FreePool(UsbBusDev);
    gBS->RestoreTPL(OldTPL);
    return EFI_UNSUPPORTED;
  }

  //
  // Locate the Host Controller Interface
  //
  OpenStatus = gBS->OpenProtocol (
                      Controller,
                      &gEfiUsbHcProtocolGuid,
                      (VOID **)&UsbHCInterface,
                      This->DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_BY_DRIVER
                   );

  if (EFI_ERROR(OpenStatus) && (OpenStatus != EFI_ALREADY_STARTED)) {
    gBS->CloseProtocol(
            Controller,
            &gEfiDevicePathProtocolGuid,
            This->DriverBindingHandle,
            Controller
         );
    gBS->FreePool(UsbBusDev);
    gBS->RestoreTPL(OldTPL);
    return EFI_UNSUPPORTED;
  } 
  
  if (OpenStatus == EFI_ALREADY_STARTED) {
    gBS->CloseProtocol(
            Controller,
            &gEfiDevicePathProtocolGuid,
            This->DriverBindingHandle,
            Controller
         );
    gBS->FreePool(UsbBusDev);
    gBS->RestoreTPL(OldTPL);
    return EFI_ALREADY_STARTED;
  }

  UsbBusDev->UsbHCInterface = UsbHCInterface;

  //
  // Attach EFI_USB_BUS_PROTOCOL to controller handle,
  // for locate UsbBusDev later
  //
  Status = gBS->InstallProtocolInterface(
                  &Controller,
                  &mUsbBusProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &UsbBusDev->BusIdentify
                );

  if (EFI_ERROR(Status)) {
    gBS->CloseProtocol(
            Controller,
            &gEfiDevicePathProtocolGuid,
            This->DriverBindingHandle,
            Controller
         );
    gBS->CloseProtocol(
            Controller,
            &gEfiUsbHcProtocolGuid,
            This->DriverBindingHandle,
            Controller
         );
    gBS->FreePool(UsbBusDev);
    gBS->RestoreTPL(OldTPL);
    return Status;
  }

  //
  // Add root hub to the tree
  //
  RootHub = NULL;
  Status = gBS->AllocatePool(
                 EfiBootServicesData,
                 sizeof (USB_IO_DEVICE),
                 (VOID **)&RootHub
                );

  if (EFI_ERROR(Status)) {
     gBS->UninstallProtocolInterface(
            Controller,
            &mUsbBusProtocolGuid,
            &UsbBusDev->BusIdentify
          );
    gBS->CloseProtocol(
            Controller,
            &gEfiDevicePathProtocolGuid,
            This->DriverBindingHandle,
            Controller
         );
    gBS->CloseProtocol(
            Controller,
            &gEfiUsbHcProtocolGuid,
            This->DriverBindingHandle,
            Controller
         );
    gBS->FreePool(UsbBusDev);
    gBS->RestoreTPL(OldTPL);
    return EFI_OUT_OF_RESOURCES;
  }

  EfiZeroMem(RootHub, sizeof(USB_IO_DEVICE));

  RootHub->BusController  = UsbBusDev;
  RootHub->DeviceAddress  = UsbAllocateAddress(UsbBusDev->AddressPool);

  UsbBusDev->Root = RootHub;

  //
  // Allocate Root Hub Controller
  //
  RootHubController = CreateUsbIoControllerDevice();
  if(RootHubController == NULL) {
     gBS->UninstallProtocolInterface(
            Controller,
            &mUsbBusProtocolGuid,
            &UsbBusDev->BusIdentify
          );
    gBS->CloseProtocol(
            Controller,
            &gEfiDevicePathProtocolGuid,
            This->DriverBindingHandle,
            Controller
         );
    gBS->CloseProtocol(
            Controller,
            &gEfiUsbHcProtocolGuid,
            This->DriverBindingHandle,
            Controller
         );
    gBS->FreePool(UsbBusDev);
    gBS->FreePool(RootHub);
    gBS->RestoreTPL(OldTPL);
    return EFI_OUT_OF_RESOURCES;
  }

  UsbHCInterface->GetRootHubPortNumber (
                    UsbHCInterface,
                    &RootHubController->DownstreamPorts
                  );
  RootHubController->UsbDevice      = RootHub;
  RootHubController->IsUsbHub       = TRUE;
  RootHubController->DevicePath     = UsbBusDev->BusControllerDevicePath;
  RootHubController->HostController = Controller;

  RootHub->NumOfControllers = 1;
  RootHub->UsbController[0] = RootHubController;

  //
  // Create a timer to query root ports periodically
  //
  Status = gBS->CreateEvent(
                  EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_CALLBACK,
                  UsbEnumeration,
                  RootHubController,
                  &RootHubController->HubNotify
                );
  if (EFI_ERROR(Status)) {
    gBS->UninstallProtocolInterface(
           Controller,
           &mUsbBusProtocolGuid,
           &UsbBusDev->BusIdentify
         );

    gBS->CloseProtocol(
           Controller,
           &gEfiDevicePathProtocolGuid,
           This->DriverBindingHandle,
           Controller
         );

    gBS->CloseProtocol(
           Controller,
           &gEfiUsbHcProtocolGuid,
           This->DriverBindingHandle,
           Controller
         );

    gBS->FreePool(RootHubController);
    gBS->FreePool(RootHub);
    gBS->FreePool(UsbBusDev);
    gBS->RestoreTPL(OldTPL);

    return EFI_UNSUPPORTED;
  }

  Status = gBS->SetTimer(
                  RootHubController->HubNotify,
                  TimerPeriodic,
                  ONESECOND
                );
  if (EFI_ERROR(Status)) {
     gBS->UninstallProtocolInterface(
            Controller,
            &mUsbBusProtocolGuid,
            &UsbBusDev->BusIdentify
          );

     gBS->CloseProtocol(
            Controller,
            &gEfiDevicePathProtocolGuid,
            This->DriverBindingHandle,
            Controller
          );

    gBS->CloseProtocol(
            Controller,
            &gEfiUsbHcProtocolGuid,
            This->DriverBindingHandle,
            Controller
          );

    gBS->CloseEvent(RootHubController->HubNotify);
    gBS->FreePool(RootHubController);
    gBS->FreePool(RootHub);
    gBS->FreePool(UsbBusDev);
    gBS->RestoreTPL(OldTPL);
    return EFI_UNSUPPORTED;
  }

  //
  // Reset USB Host Controller
  //
  UsbHCInterface->Reset (
                    UsbHCInterface,
                    EFI_USB_HC_RESET_GLOBAL
                  );

  //
  // Start USB Host Controller
  //
  UsbHCInterface->SetState (
                    UsbHCInterface,
                    EfiUsbHcStateOperational
                  );

  gBS->RestoreTPL(OldTPL);

  return EFI_SUCCESS;
}

//
// Stop the bus controller
//
EFI_STATUS
UsbBusControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
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
    EFI_SUCCES
    others

--*/
{
  EFI_STATUS                  Status;
  USB_IO_DEVICE               *Root;
  USB_IO_CONTROLLER_DEVICE    *RootHubController;
  USB_BUS_CONTROLLER_DEVICE   *UsbBusController;
  EFI_USB_BUS_PROTOCOL        *UsbIdentifier;
  UINT8                       i;
  EFI_USB_HC_PROTOCOL         *UsbHCInterface;
  USB_IO_CONTROLLER_DEVICE    *UsbController;
  USB_IO_DEVICE               *UsbIoDevice;
  USB_IO_CONTROLLER_DEVICE    *HubController;
  UINTN                       Index;
  BOOLEAN                     AllChildrenStopped;
  EFI_USB_IO_PROTOCOL         *UsbIo;

  if (NumberOfChildren > 0) {
    
    AllChildrenStopped = TRUE;
    
    for (Index = 0; Index < NumberOfChildren; Index++) {
      Status = gBS->OpenProtocol (
                  ChildHandleBuffer[Index],
                  &gEfiUsbIoProtocolGuid,  
                  (VOID**)&UsbIo,
                  This->DriverBindingHandle,             
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
      if (EFI_ERROR(Status)) {
        AllChildrenStopped = FALSE;
      }
    }

    for (Index = 0; Index < NumberOfChildren; Index++) {
      Status = gBS->OpenProtocol (
                  ChildHandleBuffer[Index],
                  &gEfiUsbIoProtocolGuid,  
                  (VOID**)&UsbIo,
                  This->DriverBindingHandle,             
                  Controller,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
      if (EFI_ERROR(Status)) {
        continue;
      }
      UsbController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS (UsbIo);
      UsbIoDevice = UsbController->UsbDevice;
      HubController =  UsbController->Parent;
      UsbDeviceDeConfiguration(UsbIoDevice);
      for (i = 0; i < HubController->DownstreamPorts; i ++) {
        if (HubController->Children[i] == UsbIoDevice) {
          HubController->Children[i] = NULL;
        }
      }
    }
    if (!AllChildrenStopped) {
      return EFI_DEVICE_ERROR;
    }
    return EFI_SUCCESS;
  }

  //
  // Get the USB_BUS_CONTROLLER_DEVICE
  //
  Status = gBS->OpenProtocol(
                  Controller,
                  &mUsbBusProtocolGuid,
                  (VOID **)&UsbIdentifier,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );

  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  UsbBusController = USB_BUS_CONTROLLER_DEVICE_FROM_THIS(UsbIdentifier);

  //
  // Stop USB Host Controller
  //
  UsbHCInterface = UsbBusController->UsbHCInterface;

  UsbHCInterface->SetState (
                    UsbHCInterface,
                    EfiUsbHcStateHalt
                  );

  //
  // Deconfiguration all its devices
  //
  Root = UsbBusController->Root;
  RootHubController = Root->UsbController[0];
  
//  gBS->SetTimer (
//        RootHubController->HubNotify,
//        TimerCancel, 
//        0
//       );

  gBS->CloseEvent (RootHubController->HubNotify);
  
  for (i = 0; i < RootHubController->DownstreamPorts; i++) {
    if (RootHubController->Children[i]) {
      UsbDeviceDeConfiguration(RootHubController->Children[i]);
      RootHubController->Children[i] = NULL;
    }
  }  
  
  gBS->FreePool(RootHubController);
  gBS->FreePool(Root);

  //
  // Uninstall USB Bus Protocol
  //
  gBS->UninstallProtocolInterface(
         Controller,
         &mUsbBusProtocolGuid,
         &UsbBusController->BusIdentify
       );

  //
  // Close USB_HC_PROTOCOL & DEVICE_PATH_PROTOCOL
  // Opened by this Controller
  //
  gBS->CloseProtocol(
        Controller,
        &gEfiUsbHcProtocolGuid,
        This->DriverBindingHandle,
        Controller
       );
  
  gBS->CloseProtocol(
         Controller,
         &gEfiDevicePathProtocolGuid,
         This->DriverBindingHandle,
         Controller
       );

  gBS->FreePool(UsbBusController);

  return EFI_SUCCESS;
}

//
// USB Device Configuration
//
STATIC
EFI_STATUS
UsbDeviceConfiguration (
  IN  USB_IO_CONTROLLER_DEVICE      *ParentHubController,
  IN  EFI_HANDLE                    HostController,
  IN  UINT8                         ParentPort,
  IN  USB_IO_DEVICE                 *UsbIoDevice
  )
/*++

  Routine Description:
    Configurate a new device attached to the usb bus

  Arguments:
    ParentHubController   -   Parent Hub which this device is connected.
    ParentPort            -   Parent Hub port which this device is connected.
    UsbIoDevice           -   The device to be configured.

  Returns:
    EFI_SUCCESS
    EFI_DEVICE_ERROR

--*/
{
  UINT8                       DevAddress;
  UINT8                       i;
  EFI_STATUS                  Result;
  UINT32                      Status;
  CHAR16                      *StrManufacturer;
  CHAR16                      *StrProduct;
  CHAR16                      *StrSerialNumber;
  EFI_USB_IO_PROTOCOL         *UsbIo;
  UINT8                       NumOfInterface;
  USB_IO_CONTROLLER_DEVICE    *FirstController;

  DEBUG ((EFI_D_USB, "Configuration Usb Device...\n"));
  
  //
  // Since a USB device must have at least on interface,
  // so create this instance first
  //
  FirstController = CreateUsbIoControllerDevice ();
  FirstController->UsbDevice = UsbIoDevice;
  UsbIoDevice->UsbController[0] = FirstController;
  FirstController->InterfaceNumber  = 0;
  FirstController->ParentPort       = ParentPort;
  FirstController->Parent           = ParentHubController;
  FirstController->HostController   = HostController;
  
  InitializeUsbIoInstance (FirstController);

  DevAddress = UsbAllocateAddress(UsbIoDevice->BusController->AddressPool);
  if (DevAddress == 0) {
    DEBUG ((EFI_D_USB, "Cannot allocate address\n"));
    gBS->FreePool(FirstController);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Ensure we used the correctly USB I/O instance
  //
  UsbIo = &FirstController->UsbIo;

  //
  // First retrieve the 1st 8 bytes of
  // in order to get the MaxPacketSize for Endpoint 0
  //
  Result = UsbGetDescriptor(
              UsbIo,
              0x0100,             // Value
              0,                  // Index
              8,                  // Length
              &UsbIoDevice->DeviceDescriptor,
              &Status
           );
  if (EFI_ERROR(Result) || (UsbIoDevice->DeviceDescriptor.MaxPacketSize0 == 0)) {    
    DEBUG ((EFI_D_USB, "Get Device Descriptor error\n"));
    //
    // Port Reset that controller, and try again
    //
    ParentPortReset(FirstController, FALSE);
    //
    // force the maximum packet size to 8 and try the command again.
    //
    UsbIoDevice->DeviceDescriptor.MaxPacketSize0 = 8;
    Result = UsbGetDescriptor(
               UsbIo,
               0x0100,             // Value
               0,                  // Index
               8,                  // Length
               &UsbIoDevice->DeviceDescriptor,
               &Status
             );
    if (EFI_ERROR(Result)) {
      DEBUG ((EFI_D_USB, "Get Device Descriptor error again\n"));
      UsbFreeAddress(
        DevAddress,
        UsbIoDevice->BusController->AddressPool
      );

      gBS->FreePool (FirstController);
      return EFI_DEVICE_ERROR;
    }
  }

  //
  // Assign a unique address to this device
  //
  Result = UsbSetDeviceAddress(UsbIo, DevAddress, &Status);

  if (EFI_ERROR (Result)) {
    DEBUG ((EFI_D_USB, "Set address error\n"));
    UsbFreeAddress(
      DevAddress,
      UsbIoDevice->BusController->AddressPool
    );

    gBS->FreePool(FirstController);
    return EFI_DEVICE_ERROR ;
  }

  UsbIoDevice->DeviceAddress = DevAddress ;

  //
  // Get the whole device descriptor
  //
  Result = UsbGetDescriptor(
              UsbIo,
              0x0100,
              0,
              sizeof(EFI_USB_DEVICE_DESCRIPTOR),
              &UsbIoDevice->DeviceDescriptor,
              &Status
          );
  if (EFI_ERROR(Result)) {
    DEBUG ((EFI_D_USB, "Get whole Device Descriptor error\n"));
    UsbFreeAddress(
      DevAddress,
      UsbIoDevice->BusController->AddressPool
    );

    gBS->FreePool(FirstController);
    return EFI_DEVICE_ERROR ;
  }

  InitializeListHead(&UsbIoDevice->ConfigDescListHead);

  //
  // Get & parse all configurations for this device, including
  // all configuration descriptors, all interface descriptors, all
  // endpoint descriptors
  //
  Result = UsbGetAllConfigurations(UsbIoDevice);

  if (EFI_ERROR (Result)) {
    DEBUG ((EFI_D_USB, "Failed to get device configuration\n")) ;
    UsbFreeAddress(
      DevAddress,
      UsbIoDevice->BusController->AddressPool
    );

    gBS->FreePool (FirstController);
    return EFI_DEVICE_ERROR ;
  }

  //
  // Set the 1st configuration value
  //
  Result = UsbSetDefaultConfiguration(UsbIoDevice);
  if (EFI_ERROR (Result)) {
    DEBUG ((EFI_D_USB, "Failed to set device configuration\n")) ;
    UsbFreeAddress(
      DevAddress,
      UsbIoDevice->BusController->AddressPool
    );

    gBS->FreePool (FirstController);
    return EFI_DEVICE_ERROR ;
  }

  UsbIoDevice->IsConfigured = TRUE;

  //
  // Get all string table if applicable
  //
  Result = UsbGetStringtable (UsbIoDevice);
  if(EFI_ERROR(Result)) {
    DEBUG((EFI_D_USB, "Failed to get device strings\n")) ;
//  return EFI_DEVICE_ERROR ;
  }

  StrManufacturer = NULL;
  UsbIo->UsbGetStringDescriptor(
           UsbIo,
           UsbIoDevice->LangID[0],
           (UsbIoDevice->DeviceDescriptor).StrManufacturer,
           &StrManufacturer
         );

  StrProduct = NULL;
  UsbIo->UsbGetStringDescriptor(
           UsbIo,
           UsbIoDevice->LangID[0],
           (UsbIoDevice->DeviceDescriptor).StrProduct,
           &StrProduct
         );

  StrSerialNumber = NULL;
  UsbIo->UsbGetStringDescriptor(
           UsbIo,
           UsbIoDevice->LangID[0],
           (UsbIoDevice->DeviceDescriptor).StrSerialNumber,
           &StrSerialNumber
        );

  if (StrManufacturer) {
    gBS->FreePool(StrManufacturer);
  }

  if (StrProduct) {
    gBS->FreePool(StrProduct);
  }

  if (StrSerialNumber) {
    gBS->FreePool(StrSerialNumber);
  }

  //
  // Create USB_IO_CONTROLLER_DEVICE for
  // each detected interface
  //
  FirstController->CurrentConfigValue = \
    UsbIoDevice->ActiveConfig->CongfigDescriptor.ConfigurationValue;

  NumOfInterface = UsbIoDevice->ActiveConfig->CongfigDescriptor.NumInterfaces;
  UsbIoDevice->NumOfControllers = NumOfInterface;

  Result = InitUsbIoController (FirstController);
  if (EFI_ERROR (Result)) {
    gBS->FreePool(FirstController);
    UsbIoDevice->UsbController[0] = NULL;
    return EFI_DEVICE_ERROR;
  }

  for (i = 1; i < NumOfInterface; i++) {
    USB_IO_CONTROLLER_DEVICE    *UsbIoController;

    UsbIoController = CreateUsbIoControllerDevice();
    UsbIoController->UsbDevice = UsbIoDevice;
    UsbIoController->CurrentConfigValue = \
        UsbIoDevice->ActiveConfig->CongfigDescriptor.ConfigurationValue;
    UsbIoController->InterfaceNumber = i;
    UsbIoDevice->UsbController[i] = UsbIoController;
    UsbIoController->ParentPort = ParentPort;
    UsbIoController->Parent = ParentHubController;
    UsbIoController->HostController = HostController;

    //
    // First copy the USB_IO Protocol instance
    //
    EfiCopyMem (
      &UsbIoController->UsbIo,
      UsbIo,
      sizeof(EFI_USB_IO_PROTOCOL)
    );

    Result = InitUsbIoController (UsbIoController);
    if (EFI_ERROR(Result)) {
       gBS->FreePool (UsbIoController);
       UsbIoDevice->UsbController[i] = NULL;
    }
  }

  return EFI_SUCCESS ;
}

//
// USB Device DeConfiguration
//
STATIC EFI_STATUS
UsbDeviceDeConfiguration(
  IN  USB_IO_DEVICE  *UsbIoDevice
  )
/*++

  Routine Description:
    Remove Device, Device Handles, Uninstall Protocols.

  Parameters:
    UsbIoDevice     -   The device to be deconfigured.

  Return Value:
    EFI_SUCCESS
    EFI_DEVICE_ERROR

--*/
{
  USB_IO_CONTROLLER_DEVICE    *UsbController;
  UINT8                       index;
  USB_IO_DEVICE               *ChildDevice;
  UINT8                       i;
  EFI_USB_IO_PROTOCOL         *UsbIo;

  DEBUG ((EFI_D_USB, "Enter Usb Device Deconfiguration\n"));

  //
  // Double check UsbIoDevice exists
  //
  if (UsbIoDevice == NULL) {
    return EFI_SUCCESS;
  }

  for (index = 0; index < UsbIoDevice->NumOfControllers; index++) {
    //
    // Check if it is a hub, if so, de configuration all its
    // downstream ports
    //
    UsbController = UsbIoDevice->UsbController[index];

    //
    // Check the controller pointer
    //
    if (UsbController == NULL) {
      continue;
    }

    if (UsbController->IsUsbHub) {       

       DEBUG ((EFI_D_USB, "Hub Deconfig, First Deconfig its downstream ports\n"));

       //
       // First Remove interrupt transfer request for the status
       // change port
       //
       UsbIo = &UsbController->UsbIo;
       UsbIo->UsbAsyncInterruptTransfer(
                UsbIo,
                UsbController->HubEndpointAddress,
                FALSE,
                0,
                0,
                NULL,
                NULL
              );
       
       if (NULL != UsbController->HubNotify) {
         gBS->CloseEvent (UsbController->HubNotify);
       }

       for (i = 0; i < UsbController->DownstreamPorts; i++) {
         if (UsbController->Children[i]) {
           ChildDevice = UsbController->Children[i];
           UsbDeviceDeConfiguration(ChildDevice);
           UsbController->Children[i] = NULL;
         }
       }
    }

    //
    // If the controller is managed by a device driver, we need to
    // disconnect them
    //
    if (UsbController->IsManagedByDriver) {
       gBS->DisconnectController(
              UsbController->Handle,
              NULL,
              NULL
            );
    }
    
    //
    // remove child handle reference to the USB_HC_PROTOCOL
    //
    gBS->CloseProtocol(
          UsbController->HostController,
          &gEfiUsbHcProtocolGuid,
          gUsbBusDriverBinding.DriverBindingHandle,
          UsbController->Handle
         );

    //
    // Uninstall EFI_USB_IO_PROTOCOL & DEVICE_PATH_PROTOCOL
    // installed on this handle
    //
    gBS->UninstallMultipleProtocolInterfaces(
           UsbController->Handle,
           &gEfiDevicePathProtocolGuid,
           UsbController->DevicePath,
           &gEfiUsbIoProtocolGuid,
           &UsbController->UsbIo,
           NULL
         );    

    gBS->FreePool(UsbController);
    UsbIoDevice->UsbController[index] = NULL;
  }

  //
  // Free address for later use
  //
  UsbFreeAddress(
    UsbIoDevice->DeviceAddress,
    UsbIoDevice->BusController->AddressPool
  );

  //
  // Free all resouces allocated for all its configurations
  //
  UsbDestroyAllConfiguration(UsbIoDevice);

  if (UsbIoDevice) {
    gBS->FreePool(UsbIoDevice);
    UsbIoDevice = NULL;
  }

  return EFI_SUCCESS;
}

//
// After interrupt complete, this function will be called,
// This function need to be well-defined later
//
STATIC EFI_STATUS
OnHubInterruptComplete (
  IN  VOID          *Data,
  IN  UINTN         DataLength,
  IN  VOID          *Context,
  IN  UINT32        Result
  )
/*++

  Routine Description:
    Whenever hub interrupt occurs, this routine will be called to check
    which event happens.

  Parameter:
    Data          -   Hub interrupt transfer data.
    DataLength    -   The length of the Data.
    Context       -   Hub Controller Device.
    Result        -   Hub interrupt transfer status.

  Return Value:
    EFI_SUCCESS
    EFI_DEVICE_ERROR

--*/
{
  USB_IO_CONTROLLER_DEVICE    *HubController;
  UINT8                       i;
  UINT8                       *ptr;
  EFI_USB_IO_PROTOCOL         *UsbIo;
  UINT32                      UsbResult;
  
  HubController = (USB_IO_CONTROLLER_DEVICE *)Context;
  UsbIo = &HubController->UsbIo;
  
  //
  // If something error in this interrupt transfer,
  //
  if (Result != EFI_USB_NOERROR) {
    if ((Result & EFI_USB_ERR_STALL) == EFI_USB_ERR_STALL) {
      UsbClearEndpointHalt(
        UsbIo,
        HubController->HubEndpointAddress,
        &UsbResult
      );      
    }
    
    //
    // Delete & Submit this interrupt again
    //
    UsbIo->UsbAsyncInterruptTransfer(
             UsbIo,
             HubController->HubEndpointAddress,
             FALSE,
             0,
             0,
             NULL,
             NULL
           );
           
    UsbIo->UsbAsyncInterruptTransfer(
             UsbIo,
             HubController->HubEndpointAddress,
             TRUE,
             100,
             1,  //Hub ports < 7
             OnHubInterruptComplete,
             HubController
           );
           
    return EFI_DEVICE_ERROR;           
  }
  
  if(DataLength == 0 || Data == NULL) {
    return EFI_SUCCESS;
  }
  
  //
  // Scan which port has status change
  // Bit 0 stands for hub itself, other bit stands for
  // the corresponding port
  //

  for (i = 0; i < DataLength * 8; i++) {
    ptr = (UINT8 *)Data + i /8;
    if ((*ptr) & (1 << (i % 8))) {
      HubController->StatusChangePort = i;
      break;
    }
  }

  //
  //Signal hub notify event
  //
  gBS->SignalEvent (HubController->HubNotify);

  return EFI_SUCCESS;
}

//
// USB Root Hub Enumerator
//
STATIC
VOID
UsbEnumeration (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
/*++

  Routine Description:
    This is USB enumerator

  Arguments:
    Event:   Indicating which event is signaled
    Context: actually it is a USB_IO_DEVICE

  Returns:
    EFI_SUCCESS
    Others

--*/
{
  USB_IO_CONTROLLER_DEVICE            *HubController;
  EFI_USB_PORT_STATUS                 HubPortStatus;
  EFI_STATUS                          Status;
  UINT8                               i;
  EFI_USB_HC_PROTOCOL                 *UsbHCInterface;
  USB_IO_DEVICE                       *UsbIoDev;
  EFI_HANDLE                          HostController;
  USB_IO_DEVICE                       *OldUsbIoDevice;
  USB_IO_DEVICE                       *NewDevice;
  USB_IO_CONTROLLER_DEVICE            *NewController;
  UINT8                               j;
  UINT8                               n;
  EFI_USB_IO_PROTOCOL                 *UsbIo;
  UINT8                               StatusChangePort;
  
  HubController = (USB_IO_CONTROLLER_DEVICE *)Context;
  HostController = HubController->HostController;
  
  if (HubController->UsbDevice->DeviceAddress == 1) {
    //
    // Root hub has the address 1
    //
    UsbIoDev = HubController->UsbDevice;
    UsbHCInterface = UsbIoDev->BusController->UsbHCInterface;

    for (i = 0; i < HubController->DownstreamPorts; i++) {
      UsbHCInterface->GetRootHubPortStatus (
                        UsbHCInterface,
                        i,
                        (EFI_USB_PORT_STATUS *)&HubPortStatus
                       );

      if (!IsPortConnectChange(HubPortStatus.PortChangeStatus)) {
        continue;
      }

      //
      //Clear root hub status change status
      //
      ClearRootPortConnectionChangeStatus(
        i,
        UsbHCInterface
      );

      UsbHCInterface->GetRootHubPortStatus (
                        UsbHCInterface,
                        i,
                        (EFI_USB_PORT_STATUS *)&HubPortStatus
                       );

      if (IsPortConnect(HubPortStatus.PortStatus)) {
        
        //
        // There is something connected to this port
        //
        DEBUG ((EFI_D_USB, "Something attached from Root Hub\n"));
        
        //
        // if there is something physically detached, but still logically 
        // attached...
        //
        OldUsbIoDevice = HubController->Children[i];

        if (NULL != OldUsbIoDevice) {
          UsbDeviceDeConfiguration(OldUsbIoDevice);
          HubController->Children[i] = NULL;
        }
        
        //
        // Reset the port.
        //
        ResetRootPort(
          i,
          UsbHCInterface
        );

        UsbHCInterface->GetRootHubPortStatus (
                          UsbHCInterface,
                          i,
                          (EFI_USB_PORT_STATUS *)&HubPortStatus
                         );
                         
        NewDevice = NULL;
        Status = gBS->AllocatePool(
                        EfiBootServicesData,
                        sizeof(USB_IO_DEVICE),
                        (VOID **)&NewDevice
                      );

        if (EFI_ERROR(Status)) {
           return;
        }

        EfiZeroMem(NewDevice, sizeof(USB_IO_DEVICE));

        //
        // Initialize some fields by copying data from
        // its parents
        //
        NewDevice->IsSlowDevice = \
             IsPortLowSpeedDeviceAttached (HubPortStatus.PortStatus);

        NewDevice->DeviceDescriptor.MaxPacketSize0 = 8;

        NewDevice->BusController = UsbIoDev->BusController;

        //
        // Configure that device
        //
        Status = UsbDeviceConfiguration(
                   HubController,
                   HostController,
                   i,
                   NewDevice
                  );
        if (EFI_ERROR (Status)) {
          gBS->FreePool (NewDevice);
          return;
        }

        //
        // Add this device to the usb bus tree
        //
        HubController->Children[i] = NewDevice;

        for (j = 0; j < NewDevice->NumOfControllers; j++) {
          //
          // If this device is hub, add to the hub index
          //
          NewController = NewDevice->UsbController[j];

          Status = gBS->ConnectController(
                          NewController->Handle,
                          NULL,
                          NULL,
                          TRUE
                        );
          //
          // If connect success, we need to disconnect when
          // stop the controller, otherwise we need not call
          // gBS->DisconnectController()
          // This is used by those usb devices we don't plan
          // to support. We can allocate
          // controller handles for them, but we don't have
          // device drivers to manage them.
          //
          NewController->IsManagedByDriver = (BOOLEAN)(!EFI_ERROR(Status));

          if (IsHub (NewController)) {

            NewController->IsUsbHub = TRUE;

            //
            // Configure Hub Controller
            //
            Status = DoHubConfig (NewController);
            if(EFI_ERROR(Status)) {
               continue;
            }

            //
            // Create an event to do hub enumeration
            //
            gBS->CreateEvent(
                   EFI_EVENT_NOTIFY_SIGNAL,
                   EFI_TPL_CALLBACK,
                   UsbEnumeration,
                   NewController,
                   &NewController->HubNotify
                 );

             //
             // Add request to do query hub status
             // change endpoint
             //
             UsbIo = &NewController->UsbIo;
             UsbIo->UsbAsyncInterruptTransfer(
                       UsbIo,
                       NewController->HubEndpointAddress,
                       TRUE,
                       100,
                       1,  //Hub ports < 7
                       OnHubInterruptComplete,
                       NewController
                    );

          }
        }
      } else {
        //
        // Something disconnected from USB root hub
        //
        
        DEBUG ((EFI_D_USB, "Something deteached from Root Hub\n"));
        
        OldUsbIoDevice = HubController->Children[i];

        UsbDeviceDeConfiguration(OldUsbIoDevice);

        HubController->Children[i] = NULL;

        UsbHCInterface->ClearRootHubPortFeature(
                          UsbHCInterface,
                          i,
                          EfiUsbPortEnableChange
                        );

        UsbHCInterface->GetRootHubPortStatus (
                          UsbHCInterface,
                          i,
                          (EFI_USB_PORT_STATUS *)&HubPortStatus
                         );
                        
      }
    }
    return;
  } else {
    //
    // Event from Hub, Get the hub controller handle
    //    

    //
    // Get the status change endpoint
    //
    StatusChangePort = HubController->StatusChangePort;

    //
    // Clear HubController Status Change Bit
    //
    HubController->StatusChangePort = 0;

    if (StatusChangePort == 0) {
      //
      // Hub changes, we don't handle here
      //
      return;
    }

    //
    // Check which event took place at that port
    //
    UsbIo = &HubController->UsbIo;
    Status = HubGetPortStatus(
               UsbIo,
               StatusChangePort,
               (UINT32 *)&HubPortStatus
             );

    if (EFI_ERROR(Status)) {
      return;
    }

    //
    // Clear some change status
    //
    if (HubPortStatus.PortChangeStatus & USB_PORT_STAT_C_ENABLE) {
      //
      // Clear Hub port enable change
      //
      DEBUG ((EFI_D_USB, "Port Enable Change\n"));
      HubClearPortFeature(
        UsbIo,
        StatusChangePort,
        EfiUsbPortEnableChange
      );
      
      HubGetPortStatus(
        UsbIo,
        StatusChangePort,
        (UINT32 *)&HubPortStatus
      );
    }

    if (HubPortStatus.PortChangeStatus & USB_PORT_STAT_C_RESET) {
      //
      // Clear Hub reset change
      //
      DEBUG ((EFI_D_USB, "Port Reset Change\n"));
      HubClearPortFeature(
        UsbIo,
        StatusChangePort,
        EfiUsbPortResetChange
      );

      HubGetPortStatus(
        UsbIo,
        StatusChangePort,
        (UINT32 *)&HubPortStatus
      );
    }
  
    if (HubPortStatus.PortChangeStatus & USB_PORT_STAT_C_OVERCURRENT) {
      //
      // Clear Hub overcurrent change
      //
      DEBUG ((EFI_D_USB, "Port Overcurrent Change\n"));
      HubClearPortFeature(
        UsbIo,
        StatusChangePort,
        EfiUsbPortOverCurrentChange
      );

      HubGetPortStatus(
        UsbIo,
        StatusChangePort,
        (UINT32 *)&HubPortStatus
      );
    }

    if (IsPortConnectChange(HubPortStatus.PortChangeStatus)) {
      //
      // First clear port connection change
      //
      DEBUG ((EFI_D_USB, "Port Connection Change\n"));
      HubClearPortFeature(
        UsbIo,
        StatusChangePort,
        EfiUsbPortConnectChange
      );

      HubGetPortStatus(
        UsbIo,
        StatusChangePort,
        (UINT32 *)&HubPortStatus
      );

      if (IsPortConnect(HubPortStatus.PortStatus)) {

        DEBUG ((EFI_D_USB, "New Device Connect on Hub port \n"));
        
        //
        // if there is something physically detached, but still logically 
        // attached...
        //
        OldUsbIoDevice = HubController->Children[StatusChangePort - 1];

        if (NULL != OldUsbIoDevice) {
          UsbDeviceDeConfiguration(OldUsbIoDevice);
          HubController->Children[StatusChangePort - 1] = NULL;
        }
        
        NewDevice = NULL;
        Status = gBS->AllocatePool(
                        EfiBootServicesData,
                        sizeof(USB_IO_DEVICE),
                        (VOID **)&NewDevice
                      );

        if (EFI_ERROR (Status)) {
          return;
        }

        EfiZeroMem(NewDevice, sizeof(USB_IO_DEVICE));

        //
        // Initialize some fields
        //
        NewDevice->IsSlowDevice = \
             IsPortLowSpeedDeviceAttached(HubPortStatus.PortStatus);

        NewDevice->DeviceDescriptor.MaxPacketSize0 = 8;
        NewDevice->BusController = HubController->UsbDevice->BusController;

        //
        // There is something connected to this port,
        // reset that port
        //
        HubSetPortFeature(
          UsbIo,
          StatusChangePort,
          EfiUsbPortReset
        );

        gBS->Stall(50 * 1000);

        //
        // Wait for port reset complete
        //
        n = 10;
        do {
          HubGetPortStatus(
            UsbIo,
            StatusChangePort,
            (UINT32 *)&HubPortStatus
          );
          gBS->Stall(10 * 100);
          n -= 1;
        }while((HubPortStatus.PortChangeStatus & USB_PORT_STAT_C_RESET ) == 0 && n > 0);

        if (n == 0) {
          //
          // Cannot reset port, return error
          //
          gBS->FreePool (NewDevice);
          return;
        }

        //
        // reset port will cause some bits change, clear them
        //
        if (HubPortStatus.PortChangeStatus & USB_PORT_STAT_C_ENABLE) {
           DEBUG ((EFI_D_USB, "Port Enable Change\n"));
           HubClearPortFeature(
             UsbIo,
             StatusChangePort,
             EfiUsbPortEnableChange
           );

          HubGetPortStatus(
            UsbIo,
            StatusChangePort,
            (UINT32 *)&HubPortStatus
          );
        }

        if (HubPortStatus.PortChangeStatus & USB_PORT_STAT_C_RESET) {
          DEBUG ((EFI_D_USB, "Port Reset Change\n"));
          HubClearPortFeature(
            UsbIo,
            StatusChangePort,
            EfiUsbPortResetChange
          );
          HubGetPortStatus(
            UsbIo,
            StatusChangePort,
            (UINT32 *)&HubPortStatus
          );
        }

        //
        // Configure that device
        //
        Status = UsbDeviceConfiguration(
                   HubController,
                   HostController,
                   (UINT8)(StatusChangePort - 1),
                    NewDevice
                 );

        if (EFI_ERROR (Status)) {
          gBS->FreePool (NewDevice);
          return;
        }

        //
        // Add this device to the usb bus tree
        // StatusChangePort is begin from 1,
        //
        HubController->Children[StatusChangePort - 1] = NewDevice;

        for (j = 0; j < NewDevice->NumOfControllers; j++) {
          //
          // If this device is hub, add to the hub index
          //
          NewController = NewDevice->UsbController[j];

          //
          // Connect the controller to the driver image
          //
          Status = gBS->ConnectController(
                          NewController->Handle,
                          NULL,
                          NULL,
                          TRUE
                        );
          //
          // If connect success, we need to disconnect when
          // stop the controller, otherwise we need not call
          // gBS->DisconnectController()
          // This is used by those usb devices we don't plan
          // to support. We can allocate
          // controller handles for them, but we don't have
          // device drivers to manage them.
          //
          NewController->IsManagedByDriver = (BOOLEAN)(!EFI_ERROR(Status));

          //
          // If this device is hub, add to the hub index
          //
          if (IsHub (NewController)) {

            NewController->IsUsbHub = TRUE;

            //
            // Configure Hub
            //
            Status = DoHubConfig(NewController);

            if(EFI_ERROR(Status)) {
               continue;
            }

            //
            // Create an event to do hub enumeration
            //
            gBS->CreateEvent(
                   EFI_EVENT_NOTIFY_SIGNAL,
                   EFI_TPL_CALLBACK,
                   UsbEnumeration,
                   NewController,
                   &NewController->HubNotify
                 );

            //
            // Add request to do query hub status
            // change endpoint
            //
            UsbIo = &NewController->UsbIo;
            UsbIo->UsbAsyncInterruptTransfer(
                     UsbIo,
                     NewController->HubEndpointAddress, //Hub endpoint address
                     TRUE,
                     100,
                     1,  //Hub ports < 7
                     OnHubInterruptComplete,
                     NewController
                   );
          }
        }
      } else {
        //
        // Something disconnected from USB hub
        //

        DEBUG ((EFI_D_USB, "Something Device Detached on Hub port\n"));

        OldUsbIoDevice = HubController->Children[StatusChangePort - 1];

        UsbDeviceDeConfiguration (OldUsbIoDevice);

        HubController->Children[StatusChangePort - 1] = NULL;

      }

      return;
    }

    return;
  }
}


//
// Clear port connection change status over a given root hub port
//
VOID
ClearRootPortConnectionChangeStatus (
  UINT8                   PortNum,
  EFI_USB_HC_PROTOCOL     *UsbHCInterface
)
/*++

  Routine Description:
    Clear port connection change status over a given root hub port

  Parameters:
    PortNum         -   The given port.
    UsbHCInterface  -   The EFI_USB_HC_PROTOCOL instance.

  Return Value:
    N/A

--*/
{
  UsbHCInterface->ClearRootHubPortFeature(
                    UsbHCInterface,
                    PortNum,
                    EfiUsbPortConnectChange
                  );
}

STATIC
USB_IO_CONTROLLER_DEVICE *
CreateUsbIoControllerDevice (
  VOID
)
/*++

  Routine Description:
    Allocate a structure for USB_IO_CONTROLLER_DEVICE

  Parameters:
    N/A

  Return Value:
    A pointer to a USB_IO_CONTROLLER_DEVICE structure,
    Or NULL.

--*/
{
  USB_IO_CONTROLLER_DEVICE  *UsbIoControllerDev;
  EFI_STATUS                Status;

  //
  // Allocate USB_IO_CONTROLLER_DEVICE structure
  //
  UsbIoControllerDev = NULL;
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(USB_IO_CONTROLLER_DEVICE),
                  (VOID **)&UsbIoControllerDev
                );

  if (EFI_ERROR(Status)) {
    return NULL;
  }

  EfiZeroMem (UsbIoControllerDev, sizeof(USB_IO_CONTROLLER_DEVICE));

  UsbIoControllerDev->Signature = USB_IO_CONTROLLER_SIGNATURE;

  return UsbIoControllerDev;
}

STATIC
EFI_STATUS
InitUsbIoController (
  IN USB_IO_CONTROLLER_DEVICE     *UsbIoController
  )
/*++

  Routine Description:
    Init and install EFI_USB_IO_PROTOCOL onto that controller.

  Parameters:
    UsbIoController   -   The Controller to be operated.

  Return Value:
    EFI_SUCCESS
    Others

--*/
{
  USB_DEVICE_PATH             UsbNode;
  EFI_STATUS                  Status;
  EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath;
  EFI_USB_HC_PROTOCOL         *UsbHcProtocol;
  
  //
  // Build the child device path for each new USB_IO device
  //
  EfiZeroMem(&UsbNode, sizeof(UsbNode));
  UsbNode.Header.Type = MESSAGING_DEVICE_PATH;
  UsbNode.Header.SubType = MSG_USB_DP;
  SetDevicePathNodeLength (&UsbNode.Header, sizeof(UsbNode));
  UsbNode.InterfaceNumber = UsbIoController->InterfaceNumber;
  UsbNode.ParentPortNumber = UsbIoController->ParentPort;
  ParentDevicePath = UsbIoController->Parent->DevicePath;

  UsbIoController->DevicePath = \
        EfiAppendDevicePathNode (ParentDevicePath, &UsbNode.Header);
  if (UsbIoController->DevicePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &UsbIoController->Handle,
                  &gEfiDevicePathProtocolGuid,
                  UsbIoController->DevicePath,
                  &gEfiUsbIoProtocolGuid,
                  &UsbIoController->UsbIo,
                  NULL
                );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  Status = gBS->OpenProtocol (
                  UsbIoController->HostController,
                  &gEfiUsbHcProtocolGuid, 
                  &UsbHcProtocol,
                  gUsbBusDriverBinding.DriverBindingHandle,   
                  UsbIoController->Handle,   
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                );

  return Status;  
}

STATIC
EFI_STATUS
ParentPortReset (
  IN USB_IO_CONTROLLER_DEVICE    *UsbIoController,
  IN BOOLEAN                     ReConfigure
  )
/*++

  Routine Description:
    Reset parent hub port to which this device is connected.

  Parameters:
    UsbIoController   -   Indicating the Usb Controller Device.
    Reconfigure       -   Do we need to reconfigure it.

  Return Value:
    EFI_SUCCESS
    EFI_DEVICE_ERROR

--*/
{
  USB_IO_DEVICE               *ParentIoDev;
  USB_IO_DEVICE               *UsbIoDev;
  USB_IO_CONTROLLER_DEVICE    *ParentController;
  UINT8                       HubPort;
  UINT32                      Status;
  EFI_STATUS                  Result;
  EFI_USB_IO_PROTOCOL         *UsbIo;
  UINT8                       Address;

  ParentController = UsbIoController->Parent;
  ParentIoDev = ParentController->UsbDevice;
  UsbIoDev = UsbIoController->UsbDevice;
  HubPort = UsbIoController->ParentPort;

  if (UsbIoController->IsUsbHub) {
    return EFI_INVALID_PARAMETER;
  }

  if (ParentIoDev->DeviceAddress == 1) {
    //
    // Send RESET signal from Root Hub Port
    //
    DEBUG ((EFI_D_USB, "\n"));
    ResetRootPort(HubPort, ParentIoDev->BusController->UsbHCInterface);
  } else {

    //
    // Send RESET signal from Hub Port
    //
    DEBUG ((EFI_D_USB, "\n"));
    HubSetPortFeature(
      &ParentController->UsbIo,
      (UINT8)(HubPort + 1),
      EfiUsbPortReset
    );
  }

  gBS->Stall (50 * 1000);

  //
  // If we only need port reset, just return
  //
  if (!ReConfigure) {
    return EFI_SUCCESS;
  }

  //
  // Re-config that USB device
  //
  UsbIo = &UsbIoController->UsbIo;

  //
  // Assign a unique address to this device
  //
  Address = UsbIoDev->DeviceAddress;
  UsbIoDev->DeviceAddress = 0;

  Result = UsbSetDeviceAddress (UsbIo, Address, &Status);
  UsbIoDev->DeviceAddress = Address;

  if (EFI_ERROR(Result)) {
    return EFI_DEVICE_ERROR ;
  }

  //
  // Set the device to the default configuration
  //
  Result = UsbSetDefaultConfiguration (UsbIoDev);
  if (EFI_ERROR(Result)) {
    return EFI_DEVICE_ERROR ;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
UsbPortReset (
  IN EFI_USB_IO_PROTOCOL      *This
)
/*++

  Routine Description:
    Resets and reconfigures the USB controller.  This function will
    work for all USB devices except USB Hub Controllers.

  Parameters:
    This          -   Indicates the calling context.

  Return Value:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER
    EFI_DEVICE_ERROR

--*/
{
  USB_IO_CONTROLLER_DEVICE    *UsbIoController;
  EFI_STATUS                  Status;

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS(This);

  //
  // Since at this time, this device has already been configured,
  // it needs to be re-configured.
  //
  Status = ParentPortReset (UsbIoController, TRUE);

  return Status;
}

VOID
ResetRootPort(
  UINT8                  PortNum,
  EFI_USB_HC_PROTOCOL    *UsbHCInterface
  )
/*++

  Routine Description:
    Reset Root Hub port.

  Parameters:
    PortNum         -   The given port to be reset.
    UsbHCInterface  -   The EFI_USB_HC_PROTOCOL instance.

  Return Value:
    N/A

--*/
{
  EFI_USB_PORT_STATUS     PortStatus;

  UsbHCInterface->GetRootHubPortStatus(
                    UsbHCInterface,
                    PortNum,
                    &PortStatus
                  );

  //
  // reset root port
  //
  UsbHCInterface->SetRootHubPortFeature(
                    UsbHCInterface,
                    PortNum,
                    EfiUsbPortReset
                  );

  gBS->Stall(200 * 1000);

  //
  // clear reset root port
  //
  UsbHCInterface->ClearRootHubPortFeature(
                    UsbHCInterface,
                    PortNum,
                    EfiUsbPortReset
                  );

  gBS->Stall (1000);

  ClearRootPortConnectionChangeStatus (PortNum, UsbHCInterface);

  //
  // Set port enable
  //
  UsbHCInterface->SetRootHubPortFeature(
                    UsbHCInterface,
                    PortNum,
                    EfiUsbPortEnable
                  );

  gBS->Stall(500);

  return;
}
