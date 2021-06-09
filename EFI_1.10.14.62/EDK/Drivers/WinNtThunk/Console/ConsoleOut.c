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

  ConsoleOut.c

Abstract:

  Console based on Win32 APIs. 

  This file creates an Win32 window and attaches a SimpleTextOut protocol.

--*/

#include "Console.h"

//
// Private worker functions.
//

STATIC
VOID
WinNtSimpleTextOutScrollScreen (
  IN OUT  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Console
  );

STATIC
VOID
WinNtSimpleTextOutPutChar (
  IN OUT  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Console,
  IN      CHAR16                              Char
  );

//
// Modeule Global for Simple Text Out Mode.
//
#define MAX_SIMPLE_TEXT_OUT_MODE  \
        (sizeof(mWinNtSimpleTextOutSupportedModes)/sizeof(WIN_NT_SIMPLE_TEXT_OUT_MODE))
                               
STATIC WIN_NT_SIMPLE_TEXT_OUT_MODE  mWinNtSimpleTextOutSupportedModes[] = {
  { 80, 25 },         
  { 80, 50 },         
  { 80, 43 },         
  { 100, 100 },       
  { 100, 999 }         
};


STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutReset (
  IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN BOOLEAN                              ExtendedVerification
  )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private;

  Private = WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(This);

  //reset the background and keep the foreground unchanged
  //
  This->SetAttribute(This,This->Mode->Attribute & 0xf);

  WinNtSimpleTextOutSetMode (This, 0);
  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutOutputString (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN CHAR16                                 *String
    )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA     *Private;
  CHAR16                              *Str;

  Private = WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(This);

  for (Str = String; *Str != '\0'; Str++) {
    switch (*Str) {
      case '\n':
        if (Private->Possition.Y == (Private->MaxScreenSize.Y - 1)) {
          WinNtSimpleTextOutScrollScreen (Private);
        }
        if (Private->Possition.Y < (Private->MaxScreenSize.Y - 1)) {
          Private->Possition.Y++;
          This->Mode->CursorRow++;
        }
        break;

      case '\r':
        Private->Possition.X = 0;
        This->Mode->CursorColumn = 0;
        break;

      case '\b':
        if (Private->Possition.X > 0) {
          Private->Possition.X--;
          This->Mode->CursorColumn--;
        }
        break;

      default:
        WinNtSimpleTextOutPutChar (Private, *Str);
    }
  }

  return EFI_SUCCESS;
}


STATIC
VOID
WinNtSimpleTextOutPutChar (
  IN OUT  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA   *Console,
  IN      CHAR16                            Char
  )
{
  SMALL_RECT  Region;
  COORD       StrCoordinate;
  COORD       StrSize;
  CHAR_INFO   CharInfo;
  BOOL        Flag;        

  CharInfo.Char.UnicodeChar = Char;
  CharInfo.Attributes = Console->Attribute;

  StrSize.X = 1;
  StrSize.Y = 1;
  StrCoordinate.X = 0;
  StrCoordinate.Y = 0;

  Region.Left   = (short int)Console->Possition.X;
  Region.Top    = (short int)Console->Possition.Y;
  Region.Right  = (short int)(Console->Possition.X + 1);
  Region.Bottom = (short int)Console->Possition.Y;

  Console->WinNtThunk->WriteConsoleOutput (
                         Console->NtOutHandle,
                         &CharInfo,
                         StrSize,
                         StrCoordinate,
                         &Region
                         );

  if (Console->Possition.X >= (Console->MaxScreenSize.X - 1)) {
    //
    // If you print off the end wrap around
    //
    Console->SimpleTextOut.OutputString (&Console->SimpleTextOut, L"\n\r");     
  } else {
    Console->Possition.X++;
    Console->SimpleTextOut.Mode->CursorColumn++;
  }    
  
  Flag = Console->WinNtThunk->SetConsoleCursorPosition (Console->NtOutHandle, Console->Possition);
}

STATIC
VOID
WinNtSimpleTextOutScrollScreen (
  IN OUT  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Console
  )
{
  SMALL_RECT  Scroll;
  CHAR_INFO   CharInfo;
  COORD       Origin;

  CharInfo.Char.UnicodeChar = ' ';
  CharInfo.Attributes = Console->Attribute;

  Origin.X = 0;
  Origin.Y = 0;

  Scroll.Top = 1;
  Scroll.Left = 0;
  Scroll.Right = (short int)Console->MaxScreenSize.X;
  Scroll.Bottom = (short int)Console->MaxScreenSize.Y;

  Console->WinNtThunk->ScrollConsoleScreenBuffer (
                         Console->NtOutHandle,
                         &Scroll,
                         NULL,
                         Origin,
                         &CharInfo
                         );
}




STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutTestString (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN CHAR16                                 *String
    )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private;

  Private = WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(This);

  //
  // BugBug: The correct answer would be a function of what code pages
  //         are currently loaded? For now we will just return success.
  //
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutQueryMode (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN UINTN                                  ModeNumber,
    OUT UINTN                                 *Columns,
    OUT UINTN                                 *Rows
    )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private;

  Private = WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(This);

 if (ModeNumber > MAX_SIMPLE_TEXT_OUT_MODE) {
    return EFI_INVALID_PARAMETER;
  }

  *Columns  = mWinNtSimpleTextOutSupportedModes[ModeNumber].ColumnsX;
  *Rows     = mWinNtSimpleTextOutSupportedModes[ModeNumber].RowsY;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutSetMode (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN UINTN                                  ModeNumber
    )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private;

  Private = WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(This);

  if (ModeNumber > MAX_SIMPLE_TEXT_OUT_MODE) {
    return EFI_INVALID_PARAMETER;
  }

  Private->MaxScreenSize.X = (WORD)mWinNtSimpleTextOutSupportedModes[ModeNumber].ColumnsX;
  Private->MaxScreenSize.Y = (WORD)mWinNtSimpleTextOutSupportedModes[ModeNumber].RowsY;
  
  Private->WinNtThunk->SetConsoleScreenBufferSize (Private->NtOutHandle, Private->MaxScreenSize);
  Private->WinNtThunk->SetConsoleActiveScreenBuffer (Private->NtOutHandle);

  This->Mode->Mode = (INT32)ModeNumber;

  This->EnableCursor (This, TRUE);
  This->ClearScreen (This);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutSetAttribute (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN UINTN                      Attribute
    )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private;

  Private = WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(This);
  
  Private->Attribute = (WORD)Attribute;
  This->Mode->Attribute = (INT32)Attribute;
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutClearScreen (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This
    )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private;
  DWORD                           ConsoleWindow;                          

  Private = WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(This);

  This->SetCursorPosition (This, 0, 0); 

  Private->WinNtThunk->FillConsoleOutputCharacter (
                         Private->NtOutHandle,
                         ' ',
                         Private->MaxScreenSize.X * Private->MaxScreenSize.Y,
                         Private->Possition,
                         &ConsoleWindow
                         );
  Private->WinNtThunk->FillConsoleOutputAttribute (
                         Private->NtOutHandle,
                         Private->Attribute,
                         Private->MaxScreenSize.X * Private->MaxScreenSize.Y,
                         Private->Possition,
                         &ConsoleWindow
                         );

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutSetCursorPosition (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN UINTN                                  Column,
    IN UINTN                                  Row
    )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private;

  Private = WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(This);

  Private->Possition.X      = (WORD)Column;
  This->Mode->CursorColumn  = (INT32)Column;

  Private->Possition.Y  = (WORD)Row;
  This->Mode->CursorRow = (INT32)Row;
  Private->WinNtThunk->SetConsoleCursorPosition (Private->NtOutHandle, Private->Possition);
  
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
WinNtSimpleTextOutEnableCursor (
    IN struct _EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
    IN BOOLEAN                    Enable
    )
{
  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private;
  CONSOLE_CURSOR_INFO     Info;

  Private = WIN_NT_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(This);
  Private->CursorEnable = Enable;
  This->Mode->CursorVisible = Enable;

  Private->WinNtThunk->GetConsoleCursorInfo (Private->NtOutHandle, &Info);
  Info.bVisible = Enable;
  Private->WinNtThunk->SetConsoleCursorInfo (Private->NtOutHandle, &Info);
  
  return EFI_SUCCESS;
}




EFI_STATUS
WinNtSimpleTextOutOpenWindow (
  IN OUT  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Private
  )
{
  EFI_SIMPLE_TEXT_OUT_PROTOCOL      *SimpleTextOut;
  CHAR16                            *WindowName;

  WindowName = Private->WinNtIo->EnvString;
  Private->Attribute = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  if (*WindowName == '?') {
    Private->Attribute = BACKGROUND_RED | FOREGROUND_INTENSITY |
                         FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
    WindowName = L"EFI Emulator Error Console";
  }

  EfiLibAddUnicodeString (
    "eng", 
    gWinNtConsoleComponentName.SupportedLanguages, 
    &Private->ControllerNameTable, 
    WindowName
    );

  //
  // Fill in protocol member functions
  //
  SimpleTextOut = &Private->SimpleTextOut;
  SimpleTextOut->Reset              = WinNtSimpleTextOutReset;
  SimpleTextOut->OutputString       = WinNtSimpleTextOutOutputString;
  SimpleTextOut->TestString         = WinNtSimpleTextOutTestString;
  SimpleTextOut->QueryMode          = WinNtSimpleTextOutQueryMode;
  SimpleTextOut->SetMode            = WinNtSimpleTextOutSetMode;
  SimpleTextOut->SetAttribute       = WinNtSimpleTextOutSetAttribute;
  SimpleTextOut->ClearScreen        = WinNtSimpleTextOutClearScreen;
  SimpleTextOut->SetCursorPosition  = WinNtSimpleTextOutSetCursorPosition;
  SimpleTextOut->EnableCursor       = WinNtSimpleTextOutEnableCursor;

  //
  // Initialize SimpleTextOut protocol mode structure
  //
  SimpleTextOut->Mode = &Private->SimpleTextOutMode;
  SimpleTextOut->Mode->MaxMode = MAX_SIMPLE_TEXT_OUT_MODE;
  SimpleTextOut->Mode->Attribute = (INT32)Private->Attribute;

  //
  // Open the window an initialize it!
  //
  Private->NtOutHandle = Private->WinNtThunk->CreateConsoleScreenBuffer(
                                                GENERIC_WRITE | GENERIC_READ,
                                                FILE_SHARE_WRITE | FILE_SHARE_READ,
                                                NULL,
                                                CONSOLE_TEXTMODE_BUFFER,
                                                NULL
                                                );
  Private->WinNtThunk->SetConsoleTitle (WindowName);

  return SimpleTextOut->SetMode (SimpleTextOut, 0);
}


EFI_STATUS
WinNtSimpleTextOutCloseWindow (
  IN OUT  WIN_NT_SIMPLE_TEXT_PRIVATE_DATA *Console
  )
{
  Console->WinNtThunk->CloseHandle (Console->NtOutHandle);
  return EFI_SUCCESS;
}


  
