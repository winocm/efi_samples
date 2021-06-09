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

  serial.c
  
Abstract:

  Serial driver for standard UARTS on an ISA bus.

Revision History:

--*/

#include "serial.h"

#define SERIAL_PORT_NAME "ISA Serial Port # "

//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
SerialControllerDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SerialControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
SerialControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
SerialControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// Serial I/O Protocol Interface
//

EFI_STATUS
IsaSerialReset (
  IN EFI_SERIAL_IO_PROTOCOL  *This
  );

EFI_STATUS
IsaSerialSetAttributes (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN UINT64                  BaudRate,
  IN UINT32                  ReceiveFifoDepth, 
  IN UINT32                  Timeout,
  IN EFI_PARITY_TYPE         Parity,
  IN UINT8                   DataBits,
  IN EFI_STOP_BITS_TYPE      StopBits
  );

EFI_STATUS
IsaSerialSetControl (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN UINT32                  Control
  );

EFI_STATUS
IsaSerialGetControl (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  OUT UINT32                 *Control
  );

EFI_STATUS
IsaSerialWrite (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN OUT UINTN               *BufferSize,
  IN VOID                    *Buffer
  );

EFI_STATUS
IsaSerialRead (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN OUT UINTN               *BufferSize,
  OUT VOID                   *Buffer
  );

//
//Internal Functions
//

BOOLEAN
IsaSerialPortPresent(
  IN SERIAL_DEV *SerialDevice
  );

BOOLEAN
IsaSerialFifoFull(
  IN SERIAL_DEV_FIFO *Fifo
  );

BOOLEAN
IsaSerialFifoEmpty(
  IN SERIAL_DEV_FIFO *Fifo
  );

EFI_STATUS
IsaSerialFifoAdd(
  IN SERIAL_DEV_FIFO *Fifo,
  IN UINT8           Data
  );

EFI_STATUS
IsaSerialFifoRemove(
  IN  SERIAL_DEV_FIFO *Fifo,
  OUT UINT8           *Data
  );

EFI_STATUS
IsaSerialReceiveTransmit (
  IN SERIAL_DEV *SerialDevice
  );


UINT8 
IsaSerialReadPort (
  IN EFI_ISA_IO_PROTOCOL   *IsaIo,
  IN UINT16                BaseAddress,
  IN UINT32                Offset
  );

VOID 
IsaSerialWritePort (
  IN EFI_ISA_IO_PROTOCOL   *IsaIo,
  IN UINT16                BaseAddress,
  IN UINT32                Offset,
  IN UINT8                 Data
  );
    
//
// ISA Serial Driver Global Variables
//

EFI_DRIVER_BINDING_PROTOCOL gSerialControllerDriver = {
  SerialControllerDriverSupported,
  SerialControllerDriverStart,
  SerialControllerDriverStop,
  0x10,
  NULL,
  NULL
};


EFI_DRIVER_ENTRY_POINT(SerialControllerDriverEntryPoint)

//
// ISA Bus Driver Support Functions
//

EFI_STATUS
SerialControllerDriverEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++
  
  Routine Description:
  
  Arguments:
  
  Returns:
  
--*/                
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gSerialControllerDriver, 
           ImageHandle,
           &gIsaSerialComponentName,
           NULL,
           NULL
           );
}


EFI_STATUS
SerialControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )

/*++

  Routine Description:

  Arguments:

  Returns:

--*/

{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;
  EFI_ISA_IO_PROTOCOL       *IsaIo;
  UART_DEVICE_PATH          UartNode;
 
  //
  // Ignore the RemainingDevicePath
  //

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,   
                  &gEfiDevicePathProtocolGuid,  
                  (VOID **)&ParentDevicePath,
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
         &gEfiDevicePathProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );

  Status = gBS->OpenProtocol (
                  Controller,  
                  &gEfiIsaIoProtocolGuid, 
                  (VOID **)&IsaIo,
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

  //
  // Use the ISA I/O Protocol to see if Controller is standard ISA UART that
  // can be managed by this driver.
  //
  Status = EFI_SUCCESS;
  if (IsaIo->ResourceList->Device.HID != EISA_PNP_ID(0x501)) {
    Status = EFI_UNSUPPORTED;
    goto Error;
  }

  //
  // Make sure RemainingDevicePath is valid
  //
  if (RemainingDevicePath != NULL) {
    Status = EFI_UNSUPPORTED;
    EfiCopyMem (&UartNode, (UART_DEVICE_PATH *)RemainingDevicePath, sizeof (UART_DEVICE_PATH));
    if (UartNode.Header.Type != MESSAGING_DEVICE_PATH ||
        UartNode.Header.SubType != MSG_UART_DP ||
        DevicePathNodeLength((EFI_DEVICE_PATH_PROTOCOL *)&UartNode) != sizeof(UART_DEVICE_PATH)) {
      goto Error;
    }
    if (UartNode.BaudRate < 0 || UartNode.BaudRate > SERIAL_PORT_MAX_BAUD_RATE) {
      goto Error;
    }
    if (UartNode.Parity < NoParity || UartNode.Parity > SpaceParity) {
      goto Error;
    }
    if (UartNode.DataBits < 5 || UartNode.DataBits > 8) {
      goto Error;
    }
    if (UartNode.StopBits < OneStopBit || UartNode.StopBits > TwoStopBits) {
      goto Error;
    }
    if ((UartNode.DataBits == 5) && (UartNode.StopBits == TwoStopBits)) {
      goto Error;
    }
    if ((UartNode.DataBits >= 6) && (UartNode.DataBits <=8) && (UartNode.StopBits == OneFiveStopBits)) {
      goto Error;
    }
    Status = EFI_SUCCESS;
  }

Error:
  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         Controller,  
         &gEfiIsaIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
         
  return Status;
}

EFI_STATUS
SerialControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )

/*++

  Routine Description:

  Arguments:

  Returns:

--*/

