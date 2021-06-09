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
  
    getmtc.c
  
Abstract:

  EFI Shell command "GetMtc" - Get next monotonic count

Revision History

--*/

#include "shell.h"

//
//
//

EFI_STATUS
InitializeGetMTC (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );


//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeGetMTC)
#endif

EFI_STATUS
InitializeGetMTC (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Command entry point. Get next monotonic count.  

Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:
  EFI_SUCCESS     - The command completed successfully
  
--*/
{
  UINT64            mtc;
  EFI_STATUS        Status;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeGetMTC,
    L"getmtc",     // command
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

  Status = BS->GetNextMonotonicCount (&mtc);
  if (EFI_ERROR(Status)) {
    Print (L"getmtc: Cannot get monotonic count - %r\n", Status);
  } else {
    Print (L"Monotonic count = %hlx\n", mtc);
  }

  return Status;
}
