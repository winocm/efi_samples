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

  Ps2KbdTextIn.c
  
Abstract:

  PS/2 Keyboard  driver
  Routines that support SIMPLE_TEXT_IN protocol

Revision History

--*/

#include "Ps2Keyboard.h"

//
// function declarations
//
EFI_STATUS
EFIAPI 
KeyboardEfiReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This, 
  IN  BOOLEAN                 ExtendedVerification
  );

EFI_STATUS
EFIAPI
KeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This, 
  OUT EFI_INPUT_KEY           *Key
  );

VOID
EFIAPI
KeyboardWaitForKey (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );
  
EFI_STATUS
KeyboardCheckForKey (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL *This 
  );


EFI_STATUS 
EFIAPI
KeyboardEfiReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This, 
  IN  BOOLEAN                 ExtendedVerification
  )
/*++

Routine Description:

  Implement SIMPLE_TEXT_IN.Reset()
  Perform 8042 controller and keyboard initialization

Arguments:

Returns:

--*/
{
  EFI_STATUS              Status;
  KEYBOARD_CONSOLE_IN_DEV *ConsoleIn;
  EFI_TPL                 OldTpl;
  

  ConsoleIn = KEYBOARD_CONSOLE_IN_DEV_FROM_THIS (This);
  if (ConsoleIn->KeyboardErr) {
    return EFI_DEVICE_ERROR;
  }
  
  //
  // Enter critical section
  //
  OldTpl = gBS -> RaiseTPL(EFI_TPL_NOTIFY);

  //
  // Call InitKeyboard to initialize the keyboard
  //
  
  Status = InitKeyboard (ConsoleIn, ExtendedVerification);
  if (EFI_ERROR(Status)) {
    //
    // Leave critical section and return
    //
    gBS -> RestoreTPL(OldTpl);
    return EFI_DEVICE_ERROR;
  }

  //
  // Clear the status of ConsoleIn.Key
  //
  ConsoleIn->Key.ScanCode           = SCAN_NULL;
  ConsoleIn->Key.UnicodeChar        = 0x0000;  
  
  //
  // Leave critical section and return
  //
  gBS -> RestoreTPL(OldTpl);
  
  return EFI_SUCCESS;
}


EFI_STATUS 
EFIAPI 
KeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This, 
  OUT EFI_INPUT_KEY           *Key
  )
/*++

Routine Description:

  Implement SIMPLE_TEXT_IN.ReadKeyStroke().
  Retrieve key values for driver user.

Arguments:

Returns:

--*/
{
  EFI_STATUS              Status;
  KEYBOARD_CONSOLE_IN_DEV *ConsoleIn;
  EFI_TPL                 OldTpl;

  ConsoleIn = KEYBOARD_CONSOLE_IN_DEV_FROM_THIS (This);
  
  //
  // Enter critical section
  //
  OldTpl = gBS -> RaiseTPL(EFI_TPL_NOTIFY);
  
  if (ConsoleIn->KeyboardErr) {
    //
    // Leave critical section and return
    //
    gBS -> RestoreTPL(OldTpl);
    
    return EFI_DEVICE_ERROR;
  }
  
  //
  // If there's no key, just return
  //
  Status = KeyboardCheckForKey (This);
  if (EFI_ERROR(Status)) {
    //
    // Leave critical section and return
    //
    gBS -> RestoreTPL(OldTpl);
    return EFI_NOT_READY;
  }

  Key->ScanCode    = ConsoleIn->Key.ScanCode;
  Key->UnicodeChar = ConsoleIn->Key.UnicodeChar;

  ConsoleIn->Key.ScanCode    = SCAN_NULL;
  ConsoleIn->Key.UnicodeChar = 0x0000;
  
  //
  // Leave critical section and return
  //
  gBS -> RestoreTPL(OldTpl);

  return EFI_SUCCESS;
}


VOID 
EFIAPI
KeyboardWaitForKey (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  )
/*++

Routine Description:

  Event notification function for SIMPLE_TEXT_IN.WaitForKey event
  Signal the event if there is key available

Arguments:

Returns:

--*/
{
  EFI_TPL                 OldTpl;
  KEYBOARD_CONSOLE_IN_DEV *ConsoleIn;
  
  ConsoleIn = KEYBOARD_CONSOLE_IN_DEV_FROM_THIS (Context);

  //    
  // Enter critical section
  //
  OldTpl = gBS -> RaiseTPL(EFI_TPL_NOTIFY);
  
  if (ConsoleIn->KeyboardErr)
  {
    //
    // Leave critical section and return
    //
    gBS -> RestoreTPL(OldTpl);
    return;
  }    
  
  //
  // Someone is waiting on the keyboard event, if there's
  // a key pending, signal the event
  //
  if (!EFI_ERROR(KeyboardCheckForKey(Context))) {
    gBS->SignalEvent(Event);
  }
  
  //
  // Leave critical section and return
  //
  gBS -> RestoreTPL(OldTpl);
  
  return;
}


EFI_STATUS 
KeyboardCheckForKey (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL *This 
  )
{   
  KEYBOARD_CONSOLE_IN_DEV *ConsoleIn;

  ConsoleIn = KEYBOARD_CONSOLE_IN_DEV_FROM_THIS (This);

  //
  // If ready to read next key, check it
  //
  if (ConsoleIn->Key.ScanCode == SCAN_NULL && ConsoleIn->Key.UnicodeChar == 0x00) {
    return (KeyGetchar(ConsoleIn));
  }
  return EFI_SUCCESS;
}