{
  EFI_STATUS                           Status;
  EFI_ISA_IO_PROTOCOL                  *IsaIo;
  SERIAL_DEV                           *SerialDevice;
  UINTN                                Index;
  UART_DEVICE_PATH                     Node;
  EFI_DEVICE_PATH_PROTOCOL             *ParentDevicePath;
  CHAR16                               SerialPortName[sizeof(SERIAL_PORT_NAME)];
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY  *OpenInfoBuffer;
  UINTN                                EntryCount;
  EFI_SERIAL_IO_PROTOCOL               *SerialIo;

  
  SerialDevice = NULL;
  //
  // Get the Parent Device Path
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
  // Grab the IO abstraction we need to get any work done
  //
  Status = gBS->OpenProtocol (
                  Controller, 
                  &gEfiIsaIoProtocolGuid, 
                  (VOID **)&IsaIo,
                  This->DriverBindingHandle,   
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER 
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    goto Error;
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
                    Controller,
                    &gEfiIsaIoProtocolGuid,
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
                        Controller,
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
  // Initialize the serial device instance
  //
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(SERIAL_DEV),
                  (VOID **)&SerialDevice
                  );
  
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  
  EfiZeroMem(SerialDevice, sizeof(SERIAL_DEV));

  SerialDevice->IsaIo               = IsaIo;
  SerialDevice->ParentDevicePath    = ParentDevicePath;
  SerialDevice->ControllerNameTable = NULL;

  EfiStrCpy(SerialPortName, L"ISA Serial Port # ");
  SerialPortName[sizeof(SERIAL_PORT_NAME)-2]
                      = (CHAR16)(L'0' + (UINT8)IsaIo->ResourceList->Device.UID);
  EfiLibAddUnicodeString (
    "eng", 
    gIsaSerialComponentName.SupportedLanguages, 
    &SerialDevice->ControllerNameTable, 
    (CHAR16 *)SerialPortName
    );
  
  for (Index = 0; SerialDevice->IsaIo->ResourceList->ResourceItem[Index].Type != EfiIsaAcpiResourceEndOfList; Index++) {
    if (SerialDevice->IsaIo->ResourceList->ResourceItem[Index].Type == EfiIsaAcpiResourceIo) {
      SerialDevice->BaseAddress = (UINT16)SerialDevice->IsaIo->ResourceList->ResourceItem[Index].StartRange;
    }
  }
  
  
  if (IsaSerialPortPresent (SerialDevice) != TRUE) {
    Status = EFI_DEVICE_ERROR;
    goto Error;
  }

  SerialDevice->Signature                   = SERIAL_DEV_SIGNATURE;
  SerialDevice->Type                        = UART16550A;
  SerialDevice->SoftwareLoopbackEnable      = FALSE;
  SerialDevice->HardwareFlowControl         = FALSE;
  SerialDevice->Handle                      = NULL;
  SerialDevice->Receive.First               = 0;
  SerialDevice->Receive.Last                = 0;
  SerialDevice->Receive.Surplus             = SERIAL_MAX_BUFFER_SIZE;
  SerialDevice->Transmit.First              = 0; 
  SerialDevice->Transmit.Last               = 0;
  SerialDevice->Transmit.Surplus            = SERIAL_MAX_BUFFER_SIZE;
  
  //
  // Serial I/O
  //
  SerialDevice->SerialIo.Revision           = SERIAL_IO_INTERFACE_REVISION;
  SerialDevice->SerialIo.Reset              = IsaSerialReset;
  SerialDevice->SerialIo.SetAttributes      = IsaSerialSetAttributes;
  SerialDevice->SerialIo.SetControl         = IsaSerialSetControl;
  SerialDevice->SerialIo.GetControl         = IsaSerialGetControl;
  SerialDevice->SerialIo.Write              = IsaSerialWrite;
  SerialDevice->SerialIo.Read               = IsaSerialRead;
  SerialDevice->SerialIo.Mode               = &(SerialDevice->SerialMode);

  if (RemainingDevicePath != NULL) {
    //
    // Match the configuration of the RemainingDevicePath. IsHandleSupported()
    // already checked to make sure the RemainingDevicePath contains settings
    // that we can support.
    //
    EfiCopyMem (&SerialDevice->UartDevicePath, RemainingDevicePath, sizeof (UART_DEVICE_PATH));
  } else {
    //
    // Build the device path by appending the UART node to the ParentDevicePath 
    // from the WinNtIo handle. The Uart setings are zero here, since 
    // SetAttribute() will update them to match the default setings.
    //
    EfiZeroMem (&SerialDevice->UartDevicePath, sizeof (UART_DEVICE_PATH));
    SerialDevice->UartDevicePath.Header.Type = MESSAGING_DEVICE_PATH;
    SerialDevice->UartDevicePath.Header.SubType = MSG_UART_DP;
    SetDevicePathNodeLength ((EFI_DEVICE_PATH_PROTOCOL *)&SerialDevice->UartDevicePath, sizeof(UART_DEVICE_PATH));
  }

  //
  // Build the device path by appending the UART node to the ParentDevicePath 
  // from the WinNtIo handle. The Uart setings are zero here, since 
  // SetAttribute() will update them to match the current setings.
  //
  SerialDevice->DevicePath = EfiAppendDevicePathNode (
                          ParentDevicePath, 
                          (EFI_DEVICE_PATH_PROTOCOL *)&SerialDevice->UartDevicePath
                          );
  
  if (SerialDevice->DevicePath == NULL) {
    Status =  EFI_DEVICE_ERROR;
    goto Error;
  }                        

  //
  // Fill in Serial I/O Mode structure based on either the RemainingDevicePath or defaults.
  //
  SerialDevice->SerialMode.ControlMask      = SERIAL_PORT_DEFAULT_CONTROL_MASK;
  SerialDevice->SerialMode.Timeout          = SERIAL_PORT_DEFAULT_TIMEOUT;
  SerialDevice->SerialMode.BaudRate         = SerialDevice->UartDevicePath.BaudRate;
  SerialDevice->SerialMode.ReceiveFifoDepth = SERIAL_PORT_DEFAULT_RECEIVE_FIFO_DEPTH;
  SerialDevice->SerialMode.DataBits         = SerialDevice->UartDevicePath.DataBits;
  SerialDevice->SerialMode.Parity           = SerialDevice->UartDevicePath.Parity;
  SerialDevice->SerialMode.StopBits         = SerialDevice->UartDevicePath.StopBits;

  //
  // Issue a reset to initialize the COM port
  //
  Status = SerialDevice->SerialIo.Reset (&SerialDevice->SerialIo);
  if (EFI_ERROR (Status)) {
     goto Error;
  }

  //
  // Install protocol interfaces for the serial device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &SerialDevice->Handle,            
                  &gEfiDevicePathProtocolGuid, SerialDevice->DevicePath,
                  &gEfiSerialIoProtocolGuid,   &SerialDevice->SerialIo,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // Open For Child Device
  //
  Status = gBS->OpenProtocol (
                  Controller,   
                  &gEfiIsaIoProtocolGuid,  
                  (VOID **)&IsaIo,
                  This->DriverBindingHandle,   
                  SerialDevice->Handle,   
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );

Error:
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
         Controller,  
         &gEfiDevicePathProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    gBS->CloseProtocol (
         Controller,  
         &gEfiIsaIoProtocolGuid, 
         This->DriverBindingHandle,   
         Controller   
         );
    if (SerialDevice) {
      if (SerialDevice->DevicePath) {
        gBS->FreePool (SerialDevice->DevicePath);
      }
      EfiLibFreeUnicodeStringTable (SerialDevice->ControllerNameTable);
      gBS->FreePool (SerialDevice);     
    }
  }
  return Status;
}

