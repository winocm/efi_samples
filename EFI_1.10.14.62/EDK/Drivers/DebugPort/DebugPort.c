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

    DebugPort.c

Abstract:

    Top level C file for debugport driver.  Contains initialization function.
    This driver layers on top of SerialIo.
    
    ALL CODE IN THE SERIALIO STACK MUST BE RE-ENTRANT AND CALLABLE FROM
    INTERRUPT CONTEXT.

Revision History

--*/

#include "DebugPort.h"
#include "efiapi.h"

//
// Misc. functions local to this module
//
static
VOID
GetDebugPortVariable (
  DEBUGPORT_DEVICE  *DebugPortDevice
  );

static
EFI_STATUS
ImageUnloadHandler (
  EFI_HANDLE ImageHandle
  );


//
// Globals
//
DEBUGPORT_DEVICE    *gDebugPortDevice;
static UINT32       mHid16550;
static UINT32       mHidStdPcComPort;


//
// implementation code
//
EFI_DRIVER_ENTRY_POINT (InitializeDebugPortDriver);

EFI_STATUS
InitializeDebugPortDriver (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )
/*++

Routine Description:
  Driver entry point.  Reads DebugPort variable to determine what device and settings
  to use as the debug port.  Binds exclusively to SerialIo. Reverts to defaults \
  if no variable is found.  
  
  Creates debugport and devicepath protocols on new handle.

Arguments:
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable

Returns:

  EFI_STATUS

--*/
{
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImageInterface;

  mHid16550         = EFI_ACPI_16550UART_HID;
  mHidStdPcComPort  = EFI_ACPI_PC_COMPORT_HID;

  //
  // Check to be sure we have an EFI 1.1 core
  //
  if (SystemTable->Hdr.Revision < EFI_1_10_SYSTEM_TABLE_REVISION) {
    return (EFI_UNSUPPORTED);
  }

  //
  //  Initialize EFI driver library
  //
  EfiInitializeDriverLib (ImageHandle, SystemTable);

  //
  // Allocate and Initialize dev structure
  //
  gDebugPortDevice = EfiLibAllocateZeroPool (sizeof (DEBUGPORT_DEVICE));
  if (gDebugPortDevice == NULL) {
    return (EFI_OUT_OF_RESOURCES);
  }

  //
  // Fill in static and default pieces of device structure first.
  //
  gDebugPortDevice->Signature           = DEBUGPORT_DEVICE_SIGNATURE;

  gDebugPortDevice->DriverBindingInterface.Supported   = DebugPortSupported;
  gDebugPortDevice->DriverBindingInterface.Start       = DebugPortStart;
  gDebugPortDevice->DriverBindingInterface.Stop        = DebugPortStop;
  gDebugPortDevice->DriverBindingInterface.Version     = DEBUGPORT_DRIVER_VERSION;
  gDebugPortDevice->DriverBindingInterface.ImageHandle = ImageHandle;
  gDebugPortDevice->DriverBindingInterface.DriverBindingHandle = ImageHandle;

  gDebugPortDevice->ComponentNameInterface.GetDriverName      = DebugPortComponentNameGetDriverName;
  gDebugPortDevice->ComponentNameInterface.GetControllerName  = DebugPortComponentNameGetControllerName;
  gDebugPortDevice->ComponentNameInterface.SupportedLanguages = "eng";
  
  gDebugPortDevice->DebugPortInterface.Reset = DebugPortReset;
  gDebugPortDevice->DebugPortInterface.Read  = DebugPortRead;
  gDebugPortDevice->DebugPortInterface.Write = DebugPortWrite;
  gDebugPortDevice->DebugPortInterface.Poll  = DebugPortPoll;

  gDebugPortDevice->BaudRate         = DEBUGPORT_UART_DEFAULT_BAUDRATE;
  gDebugPortDevice->ReceiveFifoDepth = DEBUGPORT_UART_DEFAULT_FIFO_DEPTH;
  gDebugPortDevice->Timeout          = DEBUGPORT_UART_DEFAULT_TIMEOUT;
  gDebugPortDevice->Parity           = DEBUGPORT_UART_DEFAULT_PARITY;
  gDebugPortDevice->DataBits         = DEBUGPORT_UART_DEFAULT_DATA_BITS;
  gDebugPortDevice->StopBits         = DEBUGPORT_UART_DEFAULT_STOP_BITS;
  
  //
  //  Install unload handler...
  //
  gBS->OpenProtocol (
                  ImageHandle,  
                  &gEfiLoadedImageProtocolGuid, 
                  &LoadedImageInterface,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  LoadedImageInterface->Unload = ImageUnloadHandler;

  //
  // Publish EFI 1.10 driver model protocols
  // 
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gDebugPortDevice->DriverBindingInterface, 
           ImageHandle,
           &gDebugPortDevice->ComponentNameInterface,
           NULL,
           NULL
           );
}


