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

  dumpbs.c
  
Abstract:
  shell command "dumpbs". 

Revision History

--*/

#include "shelle.h"
#include "efilib.h"

#define BOOT_HEADER_SIZE  512

//
// Start & End position when write the boot sector
// Fat16 index is 0, Fat32 index is 1
//
int mPositionArray[2][2] = {{3, 61},{3, 89}};

EFI_STATUS
DumpBootSector (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
GetBlkIO (
  IN  CHAR16                  *BlockId,
  OUT EFI_BLOCK_IO_PROTOCOL   **BlkIo
  );
  
VOID
ShowHelpMessage();

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(DumpBootSector)
#endif

EFI_STATUS
DumpBootSector (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Dump Data from block IO devices.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error

Notes:  
    dumpbs [[-r|-R|-d|-D]|[-w|-W -fat16|-fat32]] [BlockDeviceName] SectorFileName

--*/
{
  UINTN                     Index;
  EFI_STATUS                Status;
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;
  VOID                      *Buffer, *TempBuffer;
  EFI_LIST_ENTRY            FileList;
  EFI_FILE_HANDLE           FileHandle;
  SHELL_FILE_ARG            *FileArg;
  BOOLEAN                   IsDumping;
  BOOLEAN                   Verbose;
  BOOLEAN                   ShowHelp;
  UINTN                     BufSize;
  CHAR16                    **Argv;
  UINTN                     Argc;
  UINTN                     BlockIdIndex, FileNameIndex;
  UINTN                     PosArrayIndex, StartPos, EndPos;
  UINTN                     ParamNum;
  CHAR16                    InputString[10];

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   DumpBootSector,
    L"dumpbs",      // command
    NULL,           // command syntax
    NULL,           // 1 line descriptor
    NULL            // command help page
    );
#endif

  InitializeShellApplication (ImageHandle, SystemTable);

  ShowHelp      = FALSE;
  IsDumping     = TRUE;
  Verbose       = FALSE;
  Buffer        = NULL;
	TempBuffer    = NULL;
	FileHandle    = NULL;
  BlockIdIndex  = 0;
  FileNameIndex = 0;
  StartPos      = 0;
  EndPos        = 0;
  ParamNum      = 0;
  InputString[0]= 0;
  PosArrayIndex = -1;
  StartPos      = -1;
  EndPos        = -1;
  Argc          = SI->Argc;
  Argv          = SI->Argv;
  Status        = EFI_SUCCESS;
    
  if ( Argc < 2) {
    Print (L"dumpbs: Too few arguments\n");
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Parse parameters
  //  
  for (Index = 1; Index < Argc; Index ++) {
    if (Argv[Index][0] == '-' && Argv[Index][1] == 'd') {
      ParamNum++;
      IsDumping = TRUE;
    } else if (Argv[Index][0] == '-' && Argv[Index][1] == 'b') {
      ParamNum++;
      EnablePageBreak(DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
    } else if (Argv[Index][0] == '-' && Argv[Index][1] == 'w') {
      ParamNum++;
      IsDumping = FALSE;
    } else if (Argv[Index][0] == '-' && Argv[Index][1] == 'v') {
      ParamNum++;
      Verbose = TRUE;
    } else if(Argv[Index][0] == '-' && !StrCmp(Argv[Index]+1,L"fat16")) {
      ParamNum++;
      PosArrayIndex = 0;
    } else if(Argv[Index][0] == '-' && !StrCmp(Argv[Index]+1,L"fat32")) {
      ParamNum++;
      PosArrayIndex = 1;
    } else if (Argv[Index][1] == '?' && (Argv[Index][0] == '-' || Argv[Index][0] == '/')) {
      ShowHelp = TRUE;
    } else if(Argv[Index][0] == '-') {
      Print(L"dumpbs: Invalid parameters - %hs\n", Argv[Index]);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  
  //
  // Show help message
  //
  if(ShowHelp == TRUE) {
    ShowHelpMessage();
    return EFI_SUCCESS;
  }
  
  //
  // Verbose, show info of a block device
  //
  if(Verbose == TRUE) {
    if(Argc - ParamNum > 2) {
      Print(L"dumpbs: Too mang arguments\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } else if(Argc - ParamNum< 2) {
      Print(L"dumpbs: Too few arguments\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
    
    //
    // Get the block io
    //
    Status = GetBlkIO(Argv[Argc-1], &BlkIo);
    if(EFI_ERROR(Status)) {
      goto Done;
    }
    
    Buffer = AllocatePool (BOOT_HEADER_SIZE);
    
    //
    // Read block device's boot sector to TempBuffer
    //      
    Status = BlkIo->ReadBlocks(BlkIo, BlkIo->Media->MediaId, 0, BOOT_HEADER_SIZE, Buffer);
    if (EFI_ERROR(Status)) {
      Print(L"dumpbs: Read boot sector error - %r\n", Status);
      goto Done;
    }
    
    EFIStructsPrint(Buffer, BlkIo->Media->BlockSize, 0, BlkIo);
    
    goto Done;
  }  
  
  //
  // When write boot sector, it must specify file system format
  //
  if(IsDumping == FALSE && PosArrayIndex == -1 ) {
    Print(L"dumpbs: Invalid parameters - When write boot sector, must specify file system format\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  
  //
  // Check arguments' number
  //
  if(ParamNum == 0) {
    Print(L"dumpbs: Invalid arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  
  if(Argc-ParamNum > 3) {
    Print(L"dumpbs: Too mang arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  
  if(Argc-ParamNum < 3) {
    Print(L"dumpbs: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Set block id index and file name index
  //
  BlockIdIndex  = Argc-2;
  FileNameIndex = Argc-1;
  
  Status = GetBlkIO(Argv[BlockIdIndex], &BlkIo);
  if(EFI_ERROR(Status)) {
    goto Done;
  }
  
  //
  // Get the File from sector file name
  //
  InitializeListHead (&FileList);
  Status = ShellFileMetaArg(Argv[FileNameIndex], &FileList);
  if (EFI_ERROR(Status) || IsListEmpty(&FileList)) {
    Print (L"dumpbs: Cannot open file - %r\n", Status);
    goto Done;
  }
  
  //
  // Whether it is a multiple destinations
  //
  if (FileList.Flink->Flink != &FileList) {
    Print (L"dumpbs: Multiple destinations are not allowed\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  
  FileArg = CR(FileList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
  FileHandle = FileArg->Handle;
  
  if(IsDumping == FALSE) {
    if (FileArg->Status == EFI_SUCCESS &&
        FileArg->Handle &&
        FileArg->Info &&
        FileArg->Info->Attribute & EFI_FILE_ARCHIVE) {
      
      //
      // Write the boot sector to block device from a file
      //
            
      //
      // Prompt and wait for user's key stroke
      //
      Print(L"%HWarning: %N Are you sure to write boot sector to \"%hs\" ? (Yes/No): ", Argv[BlockIdIndex]);
      while(TRUE) {
        Input(L"", InputString, 2);
        Print(L"\n");
        
        if (InputString[0] == 'Y' || InputString[0] == 'y') {
          break;
        } else {
          Status = EFI_ABORTED;
          goto Done;
        }
      }
      
      //
      // Init
      //
      BufSize = BOOT_HEADER_SIZE;
      Buffer = AllocatePool (BOOT_HEADER_SIZE);
      TempBuffer = AllocatePool (BOOT_HEADER_SIZE);
      
      //
      // Fill StartPos and EndPos
      //
      StartPos = mPositionArray[PosArrayIndex][0];
      EndPos   = mPositionArray[PosArrayIndex][1];
      
      //
      // Set file pointer to end start position
      //
      Status = FileHandle->SetPosition(FileHandle, 0);
      if (EFI_ERROR(Status)) {
        Print(L"dumpbs: Set %hs pos error - %r\n", Argv[FileNameIndex], Status);
        goto Done;
      }
      
      //
      // Read data from file
      //    
      Status = FileHandle->Read (FileHandle, &BufSize, Buffer);
      if (EFI_ERROR(Status)) {
        Print(L"dumpbs: - Read error - %r\n", Status);
        goto Done;
      }
  
      //
      // Whether Boot sector file size is 512
      //
      if (BufSize != BOOT_HEADER_SIZE) {
        Print(L"dumpbs: Error, Boot sector file is smaller than %d bytes\n", BOOT_HEADER_SIZE);
        Status = EFI_ABORTED;
        goto Done;
      }
      
      //
      // Read block device's boot sector to TempBuffer
      //      
      Status = BlkIo->ReadBlocks(BlkIo, BlkIo->Media->MediaId, 0, BOOT_HEADER_SIZE, TempBuffer);
      if (EFI_ERROR(Status)) {
        Print(L"dumpbs: Read boot sector error - %r\n", Status);
        goto Done;
      }
      
      //
      // Block device's boot secotr has a range to be preserved
      //
      CopyMem((CHAR8*)Buffer+StartPos, (CHAR8*)TempBuffer+StartPos, EndPos-StartPos+1);
      
      //
      // Finnally write the boot sector
      //
      Status = BlkIo->WriteBlocks(BlkIo, BlkIo->Media->MediaId, 0, BOOT_HEADER_SIZE, Buffer);
      if (EFI_ERROR(Status)) {
        Print(L"dumpbs: Write boot sector error - %r\n", Status);
      } else {
        Print(L"dumpbs: Write boot sector finished\n");
      }
    } else {
      Print(L"dumpbs: Open %hs error\n", FileArg->FileName);
    }
  } else { 
    
    //
    // Dump the boot sector to file from a block device
    //
    
    if (FileArg->Status == EFI_SUCCESS &&
    FileArg->OpenMode & EFI_FILE_MODE_READ &&
    FileArg->OpenMode & EFI_FILE_MODE_WRITE) {
      //
      // Everything is ok, do nothing
      //
    } else if (FileArg->Status == EFI_NOT_FOUND) {
      
      //
      // File not found, we try to open the dst
      // The parent of the dst should have been opened once ShellFileMetaArg
      //
      
      Status = FileArg->Parent->Open(FileArg->Parent,
        &FileHandle,
        FileArg->FileName,
        EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE,
        0
        );

      if (EFI_ERROR(Status)) {
        Print (L"dumpbs: Cannot create destination %hs - %r\n", FileArg->FullName, Status);
        goto Done;
      }
    } else if (FileArg->OpenMode & EFI_FILE_MODE_READ) {
      //
      // File is read only
      //
      Print (L"dumpbs: Destination is read only or write protected\n");
      Status = EFI_ACCESS_DENIED;
      goto Done;
    } else {
      //
      // Something error
      //
      Print (L"dumpbs: Cannot open %hs - %r\n",
      FileArg->FullName, FileArg->Status);
      Status = FileArg->Status;
      goto Done;
    }
    
    //
    // Init
    //
    BufSize = BOOT_HEADER_SIZE;
    Buffer = AllocatePool (BOOT_HEADER_SIZE);
    
    //
    // Set file pointer to end start position
    //
    Status = FileHandle->SetPosition(FileHandle, 0);
    if (EFI_ERROR(Status)) {
      Print(L"dumpbs: Set %hs pos error - %r\n", Argv[FileNameIndex], Status);
      goto Done;
    }
    
    //
    // Read the block device's boot sector
    //
    Status = BlkIo->ReadBlocks(BlkIo, BlkIo->Media->MediaId, 0, BOOT_HEADER_SIZE, Buffer);
    if (EFI_ERROR(Status)) {
      Print(L"dumpbs: Read boot sector error - %r\n", Status);
      goto Done;
    }
    
    //
    // Write the boot sector to file
    //
    Status = FileHandle->Write (FileHandle, &BufSize, Buffer);
    if (EFI_ERROR(Status)) {
      Print(L"dumpbs: - Write error - %r\n", Status);
      goto Done;
    }

    //
    // Boot sector file is smaller than 512 bytes
    //
    if (BufSize != BOOT_HEADER_SIZE) {
      Print(L"dumpbs: Boot sector file is smaller than %d bytes\n", BOOT_HEADER_SIZE);
      Status = EFI_ABORTED;
      goto Done;
    }
    
    Print(L"dumpbs: Dump boot sector successfully\n");
  }
  
  //
  // Clean up routine
  //
Done:
  if(TempBuffer)
    FreePool(TempBuffer);
  if(Buffer)
    FreePool(Buffer);
  if(FileHandle)
    FileHandle->Close(FileHandle);

  return Status;
}

EFI_STATUS
GetBlkIO (
  IN  CHAR16                        *BlockId,
  OUT EFI_BLOCK_IO_PROTOCOL         **BlkIo
  )
/*++

Routine Description:

Arguments:
    
  BlockId               Pointer to the block device name
  BlkIo                 Returned block IO

Returns:

  EFI_SUCCESS             Get BlkIo successful
  EFI_INVALID_PARAMETER   Invalid BlockId

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_STATUS                Status;
    
  DevicePath  = NULL;
  
  //
  // Get the device path from block device id
  //
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)ShellGetMap (BlockId);
  if (DevicePath == NULL) {
    Print(L"dumpbs: Invalid block id \"%hs\"\n", BlockId);
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Get the block io from device path
  //
  Status = LibDevicePathToInterface (&gEfiBlockIoProtocolGuid, DevicePath, (VOID **)BlkIo);
  if (EFI_ERROR(Status)) {
    Print (L"dumpbs: Device is not a BlockIo device - %r\n", Status);
    return Status;
  }
  
  return EFI_SUCCESS;
}

VOID
ShowHelpMessage()
{
  Print(L"Dump or write the boot sector of the specified block device.\n \
  \n \
dumpbs -v|-d|(-w -fat16|-fat32) [/?|-?] [-b] BlockDeviceName SectorFileName\n \
    /?                - Show help message\n \
    -?                - Show help message\n \
    -b                - Displays one screen at a time\n \
    -v                - Display boot sector's data\n \
    -d                - Dump the boot sector to file\n \
    -w                - write boot sector from a file\n \
    -fat16            - The target block device is fat16 format\n \
    -fat32            - The target block device is fat32 format\n \
    BlockDeviceName   - Block device name\n \
    SectorFileName    - Boot sector file name\n \
  \n");
  
  Print(L"Examples:\n \
  * Display blk0's boot sector\n \
    Shell> dumpbs -v blk0\n \
    Fat 12 BPB  FatLabel: 'EFI FLOPPY '  SystemId: 'FAT12   ' OemId: 'INTEL   '\n \
    SectorSize 0x200  SectorsPerCluster 1 ReservedSectors 1  # Fats 2\n \
    Root Entries 0xE0  Media 0xF0  Sectors 0xB40  SectorsPerFat 0x9\n \
    SectorsPerTrack 0x12 Heads 2\n \
  \n \
  * Show help message one screen at a time\n \
    Shell> dumpbs -? -b\n \
  \n \
  * Dump blk0's boot sector  to file\n \
    Shell> dumpbs -d blk0 fs0:\\dumpbs.out\n \
  \n \
  * Write a fat16 format to blk0's boot sector\n \
    Shell> dumpbs -w -fat16 blk0 fs0:\\dumpbs.in\n \
  \n \
  * Write a fat32 format to blk0's boot sector\n \
    Shell> dumpbs -w -fat32 blk0 fs0:\\dumpbs.in");
}