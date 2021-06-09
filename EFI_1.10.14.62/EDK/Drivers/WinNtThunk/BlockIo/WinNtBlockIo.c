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

  WinNtBlockIo.c

Abstract:

  Produce block IO abstractions for real devices on your PC using Win32 APIs.
  The configuration of what devices to mount or emulate comes from NT 
  environment variables. The variables must be visible to the Microsoft* 
  Developer Studio for them to work.

  <F>ixed       - Fixed disk like a hard drive.
  <R>emovable   - Removable media like a floppy or CD-ROM.
  Read <O>nly   - Write protected device.
  Read <W>rite  - Read write device.
  <block count> - Decimal number of blocks a device supports.
  <block size>  - Decimal number of bytes per block.

  NT envirnonment variable contents. '<' and '>' are not part of the variable, 
  they are just used to make this help more readable. There should be no 
  spaces between the ';'. Extra spaces will break the variable. A '!' is 
  used to seperate multiple devices in a variable.

  EFI_WIN_NT_VIRTUAL_DISKS = 
    <F | R><O | W>;<block count>;<block size>[!...]

  EFI_WIN_NT_PHYSICAL_DISKS =
    <drive letter>:<F | R><O | W>;<block count>;<block size>[!...]

  Virtual Disks: These devices use a file to emulate a hard disk or removable
                 media device. 
                 
    Thus a 20 MB emulated hard drive would look like:
    EFI_WIN_NT_VIRTUAL_DISKS=FW;40960;512

    A 1.44MB emulated floppy with a block size of 1024 would look like:
    EFI_WIN_NT_VIRTUAL_DISKS=RW;1440;1024

  Physical Disks: These devices use NT to open a real device in your system

    Thus a 120 MB floppy would look like:
    EFI_WIN_NT_PHYSICAL_DISKS=B:RW;245760;512

    Thus a standard CD-ROM floppy would look like:
    EFI_WIN_NT_PHYSICAL_DISKS=Z:RO;307200;2048


  * Other names and brands may be claimed as the property of others.

--*/

#include "WinNtBlockIo.h"

EFI_DRIVER_BINDING_PROTOCOL gWinNtBlockIoDriverBinding = {
  WinNtBlockIoDriverBindingSupported,
  WinNtBlockIoDriverBindingStart,
  WinNtBlockIoDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT (InitializeWinNtBlockIo)

EFI_STATUS
InitializeWinNtBlockIo (
  IN EFI_HANDLE			    	ImageHandle,
  IN EFI_SYSTEM_TABLE			*SystemTable
  )
/*++

Routine Description:

  Intialize Win32 windows to act as EFI SimpleTextOut and SimpleTextIn windows. . 

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  EFI_STATUS

--*/
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle, 
           SystemTable, 
           &gWinNtBlockIoDriverBinding, 
           ImageHandle,
           &gWinNtBlockIoComponentName,
           &gWinNtBlockIoDriverConfiguration,
           &gWinNtBlockIoDriverDiagnostics
           );
}

EFI_STATUS
WinNtBlockIoDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                        Status;
  EFI_WIN_NT_IO_PROTOCOL *WinNtIo;
  
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiWinNtIoProtocolGuid,  
                  &WinNtIo,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Make sure the WinNtThunkProtocol is valid 
  //
  Status = EFI_UNSUPPORTED;
  if (WinNtIo->WinNtThunk->Signature == EFI_WIN_NT_THUNK_PROTOCOL_SIGNATURE) {

    //
    // Check the GUID to see if this is a handle type the driver supports
    //
    if (EfiCompareGuid (WinNtIo->TypeGuid, &gEfiWinNtVirtualDisksGuid) ||
        EfiCompareGuid (WinNtIo->TypeGuid, &gEfiWinNtPhysicalDisksGuid) ) {
      Status = EFI_SUCCESS;
    }
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         Handle,   
         &gEfiWinNtIoProtocolGuid,  
         This->DriverBindingHandle,   
         Handle   
        );

  return Status;
}

