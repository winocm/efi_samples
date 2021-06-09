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

  WinNtSerialIo.c

Abstract:

  Our DriverBinding member functions operate on the handles
  created by the NT Bus driver.

  Handle(1) - WinNtIo - DevicePath(1)

  If a serial port is added to the system this driver creates a new handle. 
  The new handle is required, since the serial device must add an UART device
  pathnode.

  Handle(2) - SerialIo - DevicePath(1)\UART

  The driver then adds a gEfiWinNtSerialPortGuid as a protocol to Handle(1).
  The instance data for this protocol is the private data used to create
  Handle(2).

  Handle(1) - WinNtIo - DevicePath(1) - WinNtSerialPort

  If the driver is unloaded Handle(2) is removed from the system and 
  gEfiWinNtSerialPortGuid is removed from Handle(1).

  Note: Handle(1) is any handle created by the Win NT Bus driver that is passed
  into the DriverBinding member functions of this driver. This driver requires
  a Handle(1) to contain a WinNtIo protocol, a DevicePath protocol, and 
  the TypeGuid in the WinNtIo must be gEfiWinNtSerialPortGuid. 
  
  If Handle(1) contains a gEfiWinNtSerialPortGuid protocol then the driver is 
  loaded on the device.

--*/

#include "WinNtSerialIo.h"

