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

  init.c
  
Abstract:

  Shell Environment driver

Revision History

--*/

#include "shelle.h"

EFI_LIST_ENTRY SEnvCurMapping;
EFI_LIST_ENTRY SEnvOrgFsDevicePaths;
EFI_LIST_ENTRY SEnvCurFsDevicePaths;
EFI_LIST_ENTRY SEnvOrgBlkDevicePaths;
EFI_LIST_ENTRY SEnvCurBlkDevicePaths;

//
//
//
EFI_STATUS
InitializeShellEnvironment (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
//
//
#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeShellEnvironment)
#endif

EFI_STATUS
InitializeShellEnvironment (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

Arguments:

  ImageHandle     - The handle for this driver

  SystemTable     - The system table

Returns:

  EFI file system driver is enabled

--*/
{
  EFI_HANDLE              Handle;
  UINTN                   BufferSize;
  EFI_STATUS              Status;

  //
  // Initialize EFI library
  //
  InitializeLib (ImageHandle, SystemTable);

  //
  // If we are already installed, don't install again
  //
  BufferSize = sizeof(Handle);
  Status = BS->LocateHandle (ByProtocol, 
                            &ShellEnvProtocol, 
                            NULL, 
                            &BufferSize, 
                            &Handle);  
  if (!EFI_ERROR (Status)) {
    return EFI_LOAD_ERROR;
  }

  //
  // Initialize globals
  //
  InitializeLock (&SEnvLock, EFI_TPL_APPLICATION);
  InitializeLock (&SEnvGuidLock, EFI_TPL_NOTIFY);

  if (SEnvCmds.Flink == NULL) {
    SEnvInitCommandTable();
  }
  SEnvInitProtocolInfo();
  SEnvInitVariables();
  SEnvInitHandleGlobals();
  SEnvInitMap();
  SEnvLoadInternalProtInfo();
  SEnvConIoInitDosKey();
  SEnvInitBatch();

  //
  // Install shellenv handle (or override the existing one)
  //
  Handle = ImageHandle;
  Status = LibInstallProtocolInterfaces (&Handle, 
                                         &ShellEnvProtocol, 
                                         &SEnvInterface, 
                                         NULL);

  ASSERT (!EFI_ERROR(Status));
  
  //
  // Initialize linked list for current mapping
  //
  InitializeListHead (&SEnvCurMapping);

  //
  // Initialize linked list for current device paths
  //
  InitializeListHead (&SEnvOrgFsDevicePaths);
  InitializeListHead (&SEnvCurFsDevicePaths);
  InitializeListHead (&SEnvOrgBlkDevicePaths);
  InitializeListHead (&SEnvCurBlkDevicePaths);

  return EFI_SUCCESS;
}


EFI_SHELL_INTERFACE *
SEnvNewShell (
  IN EFI_HANDLE                   ImageHandle
  )
{
  EFI_SHELL_INTERFACE             *ShellInt;
  EFI_STATUS            Status;
  
  //
  // Allocate a new structure
  //
  ShellInt = AllocateZeroPool (sizeof(EFI_SHELL_INTERFACE));
  ASSERT (ShellInt);

  //
  // Fill in the SI pointer
  //
  Status = BS->HandleProtocol (ImageHandle, 
                               &gEfiLoadedImageProtocolGuid, 
                               (VOID*)&ShellInt->Info);

  if (EFI_ERROR(Status) || ShellInt->Info == NULL) {
    FreePool (ShellInt);
    return NULL;
  }

  //
  // Fill in the std file handles
  //
  ShellInt->ImageHandle = ImageHandle;
  ShellInt->StdIn  = &SEnvIOFromCon;
  ShellInt->StdOut = &SEnvIOFromCon;
  ShellInt->StdErr = &SEnvErrIOFromCon;

  return ShellInt;
}