EFI_STATUS
WinNtBlockIoDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                        Status;
  EFI_WIN_NT_IO_PROTOCOL *WinNtIo;
  WIN_NT_RAW_DISK_DEVICE_TYPE       DiskType;                      
  UINT16                            Buffer[FILENAME_BUFFER_SIZE];
  CHAR16                            *Str;
  BOOLEAN                           RemovableMedia;
  BOOLEAN                           WriteProtected;
  UINTN                             NumberOfBlocks;
  UINTN                             BlockSize;
 
  //
  // Grab the protocols we need
  //
  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiWinNtIoProtocolGuid,  
                  &WinNtIo,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Set DiskType
  //
  if (EfiCompareGuid (WinNtIo->TypeGuid, &gEfiWinNtVirtualDisksGuid)) {
    DiskType = EfiWinNtVirtualDisks;
  } else if (EfiCompareGuid (WinNtIo->TypeGuid, &gEfiWinNtPhysicalDisksGuid)) {
    DiskType = EfiWinNtPhysicalDisks;
  } else {
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Status = EFI_NOT_FOUND;
  Str = WinNtIo->EnvString;
  if (DiskType == EfiWinNtVirtualDisks) {
    WinNtIo->WinNtThunk->SPrintf (
                                            Buffer, L"Diskfile%d", 
                                            WinNtIo->InstanceNumber
                                            );
  } else {
    if (*Str >= 'A' && *Str <= 'Z' || *Str >= 'a' && *Str <= 'z') {
      WinNtIo->WinNtThunk->SPrintf (Buffer, L"\\\\.\\%c:", *Str);
    } else {
      WinNtIo->WinNtThunk->SPrintf (Buffer, L"\\\\.\\PHYSICALDRIVE%c", *Str);
    }
    
    Str++;
    if (*Str != ':') {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
    Str++;
  }

  if (*Str == 'R' || *Str == 'F') {
    RemovableMedia = (BOOLEAN)(*Str == 'R'); 
    Str++;
    if (*Str == 'O' || *Str == 'W') {
      WriteProtected = (BOOLEAN)(*Str == 'O');
      Str = GetNextElementPastTerminator (Str, ';');
      
      NumberOfBlocks = Atoi (Str);
      if (NumberOfBlocks != 0) {
        Str = GetNextElementPastTerminator (Str, ';');
        BlockSize = Atoi (Str);
        if (BlockSize != 0) {
          //
          // If we get here the variable is valid so do the work.
          //
          Status = WinNtBlockIoCreateMapping (
                      WinNtIo,
                      Handle,
                      Buffer,
                      WriteProtected,
                      RemovableMedia,
                      NumberOfBlocks,
                      BlockSize,
                      DiskType
                      );

        }
      }
    }
  }  

Done:
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
           Handle,   
           &gEfiWinNtIoProtocolGuid,  
           This->DriverBindingHandle,   
           Handle
           );
  }
  return Status;
}

EFI_STATUS
WinNtBlockIoDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
{
  EFI_BLOCK_IO_PROTOCOL             *BlockIo;
  EFI_STATUS                        Status;
  WIN_NT_BLOCK_IO_PRIVATE           *Private;
  
  //
  // Get our context back
  //
  Status = gBS->OpenProtocol (
                  Handle,   
                  &gEfiBlockIoProtocolGuid,  
                  &BlockIo,
                  This->DriverBindingHandle,   
                  Handle,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Private = WIN_NT_BLOCK_IO_PRIVATE_DATA_FROM_THIS (BlockIo);


  //
  // BugBug: If we need to kick people off, we need to make Uninstall Close the handles. 
  //         We could pass in our image handle or FLAG our open to be closed via 
  //         Unistall (== to saying any CloseProtocol will close our open)
  //

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Private->EfiHandle, 
                  &gEfiBlockIoProtocolGuid, &Private->BlockIo,
                  NULL
                  );
  if (!EFI_ERROR (Status)) {

    Status = gBS->CloseProtocol (
                    Handle, 
                    &gEfiWinNtIoProtocolGuid, 
                    This->DriverBindingHandle,   
                    Handle
                    );

    //
    // Shut down our device
    //
    Private->WinNtThunk->CloseHandle (Private->NtHandle);

    //
    // Free our instance data
    //
    EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);

    gBS->FreePool (Private);
  }

  return Status;
}

