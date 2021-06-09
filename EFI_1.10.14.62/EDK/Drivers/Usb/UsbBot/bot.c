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

  BOT.c

Abstract:

--*/

#include "Efi.h"
#include "EfiDriverLib.h"
#include "usb.h"

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(DriverBinding)
#include EFI_PROTOCOL_DEFINITION(UsbIo)


//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION(UsbAtapi)

#include "usblib.h"
#include "bot.h"

//
// Function prototypes
//
EFI_STATUS
UsbBotDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Bot Driver Binding Protocol
//
static EFI_STATUS
BotDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

static EFI_STATUS
BotDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

static EFI_STATUS
BotDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

EFI_DRIVER_BINDING_PROTOCOL gUsbBotDriverBinding = {
  BotDriverBindingSupported,
  BotDriverBindingStart,
  BotDriverBindingStop,
  0x10,
  NULL,
  NULL
};

//
// Bot Protocol
//
static EFI_STATUS
BotCommandPhase (
  IN  USB_BOT_DEVICE            *UsbBotDev,
  IN  VOID                      *Command,
  IN  UINT8                     CommandSize,
  IN  UINT32                    DataTransferLength,
  IN  EFI_USB_DATA_DIRECTION    Direction,
  IN  UINT16                    Timeout
);

static EFI_STATUS
BotDataPhase (
  IN  USB_BOT_DEVICE          *UsbBotDev,
  IN  UINT32                  *DataSize,
  IN  OUT VOID                *DataBuffer,
  IN  EFI_USB_DATA_DIRECTION  Direction,
  IN  UINT16                  Timeout
);

static EFI_STATUS
BotStatusPhase (
  IN  USB_BOT_DEVICE      *UsbBotDev,
  OUT UINT8               *TransferStatus,
  IN  UINT16              Timeout
);

//
// USB Atapi protocol prototype
//
static EFI_STATUS
BotAtapiCommand (
  IN  EFI_USB_ATAPI_PROTOCOL    *This,
  IN  VOID                      *Command,
  IN  UINT8                     CommandSize,
  IN  VOID                      *DataBuffer,
  IN  UINT32                    BufferLength,
  IN  EFI_USB_DATA_DIRECTION    Direction,
  IN  UINT16                    TimeOutInMilliSeconds
);

static EFI_STATUS
BotMassStorageReset (
  IN  EFI_USB_ATAPI_PROTOCOL    *This,
  IN  BOOLEAN                   ExtendedVerification
);

static EFI_USB_ATAPI_PROTOCOL BotAtapiProtocol = {
  BotAtapiCommand,
  BotMassStorageReset,
  0
};


EFI_DRIVER_ENTRY_POINT(UsbBotDriverEntryPoint)

EFI_STATUS
UsbBotDriverEntryPoint(
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
           &gUsbBotDriverBinding, 
           ImageHandle,
           &gUsbBotComponentName,
           NULL,
           NULL
         );
}

