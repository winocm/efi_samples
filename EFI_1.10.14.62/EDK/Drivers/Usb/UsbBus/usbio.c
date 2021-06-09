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

    UsbIo.c

  Abstract:

    USB I/O Abstraction Driver

  Revision History

--*/

#include "Efi.h"
#include "EfiDriverLib.h"

#include EFI_PROTOCOL_DEFINITION(UsbIo)

#include "usb.h"
#include "usbbus.h"
#include "usblib.h"

//
// USB I/O Support Function Prototypes
//
STATIC
EFI_STATUS
UsbControlTransfer(
  IN EFI_USB_IO_PROTOCOL      *This,
  IN EFI_USB_DEVICE_REQUEST   *Request,
  IN EFI_USB_DATA_DIRECTION   Direction,
  IN UINT32                   Timeout,
  IN OUT VOID                 *Data OPTIONAL,
  IN UINTN                    DataLength OPTIONAL,
  OUT UINT32                  *Status
);

STATIC
EFI_STATUS
UsbBulkTransfer (
  IN EFI_USB_IO_PROTOCOL      *This,
  IN UINT8                    DeviceEndpoint,
  IN OUT VOID                 *Data,
  IN OUT UINTN                *DataLength,
  IN UINTN                    Timeout,
  OUT UINT32                  *Status
);

STATIC
EFI_STATUS
UsbAsyncInterruptTransfer (
  IN EFI_USB_IO_PROTOCOL              *This,
  IN UINT8                            DeviceEndpoint,
  IN BOOLEAN                          IsNewTransfer,
  IN UINTN                            PollingInterval OPTIONAL,
  IN UINTN                            DataLength OPTIONAL,
  IN EFI_ASYNC_USB_TRANSFER_CALLBACK  InterruptCallBack OPTIONAL,
  IN VOID                             *Context
);

STATIC
EFI_STATUS
UsbSyncInterruptTransfer (
  IN EFI_USB_IO_PROTOCOL      *This,
  IN UINT8                    DeviceEndpoint,
  IN OUT VOID                 *Data,
  IN OUT UINTN                *DataLength,
  IN UINTN                    Timeout,
  OUT UINT32                  *Status
);

STATIC
EFI_STATUS
UsbIsochronousTransfer (
  IN EFI_USB_IO_PROTOCOL      *This,
  IN UINT8                    DeviceEndpoint,
  IN OUT VOID                 *Data,
  IN UINTN                    DataLength,
  OUT UINT32                  *Status
);

STATIC
EFI_STATUS
UsbAsyncIsochronousTransfer (
  IN EFI_USB_IO_PROTOCOL              *This,
  IN UINT8                            DeviceEndpoint,
  IN OUT VOID                         *Data,
  IN OUT UINTN                        DataLength,
  IN EFI_ASYNC_USB_TRANSFER_CALLBACK  IsochronousCallBack,
  IN VOID                             *Context   OPTIONAL
);


extern
EFI_STATUS
UsbPortReset (
  IN EFI_USB_IO_PROTOCOL      *This
);

STATIC
EFI_STATUS
NewUsbGetDeviceDescriptor (
  IN  EFI_USB_IO_PROTOCOL            *This,
  OUT EFI_USB_DEVICE_DESCRIPTOR      *DeviceDescriptor
);

STATIC
EFI_STATUS
NewUsbGetActiveConfigDescriptor (
  IN  EFI_USB_IO_PROTOCOL            *This,
  OUT EFI_USB_CONFIG_DESCRIPTOR      *ConfigurationDescriptor
);

STATIC
EFI_STATUS
NewUsbGetInterfaceDescriptor (
  IN  EFI_USB_IO_PROTOCOL            *This,
  OUT EFI_USB_INTERFACE_DESCRIPTOR   *InterfaceDescriptor
);

STATIC
EFI_STATUS
NewUsbGetEndpointDescriptor (
  IN  EFI_USB_IO_PROTOCOL            *This,
  IN  UINT8                          EndpointIndex,
  OUT EFI_USB_ENDPOINT_DESCRIPTOR    *EndpointDescriptor
);

