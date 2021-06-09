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

  cbi1.c

Abstract:
    cbi1 transportation protocol implementation files

--*/

#include "Efi.h"
#include "EfiDriverLib.h"
#include "EfiPrintLib.h"

#include "usb.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(UsbIo)
#include EFI_PROTOCOL_DEFINITION(DriverBinding)

//
// Driver produced Protocol
//
#include EFI_PROTOCOL_DEFINITION(UsbAtapi)

#include "usblib.h"
#include "..\cbi.h"

EFI_STATUS
UsbCBI1DriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// CBI Function prototypes
//
static EFI_STATUS
CBI1CommandPhase (
  IN  USB_CBI_DEVICE          *UsbCbiDev,
  IN  VOID          *Command,
  IN  UINT8           CommandSize,
  OUT UINT32                  *Result
);

static EFI_STATUS
CBI1DataPhase (
  IN  USB_CBI_DEVICE          *UsbCbiDev,
  IN  UINT32                  DataSize,
  IN  OUT VOID                *DataBuffer,
  IN  EFI_USB_DATA_DIRECTION  Direction,
  IN  UINT16                  Timeout,
  OUT UINT32                  *Result
);

//
// USB Atapi implementation
//
static EFI_STATUS
CBI1AtapiCommand (
  IN  EFI_USB_ATAPI_PROTOCOL    *This,
  IN  VOID                      *Command,
  IN  UINT8                     CommandSize,
  IN  VOID                      *DataBuffer,
  IN  UINT32                    BufferLength,
  IN  EFI_USB_DATA_DIRECTION    Direction,
  IN  UINT16                    TimeOutInMilliSeconds
);

static EFI_STATUS
CBI1MassStorageReset (
  IN  EFI_USB_ATAPI_PROTOCOL    *This,
  IN  BOOLEAN                     ExtendedVerification
);

//
// CBI1 Driver Binding Protocol
//
static EFI_STATUS
CBI1DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

static EFI_STATUS
CBI1DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

static EFI_STATUS
CBI1DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

EFI_DRIVER_BINDING_PROTOCOL gCBI1DriverBinding = {
  CBI1DriverBindingSupported,
  CBI1DriverBindingStart,
  CBI1DriverBindingStop,
  0x10,
  NULL,
  NULL
};

static EFI_USB_ATAPI_PROTOCOL CBI1AtapiProtocol = {
    CBI1AtapiCommand,
    CBI1MassStorageReset,
    0
};

//
// CBI1 Driver Entry Point
//
EFI_DRIVER_ENTRY_POINT(UsbCBI1DriverEntryPoint)

EFI_STATUS
UsbCBI1DriverEntryPoint(
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
  return EfiLibInstallDriverBinding (
          ImageHandle, 
          SystemTable, 
          &gCBI1DriverBinding, 
          ImageHandle
         );
}

//
// CBI1 Driver Binding implementation
//
static EFI_STATUS
CBI1DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
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
    EFI_STATUS                    Status;
    EFI_USB_IO_PROTOCOL           *UsbIo;
    EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;

    //
    // Check if the Controller supports USB IO protocol
    //
    Status = gBS->OpenProtocol (
                    ControllerHandle,
                    &gEfiUsbIoProtocolGuid,
                    &UsbIo,
                    This->DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Get the Controller interface descriptor
    //
    Status = UsbIo->UsbGetInterfaceDescriptor(
                      UsbIo,
                      &InterfaceDescriptor
                    );
    if(EFI_ERROR(Status)) {
      goto Exit;
    }

    //
    // Bug here: just let Vendor specific CBI protocol get supported
    //
    if(!((InterfaceDescriptor.InterfaceClass == 0xFF)
        && (InterfaceDescriptor.InterfaceProtocol == 0))) {
        Status = EFI_UNSUPPORTED;
        goto Exit;
    }

Exit:
    gBS->CloseProtocol(
            ControllerHandle,
            &gEfiUsbIoProtocolGuid,
            This->DriverBindingHandle,
            ControllerHandle
          );
    return Status;

}