static EFI_STATUS
BotDriverBindingSupported (
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
    goto Exit;
  }
  //
  // Check if it is a BOT type Mass Storage Device
  //
  if((InterfaceDescriptor.InterfaceClass != 0x08)
    || (InterfaceDescriptor.InterfaceProtocol != BOT))
  {
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
BotDriverBindingStart (
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
  USB_BOT_DEVICE                *UsbBotDev;
  UINT8                         i;
  EFI_USB_ENDPOINT_DESCRIPTOR   *EndpointDescriptor;
  EFI_USB_INTERFACE_DESCRIPTOR  *InterfaceDescriptor;
  EFI_STATUS                    Status;
  EFI_USB_IO_PROTOCOL           *UsbIo;

  //
  // Check if the Controller supports USB IO protocol
  //
  UsbBotDev = NULL;

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

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof(EFI_USB_INTERFACE_DESCRIPTOR),
                  &InterfaceDescriptor
                );

  if (EFI_ERROR(Status)) {
    gBS->CloseProtocol (
         ControllerHandle,
         &gEfiUsbIoProtocolGuid,
         This->DriverBindingHandle,
         ControllerHandle
       );
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Get the controller interface descriptor,
  //
  Status = UsbIo->UsbGetInterfaceDescriptor(
                    UsbIo,
                    InterfaceDescriptor
                  );
  if (EFI_ERROR(Status)) {
    gBS->FreePool (InterfaceDescriptor);
    goto ErrorExit;
  }

  BotAtapiProtocol.CommandProtocol = InterfaceDescriptor->InterfaceSubClass;
  
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(USB_BOT_DEVICE),
                  &UsbBotDev
                );

  if (EFI_ERROR(Status)) {
    gBS->FreePool (InterfaceDescriptor);
    goto ErrorExit;
  }

  EfiZeroMem(UsbBotDev, sizeof(USB_BOT_DEVICE));

  UsbBotDev->Signature = USB_BOT_DEVICE_SIGNATURE;
  UsbBotDev->UsbIo = UsbIo;
  UsbBotDev->InterfaceDescriptor = InterfaceDescriptor;
  UsbBotDev->UsbAtapiProtocol = BotAtapiProtocol;

  for (i = 0; i < InterfaceDescriptor->NumEndpoints; i++) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    sizeof(EFI_USB_INTERFACE_DESCRIPTOR),
                    &EndpointDescriptor
                  );
    if (EFI_ERROR(Status)) {
      Status = EFI_OUT_OF_RESOURCES;
      goto ErrorExit;
    }

    UsbIo->UsbGetEndpointDescriptor(
            UsbIo,
            i,
            EndpointDescriptor
           );

    //
    // We parse bulk endpoint
    //
    if (EndpointDescriptor->Attributes == 0x02) {
      if ((EndpointDescriptor->EndpointAddress & 0x80) != 0) {
        UsbBotDev->BulkInEndpointDescriptor = EndpointDescriptor;
      } else {
        UsbBotDev->BulkOutEndpointDescriptor = EndpointDescriptor;
      }
      continue;
    }

    gBS->FreePool (EndpointDescriptor);
  }

  //
  // Double check we have these endpoint descriptors
  //
  if(!(UsbBotDev->BulkInEndpointDescriptor
    && UsbBotDev->BulkOutEndpointDescriptor))
  {
    Status = EFI_DEVICE_ERROR;
    goto ErrorExit;
  }

  //
  // Install Usb-Atapi Protocol onto the handle
  //
  Status = gBS->InstallProtocolInterface (
                  &ControllerHandle,
                  &gEfiUsbAtapiProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &UsbBotDev->UsbAtapiProtocol
                 );

  if(EFI_ERROR(Status)) {
    goto ErrorExit;
  }

  UsbBotDev->ControllerNameTable = NULL;
  EfiLibAddUnicodeString (
    "eng", 
    gUsbBotComponentName.SupportedLanguages, 
    &UsbBotDev->ControllerNameTable, 
    L"Usb Bot Mass Storage"
  );
    
  return EFI_SUCCESS;

ErrorExit:
  gBS->CloseProtocol (
         ControllerHandle,
         &gEfiUsbIoProtocolGuid,
         This->DriverBindingHandle,
         ControllerHandle
       );

  if (UsbBotDev != NULL) {
    if (UsbBotDev->InterfaceDescriptor != NULL) {
      gBS->FreePool (UsbBotDev->InterfaceDescriptor);
    }

    if (UsbBotDev->BulkInEndpointDescriptor != NULL) {
      gBS->FreePool (UsbBotDev->BulkInEndpointDescriptor);
    }

    if (UsbBotDev->BulkOutEndpointDescriptor != NULL) {
      gBS->FreePool (UsbBotDev->BulkOutEndpointDescriptor);
    }

    gBS->FreePool(UsbBotDev);
  }

  return Status;
}