STATIC
CHAR16 *
GetNextElementPastTerminator (
  IN  CHAR16  *EnvironmentVariable,
  IN  CHAR16  Terminator
  )
/*++

Routine Description:

  Worker function to parse environment variables.

Arguments:
  EnvironmentVariable - Envirnment variable to parse.

  Terminator          - Terminator to parse for.

Returns: 

  Pointer to next eliment past the first occurence of Terminator or the '\0'
  at the end of the string.

--*/
{
  CHAR16 *Ptr;

  for (Ptr = EnvironmentVariable; *Ptr != '\0'; Ptr++) {
    if (*Ptr == Terminator) {
      Ptr++;
      break;
    }
  }
  return Ptr;
}

STATIC
EFI_STATUS
WinNtBlockIoCreateMapping (
  IN EFI_WIN_NT_IO_PROTOCOL  *WinNtIo,
  IN EFI_HANDLE                         EfiDeviceHandle,
  IN CHAR16                             *Filename,
  IN BOOLEAN                            ReadOnly,
  IN BOOLEAN                            RemovableMedia,
  IN UINTN                              NumberOfBlocks,
  IN UINTN                              BlockSize,
  IN WIN_NT_RAW_DISK_DEVICE_TYPE        DeviceType
  )
{
  EFI_STATUS              Status;
  EFI_BLOCK_IO_PROTOCOL   *BlockIo;
  WIN_NT_BLOCK_IO_PRIVATE *Private;
  UINTN                   Index;
    
  WinNtIo->WinNtThunk->SetErrorMode (SEM_FAILCRITICALERRORS);

	Status = gBS->AllocatePool(
                  EfiBootServicesData,
									sizeof(WIN_NT_BLOCK_IO_PRIVATE), 
                  &Private
                  );
  ASSERT_EFI_ERROR(Status);

  EfiInitializeLock (&Private->Lock, EFI_TPL_NOTIFY);

  Private->WinNtThunk = WinNtIo->WinNtThunk;

  Private->Signature  = WIN_NT_BLOCK_IO_PRIVATE_SIGNATURE;
  Private->LastBlock  = NumberOfBlocks - 1;
  Private->BlockSize  = BlockSize;

  for (Index = 0; Filename[Index] != 0; Index++) {
    Private->Filename[Index] = Filename[Index];
  }
  Private->Filename[Index] = 0;

  Private->ReadMode   = GENERIC_READ | (ReadOnly ? 0 : GENERIC_WRITE);
  Private->ShareMode  = FILE_SHARE_READ | FILE_SHARE_WRITE;

  Private->NumberOfBlocks = NumberOfBlocks;
  Private->DeviceType     = DeviceType;
  Private->NtHandle       = INVALID_HANDLE_VALUE;

  Private->ControllerNameTable = NULL;

  EfiLibAddUnicodeString (
    "eng", 
    gWinNtBlockIoComponentName.SupportedLanguages, 
    &Private->ControllerNameTable, 
    Private->Filename
    );

  BlockIo                   = &Private->BlockIo;
  BlockIo->Revision         = EFI_BLOCK_IO_PROTOCOL_REVISION;
  BlockIo->Media            = &Private->Media;
  BlockIo->Media->BlockSize = Private->BlockSize;
  BlockIo->Media->LastBlock = Private->NumberOfBlocks - 1;
  BlockIo->Media->MediaId   = 0;;

  BlockIo->Reset        = WinNtBlockIoResetBlock;
  BlockIo->ReadBlocks   = WinNtBlockIoReadBlocks;
  BlockIo->WriteBlocks  = WinNtBlockIoWriteBlocks;
  BlockIo->FlushBlocks  = WinNtBlockIoFlushBlocks;

  BlockIo->Media->ReadOnly          = ReadOnly;
  BlockIo->Media->RemovableMedia    = RemovableMedia;
  BlockIo->Media->LogicalPartition  = FALSE;
  BlockIo->Media->MediaPresent      = TRUE;

  if (DeviceType == EfiWinNtVirtualDisks) {
    BlockIo->Media->IoAlign = 1;

    //
    // Create a file to use for a virtual disk even if it does not exist.
    //
    Private->OpenMode = OPEN_ALWAYS;
  } else if (DeviceType == EfiWinNtPhysicalDisks) {
    //
    // Physical disk and floppy devices require 4 byte alignment.
    //
    BlockIo->Media->IoAlign = 4;

    //
    // You can only open a physical device if it exists.
    //
    Private->OpenMode = OPEN_EXISTING;
  } else {
    ASSERT(FALSE);
  }

  Private->EfiHandle = EfiDeviceHandle;
  Status = WinNtBlockIoOpenDevice (Private);
  if (!EFI_ERROR (Status)) {

    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Private->EfiHandle,       
                    &gEfiBlockIoProtocolGuid, &Private->BlockIo,
                    NULL
                    );
    ASSERT(!EFI_ERROR(Status));
    DEBUG ((EFI_D_INIT, "BlockDevice added: %s\n", Filename));
  }

  return Status;
}


