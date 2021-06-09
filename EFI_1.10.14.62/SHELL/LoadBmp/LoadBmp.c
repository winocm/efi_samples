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

  LoadBmp.c
  
Abstract:

  LoadBmp shell command. Load a BMP graphics image on the UGA Draw
  Protocol. 

  -c Center image on the screen

  -t tile image across all of the screen

  -i UGA Draw Protocol Instance to display BMP file on

Revision History

--*/

#include "shell.h"
#include EFI_PROTOCOL_DEFINITION (UgaDraw)
#include EFI_PROTOCOL_DEFINITION (ConsoleControl)
#include EFI_GUID_DEFINITION (Bmp)

VOID
LoadBmpFile (
  IN  SHELL_FILE_ARG        *Arg,
  IN  EFI_UGA_DRAW_PROTOCOL *UgaDraw,
  IN  UINT8                 *ImageBuffer,
  IN  UINTN                 ImageSize,
  IN  BOOLEAN               CenterFlag,
  IN  BOOLEAN               TileFlag
  )
{
  EFI_STATUS              Status;
  BMP_IMAGE_HEADER        *BmpHeader;
  BMP_COLOR_MAP           *BmpColorMap;
  UINT8                   *Image;
  EFI_UGA_PIXEL           *BltBuffer, *Blt;
  UINTN                   BltBufferSize, Index;
  UINTN                   TileHeight, TileWidth, Height, Width;
  UINTN                   StartHeight, StartWidth, ImageIndex;
  UINT32                  SizeOfX, SizeOfY;
  UINT32                  ColorDepth;
  UINT32                  RefreshRate;

  BltBuffer = NULL;

  //
  // Verify BMP Header
  //
  BmpHeader = (BMP_IMAGE_HEADER *)ImageBuffer;
  if (BmpHeader->CharB != 'B' || BmpHeader->CharM != 'M') {
    Print (L"UgaDraw: Cannot read BMP header\n");
    goto Exit;
  }

  if (BmpHeader->CompressionType != 0) {
    Print (L"UgaDraw: BMP compression not supported\n");
    goto Exit;  
  }

  //
  // Calculate Color Map offset in the image.
  //
  BmpColorMap = (BMP_COLOR_MAP *)(ImageBuffer + sizeof (BMP_IMAGE_HEADER));

  //
  // Calculate graphics image data address in the image
  //
  Image = ImageBuffer + BmpHeader->ImageOffset;

  //
  // Lets allocate the UgaDraw Blit Buffer.
  //
  BltBufferSize = BmpHeader->PixelWidth * BmpHeader->PixelHeight;
  BltBuffer = AllocateZeroPool (BltBufferSize * sizeof (EFI_UGA_PIXEL));
  if (BltBuffer == NULL) {
    Print (L"UgaDraw: Out of memory for Blt Buffer\n");
    goto Exit;
  }

  Status = UgaDraw->GetMode (UgaDraw, &SizeOfX, &SizeOfY, &ColorDepth, &RefreshRate);

  //
  // Convert image from BMP to Blt buffer format
  //
  for (Height = 0; Height < BmpHeader->PixelHeight; Height++) {
    Blt = &BltBuffer[(BmpHeader->PixelHeight - Height - 1) * BmpHeader->PixelWidth];
    for (Width = 0; Width < BmpHeader->PixelWidth; Width++, Image++, Blt++) {
      switch (BmpHeader->BitPerPixel) {
      case 4:
        //
        // Convert BMP Palette to 24-bit color
        //
        Index = (*Image) >> 4;
        Blt->Red   = BmpColorMap[Index].Red;
        Blt->Green = BmpColorMap[Index].Green;
        Blt->Blue  = BmpColorMap[Index].Blue;
        if (Width < (BmpHeader->PixelWidth - 1)) {
          Blt++;
          Width++;
          Index = (*Image) & 0x0f;
          Blt->Red   = BmpColorMap[Index].Red;
          Blt->Green = BmpColorMap[Index].Green;
          Blt->Blue  = BmpColorMap[Index].Blue;
        }
        break;
      case 8:
        //
        // Convert BMP Palette to 24-bit color
        //
        Blt->Red   = BmpColorMap[*Image].Red;
        Blt->Green = BmpColorMap[*Image].Green;
        Blt->Blue  = BmpColorMap[*Image].Blue;
      break;
      case 24:
        Blt->Blue = *Image++;
        Blt->Green = *Image++;
        Blt->Red = *Image;
        break;
      default:
        Print (L"UgaDraw: BitPerPixel not supported");
        goto Exit;
        break;
      };

    }

    ImageIndex = (UINTN)(Image - BmpHeader->ImageOffset);
    if ((ImageIndex % 4) != 0) {
      //
      // Bmp Image starts each row on a 32-bit boundary!
      //
      Image = Image + (4 - (ImageIndex % 4));
    }
  }



  if (TileFlag) {
    for (TileWidth = 0; TileWidth < SizeOfX; TileWidth += BmpHeader->PixelWidth) {
      for (TileHeight = 0; TileHeight < SizeOfY; TileHeight += BmpHeader->PixelHeight) {
        UgaDraw->Blt (
                  UgaDraw, 
                  BltBuffer,              EfiUgaBltBufferToVideo, 
                  0,                      0,
                  TileWidth,              TileHeight,
                  BmpHeader->PixelWidth,  BmpHeader->PixelHeight,
                  BmpHeader->PixelWidth * sizeof (EFI_UGA_PIXEL)
                  );
      }
    }
  } else {
    if (CenterFlag) {
      //
      // If CenterFlag is set start in the middle.
      // If the image is bigger than the screen these values will be negative.
      //
      StartWidth = SizeOfX/2 - BmpHeader->PixelWidth/2;
      StartHeight = SizeOfY/2 - BmpHeader->PixelHeight/2;
    } else {
      //
      // Start in upper left hand corner (0, 0)
      //
      StartHeight = 0;
      StartWidth = 0;
    }

    UgaDraw->Blt (
              UgaDraw, 
              BltBuffer,              EfiUgaBltBufferToVideo,
              0,                      0,
              StartWidth,             StartHeight,
              BmpHeader->PixelWidth,  BmpHeader->PixelHeight,
              BmpHeader->PixelWidth * sizeof (EFI_UGA_PIXEL)
              );
  }

Exit:
  if (BltBuffer != NULL) {
    BS->FreePool (BltBuffer);
  }
  return;
}




