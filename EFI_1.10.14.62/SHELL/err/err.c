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

  err.c
  
Abstract:

  Shell command "err".

Revision History

--*/

#include "shell.h"

EFI_STATUS
InitializeError (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

UINTN
PrintErrMsg (
  IN UINTN EFIDebug
  );
  
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeError)
#endif

EFI_STATUS
InitializeError (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
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
  EFI_NOT_FOUND           - Files not found
--*/
{
  EFI_STATUS    Status;
  UINT32        Attributes;
  UINTN         DataSize;
  CHAR16        Str[80];
  UINTN         DebugLevel;
 
#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeError,
    L"err",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  // 
  //  We are no being installed as an internal command driver, initialize
  //  as an nshell app and run
  //
  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // 
  //
  Status = EFI_SUCCESS;
  if ( SI->Argc > 1 ) {
    //
    // Set the current value for the shell.
    //
    DebugLevel = xtoi(SI->Argv[1]);
    
   
    EFIDebug = DebugLevel;
    while (1) {
      Input (L"Make this change and save to NVRAM? [Y/N]", Str, sizeof(Str)/sizeof(CHAR16));
      Print (L"\n");
  
      if (*Str == 'Y' || *Str == 'y') {
        //
        // Tell the core to use this value on the next boot 
        //
        Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE;
        DataSize = sizeof(EFIDebug);
        Status = RT->SetVariable(L"EFIDebug", &EfiGlobalVariable, Attributes, DataSize, &EFIDebug);
        break;
      } else if (*Str == 'N' || *Str == 'n' ) {
        break;
      }
    }
  } 
  Print (L"%EEFI ERROR%H %08x %N\n", EFIDebug);
  PrintErrMsg (EFIDebug);

  return Status;
}

UINTN
PrintErrMsg (
  IN UINTN EFIDebug
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{ 
  //
  // if a debug level is not set, 
  //    print it normally
  // if a debug level is now set, 
  //    print it highlight
  //

  if (EFIDebug & EFI_D_INIT) {
    Print (L"%H  %08x  EFI_D_INIT%N\n",     EFI_D_INIT);
  } else {
    Print (L"  %08x  EFI_D_INIT\n",     EFI_D_INIT);
  }

  if (EFIDebug & EFI_D_WARN) {
    Print (L"%H  %08x  EFI_D_WARN%N\n",     EFI_D_WARN);
  } else {
    Print (L"  %08x  EFI_D_WARN\n",     EFI_D_WARN);
  }
  
  if (EFIDebug & EFI_D_LOAD) {
    Print (L"%H  %08x  EFI_D_LOAD%N\n",     EFI_D_LOAD);
  } else {
    Print (L"  %08x  EFI_D_LOAD\n",     EFI_D_LOAD);
  }

  if (EFIDebug & EFI_D_FS) {
    Print (L"%H  %08x  EFI_D_FS%N\n",     EFI_D_FS);
  } else {
    Print (L"  %08x  EFI_D_FS\n",     EFI_D_FS);
  }

  if (EFIDebug & EFI_D_POOL) {
    Print (L"%H  %08x  EFI_D_POOL%N\n",     EFI_D_POOL);
  } else {
    Print (L"  %08x  EFI_D_POOL\n",     EFI_D_POOL);
  }
  
  if (EFIDebug & EFI_D_PAGE) {
    Print (L"%H  %08x  EFI_D_PAGE%N\n",     EFI_D_PAGE);
  } else {
    Print (L"  %08x  EFI_D_PAGE\n",     EFI_D_PAGE);
  }

  if (EFIDebug & EFI_D_INFO) {
    Print (L"%H  %08x  EFI_D_INFO%N\n",     EFI_D_INFO);
  } else {
    Print (L"  %08x  EFI_D_INFO\n",     EFI_D_INFO);
  }

  if (EFIDebug & EFI_D_VARIABLE) {
    Print (L"%H  %08x  EFI_D_VARIABLE%N\n", EFI_D_VARIABLE);
  } else {
    Print (L"  %08x  EFI_D_VARIABLE\n", EFI_D_VARIABLE);
  }

  if (EFIDebug & EFI_D_BM) {
    Print (L"%H  %08x  EFI_D_BM%N\n", EFI_D_BM);
  } else {
    Print (L"  %08x  EFI_D_BM\n", EFI_D_BM);
  }

  if (EFIDebug & EFI_D_BLKIO) {
    Print (L"%H  %08x  EFI_D_BLKIO%N\n",      EFI_D_BLKIO);
  } else {
    Print (L"  %08x  EFI_D_BLKIO\n",      EFI_D_BLKIO);
  }

  if (EFIDebug & EFI_D_NET) {
    Print (L"%H  %08x  EFI_D_NET%N\n",      EFI_D_NET);
  } else {
    Print (L"  %08x  EFI_D_NET\n",      EFI_D_NET);
  }

  if (EFIDebug & EFI_D_UNDI) {
    Print (L"%H  %08x  EFI_D_UNDI%N\n",      EFI_D_UNDI);
  } else {
    Print (L"  %08x  EFI_D_UNDI\n",      EFI_D_UNDI);
  }

  if (EFIDebug & EFI_D_LOADFILE) {
    Print (L"%H  %08x  EFI_D_LOADFILE%N\n",      EFI_D_LOADFILE);
  } else {
    Print (L"  %08x  EFI_D_LOADFILE\n",      EFI_D_LOADFILE);
  }
  
  if (EFIDebug & EFI_D_EVENT) {
    Print (L"%H  %08x  EFI_D_EVENT%N\n",    EFI_D_EVENT);
  } else {
    Print (L"  %08x  EFI_D_EVENT\n",    EFI_D_EVENT);
  }
    
  if (EFIDebug & EFI_D_ERROR) {
    Print (L"%H  %08x  EFI_D_ERROR%N\n",    EFI_D_ERROR);
  } else {
    Print (L"  %08x  EFI_D_ERROR\n",    EFI_D_ERROR);
  }
  //
  //
  //
  return EFIDebug;
}
