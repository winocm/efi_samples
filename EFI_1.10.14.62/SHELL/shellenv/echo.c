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

  echo.c
  
Abstract:

  Shell app "echo"

Revision History

--*/

#include "shelle.h"

EFI_STATUS
SEnvCmdEcho (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Shell command "echo".

Arguments:

Returns:

--*/
{
  CHAR16                  **Argv;
  UINTN                   Argc;
  UINTN                   Index;

  InitializeShellApplication (ImageHandle, SystemTable);

  Argv = SI->Argv;
  Argc = SI->Argc;

  //
  //  No args: print status
  //  One arg, either -on or -off: set console echo flag
  //  Otherwise: echo all the args.  Shell parser will expand any args or vars.
  //
  if ( Argc == 1 ) {
    Print( L"Echo is %s\n", (SEnvBatchGetEcho()?L"on":L"off") );

  } else if ( Argc == 2 && StriCmp( Argv[1], L"-on" ) == 0 ) {
    SEnvBatchSetEcho( TRUE );

  } else if ( Argc == 2 && StriCmp( Argv[1], L"-off" ) == 0 ) {
    SEnvBatchSetEcho( FALSE );

  } else {
    for (Index = 1; Index < Argc; Index += 1) {
      Print( L"%s ", Argv[Index] );
    }
    Print( L"\n" );
  }

  return EFI_SUCCESS;
}
