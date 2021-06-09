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

  decompress.c
  
Abstract:

  EFI shell command "EfiDecompress" - decompress a file

Revision History

--*/

#include "shell.h"

#include EFI_PROTOCOL_DEFINITION(Decompress)

//
// Function declarations
//
EFI_STATUS
InitializeDecompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Entry Point
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeDecompress)
#endif

EFI_STATUS
InitializeDecompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Command entry point. Decompress the contents of a file.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_OUT_OF_RESOURCES    - Out of memory
  EFI_UNSUPPORTED         - Protocols unsupported
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
  UINTN                   ScratchSize;
  UINT8                   *File1Buffer;
  UINT8                   *File2Buffer;
  UINT8                   *Scratch;
  EFI_STATUS              Status;
  UINTN                   Index;
  EFI_DECOMPRESS_PROTOCOL *Decompress;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeDecompress,
    L"EfiDecompress",     // command
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
  Scratch = NULL;
  Index = 0;

  //
  //
  //
  Status = BS->LocateProtocol(&gEfiDecompressProtocolGuid, NULL, &Decompress);
  if (EFI_ERROR (Status)) {
    Print(L"EfiDecomprss: Decompress Protocol not found\n");
    Status = EFI_UNSUPPORTED;
    goto Done;
  }
  
  //
  // Parse command line arguments
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"EfiDecompress: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } 
  }
  
  //
  // verify number of arguments
  //
  if (Argc < 3) {
    Print (L"EfiDecompress: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // validate first file
  //
  Status = ShellFileMetaArg (Argv[1], &File1List);
  if (EFI_ERROR(Status)) {
    Print(L"EfiDecompress: Cannot open %hs - %r\n", Argv[1], Status);
    goto Done;
  }
  
  //
  // empty list
  //
  if (IsListEmpty(&File1List)) {
    Status = EFI_NOT_FOUND;
    Print(L"EfiDecompress: Cannot open %hs - %r\n", Argv[1], Status);
    goto Done;
  }

  //
  // multiple files
  //
  if (File1List.Flink->Flink != &File1List) {
    Print(L"EfiDecompress: First argument cannot be multiple files\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  File1Arg = CR(File1List.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  //
  // Open error
  //
  if ( EFI_ERROR(File1Arg->Status) || !File1Arg->Handle ) {
    Print (L"EfiDecompress: Cannot open %hs - %r\n", File1Arg->FullName, File1Arg->Status);
    Status = File1Arg->Status;
    goto Done;
  }

  //
  // directory
  //
  if (File1Arg->Info && (File1Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    Print(L"EfiDecompress: First argument cannot be a directory\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Validate second file
  //
  Status = ShellFileMetaArg (Argv[2], &File2List);
  if (EFI_ERROR(Status)) {
    Print(L"EfiDecompress: Cannot open %hs - %r\n", Argv[2], Status);
    goto Done;
  }

  //
  // multiple files
  //
  if (File2List.Flink->Flink != &File2List) {
    Print(L"EfiDecompress: Second argument cannot be multiple files\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  File2Arg = CR(File2List.Flink, SHELL_FILE_ARG, Link, 
    SHELL_FILE_ARG_SIGNATURE);

  //
  // directory
  //
  if (File2Arg->Info && (File2Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    Print(L"EfiDecompress: Second argument cannot be a directory\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Allocate buffers for both files
  //
  SourceSize = (UINTN)File1Arg->Info->FileSize;
  File1Buffer = AllocatePool (SourceSize);
  if (File1Buffer == NULL) {
    Print(L"EfiDecompress: Out of memory\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = File1Arg->Handle->Read (File1Arg->Handle, &SourceSize, File1Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"EfiDecompress: Read %hs error - %r\n", File1Arg->FullName, Status);
    goto Done;
  }

  DestinationSize = 0;
  ScratchSize = 0;
  Status = Decompress->GetInfo(
                         Decompress, 
                         File1Buffer, 
                         (UINT32) SourceSize, 
                         (UINT32*) &DestinationSize, 
                         (UINT32*) &ScratchSize
                         );
  if (EFI_ERROR (Status)) {
    Print(L"EfiDecompress: Compressed file %hs has damaged - %r\n", 
      File1Arg->FullName, Status);
    goto Done;
  }

  File2Buffer = AllocatePool (DestinationSize);
  if (File2Buffer == NULL) {
    Print(L"EfiDecompress: Out of memory\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Scratch = AllocatePool (ScratchSize);
  if (Scratch == NULL) {
    Print(L"EfiDecompress: Out of memory\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = Decompress->Decompress(
                         Decompress, 
                         File1Buffer, 
                         (UINT32) SourceSize, 
                         File2Buffer,
                         (UINT32) DestinationSize, 
                         Scratch,
                         (UINT32) ScratchSize
                         );
  if (EFI_ERROR (Status)) {
    Print(L"EfiDecompress: Decompress %hs error - %r\n", 
      File1Arg->FullName, Status);
    goto Done;
  }


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
      Print(L"EfiDecompress: Write %hs error - %r\n", File2Arg->FullName, Status);
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
      Print (L"EfiDecompress: Create %hs error - %r\n",File2Arg->FullName,Status);
      goto Done;
    }

    Status = File2Handle->Write (File2Handle, &DestinationSize, File2Buffer);
    if (EFI_ERROR(Status)) {
      Print(L"EfiDecompress: Write %hs error - %r\n", File2Arg->FullName, Status);
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
  if (Scratch) {
    FreePool (Scratch);
  }

  ShellFreeFileList (&File1List);
  ShellFreeFileList (&File2List);

  //
  // Shell command always succeeds
  //
  return Status;
}
