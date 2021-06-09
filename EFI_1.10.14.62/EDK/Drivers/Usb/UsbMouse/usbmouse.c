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

    UsbMouse.c

  Abstract:

--*/

#include "Efi.h"
#include "EfiDriverLib.h"
#include "EfiPrintLib.h"

#include "usb.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(DriverBinding)
#include EFI_PROTOCOL_DEFINITION(UsbIo)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(SimplePointer)

#include "usblib.h"
#include "hid.h"
#include "usbmouse.h"
#include "mousehid.h"

//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
USBMouseDriverBindingEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
USBMouseDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
USBMouseDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
USBMouseDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Controller,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  );

EFI_DRIVER_BINDING_PROTOCOL gUsbMouseDriverBinding = {
  USBMouseDriverBindingSupported,
  USBMouseDriverBindingStart,
  USBMouseDriverBindingStop,
  0x10,
  NULL,
  NULL
};

//
// helper functions
//
static BOOLEAN
IsUsbMouse (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo
  );

static EFI_STATUS
InitializeUsbMouseDevice (
  IN  USB_MOUSE_DEV           *UsbMouseDev
  );

static VOID
EFIAPI
UsbMouseWaitForInput (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

//
// Mouse interrupt handler
//
static EFI_STATUS
OnMouseInterruptComplete (
  IN  VOID        *Data,
  IN  UINTN       DataLength,
  IN  VOID        *Context,
  IN  UINT32      Result
  );

//
// Mouse Protocol
//
static EFI_STATUS
GetMouseState(
  IN   EFI_SIMPLE_POINTER_PROTOCOL  *This,
  OUT  EFI_SIMPLE_POINTER_STATE     *MouseState
);

static EFI_STATUS
UsbMouseReset(
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN BOOLEAN                        ExtendedVerification
  );

//
// Driver start here
//
EFI_DRIVER_ENTRY_POINT(USBMouseDriverBindingEntryPoint)

EFI_STATUS
USBMouseDriverBindingEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Register Driver Binding protocol for this driver.

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:
  EFI_SUCCESS - Driver loaded
  other       - Driver not loaded

--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gUsbMouseDriverBinding, 
           ImageHandle,
           &gUsbMouseComponentName,
           NULL,
           NULL
         );
}


EFI_STATUS
USBMouseDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

  Routine Description:
    Test to see if this driver supports ControllerHandle. Any ControllerHandle
    than contains a BlockIo and DiskIo protocol can be supported.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to test
    RemainingDevicePath - Not used

  Returns:
    EFI_SUCCES          - This driver supports this device
    EFI_ALREADY_STARTED - This driver is already running on this device
    other               - This driver does not support this device

--*/
{
  EFI_STATUS                OpenStatus;
  EFI_USB_IO_PROTOCOL       *UsbIo;
  EFI_STATUS              Status;

  OpenStatus = gBS->OpenProtocol(
                    Controller,
                    &gEfiUsbIoProtocolGuid,
                    &UsbIo,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_BY_DRIVER
                 );
  if (EFI_ERROR (OpenStatus) && (OpenStatus != EFI_ALREADY_STARTED)) {
      return EFI_UNSUPPORTED;
  }

  if (OpenStatus == EFI_ALREADY_STARTED) {
    return EFI_ALREADY_STARTED;
  }
  
  //
  // Use the USB I/O protocol interface to see the Controller is
  // the Mouse controller that can be managed by this driver.
  //
  Status = EFI_SUCCESS;
  if(!IsUsbMouse(UsbIo)) {
      Status = EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
       );
  return Status;
}


EFI_STATUS
USBMouseDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

  Routine Description:
    Start this driver on ControllerHandle by opening a Block IO and Disk IO
    protocol, reading Device Path, and creating a child handle with a
    Disk IO and device path protocol.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to bind driver to
    RemainingDevicePath - Not used

  Returns:
    EFI_SUCCES          - This driver is added to DeviceHandle
    EFI_ALREADY_STARTED - This driver is already running on DeviceHandle
    other               - This driver does not support this device