STATIC
EFI_STATUS
WinNtBlockIoOpenDevice (
  WIN_NT_BLOCK_IO_PRIVATE                 *Private
  )
{
  EFI_STATUS            Status;
  UINT64                 FileSize;
  UINT64                 EndOfFile;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;

  BlockIo = &Private->BlockIo;
  EfiAcquireLock (&Private->Lock);

  //
  // If the device is already opened, close it
  //
  if (Private->NtHandle != INVALID_HANDLE_VALUE) {
    BlockIo->Reset(BlockIo, FALSE);
  }

  //
  // Open the device
  //   
  Private->NtHandle = Private->WinNtThunk->CreateFile (
                                        Private->Filename,
                                        Private->ReadMode,
                                        Private->ShareMode,
                                        NULL,
                                        Private->OpenMode, 
                                        0,
                                        NULL
                                        );

  Status = Private->WinNtThunk->GetLastError();

  if (Private->NtHandle == INVALID_HANDLE_VALUE) {
    DEBUG((EFI_D_INFO, "PlOpenBlock: Could not open %s, %x\n", Private->Filename, Private->WinNtThunk->GetLastError()));
    BlockIo->Media->MediaPresent = FALSE;
    Status = EFI_NO_MEDIA;
    goto Done;
  }
  
  if (!BlockIo->Media->MediaPresent) {
    //
    // BugBug: try to emulate if a CD appears - notify drivers to check it out
    //
    BlockIo->Media->MediaPresent = TRUE;
    EfiReleaseLock (&Private->Lock);
//    gBS->ReinstallProtocolInterface (Private->EfiHandle, &gEfiBlockIoProtocolGuid, BlockIo, BlockIo);
    EfiAcquireLock (&Private->Lock);
  }

  //
  // get the size of the file
  //
  //FileSizeLow = Private->WinNtThunk->SetFilePointer (Private->NtHandle, 0, NULL, FILE_END);
  Status = SetFilePointer64 (Private, 0, &FileSize, FILE_END);
  
  if (EFI_ERROR(Status)) {
    FileSize = DriverLibMultU64x32 (Private->NumberOfBlocks, Private->BlockSize);
    if (Private->DeviceType == EfiWinNtVirtualDisks) {
      DEBUG ((EFI_D_ERROR, "PlOpenBlock: Could not get filesize of %s\n", Private->Filename));
      Status = EFI_UNSUPPORTED;
      goto Done;
    }
  }

  if (Private->NumberOfBlocks == 0) {
    Private->NumberOfBlocks = DriverLibDivU64x32 (FileSize, Private->BlockSize, NULL);
  }
  EndOfFile = DriverLibMultU64x32 (Private->NumberOfBlocks, Private->BlockSize);
    
  if (FileSize != EndOfFile) {
    // file is not the proper size, change it
    DEBUG ((EFI_D_INIT, "PlOpenBlock: Initializing block device: %hs\n", Private->Filename));

    //
    // first set it to 0
    //
    //Private->WinNtThunk->SetFilePointer (Private->NtHandle, 0, NULL, FILE_BEGIN);
    SetFilePointer64 (Private, 0, NULL, FILE_BEGIN);
    Private->WinNtThunk->SetEndOfFile (Private->NtHandle);

    //
    // then set it to the needed file size (OS will zero fill it)
    //
    //Private->WinNtThunk->SetFilePointer (Private->NtHandle, EndOfFile, NULL, FILE_BEGIN);
    SetFilePointer64 (Private, EndOfFile, NULL, FILE_BEGIN);
    Private->WinNtThunk->SetEndOfFile (Private->NtHandle);
  }

  DEBUG((EFI_D_INIT, "%HPlOpenBlock: opened %s%N\n", Private->Filename));
  Status = EFI_SUCCESS;
    
Done:
  if (EFI_ERROR(Status)) {
    if (Private->NtHandle != INVALID_HANDLE_VALUE) {
      BlockIo->Reset (BlockIo, FALSE);
    }
  } 

  EfiReleaseLock (&Private->Lock);
  return Status;
}

