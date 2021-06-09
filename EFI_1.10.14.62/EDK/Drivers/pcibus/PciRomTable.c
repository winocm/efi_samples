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

  PciRomTable.c
  
Abstract:

  Option Rom Support for PCI Bus Driver

Revision History

--*/
#include "pcibus.h"

#include EFI_GUID_DEFINITION(PciOptionRomTable)

typedef struct {
  EFI_HANDLE  ImageHandle;
  UINTN       Seg;
  UINT8       Bus;
  UINT8       Dev;
  UINT8       Func;
} EFI_PCI_ROM_IMAGE_MAPPING;

static UINTN                      mNumberOfPciRomImages    = 0;
static UINTN                      mMaxNumberOfPciRomImages = 0;
static EFI_PCI_ROM_IMAGE_MAPPING  *mRomImageTable          = NULL;

static CHAR16 mHexDigit[17] = L"0123456789ABCDEF";

static
VOID
PciRomAddImageMapping (
  IN EFI_HANDLE  ImageHandle,
  IN UINTN       Seg,
  IN UINT8       Bus,
  IN UINT8       Dev,
  IN UINT8       Func
  )

{
  EFI_STATUS                 Status;
  EFI_PCI_ROM_IMAGE_MAPPING  *TempMapping;
  
  if (mNumberOfPciRomImages >= mMaxNumberOfPciRomImages) {

    mMaxNumberOfPciRomImages += 0x20;

    TempMapping = NULL;
    Status = gBS->AllocatePool(
                    EfiBootServicesData,
                    mMaxNumberOfPciRomImages * sizeof(EFI_PCI_ROM_IMAGE_MAPPING),
                    (VOID **)&TempMapping
                    );
    if (EFI_ERROR (Status) || TempMapping == NULL) {
      return;
    }

    EfiCopyMem(TempMapping, mRomImageTable, mNumberOfPciRomImages * sizeof(EFI_PCI_ROM_IMAGE_MAPPING));

    if (mRomImageTable != NULL) {
      gBS->FreePool(mRomImageTable);
    }

    mRomImageTable = TempMapping;
  }
  mRomImageTable[mNumberOfPciRomImages].ImageHandle = ImageHandle;
  mRomImageTable[mNumberOfPciRomImages].Seg         = Seg;
  mRomImageTable[mNumberOfPciRomImages].Bus         = Bus;
  mRomImageTable[mNumberOfPciRomImages].Dev         = Dev;
  mRomImageTable[mNumberOfPciRomImages].Func        = Func;
  mNumberOfPciRomImages++;
}

static
VOID
HexToString (
  CHAR16  *String,
  UINTN   Value,
  UINTN   Digits
  )

{
  for (; Digits > 0; Digits--, String++) {
    *String = mHexDigit[((Value >> (4*(Digits-1))) & 0x0f)];
  }
}

EFI_STATUS
PciRomLoadEfiDriversFromRomImage (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_PCI_OPTION_ROM_DESCRIPTOR  *PciOptionRomDescriptor
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
  VOID                          *RomBar;
  UINTN                         RomSize;
  CHAR16                        *FileName;
  EFI_PCI_EXPANSION_ROM_HEADER  *EfiRomHeader;
  PCI_DATA_STRUCTURE            *Pcir;
  UINTN                         ImageIndex;
  UINTN                         RomBarOffset;
  UINT32                        ImageSize;
  UINT16                        ImageOffset;
  EFI_HANDLE                    ImageHandle;
  EFI_STATUS                    Status;
  EFI_STATUS                    retStatus;
  EFI_DEVICE_PATH_PROTOCOL      *FilePath;
  BOOLEAN                       SkipImage;
  UINT32                        DestinationSize;
  UINT32                        ScratchSize;
  UINT8                         *Scratch;
  VOID                          *ImageBuffer;
  VOID                          *DecompressedImageBuffer;
  UINT32                        ImageLength;
  EFI_DECOMPRESS_PROTOCOL       *Decompress;

  RomBar  = (VOID *)(UINTN)PciOptionRomDescriptor->RomAddress,
  RomSize = (UINTN)PciOptionRomDescriptor->RomLength,

  FileName = L"PciRom Seg=00000000 Bus=00 Dev=00 Func=00 Image=0000";
  HexToString(&FileName[11], PciOptionRomDescriptor->Seg, 8);
  HexToString(&FileName[24], PciOptionRomDescriptor->Bus, 2);
  HexToString(&FileName[31], PciOptionRomDescriptor->Dev, 2);
  HexToString(&FileName[39], PciOptionRomDescriptor->Func, 2);

  ImageIndex = 0;
  retStatus = EFI_NOT_FOUND;
  RomBarOffset = (UINTN)RomBar;
  
  do {

    EfiRomHeader = (EFI_PCI_EXPANSION_ROM_HEADER *)(UINTN)RomBarOffset;

    if (EfiRomHeader->Signature != 0xaa55) {
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
          Status = gBS->LocateProtocol(&gEfiDecompressProtocolGuid, NULL, (VOID **)&Decompress);
          if (EFI_ERROR (Status)) {
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
              DecompressedImageBuffer = NULL;
              Status = gBS->AllocatePool(
                              EfiBootServicesData,
                              DestinationSize,
                              &DecompressedImageBuffer
                              );
              if (!EFI_ERROR (Status) && DecompressedImageBuffer != NULL) {
                Scratch = NULL;
                Status = gBS->AllocatePool(
                                EfiBootServicesData,
                                ScratchSize,
                                &Scratch
                                );
                if (!EFI_ERROR (Status) && Scratch != NULL) {
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
                  gBS->FreePool(Scratch);
                }
              }
            }
          }
        }

        if (SkipImage == FALSE) {
        
          // 
          // load image and start image
          //

          HexToString(&FileName[48], ImageIndex, 4);
          FilePath = EfiFileDevicePath (NULL, FileName);
        
          Status = gBS->LoadImage(
                         TRUE,
                         This->ImageHandle,  
                         FilePath,
                         ImageBuffer,
                         ImageLength,
                         &ImageHandle
                         );
          if (!EFI_ERROR (Status)) {
            Status = gBS->StartImage (ImageHandle, 0, NULL);
            if (!EFI_ERROR (Status)) {
                PciRomAddImageMapping (
                  ImageHandle,
                  PciOptionRomDescriptor->Seg,
                  PciOptionRomDescriptor->Bus,
                  PciOptionRomDescriptor->Dev,
                  PciOptionRomDescriptor->Func
                  );
              retStatus = Status;
            }
          }
        }

        if (DecompressedImageBuffer != NULL) {
          gBS->FreePool(DecompressedImageBuffer);
        }

      } 
    }
    RomBarOffset = RomBarOffset + ImageSize;
    ImageIndex++;
  } while (((Pcir->Indicator & 0x80) == 0x00) && ((RomBarOffset - (UINTN)RomBar) < RomSize));
  
  return retStatus;
}