EFI_DRIVER_BINDING_PROTOCOL gWinNtSerialIoDriverBinding = {
  WinNtSerialIoDriverBindingSupported,
  WinNtSerialIoDriverBindingStart,
  WinNtSerialIoDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT(InitializeWinNtSerialIo)

EFI_STATUS
InitializeWinNtSerialIo (
  IN EFI_HANDLE			    	ImageHandle,
  IN EFI_SYSTEM_TABLE			*SystemTable
  )
/*++

Routine Description:

  Intialize Win32 windows to act as EFI SimpleTextOut and SimpleTextIn windows. . 

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  EFI_STATUS

--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gWinNtSerialIoDriverBinding, 
           ImageHandle,
           &gWinNtSerialIoComponentName,
           NULL,
           NULL
           );
}

STATIC
EFI_STATUS
EFIAPI
WinNtSerialIoDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;
  EFI_WIN_NT_IO_PROTOCOL    *WinNtIo;
  UART_DEVICE_PATH          *UartNode;
  
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiDevicePathProtocolGuid,  
                  &ParentDevicePath,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
         Handle,   
         &gEfiDevicePathProtocolGuid,  
         This->DriverBindingHandle,   
         Handle   
         );

  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiWinNtIoProtocolGuid,  
                  &WinNtIo,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Make sure that the WinNt Thunk Protocol is valid 
  //
  if (WinNtIo->WinNtThunk->Signature != EFI_WIN_NT_THUNK_PROTOCOL_SIGNATURE) {
    Status = EFI_UNSUPPORTED;
    goto Error;
  }

  //
  // Check the GUID to see if this is a handle type the driver supports
  //
  if (!EfiCompareGuid (WinNtIo->TypeGuid, &gEfiWinNtSerialPortGuid)) {
    Status = EFI_UNSUPPORTED;
    goto Error;
  }

  if (RemainingDevicePath != NULL) {
    Status = EFI_UNSUPPORTED;
    UartNode = (UART_DEVICE_PATH *)RemainingDevicePath;
    if (UartNode->Header.Type != MESSAGING_DEVICE_PATH ||
        UartNode->Header.SubType != MSG_UART_DP ||
        DevicePathNodeLength((EFI_DEVICE_PATH_PROTOCOL *)UartNode) != sizeof(UART_DEVICE_PATH)) {
      goto Error;
    }
    if (UartNode->BaudRate < 0 || UartNode->BaudRate > SERIAL_PORT_MAX_BAUD_RATE) {
      goto Error;
    }
    if (UartNode->Parity < NoParity || UartNode->Parity > SpaceParity) {
      goto Error;
    }
    if (UartNode->DataBits < 5 || UartNode->DataBits > 8) {
      goto Error;
    }
    if (UartNode->StopBits < OneStopBit || UartNode->StopBits > TwoStopBits) {
      goto Error;
    }
    if ((UartNode->DataBits == 5) && (UartNode->StopBits == TwoStopBits)) {
      goto Error;
    }
    if ((UartNode->DataBits >= 6) && (UartNode->DataBits <=8) && (UartNode->StopBits == OneFiveStopBits)) {
      goto Error;
    }
    Status = EFI_SUCCESS;
  }
   
Error:
  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         Handle,   
         &gEfiWinNtIoProtocolGuid,  
         This->DriverBindingHandle,   
         Handle   
         );

  return Status;
}


STATIC
EFI_STATUS
EFIAPI
WinNtSerialIoDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                           Status;
  EFI_WIN_NT_IO_PROTOCOL               *WinNtIo;
  WIN_NT_SERIAL_IO_PRIVATE_DATA        *Private;
  HANDLE                               NtHandle;
  UART_DEVICE_PATH                     Node;
  EFI_DEVICE_PATH_PROTOCOL             *ParentDevicePath;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY  *OpenInfoBuffer;
  UINTN                                EntryCount;
  UINTN                                Index;
  EFI_SERIAL_IO_PROTOCOL               *SerialIo;

  Private = NULL;
  NtHandle = INVALID_HANDLE_VALUE;

  //
  // Grab the protocols we need
  //
  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiDevicePathProtocolGuid,  
                  &ParentDevicePath,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    return Status;
  }                   

  //
  // Grab the IO abstraction we need to get any work done
  //
  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiWinNtIoProtocolGuid,  
                  &WinNtIo,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    gBS->CloseProtocol (
           Handle, 
           &gEfiDevicePathProtocolGuid, 
           This->DriverBindingHandle, 
           Handle
           );
    return Status;
  }

  if (Status == EFI_ALREADY_STARTED) {

    if (RemainingDevicePath == NULL) {
      return EFI_SUCCESS;
    }

    //
    // Make sure a child handle does not already exist.  This driver can only 
    // produce one child per serial port.
    //
    Status = gBS->OpenProtocolInformation (
                    Handle,
                    &gEfiWinNtIoProtocolGuid,
                    &OpenInfoBuffer,
                    &EntryCount
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = EFI_ALREADY_STARTED;
    for (Index = 0; Index < EntryCount; Index++) {
      if (OpenInfoBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) {
        Status = gBS->OpenProtocol (
                        OpenInfoBuffer[Index].ControllerHandle,   
                        &gEfiSerialIoProtocolGuid,  
                        &SerialIo,
                        This->DriverBindingHandle,   
                        Handle,   
                        EFI_OPEN_PROTOCOL_GET_PROTOCOL
                        );
        if (!EFI_ERROR (Status)) {
          EfiCopyMem (&Node, RemainingDevicePath, sizeof (UART_DEVICE_PATH));
          Status = SerialIo->SetAttributes (
                               SerialIo,
                               Node.BaudRate,
                               SerialIo->Mode->ReceiveFifoDepth,
                               SerialIo->Mode->Timeout,
                               Node.Parity,
                               Node.DataBits,
                               Node.StopBits 
                               );
        }
        break;
      }
    }
    gBS->FreePool (OpenInfoBuffer);
    return Status;
  }

  //
  // Check to see if we can access the hardware device. If it's Open in NT we
  // will not get access.
  //
  NtHandle = WinNtIo->WinNtThunk->CreateFile (
                                    WinNtIo->EnvString,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL
                                    );
  if (NtHandle == INVALID_HANDLE_VALUE) {
    Status = EFI_DEVICE_ERROR;
    goto Error;
  }

  //
  // Construct Private data
  //
  Status = gBS->AllocatePool(
                EfiBootServicesData,
								sizeof(WIN_NT_SERIAL_IO_PRIVATE_DATA), 
                &Private
                );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // This signature must be valid before any member function is called
  //
  Private->Signature           = WIN_NT_SERIAL_IO_PRIVATE_DATA_SIGNATURE;
  Private->NtHandle            = NtHandle;
  Private->ControllerHandle    = Handle;
  Private->Handle              = NULL;
  Private->WinNtThunk          = WinNtIo->WinNtThunk;
  Private->ParentDevicePath    = ParentDevicePath;
  Private->ControllerNameTable = NULL;

  EfiLibAddUnicodeString (
    "eng", 
    gWinNtSerialIoComponentName.SupportedLanguages, 
    &Private->ControllerNameTable, 
    WinNtIo->EnvString
    );

  Private->SerialIo.Revision        = SERIAL_IO_INTERFACE_REVISION;
  Private->SerialIo.Reset           = WinNtSerialIoReset;
  Private->SerialIo.SetAttributes   = WinNtSerialIoSetAttributes;
  Private->SerialIo.SetControl      = WinNtSerialIoSetControl;
  Private->SerialIo.GetControl      = WinNtSerialIoGetControl;
  Private->SerialIo.Write           = WinNtSerialIoWrite;
  Private->SerialIo.Read            = WinNtSerialIoRead;
  Private->SerialIo.Mode            = &Private->SerialIoMode;

  if (RemainingDevicePath != NULL) {
    //
    // Match the configuration of the RemainingDevicePath. IsHandleSupported()
    // already checked to make sure the RemainingDevicePath contains settings
    // that we can support.
    //
    EfiCopyMem (&Private->UartDevicePath, RemainingDevicePath, sizeof (UART_DEVICE_PATH));
  } else {
    //
    // Build the device path by appending the UART node to the ParentDevicePath 
    // from the WinNtIo handle. The Uart setings are zero here, since 
    // SetAttribute() will update them to match the default setings.
    //
    EfiZeroMem (&Private->UartDevicePath, sizeof (UART_DEVICE_PATH));
    Private->UartDevicePath.Header.Type = MESSAGING_DEVICE_PATH;
    Private->UartDevicePath.Header.SubType = MSG_UART_DP;
    SetDevicePathNodeLength ((EFI_DEVICE_PATH_PROTOCOL *)&Private->UartDevicePath, sizeof(UART_DEVICE_PATH));
  }

  //
  // Build the device path by appending the UART node to the ParentDevicePath 
  // from the WinNtIo handle. The Uart setings are zero here, since 
  // SetAttribute() will update them to match the current setings.
  //
  Private->DevicePath = EfiAppendDevicePathNode (
                          ParentDevicePath, 
                          (EFI_DEVICE_PATH_PROTOCOL *)&Private->UartDevicePath
                          );
  if (Private->DevicePath == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }

  //
  // Fill in Serial I/O Mode structure based on either the RemainingDevicePath or defaults.
  //
  Private->SerialIoMode.ControlMask      = SERIAL_CONTROL_MASK;
  Private->SerialIoMode.Timeout          = SERIAL_TIMEOUT_DEFAULT;
  Private->SerialIoMode.BaudRate         = Private->UartDevicePath.BaudRate;
  Private->SerialIoMode.ReceiveFifoDepth = SERIAL_FIFO_DEFAULT;
  Private->SerialIoMode.DataBits         = Private->UartDevicePath.DataBits;
  Private->SerialIoMode.Parity           = Private->UartDevicePath.Parity;
  Private->SerialIoMode.StopBits         = Private->UartDevicePath.StopBits;

  //
  // Issue a reset to initialize the COM port
  //
  Status = Private->SerialIo.Reset (&Private->SerialIo);
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // Create new child handle 
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,                 
                  &gEfiSerialIoProtocolGuid,        &Private->SerialIo,
                  &gEfiDevicePathProtocolGuid,      Private->DevicePath,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // Open For Child Device
  //
  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiWinNtIoProtocolGuid,  
                  &WinNtIo,
                  This->DriverBindingHandle,   
                  Private->Handle,   
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  return EFI_SUCCESS;

Error:
  //
  // Use the Stop() function to free all resources allocated in Start()
  //
  if (Private != NULL) {
    if (Private->Handle != NULL) {
      This->Stop (This, Handle, 1, &Private->Handle);
    } else {
      if (NtHandle != INVALID_HANDLE_VALUE) {
        Private->WinNtThunk->CloseHandle (NtHandle);
      }
      if (Private->DevicePath != NULL) {
        gBS->FreePool (Private->DevicePath);
      }
      EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);

      gBS->FreePool (Private);
    }
  }
  This->Stop (This, Handle, 0, NULL);

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
WinNtSerialIoDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  )
{
  EFI_STATUS                      Status;
  UINTN                           Index;
  BOOLEAN                         AllChildrenStopped;
  EFI_SERIAL_IO_PROTOCOL          *SerialIo;
  WIN_NT_SERIAL_IO_PRIVATE_DATA   *Private;
  EFI_WIN_NT_IO_PROTOCOL          *WinNtIo;

  //
  // Complete all outstanding transactions to Controller.
  // Don't allow any new transaction to Controller to be started.
  //

  if (NumberOfChildren == 0) {
    //
    // Close the bus driver
    //
    Status =  gBS->CloseProtocol (
                     Handle, 
                     &gEfiWinNtIoProtocolGuid, 
                     This->DriverBindingHandle, 
                     Handle
                     );
    Status =  gBS->CloseProtocol (
                     Handle, 
                     &gEfiDevicePathProtocolGuid, 
                     This->DriverBindingHandle, 
                     Handle
                     );
    return Status;
  }

  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {
    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index],   
                    &gEfiSerialIoProtocolGuid,  
                    &SerialIo,
                    This->DriverBindingHandle,   
                    Handle,   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {
      Private = WIN_NT_SERIAL_IO_PRIVATE_DATA_FROM_THIS (SerialIo);

      ASSERT (Private->Handle == ChildHandleBuffer[Index]);

      Status = gBS->CloseProtocol (
                      Handle, 
                      &gEfiWinNtIoProtocolGuid, 
                      This->DriverBindingHandle, 
                      ChildHandleBuffer[Index]
                      );

      Status = gBS->UninstallMultipleProtocolInterfaces (
                      ChildHandleBuffer[Index],                
                      &gEfiSerialIoProtocolGuid,        &Private->SerialIo,
                      &gEfiDevicePathProtocolGuid,      Private->DevicePath,
                      NULL
                      );  

      if (EFI_ERROR (Status)) {
        gBS->OpenProtocol (
               Handle,   
               &gEfiWinNtIoProtocolGuid,  
               (VOID **)&WinNtIo,
               This->DriverBindingHandle,   
               ChildHandleBuffer[Index],
               EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
               );
      } else {
        Private->WinNtThunk->CloseHandle (Private->NtHandle);

        gBS->FreePool (Private->DevicePath);

        EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);
        
        gBS->FreePool (Private);
      }
    }

    if (EFI_ERROR(Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

//
// Serial IO Protocol member functions
//

STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoReset (
  IN EFI_SERIAL_IO_PROTOCOL *This
  )
{
  WIN_NT_SERIAL_IO_PRIVATE_DATA  *Private;

  Private = WIN_NT_SERIAL_IO_PRIVATE_DATA_FROM_THIS (This);

  Private->WinNtThunk->PurgeComm (
                         Private->NtHandle, 
                         PURGE_TXCLEAR | PURGE_RXCLEAR
                         );

  return This->SetAttributes (
                 This,
                 This->Mode->BaudRate,
                 This->Mode->ReceiveFifoDepth,
                 This->Mode->Timeout,
                 This->Mode->Parity,
                 (UINT8)This->Mode->DataBits,
                 This->Mode->StopBits 
                 );
}

STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoSetAttributes (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN UINT64                 BaudRate,
  IN UINT32                 ReceiveFifoDepth,
  IN UINT32                 Timeout,
  IN EFI_PARITY_TYPE        Parity,
  IN UINT8                  DataBits,
  IN EFI_STOP_BITS_TYPE     StopBits
  )
{
  EFI_STATUS                     Status;
  WIN_NT_SERIAL_IO_PRIVATE_DATA  *Private;
  COMMTIMEOUTS                   PortTimeOuts;
  DWORD                          ConvertedTime;
  BOOL                           Result;
  EFI_DEVICE_PATH_PROTOCOL       *NewDevicePath;

  Private = WIN_NT_SERIAL_IO_PRIVATE_DATA_FROM_THIS (This);

  //
  //  Some of our arguments have defaults if a null value is passed in, and
  //   we must set the default values if a null argument is passed in.
  //
  if (BaudRate == 0) {
    BaudRate = SERIAL_BAUD_DEFAULT;
  }
  if (ReceiveFifoDepth == 0) {
    ReceiveFifoDepth = SERIAL_FIFO_DEFAULT;
  }
  if (Timeout == 0) {
    Timeout = SERIAL_TIMEOUT_DEFAULT;
  }
  if (Parity == DefaultParity) {
    Parity = NoParity; 
  }
  if (DataBits == 0) {
    DataBits = SERIAL_DATABITS_DEFAULT;
  }
  if (StopBits == DefaultStopBits) {
    StopBits = OneStopBit;
  }

  //
  // See if the new attributes already match the current attributes
  //
  if (Private->UartDevicePath.BaudRate       == BaudRate         &&
      Private->UartDevicePath.DataBits       == DataBits         &&
      Private->UartDevicePath.Parity         == Parity           &&
      Private->UartDevicePath.StopBits       == StopBits         &&
      Private->SerialIoMode.ReceiveFifoDepth == ReceiveFifoDepth &&
      Private->SerialIoMode.Timeout          == Timeout             ) {
    return EFI_SUCCESS;
  }

  //
  //  Get current values from NT
  //
  EfiZeroMem (&Private->NtDCB, sizeof(DCB));
  Private->NtDCB.DCBlength = sizeof(DCB);

  if (!Private->WinNtThunk->GetCommState (Private->NtHandle, &Private->NtDCB)) {
    Private->NtError = Private->WinNtThunk->GetLastError();
    DEBUG((EFI_D_ERROR, "SerialSetAttributes: GetCommState %d\n", Private->NtError));
    return EFI_DEVICE_ERROR;
  }

  //
  // Map EFI com setting to NT
  //
  Private->NtDCB.BaudRate        = ConvertBaud2Nt (BaudRate);
  Private->NtDCB.ByteSize        = ConvertData2Nt (DataBits);
  Private->NtDCB.Parity          = ConvertParity2Nt (Parity);
  Private->NtDCB.StopBits        = ConvertStop2Nt (StopBits);

  Private->NtDCB.fBinary         = TRUE;
  Private->NtDCB.fParity         = Private->NtDCB.Parity == NOPARITY ? FALSE : TRUE;
  Private->NtDCB.fOutxCtsFlow    = FALSE;
  Private->NtDCB.fOutxDsrFlow    = FALSE;
  Private->NtDCB.fDtrControl     = DTR_CONTROL_ENABLE;
  Private->NtDCB.fDsrSensitivity = FALSE;
  Private->NtDCB.fOutX           = FALSE;
  Private->NtDCB.fInX            = FALSE;
  Private->NtDCB.fRtsControl     = RTS_CONTROL_ENABLE;
  Private->NtDCB.fNull           = FALSE;

  //
  //  Set new values
  //
  Result = Private->WinNtThunk->SetCommState (Private->NtHandle, &Private->NtDCB);
  if (!Result) {
    Private->NtError = Private->WinNtThunk->GetLastError();
    DEBUG((EFI_D_ERROR, "SerialSetAttributes: SetCommState %d\n", Private->NtError ));
    return EFI_DEVICE_ERROR;
  }

  //
  //  Set com port read/write timeout values
  //
  ConvertedTime = ConvertTime2Nt (Timeout);
  PortTimeOuts.ReadIntervalTimeout         = MAXDWORD;
  PortTimeOuts.ReadTotalTimeoutMultiplier  = 0;
  PortTimeOuts.ReadTotalTimeoutConstant    = ConvertedTime;
  PortTimeOuts.WriteTotalTimeoutMultiplier = ConvertedTime == 0 ? 1 : ConvertedTime;
  PortTimeOuts.WriteTotalTimeoutConstant   = 0;

  if (!Private->WinNtThunk->SetCommTimeouts (Private->NtHandle, &PortTimeOuts)) {
    Private->NtError = Private->WinNtThunk->GetLastError();
    DEBUG((EFI_D_ERROR, "SerialSetAttributes: SetCommTimeouts %d\n", Private->NtError ));
    return EFI_DEVICE_ERROR;
  }

  //
  //  Update mode
  //
  Private->SerialIoMode.BaudRate         = BaudRate;
  Private->SerialIoMode.ReceiveFifoDepth = ReceiveFifoDepth;
  Private->SerialIoMode.Timeout          = Timeout;
  Private->SerialIoMode.Parity           = Parity;
  Private->SerialIoMode.DataBits         = DataBits;
  Private->SerialIoMode.StopBits         = StopBits;

  //
  // See if Device Path Node has actually changed
  //
  if (Private->UartDevicePath.BaudRate     == BaudRate &&
      Private->UartDevicePath.DataBits     == DataBits &&
      Private->UartDevicePath.Parity       == Parity   &&
      Private->UartDevicePath.StopBits     == StopBits    ) {
    return EFI_SUCCESS;
  }

  //
  // Update the device path
  //
  Private->UartDevicePath.BaudRate = BaudRate;
  Private->UartDevicePath.DataBits = DataBits;
  Private->UartDevicePath.Parity   = (UINT8)Parity;
  Private->UartDevicePath.StopBits = (UINT8)StopBits;

  NewDevicePath = EfiAppendDevicePathNode (
                    Private->ParentDevicePath,
                    (EFI_DEVICE_PATH_PROTOCOL *)&Private->UartDevicePath
                    );
  if (NewDevicePath == NULL) {
    return EFI_DEVICE_ERROR;
  }

  if (Private->Handle != NULL) {
    Status = gBS->ReinstallProtocolInterface (
                    Private->Handle,                 
                    &gEfiDevicePathProtocolGuid, 
                    Private->DevicePath, 
                    NewDevicePath
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (Private->DevicePath) {
    gBS->FreePool (Private->DevicePath);
  }
  Private->DevicePath = NewDevicePath;

  return EFI_SUCCESS;
}


STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoSetControl (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN UINT32                 Control
  )
{
  WIN_NT_SERIAL_IO_PRIVATE_DATA *Private;
  BOOL                          Result;

  Private = WIN_NT_SERIAL_IO_PRIVATE_DATA_FROM_THIS (This);

  Result = Private->WinNtThunk->EscapeCommFunction (
                                  Private->NtHandle,
                                  (Control & EFI_SERIAL_REQUEST_TO_SEND) ? SETRTS : CLRRTS
                                  );
  if (!Result) {    
    Private->NtError = Private->WinNtThunk->GetLastError();
    DEBUG((EFI_D_ERROR, "SerialSetControl: EscapeCommFunct - RTS %d\n", Private->NtError));
    return EFI_DEVICE_ERROR;
  }

  Result = Private->WinNtThunk->EscapeCommFunction (
                                  Private->NtHandle,
                                  (Control & EFI_SERIAL_DATA_TERMINAL_READY) ? SETDTR : CLRDTR
                                  );
  if (!Result) {
    Private->NtError = Private->WinNtThunk->GetLastError();
    DEBUG((EFI_D_ERROR, "SerialSetControl: EscpeCommFunct - DTR %d\n", Private->NtError ));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}


STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoGetControl (
  IN  EFI_SERIAL_IO_PROTOCOL  *This,
  OUT UINT32                  *Control
  )
{
  WIN_NT_SERIAL_IO_PRIVATE_DATA *Private;
  DWORD                         ModemStatus;
  DWORD                         Errors;
  UINT32                        Bits;

  Private = WIN_NT_SERIAL_IO_PRIVATE_DATA_FROM_THIS (This);

  //
  //  Get modem status
  //
  if (!Private->WinNtThunk->GetCommModemStatus (Private->NtHandle, &ModemStatus)) {
      Private->NtError = Private->WinNtThunk->GetLastError();
      DEBUG((EFI_D_ERROR, "SerialGetControl: GetCommModemStatus %d\n", Private->NtError ));
      return EFI_DEVICE_ERROR;
  }

  Bits = 0;
  if (ModemStatus & MS_CTS_ON) {
    Bits |= EFI_SERIAL_CLEAR_TO_SEND;
  }
  if (ModemStatus & MS_DSR_ON) {
    Bits |= EFI_SERIAL_DATA_SET_READY;
  }
  if (ModemStatus & MS_RING_ON) {
    Bits |= EFI_SERIAL_RING_INDICATE;
  }
  if (ModemStatus & MS_RLSD_ON) {
    Bits |= EFI_SERIAL_CARRIER_DETECT;
  }

  //
  //  Get input buffer status
  //
  if (!Private->WinNtThunk->ClearCommError (Private->NtHandle, &Errors, &Private->NtComStatus)) {
    Private->NtError = Private->WinNtThunk->GetLastError();
    DEBUG((EFI_D_ERROR, "SerialGetControl: ClearCommError %d\n", Private->NtError ));
    return EFI_DEVICE_ERROR;
  }

  if (Private->NtComStatus.cbInQue == 0) {
    Bits |= EFI_SERIAL_INPUT_BUFFER_EMPTY;
  }

  *Control = Bits;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoWrite (
  IN EFI_SERIAL_IO_PROTOCOL   *This,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  )
{
  WIN_NT_SERIAL_IO_PRIVATE_DATA *Private;
  UINT8                         *ByteBuffer;
  UINTN                         TotalBytesWritten;
  DWORD                         BytesToGo;
  DWORD                         BytesWritten;
  BOOL                          Result;
        
  Private = WIN_NT_SERIAL_IO_PRIVATE_DATA_FROM_THIS (This);

  ByteBuffer = (UINT8 *)Buffer;
  BytesToGo = (DWORD)(*BufferSize);
  TotalBytesWritten = 0;
  do {
    //
    //  Do the write
    //
    Result = Private->WinNtThunk->WriteFile (
                                    Private->NtHandle, 
                                    &ByteBuffer[TotalBytesWritten], 
                                    BytesToGo, 
                                    &BytesWritten, 
                                    NULL
                                    );
    TotalBytesWritten += BytesWritten;
    BytesToGo -= BytesWritten;
    if (!Result) {
      Private->NtError = Private->WinNtThunk->GetLastError();
      DEBUG((EFI_D_ERROR, "SerialWrite: FileWrite %d\n", Private->NtError ));
      *BufferSize = TotalBytesWritten;
      return EFI_DEVICE_ERROR;
    }
  } while (BytesToGo > 0);

  *BufferSize = TotalBytesWritten;
  return EFI_SUCCESS;
}


STATIC
EFI_STATUS 
EFIAPI
WinNtSerialIoRead (
  IN  EFI_SERIAL_IO_PROTOCOL  *This,
  IN  OUT UINTN               *BufferSize,
  OUT VOID                    *Buffer
  )
{
  WIN_NT_SERIAL_IO_PRIVATE_DATA *Private;
  BOOL                          Result;
  DWORD                         BytesRead;
  EFI_STATUS                    Status;

  Private = WIN_NT_SERIAL_IO_PRIVATE_DATA_FROM_THIS (This);

  //
  //  Do the read
  //
  Result = Private->WinNtThunk->ReadFile (
                                  Private->NtHandle, 
                                  Buffer, 
                                  (DWORD)*BufferSize, 
                                  &BytesRead, 
                                  NULL
                                  );
  if (!Result) {
    Private->NtError = Private->WinNtThunk->GetLastError();
    DEBUG((EFI_D_ERROR, "SerialRead: FileRead %d\n", Private->NtError ));
    return EFI_DEVICE_ERROR;
  }
    
  if (BytesRead != *BufferSize) {
    Status = EFI_TIMEOUT;
  } else {
    Status = EFI_SUCCESS;
  }

  *BufferSize = (UINTN)BytesRead;
  return Status;
}



