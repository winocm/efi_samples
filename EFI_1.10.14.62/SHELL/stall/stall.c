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

  stall.c
  
Abstract: 

  Shell app "stall" - stall for some microseconds

Revision History

--*/

#include "shell.h"

EFI_STATUS
InitializeStall (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeStall)
#endif

EFI_STATUS
InitializeStall (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++
Routine Description:
  Command entry point. Stall for [microseconds].

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_UNSUPPORTED         - Stall service is not supported
--*/
{
  UINTN    Microseconds;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeStall,
    L"stall",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);
  
  if (SI->Argc < 2) {
    Print (L"stall: Too few arguments\n");
    return EFI_INVALID_PARAMETER;
  }
  if (SI->Argc > 2) {
    Print (L"stall: Too many arguments\n");
    return EFI_INVALID_PARAMETER;
  }

  if (BS->Stall == NULL) {
    Print(L"stall: Stall service not available\n");
    return EFI_UNSUPPORTED;
  }
  Microseconds = Atoi (SI->Argv[1]);
  Print(L"Stall for %d uS\n", Microseconds);
  
  if( Microseconds > 1000000 )
  {
    UINTN   Seconds;
    UINTN   Remain;
    UINTN   SecondsCount;
    
    Seconds = Microseconds / 1000000;
    Remain  = Microseconds % 1000000;
    for( SecondsCount = 0; SecondsCount < Seconds; SecondsCount++ )
    {
      BS->Stall (1000000);
    }
    BS->Stall( Remain );
  
  }
  else
    BS->Stall (Microseconds);
  
  return EFI_SUCCESS;
}
