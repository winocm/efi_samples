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

  EfiKey.c
    
Abstract:

  USB Keyboard Driver

Revision History

--*/
#include "efikey.h"
#include "keyboard.h"


//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
USBKeyboardDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
USBKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
USBKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
USBKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );


//
// Simple Text In Protocol Interface
//
STATIC
EFI_STATUS 
EFIAPI 
USBKeyboardReset (
    IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This, 
    IN  BOOLEAN                     ExtendedVerification
    );

STATIC
EFI_STATUS 
EFIAPI 
USBKeyboardReadKeyStroke (
    IN  EFI_SIMPLE_TEXT_IN_PROTOCOL   *This, 
    OUT EFI_INPUT_KEY                 *Key
    );

STATIC
VOID 
EFIAPI
USBKeyboardWaitForKey (
    IN  EFI_EVENT               Event,
    IN  VOID                    *Context
    );
    
//  Helper functions

STATIC
EFI_STATUS
USBKeyboardCheckForKey (
  IN  USB_KB_DEV      *UsbKeyboardDevice
  );


//
// USB Keyboard Driver Global Variables
//

EFI_DRIVER_BINDING_PROTOCOL gUsbKeyboardDriverBinding = {
  USBKeyboardDriverBindingSupported,
  USBKeyboardDriverBindingStart,
  USBKeyboardDriverBindingStop,
  0x10,
  NULL,
  NULL
};


EFI_DRIVER_ENTRY_POINT(USBKeyboardDriverBindingEntryPoint)

EFI_STATUS
USBKeyboardDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
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
           &gUsbKeyboardDriverBinding, 
           ImageHandle,
           &gUsbKeyboardComponentName,
           NULL,
           NULL
         );
}