STATIC
EFI_STATUS
UsbGetStringDescriptor (
  IN  EFI_USB_IO_PROTOCOL           *This,
  IN  UINT16                        LangID,
  IN  UINT8                         StringIndex,
  OUT CHAR16                        **String
);

STATIC
EFI_STATUS
UsbGetSupportedLanguages (
  IN  EFI_USB_IO_PROTOCOL           *This,
  OUT UINT16                        **LangIDTable,
  OUT UINT16                        *TableSize
);

//
// USB I/O Interface structure
//
STATIC EFI_USB_IO_PROTOCOL UsbIoInterface = {
    UsbControlTransfer,
    UsbBulkTransfer,
    UsbAsyncInterruptTransfer,
    UsbSyncInterruptTransfer,
    UsbIsochronousTransfer,
    UsbAsyncIsochronousTransfer,
    NewUsbGetDeviceDescriptor,
    NewUsbGetActiveConfigDescriptor,
    NewUsbGetInterfaceDescriptor,
    NewUsbGetEndpointDescriptor,
    UsbGetStringDescriptor,
    UsbGetSupportedLanguages,
    UsbPortReset
};

VOID
InitializeUsbIoInstance (
  IN USB_IO_CONTROLLER_DEVICE *UsbIoController
  )
{
  //
  // Copy EFI_USB_IO protocol instance
  //
  EfiCopyMem (
    &UsbIoController->UsbIo,
    &UsbIoInterface,
    sizeof(EFI_USB_IO_PROTOCOL)
  );
}

