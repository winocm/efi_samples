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

    Terminal.c
    
Abstract: 

Revision History:

--*/

#include "Terminal.h"

//
// Function Prototypes
//
EFI_STATUS
TerminalDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
TerminalDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
TerminalDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// Globals
//

EFI_DRIVER_BINDING_PROTOCOL gTerminalDriverBinding = {
  TerminalDriverBindingSupported,
  TerminalDriverBindingStart,
  TerminalDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_GUID gTerminalDriverGuid = {
  0x10634d8e, 0x1c05, 0x46cb, 0xbb, 0xc, 0x5a, 0xfd, 0xc8, 0x29, 0xa8, 0xc8
};

//
// Body of the driver
//

EFI_DRIVER_ENTRY_POINT (InitializeTerminal)

EFI_STATUS
InitializeTerminal (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE   *SystemTable
  )
/*++
  Routine Description:
  
    This function is the entry point of Terminal driver.
  
  Arguments:
  
    EFI_HANDLE        IN    ImageHandle
         A handle for the image that is initializing this driver.
         
    EFI_SYSTEM_TABLE  IN    *SystemTable
        A pointer to the EFI system table.
        
  Returns:
  
    EFI_SUCCESS
       only returns this status code.

--*/
{
  //
  // install driver binding protocol and component name protocol.
  //
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gTerminalDriverBinding, 
           ImageHandle,
           &gTerminalComponentName,
           NULL,
           NULL
           );
}

EFI_STATUS
TerminalDriverBindingSupported (
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
  EFI_SERIAL_IO_PROTOCOL    *SerialIo;
  VENDOR_DEVICE_PATH        *Node;

  //
  // If remaining device path is not NULL, then make sure it is a 
  // device path that describes a terminal communications protocol.
  //
  if (RemainingDevicePath != NULL) {
  
    Node = (VENDOR_DEVICE_PATH *)RemainingDevicePath;
  
    if (Node->Header.Type != MESSAGING_DEVICE_PATH ||
        Node->Header.SubType != MSG_VENDOR_DP      ||
        DevicePathNodeLength(&Node->Header) != sizeof(VENDOR_DEVICE_PATH)) {
          
            return EFI_UNSUPPORTED;
          
    }
  
    //
    // only supports PC ANSI, VT100, VT100+ and VT-UTF8 terminal types
    //    
    if (!EfiCompareGuid (&Node->Guid, &gEfiPcAnsiGuid) &&
        !EfiCompareGuid (&Node->Guid, &gEfiVT100Guid) &&
        !EfiCompareGuid (&Node->Guid, &gEfiVT100PlusGuid) &&
        !EfiCompareGuid (&Node->Guid, &gEfiVTUTF8Guid)) {
  
        return EFI_UNSUPPORTED;
    }
  }

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

  //
  // The Controller must support the Serial I/O Protocol.
  // This driver is a bus driver with at most 1 child device, so it is 
  // ok for it to be already started.
  //
  Status = gBS->OpenProtocol (
                  Controller,       
                  &gEfiSerialIoProtocolGuid,  
                  (VOID **)&SerialIo,
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
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         Controller,       
         &gEfiSerialIoProtocolGuid,  
         This->DriverBindingHandle,   
         Controller   
         );

  return Status;
}