EFI_STATUS
USBKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
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
  EFI_STATUS            OpenStatus;
  EFI_USB_IO_PROTOCOL   *UsbIo;
  EFI_STATUS            Status;

  //
  // Check if USB_IO protocol is attached on the controller handle.
  //
  OpenStatus = gBS->OpenProtocol (
                      Controller,
                      &gEfiUsbIoProtocolGuid,
                      &UsbIo,
                      This->DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_BY_DRIVER
                 );
  if (EFI_ERROR (OpenStatus)) {
    return OpenStatus;
  }
   
  //
  // Use the USB I/O protocol interface to check whether the Controller is
  // the Keyboard controller that can be managed by this driver.
  //
  Status = EFI_SUCCESS;
  
  if(!IsUSBKeyboard(UsbIo)) {
    Status =  EFI_UNSUPPORTED;
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
USBKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Start.
  
  Arguments:
    (Standard DriverBinding Protocol Start() function)
    
  Returns:
    EFI_STATUS
  
--*/       
{ 
  EFI_STATUS                    Status;
  EFI_USB_IO_PROTOCOL           *UsbIo;
  USB_KB_DEV                    *UsbKeyboardDevice;
  UINT8                         EndpointNumber;
  EFI_USB_ENDPOINT_DESCRIPTOR   EndpointDescriptor;
  UINT8                         Index;
  UINT8                         EndpointAddr;
  UINT8                         PollingInterval;
  UINT8                         PacketSize;
  BOOLEAN                       Found;
  
  UsbKeyboardDevice = NULL;
  Found = FALSE;
  
  //
  // Open USB_IO Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller, 
                  &gEfiUsbIoProtocolGuid, 
                  &UsbIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {    
    return Status;
  } 
  
  Status = gBS->AllocatePool(
          EfiBootServicesData,
          sizeof(USB_KB_DEV),
          &UsbKeyboardDevice
          );
  
  if(EFI_ERROR(Status)) {
    gBS->CloseProtocol (
            Controller, 
            &gEfiUsbIoProtocolGuid, 
            This->DriverBindingHandle,   
            Controller   
         );
    return Status;
  }

  EfiZeroMem(UsbKeyboardDevice,sizeof(USB_KB_DEV));

  //
  // Initialize UsbKeyboardDevice
  //
  UsbKeyboardDevice->UsbIo = UsbIo;
  
  //
  // Get interface & endpoint descriptor
  //
  UsbIo->UsbGetInterfaceDescriptor(
                UsbIo,
                &UsbKeyboardDevice->InterfaceDescriptor
                );

  EndpointNumber = UsbKeyboardDevice->InterfaceDescriptor.NumEndpoints;

  for(Index = 0; Index < EndpointNumber; Index ++) {
    
    UsbIo->UsbGetEndpointDescriptor(
            UsbIo,
            Index,
            &EndpointDescriptor
            );

    if((EndpointDescriptor.Attributes & 0x03) == 0x03) {
      //
      // We only care interrupt endpoint here
      //
      UsbKeyboardDevice->IntEndpointDescriptor = EndpointDescriptor;
      Found = TRUE;
    }
  }

  if(!Found) {
    //
    // No interrupt endpoint found, then return unsupported.
    //    
    gBS->FreePool(UsbKeyboardDevice);
    gBS->CloseProtocol (
            Controller, 
            &gEfiUsbIoProtocolGuid, 
            This->DriverBindingHandle,   
            Controller   
         );
    return EFI_UNSUPPORTED;
  }
  
  UsbKeyboardDevice->Signature = USB_KB_DEV_SIGNATURE;
  UsbKeyboardDevice->SimpleInput.Reset = USBKeyboardReset;
  UsbKeyboardDevice->SimpleInput.ReadKeyStroke = USBKeyboardReadKeyStroke;
  Status = gBS->CreateEvent (
                    EFI_EVENT_NOTIFY_WAIT,
                    EFI_TPL_NOTIFY,
                    USBKeyboardWaitForKey,
                    UsbKeyboardDevice,
                    &(UsbKeyboardDevice->SimpleInput.WaitForKey)
                    );
    
  if(EFI_ERROR(Status)) {
    gBS->FreePool(UsbKeyboardDevice);
    gBS->CloseProtocol (
            Controller, 
            &gEfiUsbIoProtocolGuid, 
            This->DriverBindingHandle,   
            Controller   
         );
    return Status;
  }   
       
  //
  // Install simple txt in protocol interface 
  // for the usb keyboard device.
  // Usb keyboard is a hot plug device, and expected to work immediately
  // when plugging into system, so a HotPlugDeviceGuid is installed onto
  // the usb keyboard device handle, to distinguish it from other conventional
  // console devices.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                    &Controller,
                    &gEfiSimpleTextInProtocolGuid, 
                    &UsbKeyboardDevice->SimpleInput,
                    &gEfiHotPlugDeviceGuid,
                    NULL,
                    NULL
                    );
  if (EFI_ERROR(Status)) {
    gBS->CloseEvent(UsbKeyboardDevice->SimpleInput.WaitForKey);
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
            Controller, 
            &gEfiUsbIoProtocolGuid, 
            This->DriverBindingHandle,   
            Controller   
         );
    return Status;
  }
    
  //
  // Reset USB Keyboard Device
  //
  Status = UsbKeyboardDevice->SimpleInput.Reset(
                          &UsbKeyboardDevice->SimpleInput,
                          TRUE
                          ); 
  if(EFI_ERROR(Status)) {
    gBS->UninstallMultipleProtocolInterfaces (
                 Controller,
                 &gEfiSimpleTextInProtocolGuid,
                 &UsbKeyboardDevice->SimpleInput,
                 &gEfiHotPlugDeviceGuid,
                 NULL,
                 NULL
                 );
    gBS->CloseEvent(UsbKeyboardDevice->SimpleInput.WaitForKey);
    gBS->FreePool(UsbKeyboardDevice);
    gBS->CloseProtocol (
            Controller, 
            &gEfiUsbIoProtocolGuid, 
            This->DriverBindingHandle,   
            Controller   
         );
    return Status;
  } 

  //
  // submit async interrupt transfer
  //
  EndpointAddr    = UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress;
  PollingInterval = UsbKeyboardDevice->IntEndpointDescriptor.Interval;
  PacketSize      = 
        (UINT8)(UsbKeyboardDevice->IntEndpointDescriptor.MaxPacketSize);
    
  Status = UsbIo->UsbAsyncInterruptTransfer(
                    UsbIo,
                    EndpointAddr,
                    TRUE,
                    PollingInterval,
                    PacketSize,
                    KeyboardHandler,
                    UsbKeyboardDevice
              );
    
  if(EFI_ERROR(Status)) {
    
    gBS->UninstallMultipleProtocolInterfaces (
                 Controller,
                 &gEfiSimpleTextInProtocolGuid,
                 &UsbKeyboardDevice->SimpleInput,
                 &gEfiHotPlugDeviceGuid,
                 NULL,
                 NULL
                 );
    gBS->CloseEvent(UsbKeyboardDevice->SimpleInput.WaitForKey);    
    gBS->FreePool(UsbKeyboardDevice);
    gBS->CloseProtocol (
            Controller, 
            &gEfiUsbIoProtocolGuid, 
            This->DriverBindingHandle,   
            Controller   
         );
    return Status;
  }

  UsbKeyboardDevice->ControllerNameTable = NULL;
  EfiLibAddUnicodeString (
    "eng", 
    gUsbKeyboardComponentName.SupportedLanguages, 
    &UsbKeyboardDevice->ControllerNameTable,
    L"Generic Usb Keyboard"
  );
      
  return EFI_SUCCESS;
}


