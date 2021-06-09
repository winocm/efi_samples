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

  SerialMouse.c
  
Abstract:

  Serial Mouse driver that layers it's self on every Serial I/O protocol in the system
  and produces a Simple Pointer protocol if a serial mouse is detected.

--*/

#include "SerialMouse.h"

//
// Prototypes
// Driver model protocol interface
//

EFI_STATUS
SerialMouseDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
SerialMouseDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
SerialMouseDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
SerialMouseDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// Simple Pointer Protocol Interface
//
EFI_STATUS
SerialMouseReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL  *This,
  IN BOOLEAN                      ExtendedVerification
  );

EFI_STATUS
SerialMouseGetState (
  IN     EFI_SIMPLE_POINTER_PROTOCOL  *This,
  IN OUT EFI_SIMPLE_POINTER_STATE     *State
  );

VOID 
SerialMouseWaitForInput (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  );

VOID 
EFIAPI
SerialMousePoll (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  );

EFI_STATUS
SerialMouseRead(
  SERIAL_MOUSE_PRIVATE_DATA  *PrivateData,
  OUT    UINT8               *Buffer, 
  IN OUT UINTN               *BufSize
  );

EFI_STATUS
SerialMouseGetPacket(
  SERIAL_MOUSE_PRIVATE_DATA  *PrivateData
	);

//
// EFI Driver Binding Protocol instance
//
EFI_DRIVER_BINDING_PROTOCOL gSerialMouseDriverBinding = {
  SerialMouseDriverBindingSupported,
  SerialMouseDriverBindingStart,
  SerialMouseDriverBindingStop,
  0xfffffff0,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT (SerialMouseDriverEntryPoint)

EFI_STATUS
EFIAPI
SerialMouseDriverEntryPoint(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:
  Register Driver Binding protocol for this driver.
  
Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 
  EFI_SUCCESS - Driver loaded.
  other       - Driver not loaded.

--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gSerialMouseDriverBinding, 
           ImageHandle,
           &gSerialMouseComponentName,
           NULL,
           NULL
           );
}


EFI_STATUS
EFIAPI
SerialMouseDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath  OPTIONAL
  )
/*++

  Routine Description:
    Test to see if this driver supports ControllerHandle. Any ControllerHandle
    than contains a SerialIo protocol can be supported.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to test.
    RemainingDevicePath - Not used.

  Returns:
    EFI_SUCCESS         - This driver supports this device.
    EFI_ALREADY_STARTED - This driver is already running on this device.
    other               - This driver does not support this device.

--*/
{
  EFI_STATUS                Status;
  EFI_SERIAL_IO_PROTOCOL    *SerialIo;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  //
  // Open the IO Abstraction(s) needed to perform the supported test.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,  
                  &gEfiSerialIoProtocolGuid, 
                  (VOID **)&SerialIo,
                  This->DriverBindingHandle,
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test.
  //
  Status = gBS->CloseProtocol (
                  ControllerHandle,  
                  &gEfiSerialIoProtocolGuid, 
                  This->DriverBindingHandle,   
                  ControllerHandle   
                  );

  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiDevicePathProtocolGuid, 
                  (VOID **)&DevicePath,
                  This->DriverBindingHandle,   
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = gBS->CloseProtocol (
                  ControllerHandle,  
                  &gEfiDevicePathProtocolGuid, 
                  This->DriverBindingHandle,   
                  ControllerHandle   
                  );

  return Status;
}

EFI_STATUS
EFIAPI
SerialMouseDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath  OPTIONAL
  )