//
// DebugPort driver binding member functions...
//
EFI_STATUS
DebugPortSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:
  Checks to see that there's not already a DebugPort interface somewhere.  If so,
  fail.
  
  If there's a DEBUGPORT variable, the device path must match exactly.  If there's
  no DEBUGPORT variable, then device path is not checked and does not matter.
  
  Checks to see that there's a serial io interface on the controller handle
  that can be bound BY_DRIVER | EXCLUSIVE.
  
  If all these tests succeed, then we return EFI_SUCCESS, else, EFI_UNSUPPORTED
  or other error returned by OpenProtocol.

Arguments:

Returns:
  
--*/
{
  EFI_STATUS                  Status;
  EFI_DEVICE_PATH_PROTOCOL    *Dp1, *Dp2;
  EFI_SERIAL_IO_PROTOCOL      *SerialIo;
  EFI_DEBUGPORT_PROTOCOL      *DebugPortInterface;
  EFI_HANDLE                  TempHandle;

  //
  // Check to see that there's not a debugport protocol already published
  //
  if (gBS->LocateProtocol (&gEfiDebugPortProtocolGuid, NULL, (VOID **)&DebugPortInterface) !=
        EFI_NOT_FOUND) {
    return (EFI_UNSUPPORTED);
  }
                            
  //
  // Read DebugPort variable to determine debug port selection and parameters
  //
  GetDebugPortVariable (gDebugPortDevice);
                                             
  if (gDebugPortDevice->DebugPortVariable != NULL) {
    //
    // There's a DEBUGPORT variable, so do LocateDevicePath and check to see if
    // the closest matching handle matches the controller handle, and if it does,
    // check to see that the remaining device path has the DebugPort GUIDed messaging
    // device path only.  Otherwise, it's a mismatch and EFI_UNSUPPORTED is returned.
    //
    Dp1 = EfiDuplicateDevicePath ((EFI_DEVICE_PATH_PROTOCOL *) gDebugPortDevice->DebugPortVariable);
    if (Dp1 == NULL) {
      return (EFI_OUT_OF_RESOURCES);
    }
    Dp2 = Dp1;
    
    Status = gBS->LocateDevicePath (&gEfiSerialIoProtocolGuid,
                                    &Dp2,
                                    &TempHandle);
                                    
    if (Status == EFI_SUCCESS && TempHandle != ControllerHandle) {
      Status = EFI_UNSUPPORTED;
    }                                
                                    
    if (Status == EFI_SUCCESS && 
        (Dp2->Type != 3 || Dp2->SubType != 10 || *((UINT16 *)Dp2->Length) != 20)) {
      Status = EFI_UNSUPPORTED;
    }
    
    if (Status == EFI_SUCCESS && 
      EfiCompareMem (&gEfiDebugPortDevicePathGuid, Dp2 + 1, sizeof (EFI_GUID))) {
      Status = EFI_UNSUPPORTED;
    }
    
    gBS->FreePool (Dp1);
    if (EFI_ERROR (Status)) {
      return (Status);
    }
  }
  
  Status = gBS->OpenProtocol (
                  ControllerHandle,  
                  &gEfiSerialIoProtocolGuid, 
                  &SerialIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
         ControllerHandle,  
         &gEfiSerialIoProtocolGuid, 
         This->DriverBindingHandle,
         ControllerHandle
         );

  return EFI_SUCCESS;
}


EFI_STATUS
DebugPortStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:
  Binds exclusively to serial io on the controller handle.  Produces DebugPort
  protocol and DevicePath on new handle.

Arguments:

Returns:
  