EFI_STATUS
USBKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
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
  EFI_STATUS                      Status;
  EFI_SIMPLE_TEXT_IN_PROTOCOL     *SimpleInput;
  USB_KB_DEV                      *UsbKeyboardDevice;

  Status = gBS->OpenProtocol (
                    Controller, 
                    &gEfiSimpleTextInProtocolGuid, 
                    &SimpleInput,
                    This->DriverBindingHandle,   
                    Controller,   
                    EFI_OPEN_PROTOCOL_BY_DRIVER
                    );                     
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }
  
  //
  // Get USB_KB_DEV instance.
  //
  UsbKeyboardDevice = USB_KB_DEV_FROM_THIS (SimpleInput);
    
  gBS->CloseProtocol(
            Controller,
            &gEfiSimpleTextInProtocolGuid,
            This->DriverBindingHandle,
            Controller
            );
  
  //
  // Destroy asynchronous interrupt transfer
  //
  UsbKeyboardDevice->UsbIo->UsbAsyncInterruptTransfer(
                 UsbKeyboardDevice->UsbIo,
                 UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
                 FALSE,
                 UsbKeyboardDevice->IntEndpointDescriptor.Interval,
                 0,
                 NULL,
                 NULL
                 );
      
  gBS->CloseProtocol (
              Controller, 
              &gEfiUsbIoProtocolGuid, 
              This->DriverBindingHandle, 
              Controller
           ); 
  
  Status = gBS->UninstallMultipleProtocolInterfaces (
                 Controller,
                 &gEfiSimpleTextInProtocolGuid,
                 &UsbKeyboardDevice->SimpleInput,
                 &gEfiHotPlugDeviceGuid,
                 NULL,
                 NULL
                 );
  //
  // free all the resources.
  //
  gBS->CloseEvent(UsbKeyboardDevice->RepeatTimer);
  gBS->CloseEvent((UsbKeyboardDevice->SimpleInput).WaitForKey);
  
  if (UsbKeyboardDevice->ControllerNameTable != NULL) {
    EfiLibFreeUnicodeStringTable (UsbKeyboardDevice->ControllerNameTable);
  }
      
  gBS->FreePool (UsbKeyboardDevice);
  
  return Status;
  
}


EFI_STATUS
USBKeyboardReset (
    IN  EFI_SIMPLE_TEXT_IN_PROTOCOL   *This, 
    IN  BOOLEAN             ExtendedVerification
    )
