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

  Shell

Revision History

--*/

#include "nshell.h"

//
// Globals
//
CHAR16 *ShellEnvPathName[] = {
  L"shellenv.efi",
  L"efi\\shellenv.efi",
  L"efi\\tools\\shellenv.efi",
  NULL
} ;

//
// Prototypes
//
EFI_STATUS
InitializeShell (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
ShellLoadEnvDriver (
  IN EFI_HANDLE           ImageHandle
  );

EFI_STATUS
NShellPrompt (
  IN EFI_HANDLE           ImageHandle
  );

BOOLEAN
ParseLoadOptions (
  EFI_HANDLE  ImageHandle,
  OUT CHAR16  **CommandLine,
  OUT CHAR16  **CurrentDir
  );

//
//
//
EFI_DRIVER_ENTRY_POINT (InitializeShell)

EFI_STATUS
InitializeShell (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

Arguments:

  ImageHandle     - The handle for this driver

  SystemTable     - The system table

Returns:

--*/
{
  EFI_STATUS              Status;
  EFI_HANDLE              Handle;
  UINTN                   BufferSize;
  VOID                    *Junk;
  BOOLEAN                 IsRootInstance;
  BOOLEAN                 DidILoadSE;
  CHAR16                  *CommandLine;
  CHAR16                  *CurrentDir;

  //
  // The shell may be started as either:
  //  1. the first time with no shell environment loaded yet
  //  2. not the first time, but with a shell environment loaded
  //  3. as a child of a parent shell image
  //
  IsRootInstance = FALSE;
  InitializeLib (ImageHandle, SystemTable);

  //
  // When the shell starts, turn off the watchdog timer
  //

  BS->SetWatchdogTimer ( 0x0000, 0x0000, 0x0000, NULL );

#ifdef EFI_MONOSHELL

  //
  // Load shell environment in monolithic shell.
  //
  InitializeShellEnvironment (ImageHandle, SystemTable);
#endif
    
  //
  // If the shell environment is not loaded, load it now.
  //
  DidILoadSE = FALSE;
  BufferSize = sizeof (Handle);
  Status = BS->LocateHandle (ByProtocol, &ShellEnvProtocol, NULL, &BufferSize, &Handle);
  if (EFI_ERROR (Status)) {
    #ifdef EFI_BOOTSHELL
      Status = ShellLoadEnvDriver (ImageHandle);
    #endif
    if (EFI_ERROR (Status)) {
      Print (L"Shell environment driver not loaded\n");
      BS->Exit (ImageHandle, Status, 0, NULL);
    }
  } 

  DidILoadSE = TRUE;
   
  //
  // Check to see if we're a child of a previous shell.
  //
  Status = BS->HandleProtocol (ImageHandle, &ShellInterfaceProtocol, (VOID*)&Junk);
  
  if (EFI_ERROR (Status)) {
    // 
    // Special case were the shell is being started directly .
    // (e.g., not as a child of another shell)
    //
    BufferSize = sizeof (Handle);
    Status = BS->LocateHandle (ByProtocol, &ShellEnvProtocol, NULL, &BufferSize, &Handle);
    ASSERT (!EFI_ERROR (Status));
    
    //
    // Get the shell environment interface pointer.
    //
    Status = BS->HandleProtocol (Handle, &ShellEnvProtocol, (VOID*)&SE);
    ASSERT (!EFI_ERROR (Status));

    //
    // Allocate a new shell interface structure, and assign it to our
    // image handle.
    //

    //
    // Note: SI should be freed when shell exits.
    //
    SI = SE->NewShell (ImageHandle);
    ASSERT (SI);
    
    Status = LibInstallProtocolInterfaces (&ImageHandle, &ShellInterfaceProtocol, SI, NULL);
    ASSERT (!EFI_ERROR (Status));
    
    IsRootInstance = TRUE;
  }

  //
  // Now we can initialize like a normal shell app.
  //
  InitializeShellApplication (ImageHandle, SystemTable);

  //
  // If there are load options, assume they contain a command line and
  // possible current working directory.
  //
  if (ParseLoadOptions (ImageHandle, &CommandLine, &CurrentDir)) {
    //
    // Skip the 1st argument which should be us.
    //
    while (*CommandLine != L' ' && *CommandLine != 0) {
      CommandLine++;
    }

    //
    // Get to the beginning of the next argument.
    //
    while (*CommandLine == L' ') {
      CommandLine++;
    }

    //
    // If there was a current working directory, set it.
    //
    if (CurrentDir) {
      CHAR16  CmdLine[256], *Tmp;

      //
      // Set a mapping
      //
      StrCpy (CmdLine, CurrentDir);
      for (Tmp = CmdLine; *Tmp && *Tmp != L':'; Tmp++);
      if ( *Tmp ) {
        Tmp += 1;
        *Tmp = 0;
        ShellExecute (ImageHandle, CmdLine, TRUE);
      }

      //
      // Now change to that directory
      //
      StrCpy (CmdLine, L"cd ");
      if ((StrLen (CmdLine) + StrLen (CurrentDir) + sizeof (CHAR16)) <
                  (sizeof (CmdLine) / sizeof (CHAR16))) {
        StrCat (CmdLine, CurrentDir);
        ShellExecute (ImageHandle, CmdLine, TRUE);
      }
    }

    //
    // Have the shell execute the remaining command line.  If there is
    // nothing remaining, run the shell main loop below.
    //
    if ( *CommandLine != 0 ) {
      return (ShellExecute (ImageHandle, CommandLine, TRUE));
    }
  }

  //
  // If this is the root instance, execute the command to load the default values
  //
  if (IsRootInstance) {

    Print (L"%EEFI Shell version %01d.%02d [%d.%d]\n%N",
      (ST->Hdr.Revision >> 16),
      (ST->Hdr.Revision & 0xffff),
      (ST->FirmwareRevision >> 16),
      (ST->FirmwareRevision & 0xffff));

    ShellExecute (ImageHandle, L"_load_defaults", TRUE);

    //
    // Dump device mappings, -r to sync with current hardware
    //
    ShellExecute (ImageHandle, L"map -r", TRUE);

    //
    // Run startup script (if any)
    //
    
    //
    // Turned on echo so you can tell the startup.nsh is running
    //
    ShellExecute (ImageHandle, L"startup.nsh", FALSE);
  }

  //
  // EFI Shell main loop
  //

  Status = EFI_SUCCESS;
  while (Status != -1) {
    Status = NShellPrompt (ImageHandle);
  }

  //
  // Done - cleanup the shell
  //
  Status = EFI_SUCCESS;

  //
  // Removed print to support "Disconnect All" exit
  //
  //  Print (L"Shell exit - %r\n", Status);

  //
  // If this is a root instance, we allocated a dummy shell interface for 
  // ourselves, free it now.
  //
  if (IsRootInstance) {
    //
    // Free all resources allocated in Shell
    //
    ShellFreeResources ();
    //
    // Uninstall Shell Interface Protocol
    //
    BS->UninstallProtocolInterface (ImageHandle, &ShellInterfaceProtocol, SI);
    FreePool (SI);
    if (DidILoadSE) {
      #ifdef EFI_MONOSHELL
      //
      // Uninstall Shell Environment Protocol
      //
      BS->UninstallProtocolInterface (Handle, &ShellEnvProtocol, SE);
      #endif
    }
  }

  return Status;
}


EFI_STATUS 
ShellLoadEnvDriverByPath (
  IN EFI_HANDLE           ParentImageHandle,
  IN EFI_HANDLE           DeviceHandle
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_HANDLE                NewImageHandle;
  UINTN                     Index;
  BOOLEAN                   SearchNext;

  Status = EFI_SUCCESS;
  
  //
  // If there's no device to search forget it
  //
  if (!DeviceHandle) {
    return EFI_NOT_FOUND;
  }

  //
  // Try loading shellenv from each path
  //
  SearchNext = TRUE;
  for (Index = 0; ShellEnvPathName[Index]  &&  SearchNext; Index++) {

    //
    // Load it
    //
    FilePath = FileDevicePath (DeviceHandle, ShellEnvPathName[Index]);
    ASSERT (FilePath);
    Status = BS->LoadImage (FALSE, ParentImageHandle, FilePath, NULL, 0, &NewImageHandle);
    FreePool (FilePath);

    //
    // Only search the next path if it was not found on this path
    //
    SearchNext = FALSE;
    if (Status == EFI_LOAD_ERROR || Status == EFI_NOT_FOUND) {
      SearchNext = TRUE;
    }

    //
    // If there was no error, start the image
    //
    if (!EFI_ERROR (Status)) {
      Status = BS->StartImage (NewImageHandle, NULL, NULL);
    }
  }

  return Status;
}



EFI_STATUS
ShellLoadEnvDriver (
  IN EFI_HANDLE           ImageHandle
  )
{
  EFI_STATUS                Status;
  EFI_LOADED_IMAGE_PROTOCOL *Image;
  UINTN                     Index, NoHandles;
  EFI_HANDLE                *Handles;

  //
  // Get the file path for the current image
  //
  Status = BS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID*)&Image);
  ASSERT (!EFI_ERROR (Status));

  //
  // Attempt to load shellenv
  //
  Status = ShellLoadEnvDriverByPath (Image->ParentHandle, Image->DeviceHandle);
  if (EFI_ERROR (Status)) {

    //
    // shellenv was not found.  Search all file systems for it
    //
    Status = LibLocateHandle (ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &NoHandles, &Handles);

    for (Index = 0; Index < NoHandles; Index++) {
      Status = ShellLoadEnvDriverByPath (Image->ParentHandle, Handles[Index]);
      if (!EFI_ERROR (Status)) {
        break;
      }
    }

    if (Handles) {
      FreePool (Handles);
    }
  }

  //
  // Done
  //
  return Status;
}


