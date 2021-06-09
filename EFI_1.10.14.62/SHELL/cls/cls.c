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

  cls.c
  
Abstract:


Revision History

--*/

#include "shell.h"

EFI_STATUS
InitializeCls (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeCls)
#endif

EFI_STATUS
InitializeCls (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  UINTN Background;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeCls,
    L"cls",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  InitializeShellApplication (ImageHandle, SystemTable);

  //
  //
  //
  if ( SI->Argc > 1 ) {
  
    Background = xtoi(SI->Argv[1]);
    if (Background > EFI_LIGHTGRAY) {
      Background = EFI_BLACK;
    }
    ST->ConOut->SetAttribute(ST->ConOut,(ST->ConOut->Mode->Attribute & 0x0f) | (Background << 4));
  }
  
  ST->ConOut->ClearScreen(ST->ConOut);

  return EFI_SUCCESS;
}
