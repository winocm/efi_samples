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

  mv.c

Abstract:

  EFI Shell command 'mv'

Revision History

--*/

#include "shell.h"

//
// Global variables
//

//
// Function Declarations
//
EFI_STATUS
InitializeMv (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
MvFile (
  IN SHELL_FILE_ARG       *Arg,
  IN CHAR16               *NewName
  );

//
// Entry point
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeMv)
#endif

EFI_STATUS
InitializeMv (
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
  EFI_UNSUPPORTED         - Unsupported function (mv between different file systems)
  EFI_NO_MAPPING          - Device not mapped
  EFI_OUT_OF_RESOURCES    - Out of memory or file handle
  Other value             - Unknown error
  
--*/
{
  EFI_STATUS                  Status;
  CHAR16                      **Argv;
  UINTN                       Argc;
  UINTN                       Index;
  UINTN                       Index1;
  EFI_LIST_ENTRY              SrcList;
  EFI_LIST_ENTRY              DstList;
  EFI_LIST_ENTRY              *Link;
  SHELL_FILE_ARG              *Arg;
  SHELL_FILE_ARG              *DstArg;
  CHAR16                      *DestName;
  CHAR16                      *FullDestName;
  BOOLEAN                     DestWild;
  CHAR16                      *PtrOne;
  CHAR16                      *PtrTwo;
  UINTN                       BufferSize;
  CHAR16                      *CurPath;
  CHAR16                      *CurDir;
  BOOLEAN                     DstExists;
  CHAR16                      DefaultDir[2];
  EFI_DEVICE_PATH_PROTOCOL    *CurDevice;
  CHAR16                      *CurDev;
  EFI_DEVICE_PATH_PROTOCOL    *TargetDevice;
  CHAR16                      *TargetDev;
  CHAR16                      *TargetPath;
  EFI_DEVICE_PATH_PROTOCOL    *DstDevice;
  EFI_DEVICE_PATH_PROTOCOL    *SrcDevice;
  CHAR16                      *DstDev;
  CHAR16                      *SrcDev;
  CHAR16                      *CurFullName;
  CHAR16                      TempChar;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeMv,
    L"mv",     // command
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

  Index = 0; Index1 = 0;
  InitializeListHead (&SrcList);
  InitializeListHead (&DstList);
  Link = NULL;
  Arg = NULL;
  DstArg = NULL;
  DestName = NULL;
  FullDestName = NULL;
  DestWild = FALSE;
  PtrOne = NULL;
  BufferSize = 0;
  CurPath = CurDir = NULL;
  DstExists = FALSE;
  DefaultDir[0] = '.';
  DefaultDir[1] = 0;
  CurDevice = NULL;
  CurDev = NULL;
  TargetDevice = NULL;
  TargetDev = NULL;
  TargetPath = NULL;
  PtrTwo = NULL;
  DstDevice = SrcDevice = NULL;
  DstDev = SrcDev = NULL;
  CurFullName = NULL;
  TempChar = 0;

  //
  // Parse command line arguments
  //

  if (Argc < 2) {
    Print (L"mv: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"mv: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  //
  // If the last arg has wild cards then report an error
  //
  DestWild = FALSE;

  if (Argc == 2) {

    //
    // Destination is not specified in command line
    //
    DestName = DefaultDir;
  } else {
    DestName = Argv[Argc-1];
    if (StrLen(DestName) == 0) {
      Print (L"mv: Destination not specified\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  for (PtrOne = DestName; *PtrOne; PtrOne += 1) {
    if (*PtrOne == '*' || *PtrOne == '?' || *PtrOne == '[' || *PtrOne == ']') {
      DestWild = TRUE;
    }
  }

  if (DestWild) {
    Print (L"mv: Destination name cannot contain wildcards\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // find out the current directory
  //
  CurDir = ShellCurDir(NULL);
  if (CurDir) {
    CurDev = StrDuplicate (CurDir);
    for (Index = 0; Index < StrLen(CurDev); Index ++) {
      if (CurDev[Index] == ':') {
        break;
      }
    }
    CurDev[Index] = 0;
  
    CurDevice = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (CurDev);
  }
  
  //
  // Verify destination and sources contain the same device mapping
  //

  //
  // Get Dst device
  //
  DstDev = StrDuplicate (DestName);

  for (Index = 0; Index < StrLen(DstDev) ; Index ++) {
    if (DstDev[Index] == ':') {
      break;
    }
  }

  if (Index == StrLen(DstDev)) {
    if (CurDevice) {
      DstDevice = CurDevice;
    } else {
      Print (L"mv: Cannot get current directory");
      Status = EFI_ABORTED;
      goto Done;
    }    
  } else {
    DstDev[Index] = 0;
    DstDevice = (EFI_DEVICE_PATH_PROTOCOL*)ShellGetMap (DstDev);
    if (!DstDevice) {
      Print (L"mv: Destination not mapped\n");
      Status = EFI_NO_MAPPING;
      goto Done;
    }
  }

  //
  // Get Source devices and compare them with destination device
  //
  for (Index1 = 1; Index1 < Argc-1 || Index1 == 1; Index1 ++) {
    if (SrcDev) {
      FreePool(SrcDev);
    }

    SrcDev = StrDuplicate(Argv[Index1]);

    for (Index = 0; Index < StrLen(SrcDev) ; Index++) {
      if (SrcDev[Index] == ':') {
        break;
      }
    }

    if (Index == StrLen(SrcDev)) {
      if (CurDevice) {
        SrcDevice = CurDevice;
      } else {
        Print (L"mv: Cannot get current directory\n");
        Status = EFI_ABORTED;
        goto Done;
      }      
    } else {
      SrcDev[Index] = 0;

      SrcDevice = (EFI_DEVICE_PATH_PROTOCOL*)ShellGetMap (SrcDev);
      if (!SrcDevice) {
        Print (L"mv: Cannot find device mapping for %hs\n", Argv[Index1]);
        Status = EFI_NO_MAPPING;
        goto Done;
      }
    }

    //
    // compare source device with dst device
    //
    if (DevicePathSize(SrcDevice) != DevicePathSize(DstDevice)) {
      Print (L"mv: Source and destination should be on the same file system\n");
      Status = EFI_UNSUPPORTED;
      goto Done;
    } else if (CompareMem(SrcDevice,
            DstDevice,
            DevicePathSize(SrcDevice)
            ) != 0
        ) {
      Print (L"mv: Source and destination should be on the same file system\n");
      Status = EFI_UNSUPPORTED;
      goto Done;
    }
  }

  //
  // at this stage, sources and destination are on the same file system,
  // we let cur dir point to the cur dir of destination device
  //
  if (DstDevice != CurDevice) {
    CurDevice = DstDevice;

    if (CurDev) {
      FreePool(CurDev);
    }
    CurDev = StrDuplicate(DstDev);

    if (CurDir) {
      FreePool(CurDir);
    }
    CurDir = ShellCurDir(CurDev);

    if (!CurDir) {
      Print (L"mv: Cannot get current directory\n", CurDev);
      Status = EFI_ABORTED;
      goto Done;
    }
  }

  //
  // Check to see if the dst is an existing dir
  //

  DstExists = FALSE;
  Status = ShellFileMetaArg (DestName, &DstList);

  if (Status == EFI_SUCCESS && !IsListEmpty(&DstList)) {
    DstArg = CR(DstList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (DstArg->Status == EFI_SUCCESS && DstArg->Handle &&
      DstArg->Info->Attribute & EFI_FILE_DIRECTORY) {
      DstExists = TRUE;
    }
  }

  //
  // we no longer need DstList
  //
  ShellFreeFileList (&DstList);

  //
  // Move DestName after ':' if there is ':'
  //
  PtrOne = DestName;
  for (;*DestName && *DestName != ':'; DestName ++);
  if (*DestName) {
    DestName ++;
  } else {
    DestName = PtrOne;
  }

  //
  //Strip off the trailing '\'s of DestName
  //
  if (StrLen(DestName) > 0) {
    for (PtrOne = &DestName[StrLen(DestName)-1]; PtrOne > &DestName[0]; PtrOne --) {
      if (*PtrOne == '\\') {
        *PtrOne = 0;
      }
      else {
        break;
      }
    }
  }

  if (StrLen(DestName) == 0) {
    DestName = DefaultDir;
  }

  //
  // Expand each source arg
  //
  for (Index = 1; Index < Argc-1 || Index == 1; Index += 1) {
    Status = ShellFileMetaArg (Argv[Index], &SrcList);
    if (EFI_ERROR(Status) || IsListEmpty(&SrcList)) {
      if (IsListEmpty(&SrcList)) {
        Status = EFI_NOT_FOUND;
      }
      Print(L"mv: Cannot open %hs - %r\n", Argv[Index], Status);
      goto Done;
    }
  }


  //
  // Check to see if we are moving root directory,
  // or current dir's ancestor
  //
  for (Link=SrcList.Flink; Link != &SrcList; Link=Link->Flink) {

    Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (Arg->Status != EFI_SUCCESS) {
      continue;
    }

    if (Arg->FileName &&
      (StriCmp(Arg->FileName, L".") == 0 || StriCmp(Arg->FileName, L"..")==0)) {
      Print(L"mv: Cannot move current directory or its ancestor\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    //
    // Check to see if we are moving a root dir
    //
    for (PtrTwo = Arg->FullName; (*PtrTwo); PtrTwo++) {
      if ((*PtrTwo) == ':') {
        break;
      }
    }

    if (*PtrTwo && *(PtrTwo + 1) == '\\' && *(PtrTwo + 2) == 0) {
      Print (L"mv: Cannot move root directory\n");
      Status = EFI_INVALID_PARAMETER;
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

    TargetDevice = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (TargetDev);

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
      Print (L"mv: Out of resources\n");
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    if (DevicePathSize(CurDevice) == DevicePathSize(TargetDevice)) {
      if (
        CompareMem(CurDevice,
               TargetDevice,
               DevicePathSize(CurDevice)
               ) == 0
         ) {        
        for (Index = StrLen(CurFullName); Index > 0; Index --) {
          TempChar = CurFullName[Index];
          CurFullName[Index] = 0;
          if (StriCmp(TargetPath, CurFullName) == 0) {
            if (TargetPath[Index - 1] == '\\' || TempChar == '\\' || TempChar == 0) {
              Print(L"mv: Cannot move current directory or its ancestor\n");
              Status = EFI_INVALID_PARAMETER;
              goto Done;
            }
          }
        }
      }
    }
  }

  //
  // Perform move
  //

  //
  // if dest is a relative path, we compose the full path
  //
  if (DestName[0] != '\\') {
    //
    //Strip off the trailing '\\' of curdir
    //
    if (StrLen(CurDir) > 0 && CurDir[StrLen(CurDir) - 1] == '\\') {
      CurDir[StrLen(CurDir) - 1] = 0;
    }

    //
    //skip the device mapping name
    //
    for (CurPath = CurDir; (*CurPath) && (*CurPath) != '\\'; CurPath ++) {
      ;
    }

    BufferSize = StrSize(CurPath) + StrSize(DestName) + EFI_FILE_STRING_SIZE;
    FullDestName = AllocatePool (BufferSize);

    if (!FullDestName) {
      Print (L"mv: Out of memory\n");
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    if (!DstExists) {

      //
      // Destination does not exist
      //

      ShellFreeFileList(&SrcList);
      for (Index = 1; Index < Argc-1 || Index == 1; Index += 1) {
        Status = ShellFileMetaArg (Argv[Index], &SrcList);
        if (EFI_ERROR(Status) || IsListEmpty(&SrcList)) {
          if (IsListEmpty (&SrcList)) {
            Status = EFI_NOT_FOUND;
          }
          Print(L"mv: Cannot open %hs - %r\n", Argv[Index], Status);
          continue;
        }

        for (Link=SrcList.Flink; Link != &SrcList; Link=Link->Flink) {
          Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

          SPrint(FullDestName, BufferSize, L"%s\\%s", CurPath, DestName);
          Status = MvFile (Arg, FullDestName);
        }
        ShellFreeFileList(&SrcList);
      }
    } else {

      //
      // Destination exists
      //

      ShellFreeFileList(&SrcList);
      for (Index = 1; Index < Argc-1 || Index == 1; Index += 1) {
        Status = ShellFileMetaArg (Argv[Index], &SrcList);
        if (EFI_ERROR(Status) || IsListEmpty(&SrcList)) {
          if (IsListEmpty(&SrcList)) {
            Status = EFI_NOT_FOUND;
          }
          Print(L"mv: Cannot open %hs - %r\n", Argv[Index], Status);
          continue;
        }

        for (Link=SrcList.Flink; Link != &SrcList; Link=Link->Flink) {
          Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

          SPrint (FullDestName, BufferSize, L"%s\\%s\\%s",
            CurPath, DestName, Arg->FileName);
          Status = MvFile (Arg, FullDestName);
        }
        ShellFreeFileList(&SrcList);
      }
    }
  }
  //
  // destination is an absolute path
  //
  else
  {
    BufferSize = StrSize(DestName) + EFI_FILE_STRING_SIZE;
    FullDestName = AllocatePool (BufferSize);
    if (!FullDestName) {
      Print (L"mv: Out of memory\n");
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    if (!DstExists) {

      //
      // Destination does not exist
      //

      ShellFreeFileList(&SrcList);
      for (Index = 1; Index < Argc-1 || Index == 1; Index += 1) {
        Status = ShellFileMetaArg (Argv[Index], &SrcList);
        if (EFI_ERROR(Status) || IsListEmpty(&SrcList)) {
          if (IsListEmpty(&SrcList)) {
            Status = EFI_NOT_FOUND;
          }
          Print(L"mv: Cannot open %hs - %r\n", Argv[Index], Status);
          continue;
        }

        for (Link=SrcList.Flink; Link != &SrcList; Link=Link->Flink) {
          Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

          Status = MvFile (Arg, DestName);
        }
        ShellFreeFileList(&SrcList);
      }
    } else {

      //
      // Destination exists
      //

      ShellFreeFileList(&SrcList);
      for (Index = 1; Index < Argc-1 || Index == 1; Index += 1) {
        Status = ShellFileMetaArg (Argv[Index], &SrcList);
        if (Status != EFI_SUCCESS || IsListEmpty(&SrcList)) {
          if (IsListEmpty(&SrcList)) {
            Status = EFI_NOT_FOUND;
          }
          Print(L"mv: Cannot open %hs - %r\n", Argv[Index], Status);
          continue;
        }

        for (Link=SrcList.Flink; Link != &SrcList; Link=Link->Flink) {
          Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
          if (StrLen(DestName)>0 && DestName[StrLen(DestName)-1] == '\\') {
            SPrint (FullDestName, BufferSize, L"%s%s", DestName, Arg->FileName);
          }
          else {
            SPrint (FullDestName, BufferSize, L"%s\\%s", DestName, Arg->FileName);
          }

          Status = MvFile (Arg, FullDestName);
        }
        ShellFreeFileList(&SrcList);
      }
    }
  }

Done:
  if (CurDir) {
    FreePool(CurDir);
  }

  if (SrcDev) {
    FreePool(SrcDev);
  }

  if (DstDev) {
    FreePool(DstDev);
  }

  if (CurDev) {
    FreePool(CurDev);
  }

  if (TargetDev) {
    FreePool(TargetDev);
  }

  if (FullDestName) {
    FreePool(FullDestName);
  }
  
  if (CurFullName) {
    FreePool (CurFullName);
  }  

  ShellFreeFileList (&SrcList);

  return Status;
}


EFI_STATUS
MvFile (
  IN SHELL_FILE_ARG           *Arg,
  IN CHAR16                   *NewName
  )
/*++

Routine Description:

  Move a file to new name

Arguments:

  Arg         The file to be moved.
  NewName     The newname.

Returns:

  

--*/
{
  EFI_STATUS                  Status;
  EFI_FILE_INFO               *Info;
  UINTN                       NameSize;

  Status = Arg->Status;
  if (!EFI_ERROR(Status)) {
    NameSize = StrSize(NewName);
    Info = AllocatePool (SIZE_OF_EFI_FILE_INFO + NameSize);
    Status = EFI_OUT_OF_RESOURCES;

    if (Info) {
      CopyMem (Info, Arg->Info, SIZE_OF_EFI_FILE_INFO);
      CopyMem (Info->FileName, NewName, NameSize);
      Info->Size = SIZE_OF_EFI_FILE_INFO + NameSize;
      Status = Arg->Handle->SetInfo(
            Arg->Handle,
            &GenericFileInfo,
            (UINTN) Info->Size,
            Info
            );

      FreePool (Info);
    }
  }

  if (EFI_ERROR(Status)) {
    Print (L"moving %s -> %s\n - error - %r\n", Arg->FullName, NewName, Status);
  }
  else {
    Print (L"moving %s -> %s\n - [ok]\n", Arg->FullName, NewName);
  }
  
  return Status;
}
