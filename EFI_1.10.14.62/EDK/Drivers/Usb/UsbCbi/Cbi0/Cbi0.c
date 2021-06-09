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

  Cbi0.c

Abstract:

--*/

#include "Efi.h"
#include "EfiDriverLib.h"
#include "usb.h"

#include "..\cbi.h"
#include "usblib.h"

extern EFI_COMPONENT_NAME_PROTOCOL gUsbCbi0ComponentName;
//
// Function prototypes
//
EFI_STATUS
UsbCbi0DriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Bot Driver Binding Protocol
//
static EFI_STATUS
Cbi0DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

static EFI_STATUS
Cbi0DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

static EFI_STATUS
Cbi0DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

EFI_DRIVER_BINDING_PROTOCOL gUsbCbi0DriverBinding = {
  Cbi0DriverBindingSupported,
  Cbi0DriverBindingStart,
  Cbi0DriverBindingStop,
  0x10,
  NULL,
  NULL
};

STATIC
EFI_STATUS
Cbi0RecoveryReset (
  IN  USB_CBI_DEVICE   *UsbCbiDev
  );
  
STATIC EFI_STATUS
Cbi0CommandPhase (
  IN  USB_CBI_DEVICE            *UsbCbiDev,
  IN  VOID                      *Command,
  IN  UINT8                     CommandSize,
  IN  UINT16                    Timeout
);

STATIC EFI_STATUS
Cbi0DataPhase (
  IN  USB_CBI_DEVICE          *UsbCbiDev,
  IN  UINT32                  *DataSize,
  IN  OUT VOID                *DataBuffer,
  IN  EFI_USB_DATA_DIRECTION  Direction,
  IN  UINT16                  Timeout
);

STATIC EFI_STATUS
Cbi0StatusPhase (
  IN  USB_CBI_DEVICE        *UsbCbiDev,
  OUT INTERRUPT_DATA_BLOCK  *InterruptDataBlock,
  IN  UINT16                Timeout
  );

//
// USB Atapi protocol prototype
//
STATIC EFI_STATUS
Cbi0AtapiCommand (
  IN  EFI_USB_ATAPI_PROTOCOL    *This,
  IN  VOID                      *Command,
  IN  UINT8                     CommandSize,
  IN  VOID                      *DataBuffer,
  IN  UINT32                    BufferLength,
  IN  EFI_USB_DATA_DIRECTION    Direction,
  IN  UINT16                    TimeOutInMilliSeconds
);

STATIC EFI_STATUS
Cbi0MassStorageReset (
  IN  EFI_USB_ATAPI_PROTOCOL    *This,
  IN  BOOLEAN                   ExtendedVerification
);

STATIC EFI_USB_ATAPI_PROTOCOL Cbi0AtapiProtocol = {
  Cbi0AtapiCommand,
  Cbi0MassStorageReset,
  0
};


EFI_DRIVER_ENTRY_POINT(UsbCbi0DriverEntryPoint)

EFI_STATUS
UsbCbi0DriverEntryPoint(
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
           &gUsbCbi0DriverBinding, 
           ImageHandle,
           &gUsbCbi0ComponentName,
           NULL,
           NULL
         );
}

STATIC EFI_STATUS
Cbi0DriverBindingSupported (
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
  // Get the Default interface descriptor, now we only
  // suppose interface 1
  //
  Status = UsbIo->UsbGetInterfaceDescriptor(
                    UsbIo,
                    &InterfaceDescriptor
                  );
  if(EFI_ERROR(Status)) {
    gBS->CloseProtocol(
        ControllerHandle,
        &gEfiUsbIoProtocolGuid,
        This->DriverBindingHandle,
        ControllerHandle
       );
    return Status;
  }
  //
  // Check if it is a Cbi0 Type Mass Storage Device
  //
  if((InterfaceDescriptor.InterfaceClass != MASS_STORAGE_CLASS)
    || (InterfaceDescriptor.InterfaceProtocol != CBI0_INTERFACE_PROTOCOL)) {
    Status = EFI_UNSUPPORTED;
  } else {
    Status = EFI_SUCCESS;
  }

  gBS->CloseProtocol(
        ControllerHandle,
        &gEfiUsbIoProtocolGuid,
        This->DriverBindingHandle,
        ControllerHandle
       );

  return Status;
}