EFI_STATUS
NShellPrompt (
  IN EFI_HANDLE           ImageHandle
  )
{
  UINTN                   Column;
  UINTN                   Row;
  CHAR16                  *CmdLine;
  CHAR16                  *CurDir;
  UINTN                   BufferSize;
  EFI_STATUS              Status;
  UINTN                   Index;
  BOOLEAN                 AddLine;

  //
  // If there is no console output device, then a shell prompt can not be displayed, so ASSERT
  //
  ASSERT (ST->ConOut != NULL);

  //
  // Prompt for input
  //
  ST->ConOut->SetCursorPosition (ST->ConOut, 0, ST->ConOut->Mode->CursorRow);

  CurDir = ShellCurDir (NULL);
  if (CurDir) {
    Print (L"%E%s> ", CurDir);
    FreePool (CurDir);
  } else {
    Print (L"%EShell> ");
  }

  //
  // Get screen setting to decide size of the command line buffer
  //
  ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &Column, &Row);
  BufferSize = Column * Row * sizeof (CHAR16);
  CmdLine = AllocateZeroPool (BufferSize);
  if (CmdLine == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  //
  // If there is no console input device, then a command can not be read, so ASSERT
  //
  ASSERT (ST->ConIn != NULL);

  //
  // Read a line from the console
  //
  Status = SI->StdIn->Read (SI->StdIn, &BufferSize, CmdLine);

  //
  // Null terminate the string and parse it
  //
  if (!EFI_ERROR (Status)) {
    CmdLine[BufferSize / sizeof (CHAR16)] = 0;
    AddLine = FALSE;
    for (Index = 0; CmdLine[Index] != 0 ; Index++) {
      if (CmdLine[Index] != L' ') {
        AddLine = TRUE;
        break;
      }
    }
    Status = ShellExecute (ImageHandle, CmdLine, TRUE);
    if (AddLine) {
      Print (L"\n");
    }
  }

  //
  // Done with this command
  //
  FreePool (CmdLine);
  return Status;
}


BOOLEAN
ParseLoadOptions (
  EFI_HANDLE  ImageHandle,
  OUT CHAR16  **CommandLine,
  OUT CHAR16  **CurrentDir
  )
{
  EFI_LOADED_IMAGE_PROTOCOL    *Image;
  EFI_STATUS                   Status;

  //
  // Set defaults.
  //
  *CommandLine = NULL;
  *CurrentDir = NULL;

  Status = BS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID*)&Image);

  if (!EFI_ERROR (Status)) {

    CHAR16 *CmdLine = Image->LoadOptions;
    
    //
    // Make sure it is power of 2
    //
    UINT32  CmdSize = Image->LoadOptionsSize & ~1; 

    if (CmdLine && CmdSize) {

      //
      // Set command line pointer for caller
      //
      *CommandLine = CmdLine;

      //
      // See if current working directory was passed.
      //
      while ((*CmdLine != 0) && CmdSize) {
        CmdLine++;
        CmdSize -= sizeof (CHAR16);
      }

      //
      // If a current working directory was passed, set it.
      //
      if (CmdSize > sizeof (CHAR16)) {
        CmdLine += 1;
        *CurrentDir = CmdLine;
      }

      return TRUE;
    }
  }

  return FALSE;
}
