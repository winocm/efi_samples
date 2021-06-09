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

  Ps2Keyboard.c

Abstract:

  PS/2 Keyboard driver. Routines that interacts with callers,
  conforming to EFI driver model

Revision History:

--*/

#include "Ps2Keyboard.h"

//
// Function prototypes
//
EFI_STATUS
EFIAPI
KbdControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
KbdControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
KbdControllerDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// Global variables
//

//
// DriverBinding Protocol Instance
//
EFI_DRIVER_BINDING_PROTOCOL gKeyboardControllerDriver = {
  KbdControllerDriverSupported,
  KbdControllerDriverStart,
  KbdControllerDriverStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT(InstallPs2KeyboardDriver)

EFI_STATUS
EFIAPI
InstallPs2KeyboardDriver (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Driver entry point. Installs DriverBinding protocol.

Arguments:

  ImageHandle
  SystemTable

Returns:

  EFI_STATUS

--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gKeyboardControllerDriver, 
           ImageHandle,
           &gPs2KeyboardComponentName,
           NULL,
           NULL
           );
}


EFI_STATUS
EFIAPI
KbdControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )

/*++

Routine Description:

  ControllerDriver Protocol Method

Arguments:

Returns:

--*/

{
  EFI_STATUS             Status;
  EFI_ISA_IO_PROTOCOL    *IsaIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiIsaIoProtocolGuid,
                  (VOID **)&IsaIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Use the ISA I/O Protocol to see if Controller is the Keyboard controller
  //
  if (IsaIo->ResourceList->Device.HID != EISA_PNP_ID(0x303) || IsaIo->ResourceList->Device.UID != 0) {
    Status = EFI_UNSUPPORTED;
  }

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
EFIAPI
KbdControllerDriverStart (
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
  EFI_STATUS                 Status;
  EFI_STATUS                 Status1;
  EFI_ISA_IO_PROTOCOL        *IsaIo;
  KEYBOARD_CONSOLE_IN_DEV    *ConsoleIn;
  UINT8                      Data;

  
  //
  // Get the ISA I/O Protocol on Controller's handle
  //
  Status = gBS->OpenProtocol(
                  Controller,
                  &gEfiIsaIoProtocolGuid,
                  (VOID **)&IsaIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR(Status)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Allocate private data
  //
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
                  sizeof(KEYBOARD_CONSOLE_IN_DEV),
                  (VOID **)&ConsoleIn
                  );

  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  EfiZeroMem(ConsoleIn, sizeof(KEYBOARD_CONSOLE_IN_DEV));

  //
  // Setup the device instance
  //

  ConsoleIn->Signature = KEYBOARD_CONSOLE_IN_DEV_SIGNATURE;

  ConsoleIn->Handle = Controller;
  (ConsoleIn->ConIn).Reset = KeyboardEfiReset;
  (ConsoleIn->ConIn).ReadKeyStroke = KeyboardReadKeyStroke;

  ConsoleIn->DataRegisterAddress    = KEYBOARD_8042_DATA_REGISTER;
  ConsoleIn->StatusRegisterAddress  = KEYBOARD_8042_STATUS_REGISTER;
  ConsoleIn->CommandRegisterAddress = KEYBOARD_8042_COMMAND_REGISTER;

  ConsoleIn->IsaIo = IsaIo;
  
  ConsoleIn->ScancodeBufStartPos = 0;
  ConsoleIn->ScancodeBufEndPos = KEYBOARD_BUFFER_MAX_COUNT - 1;
  ConsoleIn->ScancodeBufCount = 0;
  ConsoleIn->Ctrled = FALSE;
  ConsoleIn->Alted = FALSE;

  //
  // Setup the WaitForKey event
  //
  Status = gBS->CreateEvent (
        EFI_EVENT_NOTIFY_WAIT,
        EFI_TPL_NOTIFY,
        KeyboardWaitForKey,
        &(ConsoleIn->ConIn),
        &((ConsoleIn->ConIn).WaitForKey)
        );

  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  //
  // Setup a periodic timer, used for reading keystrokes
  // at a fixed interval
  //
  Status = gBS -> CreateEvent(
      EFI_EVENT_TIMER|EFI_EVENT_NOTIFY_SIGNAL,
      EFI_TPL_NOTIFY,
      KeyboardTimerHandler,
      ConsoleIn,
      &ConsoleIn->TimerEvent
      );

  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  Status = gBS -> SetTimer(
          ConsoleIn->TimerEvent,
          TimerPeriodic,
          KEYBOARD_TIMER_INTERVAL
          );

  if (EFI_ERROR(Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

 
  //
  // Reset the keyboard device
  //
  Status = ConsoleIn->ConIn.Reset (&ConsoleIn->ConIn, TRUE);
  if (EFI_ERROR(Status)) {
    Status = EFI_DEVICE_ERROR;
    goto ErrorExit;
  }

  ConsoleIn->ControllerNameTable = NULL;
  EfiLibAddUnicodeString (
    "eng", 
    gPs2KeyboardComponentName.SupportedLanguages, 
    &ConsoleIn->ControllerNameTable, 
    L"PS/2 Keyboard Device"
    );

  //
  // Install protocol interfaces for the keyboard device.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
        &Controller,
        &gEfiSimpleTextInProtocolGuid,        &ConsoleIn->ConIn,
        NULL
        );
  if (EFI_ERROR(Status)) {
     goto ErrorExit;
  }
 
  return Status;

ErrorExit:
 
  if ( ConsoleIn && ConsoleIn->ConIn.WaitForKey ) {
    gBS->CloseEvent(ConsoleIn->ConIn.WaitForKey);
  }
  
  if ( ConsoleIn && ConsoleIn->TimerEvent ) {
    gBS->CloseEvent(ConsoleIn->TimerEvent);
  }

  if ( ConsoleIn && ConsoleIn->ControllerNameTable ) {
    EfiLibFreeUnicodeStringTable(ConsoleIn->ControllerNameTable);
  }

  //
  // Since there will be no timer handler for keyboard input any more,
  // exhaust input data just in case there is still keyboard data left
  //

  Status1 = EFI_SUCCESS;
  while (!EFI_ERROR (Status1)) {
    Status1 = KeyboardRead(ConsoleIn, &Data);;
  }

  if ( ConsoleIn ) {
    gBS ->FreePool(ConsoleIn);
  }
  
  gBS->CloseProtocol (
              Controller,
              &gEfiIsaIoProtocolGuid,
              This->DriverBindingHandle,
              Controller
              );

  return Status;
}


EFI_STATUS
EFIAPI
KbdControllerDriverStop (
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
  EFI_SIMPLE_TEXT_IN_PROTOCOL   *ConIn;
  KEYBOARD_CONSOLE_IN_DEV       *ConsoleIn;
  UINT8                         Data;

  //
  // Disable Keyboard
  //

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  (VOID **)&ConIn,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ConsoleIn = KEYBOARD_CONSOLE_IN_DEV_FROM_THIS (ConIn);

  if (ConsoleIn->TimerEvent) {  
    gBS->CloseEvent(ConsoleIn->TimerEvent);
    ConsoleIn->TimerEvent = NULL;
  }

  //
  // Disable the keyboard interface
  //
  Status = DisableKeyboard (ConsoleIn);

  //
  // Since there will be no timer handler for keyboard input any more,
  // exhaust input data just in case there is still keyboard data left
  //

  Status = EFI_SUCCESS;
  while (!EFI_ERROR (Status)) {
    Status = KeyboardRead(ConsoleIn, &Data);;
  }

  //
  // Uninstall the Simple TextIn Protocol
  //
  
  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,    
                  &ConsoleIn->ConIn
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Release the IsaIo protocol on the controller handle
  //

  gBS->CloseProtocol (
         Controller,
         &gEfiIsaIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  //
  // Free other resources
  //
  
  if ((ConsoleIn->ConIn).WaitForKey) {
    gBS->CloseEvent((ConsoleIn->ConIn).WaitForKey);
    (ConsoleIn->ConIn).WaitForKey = NULL;
  }
  
  EfiLibFreeUnicodeStringTable (ConsoleIn->ControllerNameTable);
  gBS->FreePool(ConsoleIn);

  return EFI_SUCCESS;
}