STATIC EFI_STATUS
Cbi0DriverBindingStart (
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
  UINT8                           Index;
  EFI_USB_ENDPOINT_DESCRIPTOR     EndpointDescriptor;
  EFI_USB_INTERFACE_DESCRIPTOR    InterfaceDescriptor;
  EFI_STATUS                      Status;
  EFI_USB_IO_PROTOCOL             *UsbIo;
  UINT8                           EndpointExistMask;
  
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
    gBS->CloseProtocol (
         ControllerHandle,
         &gEfiUsbIoProtocolGuid,
         This->DriverBindingHandle,
         ControllerHandle
       );
    return Status;
  }

  Cbi0AtapiProtocol.CommandProtocol = InterfaceDescriptor.InterfaceSubClass;
  
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(USB_CBI_DEVICE),
                  &UsbCbiDev
                );

  if(EFI_ERROR(Status)) {
    gBS->CloseProtocol (
         ControllerHandle,
         &gEfiUsbIoProtocolGuid,
         This->DriverBindingHandle,
         ControllerHandle
       );
    return Status;
  }

  EfiZeroMem(UsbCbiDev, sizeof(USB_CBI_DEVICE));

  UsbCbiDev->Signature = USB_CBI_DEVICE_SIGNATURE;
  UsbCbiDev->UsbIo = UsbIo;
  UsbCbiDev->InterfaceDescriptor = InterfaceDescriptor;
  UsbCbiDev->UsbAtapiProtocol = Cbi0AtapiProtocol;
  
  //
  // Mask used to see whether all three kinds of endpoints exist,
  // Mask value:
  //  bit0: bulk in endpoint;
  //  bit1: bulk out endpoint;
  //  bit2: interrupt in endpoint;
  //
  EndpointExistMask = 0;
  for(Index = 0; Index < InterfaceDescriptor.NumEndpoints; Index ++) {
    UsbIo->UsbGetEndpointDescriptor(
              UsbIo,
              Index,
              &EndpointDescriptor
              );

    //
    // We parse bulk endpoint
    //
    if(EndpointDescriptor.Attributes == 0x02) {
      if(EndpointDescriptor.EndpointAddress & 0x80) {
        UsbCbiDev->BulkInEndpointDescriptor = EndpointDescriptor;
        EndpointExistMask |= bit(0);
      } else {
        UsbCbiDev->BulkOutEndpointDescriptor = EndpointDescriptor;
        EndpointExistMask |= bit(1);
      }
    }

    //
    // We parse interrupt endpoint
    //
    if(EndpointDescriptor.Attributes == 0x03) {
      UsbCbiDev->InterruptEndpointDescriptor = EndpointDescriptor;
      EndpointExistMask |= bit(2);
    }

  }

  //
  // Double check we have all endpoints needed
  //
  if(EndpointExistMask != (bit(0) | bit(1) | bit(2))) {
    gBS->CloseProtocol (
         ControllerHandle,
         &gEfiUsbIoProtocolGuid,
         This->DriverBindingHandle,
         ControllerHandle
       );

    if(UsbCbiDev != NULL) {
      gBS->FreePool(UsbCbiDev);
    }
    return EFI_UNSUPPORTED;
  }

  Status = gBS->InstallProtocolInterface (
                  &ControllerHandle,
                  &gEfiUsbAtapiProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &UsbCbiDev->UsbAtapiProtocol
                );
  if(EFI_ERROR(Status)) {
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

  UsbCbiDev->ControllerNameTable = NULL;
  EfiLibAddUnicodeString (
    "eng", 
    gUsbCbi0ComponentName.SupportedLanguages, 
    &UsbCbiDev->ControllerNameTable, 
    L"Usb Cbi0 Mass Storage"
  );
  
  return EFI_SUCCESS;
}