EFI_STATUS
TerminalDriverBindingStart (
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
  EFI_SERIAL_IO_PROTOCOL               *SerialIo;
  EFI_DEVICE_PATH_PROTOCOL             *ParentDevicePath;  
  VENDOR_DEVICE_PATH                   *Node;
  EFI_SERIAL_IO_MODE                   *Mode;  
  UINTN                                SerialInTimeOut;  
  TERMINAL_DEV                         *TerminalDevice = NULL;
  UINT8                                TerminalType;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY  *OpenInfoBuffer;
  UINTN                                EntryCount;
  UINTN                                Index;

  //
  // Get the Device Path Protocol to build the device path of the child device
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
  // Open the Serial I/O Protocol BY_DRIVER.  It might already be started.
  //
  Status = gBS->OpenProtocol (
                  Controller,     
                  &gEfiSerialIoProtocolGuid,  
                  (VOID **)&SerialIo,
                  This->DriverBindingHandle, 
                  Controller,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    return Status;
  }

  if (Status != EFI_ALREADY_STARTED) {
    //
    // If Serial I/O is not already open by this driver, then tag the handle
    // with the Terminal Driver GUID and update the ConInDev, ConOutDev, and
    // StdErrDev variables with the list of possible terminal types on this
    // serial port.
    //
    Status = gBS->OpenProtocol (
                    Controller,     
                    &gTerminalDriverGuid,
                    NULL,
                    This->DriverBindingHandle, 
                    Controller,   
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &Controller,
                      &gTerminalDriverGuid, 
                      EfiDuplicateDevicePath (ParentDevicePath),
                      NULL
                      );
      if (EFI_ERROR (Status)) {
        goto Error;
      }
      
      //
      // if the serial device is a hot plug device, do not update the 
      // ConInDev, ConOutDev, and StdErrDev variables.
      //
      Status = gBS->OpenProtocol (
                    Controller,     
                    &gEfiHotPlugDeviceGuid,
                    NULL,
                    This->DriverBindingHandle, 
                    Controller,   
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );
      if (EFI_ERROR(Status)) {
        TerminalUpdateConsoleDevVariable (VarConsoleInpDev, ParentDevicePath);
        TerminalUpdateConsoleDevVariable (VarConsoleOutDev, ParentDevicePath);
        TerminalUpdateConsoleDevVariable (VarErrorOutDev,   ParentDevicePath);
      }
    }
  }

  //
  // If RemainingDevicePath is NULL, then there is no enough information
  // to create a child device, so return
  //
  if (RemainingDevicePath == NULL) {
    return EFI_SUCCESS;
  }

  //
  // Make sure a child handle does not already exist.  This driver can only 
  // produce one child per serial port.
  //
  Status = gBS->OpenProtocolInformation (
                  Controller,
                  &gEfiSerialIoProtocolGuid,  
                  &OpenInfoBuffer,
                  &EntryCount
                  );
  if (!EFI_ERROR (Status)) {
    Status = EFI_SUCCESS;
    for (Index = 0; Index < EntryCount; Index++) {
      if (OpenInfoBuffer[Index].Attributes 
            & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) {
        Status = EFI_ALREADY_STARTED;
      }
    }
    gBS->FreePool (OpenInfoBuffer);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Use the RemainingDevicePath to determine the terminal type
  //
  Node = (VENDOR_DEVICE_PATH *)RemainingDevicePath;   

  if (EfiCompareGuid (&Node->Guid, &gEfiPcAnsiGuid)) {

    TerminalType = PcAnsiType;

  } else if (EfiCompareGuid (&Node->Guid, &gEfiVT100Guid)) {

    TerminalType = VT100Type;

  } else if (EfiCompareGuid (&Node->Guid, &gEfiVT100PlusGuid)) {

    TerminalType = VT100PlusType;

  } else if (EfiCompareGuid (&Node->Guid, &gEfiVTUTF8Guid)) {

    TerminalType = VTUTF8Type;

  } else {
    goto Error;
  }

  //
  // Initialize the Terminal Dev
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (TERMINAL_DEV),
                  (VOID **)&TerminalDevice
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  EfiZeroMem(TerminalDevice, sizeof (TERMINAL_DEV)); 

  TerminalDevice->Signature = TERMINAL_DEV_SIGNATURE;  

  TerminalDevice->TerminalType = TerminalType;

  TerminalDevice->SerialIo                    = SerialIo;

  //
  // Simple Input Protocol
  //
  TerminalDevice->SimpleInput.Reset            = TerminalConInReset;
  TerminalDevice->SimpleInput.ReadKeyStroke    = TerminalConInReadKeyStroke;

  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_WAIT,
                  EFI_TPL_NOTIFY,
                  TerminalConInWaitForKey,
                  &TerminalDevice->SimpleInput,
                  &TerminalDevice->SimpleInput.WaitForKey
                  ); 
  if (EFI_ERROR (Status)) {   
    goto Error;
  }
                   
  //
  // initialize the FIFO buffer used for accommodating 
  // the pre-read pending characters
  //
  InitializeRawFiFo (TerminalDevice);
  InitializeUnicodeFiFo (TerminalDevice);
  InitializeEfiKeyFiFo (TerminalDevice);

  //
  // Set the timeout value of serial buffer for 
  // keystroke response performance issue
  //
  Mode = TerminalDevice->SerialIo->Mode;

  SerialInTimeOut = 0;
  if (Mode->BaudRate != 0) {
    SerialInTimeOut = (1 + Mode->DataBits + Mode->StopBits) 
                        * 2 * 1000000 / (UINTN) Mode->BaudRate;
  }

  Status = TerminalDevice->SerialIo->SetAttributes (
                            TerminalDevice->SerialIo,
                            Mode->BaudRate,Mode->ReceiveFifoDepth,
                            (UINT32)SerialInTimeOut,
                            Mode->Parity,
                            (UINT8)Mode->DataBits,
                            Mode->StopBits
                            );
  if(EFI_ERROR(Status)) {
    //
    // if set attributes operation fails, invalidate 
    // the value of SerialInTimeOut,thus make it 
    // inconsistent with the default timeout value 
    // of serial buffer. This will invoke the recalculation 
    // in the readkeystroke routine.
    //
    TerminalDevice->SerialInTimeOut = 0;
  } else {    
    TerminalDevice->SerialInTimeOut = SerialInTimeOut;
  }

  Status = TerminalDevice->SimpleInput.Reset (
                                    &TerminalDevice->SimpleInput, 
                                    FALSE
                                    );
  if (EFI_ERROR (Status)) {   
    goto Error;
  }

  //
  // Simple Text Output Protocol
  //
  TerminalDevice->SimpleTextOutput.Reset             = TerminalConOutReset;
  TerminalDevice->SimpleTextOutput.OutputString      = TerminalConOutOutputString;
  TerminalDevice->SimpleTextOutput.TestString        = TerminalConOutTestString;
  TerminalDevice->SimpleTextOutput.QueryMode         = TerminalConOutQueryMode;
  TerminalDevice->SimpleTextOutput.SetMode           = TerminalConOutSetMode;
  TerminalDevice->SimpleTextOutput.SetAttribute      = TerminalConOutSetAttribute;
  TerminalDevice->SimpleTextOutput.ClearScreen       = TerminalConOutClearScreen;
  TerminalDevice->SimpleTextOutput.SetCursorPosition = TerminalConOutSetCursorPosition; 
  TerminalDevice->SimpleTextOutput.EnableCursor      = TerminalConOutEnableCursor;
  TerminalDevice->SimpleTextOutput.Mode              = &TerminalDevice->SimpleTextOutputMode;

  TerminalDevice->SimpleTextOutputMode.MaxMode       = 1;
  //
  // For terminal devices, cursor is always visible
  //
  TerminalDevice->SimpleTextOutputMode.CursorVisible = TRUE;
  TerminalDevice->SimpleTextOutputMode.Attribute     = EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK); 

  Status = TerminalDevice->SimpleTextOutput.Reset (
                                    &TerminalDevice->SimpleTextOutput,
                                    FALSE
                                     );
  if (EFI_ERROR (Status)) {   
    goto Error;
  }
  Status = TerminalDevice->SimpleTextOutput.SetMode (
                                    &TerminalDevice->SimpleTextOutput,
                                    0
                                    );
  if (EFI_ERROR (Status)) {   
    goto Error;
  }
  Status = TerminalDevice->SimpleTextOutput.EnableCursor (
                                    &TerminalDevice->SimpleTextOutput,
                                    TRUE
                                    );
  if (EFI_ERROR (Status)) {   
    goto Error;
  }

  //
  //
  //
  TerminalDevice->InputState = INPUT_STATE_DEFAULT;
  TerminalDevice->ResetState = RESET_STATE_DEFAULT;

  Status = gBS->CreateEvent (
                  EFI_EVENT_TIMER,
                  EFI_TPL_CALLBACK,
                  NULL,
                  NULL,
                  &TerminalDevice->TwoSecondTimeOut
                  );

  //
  // Build the device path for the child device
  //
  Status = SetTerminalDevicePath (
                          TerminalDevice->TerminalType,
                          ParentDevicePath,
                          &TerminalDevice->DevicePath
                          );
  if (EFI_ERROR(Status)) {
    goto Error;
  }

  //
  // Build the component name for the child device
  //
  TerminalDevice->ControllerNameTable = NULL;
  switch (TerminalDevice->TerminalType) {
  case PcAnsiType:
    EfiLibAddUnicodeString (
      "eng", 
      gTerminalComponentName.SupportedLanguages, 
      &TerminalDevice->ControllerNameTable, 
      L"PC-ANSI Serial Console"
      );
    break;
  case VT100Type:
    EfiLibAddUnicodeString (
      "eng", 
      gTerminalComponentName.SupportedLanguages, 
      &TerminalDevice->ControllerNameTable, 
      L"VT-100 Serial Console"
      );
    break;
  case VT100PlusType:
    EfiLibAddUnicodeString (
      "eng", 
      gTerminalComponentName.SupportedLanguages, 
      &TerminalDevice->ControllerNameTable, 
      L"VT-100+ Serial Console"
      );
    break;
  case VTUTF8Type:
    EfiLibAddUnicodeString (
      "eng", 
      gTerminalComponentName.SupportedLanguages, 
      &TerminalDevice->ControllerNameTable, 
      L"VT-UTF8 Serial Console"
      );
    break;
  }

  //
  // Install protocol interfaces for the serial device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                 &TerminalDevice->Handle,            
                 &gEfiDevicePathProtocolGuid,    TerminalDevice->DevicePath,
                 &gEfiSimpleTextInProtocolGuid,  &TerminalDevice->SimpleInput,
                 &gEfiSimpleTextOutProtocolGuid, 
                 &TerminalDevice->SimpleTextOutput,
                 NULL
                 );
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  
  //
  // if the serial device is a hot plug device, attaches the HotPlugGuid
  // onto the terminal device handle.
  //
  Status = gBS->OpenProtocol (
                    Controller,     
                    &gEfiHotPlugDeviceGuid,
                    NULL,
                    This->DriverBindingHandle, 
                    Controller,   
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );
  if (!EFI_ERROR(Status)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                 &TerminalDevice->Handle,
                 &gEfiHotPlugDeviceGuid, NULL,
                 NULL
                 );
  }

  //
  // Register the Parent-Child relationship via 
  // EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSerialIoProtocolGuid,  
                  (VOID **)&TerminalDevice->SerialIo,
                  This->DriverBindingHandle, 
                  TerminalDevice->Handle, 
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
  if (TerminalDevice != NULL) {
    
    if (TerminalDevice->Handle != NULL) {
      This->Stop (This, Controller, 1, &TerminalDevice->Handle);
    } else {

      if (TerminalDevice->TwoSecondTimeOut != NULL) {
        gBS->CloseEvent (TerminalDevice->TwoSecondTimeOut);
      }

      if (TerminalDevice->SimpleInput.WaitForKey != NULL) {
        gBS->CloseEvent (TerminalDevice->SimpleInput.WaitForKey);
      }
      
      if (TerminalDevice->ControllerNameTable) {
        EfiLibFreeUnicodeStringTable (TerminalDevice->ControllerNameTable);
      }
      
      gBS->FreePool (TerminalDevice);
    }
  }
  
  This->Stop (This, Controller, 0, NULL);           
  return Status;
}