EFI_STATUS
SerialControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/

{
  EFI_STATUS                    Status;
  UINTN                         Index;
  BOOLEAN                       AllChildrenStopped;
  EFI_SERIAL_IO_PROTOCOL        *SerialIo;
  SERIAL_DEV                    *SerialDevice;
  EFI_ISA_IO_PROTOCOL           *IsaIo;
  
  //
  // Complete all outstanding transactions to Controller.
  // Don't allow any new transaction to Controller to be started.
  //

  if (NumberOfChildren == 0) {
    //
    // Close the bus driver
    //
    Status =  gBS->CloseProtocol (
                     Controller, 
                     &gEfiIsaIoProtocolGuid, 
                     This->DriverBindingHandle, 
                     Controller
                     );

    Status =  gBS->CloseProtocol (
                     Controller, 
                     &gEfiDevicePathProtocolGuid, 
                     This->DriverBindingHandle, 
                     Controller
                     );
    return Status;
  }

  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {

    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index], 
                    &gEfiSerialIoProtocolGuid, 
                    (VOID **)&SerialIo,
                    This->DriverBindingHandle,   
                    Controller,   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR(Status)) {

      SerialDevice = SERIAL_DEV_FROM_THIS (SerialIo);

      Status =  gBS->CloseProtocol (
                       Controller, 
                       &gEfiIsaIoProtocolGuid, 
                       This->DriverBindingHandle, 
                       ChildHandleBuffer[Index]
                       );

      Status = gBS->UninstallMultipleProtocolInterfaces (
                 ChildHandleBuffer[Index], 
                 &gEfiDevicePathProtocolGuid,      SerialDevice->DevicePath,
                 &gEfiSerialIoProtocolGuid,        &SerialDevice->SerialIo,
                 NULL
                 );
      if (EFI_ERROR (Status)) {
        gBS->OpenProtocol (
               Controller,   
               &gEfiIsaIoProtocolGuid,  
               (VOID **)&IsaIo,
               This->DriverBindingHandle,   
               ChildHandleBuffer[Index],
               EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
               );
      } else {
        if (SerialDevice->DevicePath) {
          gBS->FreePool (SerialDevice->DevicePath);
        }
        EfiLibFreeUnicodeStringTable (SerialDevice->ControllerNameTable);
        gBS->FreePool (SerialDevice);
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



BOOLEAN
IsaSerialFifoFull(
  IN SERIAL_DEV_FIFO *Fifo
)
/*++

  Routine Description:
  Detect whether specific FIFO is full or not
  
  Arguments:
  Fifo  SERIAL_DEV_FIFO *: A pointer to the Data Structure SERIAL_DEV_FIFO
    
  Returns:
  TRUE:  the FIFO is full
  FALSE: the FIFO is not full

--*/
{
  if (Fifo->Surplus == 0) {
    return TRUE;
  }
  
  return FALSE;
}


BOOLEAN
IsaSerialFifoEmpty(
  IN SERIAL_DEV_FIFO *Fifo
)
/*++

  Routine Description:
  Detect whether specific FIFO is empty or not
  
  Arguments:
    Fifo  SERIAL_DEV_FIFO *: A pointer to the Data Structure SERIAL_DEV_FIFO
    
  Returns:
    TRUE:  the FIFO is empty
    FALSE: the FIFO is not empty

--*/
{
  if (Fifo->Surplus == SERIAL_MAX_BUFFER_SIZE ) {
    return TRUE;
  }
  
  return FALSE;
}


EFI_STATUS
IsaSerialFifoAdd(
  IN SERIAL_DEV_FIFO *Fifo,
  IN UINT8           Data
)
/*++

  Routine Description:
  Add data to specific FIFO
  
  Arguments:
    Fifo  SERIAL_DEV_FIFO *: A pointer to the Data Structure SERIAL_DEV_FIFO
    Data  UINT8: the data added to FIFO  
  
  Returns:
    EFI_SUCCESS:  Add data to specific FIFO successfully
    EFI_OUT_RESOURCE: Failed to add data because FIFO is already full 

--*/
{
  //
  //if FIFO full can not add data
  //
  if (IsaSerialFifoFull(Fifo)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  //
  //FIFO is not full can add data
  //
  Fifo->Data[Fifo->Last] = Data;
  Fifo->Surplus --;
  Fifo->Last ++;
  if (Fifo->Last == SERIAL_MAX_BUFFER_SIZE) {
    Fifo->Last = 0;
  }
  
  return EFI_SUCCESS;
}


EFI_STATUS
IsaSerialFifoRemove(
  IN  SERIAL_DEV_FIFO *Fifo,
  OUT UINT8           *Data
)
/*++

  Routine Description:
  Remove data from specific FIFO
  
  Arguments:
    Fifo  SERIAL_DEV_FIFO *: A pointer to the Data Structure SERIAL_DEV_FIFO
    Data  UINT8*: the data removed from FIFO  
  
  Returns:
    EFI_SUCCESS:  Remove data from specific FIFO successfully
    EFI_OUT_RESOURCE: Failed to remove data because FIFO is empty

--*/
{
  //
  //if FIFO is empty, no data can remove
  //
  if (IsaSerialFifoEmpty(Fifo)) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  //
  //FIFO is not empty, can remove data
  //
  *Data = Fifo->Data[Fifo->First];
  Fifo->Surplus ++;
  Fifo->First ++;
  if (Fifo->First == SERIAL_MAX_BUFFER_SIZE) {
    Fifo->First = 0;
  }
  
  return EFI_SUCCESS;
}


EFI_STATUS
IsaSerialReceiveTransmit (
  IN SERIAL_DEV *SerialDevice
  )
/*++

  Routine Description:
  Reads and writes all avaliable data.
  
  Arguments:
    SerialDevice - The device to flush
  
  Returns:
    EFI_SUCCESS:  Data was read/written successfully.
    EFI_OUT_RESOURCE: Failed because software receive FIFO is full.  Note, when
      this happens, pending writes are not done.

--*/
{
  SERIAL_PORT_LSR Lsr;
  UINT8           Data;
  BOOLEAN         ReceiveFifoFull;
  SERIAL_PORT_MSR Msr;
  SERIAL_PORT_MCR Mcr;
  UINTN           TimeOut;

  
  Data = 0;

  //
  // Begin the read or write
  //
  if (SerialDevice->SoftwareLoopbackEnable) {
    do {
      ReceiveFifoFull = IsaSerialFifoFull (&SerialDevice->Receive);
      if (!IsaSerialFifoEmpty (&SerialDevice->Transmit)) {
        IsaSerialFifoRemove (&SerialDevice->Transmit,&Data);
        if (ReceiveFifoFull) {
          return EFI_OUT_OF_RESOURCES;
        }
        IsaSerialFifoAdd (&SerialDevice->Receive, Data);
      }
    } while (!IsaSerialFifoEmpty (&SerialDevice->Transmit));
  } else {
    ReceiveFifoFull = IsaSerialFifoFull (&SerialDevice->Receive);
    do {
      Lsr.Data = READ_LSR (SerialDevice->IsaIo, SerialDevice->BaseAddress);
#ifdef EFI_NT_EMULATOR
      //
      // This is required for NT to avoid a forever-spin...
      // This would be better if READ_LSR was a polling operation 
      // that would timeout.  
      //
      Lsr.Bits.THRE = 1;
#endif
      //
      // Flush incomming data to prevent a an overrun during a long write
      //
      if (Lsr.Bits.DR && !ReceiveFifoFull) {
        ReceiveFifoFull = IsaSerialFifoFull (&SerialDevice->Receive);
        if (!ReceiveFifoFull) {
          if (Lsr.Bits.FIFOE || Lsr.Bits.OE || Lsr.Bits.PE || Lsr.Bits.FE || Lsr.Bits.BI) {
            if (Lsr.Bits.FIFOE || Lsr.Bits.PE || Lsr.Bits.FE || Lsr.Bits.BI) {
              Data = READ_RBR (SerialDevice->IsaIo, SerialDevice->BaseAddress);
              continue;
            }
          }
          //
          // Make sure the receive data will not be missed, Assert DTR
          //
          if (SerialDevice->HardwareFlowControl) {
            Mcr.Data = READ_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress);
            Mcr.Bits.DTRC &= 0;
            WRITE_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress, Mcr.Data);
          }

          Data = READ_RBR (SerialDevice->IsaIo, SerialDevice->BaseAddress);

          //
          // Deassert DTR
          //
          if (SerialDevice->HardwareFlowControl) {
            Mcr.Data = READ_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress);
            Mcr.Bits.DTRC |= 1;
            WRITE_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress, Mcr.Data);
          }
          
          IsaSerialFifoAdd(&SerialDevice->Receive, Data);

          continue;
        } else {
                  
        }
      }
      //
      // Do the write
      //
      if (Lsr.Bits.THRE && !IsaSerialFifoEmpty (&SerialDevice->Transmit)) {
        //
        // Make sure the transmit data will not be missed
        //
        if (SerialDevice->HardwareFlowControl) {
          //
          // Send RTS
          //
          Mcr.Data = READ_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress);
          Mcr.Bits.RTS |= 1;
          WRITE_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress, Mcr.Data);
          //
          // Wait for CTS
          //
          TimeOut = 0;
          Msr.Data = READ_MSR(SerialDevice->IsaIo, SerialDevice->BaseAddress);
          while (!Msr.Bits.CTS) {
            gBS->Stall (TIMEOUT_STALL_INTERVAL);
            TimeOut ++;
            if (TimeOut > 5) break;
            Msr.Data = READ_MSR(SerialDevice->IsaIo, SerialDevice->BaseAddress);
          }
          if (Msr.Bits.CTS) {
            IsaSerialFifoRemove (&SerialDevice->Transmit,&Data);
            WRITE_THR (SerialDevice->IsaIo, SerialDevice->BaseAddress,Data);
          }
        }

        //
        // write the data out
        //
        if (!SerialDevice->HardwareFlowControl) {
          IsaSerialFifoRemove (&SerialDevice->Transmit,&Data);
          WRITE_THR (SerialDevice->IsaIo, SerialDevice->BaseAddress,Data);
        }

        //
        // Make sure the transmit data will not be missed
        //
        if (SerialDevice->HardwareFlowControl) {
          //
          // Assert RTS
          //
          Mcr.Data = READ_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress);
          Mcr.Bits.RTS &= 0;
          WRITE_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress, Mcr.Data);
        }
      }
    } while (Lsr.Bits.THRE && !IsaSerialFifoEmpty (&SerialDevice->Transmit));
  }
  return EFI_SUCCESS;
}

