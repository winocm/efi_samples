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

  comp.c
  
Abstract:

  EFI shell command "comp" - compare two files

Revision History

--*/

#include "shell.h"

#define  BLOCK_SIZE   (64*1024)


//
// Function declarations
//
EFI_STATUS
InitializeComp (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Entry Point
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeComp)
#endif

EFI_STATUS
InitializeComp (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Command entry point. Compares the contents of two files.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS            - The command completed successfully
  EFI_INVALID_PARAMETER  - Input command arguments error
  EFI_OUT_OF_RESOURCES   - Out of memory
  Other                  - Misc error

--*/
{
  CHAR16                  **Argv;
  UINTN                   Argc;
  EFI_LIST_ENTRY          File1List;
  EFI_LIST_ENTRY          File2List;
  SHELL_FILE_ARG          *File1Arg;
  SHELL_FILE_ARG          *File2Arg;
  UINTN                   Size;
  UINTN                   ReadSize;
  UINT8                   *File1Buffer;
  UINT8                   *File2Buffer;
  UINTN                   NotTheSameCount;
  EFI_STATUS              Status;
  UINTN                   Index;
  UINTN                   Count;
  UINTN                   Address;    
  
#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeComp,
    L"comp",     // command
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
  Size = 0;
  ReadSize = 0;
  File1Buffer = File2Buffer = NULL;
  NotTheSameCount = 0;
  Status = EFI_SUCCESS;
  Index = Count = 0;
  Address = 0;

  //
  // Parse command line arguments
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"comp: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  
  //
  // verify number of arguments
  //
  if (Argc < 3) {
    Print (L"comp: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // validate first file
  //
  Status = ShellFileMetaArg (Argv[1], &File1List);
  if (EFI_ERROR(Status)) {
    Print(L"comp: Cannot open %hs - %r\n", Argv[1], Status);
    goto Done;
  }

  //
  // empty list
  //
  if (IsListEmpty(&File1List)) {
    Status = EFI_NOT_FOUND;
    Print(L"comp: Cannot open %hs - %r\n", Argv[1], Status);
    goto Done;
  }

  //
  // multiple files
  //
  if (File1List.Flink->Flink != &File1List) {
    Print(L"comp: First argument cannot be multiple files\n");
    Status = EFI_INVALID_PARAMETER;    
    goto Done;
  }

  File1Arg = CR(File1List.Flink, SHELL_FILE_ARG, Link, 
    SHELL_FILE_ARG_SIGNATURE);

  //
  // Open error
  //
  if ( EFI_ERROR(File1Arg->Status) || !File1Arg->Handle ) {
    Print (L"comp: Cannot open %hs - %r\n", File1Arg->FullName, 
      File1Arg->Status);
    Status = File1Arg->Status;
    goto Done;
  }

  //
  // directory
  //
  if (File1Arg->Info && (File1Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    Print(L"comp: First argument cannot be a directory\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Validate second file
  //
  Status = ShellFileMetaArg (Argv[2], &File2List);
  if (EFI_ERROR(Status)) {
    Print(L"comp: Cannot open %hs - %r\n", Argv[2], Status);
    goto Done;
  }

  //
  // empty list
  //
  if (IsListEmpty(&File2List)) {
    Status = EFI_NOT_FOUND;
    Print(L"comp: Cannot open %hs - %r\n", Argv[2], Status);
    goto Done;
  }

  //
  // multiple files
  //
  if (File2List.Flink->Flink != &File2List) {
    Print(L"comp: Second argument cannot be multiple files\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  File2Arg = CR(File2List.Flink, SHELL_FILE_ARG, Link, 
    SHELL_FILE_ARG_SIGNATURE);

  //
  // open error
  //
  if ( EFI_ERROR(File2Arg->Status) || !File2Arg->Handle ) {
    Print (L"comp: Cannot open %hs - %r\n", File2Arg->FullName, 
      File2Arg->Status);
    Status = File2Arg->Status;
    goto Done;
  }

  //
  // directory
  //
  if (File2Arg->Info && (File2Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    Print(L"comp: Second argument cannot be a directory\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Allocate buffers for both files
  //
  File1Buffer = AllocatePool (BLOCK_SIZE);
  File2Buffer = AllocatePool (BLOCK_SIZE);
  if (!File1Buffer || !File2Buffer) {
    Print(L"comp: Out of memory\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  //
  // Compare files
  //

  Print(L"Compare %hs to %hs\n", File1Arg->FullName, File2Arg->FullName);

  //
  // Set positions to head
  //
  Status = File1Arg->Handle->SetPosition (File1Arg->Handle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"comp: Set file %hs pos error - %r\n", File1Arg->FullName, Status);
    goto Done;
  }

  Status = File2Arg->Handle->SetPosition (File2Arg->Handle, 0);
  if (EFI_ERROR(Status)) {
    Print(L"comp: Set file %hs pos error - %r\n", File2Arg->FullName, Status);
    goto Done;
  }

  //
  // Read blocks one by one from both files and compare each pair of blocks
  //
  Size = BLOCK_SIZE;
  NotTheSameCount = 0;
  Address = 0;

  while (Size > 0 && NotTheSameCount < 10) {
    
    //
    // Read a block from first file
    //
    Size = BLOCK_SIZE;
    Status = File1Arg->Handle->Read (File1Arg->Handle, &Size, 
      File1Buffer);
    if (EFI_ERROR(Status)) {
      Print(L"comp: Read file %hs error - %r\n",
        File1Arg->FullName, Status);
      NotTheSameCount++;
      break;
    }
    
    //
    // Read a block from second file
    //
    ReadSize = BLOCK_SIZE;
    Status = File2Arg->Handle->Read (File2Arg->Handle, &ReadSize,
      File2Buffer);
    if (EFI_ERROR(Status)) {
      Print(L"comp: Read file %hs error - %r\n",
        File2Arg->FullName, Status);
      NotTheSameCount++;
      break;
    }

    if (ReadSize != Size) {
      Print(L"Difference #%d: File sizes mismatch\n", 
        NotTheSameCount + 1);
      NotTheSameCount++;
      break;
    }

    //
    // Diff the buffer
    //
    for (Index = 0; (Index < Size) && (NotTheSameCount < 10); Index++) {
      if (File1Buffer[Index] != File2Buffer[Index] ) {
        for (Count = 1; Count < 0x20 && (Index + Count) < Size; Count++) {
          if (File1Buffer[Index + Count] == File2Buffer[Index + Count]) {
            break;
          }
        }
        Print (L"Difference #%d: File1: %s\n", NotTheSameCount + 1, File1Arg->FullName);
        DumpHex (1, Address + Index, Count, &File1Buffer[Index]);
        Print (L"File2: %s\n", File2Arg->FullName);
        DumpHex (1, Address + Index, Count, &File2Buffer[Index]);
        Print (L"\n");

        NotTheSameCount++;
        Index += Count - 1;
      }
    }

    Address += Size;
  }

  //
  // Results
  //
  if (!NotTheSameCount) {
    Print(L"[no difference encountered]\n");
  } else {
    Print(L"[difference(s) encountered]\n");
    Status = EFI_ABORTED;
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