//
// Implementation
//
STATIC
EFI_STATUS
UsbControlTransfer(
  IN EFI_USB_IO_PROTOCOL      *This,
  IN EFI_USB_DEVICE_REQUEST   *Request,
  IN EFI_USB_DATA_DIRECTION   Direction,
  IN UINT32                   Timeout,
  IN OUT VOID                 *Data OPTIONAL,
  IN UINTN                    DataLength OPTIONAL,
  OUT UINT32                  *Status
)
/*++

  Routine Description:
    This function is used to manage a USB device with a control transfer pipe.

  Arguments:
    This        -   Indicates calling context.
    Request     -   A pointer to the USB device request that will be sent to
                    the USB device.
    Direction   -   Indicates the data direction.
    Data        -   A pointer to the buffer of data that will be transmitted
                    to USB device or received from USB device.
    Timeout     -   Indicates the transfer should be completed within this time
                    frame.
    DataLength  -   The size, in bytes, of the data buffer specified by Data.
    Status      -   A pointer to the result of the USB transfer.

  Returns:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER
    EFI_OUT_OF_RESOURCES
    EFI_TIMEOUT
    EFI_DEVICE_ERROR

--*/
{
  USB_IO_CONTROLLER_DEVICE      *UsbIoController;
  EFI_USB_HC_PROTOCOL           *UsbHCInterface;
  EFI_STATUS                    RetStatus;
  USB_IO_DEVICE                 *UsbIoDevice;
  UINT8                         MaxPacketLength;
  UINT32                        TransferResult;
  
  //
  // Parameters Checking
  //
  if (Status == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // leave the HostController's ControlTransfer 
  // to perform other parameters checking
  //

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS(This);
  UsbIoDevice = UsbIoController->UsbDevice;
  UsbHCInterface = UsbIoDevice->BusController->UsbHCInterface;
  MaxPacketLength = UsbIoDevice->DeviceDescriptor.MaxPacketSize0;

  //
  // using HostController's ControlTransfer to complete the request
  //
  RetStatus = UsbHCInterface->ControlTransfer (
                                UsbHCInterface,
                                UsbIoDevice->DeviceAddress,
                                UsbIoDevice->IsSlowDevice,
                                MaxPacketLength,
                                Request,
                                Direction,
                                Data,
                                &DataLength,
                                (UINTN)Timeout,
                                &TransferResult
                               );
  *Status = TransferResult;

  return RetStatus;
}

STATIC
EFI_STATUS
UsbBulkTransfer (
  IN EFI_USB_IO_PROTOCOL      *This,
  IN UINT8                    DeviceEndpoint,
  IN OUT VOID                 *Data,
  IN OUT UINTN                *DataLength,
  IN UINTN                    Timeout,
  OUT UINT32                  *Status
)
/*++

  Routine Description:
    This function is used to manage a USB device with the bulk transfer pipe.

  Arguments:
    This            - Indicates calling context.
    DeviceEndpoint  - The destination USB device endpoint to which the device
                      request is being sent.
    Data            - A pointer to the buffer of data that will be transmitted
                      to USB device or received from USB device.
    DataLength      - On input, the size, in bytes, of the data buffer
                      specified by Data.  On output, the number of bytes that
                      were actually transferred.
    Timeout         - Indicates the transfer should be completed within this
                      time frame.
    Status          - This parameter indicates the USB transfer status.

  Return Values:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER
    EFI_OUT_OF_RESOURCES
    EFI_TIMEOUT
    EFI_DEVICE_ERROR

--*/
{
  USB_IO_DEVICE               *UsbIoDev;
  UINT8                       MaxPacketLength;
  UINT8                       DataToggle, OldToggle;
  EFI_STATUS                  RetStatus;
  EFI_USB_HC_PROTOCOL         *UsbHCInterface;
  USB_IO_CONTROLLER_DEVICE    *UsbIoController;
  UINT32                      TransferResult;

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS(This);
  UsbIoDev = UsbIoController->UsbDevice;
  UsbHCInterface = UsbIoDev->BusController->UsbHCInterface;

  //
  // Parameters Checking
  //
  if ((DeviceEndpoint & 0x7F) == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DeviceEndpoint & 0x7F) > 15) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (Status == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // leave the HostController's BulkTransfer 
  // to perform other parameters checking
  //

  GetDeviceEndPointMaxPacketLength (
    UsbIoController,
    DeviceEndpoint,
    &MaxPacketLength
  );

  GetDataToggleBit (
    UsbIoController,
    DeviceEndpoint,
    &DataToggle
  );

  OldToggle = DataToggle;

  //
  // using HostController's BulkTransfer to complete the request
  //
  RetStatus = UsbHCInterface->BulkTransfer (
                                UsbHCInterface,
                                UsbIoDev->DeviceAddress,
                                DeviceEndpoint,
                                MaxPacketLength,                                
                                Data,
                                DataLength,
                                &DataToggle,
                                Timeout,
                                &TransferResult
                              );

  if (OldToggle != DataToggle) {
    //
    // Write the toggle back
    //
    SetDataToggleBit (
      UsbIoController,
      DeviceEndpoint,
      DataToggle
    );
  }

  *Status = TransferResult;

  return RetStatus;
}

STATIC
EFI_STATUS
UsbSyncInterruptTransfer (
  IN EFI_USB_IO_PROTOCOL      *This,
  IN UINT8                    DeviceEndpoint,
  IN OUT VOID                 *Data,
  IN OUT UINTN                *DataLength,
  IN UINTN                    Timeout,
  OUT UINT32                  *Status
)
/*++

  Routine Description:
    Usb Sync Interrupt Transfer

  Arguments:
    This            - Indicates calling context.
    DeviceEndpoint  - The destination USB device endpoint to which the device
                      request is being sent.
    Data            - A pointer to the buffer of data that will be transmitted
                      to USB device or received from USB device.
    DataLength      - On input, the size, in bytes, of the data buffer
                      specified by Data.  On output, the number of bytes that
                      were actually transferred.
    Timeout         - Indicates the transfer should be completed within this
                      time frame.
    Status          - This parameter indicates the USB transfer status.

  Return Values:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER
    EFI_OUT_OF_RESOURCES
    EFI_TIMEOUT
    EFI_DEVICE_ERROR

--*/
{
  USB_IO_DEVICE               *UsbIoDev;
  UINT8                       MaxPacketLength;
  UINT8                       DataToggle;
  UINT8                       OldToggle;
  EFI_STATUS                  RetStatus;
  EFI_USB_HC_PROTOCOL         *UsbHCInterface;
  USB_IO_CONTROLLER_DEVICE    *UsbIoController;

  //
  // Parameters Checking
  //
  if ((DeviceEndpoint & 0x7F) == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DeviceEndpoint & 0x7F) > 15) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (Status == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // leave the HostController's SyncInterruptTransfer 
  // to perform other parameters checking
  //

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS(This);
  UsbIoDev = UsbIoController->UsbDevice;
  UsbHCInterface = UsbIoDev->BusController->UsbHCInterface;

  GetDeviceEndPointMaxPacketLength (
    UsbIoController,
    DeviceEndpoint,
    &MaxPacketLength
  );

  GetDataToggleBit (
    UsbIoController,
    DeviceEndpoint,
    &DataToggle
  );

  OldToggle = DataToggle;
  //
  // using HostController's SyncInterruptTransfer to complete the request
  //
  RetStatus = UsbHCInterface->SyncInterruptTransfer (
                                UsbHCInterface,
                                UsbIoDev->DeviceAddress,
                                DeviceEndpoint,                                
                                UsbIoDev->IsSlowDevice,
                                MaxPacketLength,
                                Data,
                                DataLength,
                                &DataToggle,
                                Timeout,
                                Status
                              );

  if (OldToggle != DataToggle) {
    //
    // Write the toggle back
    //    
    SetDataToggleBit (
      UsbIoController,
      DeviceEndpoint,
      DataToggle
    );
  }

  return RetStatus;
}

STATIC
EFI_STATUS
UsbAsyncInterruptTransfer (
  IN EFI_USB_IO_PROTOCOL              *This,
  IN UINT8                            DeviceEndpoint,
  IN BOOLEAN                          IsNewTransfer,
  IN UINTN                            PollingInterval OPTIONAL,
  IN UINTN                            DataLength OPTIONAL,
  IN EFI_ASYNC_USB_TRANSFER_CALLBACK  InterruptCallBack OPTIONAL,
  IN VOID                             *Context
)
/*++

  Routine Description:
    Usb Async Interrput Transfer

  Arguments:
    This              -   Indicates calling context.
    DeviceEndpoint    -   The destination USB device endpoint to which the
                          device request is being sent.
    IsNewTransfer     -   If TRUE, a new transfer will be submitted to USB
                          controller.  If FALSE,  the interrupt transfer is
                          deleted from the device's interrupt transfer queue.
    PollingInterval   -   Indicates the periodic rate, in milliseconds, that
                          the transfer is to be executed.
    DataLength        -   Specifies the length, in bytes, of the data to be
                          received from the USB device.
    Context           -   Data passed to the InterruptCallback function.
    InterruptCallback -   The Callback function.  This function is called if
                          the asynchronous interrupt transfer is completed.

  Returns:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER
    EFI_OUT_OF_RESOURCES

--*/
{
  USB_IO_DEVICE               *UsbIoDev;
  UINT8                       MaxPacketLength;
  UINT8                       DataToggle;
  EFI_USB_HC_PROTOCOL         *UsbHCInterface;
  EFI_STATUS                  RetStatus;
  USB_IO_CONTROLLER_DEVICE    *UsbIoController;

  //
  // Check endpoint
  //
  if ((DeviceEndpoint & 0x7F) == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DeviceEndpoint & 0x7F) > 15) {
    return EFI_INVALID_PARAMETER;
  }

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS(This);
  UsbIoDev = UsbIoController->UsbDevice;
  UsbHCInterface = UsbIoDev->BusController->UsbHCInterface;

  if (!IsNewTransfer) {
    //
    // Delete this transfer
    //
    UsbHCInterface->AsyncInterruptTransfer (
                      UsbHCInterface,
                      UsbIoDev->DeviceAddress,
                      DeviceEndpoint,
                      UsbIoDev->IsSlowDevice,
                      0,
                      FALSE,
                      &DataToggle,
                      PollingInterval,
                      DataLength,
                      NULL,
                      NULL
                     );

    //
    // We need to store the toggle value
    //
    SetDataToggleBit (
      UsbIoController,
      DeviceEndpoint,
      DataToggle
    );

    return EFI_SUCCESS;
  }

  GetDeviceEndPointMaxPacketLength (
    UsbIoController,
    DeviceEndpoint,
    &MaxPacketLength
  );

  GetDataToggleBit (
    UsbIoController,
    DeviceEndpoint,
    &DataToggle
  );

  RetStatus = UsbHCInterface->AsyncInterruptTransfer (
                                UsbHCInterface,
                                UsbIoDev->DeviceAddress,
                                DeviceEndpoint,
                                UsbIoDev->IsSlowDevice,
                                MaxPacketLength,
                                TRUE,
                                &DataToggle,
                                PollingInterval,
                                DataLength,
                                InterruptCallBack,
                                Context
                              );

  return RetStatus;
}

