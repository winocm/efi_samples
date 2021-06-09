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

  mode.c
  
Abstract:

  EFI Shell command "mode"


Revision History

--*/

#include "shell.h"

//
//
//

EFI_STATUS
InitializeMode (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

static 
BOOLEAN
GetNum (
  IN  UINT16  *str,
  OUT UINTN   *data
  );

//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeMode)
#endif

EFI_STATUS
InitializeMode (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Command entry point. 

Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error

--*/
{
  CHAR16          **Argv;
  UINTN           Argc;
  UINTN           NewCol;
  UINTN           NewRow;
  UINTN           Col;
  UINTN           Row;
  INTN            Mode;
  EFI_STATUS      Status;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *ConOut;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeMode,
    L"mode",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  //
  // Initialize app
  //
  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  Argv = SI->Argv;
  Argc = SI->Argc;
  ConOut = ST->ConOut;

  if ( Argc == 1 ) {
    Print (L"Available modes on standard output\n");
    //
    // dump the available modes
    //
    for (Mode=0; Mode < ConOut->Mode->MaxMode; Mode++) {
      Status = ConOut->QueryMode(ConOut, Mode, &Col, &Row);
      if (EFI_ERROR(Status)) {
        Print (L"mode: Cannot query mode - %r\n", Status);
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }
      //
      // Current mode 
      //
      Print (L"  col %3d row %3d  %c\n", Col, Row, Mode == ConOut->Mode->Mode ? '*' : ' ');
    }
  }
  else if (Argc == 2 ) {
    Print (L"mode: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    return Status;
  }
  //
  // Set a new mode
  //
  else if (Argc == 3 ) {
    if ( !GetNum(Argv[1], &NewCol) || !GetNum(Argv[2], &NewRow) ) {
      Status = EFI_INVALID_PARAMETER;
      return Status;
    }
    for (Mode=0; Mode < ConOut->Mode->MaxMode; Mode++) {
      Status = ConOut->QueryMode(ConOut, Mode, &Col, &Row);
      if (EFI_ERROR(Status)) {
        Print (L"mode: Cannot query mode - %r\n", Status);
        Status = EFI_INVALID_PARAMETER;
        return Status;
      }
      if (Row == NewRow && Col == NewCol) {
        ConOut->SetMode (ConOut, Mode);
        ConOut->ClearScreen (ConOut);
        Status = EFI_SUCCESS;
        return Status;
      }
    }
    Print (L"mode: Mode (%d,%d) not found\n", NewCol, NewRow);
  }
  else {
    Print (L"mode: Too many arguments\n");
    Status = EFI_INVALID_PARAMETER;
  }
  
  return Status;    
}


static 
BOOLEAN
GetNum (
  IN  UINT16  *str,
  OUT UINTN   *data
  )
{
  UINTN     u;
  CHAR16    c;
  //
  //
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= '0' && c <= '9') {
      u = (u * 10) + c - '0';
    } else {
      return FALSE;
    }
    c = *(str++);
  }
  *data = u;
  return TRUE;  
}