STATIC EFI_STATUS
Cbi0DriverBindingStop (
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
  EFI_USB_ATAPI_PROTOCOL        *Cbi0AtapiProtocol;
  USB_CBI_DEVICE                *UsbCbiDev;

  //
  // Get our context back.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiUsbAtapiProtocolGuid,
                  &Cbi0AtapiProtocol,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  UsbCbiDev = USB_CBI_DEVICE_FROM_THIS(Cbi0AtapiProtocol);

  //
  // Uninstall protocol
  //
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

  //
  // Free all allocated resources
  //
  if (UsbCbiDev->ControllerNameTable) {
    EfiLibFreeUnicodeStringTable (UsbCbiDev->ControllerNameTable);
  }
  
  gBS->FreePool(UsbCbiDev);

  return Status;
}


STATIC
EFI_STATUS
Cbi0RecoveryReset (
  IN  USB_CBI_DEVICE   *UsbCbiDev
  )
{
  UINT8               ResetCommand[12];
  UINT8               i;
  EFI_STATUS          Status;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               EndpointAddress;
  UINT32              Result;
  UINT16              Timeout;

  UsbIo = UsbCbiDev->UsbIo;

  //
  // CBI reset command protocol
  //
  ResetCommand[0] = 0x1d;
  ResetCommand[1] = 0x04;
  for(i = 2; i < 12; i++) {
    ResetCommand[i] = 0xff;
  }
  
  Timeout = STALL_1_SECOND; // (in millisecond unit)

  Status = Cbi0AtapiCommand (
              &UsbCbiDev->UsbAtapiProtocol,
              ResetCommand,
              12,
              NULL,
              0,
              EfiUsbNoData,
              Timeout
              );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  gBS->Stall (100000);
  //
  // clear bulk in endpoint stall feature
  //
  EndpointAddress = (UsbCbiDev->BulkInEndpointDescriptor).EndpointAddress;
  Status = UsbClearEndpointHalt(
        UsbIo,
        EndpointAddress,
        &Result
        );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  //
  // clear bulk out endpoint stall feature
  //
  EndpointAddress = (UsbCbiDev->BulkOutEndpointDescriptor).EndpointAddress;
  Status = UsbClearEndpointHalt(
    UsbIo,
    EndpointAddress,
    &Result
  );
  //
  // according to CBI spec, no need to clear interrupt endpoint feature.
  //
  return Status;
}

STATIC EFI_STATUS
Cbi0CommandPhase (
  IN  USB_CBI_DEVICE          *UsbCbiDev,
  IN  VOID                    *Command,
  IN  UINT8                   CommandSize,
  IN  UINT16                  Timeout
  )