//
//Interface Functions
//

EFI_STATUS
IsaSerialReset (
  IN EFI_SERIAL_IO_PROTOCOL  *This
  )
/*++

  Routine Description:
  Reset serial device 
  
  Arguments:
  This  EFI_SERIAL_IO_PROTOCOL *: Pointer to EFI_SERIAL_IO_PROTOCOL    
  
  Returns:
    EFI_SUCCESS:  Reset successfully
    EFI_DEVICE_ERROR: Failed to reset 

--*/
{
  EFI_STATUS      Status;
  SERIAL_DEV      *SerialDevice;
  SERIAL_PORT_LCR Lcr;
  SERIAL_PORT_IER Ier;
  SERIAL_PORT_MCR Mcr;
  SERIAL_PORT_FCR Fcr;

  SerialDevice = SERIAL_DEV_FROM_THIS (This);

    
  //
  // Make sure DLAB is 0.
  //
  Lcr.Data = READ_LCR (SerialDevice->IsaIo, SerialDevice->BaseAddress);
  Lcr.Bits.DLAB = 0;
  WRITE_LCR(SerialDevice->IsaIo, SerialDevice->BaseAddress,Lcr.Data);
  
  //
  // Turn off all interrupts
  //
  Ier.Data = READ_IER(SerialDevice->IsaIo, SerialDevice->BaseAddress);
  Ier.Bits.RAVIE = 0;
  Ier.Bits.THEIE = 0;
  Ier.Bits.RIE   = 0;
  Ier.Bits.MIE   = 0;
  WRITE_IER(SerialDevice->IsaIo, SerialDevice->BaseAddress,Ier.Data); 

  //
  //Disable the FIFO.
  //
  Fcr.Bits.TRFIFOE = 0;
  WRITE_FCR(SerialDevice->IsaIo, SerialDevice->BaseAddress,Fcr.Data);

  //
  // Turn off loopback and disable device interrupt.
  //
  Mcr.Data = READ_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress);
  Mcr.Bits.OUT1 = 0;
  Mcr.Bits.OUT2 = 0;
  Mcr.Bits.LME  = 0;
  WRITE_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress,Mcr.Data);

  //
  // Clear the scratch pad register
  //
  WRITE_SCR(SerialDevice->IsaIo, SerialDevice->BaseAddress,0);

  //
  // Go set the current attributes
  //
  Status = This->SetAttributes(
    This,
    This->Mode->BaudRate,
    This->Mode->ReceiveFifoDepth,
    This->Mode->Timeout,
    This->Mode->Parity,
    (UINT8)This->Mode->DataBits,
    This->Mode->StopBits);
  
  if (EFI_ERROR(Status)) {
    //return Status;
    return EFI_DEVICE_ERROR;
  }

  //
  // Go set the current control bits
  //
  Status = This->SetControl(
    This,
    This->Mode->ControlMask);

  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // for 16550A enable FIFO, 16550 disable FIFO
  //
  Fcr.Bits.TRFIFOE = 1;
  Fcr.Bits.RESETRF = 1;
  Fcr.Bits.RESETTF = 1;  
  WRITE_FCR(SerialDevice->IsaIo, SerialDevice->BaseAddress,Fcr.Data);
  
  //
  // Reset the software FIFO
  //
  SerialDevice->Receive.First    = 0;
  SerialDevice->Receive.Last     = 0;
  SerialDevice->Receive.Surplus  = SERIAL_MAX_BUFFER_SIZE;
  SerialDevice->Transmit.First   = 0;
  SerialDevice->Transmit.Last    = 0;
  SerialDevice->Transmit.Surplus = SERIAL_MAX_BUFFER_SIZE;

  //
  // Device reset is complete
  //
  return EFI_SUCCESS;
}


