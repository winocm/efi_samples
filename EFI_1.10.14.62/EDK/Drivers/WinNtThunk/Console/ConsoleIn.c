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

  ConsoleIn.c

Abstract:

  Console based on Win32 APIs. 

  This file attaches a SimpleTextIn protocol to a previously open window.
  
  The constructor for this protocol depends on an open window. Currently
  the SimpleTextOut protocol creates a window when it's constructor is called.
  Thus this code must run after the constructor for the SimpleTextOut 
  protocol
  
--*/

#include "Console.h"

//
// Private worker functions
//
STATIC
EFI_STATUS
WinNtSimpleTextInCheckKey (
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA  *Private
  );

EFI_STATUS
EFIAPI
WinNtSimpleTextInReset (
  IN struct _EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN BOOLEAN                              ExtendedVerification
  )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private;

  Private = WIN_NT_SIMPLE_TEXT_IN_PRIVATE_DATA_FROM_THIS(This);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
WinNtConvertInputRecordToEfiKey(
  IN  INPUT_RECORD    *InputRecord,
  OUT EFI_INPUT_KEY   *Key
  )

{
  //
  // Make sure InputRecord is an event that represents a keypress
  //
  if (InputRecord->EventType == KEY_EVENT) {
    if (!InputRecord->Event.KeyEvent.bKeyDown) {
      return EFI_NOT_READY;
    }
  } else {
    return EFI_NOT_READY;
  }
  
  //
  // Check to see if we should return a scan code in place of Unicode character.
  //
  Key->ScanCode = 0;
  Key->UnicodeChar = 0;
  if ((InputRecord->Event.KeyEvent.dwControlKeyState & (NUMLOCK_ON | ENHANCED_KEY)) != NUMLOCK_ON) {
    //
    // Only check these scan codes if num lock is off.
    //
    switch (InputRecord->Event.KeyEvent.wVirtualScanCode) {
      case 0x48: Key->ScanCode = SCAN_UP;         break;
      case 0x50: Key->ScanCode = SCAN_DOWN;       break;
      case 0x4d: Key->ScanCode = SCAN_RIGHT;      break;
      case 0x4b: Key->ScanCode = SCAN_LEFT;       break;
      case 0x47: Key->ScanCode = SCAN_HOME;       break;
      case 0x4F: Key->ScanCode = SCAN_END;        break;
      case 0x52: Key->ScanCode = SCAN_INSERT;     break;
      case 0x53: Key->ScanCode = SCAN_DELETE;     break;
      case 0x49: Key->ScanCode = SCAN_PAGE_UP;    break;
      case 0x51: Key->ScanCode = SCAN_PAGE_DOWN;  break;
    }
  } 
  switch (InputRecord->Event.KeyEvent.wVirtualScanCode) {
    case 0x3b: Key->ScanCode = SCAN_F1;   break;
    case 0x3c: Key->ScanCode = SCAN_F2;   break;
    case 0x3d: Key->ScanCode = SCAN_F3;   break;
    case 0x3e: Key->ScanCode = SCAN_F4;   break;
    case 0x3f: Key->ScanCode = SCAN_F5;   break;
    case 0x40: Key->ScanCode = SCAN_F6;   break;
    case 0x41: Key->ScanCode = SCAN_F7;   break;
    case 0x42: Key->ScanCode = SCAN_F8;   break;
    case 0x43: Key->ScanCode = SCAN_F9;   break;
    case 0x44: Key->ScanCode = SCAN_F10;  break;
    case 0x01: Key->ScanCode = SCAN_ESC;  break;
  }

  //
  // If there's a scan code pass it, and don't pass the char code
  //
  if (Key->ScanCode == 0) {
    Key->UnicodeChar = InputRecord->Event.KeyEvent.uChar.UnicodeChar;
    if (Key->UnicodeChar == 0) {
      return EFI_NOT_READY;
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI 
WinNtSimpleTextInReadKeyStroke (
  IN struct _EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                       *Key
    )
{
  EFI_STATUS                       Status;
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA  *Private;
  INPUT_RECORD                     InputRecord;  
  DWORD                            NtEventCount;

  Private = WIN_NT_SIMPLE_TEXT_IN_PRIVATE_DATA_FROM_THIS(This);

  Status = WinNtSimpleTextInCheckKey (Private);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  do {

    if (!Private->WinNtThunk->ReadConsoleInput (Private->NtInHandle, &InputRecord, 1, &NtEventCount)) {
      return EFI_DEVICE_ERROR;
    }

    if (NtEventCount == 0) {
      return EFI_NOT_READY;
    }

    //
    // Convert the Input Record to an EFI Keystroke.
    //
    Status = WinNtConvertInputRecordToEfiKey (&InputRecord, Key);
  } while (EFI_ERROR (Status));

  return Status;
}


STATIC
VOID
EFIAPI
WinNtSimpleTextInWaitForKey (
    IN EFI_EVENT          Event,
    IN VOID               *Context
  )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private;
  EFI_STATUS                      Status;

  Private = (WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *)Context;
  Status = WinNtSimpleTextInCheckKey (Private);
  if (!EFI_ERROR(Status)) {
    gBS->SignalEvent (Event);
  }
}


STATIC
EFI_STATUS
WinNtSimpleTextInCheckKey (
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA   *Private
  )
{
  INPUT_RECORD    *InputRecord;
  DWORD           NtEventCount;
  DWORD           ActualNtEventCount;
  EFI_STATUS      Status;
  BOOLEAN         Success;
  UINTN           Index;
  EFI_INPUT_KEY   Key;

  InputRecord = NULL;
  NtEventCount = 0;
  Private->WinNtThunk->GetNumberOfConsoleInputEvents (Private->NtInHandle, &NtEventCount);
  if (NtEventCount == 0) {
    Status = EFI_NOT_READY;
    goto Done;
  }

  Status = gBS->AllocatePool (
              EfiBootServicesData,
							sizeof(INPUT_RECORD)*NtEventCount, 
              &InputRecord
              );
  if (EFI_ERROR (Status)) {
    Status = EFI_NOT_READY;
    goto Done;
  }

  Success = (BOOLEAN) Private->WinNtThunk->PeekConsoleInput (
                                             Private->NtInHandle, 
                                             InputRecord, 
                                             NtEventCount, 
                                             &ActualNtEventCount
                                             );
  if (!Success) {
    Status = EFI_NOT_READY;
    goto Done;
  }

  Status = EFI_NOT_READY;
  for (Index =0; Index < (UINTN)ActualNtEventCount; Index++) {
    //
    // Convert the Input Record to an EFI Keystroke.
    //
    Status = WinNtConvertInputRecordToEfiKey(&InputRecord[Index], &Key);
    if (!EFI_ERROR (Status)) {
      Status = EFI_SUCCESS;
      goto Done;
    }
  }

Done:
  if (InputRecord != NULL) {
    gBS->FreePool (InputRecord);
  }
  return Status;
}

EFI_STATUS
WinNtSimpleTextInAttachToWindow (
  IN  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private
  )
{
  EFI_STATUS  Status;

  Private->NtInHandle = Private->WinNtThunk->GetStdHandle (STD_INPUT_HANDLE);

  Private->SimpleTextIn.Reset = WinNtSimpleTextInReset;
  Private->SimpleTextIn.ReadKeyStroke = WinNtSimpleTextInReadKeyStroke;
  
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_WAIT,
                  EFI_TPL_NOTIFY,
                  WinNtSimpleTextInWaitForKey,
                  Private,
                  &Private->SimpleTextIn.WaitForKey
                  );
  ASSERT_EFI_ERROR(Status);

  return Status;
}