--*/
{
  EFI_STATUS                  Status;
  EFI_USB_IO_PROTOCOL         *UsbIo;
  EFI_USB_ENDPOINT_DESCRIPTOR *EndpointDesc;
  USB_MOUSE_DEV               *UsbMouseDevice;
  UINT8                       EndpointNumber;
  UINT8                       i;
  UINT8                       EndpointAddr;
  UINT8                       PollingInterval;
  UINT8                       PacketSize;

  UsbMouseDevice = NULL;
  Status = EFI_SUCCESS;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsbIoProtocolGuid,
                  &UsbIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
      goto ErrorExit;
  }

  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(USB_MOUSE_DEV),
                  &UsbMouseDevice
                );

  if (EFI_ERROR(Status)) {
      Status = EFI_OUT_OF_RESOURCES;
      goto ErrorExit;
  }

  EfiZeroMem(UsbMouseDevice,sizeof(USB_MOUSE_DEV));

  UsbMouseDevice->UsbIo = UsbIo;

  UsbMouseDevice->Signature = USB_MOUSE_DEV_SIGNATURE;

  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof (EFI_USB_INTERFACE_DESCRIPTOR),
                  &UsbMouseDevice->InterfaceDescriptor
                );
                
  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }
                    
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof (EFI_USB_ENDPOINT_DESCRIPTOR),
                  &EndpointDesc
                );
                
  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  //
  // Get interface & endpoint descriptor
  //
  UsbIo->UsbGetInterfaceDescriptor(
           UsbIo,
           UsbMouseDevice->InterfaceDescriptor
         );

  EndpointNumber = UsbMouseDevice->InterfaceDescriptor->NumEndpoints;

  for(i = 0; i < EndpointNumber; i++) {
    UsbIo->UsbGetEndpointDescriptor(
             UsbIo,
             i,
             EndpointDesc
           );

    if((EndpointDesc->Attributes & 0x03) == 0x03) {

        //
        // We only care interrupt endpoint here
        //
        UsbMouseDevice->IntEndpointDescriptor = EndpointDesc;
    }
  }

  if(UsbMouseDevice->IntEndpointDescriptor == NULL) {
    //
    // No interrupt endpoint, then error
    //
    Status = EFI_UNSUPPORTED;
    goto ErrorExit;
  }

  Status = InitializeUsbMouseDevice (UsbMouseDevice);
  if (EFI_ERROR(Status)) {
    goto ErrorExit;
  }

  UsbMouseDevice->SimplePointerProtocol.GetState = GetMouseState;
  UsbMouseDevice->SimplePointerProtocol.Reset = UsbMouseReset;
  UsbMouseDevice->SimplePointerProtocol.Mode = &UsbMouseDevice->Mode;

  Status = gBS->CreateEvent (
                    EFI_EVENT_NOTIFY_WAIT,
                    EFI_TPL_NOTIFY,
                    UsbMouseWaitForInput,
                    UsbMouseDevice,
                    &((UsbMouseDevice->SimplePointerProtocol).WaitForInput)
                    );
  if (EFI_ERROR(Status)) {
    goto ErrorExit;
  }

  Status = gBS->InstallProtocolInterface(
                  &Controller,
                  &gEfiSimplePointerProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &UsbMouseDevice->SimplePointerProtocol
                );

  if (EFI_ERROR(Status)) {
    Status = EFI_DEVICE_ERROR;
    goto ErrorExit;
  }
  
  //
  // submit async interrupt transfer
  //
  EndpointAddr = UsbMouseDevice->IntEndpointDescriptor->EndpointAddress;
  PollingInterval = UsbMouseDevice->IntEndpointDescriptor->Interval;
  PacketSize = (UINT8)(UsbMouseDevice->IntEndpointDescriptor->MaxPacketSize);

  Status = UsbIo->UsbAsyncInterruptTransfer(
                      UsbIo,
                      EndpointAddr,
                      TRUE,
                      PollingInterval,
                      PacketSize,
                      OnMouseInterruptComplete,
                      UsbMouseDevice
                   );

  if (!EFI_ERROR(Status)) {
    
    UsbMouseDevice->ControllerNameTable = NULL;
    EfiLibAddUnicodeString (
      "eng", 
      gUsbMouseComponentName.SupportedLanguages, 
      &UsbMouseDevice->ControllerNameTable, 
      L"Generic Usb Mouse"
    );

    return EFI_SUCCESS;
  }

  //
  // If submit error, uninstall that interface
  //
  Status = EFI_DEVICE_ERROR; 
  gBS->UninstallProtocolInterface (
        Controller,
        &gEfiSimplePointerProtocolGuid,
        &UsbMouseDevice->SimplePointerProtocol
      );
           