EFI_STATUS
IsaSerialSetAttributes (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN UINT64                  BaudRate,
  IN UINT32                  ReceiveFifoDepth,
  IN UINT32                  Timeout,
  IN EFI_PARITY_TYPE         Parity,
  IN UINT8                   DataBits,
  IN EFI_STOP_BITS_TYPE      StopBits
  )
/*++

  Routine Description:
    Set new attributes to a serial device 
  
  Arguments:
    This  EFI_SERIAL_IO_PROTOCOL *: Pointer to EFI_SERIAL_IO_PROTOCOL    
    BaudRate  UINT64: The baudrate of the serial device
    ReceiveFifoDepth UINT32: 
    Timeout   UINT32: The request timeout for a single char
    Parity    EFI_PARITY_TYPE: The type of parity used in serial device
    DataBits  UINT8: Number of databits used in serial device
    StopBits  EFI_STOP_BITS_TYPE: Number of stopbits used in serial device
  
  Returns:
    EFI_SUCCESS:  The new attributes were set 
    EFI_INVALID_PARAMETERS: One or more attributes have an unsupported value
    EFI_UNSUPPORTED: Data Bits can not set to 5 or 6
    EFI_DEVICE_ERROR:The serial device is not functioning correctly (no return)

--*/
{
  EFI_STATUS                Status;
  SERIAL_DEV                *SerialDevice;
  UINT32                    Divisor;
  UINTN                     Remained;
  SERIAL_PORT_LCR           Lcr;
  EFI_DEVICE_PATH_PROTOCOL  *NewDevicePath;

  SerialDevice = SERIAL_DEV_FROM_THIS (This);

  //
  // Check for default settings and fill in actual values.
  //
  if (BaudRate == 0) {
    BaudRate = SERIAL_PORT_DEFAULT_BAUD_RATE;
  }

  if (ReceiveFifoDepth == 0) {
    ReceiveFifoDepth = SERIAL_PORT_DEFAULT_RECEIVE_FIFO_DEPTH;
  }

  if (Timeout == 0) {
    Timeout = SERIAL_PORT_DEFAULT_TIMEOUT;
  }

  if (Parity == DefaultParity) {
    Parity = SERIAL_PORT_DEFAULT_PARITY; 
  }

  if (DataBits == 0) {
    DataBits = SERIAL_PORT_DEFAULT_DATA_BITS;
  }

  if (StopBits == DefaultStopBits) {
    StopBits = SERIAL_PORT_DEFAULT_STOP_BITS;
  }
  
  //
  // 5 and 6 data bits can not be verified on a 16550A UART
  // Return EFI_INVALID_PARAMETER if an attempt is made to use these settings.
  //

  if ((DataBits == 5) || (DataBits == 6)) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Make sure all parameters are valid
  //
  
  if ((BaudRate > SERIAL_PORT_MAX_BAUD_RATE) || (BaudRate < SERIAL_PORT_MIN_BAUD_RATE)) { 
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // 50,75,110,134,150,300,600,1200,1800,2000,2400,3600,4800,7200,9600,19200,
  // 38400,57600,115200
  //
  if (BaudRate < 75) {
    BaudRate = 50;
  } else if (BaudRate < 110) {
    BaudRate = 75;
  } else if (BaudRate < 134) {
    BaudRate = 110;
  } else if (BaudRate < 150) { 
    BaudRate = 134;
  } else if (BaudRate < 300) {
    BaudRate = 150;
  } else if (BaudRate < 600) {
    BaudRate = 300;
  } else if (BaudRate < 1200) {
    BaudRate = 600;
  } else if (BaudRate < 1800) {
    BaudRate = 1200;
  } else if (BaudRate < 2000) {
    BaudRate = 1800;
  } else if (BaudRate < 2400) {
    BaudRate = 2000;
  } else if (BaudRate < 3600) {
    BaudRate = 2400;
  } else if (BaudRate < 4800) {
    BaudRate = 3600;
  } else if (BaudRate < 7200) {
    BaudRate = 4800;
  } else if (BaudRate < 9600) {
    BaudRate = 7200;      
  } else if (BaudRate < 19200) {
    BaudRate = 9600;
  } else if (BaudRate < 38400) {
    BaudRate = 19200;
  } else if (BaudRate < 57600) {
    BaudRate = 38400;
  } else if (BaudRate < 115200) {
    BaudRate = 57600;
  } else if (BaudRate <= SERIAL_PORT_MAX_BAUD_RATE) {
    BaudRate = 115200;              
  }
  
  if ((ReceiveFifoDepth < 1) || (ReceiveFifoDepth > SERIAL_PORT_MAX_RECEIVE_FIFO_DEPTH)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Timeout < SERIAL_PORT_MIN_TIMEOUT) || (Timeout > SERIAL_PORT_MAX_TIMEOUT)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Parity < NoParity) || (Parity > SpaceParity)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DataBits < 5) || (DataBits > 8)) {
    return EFI_INVALID_PARAMETER;
  }
  
  if ((StopBits < OneStopBit) || (StopBits > TwoStopBits)) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  //for DataBits = 5, StopBits can not set TwoStopBits
  //
  //if ((DataBits == 5) && (StopBits == TwoStopBits)) {
  //  return EFI_INVALID_PARAMETER;
  //}
  
  //
  //for DataBits = 6,7,8, StopBits can not set OneFiveStopBits
  //
  if ((DataBits >= 6) && (DataBits <=8) && (StopBits == OneFiveStopBits)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // See if the new attributes already match the current attributes
  //
  if (SerialDevice->UartDevicePath.BaudRate     == BaudRate         &&
      SerialDevice->UartDevicePath.DataBits     == DataBits         &&
      SerialDevice->UartDevicePath.Parity       == Parity           &&
      SerialDevice->UartDevicePath.StopBits     == StopBits         &&
      SerialDevice->SerialMode.ReceiveFifoDepth == ReceiveFifoDepth &&
      SerialDevice->SerialMode.Timeout          == Timeout             ) {
    return EFI_SUCCESS;
  }

  //
  // Compute divisor use to program the baud rate using a round determination
  //
  Divisor = (UINT32) DriverLibDivU64x32 (SERIAL_PORT_INPUT_CLOCK ,((UINT32)BaudRate * 16), &Remained);

  if (Remained) {
    Divisor += 1;
  }
  
  if ((Divisor == 0) || (Divisor & 0xffff0000)) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Compute the actual baud rate that the serial port will be programmed for.
  //
  BaudRate = SERIAL_PORT_INPUT_CLOCK / Divisor / 16;
  
  //
  // Put serial port on Divisor Latch Mode
  //
  Lcr.Data = READ_LCR(SerialDevice->IsaIo, SerialDevice->BaseAddress);
  Lcr.Bits.DLAB = 1;
  WRITE_LCR(SerialDevice->IsaIo, SerialDevice->BaseAddress,Lcr.Data);

  //
  // Write the divisor to the serial port
  //
  WRITE_DLL(SerialDevice->IsaIo, SerialDevice->BaseAddress,(UINT8)(Divisor & 0xff));
  WRITE_DLM(SerialDevice->IsaIo, SerialDevice->BaseAddress,(UINT8)((Divisor>>8) & 0xff));

  //
  // Put serial port back in normal mode and set remaining attributes.
  //
  Lcr.Bits.DLAB = 0;

  switch (Parity) {
    case NoParity:
      Lcr.Bits.PAREN   = 0;
      Lcr.Bits.EVENPAR = 0;
      Lcr.Bits.STICPAR = 0;
      break;
    case EvenParity:
      Lcr.Bits.PAREN   = 1;
      Lcr.Bits.EVENPAR = 1;
      Lcr.Bits.STICPAR = 0;
      break;
    case OddParity:
      Lcr.Bits.PAREN   = 1;
      Lcr.Bits.EVENPAR = 0;
      Lcr.Bits.STICPAR = 0;
      break;
    case SpaceParity:
      Lcr.Bits.PAREN   = 1;
      Lcr.Bits.EVENPAR = 1;
      Lcr.Bits.STICPAR = 1;
      break;
    case MarkParity:
      Lcr.Bits.PAREN   = 1;
      Lcr.Bits.EVENPAR = 0;
      Lcr.Bits.STICPAR = 1;
      break;
  }

  switch (StopBits) {
    case OneStopBit :
      Lcr.Bits.STOPB = 0;
      break;
    case OneFiveStopBits :
    case TwoStopBits :
      Lcr.Bits.STOPB = 1;
      break;
  }

  //
  //DataBits
  //
  Lcr.Bits.SERIALDB = (UINT8)((DataBits - 5) & 0x03);
  WRITE_LCR(SerialDevice->IsaIo, SerialDevice->BaseAddress,Lcr.Data);

  //
  //Set the Serial I/O mode
  //
  This->Mode->BaudRate         = BaudRate;
  This->Mode->ReceiveFifoDepth = ReceiveFifoDepth;
  This->Mode->Timeout          = Timeout;
  This->Mode->Parity           = Parity;
  This->Mode->DataBits         = DataBits;
  This->Mode->StopBits         = StopBits;

  //
  // See if Device Path Node has actually changed
  //
  if (SerialDevice->UartDevicePath.BaudRate     == BaudRate &&
      SerialDevice->UartDevicePath.DataBits     == DataBits &&
      SerialDevice->UartDevicePath.Parity       == Parity   &&
      SerialDevice->UartDevicePath.StopBits     == StopBits    ) {
    return EFI_SUCCESS;
  }

  //
  // Update the device path
  //
  SerialDevice->UartDevicePath.BaudRate = BaudRate;
  SerialDevice->UartDevicePath.DataBits = DataBits;
  SerialDevice->UartDevicePath.Parity   = (UINT8)Parity;
  SerialDevice->UartDevicePath.StopBits = (UINT8)StopBits;

  NewDevicePath = EfiAppendDevicePathNode (
                    SerialDevice->ParentDevicePath,
                    (EFI_DEVICE_PATH_PROTOCOL *)&SerialDevice->UartDevicePath
                    );
  if (NewDevicePath == NULL) {
    return EFI_DEVICE_ERROR;
  }

  if (SerialDevice->Handle != NULL) {
    Status = gBS->ReinstallProtocolInterface (
                    SerialDevice->Handle,                 
                    &gEfiDevicePathProtocolGuid, 
                    SerialDevice->DevicePath, 
                    NewDevicePath
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (SerialDevice->DevicePath) {
    gBS->FreePool (SerialDevice->DevicePath);
  }
  SerialDevice->DevicePath = NewDevicePath;

  return EFI_SUCCESS;
}


EFI_STATUS
IsaSerialSetControl (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN UINT32               Control
  )
/*++

  Routine Description:
  Set ControlBits 
  
  Arguments:
  This  EFI_SERIAL_IO_PROTOCOL *: Pointer to EFI_SERIAL_IO_PROTOCOL    
  Control   UINT32: Control bits that can be settable   
  
  Returns:
  EFI_SUCCESS:  New Control bits were set successfully
  EFI_UNSUPPORTED: The Control bits wanted to set are not supported

--*/
{
  SERIAL_DEV      *SerialDevice;
  SERIAL_PORT_MCR Mcr;

  //
  // The control bits that can be set are :
  //     EFI_SERIAL_DATA_TERMINAL_READY: 0x0001  // WO
  //     EFI_SERIAL_REQUEST_TO_SEND: 0x0002  // WO
  //     EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE: 0x1000  // RW
  //     EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE: 0x2000  // RW
  //
  
  SerialDevice = SERIAL_DEV_FROM_THIS (This);

  //
  // first determine the parameter is invalid
  //
  if (Control & 0xffff8ffc) {
    return EFI_UNSUPPORTED;
  }
  
  Mcr.Data = READ_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress);
  Mcr.Bits.DTRC = 0;
  Mcr.Bits.RTS  = 0;
  Mcr.Bits.LME  = 0;
  SerialDevice->SoftwareLoopbackEnable = FALSE;
  SerialDevice->HardwareFlowControl = FALSE;

  if (Control & EFI_SERIAL_DATA_TERMINAL_READY) {
    Mcr.Bits.DTRC = 1;
  }
  
  if (Control & EFI_SERIAL_REQUEST_TO_SEND) {
    Mcr.Bits.RTS  = 1;
  }
  
  if (Control & EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE) {
    Mcr.Bits.LME  = 1;
  }
  
  if (Control & EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE) {
    SerialDevice->HardwareFlowControl = TRUE;
  }
  
  WRITE_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress,Mcr.Data);
  
  if (Control & EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE) {
    SerialDevice->SoftwareLoopbackEnable = TRUE;
  }
   
  return EFI_SUCCESS;
}


