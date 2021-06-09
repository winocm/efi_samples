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

  load.c
  
Abstract:

  EFI Shell command "load"


Revision History

--*/

#include "shell.h"

//
//
//

EFI_STATUS
InitializeLoad (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );


static
EFI_STATUS
LoadDriver (
  IN EFI_HANDLE       ImageHandle,
  IN SHELL_FILE_ARG   *Arg,
  BOOLEAN             Connect
  );


//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeLoad)
#endif

EFI_STATUS
InitializeLoad (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Loads EFI drivers.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error

--*/
{
  EFI_STATUS          Status;
  CHAR16              **Argv;
  UINTN               Argc;
  UINTN               Index;
  EFI_LIST_ENTRY      FileList;
  EFI_LIST_ENTRY      *Link;
  SHELL_FILE_ARG      *Arg;
  BOOLEAN             Connect;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeLoad,
    L"load",     // command
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

  //
  // Expand each arg
  //
  Connect = TRUE;
  InitializeListHead (&FileList);
  for (Index = 1; Index < Argc; Index += 1) {
    if (Argv[Index][0] == L'-') {
      if ((Argv[Index][1] == L'n' || Argv[Index][1] == L'N') &&
          (Argv[Index][2] == L'c' || Argv[Index][2] == L'C')    ) {
        Connect = FALSE;
      } else {
        Print (L"load: unknown option %s\n", Argv[Index]);
        Status = EFI_INVALID_PARAMETER;
        ShellFreeFileList (&FileList);
        return Status;
      }
    } else {
      ShellFileMetaArg (Argv[Index], &FileList);
    }
  }

  if (IsListEmpty (&FileList)) {
    Print (L"load: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    ShellFreeFileList (&FileList);
    return Status;
  }

  //
  // Load each file
  //
  for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
    Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    Status = LoadDriver (ImageHandle, Arg, Connect);
  }

  ShellFreeFileList (&FileList);
  return Status;
}

VOID
LoadConnectAllDriversToAllControllers (
  VOID
  )

{
  EFI_STATUS  Status;
  UINTN       AllHandleCount;
  EFI_HANDLE  *AllHandleBuffer;
  UINTN       Index;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINT32      *HandleType;
  UINTN       HandleIndex;
  BOOLEAN     Parent;
  BOOLEAN     Device;

  Status = LibLocateHandle (
             AllHandles,
             NULL,
             NULL,
             &AllHandleCount,
             &AllHandleBuffer
             );
  if (EFI_ERROR (Status)) {
    return;
  }

  for (Index = 0; Index < AllHandleCount; Index++) {

    //
    // Scan the handle database
    //
    Status = LibScanHandleDatabase (
               NULL,
               NULL,
               AllHandleBuffer[Index],
               NULL,
               &HandleCount, 
               &HandleBuffer, 
               &HandleType
               );
    if (EFI_ERROR (Status)) {
      return;
    }

    Device = TRUE;
    if (HandleType[Index] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE) {
      Device = FALSE;
    }
    if (HandleType[Index] & EFI_HANDLE_TYPE_IMAGE_HANDLE) {
      Device = FALSE;
    }

    if (Device) {
      Parent = FALSE;
      for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
          Parent = TRUE;
        }
      }

      if (!Parent) {
        if (HandleType[Index] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
          Status = BS->ConnectController (
                         AllHandleBuffer[Index],
                         NULL,
                         NULL,
                         TRUE
                         );
        }
      }
    }

    BS->FreePool (HandleBuffer);
    BS->FreePool (HandleType);
  }

  BS->FreePool (AllHandleBuffer);
}


static
EFI_STATUS
LoadDriver (
  IN EFI_HANDLE       ParentImage,
  IN SHELL_FILE_ARG   *Arg,
  BOOLEAN             Connect
  )
{
  EFI_HANDLE                  ImageHandle;
  EFI_STATUS                  Status;
  EFI_DEVICE_PATH_PROTOCOL    *NodePath, *FilePath;
  EFI_LOADED_IMAGE_PROTOCOL   *ImageInfo;
  CHAR16                      *LoadOptions;
  UINTN                       LoadOptionsSize;
  CHAR16                      *Cwd;

  NodePath = FileDevicePath (NULL, Arg->FileName);
  FilePath = AppendDevicePath (Arg->ParentDevicePath, NodePath);
  FreePool (NodePath);

  Status = BS->LoadImage (
        FALSE,
        ParentImage,
        FilePath,
        NULL,
        0,
        &ImageHandle
        );
  FreePool (FilePath);

  if (EFI_ERROR(Status)) {
    Print (L"load: LoadImage %hs error - %r\n", Arg->FullName, Status);
    return EFI_INVALID_PARAMETER;
  }

  //
  // Verify the image is a driver ?
  //
  BS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID*)&ImageInfo);
  if (ImageInfo->ImageCodeType != EfiBootServicesCode &&
    ImageInfo->ImageCodeType != EfiRuntimeServicesCode) {

    Print (L"load: Image %hs is not a driver\n", Arg->FullName);
    BS->Exit (ImageHandle, EFI_INVALID_PARAMETER, 0, NULL);
    return EFI_INVALID_PARAMETER;
  }

  //
  // Construct a load options buffer containing the command line and
  // current working directory.
  //
  // NOTE: To prevent memory leaks, the protocol is responsible for
  // freeing the memory associated with the load options.
  //
  // One day we'll pass arguments to the protocol....
  //
  Cwd = ShellCurDir (NULL);
  LoadOptionsSize = (StrLen (Arg->FullName)+2 + StrLen (Cwd)+2) * sizeof(CHAR16);
  LoadOptions = AllocatePool (LoadOptionsSize);
  StrCpy (LoadOptions, Arg->FullName);
  StrCpy (&LoadOptions[ StrLen (LoadOptions) + 1 ], Cwd);
  FreePool (Cwd);

  ImageInfo->LoadOptionsSize = (UINT32)LoadOptionsSize;
  ImageInfo->LoadOptions = LoadOptions;

  //
  // Start the image
  //
  Status = BS->StartImage (ImageHandle, NULL, NULL);
  if (!EFI_ERROR(Status)) {
    Print (L"load: Image %hs loaded at 0x%x - %r\n",
        Arg->FullName,
        ImageInfo->ImageBase,
        Status
        );
  } else {
    Print (L"load: Image %hs error - %r\n",
        Arg->FullName,
        Status
        );
  }

  if (Connect) {
    LoadConnectAllDriversToAllControllers();
  }

  //
  // When any driver starts, turn off the watchdog timer
  //
  BS->SetWatchdogTimer ( 0x0000, 0x0000, 0x0000, NULL );
  return Status;
}