--*/
{
  EFI_STATUS                 Status;
  DEBUGPORT_DEVICE_PATH      DebugPortDP;
  EFI_DEVICE_PATH_PROTOCOL   EndDP;
  EFI_DEVICE_PATH_PROTOCOL   *Dp1;
                                           
  Status = gBS->OpenProtocol (
                  ControllerHandle,  
                  &gEfiSerialIoProtocolGuid, 
                  &gDebugPortDevice->SerialIoBinding,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gDebugPortDevice->SerialIoDeviceHandle = ControllerHandle;
  
  //
  // Initialize the Serial Io interface...
  //
  Status = gDebugPortDevice->SerialIoBinding->SetAttributes (
                                        gDebugPortDevice->SerialIoBinding,
                                        gDebugPortDevice->BaudRate,
                                        gDebugPortDevice->ReceiveFifoDepth,
                                        gDebugPortDevice->Timeout,
                                        gDebugPortDevice->Parity,
                                        gDebugPortDevice->DataBits,
                                        gDebugPortDevice->StopBits);
  if (EFI_ERROR (Status)) {
    gDebugPortDevice->BaudRate         = 0;
    gDebugPortDevice->Parity           = DefaultParity;
    gDebugPortDevice->DataBits         = 0;
    gDebugPortDevice->StopBits         = DefaultStopBits;
    gDebugPortDevice->ReceiveFifoDepth = 0;
    Status = gDebugPortDevice->SerialIoBinding->SetAttributes (
                                          gDebugPortDevice->SerialIoBinding,
                                          gDebugPortDevice->BaudRate,
                                          gDebugPortDevice->ReceiveFifoDepth,
                                          gDebugPortDevice->Timeout,
                                          gDebugPortDevice->Parity,
                                          gDebugPortDevice->DataBits,
                                          gDebugPortDevice->StopBits);
    if (EFI_ERROR (Status)) {
      gBS->CloseProtocol (
             ControllerHandle,  
             &gEfiSerialIoProtocolGuid, 
             This->DriverBindingHandle,
             ControllerHandle
             );
      return (Status);
    }                                      
  }                                      
  
  gDebugPortDevice->SerialIoBinding->Reset (gDebugPortDevice->SerialIoBinding);
  
  //
  // Create device path instance for DebugPort
  //
  DebugPortDP.Header.Type       = MESSAGING_DEVICE_PATH;
  DebugPortDP.Header.SubType    = MSG_VENDOR_DP;
  SetDevicePathNodeLength (&(DebugPortDP.Header), sizeof (DebugPortDP));
  gBS->CopyMem (&DebugPortDP.Guid, &gEfiDebugPortDevicePathGuid, sizeof (EFI_GUID));
  
  Dp1 = EfiDevicePathFromHandle (ControllerHandle);
  if (Dp1 == NULL) {
    Dp1 = &EndDP;
    SetDevicePathEndNode (Dp1);
  }
  gDebugPortDevice->DebugPortDevicePath = EfiAppendDevicePathNode (Dp1, (EFI_DEVICE_PATH_PROTOCOL *) &DebugPortDP);
  if (gDebugPortDevice->DebugPortDevicePath == NULL) {
    return (EFI_OUT_OF_RESOURCES);
  }
  
  //
  // Publish DebugPort and Device Path protocols
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gDebugPortDevice->DebugPortDeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  gDebugPortDevice->DebugPortDevicePath,
                  &gEfiDebugPortProtocolGuid,
                  &gDebugPortDevice->DebugPortInterface,
                  NULL);
  
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
           ControllerHandle,  
           &gEfiSerialIoProtocolGuid, 
           This->DriverBindingHandle,
           ControllerHandle
           );
    return (Status);
  }                                      
  
  //
  // Connect debugport child to serial io
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,  
                  &gEfiSerialIoProtocolGuid, 
                  &gDebugPortDevice->SerialIoBinding,
                  This->DriverBindingHandle,
                  gDebugPortDevice->DebugPortDeviceHandle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG_CODE (
      {
        UINTN BufferSize = 48;
        DebugPortWrite (
          &gDebugPortDevice->DebugPortInterface,
          0,
          &BufferSize,
          "DebugPort driver failed to open child controller\n\n");
      }
    )
    gBS->CloseProtocol (
           ControllerHandle,  
           &gEfiSerialIoProtocolGuid, 
           This->DriverBindingHandle,
           ControllerHandle
           );
    return (Status);
  }
                  
  DEBUG_CODE (
    {
      UINTN BufferSize = 38;
      DebugPortWrite (
        &gDebugPortDevice->DebugPortInterface,
        0,
        &BufferSize,
        "Hello World from the DebugPort driver\n\n");
    }
  )    
  
  return EFI_SUCCESS;
}


