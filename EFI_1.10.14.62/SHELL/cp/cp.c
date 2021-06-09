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

  cp.c

Abstract:

  EFI shell command "cp"

Revision History

--*/

#include "shell.h"

#define  CP_BLOCK_SIZE  (1024*1024)


//
// Global Variables
//
BOOLEAN                     mCpRecursive;
EFI_DEVICE_PATH_PROTOCOL    *mCpDstDevice;
EFI_DEVICE_PATH_PROTOCOL    *mCpSrcDevice;
BOOLEAN                     mCpReplace;
BOOLEAN                     mCpPrompt;
VOID                        *mCpBuffer;

//
// Function Declarations
//
EFI_STATUS
InitializeCP (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
CpCopy (
  IN EFI_FILE_HANDLE      SrcHandle,
  IN EFI_FILE_HANDLE      DstHandle,
  IN CHAR16               *SrcPath,
  IN CHAR16               *DstPath,
  IN BOOLEAN              DstExists
  );

//
// Entry point
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeCP)
#endif

EFI_STATUS
InitializeCP (
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
  EFI_ACCESS_DENIED       - Read-only files/directories can't be modified
  EFI_OUT_OF_RESOURCES    - Out of memory
  Other value             - Unknown error
  
--*/
{
  EFI_STATUS              Status;
  CHAR16                  **Argv;
  UINTN                   Argc;
  UINTN                   Index;
  UINTN                   DstIndex;
  EFI_LIST_ENTRY          SrcList;
  EFI_LIST_ENTRY          DstList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *SrcArg;
  SHELL_FILE_ARG          *DstArg;
  EFI_FILE_HANDLE         DstHandle;
  BOOLEAN                 DstDir;
  CHAR16                  *SrcFilePath;
  CHAR16                  *DstFilePath;
  UINTN                   SrcDevPathLen;
  UINTN                   DstDevPathLen;
  CHAR16                  *DstDev;
  CHAR16                  *SrcDev;
  CHAR16                  *DstFullName;
  CHAR16                  TempChar;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeCP,
    L"cp",     // command
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
  Status = EFI_SUCCESS;
  Argv = SI->Argv;
  Argc = SI->Argc;
  Index = 0;
  DstIndex = 0;
  InitializeListHead (&SrcList);
  InitializeListHead (&DstList);
  Link = NULL;
  SrcArg = NULL;
  DstArg = NULL;
  DstHandle = NULL;
  DstDir = FALSE;
  SrcFilePath = NULL;
  DstFilePath = NULL;
  SrcDevPathLen = 0;
  DstDevPathLen = 0;
  DstDev = NULL;
  SrcDev = NULL;
  DstFullName = NULL;

  //
  // Global variable initializations
  //
  mCpRecursive = FALSE;
  mCpDstDevice = NULL;
  mCpSrcDevice = NULL;
  mCpReplace = FALSE;
  mCpPrompt = TRUE;
  mCpBuffer = NULL;

  //
  // verify number of arguments
  //
  if (Argc < 2) {
    Print (L"cp: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Parse command line arguments
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"cp: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  //
  // if in script execution, everything is quiet
  //

  if (ShellBatchIsActive()) {
    mCpReplace = TRUE;
    mCpPrompt = FALSE;
  }
  
  //
  // locate the last file argument as dst, while finding -r & -q
  //
  for (Index = 1; Index < Argc; Index ++) {
    if (Argv[Index][0] == '-' &&
      (Argv[Index][1] == 'r' || Argv[Index][1] == 'R')) {
      mCpRecursive = TRUE;
    } else if (Argv[Index][0] == '-' &&
      (Argv[Index][1] == 'q' || Argv[Index][1] == 'Q')) {
      mCpReplace = TRUE;
      mCpPrompt = FALSE;
    } else {
      DstIndex = Index;
    }
  }

  //
  // expand source list but excluding the last file argument
  //

  for (Index = 1; Index < Argc; Index ++) {
    if (Argv[Index][0] == '-' &&
      (Argv[Index][1] == 'r' || Argv[Index][1] == 'R' ||
      Argv[Index][1] == 'q' || Argv[Index][1] == 'Q')) {
      ;
    } else if (Index != DstIndex) {
      Status = ShellFileMetaArg(Argv[Index], &SrcList);
      if (EFI_ERROR(Status) || IsListEmpty(&SrcList)) {
        Print (L"cp: Cannot open %hs - %r\n", Argv[Index], Status);
        goto Done;
      }
    }
  }

  //
  // if no file is expanded to be source
  //
  if (IsListEmpty(&SrcList)) {
    if (DstIndex <= 0) {
      Print (L"cp: Too few arguments\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } else {
      
      //
      // only one file argument is specified,
      // 'current dir' is assumed to be dst
      //
      
      //  
      // using the only file argument as src
      //
      Status = ShellFileMetaArg(Argv[DstIndex], &SrcList);
      if (EFI_ERROR(Status) || IsListEmpty(&SrcList)) {
        Print (L"cp: Cannot open %hs - %r\n", Argv[DstIndex], Status);
        goto Done;
      }
      
      //
      // using the current dir as the dst
      //
      Status = ShellFileMetaArg(L".", &DstList);
      if (EFI_ERROR(Status) || IsListEmpty(&DstList)) {
        Print (L"cp: Cannot open current directory as destination - %r\n",
          Status);
        goto Done;
      }
      if (DstList.Flink->Flink != &DstList) {
        Print (L"cp: Multiple destinations are not allowed\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  } else {
    
    //
    // Some file(s) are expanded as source
    //

    //      
    // The last argument is treated as dst
    //
    Status = ShellFileMetaArg(Argv[DstIndex], &DstList);
    if (EFI_ERROR(Status) || IsListEmpty(&DstList)) {
      Print (L"cp: Cannot open destination - %r\n",
        Status);
      goto Done;
    }
    if (DstList.Flink->Flink != &DstList) {
      Print (L"cp: Multiple destinations are not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  //
  // At this stage, make sure there is at least one valid src
  //
  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
    SrcArg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (SrcArg->Status == EFI_SUCCESS) {
      break;
    }
  }
  if (Link == &SrcList) {
    Print (L"cp: Source file not found\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Determine if the dst should be a file or a directory
  //
  if (SrcList.Flink->Flink != &SrcList) {
    
    //
    // multiple sources
    //
    DstDir = TRUE;
  } else {
    
    //
    // single source, at this point, src list can't be empty
    //
    SrcArg = CR(SrcList.Flink, SHELL_FILE_ARG, Link,
      SHELL_FILE_ARG_SIGNATURE);
    if (SrcArg->Status == EFI_SUCCESS &&
      SrcArg->Info &&
      SrcArg->Info->Attribute & EFI_FILE_DIRECTORY) {
      DstDir = TRUE;
    }
  }

  //
  // Open or create the destination if it is not opened in read/write mode
  //
  if (DstList.Flink->Flink != &DstList) {
    Print (L"cp: Multiple destinations are not allowed\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  DstArg = CR(DstList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  if (DstArg->Status == EFI_SUCCESS &&
    DstArg->OpenMode & EFI_FILE_MODE_READ &&
    DstArg->OpenMode & EFI_FILE_MODE_WRITE) {
      
    //
    //Check if we are copying multiple sources to single existing file
    //
    if (DstArg->Info && !(DstArg->Info->Attribute & EFI_FILE_DIRECTORY)) {
      if (SrcList.Flink->Flink != &SrcList) {
        Print(L"cp: Cannot copy multiple sources to single existing file\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  } else if (DstArg->Status == EFI_NOT_FOUND) {
    
    //
    // check if destination name contains wildcards which is forbidden
    //
    for (Index = 0; Index < StrLen(DstArg->FileName); Index ++) {
      if (DstArg->FileName[Index] == '*' ||
        DstArg->FileName[Index] == '?' ||
        DstArg->FileName[Index] == '[') {
        Print (L"cp: Multiple destinations are not allowed\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }

    //
    // we try to open the dst
    // the parent of the dst should have been opened once ShellFileMetaArg
    // returns EFI_SUCCESS
    //
    Status = DstArg->Parent->Open(DstArg->Parent,
      &DstHandle,
      DstArg->FileName,
      EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE,
      DstDir ? EFI_FILE_DIRECTORY: 0
      );
    if (EFI_ERROR(Status)) {
      Print (L"cp: Cannot create destination %hs - %r\n",
        DstArg->FullName,
        Status);
      goto Done;
    }
  } else if (DstArg->OpenMode & EFI_FILE_MODE_READ) {
    Print (L"cp: Destination is read only or write protected\n");
    Status = EFI_ACCESS_DENIED;
    goto Done;
  } else {
    
    //
    // something error
    //
    Print (L"cp: Cannot open %hs - %r\n",
      DstArg->FullName, DstArg->Status);
    Status = DstArg->Status;
    goto Done;
  }

  //
  // Find out the device path of the destination
  // Note whether or not we create new dst, ParentDevicePath is always valid
  //
  DstDev = StrDuplicate (DstArg->FullName);
  for (Index = 0; Index < StrLen(DstDev); Index ++) {
    if (DstDev[Index] == ':') {
      break;
    }
  }
  DstDev[Index] = 0;

  mCpDstDevice = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (DstDev);

  //
  // Make sure no source is being copied onto itself
  //
  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
    SrcArg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    //
    // Find out the device path of the source
    //
    if (SrcDev) {
      FreePool(SrcDev);
    }

    SrcDev = StrDuplicate (SrcArg->FullName);
    for (Index = 0; Index < StrLen(SrcDev); Index ++) {
      if (SrcDev[Index] == ':') {
        break;
      }
    }
    SrcDev[Index] = 0;

    mCpSrcDevice = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (SrcDev);

    for(SrcFilePath = SrcArg->FullName; *SrcFilePath && *SrcFilePath!=':';
      SrcFilePath++) {
      ;
    }
    for(DstFilePath = DstArg->FullName; *DstFilePath && *DstFilePath!=':';
      DstFilePath++) {
      ;
    }

    //
    // if target is a directory and source is a file,
    // we need to compose a full name
    //
    if (
        SrcArg->Status == EFI_SUCCESS &&
        !(SrcArg->Info->Attribute & EFI_FILE_DIRECTORY)
        &&
        (
          DstArg->Status == EFI_SUCCESS &&
          (DstArg->Info->Attribute & EFI_FILE_DIRECTORY)
          ||
          DstDir && DstHandle
        )
      ) {
      if (DstFullName) {
        FreePool(DstFullName);
      }

      DstFullName = AllocatePool
        (StrSize(DstArg->FullName) + 2 + StrSize(SrcArg->FileName) + 2);
      if (!DstFullName) {
        Print (L"Cp: Out of resources\n");
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      StrCpy(DstFullName, DstArg->FullName);
      if (DstFullName[StrLen(DstFullName) - 1] != '\\') {
        StrCat(DstFullName, L"\\");
      }
      StrCat(DstFullName, SrcArg->FileName);

      for(DstFilePath = DstFullName; *DstFilePath && *DstFilePath!=':';
        DstFilePath++) {
        ;
      }
    }

    if (StriCmp(SrcFilePath, DstFilePath) == 0) {
      SrcDevPathLen = DevicePathSize(mCpSrcDevice);
      DstDevPathLen = DevicePathSize(mCpDstDevice);
      if (SrcDevPathLen == DstDevPathLen) {
        if (
          CompareMem(mCpSrcDevice,
                 mCpDstDevice,
                 SrcDevPathLen)
          == 0
          ) {
          Print(L"cp: Cannot copy %hs to itself\n",
            SrcArg->FullName);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
      }
    }
  }

  //
  // Make sure no source is being copied onto its sub if -r is specified
  //
  if (mCpRecursive) {
    for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
      SrcArg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

      //
      // Find out the device path of the source
      //
      if (SrcDev) {
        FreePool(SrcDev);
      }

      SrcDev = StrDuplicate (SrcArg->FullName);
      if (!SrcDev) {
        Print (L"Cp: Out of resources\n");
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      for (Index = 0; Index < StrLen(SrcDev); Index ++) {
        if (SrcDev[Index] == ':') {
          break;
        }
      }
      SrcDev[Index] = 0;

      mCpSrcDevice = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (SrcDev);

      for(SrcFilePath = SrcArg->FullName; *SrcFilePath && *SrcFilePath!=':';
        SrcFilePath++) {
        ;
      }
      for(DstFilePath = DstArg->FullName; *DstFilePath && *DstFilePath!=':';
        DstFilePath++) {
        ;
      }

      if (DstFullName) {
        FreePool(DstFullName);
      }
      DstFullName = StrDuplicate(DstFilePath);
      if (!DstFullName) {
        Print (L"Cp: Out of resources\n");
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      SrcDevPathLen = DevicePathSize(mCpSrcDevice);
      DstDevPathLen = DevicePathSize(mCpDstDevice);

      if (SrcDevPathLen == DstDevPathLen) {
        if (
          CompareMem(mCpSrcDevice,
                 mCpDstDevice,
                 SrcDevPathLen) == 0
           ) {
          for (Index = StrLen(DstFullName); Index > 0; Index --) {
            TempChar = DstFullName[Index];
            DstFullName[Index] = 0;
            if (StriCmp(SrcFilePath, DstFullName) == 0) {
              if (SrcFilePath[Index - 1] == '\\' || TempChar == '\\' || TempChar == 0) {
                Print(L"cp: Cannot copy %hs to its subdirectory\n",
                  SrcArg->FullName);
                Status = EFI_INVALID_PARAMETER;
                goto Done;
              }
            }
          }
        }
      }
    }
  }

  //
  // ready to copy
  // allocate pool for following use
  //
  mCpBuffer = AllocatePool (CP_BLOCK_SIZE);
  if (!mCpBuffer) {
    Print(L"cp: Out of memory\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  //
  // for each src, call CpCopy to copy it
  //
  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
    SrcArg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    if (SrcArg->Status != EFI_SUCCESS || !SrcArg->Handle) {
      Print (L"cp: Cannot open %hs - %r\n",
        SrcArg->FullName, SrcArg->Status);
      Status = SrcArg->Status;
      goto Done;
    } else {
      
      //
      //CpCopy requires SrcHandle and DstHandle be valid
      //
      
      if (DstHandle) {
        
        //
        // we created a new file/dir as dst
        //
        Status = CpCopy (SrcArg->Handle, DstHandle,
                SrcArg->FullName,
                DstArg->FullName,
                FALSE);
        if (EFI_ERROR(Status)) {
          
          //
          // we don't care the return status
          //
          DstHandle -> Delete(DstHandle);
          DstHandle = NULL;
          goto Done;
        }
      } else {
        
        //
        // dst already exists
        //
        Status = CpCopy (SrcArg->Handle, DstArg->Handle,
                SrcArg->FullName,
                DstArg->FullName,
                TRUE);
        if (EFI_ERROR(Status)) {
          
          //
          // Error message is supposed to be displayed in CpCopy
          //
          goto Done;
        }
      }
    }
  }

Done:
  if (DstDev) {
    FreePool(DstDev);
  }

  if (SrcDev) {
    FreePool(SrcDev);
  }

  if (DstFullName) {
    FreePool(DstFullName);
  }

  if (DstHandle) {
    DstHandle->Close(DstHandle);
  }

  if (mCpBuffer) {
    FreePool(mCpBuffer);
  }

  ShellFreeFileList(&SrcList);
  ShellFreeFileList(&DstList);

  return Status;
}


EFI_STATUS
CpCopy (
  IN EFI_FILE_HANDLE      SrcHandle,
  IN EFI_FILE_HANDLE      DstHandle,
  IN CHAR16               *SrcPath,
  IN CHAR16               *DstPath,
  IN BOOLEAN              DstExists
  ) 
/*++

Routine Description:

  Performs copy operation. 
  Four possibilities are treated separately:
  File->File, File->Directory, Directory->File, Directory->Directory

Arguments:

  SrcHandle       Handle of source file or directory 
  DstHandle       Handle of destination file or directory
  SrcPath         Path of source file or directory
  DstPath         Path of destination file or directory
  DstExists       Indicates if destination already exists

Returns:

  EFI_SUCCESS     The function completed successfully.
  Other value     The function failed due to some reason.
  
--*/
{
  UINTN                   Index;
  EFI_STATUS              Status;
  EFI_FILE_INFO           *SrcInfo;
  EFI_FILE_INFO           *DstInfo;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *DstVol;
  EFI_FILE_HANDLE         DstRootDir;
  EFI_FILE_SYSTEM_INFO    *DstFsInfo;
  CHAR16                  InputString[10];//large enough
  UINTN                   BufSize;
  UINTN                   WriteSize;
  EFI_FILE_HANDLE         SubSrcHandle;
  EFI_FILE_HANDLE         SubDstHandle;
  CHAR16                  *SubSrcPath;
  CHAR16                  *SubDstPath;
  BOOLEAN                 SubDstExists;
  EFI_FILE_INFO           *InfoBuffer;
  UINTN                   InfoBufSize;
  EFI_FILE_INFO           *SubSrcInfo;
  UINT64                  FilePhysicalSize;

  //
  // Local variable initializations
  //
  Status = EFI_SUCCESS;
  
  Index = 0;
  SrcInfo = DstInfo = NULL;
  DstVol = NULL;
  DstRootDir = NULL;
  DstFsInfo = NULL;
  InputString[0] = 0;
  BufSize = 0;
  WriteSize = 0;
  SubSrcHandle = NULL;
  SubDstHandle = NULL;
  SubSrcPath = NULL;
  SubDstPath = NULL;
  SubDstExists = FALSE;
  InfoBuffer = NULL;
  InfoBufSize = 0;
  SubSrcInfo = NULL;
  FilePhysicalSize = 0;

  //
  // Get info from both files
  //
  SrcInfo = LibFileInfo(SrcHandle);
  if (!SrcInfo) {
    Status = EFI_OUT_OF_RESOURCES;
    Print (L"cp: Access %hs error\n", SrcPath);
    goto CopyDone;
  }

  DstInfo = LibFileInfo (DstHandle);
  if (!DstInfo) {
    Status = EFI_OUT_OF_RESOURCES;
    Print (L"cp: Access %hs error\n", DstPath);
    goto CopyDone;
  }

  //
  // file -> file
  //
  if (!(SrcInfo->Attribute & EFI_FILE_DIRECTORY) &&
    !(DstInfo->Attribute & EFI_FILE_DIRECTORY)) {
      
    //
    // if already exists, prompt
    //
    if (DstExists && mCpPrompt) {
      InputString[0] = 0;
      Print(L"Overwrite %s? (Yes/No/All/Cancel):",DstPath);

      Input(L"", InputString, 2);
      Print(L"\n");

      Index = 0;
      while (Index < 5) {
        if (InputString[0] == 'Y' || InputString[0] == 'y') {
          mCpPrompt = TRUE;
          mCpReplace = TRUE;
          break;
        }
        else if (InputString[0] == 'N' || InputString[0] == 'n') {
          mCpPrompt = TRUE;
          mCpReplace = FALSE;
          break;
        }
        else if (InputString[0] == 'A' || InputString[0] == 'a') {
          mCpPrompt = FALSE;
          mCpReplace = TRUE;
          break;
        }
        else if (InputString[0] == 'C' || InputString[0] == 'c') {
          mCpPrompt = FALSE;
          mCpReplace = FALSE;
          break;
        }

        InputString[0] = 0;
        Print(L"Overwrite %s? (Yes/No/All/Cancel):", DstPath);
        Input(L"", InputString, 2);
        Print(L"\n");
        Index ++;
      }
    }

    if (!mCpReplace && DstExists) {
      goto CopyDone;
    }

    //
    // set both positions to beginning
    //
    Print(L"copying %s -> %s\n", SrcPath, DstPath);

    Status = SrcHandle->SetPosition(SrcHandle, 0);
    if (EFI_ERROR(Status)) {
      Print(L"cp: Set %hs pos error - %r\n",
        SrcPath, Status);
      goto CopyDone;
    }

    Status = DstHandle->SetPosition(DstHandle, 0);
    if (EFI_ERROR(Status)) {
      Print(L"cp: Set %hs pos error - %r\n",
        DstPath, Status);
      goto CopyDone;
    }

    //
    // Set destination file size to the same size as source file
    //
    DstInfo->FileSize = SrcInfo->FileSize;
    DstInfo->PhysicalSize = SrcInfo->PhysicalSize;

    Status = DstHandle->SetInfo(
            DstHandle,
            &GenericFileInfo,
            (UINTN)DstInfo->Size,
            DstInfo
            );
    if (EFI_ERROR(Status)) {
      Print(L"cp: Access %hs error - %r\n", DstPath, Status);
      goto CopyDone;
    }

    DstInfo->Attribute = EFI_FILE_ARCHIVE;
    //
    // Copy it
    //
    for (; ;) {
      BufSize = CP_BLOCK_SIZE;

      Status = SrcHandle->Read (SrcHandle, &BufSize, mCpBuffer);
      if (EFI_ERROR(Status)) {
        Print(L"- read error - %r\n", Status);
        goto CopyDone;
      }

      //
      // end of file
      //
      if (!BufSize) {
        break;
      }

      WriteSize = BufSize;

      Status = DstHandle->Write (DstHandle, &WriteSize, mCpBuffer);
      if (EFI_ERROR(Status)) {
        Print(L"- write error - %r\n", Status);
        goto CopyDone;
      }

      if (WriteSize != BufSize) {
        Status = EFI_OUT_OF_RESOURCES;
        Print(L"- write error - disk full\n");
        goto CopyDone;
      }
    }

    //
    // Set destination modification time as the same as the source
    //
        
    FreePool (DstInfo);
    DstInfo = LibFileInfo (DstHandle);
    if (!DstInfo) {
      Status = EFI_OUT_OF_RESOURCES;
      Print (L"cp: Cannot access '%hs'\n", DstPath);
      goto CopyDone;
    }
    DstInfo->ModificationTime = SrcInfo->ModificationTime;
    Status = DstHandle->SetInfo(
            DstHandle,
            &GenericFileInfo,
            (UINTN)DstInfo->Size,
            DstInfo
            );
    if (EFI_ERROR(Status)) {
      Print(L"cp: Cannot access '%hs' - %r\n", DstPath, Status);
      goto CopyDone;
    }

    Status = DstHandle->Flush (DstHandle);

    if (EFI_ERROR(Status)) {
      Print(L" - write error - %r\n",Status);
      goto CopyDone;
    }
    else {
      Print(L" - [ok]\n");
    }
  }
  //
  // file -> dir
  //
  else if (!(SrcInfo->Attribute & EFI_FILE_DIRECTORY) &&
    (DstInfo->Attribute & EFI_FILE_DIRECTORY)) {
      
    //
    // see if the sub destination already exists
    //
    SubDstExists = TRUE;
    Status = DstHandle->Open(DstHandle, &SubDstHandle, SrcInfo->FileName,
          EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE, 0);
    if (EFI_ERROR(Status)) {
      SubDstExists = FALSE;
      Status = DstHandle->Open(DstHandle, &SubDstHandle,
          SrcInfo->FileName,
          EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE, 0);
      if (EFI_ERROR(Status)) {
        Print(L"cp: Cannot open/create %hs under %hs\n", SrcInfo->FileName,
            DstPath);
        SubDstHandle = NULL;
        goto CopyDone;
      }
    }

    //
    // Compose new path for sub dst
    //
    SubDstPath = AllocatePool(StrSize(DstPath) + 1 +
            StrSize(SrcInfo->FileName) + 2);
    if (!SubDstPath) {
      Status = EFI_OUT_OF_RESOURCES;
      Print(L"cp: Out of memory\n");
      goto CopyDone;
    }
    StrCpy(SubDstPath, DstPath);
    if (SubDstPath[StrLen(SubDstPath)-1] != '\\') {
      StrCat(SubDstPath, L"\\");
    }
    StrCat(SubDstPath, SrcInfo->FileName);

    Status = CpCopy (SrcHandle, SubDstHandle,
            SrcPath, SubDstPath,
            SubDstExists);
    if (EFI_ERROR(Status)) {
      if (!SubDstExists) {
        //
        // we don't care return status
        //
        SubDstHandle -> Delete(SubDstHandle);
        SubDstHandle = NULL;
      }

      goto CopyDone;
    }
  }

  //
  // dir -> file
  //
  else if ((SrcInfo->Attribute & EFI_FILE_DIRECTORY) &&
    !(DstInfo->Attribute & EFI_FILE_DIRECTORY)) {
    Print(L"cp: Cannot copy from %hs to %hs\n",
      SrcPath, DstPath);
    Status = EFI_INVALID_PARAMETER;
    goto CopyDone;
  }

  //
  // dir -> dir
  //
  else if ((SrcInfo->Attribute & EFI_FILE_DIRECTORY) &&
    (DstInfo->Attribute & EFI_FILE_DIRECTORY)) {
    if (!mCpRecursive) {
      Print (L"cp: Cannot copy directory %hs without '-r'\n",
        SrcPath);
      Status = EFI_SUCCESS;
      goto CopyDone;
    }

    //
    // Set position to 0 in source
    //
    Status = SrcHandle->SetPosition(SrcHandle, 0);
    if (EFI_ERROR(Status)) {
      Print(L"cp: Access %hs error - %r\n",
        SrcPath, Status);
      goto CopyDone;
    }

    //
    // loop
    //
    for (;;) {
      
      //
      // reuse Info Buffer to store entries of source dir
      //
      InfoBufSize = SIZE_OF_EFI_FILE_INFO + 1024;            
      
      if (!InfoBuffer) {
        InfoBuffer = AllocatePool(InfoBufSize);
        if (!InfoBuffer) {
          Status = EFI_OUT_OF_RESOURCES;
          Print (L"cp: Out of memory\n");
          goto CopyDone;
        }
      }

      Status = SrcHandle->Read(SrcHandle, &InfoBufSize, InfoBuffer);
      if (EFI_ERROR(Status)) {
        Print(L"cp: Read %hs error - %r\n",
          SrcPath, Status);
        goto CopyDone;
      }
      
      //
      // no more entries
      //
      if (InfoBufSize == 0) {
        break;
      }

      //
      // skip
      //
      if (StriCmp(InfoBuffer->FileName, L".") == 0 ||
        StriCmp(InfoBuffer->FileName, L"..") == 0) {
        continue;
      }

      //
      // compose SubSrcPath
      //
      if (SubSrcPath) {
        FreePool(SubSrcPath);
      }
      SubSrcPath = AllocatePool(StrSize(SrcPath) + 1 +
              StrSize(InfoBuffer->FileName) + 2);

      if (!SubSrcPath) {
        Status = EFI_OUT_OF_RESOURCES;
        Print(L"cp: Out of memory\n");
        goto CopyDone;
      }
      StrCpy(SubSrcPath, SrcPath);
      if (SubSrcPath[StrLen(SubSrcPath)-1] != '\\') {
        StrCat(SubSrcPath, L"\\");
      }
      StrCat(SubSrcPath, InfoBuffer->FileName);

      //
      // compose SubDstPath
      //
      if (SubDstPath) {
        FreePool(SubDstPath);
      }
      SubDstPath = AllocatePool(StrSize(DstPath) + 1 +
              StrSize(InfoBuffer->FileName) + 2);

      if (!SubDstPath) {
        Status = EFI_OUT_OF_RESOURCES;
        Print(L"cp: Out of memory\n");
        goto CopyDone;
      }
      StrCpy(SubDstPath, DstPath);
      if (SubDstPath[StrLen(SubDstPath)-1] != '\\') {
        StrCat(SubDstPath, L"\\");
      }
      StrCat(SubDstPath, InfoBuffer->FileName);

      //
      // Open SubSrcHandle
      //
      Status = SrcHandle->Open(SrcHandle, &SubSrcHandle,
        InfoBuffer->FileName, EFI_FILE_MODE_READ, 0);
      if (EFI_ERROR(Status)) {
        Print(L"cp: Cannot open %hs - %r\n",
          SubSrcPath, Status);
        SubSrcHandle = NULL;
        goto CopyDone;
      }

      //
      // Get info of sub source
      //
      SubSrcInfo = LibFileInfo(SubSrcHandle);
      if (!SubSrcInfo) {
        Status = EFI_OUT_OF_RESOURCES;
        Print (L"cp: Access %hs error\n", SubDstPath);
        goto CopyDone;
      }

      //
      // Copy sub source to sub destination
      //
      if (!mCpRecursive && (SubSrcInfo->Attribute & EFI_FILE_DIRECTORY)) {
        ;
      }
      else {
        //
        // open sub destination, see if it already exists
        //
        SubDstExists = TRUE;
        Status = DstHandle->Open(DstHandle, &SubDstHandle,
              InfoBuffer->FileName,
              EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE, 0);
        if (EFI_ERROR(Status)) {
          SubDstExists = FALSE;

          if (SubSrcInfo->Attribute & EFI_FILE_DIRECTORY) {
            Print (L"Making dir %hs\n", SubDstPath);
          }

          Status = DstHandle->Open(
              DstHandle, &SubDstHandle,
              InfoBuffer->FileName,
              EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE,
              (SubSrcInfo->Attribute & EFI_FILE_DIRECTORY)?
              EFI_FILE_DIRECTORY:0
              );
          if (EFI_ERROR(Status)) {
            Print(L"cp: Cannot open/create %hs under %hs - %r\n",
              InfoBuffer->FileName, DstPath, Status);
            SubDstHandle = NULL;
            goto CopyDone;
          }
        }
        Status = CpCopy(SubSrcHandle, SubDstHandle,
              SubSrcPath, SubDstPath,
              SubDstExists);
        if (EFI_ERROR(Status)) {
          if (!SubDstExists) {
            //
            // we don't care return status
            //
            SubDstHandle -> Delete(SubDstHandle);
            SubDstHandle = NULL;
          }

          goto CopyDone;
        }
        else {
          if (SubSrcHandle) {
            SubSrcHandle->Close(SubSrcHandle);
            SubSrcHandle = NULL;
          }

          if (SubDstHandle) {
            SubDstHandle->Close(SubDstHandle);
            SubDstHandle = NULL;
          }
        }
      }
      FreePool (SubSrcInfo);
      SubSrcInfo = NULL;
    }
  }

CopyDone:
  if (SrcInfo) {
    FreePool (SrcInfo);
  }
  if (DstInfo) {
    FreePool (DstInfo);
  }
  if (DstFsInfo) {
    FreePool (DstFsInfo);
  }
  if (SubSrcPath) {
    FreePool (SubSrcPath);
  }
  if (SubDstPath) {
    FreePool (SubDstPath);
  }
  if (InfoBuffer) {
    FreePool (InfoBuffer);
  }

  if (SubDstHandle) {
    SubDstHandle->Close(SubDstHandle);
  }

  if (SubSrcHandle) {
    SubSrcHandle->Close(SubSrcHandle);
  }

  if (SubSrcInfo) {
    FreePool (SubSrcInfo);
  }

  return Status;
}