/*++

  Routine Description:
    Implements EFI_SIMPLE_TEXT_IN_PROTOCOL.Reset() function.
  
  Arguments:
    This:     The EFI_SIMPLE_TEXT_IN_PROTOCOL instance.
    ExtendedVerification:
              Indicates that the driver may perform a more exhaustive
              verification operation of the device during reset.              
    
  Returns:  

--*/      
{
  EFI_STATUS        Status;
  USB_KB_DEV        *UsbKeyboardDevice;
  
  UsbKeyboardDevice = USB_KB_DEV_FROM_THIS (This);
  
  //
  // Non Exhaustive reset:
  // only reset private data structures.
  //
  if(!ExtendedVerification) {
    InitUSBKeyBuffer(&(UsbKeyboardDevice->KeyboardBuffer));
    UsbKeyboardDevice->CurKeyChar = 0;
    return EFI_SUCCESS;
  }
  
  //
  // Exhaustive reset
  //
  Status = InitUSBKeyboard(UsbKeyboardDevice);
  if(EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
  
  return EFI_SUCCESS;
}     
    

STATIC
EFI_STATUS 
EFIAPI 
USBKeyboardReadKeyStroke (
    IN  EFI_SIMPLE_TEXT_IN_PROTOCOL   *This, 
    OUT EFI_INPUT_KEY                 *Key
    )
/*++

  Routine Description:
    Implements EFI_SIMPLE_TEXT_IN_PROTOCOL.ReadKeyStroke() function.
  
  Arguments:
    This:     The EFI_SIMPLE_TEXT_IN_PROTOCOL instance.
    Key:      A pointer to a buffer that is filled in with the keystroke
              information for the key that was pressed.
    
  Returns:  

--*/       
{
  USB_KB_DEV      *UsbKeyboardDevice;
  EFI_STATUS      Status;
  UINT8           KeyChar;
  
  UsbKeyboardDevice = USB_KB_DEV_FROM_THIS(This);
  
  //
  // if there is no saved ASCII byte, fetch it 
  // by calling USBKeyboardCheckForKey().
  //
  if(UsbKeyboardDevice->CurKeyChar == 0) {
    Status = USBKeyboardCheckForKey(UsbKeyboardDevice);
    if(EFI_ERROR(Status)) {
      return Status;
    }
  }

  Key->UnicodeChar = 0;
  Key->ScanCode = SCAN_NULL;

  KeyChar = UsbKeyboardDevice->CurKeyChar;
  
  UsbKeyboardDevice->CurKeyChar = 0;
  
  //
  // Translate saved ASCII byte into EFI_INPUT_KEY
  //
  Status = USBKeyCodeToEFIScanCode(UsbKeyboardDevice, KeyChar,Key);
  
  return Status;  
  
}     

STATIC
VOID 
EFIAPI
USBKeyboardWaitForKey (
    IN  EFI_EVENT               Event,
    IN  VOID                    *Context
    )
/*++

  Routine Description:
    Handler function for WaitForKey event.    
  
  Arguments:
    Event:        Event to be signaled when a key is pressed.
    Context:      Points to USB_KB_DEV instance.
    
  Returns:  

--*/       
{
  USB_KB_DEV        *UsbKeyboardDevice;
  
  UsbKeyboardDevice = (USB_KB_DEV*)Context;
  
  if(UsbKeyboardDevice->CurKeyChar == 0) {
    
    if (EFI_ERROR(USBKeyboardCheckForKey(UsbKeyboardDevice))) {
      return;
    }
  }
  //
  // If has key pending, signal the event.
  //
  gBS->SignalEvent(Event);
}


STATIC
EFI_STATUS
USBKeyboardCheckForKey (
  IN  USB_KB_DEV    *UsbKeyboardDevice
  )
/*++

  Routine Description:
    Check whether there is key pending.
  
  Arguments:
    UsbKeyboardDevice:    The USB_KB_DEV instance.
    
  Returns:  

--*/       
{
  EFI_STATUS      Status;
  UINT8           KeyChar;
  
  //
  // Fetch raw data from the USB keyboard input,
  // and translate it into ASCII data.
  //
  Status = USBParseKey(UsbKeyboardDevice,&KeyChar);
  if(EFI_ERROR(Status)) {
    return Status;
  }
  UsbKeyboardDevice->CurKeyChar = KeyChar;
  return EFI_SUCCESS;
}    