EFI_STATUS
TerminalDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Controller,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
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
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *SimpleTextOutput;
  TERMINAL_DEV                  *TerminalDevice;
  EFI_DEVICE_PATH_PROTOCOL      *ParentDevicePath;  
  EFI_SERIAL_IO_PROTOCOL        *SerialIo;

  //
  // Complete all outstanding transactions to Controller.
  // Don't allow any new transaction to Controller to be started.
  //

  if (NumberOfChildren == 0) {
    //
    // Close the bus driver
    //
    Status = gBS->OpenProtocol (
                    Controller,     
                    &gTerminalDriverGuid,  
                    (VOID **)&ParentDevicePath,
                    This->DriverBindingHandle, 
                    Controller,   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {
      //
      // Remove Parent Device Path from 
      // the Console Device Environment Variables
      //
      TerminalRemoveConsoleDevVariable (VarConsoleInpDev, ParentDevicePath);
      TerminalRemoveConsoleDevVariable (VarConsoleOutDev, ParentDevicePath);
      TerminalRemoveConsoleDevVariable (VarErrorOutDev,   ParentDevicePath);

      //
      // Uninstall the Terminal Driver's GUID Tag from the Serial controller
      //
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      Controller,
                      &gTerminalDriverGuid, ParentDevicePath,
                      NULL
                      );

      //
      // Free the ParentDevicePath that was duplicated in Start()
      //
      if (!EFI_ERROR (Status)) {
        gBS->FreePool (ParentDevicePath);
      }
    }

    gBS->CloseProtocol (
           Controller, 
           &gEfiSerialIoProtocolGuid, 
           This->DriverBindingHandle, 
           Controller
           );

    gBS->CloseProtocol (
           Controller,   
           &gEfiDevicePathProtocolGuid,  
           This->DriverBindingHandle,   
           Controller   
           );

    return EFI_SUCCESS;
  }

  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {

    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index], 
                    &gEfiSimpleTextOutProtocolGuid, 
                    (VOID **)&SimpleTextOutput, 
                    This->DriverBindingHandle, 
                    ChildHandleBuffer[Index],   
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR(Status)) {

      TerminalDevice = TERMINAL_CON_OUT_DEV_FROM_THIS(SimpleTextOutput);

      gBS->CloseProtocol (
             Controller, 
             &gEfiSerialIoProtocolGuid, 
             This->DriverBindingHandle, 
             ChildHandleBuffer[Index]
             );

      Status = gBS->UninstallMultipleProtocolInterfaces (
                     ChildHandleBuffer[Index], 
                     &gEfiSimpleTextInProtocolGuid,
                     &TerminalDevice->SimpleInput,
                     &gEfiSimpleTextOutProtocolGuid,
                     &TerminalDevice->SimpleTextOutput,
                     &gEfiDevicePathProtocolGuid,
                     TerminalDevice->DevicePath,
                     NULL
                     );
      if (EFI_ERROR (Status)) {
        gBS->OpenProtocol (
               Controller,
               &gEfiSerialIoProtocolGuid,  
               (VOID **)&SerialIo,
               This->DriverBindingHandle, 
               ChildHandleBuffer[Index],
               EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
               );
      } else {
        
        if (TerminalDevice->ControllerNameTable) {
          EfiLibFreeUnicodeStringTable (TerminalDevice->ControllerNameTable);
        }
        
        Status = gBS->OpenProtocol (
                ChildHandleBuffer[Index], 
                &gEfiHotPlugDeviceGuid, 
                NULL,
                This->DriverBindingHandle,
                Controller, 
                EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                );
        if (!EFI_ERROR (Status)) {
          Status = gBS->UninstallMultipleProtocolInterfaces (
                          ChildHandleBuffer[Index],
                          &gEfiHotPlugDeviceGuid, NULL,
                          NULL
                          );
        } else {
          Status = EFI_SUCCESS;
        }
        gBS->CloseEvent (TerminalDevice->TwoSecondTimeOut);
        gBS->CloseEvent (TerminalDevice->SimpleInput.WaitForKey);
        gBS->FreePool (TerminalDevice);
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

VOID
TerminalUpdateConsoleDevVariable (
  IN CHAR16                    *VariableName,
  IN EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath
  )

{
  EFI_STATUS                Status;
  UINTN                     VariableSize;
  UINT8                     TerminalType;
  EFI_DEVICE_PATH_PROTOCOL  *Variable;
  EFI_DEVICE_PATH_PROTOCOL  *NewVariable;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  
  Variable = NULL;

  //
  // Get global variable and its size according to the name given.
  //
  Variable = TerminalGetVariableAndSize (
               VariableName,
               &gEfiGlobalVariableGuid,
               &VariableSize  
               );
  //
  // Append terminal device path onto the variable.
  //               
  for (TerminalType = PcAnsiType; TerminalType <= VTUTF8Type; TerminalType++) {
    SetTerminalDevicePath (TerminalType, ParentDevicePath, &TempDevicePath);
    NewVariable = EfiAppendDevicePathInstance (Variable, TempDevicePath);
    if (Variable != NULL) {
      gBS->FreePool (Variable);
    }
    if (TempDevicePath != NULL) {
      gBS->FreePool (TempDevicePath);
    }
    Variable = NewVariable;
  }

  VariableSize = EfiDevicePathSize (Variable);
  
  Status = gRT->SetVariable (
                  VariableName,
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  VariableSize,
                  Variable
                  );
}

VOID
TerminalRemoveConsoleDevVariable (
  IN CHAR16                    *VariableName,
  IN EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath
  )

{
  EFI_STATUS                Status;
  BOOLEAN                   FoundOne;
  BOOLEAN                   Match;
  UINTN                     VariableSize;
  UINTN                     InstanceSize;
  UINT8                     TerminalType;
  EFI_DEVICE_PATH_PROTOCOL  *Instance;
  EFI_DEVICE_PATH_PROTOCOL  *Variable;
  EFI_DEVICE_PATH_PROTOCOL  *OriginalVariable;
  EFI_DEVICE_PATH_PROTOCOL  *NewVariable;
  EFI_DEVICE_PATH_PROTOCOL  *SavedNewVariable;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  
  Variable = NULL;
  Instance = NULL;
  
  //
  // Get global variable and its size according to the name given.
  //
  Variable = TerminalGetVariableAndSize (
               VariableName,
               &gEfiGlobalVariableGuid,
               &VariableSize  
               );
  if (Variable == NULL) {
    return;
  }

  FoundOne         = FALSE;
  OriginalVariable = Variable;
  NewVariable      = NULL;

  //
  // Get first device path instance from Variable
  //
  Instance = EfiDevicePathInstance (&Variable, &InstanceSize);
  if (Instance == NULL) {
    gBS->FreePool (OriginalVariable);
    return;
  }
  //
  // Loop through all the device path instances of Variable
  //
  do {

    //
    // Loop through all the terminal types that this driver supports
    //               
    Match = FALSE;
    for (TerminalType = PcAnsiType; TerminalType <= VTUTF8Type; 
          TerminalType++) {
            
      SetTerminalDevicePath (TerminalType, ParentDevicePath, &TempDevicePath);

      //
      // Compare the genterated device path to the current device path instance
      //
      if (TempDevicePath != NULL) {
        if (EfiCompareMem (Instance, TempDevicePath, InstanceSize) == 0) {
          Match = TRUE;
          FoundOne = TRUE;
        }
        gBS->FreePool (TempDevicePath);
      }
    }

    //
    // If a match was not found, then keep the current device path instance
    //
    if (!Match) {
      SavedNewVariable = NewVariable;
      NewVariable = EfiAppendDevicePathInstance (NewVariable, Instance);
      if (SavedNewVariable != NULL) {
        gBS->FreePool (SavedNewVariable);
      }
    }

    //
    // Get next device path instance from Variable
    //
    gBS->FreePool(Instance);
    Instance = EfiDevicePathInstance (&Variable, &InstanceSize);
  } while (Instance != NULL);

  gBS->FreePool (OriginalVariable);

  if (FoundOne) {
    VariableSize = EfiDevicePathSize (NewVariable);
  
    Status = gRT->SetVariable (
                VariableName,
                &gEfiGlobalVariableGuid,
                EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                VariableSize,
                NewVariable
                );
  }
}

VOID *
TerminalGetVariableAndSize (
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize
  )
/*++

Routine Description:
  Read the EFI variable (VendorGuid/Name) and return a dynamically allocated
  buffer, and the size of the buffer. On failure return NULL.

Arguments:
  Name       - String part of EFI variable name

  VendorGuid - GUID part of EFI variable name

  VariableSize - Returns the size of the EFI variable that was read

Returns:
  Dynamically allocated memory that contains a copy of the EFI variable.
  Caller is repsoncible freeing the buffer.

  NULL - Variable was not read
  
--*/
{
  EFI_STATUS    Status;
  UINTN         BufferSize;
  VOID          *Buffer;

  Buffer = NULL;

  //
  // Pass in a small size buffer to find the actual variable size.
  //
  BufferSize = 1;
  Status = gBS->AllocatePool (EfiBootServicesData, BufferSize, &Buffer); 
  if (EFI_ERROR(Status)) {
    *VariableSize = 0;
    return NULL;
  }
  
  Status = gRT->GetVariable (Name, VendorGuid, NULL, &BufferSize, Buffer);
  
  if (Status == EFI_SUCCESS) {
    *VariableSize = BufferSize;
    return Buffer;
  
  } else if (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Allocate the buffer to return
    //
    gBS->FreePool (Buffer);
    Status = gBS->AllocatePool (EfiBootServicesData, BufferSize, &Buffer); 
    if (EFI_ERROR (Status)) {
      *VariableSize = 0;
      return NULL;
    }

    //
    // Read variable into the allocated buffer.
    //
    Status = gRT->GetVariable (Name, VendorGuid, NULL, &BufferSize, Buffer);
    if (EFI_ERROR (Status)) {
      BufferSize = 0;
      gBS->FreePool (Buffer);
      Buffer = NULL;
    }
  } else {
    //
    // Variable not found or other errors met.
    //
    BufferSize = 0 ;
    gBS->FreePool (Buffer);
    Buffer = NULL;
  }  

  *VariableSize = BufferSize;
  return Buffer;
}


EFI_STATUS
SetTerminalDevicePath (
  IN  UINT8                       TerminalType,
  IN  EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath,
  OUT EFI_DEVICE_PATH_PROTOCOL    **TerminalDevicePath
  )
{ 
  VENDOR_DEVICE_PATH  Node;
  
  *TerminalDevicePath = NULL;
  Node.Header.Type    = MESSAGING_DEVICE_PATH;
  Node.Header.SubType = MSG_VENDOR_DP;
  
  //
  // generate terminal device path node according to terminal type.
  //  
  switch (TerminalType) {
    
    case PcAnsiType:
      EfiCopyMem (
            &Node.Guid, 
            &gEfiPcAnsiGuid, 
            sizeof(EFI_GUID)
            );
      break;
    
    case VT100Type:
      EfiCopyMem (
            &Node.Guid, 
            &gEfiVT100Guid, 
            sizeof(EFI_GUID)
            );
      break;
      
    case VT100PlusType:
      EfiCopyMem (
            &Node.Guid, 
            &gEfiVT100PlusGuid, 
            sizeof(EFI_GUID)
            );
      break;
      
    case VTUTF8Type:
      EfiCopyMem (
            &Node.Guid, 
            &gEfiVTUTF8Guid, 
            sizeof(EFI_GUID)
            );
      break;
    
    default:
      return EFI_UNSUPPORTED;
      break;
  }
  
  SetDevicePathNodeLength (
            &Node.Header, 
            sizeof(VENDOR_DEVICE_PATH)
            );
  //
  // append the terminal node onto parent device path
  // to generate a complete terminal device path.
  //
  *TerminalDevicePath = EfiAppendDevicePathNode (
                          ParentDevicePath, 
                          (EFI_DEVICE_PATH_PROTOCOL *)&Node
                          );
  if (*TerminalDevicePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  return EFI_SUCCESS;
                               
}                               

VOID
InitializeRawFiFo (
  IN  TERMINAL_DEV  *TerminalDevice
  )
{
  //
  // Make the raw fifo empty.
  //
  TerminalDevice->RawFiFo.Head = TerminalDevice->RawFiFo.Tail;
}

VOID
InitializeUnicodeFiFo (  
  IN  TERMINAL_DEV  *TerminalDevice
  )
{
  //
  // Make the unicode fifo empty
  //
  TerminalDevice->UnicodeFiFo.Head = TerminalDevice->UnicodeFiFo.Tail;
}  

VOID 
InitializeEfiKeyFiFo (
  IN  TERMINAL_DEV  *TerminalDevice
  )
{
  //
  // Make the efi key fifo empty
  //
  TerminalDevice->EfiKeyFiFo.Head = TerminalDevice->EfiKeyFiFo.Tail;  
}  