EFI_STATUS
IsaSerialGetControl (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  OUT UINT32              *Control
  )
/*++

  Routine Description:
  Get ControlBits 
  
  Arguments:
  This  EFI_SERIAL_IO_PROTOCOL *: Pointer to EFI_SERIAL_IO_PROTOCOL    
  Control   UINT32 *: Control signals of the serial device  
  
  Returns:
    EFI_SUCCESS:  Get Control signals successfully

--*/
{
  SERIAL_DEV      *SerialDevice;
  SERIAL_PORT_MSR Msr;
  SERIAL_PORT_MCR Mcr;

  SerialDevice = SERIAL_DEV_FROM_THIS (This);

  *Control = 0;

  //
  // Read the Modem Status Register
  //

  Msr.Data = READ_MSR(SerialDevice->IsaIo, SerialDevice->BaseAddress);

  if (Msr.Bits.CTS) {
    *Control |= EFI_SERIAL_CLEAR_TO_SEND;
  }

  if (Msr.Bits.DSR) {
    *Control |= EFI_SERIAL_DATA_SET_READY;
  }

  if (Msr.Bits.RI) {
    *Control |= EFI_SERIAL_RING_INDICATE;
  }

  if (Msr.Bits.DCD) {
    *Control |= EFI_SERIAL_CARRIER_DETECT;
  }

  //
  // Read the Modem Control Register
  //

  Mcr.Data = READ_MCR(SerialDevice->IsaIo, SerialDevice->BaseAddress);

  if (Mcr.Bits.DTRC) {
    *Control |= EFI_SERIAL_DATA_TERMINAL_READY;
  }
  
  if (Mcr.Bits.RTS) {
    *Control |= EFI_SERIAL_REQUEST_TO_SEND;
  }
  
  if (Mcr.Bits.LME) {
    *Control |= EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE;
  }

  if (SerialDevice->HardwareFlowControl) {
    *Control |= EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE;
  }

  //
  // See if the Transmit FIFO is empty
  //
  IsaSerialReceiveTransmit (SerialDevice);

  if (IsaSerialFifoEmpty (&SerialDevice->Transmit)) {
    *Control |= EFI_SERIAL_OUTPUT_BUFFER_EMPTY;
  }

  //
  // See if the Receive FIFO is empty.
  //
  IsaSerialReceiveTransmit (SerialDevice);

  if (IsaSerialFifoEmpty (&SerialDevice->Receive)) {
    *Control |= EFI_SERIAL_INPUT_BUFFER_EMPTY;
  }

  if (SerialDevice->SoftwareLoopbackEnable) {
    *Control |= EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE;
  }
  
  return EFI_SUCCESS;   
}


