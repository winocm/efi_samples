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

  WinNtSimpleFileSystem.h

Abstract:

  Produce Simple File System abstractions for a directory on your PC using Win32 APIs.
  The configuration of what devices to mount or emulate comes from NT 
  environment variables. The variables must be visible to the Microsoft* 
  Developer Studio for them to work.

  * Other names and brands may be claimed as the property of others.

--*/

#ifndef _WIN_NT_SIMPLE_FILE_SYSTEM_H_
#define _WIN_NT_SIMPLE_FILE_SYSTEM_H_

#include "EfiWinNt.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (WinNtIo)

//
// Driver Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SimpleFileSystem)

#define FILENAME_BUFFER_SIZE  80

#define WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE_SIGNATURE   EFI_SIGNATURE_32('N','T','f','s')

typedef struct _WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE {
  UINTN                            Signature;
  EFI_WIN_NT_THUNK_PROTOCOL        *WinNtThunk;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  SimpleFileSystem;
  CHAR16                           FilePath[FILENAME_BUFFER_SIZE];
  EFI_UNICODE_STRING_TABLE         *ControllerNameTable;
} WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE;

#define WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(a)  \
         CR(a, WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE, SimpleFileSystem, WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE_SIGNATURE)

#define WIN_NT_EFI_FILE_PRIVATE_SIGNATURE EFI_SIGNATURE_32('l','o','f','s')

typedef struct {
  UINTN                            Signature;
  EFI_WIN_NT_THUNK_PROTOCOL        *WinNtThunk;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *SimpleFileSystem;
  EFI_FILE                         EfiFile;
  HANDLE                           LHandle;
  BOOLEAN                          IsDirectoryPath;
  CHAR16                           *FilePath;
  WCHAR                            *FileName;
} WIN_NT_EFI_FILE_PRIVATE;

#define WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS(a) \
          CR(a, WIN_NT_EFI_FILE_PRIVATE, EfiFile, WIN_NT_EFI_FILE_PRIVATE_SIGNATURE)

//
// Global Protocol Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gWinNtSimpleFileSystemDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gWinNtSimpleFileSystemComponentName;

//
// Driver Binding protocol member functions
//
EFI_STATUS
WinNtSimpleFileSystemDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
WinNtSimpleFileSystemDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  );

EFI_STATUS
WinNtSimpleFileSystemDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

//
// Simple File System protocol member functions
//
EFI_STATUS
WinNtSimpleFileSystemOpenVolume (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
  OUT EFI_FILE                        **Root
  );

EFI_STATUS
WinNtSimpleFileSystemOpen (
  IN  EFI_FILE  *This,
  OUT EFI_FILE  **NewHandle,
  IN  CHAR16    *FileName,
  IN  UINT64    OpenMode,
  IN  UINT64    Attributes
  );

EFI_STATUS
WinNtSimpleFileSystemClose (
  IN EFI_FILE  *This
  );

EFI_STATUS
WinNtSimpleFileSystemDelete (
  IN EFI_FILE  *This
  );

EFI_STATUS
WinNtSimpleFileSystemRead (
  IN     EFI_FILE  *This,
  IN OUT UINTN     *BufferSize,
  OUT    VOID      *Buffer
  );

EFI_STATUS
WinNtSimpleFileSystemWrite (
  IN     EFI_FILE  *This,
  IN OUT UINTN     *BufferSize,
  IN     VOID      *Buffer
  );

EFI_STATUS
WinNtSimpleFileSystemSetPosition (
  IN EFI_FILE  *This,
  IN UINT64    Position
  );

EFI_STATUS
WinNtSimpleFileSystemGetPosition (
  IN  EFI_FILE  *This,
  OUT UINT64    *Position
  );

EFI_STATUS
WinNtSimpleFileSystemGetInfo (
  IN     EFI_FILE  *This,
  IN     EFI_GUID  *InformationType,
  IN OUT UINTN     *BufferSize,
  OUT    VOID      *Buffer
  );

EFI_STATUS
WinNtSimpleFileSystemSetInfo (
  IN EFI_FILE  *This,
  IN EFI_GUID  *InformationType,
  IN UINTN     BufferSize,
  IN VOID      *Buffer
  );

EFI_STATUS
WinNtSimpleFileSystemFlush (
  IN EFI_FILE  *This
  );

#endif