STATIC
EFI_STATUS
WinNtBlockIoError (
  IN WIN_NT_BLOCK_IO_PRIVATE      *Private
  )
{
  EFI_BLOCK_IO_PROTOCOL   *BlockIo;
  EFI_STATUS              Status;
  BOOLEAN                 ReinstallBlockIoFlag;

  BlockIo = &Private->BlockIo;

  switch (Private->WinNtThunk->GetLastError()) {

  case ERROR_NOT_READY:
    BlockIo->Media->ReadOnly = FALSE;
    BlockIo->Media->MediaPresent = FALSE;
    ReinstallBlockIoFlag = FALSE;
    Status = EFI_DEVICE_ERROR;
    break;

  case ERROR_WRONG_DISK:
    BlockIo->Media->ReadOnly = FALSE;
    BlockIo->Media->MediaPresent = TRUE;
    BlockIo->Media->MediaId += 1;
    ReinstallBlockIoFlag = TRUE;
    Status = EFI_MEDIA_CHANGED;
    break;

  case ERROR_WRITE_PROTECT:
    BlockIo->Media->ReadOnly = TRUE;
    ReinstallBlockIoFlag = FALSE;
    Status = EFI_WRITE_PROTECTED;
    break;

  default:
    ReinstallBlockIoFlag = FALSE;
    Status = EFI_DEVICE_ERROR;
    break;
  }

  if (ReinstallBlockIoFlag) {
    BlockIo->Reset (BlockIo, FALSE);
    
    gBS->ReinstallProtocolInterface (
          Private->EfiHandle, 
          &gEfiBlockIoProtocolGuid, 
          BlockIo, 
          BlockIo
          );
  }
  return Status;
}