static EFI_STATUS
CBI1DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
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
  USB_CBI_DEVICE                  *UsbCbiDev;
  UINT8                           i;
  EFI_USB_ENDPOINT_DESCRIPTOR     EndpointDescriptor;
  EFI_USB_INTERFACE_DESCRIPTOR    InterfaceDescriptor;
  EFI_STATUS                      Status;
  EFI_USB_IO_PROTOCOL             *UsbIo;
  BOOLEAN                         Found = FALSE;

  //
  // Check if the Controller supports USB IO protocol
  //
  UsbCbiDev = NULL;

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiUsbIoProtocolGuid,
                  &UsbIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get the controller interface descriptor
  //
  Status = UsbIo->UsbGetInterfaceDescriptor(
                    UsbIo,
                    &InterfaceDescriptor
                  );
  if(EFI_ERROR(Status)) {
    goto ErrorExit;
  }

  CBI1AtapiProtocol.CommandProtocol = InterfaceDescriptor.InterfaceSubClass;
  
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(USB_CBI_DEVICE),
                  &UsbCbiDev
                );

  if(EFI_ERROR(Status)) {
    goto ErrorExit;
  }

  EfiZeroMem(UsbCbiDev, sizeof(USB_CBI_DEVICE));

  UsbCbiDev->Signature = USB_CBI_DEVICE_SIGNATURE;
  UsbCbiDev->UsbIo = UsbIo;
  UsbCbiDev->InterfaceDescriptor = InterfaceDescriptor;
  UsbCbiDev->UsbAtapiProtocol = CBI1AtapiProtocol;

  for(i = 0; i < InterfaceDescriptor.NumEndpoints; i++) {
      UsbIo->UsbGetEndpointDescriptor(
              UsbIo,
              i,
              &EndpointDescriptor
            );

      //
      // We parse bulk endpoint
      //
      if(EndpointDescriptor.Attributes == 0x02) {
            if(EndpointDescriptor.EndpointAddress & 0x80) {
                UsbCbiDev->BulkInEndpointDescriptor = EndpointDescriptor;
            } else {
                UsbCbiDev->BulkOutEndpointDescriptor = EndpointDescriptor;
            }
            Found = TRUE;
      }

      //
      // We parse interrupt endpoint
      //
      if(EndpointDescriptor.Attributes == 0x03) {
        UsbCbiDev->InterruptEndpointDescriptor = EndpointDescriptor;
        Found = TRUE;
      }

  }

  //
  // Double check we have these
  //
  if(!Found) {
    goto ErrorExit;
  }

  Status = gBS->InstallProtocolInterface (
                  &ControllerHandle,
                  &gEfiUsbAtapiProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &UsbCbiDev->UsbAtapiProtocol
                );

  if(EFI_ERROR(Status)) {
    goto ErrorExit;
  }

  return EFI_SUCCESS;

ErrorExit:
  gBS->CloseProtocol (
         ControllerHandle,
         &gEfiUsbIoProtocolGuid,
         This->DriverBindingHandle,
         ControllerHandle
       );

  if(UsbCbiDev != NULL) {
    gBS->FreePool(UsbCbiDev);
  }

  return Status;

}

static EFI_STATUS
CBI1DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
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
  EFI_STATUS                    Status;
  EFI_USB_ATAPI_PROTOCOL        *CBI1AtapiProtocol;
  USB_CBI_DEVICE                *UsbCbiDev;

  //
  // Get our context back.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiUsbAtapiProtocolGuid,
                  &CBI1AtapiProtocol,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  UsbCbiDev = USB_CBI_DEVICE_FROM_THIS(CBI1AtapiProtocol);

  Status = gBS->UninstallProtocolInterface (
                  ControllerHandle,
                  &gEfiUsbAtapiProtocolGuid,
                  &UsbCbiDev->UsbAtapiProtocol
                );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->CloseProtocol (
                  ControllerHandle,
                  &gEfiUsbIoProtocolGuid,
                  This->DriverBindingHandle,
                  ControllerHandle
                );

  gBS->FreePool(UsbCbiDev);

  return Status;

}