EFI_STATUS
DebugPortStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
/*++

Routine Description:
  We're never intending to be stopped via the driver model so this just returns
  EFI_UNSUPPORTED

Arguments:
  Per EFI 1.10 driver model
  
Returns:
  EFI_UNSUPPORTED
  
--*/
{
  EFI_STATUS  Status;

  if (NumberOfChildren == 0) {
    //
    // Close the bus driver
    //
    gBS->CloseProtocol (
            ControllerHandle, 
            &gEfiSerialIoProtocolGuid, 
            This->DriverBindingHandle, 
            ControllerHandle);

    gDebugPortDevice->SerialIoBinding = NULL;
    
    gBS->CloseProtocol (
            ControllerHandle,   
            &gEfiDevicePathProtocolGuid,  
            This->DriverBindingHandle,   
            ControllerHandle);

    gBS->FreePool (gDebugPortDevice->DebugPortDevicePath);
    
    return (EFI_SUCCESS);
  } else {
    //
    // Disconnect SerialIo child handle
    //
    Status = gBS->CloseProtocol (
            gDebugPortDevice->SerialIoDeviceHandle,
            &gEfiSerialIoProtocolGuid,
            This->DriverBindingHandle,
            gDebugPortDevice->DebugPortDeviceHandle);
          
    if (EFI_ERROR (Status)) {
      return (Status);
    }
          
    //
    // Unpublish our protocols (DevicePath, DebugPort)
    //
    Status = gBS->UninstallMultipleProtocolInterfaces (
            gDebugPortDevice->DebugPortDeviceHandle,
            &gEfiDevicePathProtocolGuid,
            gDebugPortDevice->DebugPortDevicePath,
            &gEfiDebugPortProtocolGuid,
            &gDebugPortDevice->DebugPortInterface,
            NULL);
  
    if (EFI_ERROR (Status)) {
      gBS->OpenProtocol (
            ControllerHandle,  
            &gEfiSerialIoProtocolGuid, 
            &gDebugPortDevice->SerialIoBinding,
            This->DriverBindingHandle,
            gDebugPortDevice->DebugPortDeviceHandle,
            EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER);
    } else {
      gDebugPortDevice->DebugPortDeviceHandle = NULL;
    }
  }
  
  return (Status);
}


//
// Debugport protocol member functions
//
EFI_STATUS
DebugPortReset (
  IN EFI_DEBUGPORT_PROTOCOL   *This
  )
/*++

Routine Description:
  DebugPort protocol member function.  Calls SerialIo:GetControl to flush buffer.
  We cannot call SerialIo:SetAttributes because it uses pool services, which use
  locks, which affect TPL, so it's not interrupt context safe or re-entrant.
  SerialIo:Reset() calls SetAttributes, so it can't be used either.
  
  The port itself should be fine since it was set up during initialization.  
  
Arguments:
  IN EFI_DEBUGPORT_PROTOCOL   *This

Returns:

  EFI_STATUS

--*/
{
  DEBUGPORT_DEVICE            *DebugPortDevice;
  UINTN                       BufferSize, BitBucket;

  DebugPortDevice = DEBUGPORT_DEVICE_FROM_THIS (This);
  while (This->Poll (This) == EFI_SUCCESS) {
    BufferSize = 1;
    This->Read (This, 0, &BufferSize, &BitBucket);
  }
  
  return (EFI_SUCCESS);
}


EFI_STATUS
DebugPortRead (
  IN EFI_DEBUGPORT_PROTOCOL   *This,
  IN UINT32                   Timeout,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  )
/*++

Routine Description:
  DebugPort protocol member function.  Calls SerialIo:Read() after setting
  if it's different than the last SerialIo access.
  
Arguments:
  IN EFI_DEBUGPORT_PROTOCOL   *This
  IN UINT32                   Timeout,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer

Returns:

  EFI_STATUS

--*/
{
  DEBUGPORT_DEVICE            *DebugPortDevice;
  UINTN                       LocalBufferSize;
  EFI_STATUS                  Status;
  UINT8                       *BufferPtr;

  DebugPortDevice = DEBUGPORT_DEVICE_FROM_THIS (This);
  BufferPtr = Buffer;
  LocalBufferSize = *BufferSize;
  do {
    Status = DebugPortDevice->SerialIoBinding->Read (
    DebugPortDevice->SerialIoBinding,
      &LocalBufferSize,
      BufferPtr
    );
    if (Status == EFI_TIMEOUT) {
      if (Timeout > DEBUGPORT_UART_DEFAULT_TIMEOUT) {
        Timeout -= DEBUGPORT_UART_DEFAULT_TIMEOUT;
      } else {
        Timeout = 0;
      }
    } else if (EFI_ERROR (Status)) {
      break;
    }
    BufferPtr += LocalBufferSize;
    LocalBufferSize = *BufferSize - (BufferPtr - (UINT8 *) Buffer);
  } while (LocalBufferSize != 0 && Timeout > 0);
  
  *BufferSize = (UINTN) (BufferPtr - (UINT8 *) Buffer);
  
  return (Status);  
}