STATIC
EFI_STATUS
WinNtBlockIoReadWriteCommon (
  IN  WIN_NT_BLOCK_IO_PRIVATE     *Private,
  IN UINT32                 MediaId,
  IN EFI_LBA      Lba,
  IN UINTN        BufferSize,
  IN VOID         *Buffer,
  IN BOOLEAN      Write,
  IN CHAR8        *CallerName 
  )
{
  EFI_STATUS  Status;
  EFI_BLOCK_IO_MEDIA      *Media;
  UINTN       BlockSize;
  UINT64      LastBlock;
  INT64       DistanceToMove;
  UINT64       DistanceMoved;

  if (Private->NtHandle == INVALID_HANDLE_VALUE) {
    Status = WinNtBlockIoOpenDevice (Private);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }

  Media = Private->BlockIo.Media;

  if (!(Media->MediaPresent)) {
    Status = EFI_NO_MEDIA;
  }    
  
  if (MediaId != Media->MediaId) {
    Status = EFI_MEDIA_CHANGED;
  }

  if (Write) {
    if (Media->ReadOnly) {
      return EFI_WRITE_PROTECTED;
    }
  }

  //
  // Verify buffer size
  //
  BlockSize = Private->BlockSize;

  LastBlock = Lba + (BufferSize/BlockSize) - 1;
  if (LastBlock > Private->LastBlock) {
    DEBUG ((EFI_D_INIT, "ReadBlocks: Attempted to read off end of device\n"));
    return EFI_INVALID_PARAMETER;
  }

  if ((BufferSize % BlockSize) != 0) {
    DEBUG ((EFI_D_INIT, "%s: Invalid read size\n", CallerName));
    return EFI_BAD_BUFFER_SIZE;
  }

  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {
    DEBUG ((EFI_D_INIT, "%s: Zero length read\n", CallerName));
    return EFI_SUCCESS;
  }

  //
  // Seek to End of File
  //
  DistanceToMove = DriverLibMultU64x32 (Lba, BlockSize);
  //DistanceMoved = Private->WinNtThunk->SetFilePointer (Private->NtHandle, DistanceToMove, NULL, FILE_BEGIN);
  Status = SetFilePointer64 (Private, DistanceToMove, &DistanceMoved, FILE_BEGIN);
  
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_INIT, "WriteBlocks: SetFilePointer failed\n"));
    return WinNtBlockIoError (Private);
  }

  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
EFIAPI
WinNtBlockIoReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,
  OUT VOID                  *Buffer
  )
/*++

  Routine Description:
    Read BufferSize bytes from Lba into Buffer.

  Arguments:
    This       - Protocol instance pointer.
    MediaId    - Id of the media, changes every time the media is replaced.
    Lba        - The starting Logical Block Address to read from
    BufferSize - Size of Buffer, must be a multiple of device block size.
    Buffer     - Buffer containing read data

  Returns:
    EFI_SUCCES            - The data was read correctly from the device.
    EFI_DEVICE_ERROR      - The device reported an error while performing the read.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHANGED     - The MediaId does not matched the current device.
    EFI_BAD_BUFFER_SIZE   - The Buffer was not a multiple of the block size of the 
                            device.
    EFI_INVALID_PARAMETER - The read request contains device addresses that are not 
                            valid for the device.

--*/
{
  WIN_NT_BLOCK_IO_PRIVATE *Private;
  BOOL                    Flag;
  EFI_STATUS              Status;
  DWORD                   BytesRead;

  Private = WIN_NT_BLOCK_IO_PRIVATE_DATA_FROM_THIS(This);

  Status = WinNtBlockIoReadWriteCommon (Private, MediaId, Lba, BufferSize, Buffer, FALSE, "WinNtReadBlocks");
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Flag = Private->WinNtThunk->ReadFile (Private->NtHandle, Buffer, (DWORD)BufferSize, (LPDWORD)&BytesRead, NULL);
  if (!Flag || (BytesRead != BufferSize)) {
    DEBUG ((EFI_D_INIT, "ReadBlocks: ReadFile failed. (%d)\n", Private->WinNtThunk->GetLastError()));
    return WinNtBlockIoError (Private);
  }

  //
  // If we wrote then media is present.
  //
  This->Media->MediaPresent = TRUE;
  return EFI_SUCCESS;
}



STATIC
EFI_STATUS
EFIAPI
WinNtBlockIoWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,
  IN VOID                   *Buffer
  )
