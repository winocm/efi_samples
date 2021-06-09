/*++

Copyright (c)  1999 - 2001 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  setsize.c
  
Abstract:

  EFI Shell command "setsize"
  Test application to adjust the file's size via the SetInfo FS interface

Revision History

--*/

#include "shell.h"

//
//
//

EFI_STATUS
InitializeSetSize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

VOID
SetFileSize (
  IN SHELL_FILE_ARG   *Arg,
  IN UINTN            NewSize
  );


//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeSetSize)
#endif

EFI_STATUS
InitializeSetSize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Adjust the file's size via the SetInfo FS interface.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error

--*/
{
  EFI_STATUS        Status;
  CHAR16            **Argv;
  UINTN             Argc;
  UINTN             Index;
  EFI_LIST_ENTRY    FileList;
  EFI_LIST_ENTRY    *Link;
  SHELL_FILE_ARG    *Arg;
  UINTN             NewSize;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeSetSize,
    L"setsize",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  Argv = SI->Argv;
  Argc = SI->Argc;

  InitializeListHead (&FileList);

  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen (Argv[Index]) == 0) {
      Print (L"setsize: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  if (Argc < 3 ) {
    Status = EFI_INVALID_PARAMETER;
    Print (L"setsize: Too few arguments\n");
    goto Done;
  }

  //
  // Expand each arg
  //
  for (Index = 2; Index < Argc; Index += 1) {
    ShellFileMetaArg (Argv[Index], &FileList);
  }

  //
  // if no file specified, get the whole directory
  //
  if (IsListEmpty(&FileList)) {
    Status = EFI_NOT_FOUND;
    Print (L"setsize: File not found\n");
    goto Done;
  }

  //
  // Crack the file size param
  //
  NewSize = Atoi(Argv[1]);

  //
  // Set the file size of each file
  //
  for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
    Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    SetFileSize (Arg, NewSize);
  }

Done:
  ShellFreeFileList (&FileList);
  return Status;
}

VOID
SetFileSize (
  IN SHELL_FILE_ARG   *Arg,
  IN UINTN            NewSize
  )
{
  EFI_STATUS          Status;

  Status = Arg->Status;
  if (!EFI_ERROR(Status)) {
    Arg->Info->FileSize = NewSize;
    Status = Arg->Handle->SetInfo(  
          Arg->Handle,
          &GenericFileInfo,
          (UINTN) Arg->Info->Size,
          Arg->Info
          );
  }

  if (EFI_ERROR(Status)) {
    Print (L"setsize: Cannot set size of %hs to %,d - %r\n",
           Arg->FullName, NewSize, Status);
  } else {
    Print (L"setsize: Set file %s to %,d [ok]\n", Arg->FullName, NewSize);
  }  
}