EFI_STATUS
DebugPortWrite (
  IN EFI_DEBUGPORT_PROTOCOL   *This,
  IN UINT32                   Timeout,
  IN OUT UINTN                *BufferSize,
  OUT VOID                    *Buffer
  )
/*++

Routine Description:
  DebugPort protocol member function.  Calls SerialIo:Write() Writes 8 bytes at
  a time and does a GetControl between 8 byte writes to help insure reads are
  interspersed This is poor-man's flow control..
  
Arguments:
  IN EFI_DEBUGPORT_PROTOCOL   *This
  IN UINT32                   Timeout,
  IN OUT UINTN                *BufferSize,
  OUT VOID                    *Buffer

Returns:
  EFI_STATUS

--*/
{
  DEBUGPORT_DEVICE            *DebugPortDevice;
  UINTN                       i, WriteSize;
  EFI_STATUS                  Status;
  UINT32                      SerialControl;
  EFI_EVENT                 TimeoutEvent;

  if(Timeout == 0){
    return EFI_TIMEOUT;
  }

  //
  //Create timeout event and timer
  //
  Status = gBS->CreateEvent(EFI_EVENT_TIMER,
                          EFI_TPL_CALLBACK,
                          NULL,
                          NULL,
                          &TimeoutEvent);
  if(EFI_ERROR(Status)){
    return EFI_OUT_OF_RESOURCES;
  }
  
  //To avoid overflow when translate ms to ns
  if(Timeout > 0xffffffff/10)
    Timeout = 0xffffffff/10;
  Timeout = Timeout * 10;
    
  Status = gBS->SetTimer(TimeoutEvent,
                          TimerRelative,
                          Timeout);
  if(EFI_ERROR(Status)){
    return EFI_OUT_OF_RESOURCES;
  }

  Status = EFI_SUCCESS;
  DebugPortDevice = DEBUGPORT_DEVICE_FROM_THIS (This);

  WriteSize = 8;
  for (i = 0; i < *BufferSize && !EFI_ERROR (Status); i += WriteSize) {
    //
    //Check timeout event. If the event is signaled, It means time out is occured 
    // and should return EFI_TIMEOUT.
    //
    if(gBS->CheckEvent(TimeoutEvent) == EFI_SUCCESS){
      gBS->CloseEvent(TimeoutEvent);
      return EFI_TIMEOUT;
    }
    
    DebugPortDevice->SerialIoBinding->GetControl (
      DebugPortDevice->SerialIoBinding,
      &SerialControl
      );
    if (*BufferSize - i < 8) {
       WriteSize = *BufferSize - i;
    }
    Status = DebugPortDevice->SerialIoBinding->Write (
    DebugPortDevice->SerialIoBinding,
      &WriteSize,
      &((UINT8 *)Buffer)[i]
    );
  }

  //
  //Close timeout event
  //
  gBS->CloseEvent(TimeoutEvent);
  
  *BufferSize = i;
  return (Status);
}


EFI_STATUS
DebugPortPoll (
  IN EFI_DEBUGPORT_PROTOCOL   *This
  )
/*++

Routine Description:
  DebugPort protocol member function.  Calls SerialIo:Write() after setting
  if it's different than the last SerialIo access.
  
Arguments:
  IN EFI_DEBUGPORT_PROTOCOL   *This

Returns:
  EFI_SUCCESS - At least 1 character is ready to be read from the DebugPort interface
  EFI_NOT_READY - There are no characters ready to read from the DebugPort interface
  EFI_DEVICE_ERROR - A hardware failure occured... (from SerialIo)

--*/
{
  EFI_STATUS                  Status;
  UINT32                      SerialControl;
  DEBUGPORT_DEVICE            *DebugPortDevice;

  DebugPortDevice = DEBUGPORT_DEVICE_FROM_THIS (This);

  Status = DebugPortDevice->SerialIoBinding->GetControl (
      DebugPortDevice->SerialIoBinding,
    &SerialControl
      );
  
  if (!EFI_ERROR(Status)) {
    if (SerialControl & EFI_SERIAL_INPUT_BUFFER_EMPTY) {
      Status = EFI_NOT_READY;
    } else {
      Status = EFI_SUCCESS;
    }
  }
  
  return (Status);
}