EFI_STATUS
PciRomLoadEfiDriversFromOptionRomTable (
  IN EFI_DRIVER_BINDING_PROTOCOL      *This,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                         Status;
  EFI_PCI_OPTION_ROM_TABLE           *PciOptionRomTable;
  EFI_PCI_OPTION_ROM_DESCRIPTOR      *PciOptionRomDescriptor;
  UINTN                              Index;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *Descriptors;
  UINT16                             MinBus; 
  UINT16                             MaxBus;

  Status = EfiLibGetSystemConfigurationTable (&gEfiPciOptionRomTableGuid, (VOID **)&PciOptionRomTable);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Status = EFI_NOT_FOUND;

  for (Index = 0; Index < PciOptionRomTable->PciOptionRomCount; Index++) {
    PciOptionRomDescriptor = &PciOptionRomTable->PciOptionRomDescriptors[Index];
    if (!PciOptionRomDescriptor->DontLoadEfiRom) {
      if (PciOptionRomDescriptor->Seg == PciRootBridgeIo->SegmentNumber) {
        Status = PciRootBridgeIo->Configuration (PciRootBridgeIo, (VOID **)&Descriptors);
        if (EFI_ERROR (Status)) {
          return Status;
        }
        PciGetBusRange (Descriptors, &MinBus, &MaxBus, NULL);
        if ( (MinBus <= PciOptionRomDescriptor->Bus) && (PciOptionRomDescriptor->Bus <= MaxBus) ) {
          Status = PciRomLoadEfiDriversFromRomImage (This, PciOptionRomDescriptor);
          PciOptionRomDescriptor->DontLoadEfiRom |= 2;
        }
      }
    }
  }

  return Status;
}

EFI_STATUS
PciRomGetRomResourceFromPciOptionRomTable (
  IN EFI_DRIVER_BINDING_PROTOCOL      *This,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo,
  PCI_IO_DEVICE                       *PciIoDevice
)
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                     Status;
  EFI_PCI_OPTION_ROM_TABLE       *PciOptionRomTable;
  EFI_PCI_OPTION_ROM_DESCRIPTOR  *PciOptionRomDescriptor;
  UINTN                          Index;

  Status = EfiLibGetSystemConfigurationTable (&gEfiPciOptionRomTableGuid, (VOID **)&PciOptionRomTable);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  for (Index = 0; Index < PciOptionRomTable->PciOptionRomCount; Index++) {
    PciOptionRomDescriptor = &PciOptionRomTable->PciOptionRomDescriptors[Index];
    if (PciOptionRomDescriptor->Seg  == PciRootBridgeIo->SegmentNumber &&
        PciOptionRomDescriptor->Bus  == PciIoDevice->BusNumber         &&
        PciOptionRomDescriptor->Dev  == PciIoDevice->DeviceNumber      &&
        PciOptionRomDescriptor->Func == PciIoDevice->FunctionNumber       ) {

      PciIoDevice->PciIo.RomImage = (VOID *)(UINTN)PciOptionRomDescriptor->RomAddress;
      PciIoDevice->PciIo.RomSize  = (UINTN)PciOptionRomDescriptor->RomLength;
    }
  }

  for (Index = 0; Index < mNumberOfPciRomImages; Index++) {
    if (mRomImageTable[Index].Seg  == PciRootBridgeIo->SegmentNumber &&
        mRomImageTable[Index].Bus  == PciIoDevice->BusNumber         &&
        mRomImageTable[Index].Dev  == PciIoDevice->DeviceNumber      &&
        mRomImageTable[Index].Func == PciIoDevice->FunctionNumber       ) {

      AddDriver (PciIoDevice, mRomImageTable[Index].ImageHandle);
    }
  }

  return EFI_SUCCESS;
}