//
// CBI1 command
//
static EFI_STATUS
CBI1CommandPhase (
  IN  USB_CBI_DEVICE          *UsbCbiDev,
  IN  VOID                    *Command,
  IN  UINT8                   CommandSize,
  OUT UINT32                  *Result
)
/*++
    In order to make consistence, CBI transportation protocol does only use
    the first 3 parameters. Other parameters are not used here.
--*/
{
  EFI_STATUS            Status;
//  UINT32                Result;
  EFI_USB_IO_PROTOCOL   *UsbIo;
  EFI_USB_DEVICE_REQUEST    Request;
  UINT32                TimeOutInMilliSeconds;

  UsbIo = UsbCbiDev->UsbIo;

  EfiZeroMem ( &Request, sizeof(EFI_USB_DEVICE_REQUEST));

  //
  // Device request see CBI specification
  //
  Request.RequestType = 0x21;
  Request.Request = 0x00;
  Request.Value = 0 ;
  Request.Index = 0 ;
  Request.Length = CommandSize;
  
  TimeOutInMilliSeconds = 1000;

  Status = UsbIo->UsbControlTransfer(
              UsbIo,
              &Request,
              EfiUsbDataOut,
              TimeOutInMilliSeconds,
              Command,
              CommandSize,
              Result
            );  

  return Status ;
}


static EFI_STATUS
CBI1DataPhase (
  IN  USB_CBI_DEVICE              *UsbCbiDev,
  IN  UINT32                      DataSize,
  IN  OUT VOID                    *DataBuffer,
  IN  EFI_USB_DATA_DIRECTION      Direction,
  IN  UINT16                      Timeout,
  OUT UINT32                      *Result
)
{
  EFI_STATUS          Status;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               EndpointAddr;
  UINTN               Remain;
  UINTN               Increment;
  UINT32              MaxPacketLen;
  UINT8               *BufferPtr;

  UsbIo = UsbCbiDev->UsbIo;

  Remain = DataSize;
  BufferPtr = (UINT8 *)DataBuffer;

  //
  // retrieve the the max packet length of the given endpoint
  //
  if ( Direction == EfiUsbDataIn ) {
    MaxPacketLen = (UsbCbiDev->BulkInEndpointDescriptor).MaxPacketSize;
    EndpointAddr = (UsbCbiDev->BulkInEndpointDescriptor).EndpointAddress;
  } else {
    MaxPacketLen = (UsbCbiDev->BulkOutEndpointDescriptor).MaxPacketSize;
    EndpointAddr = (UsbCbiDev->BulkOutEndpointDescriptor).EndpointAddress;
  }

  while ( Remain > 0 ) {
    //
    // Using 15 packets to avoid Bitstuff error
    //
    if ( Remain > 15 * MaxPacketLen) {
      Increment = 15 * MaxPacketLen;
    } else {
      Increment = Remain ;
    }

    Status = UsbIo->UsbBulkTransfer(
                UsbIo,
                EndpointAddr,
                BufferPtr,
                &Increment,
                Timeout,
                Result
              );

    if(EFI_ERROR(Status)) {
      goto ErrorExit;
    }

    BufferPtr += Increment;
    Remain -= Increment ;
  }

  return EFI_SUCCESS;

ErrorExit:
  if(((*Result) & EFI_USB_ERR_STALL) == EFI_USB_ERR_STALL) {
    //
    //just endpoint stall happens
    //
    UsbClearEndpointHalt(
        UsbIo,
        EndpointAddr,
        Result
    );
  }

  return Status;
}

//
// CBI1 USB ATAPI Protocol
//
static EFI_STATUS
CBI1MassStorageReset (
  IN  EFI_USB_ATAPI_PROTOCOL      *This,
  IN  BOOLEAN                     ExtendedVerification
  )