STATIC
EFI_STATUS
UsbIsochronousTransfer (
  IN EFI_USB_IO_PROTOCOL      *This,
  IN UINT8                    DeviceEndpoint,
  IN OUT VOID                 *Data,
  IN UINTN                    DataLength,
  OUT UINT32                  *Status
)
/*++

  Routine Description:
    Usb Isochronous Transfer

  Arguments:
    This              -   Indicates calling context.
    DeviceEndpoint    -   The destination USB device endpoint to which the
                          device request is being sent.
    Data              -   A pointer to the buffer of data that will be
                          transmitted to USB device or received from USB device.
    DataLength        -   The size, in bytes, of the data buffer specified by
                          Data.
    Status            -   This parameter indicates the USB transfer status.

  Returns:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER
    EFI_OUT_OF_RESOURCES
    EFI_TIMEOUT
    EFI_DEVICE_ERROR

--*/
{
    //
    // Currently we don't support this transfer
    //
    return EFI_UNSUPPORTED;
}

EFI_STATUS
UsbAsyncIsochronousTransfer (
  IN EFI_USB_IO_PROTOCOL              *This,
  IN UINT8                            DeviceEndpoint,
  IN OUT VOID                         *Data,
  IN OUT UINTN                        DataLength,
  IN EFI_ASYNC_USB_TRANSFER_CALLBACK  IsochronousCallBack,
  IN VOID                             *Context   OPTIONAL
  )
{
  //
  // Currently we don't support this transfer
  //
  return EFI_UNSUPPORTED;
}

