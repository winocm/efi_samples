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
  
    CompressMain.c
  
Abstract:

  EFI shell command "EfiCompress" - compress a file

Revision History

--*/

#include "shell.h"
#include "Compress.h"

//
// Function declarations
//
EFI_STATUS
InitializeCompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Entry Point
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeCompress)
#endif

EFI_STATUS
InitializeCompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Command entry point. Compress the contents of a file.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error
  
--*/
{
  CHAR16                  **Argv;
  UINTN                   Argc;
  EFI_LIST_ENTRY          File1List;
  EFI_LIST_ENTRY          File2List;
  SHELL_FILE_ARG          *File1Arg;
  SHELL_FILE_ARG          *File2Arg;
  EFI_FILE_HANDLE         File2Handle;
  UINTN                   SourceSize;
  UINTN                   DestinationSize;
  UINT8                   *File1Buffer;
  UINT8                   *File2Buffer;
  EFI_STATUS              Status;
  UINTN                   Index;
  INT32                   Ratio;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeCompress,
    L"EfiCompress",     // command
    NULL,       // command syntax
    NULL,       // 1 line descriptor
    NULL        // command help page
    );
#endif

  //
  // We are not being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  Status = EFI_SUCCESS;
  InitializeShellApplication (ImageHandle, SystemTable);
  
  //
  // Local variable initializations
  //
  Argv = SI->Argv;
  Argc = SI->Argc;
  InitializeListHead (&File1List);
  InitializeListHead (&File2List);
  File1Arg = NULL;
  File2Arg = NULL;
  SourceSize = 0;
  File1Buffer = NULL;
  File2Buffer = NULL;
  Status = EFI_SUCCESS;
  Index = 0;

  //
  // Parse command line arguments
  //

  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"EfiCompress: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } 
  }
  
  //
  // verify number of arguments
  //
  if (Argc < 3) {
    Print (L"EfiCompress: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // validate first file
  //
  Status = ShellFileMetaArg (Argv[1], &File1List);
  if (EFI_ERROR(Status)) {
    Print(L"EfiCompress: Cannot open %hs - %r\n", Argv[1], Status);
    goto Done;
  }
  
  //
  // empty list
  //
  if (IsListEmpty(&File1List)) {
    Status = EFI_NOT_FOUND;
    Print(L"EfiCompress: Cannot open %hs - %r\n", Argv[1], Status);
    goto Done;
  }

  //
  // multiple files
  //
  if (File1List.Flink->Flink != &File1List) {
    Print(L"EfiCompress: First argument cannot be multiple files\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  File1Arg = CR(File1List.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  //
  // Open error
  //
  if ( EFI_ERROR(File1Arg->Status) || !File1Arg->Handle ) {
    Print (L"EfiCompress: Cannot open %hs - %r\n", File1Arg->FullName, File1Arg->Status);
    Status = File1Arg->Status;
    goto Done;
  }

  //
  // directory
  //
  if (File1Arg->Info && (File1Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    Print(L"EfiCompress: First argument cannot be a directory\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Validate second file
  //
  Status = ShellFileMetaArg (Argv[2], &File2List);
  if (EFI_ERROR(Status)) {
    Print(L"EfiCompress: Cannot open %hs - %r\n", Argv[2], Status);
    goto Done;
  }

  //
  // multiple files
  //
  if (File2List.Flink->Flink != &File2List) {
    Print(L"EfiCompress: Second argument cannot be multiple files\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  File2Arg = CR(File2List.Flink, SHELL_FILE_ARG, Link, 
    SHELL_FILE_ARG_SIGNATURE);

  //
  // directory
  //
  if (File2Arg->Info && (File2Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    Print(L"EfiCompress: Second argument cannot be a directory\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Allocate buffers for both files
  //
  SourceSize = (UINTN)File1Arg->Info->FileSize;

  File1Buffer = AllocatePool (SourceSize);
  if (File1Buffer == NULL) {
    Print(L"EfiCompress: Out of memory\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = File1Arg->Handle->Read (File1Arg->Handle, &SourceSize, File1Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"EfiCompress: Read %hs error - %r\n", File1Arg->FullName, Status);
    goto Done;
  }

  DestinationSize = SourceSize;
  File2Buffer     = AllocatePool(SourceSize);
  if (File2Buffer == NULL) {
    Print(L"EfiCompress: Out of memory\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = Compress(File1Buffer, (UINT32) SourceSize, File2Buffer, (UINT32*) &DestinationSize);

  if (SourceSize) {
    Ratio = ((INT32)SourceSize*100 - (INT32)DestinationSize*100) / (INT32)SourceSize;
    if (Ratio >= 0) {
      Print (L"\nOrig Size = %d  Comp Size = %d  Ratio = %d%%\n", SourceSize, DestinationSize, Ratio);
    } else {
      Print (L"\nOrig Size = %d  Comp Size = %d  Ratio = -%d%%\n", SourceSize, DestinationSize, -Ratio);
    }
  } else {
    Print (L"\nOrig Size = %d  Comp Size = %d\n", SourceSize, DestinationSize);
  }  
  
  if (Status == EFI_BUFFER_TOO_SMALL) {
    FreePool (File2Buffer);
    File2Buffer = AllocatePool(DestinationSize);
    if (File2Buffer == NULL) {
      Print(L"EfiCompress: Out of memory\n");
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    Status = Compress(File1Buffer, (UINT32) SourceSize, File2Buffer, (UINT32*) &DestinationSize);
  }

  if (EFI_ERROR(Status)) {
    Print(L"EfiCompress: Compress error - %r\n", Status);
    goto Done;
  }

  //
  //
  //
  if (File2Arg->Status == EFI_SUCCESS &&
      File2Arg->OpenMode & EFI_FILE_MODE_READ &&
      File2Arg->OpenMode & EFI_FILE_MODE_WRITE) {
    File2Handle = File2Arg->Handle;
    File2Arg->Info->FileSize = 0;
    Status = File2Handle->SetInfo(
                            File2Handle,
                            &GenericFileInfo,
                            (UINTN)File2Arg->Info->Size,
                            File2Arg->Info
                            );
    Status = File2Handle->Write (File2Handle, &DestinationSize, File2Buffer);
    if (EFI_ERROR(Status)) {
      Print(L"EfiCompress: Write %hs error - %r\n", 
        File2Arg->FullName, Status);
      Status = EFI_ACCESS_DENIED;
      goto Done;
    }

  } else {
    Status = File2Arg->Parent->Open(
                                 File2Arg->Parent,
                                 &File2Handle,
                                 File2Arg->FileName,
                                 EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE,
                                 0
                                 );
    if (EFI_ERROR(Status)) {
      Print (L"EfiCompress: Create %hs error - %r\n",
        File2Arg->FullName, Status);
      goto Done;
    }

    Status = File2Handle->Write (File2Handle, &DestinationSize, File2Buffer);
    if (EFI_ERROR(Status)) {
      Print(L"EfiCompress: Write %hs error - %r\n", File2Arg->FullName, Status);
      File2Handle->Close (File2Handle);
      goto Done;
    }
    File2Handle->Close (File2Handle);
  }

Done:
  if (File1Buffer) {
    FreePool (File1Buffer);
  }
  if (File2Buffer) {
    FreePool (File2Buffer);
  }

  ShellFreeFileList (&File1List);
  ShellFreeFileList (&File2List);

  //
  // Shell command always succeeds
  //
  return Status;
}
