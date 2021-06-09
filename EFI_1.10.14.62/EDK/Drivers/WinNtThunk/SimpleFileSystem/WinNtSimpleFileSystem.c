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

  WinNtSimpleFileSystem.c

Abstract:

  Produce Simple File System abstractions for directories on your PC using Win32 APIs.
  The configuration of what devices to mount or emulate comes from NT 
  environment variables. The variables must be visible to the Microsoft* 
  Developer Studio for them to work.

  * Other names and brands may be claimed as the property of others.

--*/

#include "WinNtSimpleFileSystem.h"

EFI_DRIVER_BINDING_PROTOCOL gWinNtSimpleFileSystemDriverBinding = {
  WinNtSimpleFileSystemDriverBindingSupported,
  WinNtSimpleFileSystemDriverBindingStart,
  WinNtSimpleFileSystemDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT (InitializeWinNtSimpleFileSystem)

EFI_STATUS
InitializeWinNtSimpleFileSystem (
  IN EFI_HANDLE			    	ImageHandle,
  IN EFI_SYSTEM_TABLE			*SystemTable
  )
/*++

Routine Description:

  Intialize Win32 Environment to produce Simple File System abstractions.

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  EFI_STATUS

--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gWinNtSimpleFileSystemDriverBinding, 
           ImageHandle,
           &gWinNtSimpleFileSystemComponentName,
           NULL,
           NULL
           );
}

EFI_STATUS
WinNtSimpleFileSystemDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                         Status;
  EFI_WIN_NT_IO_PROTOCOL  *WinNtIo;
  
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiWinNtIoProtocolGuid,  
                  &WinNtIo,
                  This->DriverBindingHandle,   
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Make sure GUID is for a File System handle.
  //
  Status = EFI_UNSUPPORTED;
  if (EfiCompareGuid (WinNtIo->TypeGuid, &gEfiWinNtFileSystemGuid)) {
    Status = EFI_SUCCESS;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         ControllerHandle,   
         &gEfiWinNtIoProtocolGuid,  
         This->DriverBindingHandle,   
         ControllerHandle   
        );

  return Status;
}

EFI_STATUS
WinNtSimpleFileSystemDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                         Status;
  EFI_WIN_NT_IO_PROTOCOL  *WinNtIo;
  WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE  *Private;
  UINTN                              Index;
 
  Private = NULL;

  //
  // Open the IO Abstraction(s) needed 
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiWinNtIoProtocolGuid,  
                  &WinNtIo,
                  This->DriverBindingHandle,   
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Validate GUID
  //
  if (!EfiCompareGuid (WinNtIo->TypeGuid, &gEfiWinNtFileSystemGuid)) {
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

	Status = gBS->AllocatePool(
                  EfiBootServicesData,
									sizeof(WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE), 
                  &Private
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Private->Signature  = WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE_SIGNATURE;
  Private->WinNtThunk = WinNtIo->WinNtThunk;

  for (Index = 0; WinNtIo->EnvString[Index] != 0; Index++) {
    Private->FilePath[Index] = WinNtIo->EnvString[Index];
  }
  Private->FilePath[Index] = 0;

  Private->SimpleFileSystem.Revision   = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
  Private->SimpleFileSystem.OpenVolume = WinNtSimpleFileSystemOpenVolume;

  Private->WinNtThunk->SetErrorMode (SEM_FAILCRITICALERRORS);

  Private->ControllerNameTable = NULL;

  EfiLibAddUnicodeString (
    "eng", 
    gWinNtSimpleFileSystemComponentName.SupportedLanguages, 
    &Private->ControllerNameTable, 
    WinNtIo->EnvString
    );

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ControllerHandle,
                  &gEfiSimpleFileSystemProtocolGuid, &Private->SimpleFileSystem,
                  NULL
                  );

Done:
  if (EFI_ERROR (Status)) {

    if (Private != NULL) {

      EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);

      gBS->FreePool (Private);
    }

    gBS->CloseProtocol (
           ControllerHandle,   
           &gEfiWinNtIoProtocolGuid,  
           This->DriverBindingHandle,   
           ControllerHandle
           );
  }

  return Status;
}

EFI_STATUS
WinNtSimpleFileSystemDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                         Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL    *SimpleFileSystem;
  WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE  *Private;
  
  //
  // Get our context back
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,   
                  &gEfiSimpleFileSystemProtocolGuid,  
                  &SimpleFileSystem,
                  This->DriverBindingHandle,   
                  ControllerHandle,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Private = WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS (SimpleFileSystem);

  //
  // Uninstall the Simple File System Protocol from ControllerHandle
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  ControllerHandle, 
                  &gEfiSimpleFileSystemProtocolGuid, &Private->SimpleFileSystem,
                  NULL
                  );
  if (!EFI_ERROR (Status)) {
    Status = gBS->CloseProtocol (
                    ControllerHandle, 
                    &gEfiWinNtIoProtocolGuid, 
                    This->DriverBindingHandle,   
                    ControllerHandle
                    );
  }

  if (!EFI_ERROR (Status)) {
    //
    // Free our instance data
    //
    EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);

    gBS->FreePool (Private);
  }

  return Status;
}

EFI_STATUS
WinNtSimpleFileSystemOpenVolume (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This,
  OUT EFI_FILE                        **Root
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                         Status;
  WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE  *Private;
  WIN_NT_EFI_FILE_PRIVATE            *PrivateFile;

  Private = WIN_NT_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS (This);

  PrivateFile = NULL;
	Status = gBS->AllocatePool(
                  EfiBootServicesData,
									sizeof(WIN_NT_EFI_FILE_PRIVATE),
                  &PrivateFile
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  PrivateFile->FileName = NULL;
	Status = gBS->AllocatePool(
                  EfiBootServicesData,
									EfiStrSize(L"."),
                  &PrivateFile->FileName
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  EfiStrCpy(PrivateFile->FileName,L".");
  PrivateFile->Signature            = WIN_NT_EFI_FILE_PRIVATE_SIGNATURE;
  PrivateFile->WinNtThunk           = Private->WinNtThunk;
  PrivateFile->SimpleFileSystem     = This;
  PrivateFile->IsDirectoryPath      = TRUE;
  PrivateFile->FilePath             = Private->FilePath;
  PrivateFile->EfiFile.Revision     = EFI_FILE_HANDLE_REVISION;
  PrivateFile->EfiFile.Open         = WinNtSimpleFileSystemOpen;
  PrivateFile->EfiFile.Close        = WinNtSimpleFileSystemClose;
  PrivateFile->EfiFile.Delete       = WinNtSimpleFileSystemDelete;
  PrivateFile->EfiFile.Read         = WinNtSimpleFileSystemRead;
  PrivateFile->EfiFile.Write        = WinNtSimpleFileSystemWrite;
  PrivateFile->EfiFile.GetPosition  = WinNtSimpleFileSystemGetPosition;
  PrivateFile->EfiFile.SetPosition  = WinNtSimpleFileSystemSetPosition;
  PrivateFile->EfiFile.GetInfo      = WinNtSimpleFileSystemGetInfo;
  PrivateFile->EfiFile.SetInfo      = WinNtSimpleFileSystemSetInfo;
  PrivateFile->EfiFile.Flush        = WinNtSimpleFileSystemFlush;
  PrivateFile->LHandle              = INVALID_HANDLE_VALUE;

  *Root = &PrivateFile->EfiFile;

  Status = EFI_SUCCESS;

Done:
  if (EFI_ERROR (Status)) {
    if (PrivateFile) {
      if (PrivateFile->FileName) {
        gBS->FreePool (PrivateFile->FileName);
      }
      gBS->FreePool (PrivateFile);
    }
  }

  return Status;
}

EFI_STATUS
WinNtSimpleFileSystemOpen (
  IN  EFI_FILE  *This,
  OUT EFI_FILE  **NewHandle,
  IN  CHAR16    *FileName,
  IN  UINT64    OpenMode,
  IN  UINT64    Attributes
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_FILE                 *Root;
  WIN_NT_EFI_FILE_PRIVATE  *PrivateFile;
  WIN_NT_EFI_FILE_PRIVATE  *NewPrivateFile;
  EFI_STATUS               Status;
  WCHAR                    TempFileName[256];

  PrivateFile = WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS(This);
  NewPrivateFile = NULL;

  //
  // BUGBUG: assume an open of root
  // if current location, return current data
  //

  if (EfiStrCmp(FileName, L"\\") == 0 ||
      (EfiStrCmp (FileName, L".") == 0  && PrivateFile->IsDirectoryPath)) {

    //
    // BUGBUG: assume an open root
    //
    Status = WinNtSimpleFileSystemOpenVolume (PrivateFile->SimpleFileSystem, &Root);
    NewPrivateFile = WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS(Root);
    goto Done;
  }

  if (Attributes & EFI_FILE_DIRECTORY) {
    return EFI_UNSUPPORTED;
  }

  //
  // Bugbug: hack for no path name support
  //
  if (*FileName == L'\\') {
    FileName++;
  }

  //
  // Attempt to open the file
  //
	Status = gBS->AllocatePool(
                  EfiBootServicesData,
									sizeof(WIN_NT_EFI_FILE_PRIVATE),
                  &NewPrivateFile
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  EfiCopyMem(NewPrivateFile, PrivateFile, sizeof(WIN_NT_EFI_FILE_PRIVATE));

  NewPrivateFile->FileName = NULL;
  Status = gBS->AllocatePool(
                  EfiBootServicesData,
									EfiStrSize(FileName),
                  &NewPrivateFile->FileName
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  NewPrivateFile->IsDirectoryPath = FALSE;
  EfiStrCpy (NewPrivateFile->FileName, FileName);

  EfiStrCpy (TempFileName, NewPrivateFile->FilePath);
  EfiStrCat (TempFileName, L"\\");
  EfiStrCat (TempFileName, NewPrivateFile->FileName);

  Status = EFI_SUCCESS;

  NewPrivateFile->LHandle = NewPrivateFile->WinNtThunk->CreateFile (
                                                          TempFileName,
                                                          GENERIC_READ | GENERIC_WRITE,
                                                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                          NULL,
                                                          (OpenMode & EFI_FILE_MODE_CREATE) ? OPEN_ALWAYS : OPEN_EXISTING,
                                                          0,
                                                          NULL
                                                          );

  if (NewPrivateFile->LHandle == INVALID_HANDLE_VALUE) {
    NewPrivateFile->LHandle = NewPrivateFile->WinNtThunk->CreateFile (
                                                          TempFileName,
                                                          GENERIC_READ,
                                                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                          NULL,
                                                          (OpenMode & EFI_FILE_MODE_CREATE) ? OPEN_ALWAYS : OPEN_EXISTING,
                                                          0,
                                                          NULL
                                                          );
    if (NewPrivateFile->LHandle == INVALID_HANDLE_VALUE) {
      Status = EFI_NOT_FOUND;
    }
  }

Done:
  if (EFI_ERROR(Status)) {
    if (NewPrivateFile) {
      if (NewPrivateFile->FileName) {
        gBS->FreePool(NewPrivateFile->FileName);
      }
      gBS->FreePool(NewPrivateFile);
    }
  } else {
    *NewHandle = &NewPrivateFile->EfiFile;
  }

  return Status;
}

EFI_STATUS
WinNtSimpleFileSystemClose (
  IN EFI_FILE  *This
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  WIN_NT_EFI_FILE_PRIVATE  *PrivateFile;

  PrivateFile = WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);

  if (PrivateFile->LHandle != INVALID_HANDLE_VALUE) {
    if (PrivateFile->IsDirectoryPath) {
      PrivateFile->WinNtThunk->FindClose (PrivateFile->LHandle);
    } else {
      PrivateFile->WinNtThunk->CloseHandle (PrivateFile->LHandle);
    }
  }

  if (PrivateFile->FileName) {
    gBS->FreePool (PrivateFile->FileName);
  }

  gBS->FreePool (PrivateFile);
  return EFI_SUCCESS;
}

EFI_STATUS
WinNtSimpleFileSystemDelete (
  IN EFI_FILE  *This
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS               Status;
  WIN_NT_EFI_FILE_PRIVATE  *PrivateFile;
  WCHAR                    TempFileName[256];

  PrivateFile = WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);

  Status = EFI_DEVICE_ERROR;
  if (PrivateFile->LHandle != INVALID_HANDLE_VALUE) {
    if (PrivateFile->IsDirectoryPath) {
      PrivateFile->WinNtThunk->FindClose (PrivateFile->LHandle);
    } else {
      PrivateFile->WinNtThunk->CloseHandle (PrivateFile->LHandle);
    }
  }

  if (PrivateFile->FileName) {
    EfiStrCpy (TempFileName, PrivateFile->FilePath);
    EfiStrCat (TempFileName, L"\\");
    EfiStrCat (TempFileName, PrivateFile->FileName);
    if (PrivateFile->WinNtThunk->DeleteFile(TempFileName)) {
      Status = EFI_SUCCESS;
    }
    gBS->FreePool (PrivateFile->FileName);
  }

  gBS->FreePool (PrivateFile);

  return Status;
}

static
VOID
WinNtSystemTimeToEfiTime (
  IN SYSTEMTIME             *SystemTime,
  IN TIME_ZONE_INFORMATION  *TimeZone,
  OUT EFI_TIME              *Time
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  Time->Year       = (UINT16) SystemTime->wYear;
  Time->Month      = (UINT8)  SystemTime->wMonth;
  Time->Day        = (UINT8)  SystemTime->wDay;
  Time->Hour       = (UINT8)  SystemTime->wHour;
  Time->Minute     = (UINT8)  SystemTime->wMinute;
  Time->Second     = (UINT8)  SystemTime->wSecond;
  Time->Nanosecond = (UINT32) SystemTime->wMilliseconds * 1000000;
  Time->TimeZone   = (INT16) TimeZone->Bias;

  if (TimeZone->StandardDate.wMonth) {
    Time->Daylight = EFI_TIME_ADJUST_DAYLIGHT;
  }    
}

EFI_STATUS
WinNtSimpleFileSystemRead (
  IN     EFI_FILE  *This,
  IN OUT UINTN     *BufferSize,
  OUT    VOID      *Buffer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  WIN_NT_EFI_FILE_PRIVATE  *PrivateFile;
  EFI_STATUS               Status;
  WIN32_FIND_DATA          FBuf;
  INTN                     f;
  UINTN                    Size;
  UINTN                    NameSize;
  UINTN                    ResultSize;
  UINTN                    i;
  SYSTEMTIME               SystemTime;
  EFI_FILE_INFO            *Info;
  WCHAR                    *pw;
  TIME_ZONE_INFORMATION    TimeZone;

  PrivateFile = WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);

  if (!PrivateFile->IsDirectoryPath) {
    f = PrivateFile->WinNtThunk->ReadFile (PrivateFile->LHandle, Buffer, *BufferSize, BufferSize, NULL);
    return f ? EFI_SUCCESS : EFI_DEVICE_ERROR;
  }

  //
  // Read on a directory.  Perform a find next
  //
  for (; ;) {
    f = PrivateFile->WinNtThunk->FindNextFile (PrivateFile->LHandle, &FBuf);
    if (!f) {
      *BufferSize = 0;
      return EFI_SUCCESS;
    }

    if (FBuf.cFileName[0] == '.'  &&  
      FBuf.cFileName[1] == 0) {
      continue;
    }

    if (FBuf.cFileName[0] == '.'  &&  
        FBuf.cFileName[1] == '.'  && 
        FBuf.cFileName[2] == 0) {
      continue;
    }
    break;
  }


  Size = SIZE_OF_EFI_FILE_INFO;

  NameSize = EfiStrSize(FBuf.cFileName);
  ResultSize = Size + NameSize;

  Status = EFI_BUFFER_TOO_SMALL;
  if (*BufferSize >= ResultSize) {
    Status = EFI_SUCCESS;

    Info = Buffer;
    EfiZeroMem (Info, ResultSize);

    Info->Size = ResultSize;

    PrivateFile->WinNtThunk->GetTimeZoneInformation (&TimeZone);

    PrivateFile->WinNtThunk->FileTimeToLocalFileTime (&FBuf.ftCreationTime, &FBuf.ftCreationTime);
    PrivateFile->WinNtThunk->FileTimeToSystemTime (&FBuf.ftCreationTime, &SystemTime);
    WinNtSystemTimeToEfiTime (&SystemTime, &TimeZone, &Info->CreateTime);

    PrivateFile->WinNtThunk->FileTimeToLocalFileTime (&FBuf.ftLastWriteTime, &FBuf.ftLastWriteTime);
    PrivateFile->WinNtThunk->FileTimeToSystemTime (&FBuf.ftLastWriteTime, &SystemTime);
    WinNtSystemTimeToEfiTime (&SystemTime, &TimeZone, &Info->ModificationTime);

    Info->FileSize = FBuf.nFileSizeLow;
    Info->PhysicalSize = FBuf.nFileSizeLow;

    if (FBuf.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
      Info->Attribute |= EFI_FILE_READ_ONLY;
    }

    if (FBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      Info->Attribute |= EFI_FILE_DIRECTORY;
    }

    NameSize = NameSize / sizeof(WCHAR);
    pw = (WCHAR *) ((CHAR8 *) Buffer + Size);
    for (i=0; i < NameSize; i++) {
      pw[i] = FBuf.cFileName[i];
    }
  }

  return Status;
}

EFI_STATUS
WinNtSimpleFileSystemWrite (
  IN     EFI_FILE  *This,
  IN OUT UINTN     *BufferSize,
  IN     VOID      *Buffer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  WIN_NT_EFI_FILE_PRIVATE  *PrivateFile;
  INTN                     f;

  PrivateFile = WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);

  if (PrivateFile->IsDirectoryPath) {
    return EFI_INVALID_PARAMETER;
  }

  f = PrivateFile->WinNtThunk->WriteFile (PrivateFile->LHandle, Buffer, *BufferSize, BufferSize, NULL);
  return f ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

EFI_STATUS
WinNtSimpleFileSystemSetPosition (
  IN EFI_FILE  *This,
  IN UINT64    Position
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS               Status;
  WIN_NT_EFI_FILE_PRIVATE  *PrivateFile;
  WIN32_FIND_DATA          FBuf;
  UINTN                    i;
  CHAR16                   *SearchPath;

  PrivateFile = WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);

  if (PrivateFile->IsDirectoryPath) {

    //
    // If there is an existing search, close it
    //
    if (PrivateFile->LHandle != INVALID_HANDLE_VALUE) {
      PrivateFile->WinNtThunk->FindClose (PrivateFile->LHandle);
    }

	  Status = gBS->AllocatePool(
                    EfiBootServicesData,
									  EfiStrSize(PrivateFile->FilePath) + EfiStrSize(L"\\*"),
                    &SearchPath
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    EfiStrCpy(SearchPath, PrivateFile->FilePath);
    EfiStrCat(SearchPath,L"\\*");
    PrivateFile->LHandle = PrivateFile->WinNtThunk->FindFirstFile (
                                                      SearchPath,
                                                      &FBuf
                                                      );
    gBS->FreePool(SearchPath);
    
    Status = PrivateFile->LHandle == INVALID_HANDLE_VALUE ? EFI_DEVICE_ERROR : EFI_SUCCESS;

  } else {

    if(Position == (UINT64)-1) {
      i = PrivateFile->WinNtThunk->SetFilePointer (
                                     PrivateFile->LHandle, 
                                     (ULONG) 0, 
                                     NULL, 
                                     FILE_END
                                     );
    } else {
      i = PrivateFile->WinNtThunk->SetFilePointer (
                                     PrivateFile->LHandle, 
                                     (ULONG) Position, 
                                     NULL, 
                                     FILE_BEGIN
                                     );
    }
    Status = i == 0xffffffff ? EFI_DEVICE_ERROR : EFI_SUCCESS;
  }

  return Status;
}

EFI_STATUS
WinNtSimpleFileSystemGetPosition (
  IN  EFI_FILE  *This,
  OUT UINT64    *Position
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS               Status;
  WIN_NT_EFI_FILE_PRIVATE  *PrivateFile;
  INT32                    PositionHigh;
  UINT64                   PosHigh64;
  WIN32_FIND_DATA          FBuf;
  CHAR16                   *SearchPath;

  PrivateFile = WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);

  PositionHigh = 0;
  PosHigh64    = 0;

  if (PrivateFile->IsDirectoryPath) {

    //
    // If there is an existing search, close it
    //
    if (PrivateFile->LHandle != INVALID_HANDLE_VALUE) {
      PrivateFile->WinNtThunk->FindClose (PrivateFile->LHandle);
    }

	  Status = gBS->AllocatePool(
                    EfiBootServicesData,
									  EfiStrSize(PrivateFile->FilePath) + EfiStrSize(L"\\*"),
                    &SearchPath
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    EfiStrCpy(SearchPath, PrivateFile->FilePath);
    EfiStrCat(SearchPath,L"\\*");
    PrivateFile->LHandle = PrivateFile->WinNtThunk->FindFirstFile (
                                                      SearchPath,
                                                      &FBuf
                                                      );

    gBS->FreePool(SearchPath);
    
    Status = PrivateFile->LHandle == INVALID_HANDLE_VALUE ? EFI_DEVICE_ERROR : EFI_SUCCESS;

  } else {

    PositionHigh = 0;
    *Position = PrivateFile->WinNtThunk->SetFilePointer(
                                           PrivateFile->LHandle, 
                                           0, 
                                           &PositionHigh, 
                                           FILE_CURRENT
                                           );

    Status = *Position == 0xffffffff ? EFI_DEVICE_ERROR : EFI_SUCCESS;
    if ( EFI_ERROR(Status) ) {
      goto Done;
    }
    PosHigh64 = PositionHigh;
    *Position += DriverLibLShiftU64(PosHigh64,32);
  }

Done:
  return Status;
}

static
EFI_STATUS
WinNtSimpleFileSystemFileInfo (
  IN     WIN_NT_EFI_FILE_PRIVATE  *PrivateFile,
  IN OUT UINTN                    *BufferSize, 
  OUT    VOID                     *Buffer    
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                 Status;
  UINTN                      Size;
  UINTN                      NameSize;
  UINTN                      ResultSize;
  EFI_FILE_INFO              *Info;
  BY_HANDLE_FILE_INFORMATION FileInfo;
  SYSTEMTIME                 SystemTime;

  Size = SIZE_OF_EFI_FILE_INFO;
  NameSize = EfiStrSize (PrivateFile->FileName);
  ResultSize = Size + NameSize;

  Status = EFI_BUFFER_TOO_SMALL;
  if (*BufferSize >= ResultSize) {
    Status = EFI_SUCCESS;

    Info = Buffer;
    EfiZeroMem (Info, ResultSize);

    Info->Size = ResultSize;
    PrivateFile->WinNtThunk->GetFileInformationByHandle(PrivateFile->LHandle, &FileInfo);
    Info->FileSize = FileInfo.nFileSizeLow;
    Info->PhysicalSize = Info->FileSize;

    PrivateFile->WinNtThunk->FileTimeToSystemTime(&FileInfo.ftCreationTime, &SystemTime);
    Info->CreateTime.Year    = SystemTime.wYear;
    Info->CreateTime.Month   = (UINT8)SystemTime.wMonth;
    Info->CreateTime.Day     = (UINT8)SystemTime.wDay;
    Info->CreateTime.Hour    = (UINT8)SystemTime.wHour;
    Info->CreateTime.Minute  = (UINT8)SystemTime.wMinute;
    Info->CreateTime.Second  = (UINT8)SystemTime.wSecond;

    PrivateFile->WinNtThunk->FileTimeToSystemTime(&FileInfo.ftLastAccessTime, &SystemTime);
    Info->LastAccessTime.Year    = SystemTime.wYear;
    Info->LastAccessTime.Month   = (UINT8)SystemTime.wMonth;
    Info->LastAccessTime.Day     = (UINT8)SystemTime.wDay;
    Info->LastAccessTime.Hour    = (UINT8)SystemTime.wHour;
    Info->LastAccessTime.Minute  = (UINT8)SystemTime.wMinute;
    Info->LastAccessTime.Second  = (UINT8)SystemTime.wSecond;

    PrivateFile->WinNtThunk->FileTimeToSystemTime(&FileInfo.ftLastWriteTime, &SystemTime);
    Info->ModificationTime.Year    = SystemTime.wYear;
    Info->ModificationTime.Month   = (UINT8)SystemTime.wMonth;
    Info->ModificationTime.Day     = (UINT8)SystemTime.wDay;
    Info->ModificationTime.Hour    = (UINT8)SystemTime.wHour;
    Info->ModificationTime.Minute  = (UINT8)SystemTime.wMinute;
    Info->ModificationTime.Second  = (UINT8)SystemTime.wSecond;

    if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
      Info->Attribute |= EFI_FILE_ARCHIVE;
    }
    if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
      Info->Attribute |= EFI_FILE_HIDDEN;
    }
    if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
      Info->Attribute |= EFI_FILE_READ_ONLY;
    }
    if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
      Info->Attribute |= EFI_FILE_SYSTEM;
    }
    if (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      Info->Attribute |= EFI_FILE_DIRECTORY;
    }

    if (PrivateFile->IsDirectoryPath) {
      Info->Attribute |= EFI_FILE_DIRECTORY;
    }

    EfiCopyMem ((CHAR8 *) Buffer + Size, PrivateFile->FileName, NameSize);
  }

  *BufferSize = ResultSize;
  return Status;
}

EFI_STATUS
WinNtSimpleFileSystemGetInfo (
  IN     EFI_FILE  *This,
  IN     EFI_GUID  *InformationType,
  IN OUT UINTN     *BufferSize,
  OUT    VOID      *Buffer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS               Status;
  WIN_NT_EFI_FILE_PRIVATE  *PrivateFile;
  EFI_FILE_SYSTEM_INFO     *FileSystemInfoBuffer;
  UINT32                   SectorsPerCluster;
  UINT32                   BytesPerSector;
  UINT32                   FreeClusters;
  UINT32                   TotalClusters;
  UINT32                   BytesPerCluster;
  CHAR16                   *DriveName;
  BOOLEAN                  DriveNameFound;
  BOOL                     NtStatus;
  UINTN                    Index;
  
  PrivateFile =  WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);

  Status = EFI_UNSUPPORTED;
  if (EfiCompareGuid (InformationType, &gEfiFileInfoGuid)) {
    Status = WinNtSimpleFileSystemFileInfo (PrivateFile, BufferSize, Buffer);
  }
  if (EfiCompareGuid (InformationType, &gEfiFileInfoIdGuid)) {
    if (*BufferSize < SIZE_OF_EFI_FILE_SYSTEM_INFO + sizeof(L"EFI_EMULATED")) {
      return EFI_BUFFER_TOO_SMALL;
    }
    FileSystemInfoBuffer = (EFI_FILE_SYSTEM_INFO *)Buffer;
    FileSystemInfoBuffer->Size        = SIZE_OF_EFI_FILE_SYSTEM_INFO + sizeof("EFI_EMULATED");
    FileSystemInfoBuffer->ReadOnly    = FALSE;
    
    //
    // Try to get the drive name
    //

    DriveName = NULL;
    DriveNameFound = FALSE;
    Status = gBS->AllocatePool(
                    EfiBootServicesData,
  									EfiStrSize(PrivateFile->FilePath) + 1,
                    &DriveName
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }  
    EfiStrCpy (DriveName, PrivateFile->FilePath);    
    for (Index = 0; DriveName[Index] != 0 && DriveName[Index] != ':'; Index ++) {
      ;
    }    
    if (DriveName [Index] == ':') {
      DriveName[Index + 1] = '\\';
      DriveName[Index + 2] = 0;      
      DriveNameFound = TRUE;
    } else if (DriveName[0] == '\\' && DriveName[1] == '\\') {
      for (Index = 2; DriveName[Index] != 0 && DriveName[Index] != '\\'; Index ++) {
        ;
      }
      if (DriveName[Index] == '\\') {
        DriveNameFound = TRUE;
        for (Index++; DriveName[Index] != 0 && DriveName[Index] != '\\'; Index ++) {
          ;
        }
        DriveName[Index] = '\\';
        DriveName[Index + 1] = 0;
      }              
    }    
    
    //
    // Try GetDiskFreeSpace first
    //

    NtStatus = PrivateFile->WinNtThunk->GetDiskFreeSpace (
                               DriveNameFound ? DriveName : NULL,                               
                               &SectorsPerCluster,
                               &BytesPerSector,
                               &FreeClusters,
                               &TotalClusters
                               );
    if (DriveName) {
      gBS->FreePool (DriveName);
    }
    
    if (NtStatus) {
      
      //
      // Succeeded
      //

      BytesPerCluster = BytesPerSector * SectorsPerCluster;
      FileSystemInfoBuffer->VolumeSize  = DriverLibMultU64x32 (TotalClusters, BytesPerCluster);
      FileSystemInfoBuffer->FreeSpace   = DriverLibMultU64x32 (FreeClusters, BytesPerCluster);
      FileSystemInfoBuffer->BlockSize   = BytesPerCluster;

    } else {
  
    
      //
      // try GetDiskFreeSpaceEx then
      //
      
      FileSystemInfoBuffer->BlockSize = 0;
      NtStatus = PrivateFile->WinNtThunk->GetDiskFreeSpaceEx (
                                            PrivateFile->FilePath,
                                            (PULARGE_INTEGER)(&FileSystemInfoBuffer->FreeSpace),
                                            (PULARGE_INTEGER)(&FileSystemInfoBuffer->VolumeSize),
                                            NULL
                                            );
      if (!NtStatus)  {
        return EFI_DEVICE_ERROR;
      }
    }
  
    EfiStrCpy((CHAR16 *)FileSystemInfoBuffer->VolumeLabel,L"EFI_EMULATED");
    *BufferSize = SIZE_OF_EFI_FILE_SYSTEM_INFO + sizeof(L"EFI_EMULATED");
    Status = EFI_SUCCESS;
  }
  if (EfiCompareGuid (InformationType, &gEfiFileSystemVolumeLabelInfoIdGuid)) {
    if (*BufferSize < sizeof(L"EFI_EMULATED")) {
      return EFI_BUFFER_TOO_SMALL;
    }
    EfiStrCpy((CHAR16 *)Buffer,L"EFI_EMULATED");
    *BufferSize = sizeof(L"EFI_EMULATED");
    Status = EFI_SUCCESS;
  }

  return Status;
}

EFI_STATUS
WinNtSimpleFileSystemSetInfo (
  IN EFI_FILE  *This,
  IN EFI_GUID         *InformationType,
  IN UINTN            BufferSize,
  IN VOID             *Buffer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS               Status;
  WIN_NT_EFI_FILE_PRIVATE  *PrivateFile;
  EFI_FILE_INFO            *Info;
  UINT64                   CurPos;
  WCHAR                    OriginalFileName[256];
  WCHAR                    NewFileName[256];
  BOOL                     NtStatus;
  UINTN                    Index;

  PrivateFile = WIN_NT_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);

  //
  //  We only support GenericFileInfo.FileSize for regular files on NT for now
  //

  Status = EFI_UNSUPPORTED;
  if (PrivateFile->IsDirectoryPath) {
    goto Done;
  }
  
  if ( EfiCompareGuid( InformationType, &gEfiFileInfoGuid ) ) {

      Info = Buffer;

      for (Index = 1; Info->FileName[Index]; Index++) {
        if (Info->FileName[Index] == '\\') {
          goto Done;
        }
      }

      Status = EFI_SUCCESS;
      //
      // Set the New File Name
      //
      
      PrivateFile->WinNtThunk->CloseHandle (PrivateFile->LHandle);
      
      EfiStrCpy (OriginalFileName, PrivateFile->FilePath);
      EfiStrCat (OriginalFileName, L"\\");
      EfiStrCat (OriginalFileName, PrivateFile->FileName);

      EfiStrCpy (NewFileName, PrivateFile->FilePath);
      EfiStrCat (NewFileName, L"\\");
      if (Info->FileName[0] == '\\') {
        EfiStrCat (NewFileName, &(Info->FileName[1]));
      } else {
        EfiStrCat (NewFileName, Info->FileName);
      }

      NtStatus = PrivateFile->WinNtThunk->MoveFile (OriginalFileName, NewFileName);
      if (!NtStatus) {
        Status = EFI_ACCESS_DENIED;
        goto Done;
      }     
      
      gBS->FreePool (PrivateFile->FileName);

      Status = gBS->AllocatePool(
                      EfiBootServicesData,
									    EfiStrSize(Info->FileName),
                      &PrivateFile->FileName
                      );
      if (EFI_ERROR (Status)) {
        goto Done;
      }
      EfiStrCpy (PrivateFile->FileName, Info->FileName);

      PrivateFile->LHandle = PrivateFile->WinNtThunk->CreateFile (
                                                          NewFileName,
                                                          GENERIC_READ | GENERIC_WRITE,
                                                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                          NULL,
                                                          OPEN_EXISTING,
                                                          0,
                                                          NULL
                                                          );

      if (PrivateFile->LHandle == INVALID_HANDLE_VALUE) {
        Status = EFI_DEVICE_ERROR;
        goto Done;
      }

      //
      //  Flush buffers just in case
      //
      if ( PrivateFile->WinNtThunk->FlushFileBuffers( PrivateFile->LHandle ) == 0 ) {
        Status = EFI_DEVICE_ERROR;
        goto Done;
      }

      //
      //  Set the file size
      //
      Status = This->GetPosition( This, &CurPos );
      if ( EFI_ERROR(Status) ) {
        goto Done;
      }

      Status = This->SetPosition( This, Info->FileSize );
      if ( EFI_ERROR(Status) ) {
        goto Done;
      }

      if ( PrivateFile->WinNtThunk->SetEndOfFile( PrivateFile->LHandle ) == 0 ) {
        Status = EFI_DEVICE_ERROR;
        goto Done;
      }

      Status = This->SetPosition( This, CurPos );
      if ( EFI_ERROR(Status) ) {
        goto Done;
      }
  }
  
Done:
  return Status;
}

EFI_STATUS
WinNtSimpleFileSystemFlush (
  IN EFI_FILE  *This
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  return EFI_SUCCESS;
}