/*++

  Routine Description:
    Start this driver on ControllerHandle by opening a Block IO protocol and 
    installing a Disk IO protocol on ControllerHandle.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to bind driver to.
    RemainingDevicePath - Not used, always produce all possible children.

  Returns:
    EFI_SUCCESS         - This driver is added to ControllerHandle.
    EFI_ALREADY_STARTED - This driver is already running on ControllerHandle.
    other               - This driver does not support this device.

--*/
{
  EFI_STATUS                 Status;
  EFI_DEVICE_PATH_PROTOCOL   *ParentDevicePath;
  EFI_SERIAL_IO_PROTOCOL     *SerialIo;
  SERIAL_MOUSE_PRIVATE_DATA  *Private;
  EFI_DEV_PATH                Node;

  Private = NULL;

  //
  // Connect to the Serial I/O interface on ControllerHandle.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiSerialIoProtocolGuid, 
                  &SerialIo,
                  This->DriverBindingHandle,   
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  //
  // Initialize the Serial Mouse device instance.
  //
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof (SERIAL_MOUSE_PRIVATE_DATA),
                  &Private
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  //
  // Zero all fields of the Serial Mouse device instance
  //
  EfiZeroMem (Private, sizeof(SERIAL_MOUSE_PRIVATE_DATA));

  //
  // Fill in the Signature and the Serial I/O interface that was opened above
  //
  Private->Signature  = SERIAL_MOUSE_PRIVATE_DATA_SIGNATURE;
  Private->SerialIo   = SerialIo;

  //
  // Fill in the Simple Pointer Protocol fields
  //
  Private->SimplePointer.Reset    = SerialMouseReset;
  Private->SimplePointer.GetState = SerialMouseGetState;
  Private->SimplePointer.Mode     = &Private->SimplePointerMode;
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_WAIT,
                  EFI_TPL_NOTIFY,
                  SerialMouseWaitForInput,
                  Private,
                  &Private->SimplePointer.WaitForInput
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  //
  // Fill in the Simple Pointer Mode data
  //
  Private->SimplePointerMode.ResolutionX = 8;
  Private->SimplePointerMode.ResolutionY = 8;
  Private->SimplePointerMode.ResolutionZ = 0;
  Private->SimplePointerMode.LeftButton  = TRUE;
  Private->SimplePointerMode.RightButton = TRUE;

  //
  // Fill in the communications parameters for a Microsoft* Serial Mouse
	//
	// * Other names and brands may be claimed as the property of others.
  //
	Private->SerialIoMode.Timeout          = 25000;
	Private->SerialIoMode.BaudRate         = 1200;
	Private->SerialIoMode.ReceiveFifoDepth = 3;
	Private->SerialIoMode.DataBits         = 7;
	Private->SerialIoMode.Parity           = NoParity;
	Private->SerialIoMode.StopBits         = OneStopBit;

  //
  // Extract the UART parameters from the Serial I/O Protocol
  //
  EfiCopyMem (&Private->OriginalSerialIoMode, SerialIo->Mode, sizeof (EFI_SERIAL_IO_MODE));

  //
  // Save the RTS and DTR values
  //
  Private->ControlValid = FALSE;
  Status = Private->SerialIo->GetControl (
                                Private->SerialIo, 
                                &Private->Control
                                );
  if (!EFI_ERROR (Status)) {
    Private->ControlValid = TRUE;
  }
  
  //
  // Reset the Serial Mouse Device
  //
  Status = Private->SimplePointer.Reset (&Private->SimplePointer, FALSE);
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  //
  // Create a timer event to poll the serial mouse device
  //
	Status = gBS->CreateEvent (
                  EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL, 
                  EFI_TPL_NOTIFY, 
                  SerialMousePoll, 
                  Private, 
                  &Private->TimeEvent 
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

	Status = gBS->SetTimer (
                  Private->TimeEvent, 
                  TimerPeriodic, 
                  100000
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  //
  // Build a device path for the Serial Mouse
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle, 
                  &gEfiDevicePathProtocolGuid, 
                  &ParentDevicePath,
                  This->DriverBindingHandle,   
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  EfiZeroMem(&Node, sizeof(Node));
  Node.DevPath.Type = ACPI_DEVICE_PATH;
  Node.DevPath.SubType = ACPI_DP;
  SetDevicePathNodeLength (&Node.DevPath, sizeof(ACPI_HID_DEVICE_PATH));
  Node.Acpi.HID = EISA_PNP_ID(0x0F01);
  Node.Acpi.UID = 0;

  Private->DevicePath = EfiAppendDevicePathNode (ParentDevicePath, &Node.DevPath);
  if (Private->DevicePath == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  Private->ControllerNameTable = NULL;
  EfiLibAddUnicodeString (
    "eng", 
    gSerialMouseComponentName.SupportedLanguages, 
    &Private->ControllerNameTable, 
    L"Serial Mouse Device"
    );

  //
  // Install protocol interfaces for the Serial Mouse device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle, 
                  &gEfiDevicePathProtocolGuid,    Private->DevicePath,
                  &gEfiSimplePointerProtocolGuid, &Private->SimplePointer,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  } 

  //
  // Open For Child Device
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiSerialIoProtocolGuid,  
                  &SerialIo,
                  This->DriverBindingHandle,   
                  Private->Handle,   
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );

  return EFI_SUCCESS;

ErrorExit:

  if (Private != NULL) {
    if (Private->SimplePointer.WaitForInput) {
      gBS->CloseEvent (Private->SimplePointer.WaitForInput);
    }
    if (Private->TimeEvent) {
      gBS->SetTimer (Private->TimeEvent, TimerCancel, 0);
      gBS->CloseEvent (Private->TimeEvent);
    }
    Private->SerialIo->SetAttributes (
                         Private->SerialIo,
 		                     Private->OriginalSerialIoMode.BaudRate,
		                     Private->OriginalSerialIoMode.ReceiveFifoDepth,
		                     Private->OriginalSerialIoMode.Timeout,
		                     Private->OriginalSerialIoMode.Parity,
		                     (UINT8)Private->OriginalSerialIoMode.DataBits,
		                     Private->OriginalSerialIoMode.StopBits
		                     );

    if (Private->ControlValid) {
      Status = Private->SerialIo->SetControl (
                                    Private->SerialIo, 
                                    Private->Control
                                    );
    }

    if (Private->DevicePath) {
      gBS->FreePool (Private->DevicePath);
    }

    EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);

    gBS->FreePool (Private);
  }

  gBS->CloseProtocol (
         ControllerHandle, 
         &gEfiSerialIoProtocolGuid, 
         This->DriverBindingHandle,   
         ControllerHandle   
         );

  gBS->CloseProtocol (
         ControllerHandle, 
         &gEfiDevicePathProtocolGuid, 
         This->DriverBindingHandle,   
         ControllerHandle   
         );

  return Status;
}