EFI_STATUS
IsaSerialWrite (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN OUT UINTN               *BufferSize,
  IN VOID                    *Buffer
  )
/*++

  Routine Descrition:
    Write the specified number of bytes to serial device 
  
  Arguments:
    This  EFI_SERIAL_IO_PROTOCOL *: Pointer to EFI_SERIAL_IO_PROTOCOL    
    BuferSize UINTN *: On input the size of Buffer, on output the amount of 
                       data actually written
    Buffer    VOID *:  The buffer of data to write    
  
  Returns:
    EFI_SUCCESS:  The data were written successfully
    EFI_DEVICE_ERROR: The device reported an error
    EFI_TIMEOUT:  The write operation was stopped due to timeout

--*/
{
  SERIAL_DEV *SerialDevice;
  UINT8      *CharBuffer;
  UINT32     Index;
  UINTN      Elapsed;
  UINTN      ActualWrite;
 
  
  SerialDevice = SERIAL_DEV_FROM_THIS (This);
  Elapsed = 0;
  ActualWrite = 0;
 
  
  if (*BufferSize == 0) {
    return EFI_SUCCESS;
  }
  
  if (!Buffer) {
    return EFI_DEVICE_ERROR;
  }
  
  SerialDevice->Transmit.First = 0;
  SerialDevice->Transmit.Last  = 0;
  SerialDevice->Transmit.Surplus = SERIAL_MAX_BUFFER_SIZE;
    
  CharBuffer = (UINT8 *)Buffer;
  
  for (Index = 0; Index < *BufferSize; Index ++) {
    IsaSerialFifoAdd(&SerialDevice->Transmit, CharBuffer[Index]);
  
    while (IsaSerialReceiveTransmit (SerialDevice) != EFI_SUCCESS ||
         IsaSerialFifoEmpty (&SerialDevice->Transmit) == FALSE) {

      //
      //  Unsuccessful write so check if timeout has expired, if not,
      //  stall for a bit, increment time elapsed, and try again
      //
      
//      if ( Elapsed >= This->Mode->Timeout ) {
//        *BufferSize = ActualWrite ; 
//        return EFI_TIMEOUT;
//      }
      
      gBS->Stall( TIMEOUT_STALL_INTERVAL );
      
      Elapsed += TIMEOUT_STALL_INTERVAL;
    } //end while
    
    ActualWrite ++; 
    //
    //  Successful write so reset timeout
    //
    Elapsed = 0;
      
  } //end for
  
  return EFI_SUCCESS;
}


