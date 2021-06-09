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

  touch.c
  
Abstract:

  EFI Shell command "touch"

Revision History

--*/

#include "shell.h"

//
//
//

EFI_STATUS
InitializeTouch (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

VOID
TouchFile (
  IN SHELL_FILE_ARG   *Arg,
  IN BOOLEAN          Recursive
  );

SHELL_FILE_ARG *
CreateChild (
  IN SHELL_FILE_ARG       *Parent,
  IN CHAR16               *FileName,
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

VOID
TouchFreeFileArg (
  IN SHELL_FILE_ARG   *Arg
  );

#define FILE_INFO_SIZE  (SIZE_OF_EFI_FILE_INFO + 1024)

//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeTouch)
#endif

EFI_STATUS
InitializeTouch (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Update time of file/directory with current time.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_NOT_FOUND           - Files not found

--*/
{
  CHAR16            **Argv;
  UINTN             Argc;
  UINTN             Index;
  EFI_LIST_ENTRY    FileList;
  EFI_LIST_ENTRY    *Link;
  SHELL_FILE_ARG    *Arg;
  UINT16            *Ptr;
  BOOLEAN           Recursive;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeTouch,
    L"touch",     // command
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
  Argv = SI->Argv;
  Argc = SI->Argc;
  InitializeListHead (&FileList);
  Recursive = FALSE;

  if (Argc == 1) {
    Print (L"touch: Too few arguments\n");
    return EFI_INVALID_PARAMETER;
  }
      
  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"touch: Argument with zero length is not allowed\n");
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Expand each arg
  //
  for (Index = 1; Index < Argc; Index += 1) {
    Ptr = Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
        case 'r':
        case 'R': 
          Recursive = TRUE; 
          break;
        
        default:
          Print (L"touch: Unknown flag %hs\n", Ptr);
          return EFI_INVALID_PARAMETER;
      }
      continue;
    }

    ShellFileMetaArg (Ptr, &FileList);
  }

  //
  // if no file specified, get the whole directory
  //
  if (IsListEmpty(&FileList)) {
    Print (L"touch: File not found\n");
    return EFI_NOT_FOUND;
  }

  //
  // Touch each file
  //
  for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
    Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    TouchFile (Arg, Recursive);
  }

  ShellFreeFileList (&FileList);
  return EFI_SUCCESS;
}

VOID
TouchFile (
  IN SHELL_FILE_ARG     *Arg,
  IN BOOLEAN            Recursive
  )
{
  EFI_STATUS          Status;
  SHELL_FILE_ARG      *Child;
  EFI_LIST_ENTRY      FileList;
  EFI_FILE_INFO       *FileInfo;
  UINTN               Size;  

  InitializeListHead (&FileList);

  if (Arg == NULL) {
    return;
  }

  Status = Arg->Status;
  if (!EFI_ERROR(Status)) {
    RT->GetTime (&Arg->Info->ModificationTime, NULL);
    Status = Arg->Handle->SetInfo(  
          Arg->Handle,
          &GenericFileInfo,
          (UINTN) Arg->Info->Size,
          Arg->Info
          );
  }

  if (EFI_ERROR(Status)) {
    Print (L"touch: Cannot touch %hs - %r\n", Arg->FullName, Status);
  } else {
    Print (L"touch: %s [ok]\n", Arg->FullName);
  }  

  FileInfo = AllocatePool (FILE_INFO_SIZE);
  if (!FileInfo) {
    Print (L"touch: Out of memory\n");
    return;
  }

  //
  //if it's a directory, open it and recursive
  //
  if ( Recursive && Arg->Info && Arg->Info->Attribute & EFI_FILE_DIRECTORY ) {        
    Arg->Handle->SetPosition (Arg->Handle, 0);
    for (; ;) {
      Size = FILE_INFO_SIZE;
      Status = Arg->Handle->Read (Arg->Handle, &Size, FileInfo);
      if (EFI_ERROR(Status) || Size == 0) {
        break;
      }

      //
      // Skip "." and ".."
      //
      if (StriCmp(FileInfo->FileName, L".") == 0 ||
        StriCmp(FileInfo->FileName, L"..") == 0) {
        continue;
      }

      //
      // Build a shell_file_arg for the sub-entry
      //
      Child = CreateChild (Arg, FileInfo->FileName, &FileList);

      TouchFile (Child, Recursive);

      //
      // Close the handles
      //
      ShellFreeFileList (&FileList);
    }
  }

  if (FileInfo) {
    FreePool(FileInfo);
  }
}

SHELL_FILE_ARG *
CreateChild (
  IN SHELL_FILE_ARG       *Parent,
  IN CHAR16               *FileName,
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
{
  SHELL_FILE_ARG      *Arg;
  UINTN               Len;

  Arg = AllocateZeroPool (sizeof(SHELL_FILE_ARG));
  if (!Arg) {
    return NULL;
  }

  Arg->Signature = SHELL_FILE_ARG_SIGNATURE;
  Parent->Parent->Open (Parent->Handle, &Arg->Parent, L".", EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
  
  Arg->ParentName = StrDuplicate(Parent->FullName);
  Arg->FileName = StrDuplicate(FileName);

  //
  // append filename to parent's name to get the file's full name
  //
  Len = StrLen(Arg->ParentName);
  if (Len && Arg->ParentName[Len-1] == '\\') {
    Len -= 1;
  }

  Arg->FullName = PoolPrint(L"%.*s\\%s", Len, Arg->ParentName, FileName);

  //
  // open it
  //
  Arg->Status = Parent->Handle->Open (
            Parent->Handle, 
            &Arg->Handle, 
            Arg->FileName,
            EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
            0
            );

  if (EFI_ERROR (Arg->Status)) {
    TouchFreeFileArg (Arg);
    return NULL;
  }

  Arg->Info = AllocatePool (FILE_INFO_SIZE);
  if (!Arg->Info) {
    Print (L"touch: Out of memory\n");
    TouchFreeFileArg (Arg);
    return NULL;
  }
  Len = FILE_INFO_SIZE;
  Arg->Status = Arg->Handle->GetInfo(Arg->Handle, &GenericFileInfo, &Len, Arg->Info);
  
  InsertTailList (ListHead, &Arg->Link);
  return Arg;
}

VOID
TouchFreeFileArg (
  IN SHELL_FILE_ARG   *Arg
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  //
  // Free memory for all elements belonging to Arg
  //
  if (Arg->Parent) {
    Arg->Parent->Close (Arg->Parent);
  }

  if (Arg->ParentName) {
    FreePool (Arg->ParentName);
  }

  if (Arg->ParentDevicePath) {
    FreePool (Arg->ParentDevicePath);
  }

  if (Arg->FullName) {
    FreePool (Arg->FullName);
  }

  if (Arg->FileName) {
    FreePool (Arg->FileName);
  }

  if (Arg->Handle) {
    Arg->Handle->Close (Arg->Handle);
  }

  if (Arg->Info) {
    FreePool (Arg->Info);
  }

  if (Arg->Link.Flink) {
    RemoveEntryList (&Arg->Link);
  }
  
  //
  // Free memory for Arg
  //
  FreePool(Arg);
}