//
// Misc. functions local to this module..
//
static
VOID
GetDebugPortVariable (
  DEBUGPORT_DEVICE            *DebugPortDevice
  )
/*++

Routine Description:
  Local worker function to obtain device path information from DebugPort variable.
  Records requested settings in DebugPort device structure.
  
Arguments:
  DEBUGPORT_DEVICE  *DebugPortDevice,

Returns:

  Nothing

--*/
{
  UINTN                       DataSize;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  EFI_STATUS                  Status;

  DataSize = 0;

  Status = gRT->GetVariable (EFI_DEBUGPORT_VARIABLE_NAME,
                             &gEfiDebugPortVariableGuid,
                             NULL,
                             &DataSize,
                             DebugPortDevice->DebugPortVariable);

  if (Status == EFI_BUFFER_TOO_SMALL) {
    if (gDebugPortDevice->DebugPortVariable != NULL) {
      gBS->FreePool (gDebugPortDevice->DebugPortVariable);
    }
    DebugPortDevice->DebugPortVariable = EfiLibAllocatePool (DataSize);
    if (DebugPortDevice->DebugPortVariable != NULL) {
      gRT->GetVariable (EFI_DEBUGPORT_VARIABLE_NAME,
                        &gEfiDebugPortVariableGuid,
                        NULL,
                        &DataSize,
                        DebugPortDevice->DebugPortVariable);
      DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) DebugPortDevice->DebugPortVariable;
      while (!EfiIsDevicePathEnd (DevicePath) && !EfiIsUartDevicePath (DevicePath)) {
        DevicePath = EfiNextDevicePathNode (DevicePath);
      }
      if (EfiIsDevicePathEnd (DevicePath)) {
        gBS->FreePool (gDebugPortDevice->DebugPortVariable);
        DebugPortDevice->DebugPortVariable = NULL;
      } else {
        gBS->CopyMem (&DebugPortDevice->BaudRate, &((UART_DEVICE_PATH *) DevicePath)->BaudRate, sizeof (((UART_DEVICE_PATH *) DevicePath)->BaudRate));
        DebugPortDevice->ReceiveFifoDepth = 0;
        DebugPortDevice->Timeout =          0;
        gBS->CopyMem (&DebugPortDevice->Parity, &((UART_DEVICE_PATH *) DevicePath)->Parity, sizeof (((UART_DEVICE_PATH *) DevicePath)->Parity));
        gBS->CopyMem (&DebugPortDevice->DataBits, &((UART_DEVICE_PATH *) DevicePath)->DataBits, sizeof (((UART_DEVICE_PATH *) DevicePath)->DataBits));
        gBS->CopyMem (&DebugPortDevice->StopBits, &((UART_DEVICE_PATH *) DevicePath)->StopBits, sizeof (((UART_DEVICE_PATH *) DevicePath)->StopBits));
      }
    }
  }
}


static
EFI_STATUS
ImageUnloadHandler (
  EFI_HANDLE ImageHandle
  )
/*++

Routine Description:
  Unload function that is registered in the LoadImage protocol.  It un-installs
  protocols produced and deallocates pool used by the driver.  Called by the core
  when unloading the driver.
  
Arguments:
  EFI_HANDLE ImageHandle

Returns:

  Nothing

--*/
{
  EFI_STATUS  Status;

  if (gDebugPortDevice->SerialIoBinding != NULL) {
    return (EFI_ABORTED);
  }
  
  Status = gBS->UninstallMultipleProtocolInterfaces (
          ImageHandle,
          &gEfiDriverBindingProtocolGuid,
          &gDebugPortDevice->DriverBindingInterface,
          &gEfiComponentNameProtocolGuid,
          &gDebugPortDevice->ComponentNameInterface,
          NULL);
          
  if (EFI_ERROR (Status)) {
    return (Status);
  }
          
  //
  // Clean up allocations
  //
  if (gDebugPortDevice->DebugPortVariable != NULL) {
    gBS->FreePool (gDebugPortDevice->DebugPortVariable);
  }
  gBS->FreePool (gDebugPortDevice);
  
  return (EFI_SUCCESS);
}