EFI_STATUS
IsaSerialRead (
  IN EFI_SERIAL_IO_PROTOCOL  *This,
  IN OUT UINTN               *BufferSize,
  OUT VOID                   *Buffer
  )
/*++

  Routine Description:
    Read the specified number of bytes from serial device 
  
  Arguments:
    This  EFI_SERIAL_IO_PROTOCOL *: Pointer to EFI_SERIAL_IO_PROTOCOL    
    BuferSize UINTN *: On input the size of Buffer, on output the amount of 
                       data returned in buffer
    Buffer    VOID *:  The buffer to return the data into 
  
  Returns:
    EFI_SUCCESS:  The data were read successfully
    EFI_DEVICE_ERROR: The device reported an error
    EFI_TIMEOUT:  The read operation was stopped due to timeout

--*/
{
  SERIAL_DEV      *SerialDevice;
  UINT32          Index;
  UINT8           *CharBuffer;
  UINTN           Elapsed; 
  EFI_STATUS      Status;
   
  SerialDevice = SERIAL_DEV_FROM_THIS (This);
  Elapsed = 0;
   
  if (*BufferSize == 0) {
    return EFI_SUCCESS;
  }
  
  if (!Buffer) {
    return EFI_DEVICE_ERROR;
  }
  
  //SerialDevice->Receive.First = 0;
  //SerialDevice->Receive.Last  = 0;
  //SerialDevice->Receive.Surplus = SERIAL_MAX_BUFFER_SIZE;
  
  Status = IsaSerialReceiveTransmit (SerialDevice);
  
  if (EFI_ERROR(Status)) {
    *BufferSize = 0;
    return EFI_DEVICE_ERROR;
  }
  
  CharBuffer = (UINT8 *)Buffer;
  for(Index=0;Index<*BufferSize;Index++) {
    while (IsaSerialFifoRemove(&SerialDevice->Receive, &(CharBuffer[Index])) != EFI_SUCCESS) {

      //
      //  Unsuccessful read so check if timeout has expired, if not,
      //  stall for a bit, increment time elapsed, and try again
      //  Need this time out to get conspliter to work.
      //

      if ( Elapsed >= This->Mode->Timeout ) {
        *BufferSize = Index;
        return EFI_TIMEOUT;
      }
      
      gBS->Stall( TIMEOUT_STALL_INTERVAL );            
      Elapsed += TIMEOUT_STALL_INTERVAL;
      
      Status = IsaSerialReceiveTransmit (SerialDevice);  
      if (Status == EFI_DEVICE_ERROR) {
        *BufferSize = Index;
        return EFI_DEVICE_ERROR;
      }
    }// end while

    //  Successful read so reset timeout
    Elapsed = 0;
  }// end for
  
  IsaSerialReceiveTransmit (SerialDevice);
  return EFI_SUCCESS;
}


BOOLEAN
IsaSerialPortPresent(
  IN SERIAL_DEV *SerialDevice
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/
{
  UINT8   Temp;
  BOOLEAN Status;
  
  
  Status  = TRUE;
  
  //
  // Save SCR reg
  //
  Temp = READ_SCR (SerialDevice->IsaIo, SerialDevice->BaseAddress);
  WRITE_SCR (SerialDevice->IsaIo, SerialDevice->BaseAddress, 0xAA);
  
  if (READ_SCR (SerialDevice->IsaIo, SerialDevice->BaseAddress) != 0xAA) {
#ifndef EFI_NT_EMULATOR
    Status = FALSE;
#endif
  }
  
  WRITE_SCR (SerialDevice->IsaIo, SerialDevice->BaseAddress, 0x55);
  
  if (READ_SCR (SerialDevice->IsaIo, SerialDevice->BaseAddress) != 0x55) {
#ifndef EFI_NT_EMULATOR
    Status = FALSE;
#endif
  }
  
  //
  // Restore SCR
  //
  WRITE_SCR (SerialDevice->IsaIo, SerialDevice->BaseAddress, Temp);
  return Status;
}

UINT8 
IsaSerialReadPort (
  IN EFI_ISA_IO_PROTOCOL   *IsaIo,
  IN UINT16                BaseAddress,
  IN UINT32                Offset
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/    
{
  UINT8        Data;

  //
  // Use IsaIo to access IO
  //
  IsaIo->Io.Read (
     IsaIo, 
     EfiIsaIoWidthUint8, 
     BaseAddress + Offset, 
     1, 
     &Data
     );
  return Data;
}

VOID 
IsaSerialWritePort (
  IN EFI_ISA_IO_PROTOCOL *IsaIo,
  IN UINT16              BaseAddress,
  IN UINT32              Offset,
  IN UINT8               Data
  )
/*++

  Routine Description:

  Arguments:

  Returns:

--*/    
{
  //
  // Use IsaIo to access IO
  //
  IsaIo->Io.Write (
    IsaIo, 
    EfiIsaIoWidthUint8, 
    BaseAddress + Offset, 
    1, 
    &Data
  );
}
