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

  vol.c
  
Abstract:

  EFI Shell command "vol"


Revision History

--*/

#include "shell.h"

EFI_STATUS
InitializeVol (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeVol)
#endif

EFI_STATUS
InitializeVol (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Displays volume information for specified file system.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error

--*/
{
  CHAR16                            *VolumeLabel = NULL;
  EFI_STATUS                        Status;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   *Vol;
  EFI_FILE_HANDLE                   RootFs;
  EFI_FILE_SYSTEM_INFO              *VolumeInfo;
  UINTN                             Size;
  CHAR16                            *CurDir;
  UINTN                             Index;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeVol,
    L"vol",     // command
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

  //
  // Get file system from arguments. If file system not specified, 
  // use file system of current directory as default
  //
  DevicePath = NULL;
  if (SI->Argc == 1) {
    CurDir = ShellCurDir(NULL);
    if (CurDir) {
      for (Index=0; Index < StrLen(CurDir) && CurDir[Index] != ':'; Index++) {
        ;
      }
      CurDir[Index] = 0;
      DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (CurDir);
      FreePool (CurDir);
    }
  } else {
    DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (SI->Argv[1]);
  }
  if (DevicePath == NULL) {
    Print (L"vol: %hs is not mapped\n", SI->Argv[1]);
    return EFI_INVALID_PARAMETER;
  }
  Status = LibDevicePathToInterface (&gEfiSimpleFileSystemProtocolGuid, DevicePath, (VOID **)&Vol);
  if (EFI_ERROR(Status)) {
    Print (L"vol: %hs is not a file system\n", SI->Argv[1]);
    return Status;
  }

  if ( SI->Argc == 3 ) {
    VolumeLabel = SI->Argv[2];
    if (StrLen(VolumeLabel) > 11) {
      Print(L"vol: Volume label %hs is too long. Maximum is 11 characters\n", VolumeLabel);
      return EFI_INVALID_PARAMETER;
    }
  }

  Status = Vol->OpenVolume (Vol, &RootFs);

  if (EFI_ERROR(Status)) {
    Print(L"vol: Cannot open the volume %hs\n", SI->Argv[1]);
    return Status;
  }

  //
  // Get volume information of file system 
  //
  Size = SIZE_OF_EFI_FILE_SYSTEM_INFO + 100;
  VolumeInfo = (EFI_FILE_SYSTEM_INFO *)AllocatePool(Size);
  Status = RootFs->GetInfo(RootFs,&gEfiFileSystemInfoGuid,&Size,VolumeInfo);

  if (EFI_ERROR(Status)) {
    Print(L"vol: Cannot get volume information of %hs\n", SI->Argv[1]);
    FreePool (VolumeInfo);
    return Status;
  }

  //
  // Set volume label
  //
  if (VolumeLabel) {
    StrCpy (VolumeInfo->VolumeLabel, VolumeLabel);

    Size = SIZE_OF_EFI_FILE_SYSTEM_INFO + StrSize(VolumeLabel);
    
    VolumeInfo->Size = Size;
    Status = RootFs->SetInfo(RootFs,&gEfiFileSystemInfoGuid,Size,VolumeInfo);

    if (EFI_ERROR(Status)) {
      Print(L"vol: Cannot set volume information for %hs\n", SI->Argv[1]);
      FreePool (VolumeInfo);
      return Status;
    }

    Status = RootFs->GetInfo(RootFs,&gEfiFileSystemInfoGuid,&Size,VolumeInfo);

    if (EFI_ERROR(Status)) {
      Print(L"vol: Cannot verify volume information for %hs\n", SI->Argv[1]);
      FreePool (VolumeInfo);
      return Status;
    }
  }

  //
  // Print out volume information
  //
  if (StrLen(VolumeInfo->VolumeLabel) == 0) {
    Print(L"Volume has no label",VolumeInfo->VolumeLabel);
  } else {
    Print(L"Volume %hs",VolumeInfo->VolumeLabel);
  }
  if (VolumeInfo->ReadOnly) {
    Print(L" (ro)\n");
  } else {
    Print(L" (rw)\n");
  }
  Print(L"  %13,ld bytes total disk space\n",VolumeInfo->VolumeSize);
  Print(L"  %13,ld bytes available on disk\n",VolumeInfo->FreeSpace);
  Print(L"  %13,d bytes in each allocation unit\n",VolumeInfo->BlockSize);

  RootFs->Flush(RootFs);
  RootFs->Close(RootFs);
  
  FreePool (VolumeInfo);
  
  return EFI_SUCCESS;
}