/*++

  Routine Description:
    Reset CBI Devices
    
  Arguments:
    This                    - Protocol instance pointer.
    ExtendedVerification    - TRUE if we need to do strictly reset.

  Returns:
    EFI_SUCCES          - Commond succeeded.
    EFI_DEVICE_ERROR    - Command failed.

--*/
{
  UINT8               ResetCommand[12];
  UINT8               i;
  EFI_STATUS          Status;
  EFI_USB_IO_PROTOCOL *UsbIo;
  USB_CBI_DEVICE      *UsbCbiDev;
  UINT8               EndpointAddr;
  UINT32              Result;

  UsbCbiDev = USB_CBI_DEVICE_FROM_THIS(This);
  UsbIo = UsbCbiDev->UsbIo;

  if (ExtendedVerification) {
    UsbIo->UsbPortReset(UsbIo);
  }

  //
  // CBI reset command protocol
  //
  ResetCommand[0] = 0x1d;
  ResetCommand[1] = 0x04;
  for(i = 2; i < 12; i++) {
    ResetCommand[i] = 0xff;
  }

  Status = CBI1CommandPhase(
              UsbCbiDev,
              ResetCommand,
              12,
              &Result
           );

  //
  // clear bulk in endpoint stall feature
  //
  EndpointAddr = (UsbCbiDev->BulkInEndpointDescriptor).EndpointAddress;
  UsbClearEndpointHalt(
        UsbIo,
        EndpointAddr,
        &Result
    );

  //
  // clear bulk out endpoint stall feature
  //
  EndpointAddr = (UsbCbiDev->BulkOutEndpointDescriptor).EndpointAddress;
  UsbClearEndpointHalt(
    UsbIo,
    EndpointAddr,
    &Result
  );

  return EFI_SUCCESS ;

}

static EFI_STATUS
CBI1AtapiCommand (
  IN  EFI_USB_ATAPI_PROTOCOL  *This,
  IN  VOID                    *Command,
  IN  UINT8                   CommandSize,
  IN  VOID                    *DataBuffer,
  IN  UINT32                  BufferLength,
  IN  EFI_USB_DATA_DIRECTION  Direction,
  IN  UINT16                  TimeOutInMilliSeconds
  )
/*++

  Routine Description:
    Send ATAPI command using CBI1 protocol.
    
  Arguments:
    This              - Protocol instance pointer.
    Command           - Command buffer 
    CommandSize       - Size of Command Buffer
    DataBuffer        - Data buffer
    BufferLength      - Length of Data buffer
    Direction         - Data direction of this command
    TimeoutInMilliseconds - Timeout value in ms

  Returns:
    EFI_SUCCES          - Commond succeeded.
    EFI_DEVICE_ERROR    - Command failed.

--*/
{
  EFI_STATUS              Status;
  USB_CBI_DEVICE          *UsbCbiDev;
  UINT32                  Result;
  UINT8                   i,MaxRetryNum;

  UsbCbiDev = USB_CBI_DEVICE_FROM_THIS(This);
  
  MaxRetryNum = 3;
  
  for (i = 0 ; i < MaxRetryNum; i ++) {
  
    //
    // First send ATAPI command through CBI1
    //
    Status = CBI1CommandPhase(
                UsbCbiDev,
                Command,
                CommandSize,
                &Result
             );
    if (EFI_ERROR (Status)) {

      switch (Result) {        
        //
        // when meeting the first 3 err code,
        // no need to perform retry
        //        
        case EFI_USB_NOERROR:
        case EFI_USB_ERR_STALL:
        case EFI_USB_ERR_SYSTEM:
          return EFI_DEVICE_ERROR;
          
        default:
          continue;
          break;
      }
    } else {
      break;
    }
  }
  
  if (i == MaxRetryNum) {
    return EFI_DEVICE_ERROR;
  }
  
  for (i = 0 ; i < MaxRetryNum; i ++) {
    //
    // Send/Get Data if there is a Data Stage
    //
    switch(Direction) {
      case EfiUsbDataIn:  // fall through
      case EfiUsbDataOut:
        Status = CBI1DataPhase(
                  UsbCbiDev,
                  BufferLength,
                  DataBuffer,
                  Direction,
                  TimeOutInMilliSeconds,
                  &Result
                 );
  
        if (EFI_ERROR(Status)) {

          switch (Result) {
            
            //
            // when meeting the first 3 err code,
            // no need to perform retry
            //
            case EFI_USB_NOERROR:     // fall through
            case EFI_USB_ERR_STALL:   // fall through
            case EFI_USB_ERR_SYSTEM:  // fall through
              return EFI_DEVICE_ERROR;
              
            default:
              continue;
              break;
          }

      } else {

          return EFI_SUCCESS;
        }
        break;
    
      case EfiUsbNoData:
        return EFI_SUCCESS;
    }
  }
  //
  // If goes here, means met error.
  //
  return EFI_DEVICE_ERROR;
}

