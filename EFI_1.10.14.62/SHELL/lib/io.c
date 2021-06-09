
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

  io.c

Abstract:

  Initialize the shell library



Revision History

--*/

#include "shelllib.h"

CHAR16 *
ShellGetEnv (
  IN CHAR16       *Name
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  return  SE->GetEnv (Name);
}

CHAR16 *
ShellGetMap (
  IN CHAR16       *Name
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  return SE->GetMap (Name);
}

CHAR16 *
ShellCurDir (
  IN CHAR16               *DeviceName OPTIONAL
  )
/*++

Routine Description:
  
Arguments:

Returns:

Notes:
 Results are allocated from pool.  The caller must free the pool
  
--*/
{
  return SE->CurDir (DeviceName);
}


EFI_STATUS
ShellFileMetaArg (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{  
  return  SE->FileMetaArg (Arg, ListHead); 
}


EFI_STATUS
ShellFreeFileList (
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  return  SE->FreeFileList (ListHead);  
}


EFI_FILE_HANDLE 
ShellOpenFilePath (
  IN EFI_DEVICE_PATH_PROTOCOL   *FilePath,
  IN UINT64                     FileMode
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  EFI_HANDLE                    DeviceHandle;
  EFI_STATUS                    Status;
  EFI_FILE_HANDLE               FileHandle;
  EFI_FILE_HANDLE               LastHandle;
  FILEPATH_DEVICE_PATH          *FilePathNode;

  //
  // File the file system for this file path
  //
  Status = BS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid, &FilePath, &DeviceHandle);
  if (EFI_ERROR(Status)) {
    return NULL;
  }

  //
  // Attempt to access the file via a file system interface
  //
  FileHandle = LibOpenRoot (DeviceHandle);
  Status = FileHandle ? EFI_SUCCESS : EFI_UNSUPPORTED;

  //
  // To access as a file system, the file path should only
  // contain file path components.  Follow the file path nodes
  // and find the target file
  //
  FilePathNode = (FILEPATH_DEVICE_PATH *) FilePath;
  while (!IsDevicePathEnd(&FilePathNode->Header)) {

    //
    // For file system access each node should be a file path component
    //
    if (DevicePathType(&FilePathNode->Header) != MEDIA_DEVICE_PATH ||
      DevicePathSubType(&FilePathNode->Header) != MEDIA_FILEPATH_DP) {
      Status = EFI_UNSUPPORTED;
    }

    //
    // If there's been an error, stop
    //
    if (EFI_ERROR(Status)) {
      break;
    }
    
    //
    // Open this file path node
    //
    LastHandle = FileHandle;
    FileHandle = NULL;

    Status = LastHandle->Open (
            LastHandle,
            &FileHandle,
            FilePathNode->PathName,
            FileMode & ~EFI_FILE_MODE_CREATE,
            0
            );
    
    if ((EFI_ERROR(Status)) && 
      (FileMode != (FileMode & ~EFI_FILE_MODE_CREATE))) {
      Status = LastHandle->Open (
            LastHandle,
            &FileHandle,
            FilePathNode->PathName,
            FileMode,
            0
            );         
    }
    
    //
    // Close the last node
    //
    LastHandle->Close (LastHandle);

    //
    // Get the next node
    //
    FilePathNode = (FILEPATH_DEVICE_PATH *) NextDevicePathNode(&FilePathNode->Header);
  }

  if (EFI_ERROR(Status)) {
    FileHandle = NULL;
  }

  return FileHandle;
}
