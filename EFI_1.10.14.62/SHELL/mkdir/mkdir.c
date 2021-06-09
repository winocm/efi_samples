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

  mkdir.c

Abstract:

  EFI shell command "mkdir"

Revision History

--*/

#include "shell.h"

//
// Global variable
//

//
// Function declarations
//
EFI_STATUS
InitializeMkDir (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
MkDir (
  IN SHELL_FILE_ARG       *Arg
  );

//
// Entry point
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeMkDir)
#endif

EFI_STATUS
InitializeMkDir (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Command entry point. Parses command line arguments and calls internal
  function to perform actual action.

Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error

--*/
{
  EFI_STATUS              Status;
  CHAR16                  **Argv;
  UINTN                   Argc;
  UINTN                   Index;
  EFI_LIST_ENTRY          FileList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeMkDir,
    L"mkdir",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // Local variable initializations
  //
  Argv = SI->Argv;
  Argc = SI->Argc;

  InitializeListHead (&FileList);
  Index = 0;
  Link = NULL;
  Arg = NULL;

  //
  // Parse command line arguments
  //
  if (Argc == 1) {
    Print (L"mkdir: Too few arguments\n");
    goto Done;
  } 

  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"mkdir: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  //
  // Expand each dir
  //
  for (Index = 1; Index < Argc; Index += 1) {
    Status = ShellFileMetaArg (Argv[Index], &FileList);
    if (EFI_ERROR(Status)) {
      Print (L"mkdir: Cannot find %hs - %r\n", Argv[Index], Status);
      goto Done;
    }
  }

  if (IsListEmpty(&FileList)) {
    Status = EFI_NOT_FOUND;
    Print (L"mkdir: No directory specified\n");
    goto Done;
  }

  //
  // Make each directory
  //
  for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
    Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    Status = MkDir (Arg);
  }

Done:
  ShellFreeFileList (&FileList);
  return Status;
}


EFI_STATUS
MkDir (
  IN SHELL_FILE_ARG       *Arg
  )
{
  EFI_FILE_HANDLE         NewDir;
  EFI_STATUS              Status;

  //
  // Local variable initializations
  //
  NewDir = NULL;
  Status = Arg->Status;

  //
  // if the directory already exists, we can not create it
  //
  if (!EFI_ERROR(Status)) {
    Print (L"mkdir: Directory/file %hs already exists\n", Arg->FullName);
    return Status;
  }

  //
  // this is what we want
  //
  if (Status == EFI_NOT_FOUND) {

    Status = Arg->Parent->Open (
            Arg->Parent,
            &NewDir,
            Arg->FileName,
            EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
            EFI_FILE_DIRECTORY
            );
  }

  if (EFI_ERROR(Status)) {
    Print (L"mkdir: Cannot create %hs - %r\n", Arg->FullName, Status);
  }

  if (NewDir) {
    NewDir->Close(NewDir);
  }

  return Status;
}
