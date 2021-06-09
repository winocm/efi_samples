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

  reset.c
  
Abstract: 

  EFI shell command "reset"

Revision History

--*/

#include "shell.h"

EFI_STATUS
InitializeReset (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeReset)
#endif

EFI_STATUS
InitializeReset (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Displays the contents of a file on the standard output device.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error

--*/
{
  EFI_RESET_TYPE  ResetType;
  UINTN           DataSize;
  CHAR16          *ResetData;
  CHAR16          *Str;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeReset,
    L"reset",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);
  
  ResetType = EfiResetCold;
  if (SI->Argc > 1) {

    Str = SI->Argv[1];
    if ( (Str[0] == '-' || Str[0] == '/') &&
       (Str[1] == 'W' || Str[1] == 'w') &&
       StrLen( Str ) == 2 ) {
        ResetType = EfiResetWarm;
    } else if ( (Str[0] == '-' || Str[0] == '/') &&
       (Str[1] == 'S' || Str[1] == 's') &&
       StrLen( Str ) == 2 ) {
        ResetType = EfiResetShutdown;
    } else {
      Print(L"reset: Unknown flag %hs\n", SI->Argv[1]);
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Get reset string
  //
  DataSize = 0;
  ResetData = NULL;
  if (SI->Argc > 2) {
    ResetData = SI->Argv[2];
    DataSize = StrSize(ResetData);
  }

  RT->ResetSystem(ResetType, EFI_SUCCESS, DataSize, ResetData);

  // Just return a garbage value
  return EFI_LOAD_ERROR;
}