static EFI_STATUS
BotDriverBindingStop (
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
  EFI_USB_ATAPI_PROTOCOL        *BotAtapiProtocol;
  USB_BOT_DEVICE                *UsbBotDev;

  //
  // Get our context back.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiUsbAtapiProtocolGuid,
                  &BotAtapiProtocol,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  UsbBotDev = USB_BOT_DEVICE_FROM_THIS(BotAtapiProtocol);

  //
  // Uninstall protocol
  //
  Status = gBS->UninstallProtocolInterface (
                  ControllerHandle,
                  &gEfiUsbAtapiProtocolGuid,
                  &UsbBotDev->UsbAtapiProtocol
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
  if (UsbBotDev->InterfaceDescriptor != NULL) {
    gBS->FreePool (UsbBotDev->InterfaceDescriptor);
  }

  if (UsbBotDev->BulkInEndpointDescriptor != NULL) {
    gBS->FreePool (UsbBotDev->BulkInEndpointDescriptor);
  }

  if (UsbBotDev->BulkOutEndpointDescriptor != NULL) {
    gBS->FreePool (UsbBotDev->BulkOutEndpointDescriptor);
  }

  if (UsbBotDev->ControllerNameTable) {
    EfiLibFreeUnicodeStringTable (UsbBotDev->ControllerNameTable);
  }
  
  gBS->FreePool(UsbBotDev);

  return Status;
}

STATIC
EFI_STATUS
BotRecoveryReset (
  IN  USB_BOT_DEVICE          *UsbBotDev
  )
{
  EFI_STATUS              Status;
  UINT32                  Result;
  EFI_USB_DEVICE_REQUEST  Request;
  EFI_USB_IO_PROTOCOL     *UsbIo;
  UINT8                   EndpointAddr;
  UINT32                  Timeout;

  UsbIo = UsbBotDev->UsbIo;

  EfiZeroMem ( &Request, sizeof(EFI_USB_DEVICE_REQUEST)) ;

  //
  // See BOT specification
  //
  Request.RequestType = 0x21 ;
  Request.Request = 0xFF ;
  Request.Value = 0 ;
  Request.Index = 0 ;
  Request.Length = 0 ;

  Timeout = 3000;

  Status = UsbIo->UsbControlTransfer(
                    UsbIo,
                    &Request,
                    EfiUsbNoData,
                    Timeout,
                    NULL,
                    0,
                    &Result
                  );
  
  //
  // clear bulk in endpoint stall feature
  //
  EndpointAddr = (UsbBotDev->BulkInEndpointDescriptor)->EndpointAddress;

  UsbClearEndpointHalt(
    UsbIo,
    EndpointAddr,
    &Result
  );

  //
  // clear bulk out endpoint stall feature
  //
  EndpointAddr = (UsbBotDev->BulkOutEndpointDescriptor)->EndpointAddress;
  UsbClearEndpointHalt(
    UsbIo,
    EndpointAddr,
    &Result
  );

  return Status;
}

//
// Bot Protocol Implementation
//
static EFI_STATUS
BotCommandPhase (
  IN  USB_BOT_DEVICE          *UsbBotDev,
  IN  VOID                    *Command,
  IN  UINT8                   CommandSize,
  IN  UINT32                  DataTransferLength,
  IN  EFI_USB_DATA_DIRECTION  Direction,
  IN  UINT16                  Timeout
  )
/*++

  Routine Description:
    Send ATAPI command through BOT interface.

  Parameters:

  Return Values:
    EFI_SUCCESS
    Others

--*/
{
  CBW                 cbw;
  EFI_STATUS          Status;
  UINT32              Result;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINTN               DataSize;

  UsbIo = UsbBotDev->UsbIo;

  EfiZeroMem ( &cbw, sizeof(CBW)) ;

  //
  // Fill the command block, detailed see BOT spec
  //
  cbw.dCBWSignature = CBWSIG ;
  cbw.dCBWTag = 0x01 ;
  cbw.dCBWDataTransferLength = DataTransferLength ;
  cbw.bmCBWFlags = (UINT8)(Direction << 7);
  cbw.bCBWLUN = 0 ;
  cbw.bCBWCBLength = CommandSize ;
  EfiCopyMem( cbw.CBWCB, Command, CommandSize ) ;

  DataSize = sizeof(CBW);

  Status = UsbIo->UsbBulkTransfer(
                    UsbIo,
                    (UsbBotDev->BulkOutEndpointDescriptor)->EndpointAddress,
                    &cbw,
                    &DataSize,
                    Timeout,
                    &Result
                  );
  if (EFI_ERROR (Status)) {
    //
    // Command phase fail, we need to recovery reset this device
    //
    BotRecoveryReset (UsbBotDev);
    return EFI_DEVICE_ERROR ;    
  }

  return EFI_SUCCESS;
}


static EFI_STATUS
BotDataPhase (
  IN  USB_BOT_DEVICE                  *UsbBotDev,
  IN  UINT32                          *DataSize,
  IN  OUT VOID                        *DataBuffer,
  IN  EFI_USB_DATA_DIRECTION          Direction,
  IN  UINT16                          Timeout
  )
/*++

  Routine Description:
    Get/Send Data through BOT interface

  Parameters:

  Return Value:
    EFI_SUCCESS
    Others

--*/
{
  EFI_STATUS          Status;
  UINT32              Result;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               EndpointAddr;
  UINTN               Remain;
  UINTN               Increment;
  UINT32              MaxPacketLen;
  UINT8               *BufferPtr;
  UINTN               TransferredSize;
  UINTN               RetryTimes;
    
  UsbIo = UsbBotDev->UsbIo;

  Remain = *DataSize;
  BufferPtr = (UINT8 *)DataBuffer;
  TransferredSize = 0;

  //
  // retrieve the the max packet length of the given endpoint
  //
  if ( Direction == EfiUsbDataIn ) {
    MaxPacketLen = (UsbBotDev->BulkInEndpointDescriptor)->MaxPacketSize;
    EndpointAddr = (UsbBotDev->BulkInEndpointDescriptor)->EndpointAddress;
  } else {
    MaxPacketLen = (UsbBotDev->BulkOutEndpointDescriptor)->MaxPacketSize;
    EndpointAddr = (UsbBotDev->BulkOutEndpointDescriptor)->EndpointAddress;
  }

  RetryTimes = 10;
  
  while ( Remain > 0 )
  {
    //
    // Using 15 packets to avoid Bitstuff error
    //
    if ( Remain > 16 * MaxPacketLen) {
      Increment = 16 * MaxPacketLen;
    } else {
      Increment = Remain ;
    }

    Status = UsbIo->UsbBulkTransfer(
                        UsbIo,
                        EndpointAddr,
                        BufferPtr,
                        &Increment,
                        Timeout,
                        &Result
                      );

    TransferredSize += Increment;
 
    if (EFI_ERROR (Status)) {
      RetryTimes --;
      if ((RetryTimes == 0) || ((Result & EFI_USB_ERR_TIMEOUT) == 0)) {
        goto ErrorExit;
      }
      TransferredSize -= Increment;
      continue;      
    } else {
      RetryTimes = 10;
    }

    BufferPtr += Increment;
    Remain -= Increment ;
  }

  *DataSize = (UINT32)TransferredSize;
   
  return EFI_SUCCESS;

ErrorExit:
  if((Result & EFI_USB_STALL_ERROR) == EFI_USB_ERR_STALL) {
    //
    //just endpoint stall happens
    //
    UsbClearEndpointHalt(
      UsbIo,
      EndpointAddr,
      &Result
    );
  }

  *DataSize = (UINT32)TransferredSize;

  return Status;

}

static EFI_STATUS
BotStatusPhase (
  IN  USB_BOT_DEVICE        *UsbBotDev,
  OUT UINT8                 *TransferStatus,
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
  CSW                     csw;
  EFI_STATUS              Status;
  UINT32                  Result;
  EFI_USB_IO_PROTOCOL     *UsbIo;
  UINT8                   EndpointAddr;
  UINTN                   DataSize;

  UsbIo = UsbBotDev->UsbIo;

  EfiZeroMem( &csw, sizeof(CSW) ) ;

  EndpointAddr = (UsbBotDev->BulkInEndpointDescriptor)->EndpointAddress;

  DataSize = sizeof(CSW);
  
  //
  // Get the status field from bulk transfer
  //
  Status = UsbIo->UsbBulkTransfer(
                    UsbIo,
                    EndpointAddr,
                    &csw,
                    &DataSize,
                    Timeout,
                    &Result
                  );
  if(EFI_ERROR(Status)) {    
    if((Result & EFI_USB_STALL_ERROR) == EFI_USB_ERR_STALL) {
      //
      //just endpoint stall happens
      //
      UsbClearEndpointHalt(
        UsbIo,
        EndpointAddr,
        &Result
      );
    }
    return Status;
  }

  if ( csw.dCSWSignature == CSWSIG )
  {
    *TransferStatus = csw.bCSWStatus ;
  }
  else
  {
    return EFI_DEVICE_ERROR ;
  }
  return EFI_SUCCESS;
}


//
// Usb Atapi Protocol implementation
//
static EFI_STATUS
BotAtapiCommand (
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
  EFI_STATUS              BotDataStatus = EFI_SUCCESS;
  UINT8                   TransferStatus;
  USB_BOT_DEVICE          *UsbBotDev;
  UINT32                  BufferSize;
  
  //
  // Get the context
  //
  UsbBotDev = USB_BOT_DEVICE_FROM_THIS(This);

  //
  // First send ATAPI command through Bot
  //
  Status = BotCommandPhase(
              UsbBotDev,
              Command,
              CommandSize,
              BufferLength,
              Direction,
              TimeOutInMilliSeconds
           );

  if(EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Send/Get Data if there is a Data Stage
  //
  switch (Direction)
  {
  case EfiUsbDataIn:
  case EfiUsbDataOut:
    BufferSize = BufferLength;
    
    BotDataStatus = BotDataPhase(
                      UsbBotDev,
                      &BufferSize,
                      DataBuffer,
                      Direction,
                      TimeOutInMilliSeconds
                    );
    break;
    
  case EfiUsbNoData:
    break ;
  }
  
  //
  // Status Phase
  //
  Status = BotStatusPhase(
             UsbBotDev,
             &TransferStatus,
             TimeOutInMilliSeconds
          );
  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
  
  if (TransferStatus == 0x02) {
    //
    // Phase error
    //
    BotRecoveryReset (UsbBotDev);
    return EFI_DEVICE_ERROR;
  }
  
  if (TransferStatus == 0x01) {
    return EFI_DEVICE_ERROR;
  }
  
  return BotDataStatus;
}

static EFI_STATUS
BotMassStorageReset (
  IN  EFI_USB_ATAPI_PROTOCOL      *This,
  IN  BOOLEAN                     ExtendedVerification
)
/*++

  Routine Description:
    Reset Bot Devices

  Arguments:
    This                    - Protocol instance pointer.
    ExtendedVerification    - TRUE if we need to do strictly reset.

  Returns:
    EFI_SUCCES          - Commond succeeded.
    EFI_DEVICE_ERROR    - Command failed.

--*/
{
  EFI_STATUS              Status;
  USB_BOT_DEVICE          *UsbBotDev;
  EFI_USB_IO_PROTOCOL     *UsbIo;

  UsbBotDev = USB_BOT_DEVICE_FROM_THIS(This);
  UsbIo = UsbBotDev->UsbIo;

  if(ExtendedVerification) {
    //
    // If we need to do strictly reset, reset its parent hub port
    //
    Status = UsbIo->UsbPortReset(UsbIo);
//    if (EFI_ERROR(Status)) {
    return Status;
//    }
  }

  Status = BotRecoveryReset (UsbBotDev);
  
  return Status;
}

