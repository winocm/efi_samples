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

  pause.c
  
Abstract:

  Internal Shell batch cmd "pause"

Revision History

--*/

#include "shelle.h"

//
// Internal prototypes
//
EFI_STATUS
SEnvCmdPause (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Built-in shell command "pause" for interactive continue/abort 
  functionality from scripts.

Arguments:

Returns:

--*/
{
  CHAR16                        **Argv;
  UINTN                         Argc;
  EFI_STATUS                    Status;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   *TextIn;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *TextOut;
  EFI_INPUT_KEY                 Key;
  CHAR16                        QStr[2];

  Argc    = 0;
  Status  = EFI_SUCCESS;
  TextIn  = NULL;
  TextOut = NULL;

  InitializeShellApplication (ImageHandle, SystemTable);

  Argv = SI->Argv;
  Argc = SI->Argc;

  //
  // Pause only takes affect in batch script
  //
  if ( !SEnvBatchIsActive() ) {
    Print( L"pause: Only supported in script files\n" );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  SEnvBatchGetConsole( &TextIn, &TextOut );

  Status = TextOut->OutputString( TextOut, 
                  L"Enter 'q' to quit, any other key to continue: " );
  if ( EFI_ERROR(Status) ) { 
    Print( L"pause: Writing prompt error\n" );
    goto Done;
  }

  //
  // Wait for user input
  //
  WaitForSingleEvent (TextIn->WaitForKey, 0);
  Status = TextIn->ReadKeyStroke( TextIn, &Key );
  if ( EFI_ERROR(Status) ) { 
    Print( L"pause: Reading keystroke error\n" );
    goto Done;
  }

  //
  //  Check if input character is q or Q, if so set abort flag
  //
  if ( Key.UnicodeChar == L'q' || Key.UnicodeChar == L'Q' ) {
    SEnvSetBatchAbort();
  }
  if ( Key.UnicodeChar != (CHAR16)0x0000 ) {
    QStr[0] = Key.UnicodeChar;
    QStr[1] = (CHAR16)0x0000;
    TextOut->OutputString( TextOut, QStr );
  }
  TextOut->OutputString( TextOut, L"\n\r" );

Done:
  return Status;
}
