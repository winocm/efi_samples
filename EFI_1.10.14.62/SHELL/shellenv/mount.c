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

  mount.c
  
Abstract:
  Mount a file system on removable media   

Revision History

--*/

#include "shelle.h"

static
BOOLEAN 
NameHasSpace (
  IN CHAR16           *Name
  )
{
  CHAR16  *Ptr;

  //
  // forbid space char inside name.
  //  
  for (Ptr = Name; *Ptr; Ptr += 1) {
    if (*Ptr == ' ') {
      return FALSE;
    }
  }
  return TRUE;
}


EFI_STATUS
SEnvCmdMount (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Mount BlockDeviceName.

Arguments:

Returns:

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;          
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;
  EFI_HANDLE                Handle;
  UINT8                     *Buffer;

  DEFAULT_CMD               Cmd;
  UINTN                     HandleNo;
  CHAR16                    *Sname;

  InitializeShellApplication (ImageHandle, SystemTable);

  if ( SI->Argc < 2 ) {
    Print (L"mount: Too few arguments\n");
    return EFI_INVALID_PARAMETER;
  } else if (SI->Argc == 2) {
    Sname = NULL;
  } else if (SI->Argc == 3) {
    Sname = SI->Argv[2];
  } else {
    Print (L"mount: Too many arguments\n");
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check for the device mapping
  //
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (SI->Argv[1]);
  if (DevicePath == NULL) {
    Status = EFI_INVALID_PARAMETER;
    Print (L"mount: Invalid device mapping %hs - %r\n", SI->Argv[1], Status);
    return Status;
  }

  Status = BS->LocateDevicePath (&gEfiBlockIoProtocolGuid, 
                                 &DevicePath, 
                                 &Handle);
  if (EFI_ERROR(Status)) {
    Print (L"mount: Device Path Not a BlockIo Device - %r", Status);
    return Status;
  }

  Status = BS->HandleProtocol (Handle, &gEfiBlockIoProtocolGuid, (VOID*)&BlkIo);
  if (EFI_ERROR(Status)) {
    Print (L"mount: Device Not a BlockIo Device - %r", Status);
    return Status;
  }

  //
  //
  //
  Buffer = AllocatePool (BlkIo->Media->BlockSize);
  if (Buffer == NULL) {
    Print (L"mount: Out of memory\n");
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // In EFI the file system driver registers to get notified when DiskIo 
  // Devices are added to the system. DiskIo devices register to get notified
  // when BlkIo devices are added to the system. So when a block device shows
  // up a DiskIo is added, and then a file system if added if the media 
  // contains a file system. This works great, but when you boot with no media
  // in the device and then put media in the device there is no way to make 
  // the notify happen.
  //
  // This code reads a block device. If the BlkIo device returns 
  // EFI_MEDIA_CHANGE then it must reinstall in BlkIo protocol. This cause the 
  // notifies that make the file system show up. The 4 loops is just a guess it
  // has no magic meaning. 
  //
  //  for (Count = 0; Count < 4; Count++) {
  //    Status = BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, 
  //                                0, BlkIo->Media->BlockSize, Buffer);
  //    if (Status == EFI_MEDIA_CHANGED) {
  //      Print (L"\nMedia Changed - File System will attempt to mount\n");
  //      break;
  //    } else {
      Print (L"\n%r - Force file system to mount\n",Status);
      BlkIo->FlushBlocks (BlkIo);
      BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, 0, 
                         BlkIo->Media->BlockSize, Buffer);
  //          BlkIo->Media->MediaId++;
      BS->ReinstallProtocolInterface (Handle, &gEfiBlockIoProtocolGuid, 
                                      BlkIo, BlkIo);
  //            break;
  //        }
  //    }

  if (Sname) {
    SEnvLoadHandleTable();

    for (HandleNo=0; HandleNo < SEnvNoHandles; HandleNo++) {
      if (Handle == SEnvHandles[HandleNo]) {
        break;
      }
    }
    HandleNo += 1;

    Print (L"\nmap %s %x\n", Sname, HandleNo);
    Cmd.Line = Cmd.Buffer;
    if (! NameHasSpace (Sname)) {
      SPrint (Cmd.Line, sizeof(Cmd.Buffer), L"map \"%s\" %x", Sname, HandleNo);
    } else {
      SPrint (Cmd.Line, sizeof(Cmd.Buffer), L"map %s %x", Sname, HandleNo);
    }
    SEnvExecute (ImageHandle, Cmd.Line, TRUE);

    SEnvFreeHandleTable();
  }

  //
  // This print is for debug. From the BlkIo protocol you can do a memory dump
  //  and get the instance data.
  //
  Print (L"\n%EDebug Code%N Handle -> 0x%08x BlkIo -> 0x%08x\n", Handle, BlkIo);
  FreePool (Buffer);
  return EFI_SUCCESS;
}
