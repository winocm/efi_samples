/*++

Copyright (c)  1999 - 2001 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  LoadPciRom.c
  
Abstract:

  Shell app "LoadPciRom"


Revision History

--*/

#include "shell.h"
#include "EfiImage.h"

//
//
//

EFI_STATUS
InitializeLoadPciRom (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

EFI_STATUS
LoadEfiDriversFromRomImage (
  VOID                      *RomBar,
  UINTN                     RomSize,
  CHAR16                    *FileName
  );

EFI_HANDLE  gMyImageHandle;

//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeLoadPciRom)
#endif

EFI_STATUS
InitializeLoadPciRom (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  CHAR16                  **Argv;
  UINTN                   Argc;
  EFI_LIST_ENTRY          File1List;
  SHELL_FILE_ARG          *File1Arg;
  UINTN                   SourceSize;
  UINT8                   *File1Buffer;
  EFI_STATUS              Status;
  UINTN                   Index;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   InitializeLoadPciRom,
    L"LoadPciRom",     // command
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

  gMyImageHandle = ImageHandle;

  //
  // Local variable initializations
  //
  Argv = SI->Argv;
  Argc = SI->Argc;
  InitializeListHead (&File1List);
  File1Arg = NULL;
  SourceSize = 0;
  File1Buffer = NULL;
  Index = 0;

  //
  // Parse command line arguments
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen(Argv[Index]) == 0) {
      Print (L"LoadPciRom: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } 
  }
  
  //
  // verify number of arguments
  //
  if (Argc < 2) {
    Print (L"LoadPciRom: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // validate first file
  //
  Status = ShellFileMetaArg (Argv[1], &File1List);
  if (EFI_ERROR(Status)) {
    Print(L"LoadPciRom: Cannot open %hs - %r\n", Argv[1], Status);
    goto Done;
  }
  
  //
  // empty list
  //
  if (IsListEmpty(&File1List)) {
    Status = EFI_NOT_FOUND;
    Print(L"LoadPciRom: Cannot open %hs - %r\n", Argv[1], Status);
    goto Done;
  }

  //
  // multiple files
  //
  if (File1List.Flink->Flink != &File1List) {
    Print(L"LoadPciRom: First argument cannot be multiple files\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  File1Arg = CR(File1List.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  //
  // Open error
  //
  if ( EFI_ERROR(File1Arg->Status) || !File1Arg->Handle ) {
    Print (L"LoadPciRom: Cannot open %hs - %r\n", File1Arg->FullName, File1Arg->Status);
    Status = File1Arg->Status;
    goto Done;
  }

  //
  // directory
  //
  if (File1Arg->Info && (File1Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    Print(L"LoadPciRom: First argument cannot be a directory\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Allocate buffers for both files
  //

  SourceSize = (UINTN)File1Arg->Info->FileSize;
  File1Buffer = AllocatePool (SourceSize);
  if (File1Buffer == NULL) {
    Print(L"LoadPciRom: Out of memory\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = File1Arg->Handle->Read (File1Arg->Handle, &SourceSize, File1Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"LoadPciRom: Read %hs error - %r\n", File1Arg->FullName, Status);
    goto Done;
  }

  Status = LoadEfiDriversFromRomImage(
             File1Buffer,
             SourceSize,
             File1Arg->FileName
             );

Done:
  ShellFreeFileList (&File1List);
  return Status;
}

EFI_STATUS
LoadEfiDriversFromRomImage (
  VOID                      *RomBar,
  UINTN                     RomSize,
  CHAR16                    *FileName
  )
/*++

Routine Description:
  Command entry point. 

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_UNSUPPORTED         - Protocols unsupported
  EFI_OUT_OF_RESOURCES    - Out of memory
  Other value             - Unknown error

--*/
{
  EFI_PCI_EXPANSION_ROM_HEADER  *EfiRomHeader;
  PCI_DATA_STRUCTURE            *Pcir;
  UINTN                         ImageIndex;
  UINTN                         RomBarOffset;
  UINT32                        ImageSize;
  UINT16                        ImageOffset;
  EFI_HANDLE                    ImageHandle;
  EFI_STATUS                    Status;
  EFI_STATUS                    retStatus;
  CHAR16                        RomFileName[280];
  EFI_DEVICE_PATH_PROTOCOL      *FilePath;
  BOOLEAN                       SkipImage;
  UINT32                        DestinationSize;
  UINT32                        ScratchSize;
  UINT8                         *Scratch;
  VOID                          *ImageBuffer;
  VOID                          *DecompressedImageBuffer;
  UINT32                        ImageLength;
  EFI_DECOMPRESS_PROTOCOL       *Decompress;

  ImageIndex = 0;
  retStatus = EFI_NOT_FOUND;
  RomBarOffset = (UINTN)RomBar;
  
  do {

    EfiRomHeader = (EFI_PCI_EXPANSION_ROM_HEADER *)(UINTN)RomBarOffset;

    if (EfiRomHeader->Signature != 0xaa55) {
      Print(L"LoadPciRom: Image # %hd is corrupt\n", ImageIndex);
      return retStatus;
    }

    Pcir = (PCI_DATA_STRUCTURE *)(UINTN)(RomBarOffset + EfiRomHeader->PcirOffset);
    ImageSize   = Pcir->ImageLength * 512;

    if ((Pcir->CodeType == PCI_CODE_TYPE_EFI_IMAGE) && 
        (EfiRomHeader->EfiSignature == EFI_PCI_EXPANSION_ROM_HEADER_EFISIGNATURE)) {

      if ((EfiRomHeader->EfiSubsystem == EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER)
       || (EfiRomHeader->EfiSubsystem == EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER)) {
        ImageOffset = EfiRomHeader->EfiImageHeaderOffset;
        ImageSize   = EfiRomHeader->InitializationSize * 512;

        ImageBuffer = (VOID *)(UINTN)(RomBarOffset + ImageOffset);
        ImageLength = ImageSize - ImageOffset;
        DecompressedImageBuffer = NULL;

        //
        // decompress here if needed
        //
        SkipImage = FALSE;
        if (EfiRomHeader->CompressionType > EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED) {
          SkipImage = TRUE;
        }
        if (EfiRomHeader->CompressionType == EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED) {
          Status = BS->LocateProtocol(&gEfiDecompressProtocolGuid, NULL, &Decompress);
          if (EFI_ERROR (Status)) {
            Print(L"LoadPciRom: Decompress protocol interface not found\n");
            SkipImage = TRUE;
          } else {
            SkipImage = TRUE;
            Status = Decompress->GetInfo(
                       Decompress, 
                       ImageBuffer,
                       ImageLength, 
                       &DestinationSize, 
                       &ScratchSize
                       );
            if (!EFI_ERROR (Status)) {
              DecompressedImageBuffer = AllocatePool (DestinationSize);
              if (ImageBuffer != NULL) {
                Scratch = AllocatePool (ScratchSize);
                if (Scratch != NULL) {
                  Status = Decompress->Decompress(
                                         Decompress, 
                                         ImageBuffer, 
                                         ImageLength, 
                                         DecompressedImageBuffer,
                                         DestinationSize, 
                                         Scratch,
                                         ScratchSize
                                         );
                  if (!EFI_ERROR (Status)) {
                    ImageBuffer = DecompressedImageBuffer;
                    ImageLength = DestinationSize;
                    SkipImage = FALSE;
                  }
                  FreePool(Scratch);
                }
              }
            }
          }
        }

        if (SkipImage == FALSE) {
        
          // 
          // load image and start image
          //

          SPrint(RomFileName,sizeof(RomFileName),L"%s[%d]", FileName, ImageIndex);
          FilePath = FileDevicePath (NULL, RomFileName);
        
          Status = BS->LoadImage(
                         TRUE,
                         gMyImageHandle,
                         FilePath,
                         ImageBuffer,
                         ImageLength,
                         &ImageHandle
                         );
          if (EFI_ERROR (Status)) {
            Print(L"LoadPciRom: Load image #%hd error - %r\n", ImageIndex, Status);
          } else {
            Status = BS->StartImage (ImageHandle, NULL, NULL);
            if (EFI_ERROR (Status)) {
              Print(L"LoadPciRom: Start image #%hd error - %r\n", ImageIndex, Status);
            } else {
              retStatus = Status;
            }
          }
        }

        if (DecompressedImageBuffer != NULL) {
          FreePool(DecompressedImageBuffer);
        }

      } 
    }
    RomBarOffset = RomBarOffset + ImageSize;
    ImageIndex++;
  } while (((Pcir->Indicator & 0x80) == 0x00) && ((RomBarOffset - (UINTN)RomBar) < RomSize));
  
  return retStatus;
}


