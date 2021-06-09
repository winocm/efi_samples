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

  ver.c
  
Abstract:

  Shell app "ver"



Revision History

--*/

#include "shell.h"
#include "ver.h"

//
//
//
EFI_STATUS
InitializeVer (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

//
//
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeVer)
#endif

EFI_STATUS
InitializeVer (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Displays version information.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS     - Command completed successfully

--*/
{
#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeVer,
    L"ver",     // command
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

  if (2 == SI->Argc) {
    if ('-' == SI->Argv[1][0] && ('b' == SI->Argv[1][1] || 'B' == SI->Argv[1][1]) &&
        0 == SI->Argv[1][2]) {
      EnablePageBreak(DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);    
    } else {
      Print (L"Ver:Unknown option '%s'\n", SI->Argv[1]);
      return EFI_INVALID_PARAMETER;
    }
  } else if (SI->Argc > 2) {
    Print (L"Ver: Too many parameters.\n");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Print version info
  //
  Print(L"EFI Specification Revision : %d.%d\n",ST->Hdr.Revision>>16,ST->Hdr.Revision&0xffff);
  Print(L"EFI Vendor                 : %s\n", ST->FirmwareVendor);
  Print(L"EFI Revision               : %d.%d\n", ST->FirmwareRevision >> 16, ST->FirmwareRevision & 0xffff);
#ifdef EFI_BUILD_VERSION
  Print(L"EFI Build Version          : %s\n", EFI_BUILD_VERSION);
#endif

  //
  // Display additional version info depending on processor type
  //
  DisplayExtendedVersionInfo(ImageHandle,SystemTable);

  //
  // Done
  //
  return EFI_SUCCESS;
}