EFI_STATUS
LoadBmpInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

//
//
//

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(LoadBmpInitialize)
#endif

EFI_STATUS
LoadBmpInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Command entry point.  Load a bitmap file.

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
  EFI_STATUS                          Status;
  UINTN                               Index;
  EFI_LIST_ENTRY                      FileList;
  EFI_LIST_ENTRY                      *Link;
  SHELL_FILE_ARG                      *Arg;
  EFI_UGA_DRAW_PROTOCOL               *UgaDraw;
  UINTN                               UgaDrawProtocolCount;
  EFI_HANDLE                          *UgaDrawHandles;
  BOOLEAN                             CenterFlag;
  BOOLEAN                             TileFlag;
  CHAR16                              *Char;
  UINTN                               Size;
  UINT8                               *Image;
  UINTN                               UgaDrawInstance;
  EFI_CONSOLE_CONTROL_PROTOCOL        *ConsoleControl;
  BOOLEAN                             Wait;
  UINTN                               WaitTime;
  EFI_INPUT_KEY                       Key;

#ifdef EFI_BOOTSHELL
  //
  // Check to see if the app is to install as a "internal command" 
  // to the shell
  //
  InstallInternalShellCommand (
    ImageHandle,   SystemTable,   LoadBmpInitialize,
    L"loadbmp",     // command
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
  ConsoleControl = NULL;

  //
  // Scan args for flags
  //
  InitializeListHead (&FileList);    
  CenterFlag = FALSE;
  TileFlag = FALSE;
  UgaDrawInstance = 0;
  Wait = FALSE;
  WaitTime = 0;

  if (SI->Argc < 2 ) {
    Print (L"LoadBmp: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  for (Index = 1; Index < SI->Argc; Index += 1) {
    if (SI->Argv[Index][0] == '-') {
      for (Char = SI->Argv[Index]+1; *Char; Char++) {
        switch (*Char) {
        case 'c':
        case 'C':
          CenterFlag = TRUE;
          break;

        case 't':
        case 'T':
          TileFlag = TRUE;
          break;

        case 'i':
        case 'I':
          Char++;
          UgaDrawInstance = Atoi (Char);
          break;

        case 'w':
        case 'W':
          Wait = TRUE;
          if (*(Char+1)) {
            Char++;
            WaitTime = Atoi (Char);
          }
          break;

        default:
          Print (L"LoadBmp: Unknown flag %hs\n", SI->Argv[Index]);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
      }
    }
  }

  Status = LibLocateHandle (
            ByProtocol,
            &gEfiUgaDrawProtocolGuid, 
            NULL, 
            &UgaDrawProtocolCount,
            &UgaDrawHandles
            );

  if (EFI_ERROR (Status)) {
    Print (L"LoadBmp: UGA protocol not found\n");
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (UgaDrawInstance >= UgaDrawProtocolCount) {
    UgaDrawInstance = 0;
  }

  Status = BS->HandleProtocol (
                UgaDrawHandles[UgaDrawInstance], 
                &gEfiUgaDrawProtocolGuid,
                &UgaDraw
                );
  BS->FreePool (UgaDrawHandles);
  if (EFI_ERROR (Status)) {
    Print (L"LoadBmp: UGA Draw Protocol not found\n");
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  for (Index = 1; Index < SI->Argc; Index += 1) {
    if (SI->Argv[Index][0] != '-') {
      ShellFileMetaArg (SI->Argv[Index], &FileList);
    }
  }

  if (IsListEmpty(&FileList)) {
    Status = EFI_NOT_FOUND;
    Print (L"LoadBmp: File not found\n");
    goto Done;
  }

  Status = BS->LocateProtocol (
                 &gEfiConsoleControlProtocolGuid,
                 NULL,
                 (VOID **)&ConsoleControl
                 );

  if (ConsoleControl) {
    ConsoleControl->SetMode (ConsoleControl, EfiScreenGraphics);
  }

  //
  // Process each file
  //
  for (Link=FileList.Flink; Link!=&FileList; Link=Link->Flink) {
    Arg = CR(Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (EFI_ERROR (Arg->Status) || Arg->Info->Attribute & EFI_FILE_DIRECTORY) {
      //
      // If argument is not valid exit.
      //
      Status = Arg->Status;
      goto Done;
    }
    Print(L"%HFile: %s Size %,ld%N\n", Arg->FullName, Arg->Info->FileSize);

    Size = (UINTN)Arg->Info->FileSize;
    Image = AllocatePool (Size);
    if (Image == NULL) {
      Print (L"LoadBmp: Out of memory\n");
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }
   
    Status = Arg->Handle->Read (Arg->Handle, &Size, Image);
    if (EFI_ERROR (Status)) {
      Print (L"LoadBmp: Cannot read image\n");
      BS->FreePool (Image);
      goto Done;
    }

    //
    // Draw the Bitmap to the display
    //
    LoadBmpFile (Arg, UgaDraw, Image, Size, CenterFlag, TileFlag);

    if (Wait) {
      if (WaitTime == 0) {
        do {
          Status = ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
        } while (EFI_ERROR (Status));
      } else {
        BS->Stall (WaitTime * 1000000);
      }
    }

    BS->FreePool (Image);
  }

Done:  
  if (ConsoleControl) {
    ConsoleControl->SetMode (ConsoleControl, EfiScreenText);
  }

  ShellFreeFileList (&FileList);
  return Status;
}