//
// Here is new definitions
//
STATIC
EFI_STATUS
NewUsbGetDeviceDescriptor (
  IN  EFI_USB_IO_PROTOCOL            *This,
  OUT EFI_USB_DEVICE_DESCRIPTOR      *DeviceDescriptor
)
/*++

  Rountine Description:
    Retrieves the USB Device Descriptor.

  Parameters:
    This              -   Indicates the calling context.
    DeviceDescriptor  -   A pointer to the caller allocated USB Device
                          Descriptor.

  Return Value:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER
    EFI_NOT_FOUND

--*/
{
  EFI_USB_DEVICE_DESCRIPTOR   *UsbDeviceDescriptor;
  EFI_STATUS                  Status;

  //
  // This function just wrapps UsbGetDeviceDescriptor.
  // 
  
  if (DeviceDescriptor == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = UsbGetDeviceDescriptor (This, &UsbDeviceDescriptor);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  EfiCopyMem (
    DeviceDescriptor,
    UsbDeviceDescriptor,
    sizeof(EFI_USB_DEVICE_DESCRIPTOR)
  );

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
NewUsbGetActiveConfigDescriptor (
  IN  EFI_USB_IO_PROTOCOL            *This,
  OUT EFI_USB_CONFIG_DESCRIPTOR      *ConfigurationDescriptor
)
/*++

  Rountine Description:
    Retrieves the current USB configuration Descriptor.

  Parameters:
    This                     -   Indicates the calling context.
    ConfigurationDescriptor  -   A pointer to the caller allocated USB active
                                 Configuration Descriptor.

  Return Value:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER
    EFI_NOT_FOUND

--*/
{
  EFI_USB_CONFIG_DESCRIPTOR   *UsbConfigDescriptor;
  EFI_STATUS                  Status;

  //
  // This function just wrapps UsbGetActiveConfigDescriptor.
  // 
  if (ConfigurationDescriptor == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = UsbGetActiveConfigDescriptor (This, &UsbConfigDescriptor);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  EfiCopyMem (
    ConfigurationDescriptor,
    UsbConfigDescriptor,
    sizeof(EFI_USB_CONFIG_DESCRIPTOR)
  );

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
NewUsbGetInterfaceDescriptor (
  IN  EFI_USB_IO_PROTOCOL            *This,
  OUT EFI_USB_INTERFACE_DESCRIPTOR   *InterfaceDescriptor
)
/*++

  Rountine Description:
    Retrieves the interface Descriptor for that controller.

  Parameters:
    This                  -   Indicates the calling context.
    InterfaceDescriptor   -   A pointer to the caller allocated USB interface
                              Descriptor.

  Return Value:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER
    EFI_NOT_FOUND

--*/
{
  EFI_USB_INTERFACE_DESCRIPTOR  *UsbInterfaceDescriptor;
  EFI_STATUS                    Status;

  //
  // This function just wrapps UsbGetInterfaceDescriptor.
  // 
  if (InterfaceDescriptor == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = UsbGetInterfaceDescriptor (This, &UsbInterfaceDescriptor);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  EfiCopyMem (
    InterfaceDescriptor,
    UsbInterfaceDescriptor,
    sizeof(EFI_USB_INTERFACE_DESCRIPTOR)
  );

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
NewUsbGetEndpointDescriptor (
  IN  EFI_USB_IO_PROTOCOL            *This,
  IN  UINT8                          EndpointIndex,
  OUT EFI_USB_ENDPOINT_DESCRIPTOR    *EndpointDescriptor
)
/*++

  Rountine Description:
    Retrieves the endpoint Descriptor for a given endpoint.

  Parameters:
    This                -   Indicates the calling context.
    EndpointIndex       -   Indicates which endpoint descriptor to retrieve.
                            The valid range is 0..15.
    EndpointDescriptor  -   A pointer to the caller allocated USB Endpoint
                            Descriptor of a USB controller.

  Return Value:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER
    EFI_NOT_FOUND

--*/
{
  EFI_USB_ENDPOINT_DESCRIPTOR  *UsbEndpointDescriptor;
  EFI_STATUS                   Status;

  //
  // This function just wrapps UsbGetEndpointDescriptor.
  // 

  if (EndpointDescriptor == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (EndpointIndex > 15) {
    return EFI_INVALID_PARAMETER;
  }

  Status = UsbGetEndpointDescriptor (
              This,
              EndpointIndex,
              &UsbEndpointDescriptor
           );

  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  EfiCopyMem (
    EndpointDescriptor,
    UsbEndpointDescriptor,
    sizeof(EFI_USB_ENDPOINT_DESCRIPTOR)
  );

  return EFI_SUCCESS;

}

STATIC
EFI_STATUS
UsbGetSupportedLanguages (
  IN  EFI_USB_IO_PROTOCOL           *This,
  OUT UINT16                        **LangIDTable,
  OUT UINT16                        *TableSize
)
/*++

  Routine Description:
    Get all the languages that the USB device supports

  Arguments:
    This        -   Indicates the calling context.
    LangIDTable -   Language ID for the string the caller wants to get.
    TableSize   -   The size, in bytes, of the table LangIDTable.

  Returns:
    EFI_SUCCESS
    EFI_NOT_FOUND

--*/
{
  USB_IO_DEVICE               *UsbIoDev;
  USB_IO_CONTROLLER_DEVICE    *UsbIoController;
  UINTN                       i;
  BOOLEAN                     Found;

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS(This);
  UsbIoDev = UsbIoController->UsbDevice;

  Found = FALSE;
  i = 0;
  //
  // Loop language table
  //
  while(UsbIoDev->LangID[i]) {
    Found = TRUE;
    i++;
  }

  if (!Found) {
    return EFI_NOT_FOUND;
  }

  *LangIDTable = UsbIoDev->LangID;
  *TableSize = (UINT16)i;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
UsbGetStringDescriptor (
  IN  EFI_USB_IO_PROTOCOL   *This,
  IN  UINT16                LangID,
  IN  UINT8                 StringIndex,
  OUT CHAR16                **String
)
/*++

  Routine Description:
    Get a given string descriptor

  Arguments:
    This          -   Indicates the calling context.
    LangID        -   The Language ID for the string being retrieved.
    StringID      -   The ID of the string being retrieved.
    String        -   A pointer to a buffer allocated by this function
                      with AllocatePool() to store the string.  If this
                      function returns EFI_SUCCESS, it stores the string
                      the caller wants to get.  The caller should release
                      the string buffer with FreePool() after the string
                      is not used any more.
  Returns:
    EFI_SUCCESS
    EFI_NOT_FOUND
    EFI_OUT_OF_RESOURCES

--*/
{
  UINT32                      Status;
  EFI_STATUS                  Result;
  EFI_USB_STRING_DESCRIPTOR   *StrDescriptor;
  UINT8                       *Buffer;
  CHAR16                      *UsbString;
  UINT16                      TempBuffer;
  USB_IO_DEVICE               *UsbIoDev;
  UINT8                       i;
  BOOLEAN                     Found;
  USB_IO_CONTROLLER_DEVICE    *UsbIoController;

  if (StringIndex == 0) {
    return EFI_NOT_FOUND;
  }

  //
  // Search LanguageID, check if it is supported by this device
  //
  if (LangID == 0) {
    return EFI_NOT_FOUND;
  }

  UsbIoController = USB_IO_CONTROLLER_DEVICE_FROM_USB_IO_THIS(This);
  UsbIoDev = UsbIoController->UsbDevice;

  Found = FALSE;
  i = 0;
  while (UsbIoDev->LangID[i]) {
    if (UsbIoDev->LangID[i] == LangID) {
      Found = TRUE;
      break;
    }
    i++;
  }

  if (!Found) {
    return EFI_NOT_FOUND;
  }

  //
  // Get String Length
  //
  Result = UsbGetString(
              This,
              LangID,
              StringIndex,
              &TempBuffer,
              2,
              &Status
            );
  if(EFI_ERROR(Result)) {
    return EFI_NOT_FOUND;
  }

  StrDescriptor = (EFI_USB_STRING_DESCRIPTOR *)&TempBuffer;

  if (StrDescriptor->Length == 0) {
    return EFI_UNSUPPORTED;
  }

  Result = gBS->AllocatePool(
                  EfiBootServicesData,
                  StrDescriptor->Length,
                  &Buffer
                );

  if (EFI_ERROR (Result)) {
     return EFI_OUT_OF_RESOURCES;
  }

  EfiZeroMem (Buffer, StrDescriptor->Length);

  Result = UsbGetString(
               This,
               LangID,
               StringIndex,
               Buffer,
               StrDescriptor->Length,
               &Status
             );

  if(EFI_ERROR(Result)) {
    gBS->FreePool(Buffer);
    return EFI_NOT_FOUND;
  }

  StrDescriptor = (EFI_USB_STRING_DESCRIPTOR *)Buffer;

  //
  // UsbString is a UNICODE string
  //
  Result = gBS->AllocatePool(
                  EfiBootServicesData,
                  StrDescriptor->Length,
                  (VOID **)&UsbString
                );

  if (EFI_ERROR (Result)) {
    gBS->FreePool(Buffer);
    return EFI_OUT_OF_RESOURCES;
  }

  EfiZeroMem ((VOID *)UsbString, StrDescriptor->Length);
  EfiCopyMem (
    (VOID *)UsbString,
    Buffer + 2,
    StrDescriptor->Length - 2
  );

  *String = UsbString;

  gBS->FreePool (Buffer);
  
  return EFI_SUCCESS;
}