/*++

  Routine Description:
    Send ATAPI command through CBI0 interface.

  Parameters:

  Return Values:
    EFI_SUCCESS
    Others

--*/
{
  EFI_STATUS              Status;
  UINT32                  Result;
  EFI_USB_IO_PROTOCOL     *UsbIo;
  EFI_USB_DEVICE_REQUEST  Request;

  UsbIo = UsbCbiDev->UsbIo;

  EfiZeroMem (&Request, sizeof(EFI_USB_DEVICE_REQUEST));

  //
  // Device request see CBI specification
  //
  Request.RequestType = 0x21;
  Request.Request = 0x00;
  Request.Value = 0 ;
  Request.Index = 0 ;
  Request.Length = CommandSize;
  
  Status = UsbIo->UsbControlTransfer(
              UsbIo,
              &Request,
              EfiUsbDataOut,
              Timeout,
              Command,
              CommandSize,
              &Result
            );
  if (EFI_ERROR(Status)) {    
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}


STATIC EFI_STATUS
Cbi0DataPhase (
  IN  USB_CBI_DEVICE                  *UsbCbiDev,
  IN  UINT32                          *DataSize,
  IN  OUT VOID                        *DataBuffer,
  IN  EFI_USB_DATA_DIRECTION          Direction,
  IN  UINT16                          Timeout
  )
/*++

  Routine Description:
    Get/Send Data through CBI0 interface

  Parameters:

  Return Value:
    EFI_SUCCESS
    Others

--*/
{
  EFI_STATUS          Status;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               EndpointAddress;
  UINTN               Remain;
  UINTN               Increment;
  UINT32              MaxPacketLength;
  UINT8               *BufferPtr;
  UINT32              Result;
  UINTN               TransferredSize;

  UsbIo = UsbCbiDev->UsbIo;

  Remain = *DataSize;
  BufferPtr = (UINT8 *)DataBuffer;
  TransferredSize = 0;
  //
  // retrieve the the max packet length of the given endpoint
  //
  if (Direction == EfiUsbDataIn) {
    MaxPacketLength = (UsbCbiDev->BulkInEndpointDescriptor).MaxPacketSize;
    EndpointAddress = (UsbCbiDev->BulkInEndpointDescriptor).EndpointAddress;
  } else {
    MaxPacketLength = (UsbCbiDev->BulkOutEndpointDescriptor).MaxPacketSize;
    EndpointAddress = (UsbCbiDev->BulkOutEndpointDescriptor).EndpointAddress;
  }

  while (Remain > 0) {

    if ( Remain > 16 * MaxPacketLength) {
      Increment = 16 * MaxPacketLength;
    } else {
      Increment = Remain;
    }

    Status = UsbIo->UsbBulkTransfer(
                UsbIo,
                EndpointAddress,
                BufferPtr,
                &Increment,
                Timeout,
                &Result
              );
    TransferredSize += Increment;
    
    if(EFI_ERROR(Status)) {
      goto ErrorExit;
    }

    BufferPtr += Increment;
    Remain -= Increment;
  }

  return EFI_SUCCESS;

ErrorExit:
  if((Result & EFI_USB_ERR_STALL) == EFI_USB_ERR_STALL) {
    Status = Cbi0RecoveryReset (UsbCbiDev);
    gBS->Stall (100000);
  }
  
  *DataSize = (UINT32)TransferredSize;
  return Status;
}

STATIC EFI_STATUS
Cbi0StatusPhase (
  IN  USB_CBI_DEVICE        *UsbCbiDev,
  OUT INTERRUPT_DATA_BLOCK  *InterruptDataBlock,
  IN  UINT16                Timeout
  )
/*++

  Routine Description:
    Get transfer status through BOT interface

  Parameters:

  Return Value:
    EFI_SUCCESS
    Others

--*/
{
  UINT8                 EndpointAddress;
  UINTN                 InterruptDataBlockLength;
  UINT32                Result;
  EFI_STATUS            Status;
  
  EfiZeroMem (InterruptDataBlock,sizeof (INTERRUPT_DATA_BLOCK));
  
  EndpointAddress = UsbCbiDev->InterruptEndpointDescriptor.EndpointAddress;
  InterruptDataBlockLength = sizeof (INTERRUPT_DATA_BLOCK);
  
  Status = UsbCbiDev->UsbIo->UsbSyncInterruptTransfer (
                    UsbCbiDev->UsbIo,
                    EndpointAddress,
                    InterruptDataBlock,
                    &InterruptDataBlockLength,
                    Timeout,
                    &Result
                    );
  if (EFI_ERROR(Status)) {
    if((Result & EFI_USB_STALL_ERROR) == EFI_USB_ERR_STALL) {
      //
      //just endpoint stall happens
      //
      UsbClearEndpointHalt(
        UsbCbiDev->UsbIo,
        EndpointAddress,
        &Result
      );
      gBS->Stall (100000);
    }
    return Status;
  }
  
  return EFI_SUCCESS;
}


//
// Cbi0 Atapi Protocol Implementation
//
STATIC
EFI_STATUS
Cbi0MassStorageReset (
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
  EFI_STATUS          Status;
  EFI_USB_IO_PROTOCOL *UsbIo;
  USB_CBI_DEVICE      *UsbCbiDev;

  UsbCbiDev = USB_CBI_DEVICE_FROM_THIS(This);
  UsbIo = UsbCbiDev->UsbIo;
  
  if (ExtendedVerification) {
//    UsbIo->UsbPortReset(UsbIo);
  }

  Status = Cbi0RecoveryReset (UsbCbiDev);
  return Status;
}


STATIC EFI_STATUS
Cbi0AtapiCommand (
  IN  EFI_USB_ATAPI_PROTOCOL      *This,
  IN  VOID                        *Command,
  IN  UINT8                       CommandSize,
  IN  VOID                        *DataBuffer,
  IN  UINT32                      BufferLength,
  IN  EFI_USB_DATA_DIRECTION      Direction,
  IN  UINT16                      TimeOutInMilliSeconds
  )
/*++

  Routine Description:
    Send ATAPI command using BOT protocol.

  Arguments:
    This                  - Protocol instance pointer.
    Command               - Command buffer
    CommandSize           - Size of Command Buffer
    DataBuffer            - Data buffer
    BufferLength          - Length of Data buffer
    Direction             - Data direction of this command
    TimeoutInMilliseconds - Timeout value in ms

  Returns:
    EFI_SUCCES          - Commond succeeded.
    EFI_DEVICE_ERROR    - Command failed.

--*/
{
  EFI_STATUS              Status;
  USB_CBI_DEVICE          *UsbCbiDev;
  UINT32                  BufferSize;
  INTERRUPT_DATA_BLOCK    InterruptDataBlock;
  EFI_STATUS              DataPhaseStatus;
  
  if (Direction != EfiUsbNoData) {
    if (DataBuffer == NULL || BufferLength == 0) {
      return EFI_INVALID_PARAMETER;
    }
  }
  
  DataPhaseStatus = EFI_SUCCESS;
  //
  // Get the context
  //
  UsbCbiDev = USB_CBI_DEVICE_FROM_THIS(This);

  //
  // First send ATAPI command through Cbi
  //
  Status = Cbi0CommandPhase(
              UsbCbiDev,
              Command,
              CommandSize,
              TimeOutInMilliSeconds
           );
  if(EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Send/Get Data if there is a Data Stage
  //
  switch (Direction) {
    case EfiUsbDataIn:
    case EfiUsbDataOut:      
      BufferSize = BufferLength;
    
      DataPhaseStatus = Cbi0DataPhase(
                      UsbCbiDev,
                      &BufferSize,
                      DataBuffer,
                      Direction,
                      TimeOutInMilliSeconds
                      );
      break;
    
    case EfiUsbNoData:
      break ;
  }
  
  if (EFI_ERROR(DataPhaseStatus)) {
    return EFI_DEVICE_ERROR;
  }
  
  //
  // Status Phase
  //
  Status = Cbi0StatusPhase(
             UsbCbiDev,
             &InterruptDataBlock,
             TimeOutInMilliSeconds
          );
  if (EFI_ERROR(Status)) {    
    return EFI_DEVICE_ERROR;
  }
  
  if (This->CommandProtocol != EFI_USB_SUBCLASS_UFI) {
    
    if (InterruptDataBlock.bType == 0) {
    //
    // indicates command completion
    //
      switch (InterruptDataBlock.bValue & 0x03) {
        case 0:
          Status = EFI_SUCCESS;
          break;
        
        case 1:
          Status = EFI_DEVICE_ERROR;
          break;
          
        case 2:
          Status = Cbi0RecoveryReset (UsbCbiDev);
          if (EFI_ERROR(Status)) {
            UsbCbiDev->UsbIo->UsbPortReset (UsbCbiDev->UsbIo);
          }
          Status = EFI_DEVICE_ERROR;
        
        case 3:
          //
          // issue a Request Sense Command, will implement later...
          //
          Status = EFI_DEVICE_ERROR;
      }
    } else {
      Status = DataPhaseStatus;
    }
  
  } else {
  //
  // UFI device, InterruptDataBlock.bType: ASC (Additional Sense Code)
  //             InterruptDataBlock.bValue: ASCQ (Additional Snese Code Qualifier)
  //
    Status = DataPhaseStatus;
  }
  return Status;
}