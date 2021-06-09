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

  rm.c

Abstract:

  EFI Shell command "rm"

Revision History

--*/

#include "shell.h"

//
// Global variables
//
BOOLEAN mRmUserCanceled;

//
// Function declarations
//
EFI_STATUS
InitializeRM (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
RemoveRM (
  IN EFI_FILE_HANDLE          FileHandle,
  IN CHAR16                   *FileName,
  IN BOOLEAN                  Quiet
  );

//
// Entry point
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeRM)
#endif

EFI_STATUS
InitializeRM (
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
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_ACCESS_DENIED       - Files/directories can't be removed
  EFI_OUT_OF_RESOURCES    - Out of memory/file handles
  Other value             - Unknown error

--*/
{
  EFI_STATUS              Status;
  CHAR16                  **Argv;
  UINTN                   Argc;
  UINTN                   Index;
  UINTN                   Index1;
  EFI_LIST_ENTRY          FileList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;
  CHAR16                  *Ptr;
  CHAR16                  *TargetDev;
  CHAR16                  *CurDir;
  CHAR16                  *CurPath;
  CHAR16                  *TargetPath;
  CHAR16                  *CurFullName;
  CHAR16                  TempChar;
  BOOLEAN                 Quiet;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeRM,
    L"rm",     // command
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

  //
  // Local variable initializations
  //
  Index = 0;
  Index1 = 0;
  Argv = SI->Argv;
  Argc = SI->Argc;

  InitializeListHead (&FileList);
  Link = NULL;
  Arg = NULL;
  Ptr = NULL;
  CurDir = NULL;
  TargetDev = NULL;
  CurPath = NULL;
  TargetPath = NULL;
  CurFullName = NULL;
  TempChar = 0;
  Quiet = FALSE;

  //
  // Global variable initializations
  //
  mRmUserCanceled = FALSE;

  //
  // Parse command line arguments
  //
  if (Argc < 2) {
    Print (L"rm: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  } 

  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"rm: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
    if (Argv[Index][0] == '-' && (Argv[Index][1] == 'q' || Argv[Index][1] == 'Q')) {
      Quiet = TRUE;
    }    
  }

  //
  // Expand each arg
  //
  for (Index1 = 1; Index1 < Argc; Index1 += 1) {
    
    if (Argv[Index1][0] == '-' && (Argv[Index1][1] == 'q' || Argv[Index1][1] == 'Q')) {
      //
      // Skip Switch
      //
      
      continue;
    }
    
    Status = ShellFileMetaArg (Argv[Index1], &FileList);
    if (EFI_ERROR(Status)) {
      Print (L"rm: Cannot find %hs - %r\n", Argv[Index1], Status);
      goto Done;
    }

    if (IsListEmpty(&FileList)) {
      Status = EFI_NOT_FOUND;
      Print (L"rm: File not found\n");
      goto Done;
    }

    //
    // Remove each file
    //
    for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
      Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
      if (Arg->Status != EFI_SUCCESS) {
        Print (L"rm: Cannot find %hs - %r\n", Arg->FullName, Arg->Status);
        Status = Arg->Status;
        goto Done;
      }

      //
      // Check to see if we are removing a root dir
      //
      for (Ptr = Arg->FullName; (*Ptr); Ptr++) {
        if ((*Ptr) == ':') {
          break;
        }
      }

      if (*Ptr && *(Ptr + 1) == '\\' && *(Ptr + 2) == 0) {
        Print (L"rm: Cannot remove root directory\n");
        Status = EFI_ACCESS_DENIED;
        goto Done;
      }

      //
      // Check to see if we are removing current dir or its ancestor
      //
      if (TargetDev) {
        FreePool(TargetDev);
      }

      TargetDev = StrDuplicate (Arg->FullName);
      for (Index = 0; Index < StrLen(TargetDev); Index ++) {
        if (TargetDev[Index] == ':') {
          break;
        }
      }

      TargetDev[Index] = 0;

      //
      // find out the current directory
      //
      CurDir = ShellCurDir(TargetDev);
      if (!CurDir) {
        Print (L"rm: Cannot get current directory\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      for(CurPath = CurDir; *CurPath && *CurPath!=':';
        CurPath++) {
        ;
      }
      for(TargetPath = Arg->FullName; *TargetPath && *TargetPath!=':';
        TargetPath++) {
        ;
      }

      if (CurFullName) {
        FreePool(CurFullName);
      }
      CurFullName = StrDuplicate(CurPath);
      if (!CurFullName) {
        Print (L"rm: Out of resource\n");
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      for (Index = StrLen(CurFullName); Index > 0; Index --) {
        TempChar = CurFullName[Index];
        CurFullName[Index] = 0;
        if (StriCmp(TargetPath, CurFullName) == 0) {
          if (TargetPath[Index - 1] == '\\' || TempChar == '\\' || TempChar == 0) {
            Print(L"rm: Cannot remove current directory or its ancestor\n");
            Status = EFI_ACCESS_DENIED;
            goto Done;
          }
        }
      }

      //
      // Remove it
      //

      Status = RemoveRM (Arg->Handle, Arg->FullName, Quiet);
      if (EFI_ERROR(Status)) {
        Print (L"- error - %r\n", Status);
        Arg->Handle = NULL;
        goto Done;
      } else {
        if (!mRmUserCanceled) {
          Print (L" - [ok]\n");
          Arg->Handle = NULL;
        } else {
          Print (L"- Canceled by user\n");
          mRmUserCanceled = FALSE;
        }

      }
      FreePool (CurDir);
      CurDir = NULL;
    }

    ShellFreeFileList (&FileList);
  }

Done:
  if (CurDir) {
    FreePool(CurDir);
  }

  if (TargetDev) {
    FreePool(TargetDev);
  }

  if (CurFullName) {
    FreePool (CurFullName);
  }
  
  ShellFreeFileList (&FileList);

  return Status;
}


EFI_STATUS
RemoveRM (
  IN EFI_FILE_HANDLE          FileHandle,
  IN CHAR16                   *FileName,
  IN BOOLEAN                  Quiet
  )
/*++

Routine Description:

  Remove the file or directory indicated by FileHandle

Arguments:

  FileHandle      The handle of the file or directory to be removed
  FileName        The name of the file or directory to be removed
  Quiet           Indicates if a prompt for user confirmation is required

Returns:

  EFI_SUCCESS     The function completed successfully
  Other Value     The function failed due to some reason

--*/
{
  EFI_STATUS                  Status;
  EFI_FILE_HANDLE             ChildHandle;
  UINTN                       Size;
  CHAR16                      Str[2];
  EFI_FILE_INFO               *Info;
  CHAR16                      *ChildFileName;
  UINTN                       ChildNameSize;

  //
  // Local variable initializations
  //
  Status = EFI_SUCCESS;
  ChildHandle = NULL;
  Size = 0;
  Str[0] = 0;
  Info = NULL;
  ChildFileName = NULL;
  ChildNameSize = 0;

  Size = (SIZE_OF_EFI_FILE_INFO + 1024);
  Info = AllocatePool(Size);
  if (!Info) {
    Print (L"rm: Out of resources\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  //
  // If the file is a directory check it
  //
  Status = FileHandle->GetInfo(FileHandle, &GenericFileInfo, &Size, Info);
  if (EFI_ERROR(Status)) {
    Print(L"rm: Cannot access %hs\n", FileName);
    goto Done;
  }

  if (Info->Attribute & EFI_FILE_DIRECTORY) {
    //
    // Remove all child entries from the directory
    //
    FileHandle->SetPosition (FileHandle, 0);
    for (; ;) {
      Size = (SIZE_OF_EFI_FILE_INFO + 1024);
      Status = FileHandle->Read (FileHandle, &Size, Info);
      if (EFI_ERROR(Status) || Size == 0) {
        break;
      }

      //
      // Skip "." and ".."
      //
      if (StriCmp(Info->FileName, L".") == 0 ||
        StriCmp(Info->FileName, L"..") == 0) {
        continue;
      }

      //
      // Open the child
      //
      Status = FileHandle->Open(FileHandle, &ChildHandle,
              Info->FileName,
              EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE,
              0);
      if (EFI_ERROR(Status)) {
        Print(L"rm: Cannot open %hs under %hs in write mode - %r\n",
            Info->FileName, FileName, Status);
        goto Done;
      }

      //
      // Prompt
      //
      if (!Quiet) {
        Print (L"rm: Remove subtree %hs [y/n]? ", FileName);
        Input (NULL, Str, 2);
        Print (L"\n");

        Status =
          (Str[0] == 'y' || Str[0] == 'Y') ? EFI_SUCCESS : EFI_ACCESS_DENIED;

        if (EFI_ERROR(Status)) {
          mRmUserCanceled = TRUE;
          ChildHandle->Close (ChildHandle);
          Status = EFI_SUCCESS;
          goto Done;
        }
      }

      //
      // Compose new file name
      //
      
      ChildNameSize = StrSize(FileName) + 1 + StrSize(Info->FileName) + 2;
      
      ChildFileName = AllocatePool(ChildNameSize);
      if (!ChildFileName) {
        Print(L"rm: Out of memory\n");
        ChildHandle->Close (ChildHandle);
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      StrCpy(ChildFileName, FileName);
      if (ChildFileName[StrLen(ChildFileName)-1] != '\\') {
        StrCat(ChildFileName, L"\\");
      }
      StrCat(ChildFileName, Info->FileName);

      if (ChildNameSize > 2048) {
        Print (L"rm: Path of %hs is too long\n", ChildFileName);
        ChildHandle->Close (ChildHandle);
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      //
      // call myself
      //
      Quiet = TRUE;
      Status = RemoveRM (ChildHandle, ChildFileName, TRUE);
      if (EFI_ERROR(Status)) {
        goto Done;
      } else {
        Print (L" - [ok]\n");
      }

      FreePool (ChildFileName);
      ChildFileName = NULL;
    }//end of loop
  }

  if (EFI_ERROR(Status)) {
    goto Done;
  }

  //
  // Remove the file
  //
  Print(L"removing %hs\n", FileName);

  Status = FileHandle->Delete(FileHandle);
  if (Status == EFI_WARN_DELETE_FAILURE) {
    Status = EFI_ACCESS_DENIED;
  }  

Done:
  if (ChildFileName) {
    FreePool(ChildFileName);
  }

  if (Info) {
    FreePool(Info);
  }

  return Status;
}