/*++

  Routine Description:
    Write BufferSize bytes from Lba into Buffer.

  Arguments:
    This       - Protocol instance pointer.
    MediaId    - Id of the media, changes every time the media is replaced.
    Lba        - The starting Logical Block Address to read from
    BufferSize - Size of Buffer, must be a multiple of device block size.
    Buffer     - Buffer containing read data

  Returns:
    EFI_SUCCES            - The data was written correctly to the device.
    EFI_WRITE_PROTECTED   - The device can not be written to.
    EFI_DEVICE_ERROR      - The device reported an error while performing the write.
    EFI_NO_MEDIA          - There is no media in the device.
    EFI_MEDIA_CHNAGED     - The MediaId does not matched the current device.
    EFI_BAD_BUFFER_SIZE   - The Buffer was not a multiple of the block size of the 
                            device.
    EFI_INVALID_PARAMETER - The write request contains a LBA that is not 
                            valid for the device.

--*/
{
  WIN_NT_BLOCK_IO_PRIVATE *Private;
  UINTN                   BytesWritten;
  BOOL                    Flag;
  EFI_STATUS              Status;
    
  Private = WIN_NT_BLOCK_IO_PRIVATE_DATA_FROM_THIS(This);
  
  Status = WinNtBlockIoReadWriteCommon (Private, MediaId, Lba, BufferSize, Buffer, TRUE, "WinNtWriteBlocks");
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Flag = Private->WinNtThunk->WriteFile (Private->NtHandle, Buffer, (DWORD)BufferSize, (LPDWORD)&BytesWritten, NULL);
  if (!Flag || (BytesWritten != BufferSize)) {
    DEBUG ((EFI_D_INIT, "ReadBlocks: WriteFile failed. (%d)\n", Private->WinNtThunk->GetLastError()));
    return WinNtBlockIoError (Private);
  }

  //
  // If the write succeeded, we are not write protected and media is present.
  //
  This->Media->MediaPresent = TRUE;
  This->Media->ReadOnly = FALSE;
  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
EFIAPI
WinNtBlockIoFlushBlocks(
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
/*++

  Routine Description:
    Flush the Block Device.

  Arguments:
    This             - Protocol instance pointer.

  Returns:
    EFI_SUCCES       - All outstanding data was written to the device
    EFI_DEVICE_ERROR - The device reported an error while writting back the data
    EFI_NO_MEDIA     - There is no media in the device.

--*/
{
  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
EFIAPI
WinNtBlockIoResetBlock(
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN BOOLEAN                ExtendedVerification
  )
/*++

  Routine Description:
    Reset the Block Device.

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCES            - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could 
                            not be reset.

--*/
{
  WIN_NT_BLOCK_IO_PRIVATE *Private;
    
  Private = WIN_NT_BLOCK_IO_PRIVATE_DATA_FROM_THIS(This);

  Private->WinNtThunk->CloseHandle (Private->NtHandle);
  Private->NtHandle = INVALID_HANDLE_VALUE;
    
  return EFI_SUCCESS;
}


UINTN
Atoi (
  CHAR16  *String
  )
/*++

Routine Description:

  Convert a unicode string to a UINTN

Arguments:

  String - Unicode string.

Returns: 

  UINTN of the number represented by String.  

--*/
{
  UINTN   Number;
  CHAR16  *Str;

  //
  // skip preceeding white space
  //
  Str = String;
  while ((*Str) && (*Str == ' ')) {
    Str++;
  }

  //
  // Convert ot a Number
  //
  Number = 0;
  while (*Str != '\0') {
      if ((*Str >= '0') && (*Str <= '9')) {
          Number = (Number * 10) + *Str - '0';
      } else {
          break;
      }
      Str++;
  }

  return Number;
}


EFI_STATUS
SetFilePointer64 (
  IN  WIN_NT_BLOCK_IO_PRIVATE   *Private,
  IN  INT64                     DistanceToMove,
  OUT UINT64                     *NewFilePointer,
  IN  DWORD                     MoveMethod
  )
/*++

This function extends the capability of SetFilePointer to accept 64 bit parameters

--*/
{
  EFI_STATUS      Status;
  LARGE_INTEGER   LargeInt;
  UINT32          ErrorCode;

  LargeInt.QuadPart = DistanceToMove;
  Status = EFI_SUCCESS;
  
  LargeInt.LowPart = Private->WinNtThunk->SetFilePointer(Private->NtHandle, 
                                          LargeInt.LowPart,
                                          &LargeInt.HighPart,
                                          MoveMethod
                                          );
  
  if (LargeInt.LowPart == -1 &&
      (ErrorCode = Private->WinNtThunk->GetLastError()) != NO_ERROR)
  {
    Status = EFI_INVALID_PARAMETER;
  }
  
  if (NewFilePointer != NULL)
  {
    *NewFilePointer = LargeInt.QuadPart;
  }

  return Status;  
}