EFI_STATUS
EFIAPI
SerialMouseDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
/*++

  Routine Description:
    Stop this driver on ControllerHandle by removing Disk IO protocol and closing 
    the Block IO protocol on ControllerHandle.

  Arguments:
    This              - Protocol instance pointer.
    ControllerHandle  - Handle of device to stop driver on.
    NumberOfChildren  - Not used.
    ChildHandleBuffer - Not used.

  Returns:
    EFI_SUCCESS         - This driver is removed ControllerHandle.
    other               - This driver was not removed from this device.

--*/
{
  EFI_STATUS                   Status;
  UINTN                        Index;
  BOOLEAN                      AllChildrenStopped;
  EFI_SIMPLE_POINTER_PROTOCOL  *SimplePointer;
  SERIAL_MOUSE_PRIVATE_DATA    *Private;
  EFI_SERIAL_IO_PROTOCOL       *SerialIo;

  //
  // Complete all outstanding transactions to Controller.
  // Don't allow any new transaction to Controller to be started.
  //

  if (NumberOfChildren == 0) {
    //
    // Close the bus driver
    //
    Status =  gBS->CloseProtocol (
                     ControllerHandle, 
                     &gEfiSerialIoProtocolGuid, 
                     This->DriverBindingHandle, 
                     ControllerHandle
                     );

    Status =  gBS->CloseProtocol (
                     ControllerHandle, 
                     &gEfiDevicePathProtocolGuid, 
                     This->DriverBindingHandle, 
                     ControllerHandle
                     );

    return Status;
  }

  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {

    //
    // Get our context back.
    //
    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index],
                    &gEfiSimplePointerProtocolGuid,  
                    &SimplePointer,
                    This->DriverBindingHandle,   
                    ControllerHandle,   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR(Status)) {

      Private = SERIAL_MOUSE_PRIVATE_DATA_FROM_THIS (SimplePointer);

      Status =  gBS->CloseProtocol (
                       ControllerHandle, 
                       &gEfiSerialIoProtocolGuid, 
                       This->DriverBindingHandle, 
                       ChildHandleBuffer[Index]
                       );

      Status = gBS->UninstallMultipleProtocolInterfaces (
                      ChildHandleBuffer[Index], 
                      &gEfiSimplePointerProtocolGuid, &Private->SimplePointer,
                      &gEfiDevicePathProtocolGuid,    Private->DevicePath,
                      NULL
                      );

      if (EFI_ERROR (Status)) {
        gBS->OpenProtocol (
               ControllerHandle,   
               &gEfiSerialIoProtocolGuid,  
               (VOID **)&SerialIo,
               This->DriverBindingHandle,   
               ChildHandleBuffer[Index],
               EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
               );
      } else {
        gBS->CloseEvent (Private->SimplePointer.WaitForInput);

        gBS->SetTimer (Private->TimeEvent, TimerCancel, 0);

        gBS->CloseEvent (Private->TimeEvent);

        Private->SerialIo->SetAttributes (
                             Private->SerialIo,
 		 	                       Private->OriginalSerialIoMode.BaudRate,
 		 	                       Private->OriginalSerialIoMode.ReceiveFifoDepth,
 			                       Private->OriginalSerialIoMode.Timeout,
			                       Private->OriginalSerialIoMode.Parity,
			                       (UINT8)Private->OriginalSerialIoMode.DataBits,
			                       Private->OriginalSerialIoMode.StopBits
			                       );

        if (Private->ControlValid) {
          Status = Private->SerialIo->SetControl (
                                        Private->SerialIo, 
                                        Private->Control
                                        );
        }

        gBS->FreePool (Private->DevicePath);

        EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);

        gBS->FreePool (Private);
      }

    }

    if (EFI_ERROR(Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (AllChildrenStopped == FALSE) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SerialMouseReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL  *This,
  IN BOOLEAN                      ExtendedVerification
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
	EFI_STATUS	               Status;
  SERIAL_MOUSE_PRIVATE_DATA  *PrivateData;
  UINTN                      BufferSize;
  UINT8                      Buffer[10];
  UINTN                      Index;

  PrivateData = SERIAL_MOUSE_PRIVATE_DATA_FROM_THIS (This);

  EfiZeroMem (&PrivateData->SimplePointerState, sizeof (EFI_SIMPLE_POINTER_STATE));

  //
  // Reset the serial port
  //
  Status = PrivateData->SerialIo->Reset (PrivateData->SerialIo);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Set the attributes to those required for a Serial Mouse
  //
  Status = PrivateData->SerialIo->SetAttributes (
                                    PrivateData->SerialIo,
			                              PrivateData->SerialIoMode.BaudRate,
			                              PrivateData->SerialIoMode.ReceiveFifoDepth,
			                              PrivateData->SerialIoMode.Timeout,
			                              PrivateData->SerialIoMode.Parity,
			                              (UINT8)PrivateData->SerialIoMode.DataBits,
			                              PrivateData->SerialIoMode.StopBits
			                              );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Deassert RTS and DTR
  //
  Status = PrivateData->SerialIo->SetControl (
                                    PrivateData->SerialIo, 
                                    0
                                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

	gBS->Stall (100000);

  //
  // Assert RTS and DTR to reset the serial mouse
  //
	Status = PrivateData->SerialIo->SetControl ( 
                                    PrivateData->SerialIo,
                                    EFI_SERIAL_REQUEST_TO_SEND | EFI_SERIAL_DATA_TERMINAL_READY
                                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->Stall (100000);

  //
  // Attempt to read the characters that the serial mouse sends after a reset sequence
  //
  BufferSize = 10;
  Status = PrivateData->SerialIo->Read (
                                    PrivateData->SerialIo,
                                    &BufferSize,
                                    Buffer
                                    );
  if (BufferSize == 0) {
    return EFI_DEVICE_ERROR;
  }

  //
  // If a 'M' is returned, then a serial mouse is present on the serial port
  //
  for (Index = 0; Index < BufferSize; Index++) {
    if (Buffer[Index] == 'M') {
      return EFI_SUCCESS;
    }
  }

  return EFI_DEVICE_ERROR;
}

EFI_STATUS
SerialMouseGetState (
  IN     EFI_SIMPLE_POINTER_PROTOCOL  *This,
  IN OUT EFI_SIMPLE_POINTER_STATE     *State
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  SERIAL_MOUSE_PRIVATE_DATA  *PrivateData;
  EFI_TPL                    OldTpl;

  PrivateData = SERIAL_MOUSE_PRIVATE_DATA_FROM_THIS (This);

  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (!PrivateData->StateChanged) {
	return EFI_NOT_READY;
  }

  //
  // Raise TPL to prevent changes in state from the timer driven polling mechanism
  // 

  OldTpl = gBS->RaiseTPL(EFI_TPL_NOTIFY);

  EfiCopyMem (State, &PrivateData->SimplePointerState, sizeof (EFI_SIMPLE_POINTER_STATE));

  PrivateData->SimplePointerState.RelativeMovementX = 0;
  PrivateData->SimplePointerState.RelativeMovementY = 0;
  PrivateData->SimplePointerState.RelativeMovementZ = 0;
  PrivateData->StateChanged = FALSE;

  SerialMouseGetPacket (PrivateData);

  gBS->RestoreTPL(OldTpl);

  return EFI_SUCCESS;
}

VOID 
EFIAPI
SerialMouseWaitForInput (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  if (!EFI_ERROR (SerialMouseGetPacket (Context))) {
    gBS->SignalEvent (Event);
  }
}

VOID 
EFIAPI
SerialMousePoll (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  SerialMouseGetPacket (Context);
}

EFI_STATUS
SerialMouseRead(
  SERIAL_MOUSE_PRIVATE_DATA  *PrivateData,
  OUT    UINT8               *Buffer, 
  IN OUT UINTN               *BufSize
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
	EFI_STATUS	Status;
	UINTN		    BytesGot;
	UINTN		    BytesRead;
	UINTN		    BytesToRead;
	UINTN		    N;

	Status      = EFI_SUCCESS;
  BytesRead   = 0;
  BytesToRead = *BufSize;
  N           = *BufSize;

  while (BytesRead < BytesToRead) {
    BytesGot = N ;
    Status = PrivateData->SerialIo->Read (
                                      PrivateData->SerialIo,
                                      &BytesGot,
                                      Buffer + BytesRead
                                      );
    if (EFI_ERROR (Status) || BytesGot == 0) {
      break;
    }

    BytesRead += BytesGot;
    N -= BytesGot;
  }
  *BufSize = BytesRead;

  if (BytesRead == 0 || BytesRead != BytesToRead) {
    Status = EFI_NOT_FOUND;
  }

	return Status;
}

EFI_STATUS
SerialMouseGetPacket(
  SERIAL_MOUSE_PRIVATE_DATA  *PrivateData
	)
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
	EFI_STATUS  Status;
  UINT32      Control;
	UINT8       Packet[MS_PACKET_LENGTH];
  UINT8       ByteRead;
	UINTN	      BytesToRead;
	UINTN       State;
  BOOLEAN     Left;
  BOOLEAN     Right;
  INT8        DeltaX;
  INT8        DeltaY;

  //
  // First check to see if there are any characters in the input buffer
  //
	Status = PrivateData->SerialIo->GetControl ( 
                                    PrivateData->SerialIo,
                                    &Control
                                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Control & EFI_SERIAL_INPUT_BUFFER_EMPTY) {
    return EFI_NOT_READY;
  }

  BytesToRead = 1;
  State       = MS_READ_BYTE_ONE;
  Packet[0]   = 0;
  Packet[1]   = 0;
  Packet[2]   = 0;

  //
  // State machine to retrieve 3 bytes from the serial mouse
  //
	while (TRUE) {
    switch (State) {
		case MS_READ_BYTE_ONE:
			Status = SerialMouseRead (PrivateData, &ByteRead, &BytesToRead);
			if (EFI_ERROR (Status) || BytesToRead != 1) {
				return EFI_NOT_READY;
			}
			if (IS_MS_SYNC_BYTE (ByteRead)) {
				Packet[0] = ByteRead;
				State     = MS_READ_BYTE_TWO;
			}
			break;

		case MS_READ_BYTE_TWO:
			Status = SerialMouseRead (PrivateData, &ByteRead, &BytesToRead);
			if (EFI_ERROR (Status) || BytesToRead != 1) {
				return EFI_NOT_READY;
			}
			if (IS_MS_SYNC_BYTE (ByteRead)) {
				Packet[0] = ByteRead;
				State     = MS_READ_BYTE_ONE;
      } else {
				Packet[1] = ByteRead;
				State     = MS_READ_BYTE_THREE;
			}
			break;

		case MS_READ_BYTE_THREE:
			Status = SerialMouseRead (PrivateData, &ByteRead, &BytesToRead);
			if (EFI_ERROR (Status) || BytesToRead != 1) {
				return EFI_NOT_READY;
			}
			if (IS_MS_SYNC_BYTE (ByteRead)) {
				Packet[0] = ByteRead;
				State     = MS_READ_BYTE_ONE;
      } else {
				Packet[2] = ByteRead;
				State     = MS_PROCESS_PACKET;
			}
			break;

		case MS_PROCESS_PACKET:
      //
      // Process the 3 bytes into button press/release and movement information
      //
			DeltaX = (INT8)(((Packet[0] << 6) & 0xC0) | (Packet[1] & 0x3F));
			DeltaY = (INT8)(((Packet[0] << 4) & 0xC0) | (Packet[2] & 0x3F));
			Left   = (BOOLEAN)((Packet[0] & 0x20) ? TRUE : FALSE);
			Right  = (BOOLEAN)((Packet[0] & 0x10) ? TRUE : FALSE);

			if (DeltaX != 0 || DeltaY != 0 ||
				PrivateData->SimplePointerState.LeftButton != Left ||
				PrivateData->SimplePointerState.RightButton != Right) {
  		      PrivateData->SimplePointerState.RelativeMovementX += DeltaX;
  		      PrivateData->SimplePointerState.RelativeMovementY += DeltaY;
	  	      PrivateData->SimplePointerState.LeftButton = Left;
		      PrivateData->SimplePointerState.RightButton = Right;
			  PrivateData->StateChanged = TRUE;
			}

            return EFI_SUCCESS;
		}
	}
  return EFI_SUCCESS;
}