ErrorExit:
  if(EFI_ERROR(Status)) {
    gBS->CloseProtocol (
           Controller,
           &gEfiUsbIoProtocolGuid,
           This->DriverBindingHandle,
           Controller
         );

    if (UsbMouseDevice != NULL) {
      if (UsbMouseDevice->InterfaceDescriptor != NULL) {
        gBS->FreePool (UsbMouseDevice->InterfaceDescriptor);
      }
      if (UsbMouseDevice->IntEndpointDescriptor != NULL) {
        gBS->FreePool (UsbMouseDevice->IntEndpointDescriptor);
      }
      
      if ((UsbMouseDevice->SimplePointerProtocol).WaitForInput != NULL) {
        gBS->CloseEvent ((UsbMouseDevice->SimplePointerProtocol).WaitForInput);
      }
      gBS->FreePool(UsbMouseDevice);
      UsbMouseDevice = NULL;
    }
  }

  return Status;
}


EFI_STATUS
USBMouseDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Controller,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
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
    EFI_SUCCES          - This driver is removed DeviceHandle
    other               - This driver was not removed from this device

--*/
{
  EFI_STATUS                      Status;
  USB_MOUSE_DEV                   *UsbMouseDevice;
  EFI_SIMPLE_POINTER_PROTOCOL     *SimplePointerProtocol;
  EFI_USB_IO_PROTOCOL             *UsbIo;

  //
  // Get our context back.
  //
  Status = gBS->OpenProtocol(
              Controller,
              &gEfiSimplePointerProtocolGuid,
              &SimplePointerProtocol,
              This->DriverBindingHandle,
              Controller,
              EFI_OPEN_PROTOCOL_GET_PROTOCOL
          );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  UsbMouseDevice = \
    USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL(SimplePointerProtocol);

  UsbIo = UsbMouseDevice->UsbIo;

  //
  // Delete Mouse Async Interrupt Transfer
  //
  UsbIo->UsbAsyncInterruptTransfer(
            UsbIo,
            UsbMouseDevice->IntEndpointDescriptor->EndpointAddress,
            FALSE,
            UsbMouseDevice->IntEndpointDescriptor->Interval,
            0,
            NULL,
            NULL
          );

  gBS->CloseEvent (UsbMouseDevice->SimplePointerProtocol.WaitForInput);

  Status = gBS->UninstallProtocolInterface(
                  Controller,
                  &gEfiSimplePointerProtocolGuid,
                  &UsbMouseDevice->SimplePointerProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol(
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
       );

  gBS->FreePool (UsbMouseDevice->InterfaceDescriptor);
  gBS->FreePool (UsbMouseDevice->IntEndpointDescriptor);

  if (UsbMouseDevice->ControllerNameTable) {
    EfiLibFreeUnicodeStringTable (UsbMouseDevice->ControllerNameTable);
  }

  gBS->FreePool (UsbMouseDevice);

  return EFI_SUCCESS;

}

BOOLEAN
IsUsbMouse (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo
  )
/*++

  Routine Description:
    Tell if a Usb Controller is a mouse

  Arguments:
    This              - Protocol instance pointer.

  Returns:
    TRUE              - It is a mouse
    FALSE             - It is not a mouse
--*/
{
  EFI_STATUS                      Status;
  EFI_USB_INTERFACE_DESCRIPTOR    InterfaceDescriptor;

  //
  // Get the Default interface descriptor, now we only
  // suppose it is interface 1
  //
  Status = UsbIo->UsbGetInterfaceDescriptor(
                    UsbIo,
                    &InterfaceDescriptor
                  );

  if(EFI_ERROR(Status)) {
    return FALSE;
  }

  if ((InterfaceDescriptor.InterfaceClass == CLASS_HID) &&
      (InterfaceDescriptor.InterfaceSubClass == SUBCLASS_BOOT) &&
      (InterfaceDescriptor.InterfaceProtocol == PROTOCOL_MOUSE)
      ) {
    return TRUE;
  }

  return FALSE;
}

static EFI_STATUS
InitializeUsbMouseDevice (
  IN  USB_MOUSE_DEV           *UsbMouseDev
  )
/*++

  Routine Description:
    Initialize the Usb Mouse Device.

  Arguments:
    UsbMouseDev         - Device instance to be initialized

  Returns:
    EFI_SUCCES
    other               - Init error.

--*/
{
  EFI_USB_IO_PROTOCOL         *UsbIo;
  UINT8                       Protocol;
  EFI_STATUS                  Status;
  EFI_USB_HID_DESCRIPTOR      MouseHidDesc;
  UINT8                       *ReportDesc;

  UsbIo = UsbMouseDev->UsbIo;

  //
  // Get HID descriptor
  //
  Status = UsbGetHidDescriptor(
              UsbIo,
              UsbMouseDev->InterfaceDescriptor->InterfaceNumber,
              &MouseHidDesc
           );

  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Get Report descriptor
  //
  if (MouseHidDesc.HidClassDesc[0].DescriptorType != 0x22) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  MouseHidDesc.HidClassDesc[0].DescriptorLength,
                  &ReportDesc
                );
  if (EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  EfiZeroMem(ReportDesc, MouseHidDesc.HidClassDesc[0].DescriptorLength);

  Status = UsbGetReportDescriptor(
              UsbIo,
              UsbMouseDev->InterfaceDescriptor->InterfaceNumber,
              MouseHidDesc.HidClassDesc[0].DescriptorLength,
              ReportDesc
           );

  if(EFI_ERROR(Status)) {
    gBS->FreePool(ReportDesc);
    return Status;
  }

  //
  // Parse report descriptor
  //
  Status = ParseMouseReportDescriptor(
              UsbMouseDev,
              ReportDesc,
              MouseHidDesc.HidClassDesc[0].DescriptorLength
           );

  if(EFI_ERROR(Status)) {
      gBS->FreePool(ReportDesc);
      return Status;
  }

  if(UsbMouseDev->NumberOfButtons >= 1) {
      UsbMouseDev->Mode.LeftButton = TRUE;
  }
  if(UsbMouseDev->NumberOfButtons > 1) {
      UsbMouseDev->Mode.RightButton = TRUE;
  }

//  UsbMouseDev->Mode.ResolutionX = UsbMouseDev->XLogicMax - UsbMouseDev->XLogicMin;
//  UsbMouseDev->Mode.ResolutionY = UsbMouseDev->YLogicMax - UsbMouseDev->YLogicMin;
  UsbMouseDev->Mode.ResolutionX = 8;
  UsbMouseDev->Mode.ResolutionY = 8;
  UsbMouseDev->Mode.ResolutionZ = 0;
  //
  // Here we just assume interface 0 is the mouse interface
  //
  UsbGetProtocolRequest(
    UsbIo,
    0,
    &Protocol
  );

  if (Protocol != BOOT_PROTOCOL) {
    Status = UsbSetProtocolRequest(
               UsbIo,
               0,
               BOOT_PROTOCOL
             );

    if (EFI_ERROR(Status)) {
      gBS->FreePool(ReportDesc);
      return EFI_DEVICE_ERROR;
    }
  }

  //
  // Set indefinite Idle rate for USB Mouse
  //
  UsbSetIdleRequest(
      UsbIo,
      0,
      0,
      0
  );

  gBS->FreePool (ReportDesc);

  return EFI_SUCCESS;
}

/*
static VOID
PrintMouseState(
    IN  EFI_MOUSE_STATE *MouseState
    );
*/

static EFI_STATUS
OnMouseInterruptComplete (
  IN  VOID        *Data,
  IN  UINTN       DataLength,
  IN  VOID        *Context,
  IN  UINT32      Result
  )
/*++

  Routine Description:
    It is called whenever there is data received from async interrupt
    transfer.

  Arguments:
    Data            - Data received.
    DataLength      - Length of Data
    Context         - Passed in context
    Result          - Async Interrupt Transfer result

  Returns:
    EFI_SUCCES
    EFI_DEVICE_ERROR

--*/
{
  USB_MOUSE_DEV               *UsbMouseDev;
  EFI_USB_IO_PROTOCOL         *UsbIo;
  UINT8                       EndpointAddr;
  UINT32                      UsbResult;
  EFI_SIMPLE_POINTER_STATE    TempState;

  UsbMouseDev = (USB_MOUSE_DEV *)Context;
  UsbIo = UsbMouseDev->UsbIo;

  if (Result != EFI_USB_NOERROR) {
    if ((Result & EFI_USB_ERR_STALL) == EFI_USB_ERR_STALL) {
      EndpointAddr = UsbMouseDev->IntEndpointDescriptor->EndpointAddress;

      UsbClearEndpointHalt(
        UsbIo,
        EndpointAddr,
        &UsbResult
      );
    }
    
    UsbIo->UsbAsyncInterruptTransfer(
             UsbIo,
             UsbMouseDev->IntEndpointDescriptor->EndpointAddress,
             FALSE,
             0,
             0,
             NULL,
             NULL
           );
    UsbIo->UsbAsyncInterruptTransfer(
                      UsbIo,
                      UsbMouseDev->IntEndpointDescriptor->EndpointAddress,
                      TRUE,
                      UsbMouseDev->IntEndpointDescriptor->Interval,
                      UsbMouseDev->IntEndpointDescriptor->MaxPacketSize,
                      OnMouseInterruptComplete,
                      UsbMouseDev
                );
    return EFI_DEVICE_ERROR;
  }
  
  if(DataLength == 0 || Data == NULL) {
    return EFI_SUCCESS;
  }

  //
  // Check mouse Data
  //
  TempState.LeftButton = (BOOLEAN)(*(UINT8*)Data & 0x01);
  TempState.RightButton = (BOOLEAN)(*(UINT8*)Data & 0x02);
  TempState.RelativeMovementX = *((INT8*)Data + 1);
  TempState.RelativeMovementY = *((INT8*)Data + 2);
  if(DataLength > 3) {
    TempState.RelativeMovementZ = *((INT8*)Data + 3);
  }
  else {
    TempState.RelativeMovementZ = 0;
  }

  if (TempState.RelativeMovementX != 0 ||
	  TempState.RelativeMovementY != 0 ||
	  TempState.RelativeMovementZ != 0 ||
	  TempState.LeftButton != UsbMouseDev->State.LeftButton ||
	  TempState.RightButton != UsbMouseDev->State.RightButton) {
    UsbMouseDev->State.RelativeMovementX += TempState.RelativeMovementX;
    UsbMouseDev->State.RelativeMovementY += TempState.RelativeMovementY;
    UsbMouseDev->State.RelativeMovementZ += TempState.RelativeMovementZ;
	UsbMouseDev->State.LeftButton = TempState.LeftButton;
	UsbMouseDev->State.RightButton = TempState.RightButton;
    UsbMouseDev->StateChanged = TRUE;
  }

  return EFI_SUCCESS;
}

/*
static VOID
PrintMouseState(
    IN  EFI_MOUSE_STATE *MouseState
    )
{
    Aprint("(%x: %x, %x)\n",
        MouseState->ButtonStates,
        MouseState->dx,
        MouseState->dy
        );
}
*/

static EFI_STATUS
GetMouseState(
  IN   EFI_SIMPLE_POINTER_PROTOCOL  *This,
  OUT  EFI_SIMPLE_POINTER_STATE     *MouseState
)
/*++

  Routine Description:
    Get the mouse state, see SIMPLE POINTER PROTOCOL.
    
  Arguments:
    This              - Protocol instance pointer.
    MouseState        - Current mouse state
    
  Returns:
    EFI_SUCCES
    EFI_INVALID_PARAMETER

--*/
{
  USB_MOUSE_DEV           *MouseDev;

  if (MouseState == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  MouseDev = USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL(This);

  if (!MouseDev->StateChanged) {
    return EFI_NOT_READY;
  }

  EfiCopyMem(
    MouseState,
    &MouseDev->State,
    sizeof(EFI_SIMPLE_POINTER_STATE)
  );

  //
  // Clear previous move state
  //
  MouseDev->State.RelativeMovementX = 0;
  MouseDev->State.RelativeMovementY = 0;
  MouseDev->State.RelativeMovementZ = 0;
  MouseDev->StateChanged = FALSE;

  return EFI_SUCCESS;
}

static EFI_STATUS
UsbMouseReset(
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN BOOLEAN                        ExtendedVerification
  )
/*++

  Routine Description:
    Reset the mouse device, see SIMPLE POINTER PROTOCOL.
    
  Arguments:
    This                  - Protocol instance pointer.
    ExtendedVerification  - Ignored here/
    
  Returns:
    EFI_SUCCES

--*/
{
  USB_MOUSE_DEV     *UsbMouseDev;

  UsbMouseDev = USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL (This);

  EfiZeroMem (
    &UsbMouseDev->State, 
    sizeof(EFI_SIMPLE_POINTER_STATE)
  );

  return EFI_SUCCESS;
}

static VOID
EFIAPI
UsbMouseWaitForInput (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  )
/*++

Routine Description:

  Event notification function for SIMPLE_POINTER.WaitForInput event
  Signal the event if there is input from mouse

Arguments:

Returns:

--*/
{
  USB_MOUSE_DEV     *UsbMouseDev;

  UsbMouseDev = (USB_MOUSE_DEV*)Context;

  //
  // Someone is waiting on the mouse event, if there's
  // input from mouse, signal the event
  //
  if (UsbMouseDev->StateChanged) {
    gBS->SignalEvent(Event);
  }
}
