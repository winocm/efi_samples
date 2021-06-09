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

  ElTorito.c
  
Abstract:

  Decode an El Torito formatted CD-ROM

Revision History

--*/

#include "Partition.h"
#include "ElTorito.h"


BOOLEAN
PartitionInstallElToritoChildHandles (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DISK_IO_PROTOCOL         *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL        *BlockIo,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  )
/*++

Routine Description:
  Install child handles if the Handle supports El Torito format.

Arguments:       
  Handle     - Parent Handle 
  DiskIo     - Parent DiskIo interface
  BlockIo    - Parent BlockIo interface
  DevicePath - Parent Device Path

Returns:
  TRUE       - some child handle(s) was added
  FALSE      - no child handle was added

--*/
{
  EFI_STATUS                Status;
  UINT32                    VolDescriptorLba;
  UINT32                    Lba;
  EFI_BLOCK_IO_MEDIA        *Media;
  CDROM_VOLUME_DESCRIPTOR   *VolDescriptor;
  ELTORITO_CATALOG          *Catalog;
  UINTN                     Check;
  UINTN                     Index;
  UINTN                     BootEntry;
  UINTN                     MaxIndex;
  UINT16                    *CheckBuffer;
  CDROM_DEVICE_PATH         CdDev;
  UINT32                    SubBlockSize;
  UINT32                    SectorCount;
  BOOLEAN                   Found;
  UINTN                     Dummy;
  UINT32                    VolSpaceSize;

  Found = FALSE;
  Media = BlockIo->Media;
  VolSpaceSize = 0;

  
  //
  // CD_ROM has the fixed block size as 2048 bytes
  //
  if (Media->BlockSize != 2048) {
    return FALSE;
  }
  
  VolDescriptor = EfiLibAllocatePool ((UINTN)Media->BlockSize);

  if (VolDescriptor == NULL) {
    return FALSE;
  }
  Catalog = (ELTORITO_CATALOG *)VolDescriptor;

  //
  // the ISO-9660 volume descriptor starts at 32k on the media
  // and CD_ROM has the fixed block size as 2048 bytes, so...
  //  
  VolDescriptorLba = 15; //((16*2048) / Media->BlockSize) - 1;
  
  //
  // Loop: handle one volume descriptor per time
  //
  while (TRUE) {

    VolDescriptorLba += 1;
    if (VolDescriptorLba > Media->LastBlock) {
      //
      // We are pointing past the end of the device so exit
      //
      break;
    }

    Status = BlockIo->ReadBlocks (
                        BlockIo, Media->MediaId, VolDescriptorLba, 
                        Media->BlockSize, VolDescriptor
                        );
    if (EFI_ERROR(Status)) {
      break;
    }

    //
    // Check for valid volume descriptor signature
    //
    if (VolDescriptor->Type == CDVOL_TYPE_END  ||
      EfiCompareMem (VolDescriptor->Id, CDVOL_ID, sizeof(VolDescriptor->Id)) != 0) {
      //
      // end of Volume descriptor list
      //
      break;
    }

    //
    //Read the Volume Space Size from Primary Volume Descriptor 81-88 byte,
    //the 32-bit numerical values is stored in Both-byte orders
    //
    if (VolDescriptor->Type == CDVOL_TYPE_CODED){
      VolSpaceSize = VolDescriptor->VolSpaceSize[1];
    }
    
    //
    // Is it an El Torito volume descriptor?
    //
    if (EfiCompareMem (VolDescriptor->SystemId, CDVOL_ELTORITO_ID, sizeof(CDVOL_ELTORITO_ID)-1) != 0) {
      continue;
    }

    //
    // Read in the boot El Torito boot catalog
    //
    Lba = UNPACK_INT32(VolDescriptor->EltCatalog);
    if (Lba > Media->LastBlock) {
      continue;
    }

    Status = BlockIo->ReadBlocks (
                        BlockIo, Media->MediaId, Lba, 
                        Media->BlockSize, Catalog
                        );
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "EltCheckDevice: error reading catalog %r\n", Status));
      continue;
    }
    
    //
    // We don't care too much about the Catalog header's contents, but we do want
    // to make sure it looks like a Catalog header
    //
    if (Catalog->Catalog.Indicator != ELTORITO_ID_CATALOG || Catalog->Catalog.Id55AA != 0xAA55) {
      DEBUG ((EFI_D_ERROR, "EltCheckBootCatalog: El Torito boot catalog header IDs not correct\n"));
      continue ;
    }

    Check = 0;
    CheckBuffer = (UINT16 *)Catalog;
    for (Index = 0; Index < sizeof(ELTORITO_CATALOG)/sizeof(UINT16); Index += 1) {
      Check += CheckBuffer[Index];
    }
    if (Check & 0xFFFF) {
      DEBUG ((EFI_D_ERROR, "EltCheckBootCatalog: El Torito boot catalog header checksum failed\n"));
      continue ;
    }

    MaxIndex = Media->BlockSize / sizeof (ELTORITO_CATALOG);
    for (Index = 1, BootEntry = 1; Index < MaxIndex; Index += 1) {
      //
      // Next entry
      //
      Catalog += 1;

      //
      // Check this entry
      //
      if (Catalog->Boot.Indicator != ELTORITO_ID_SECTION_BOOTABLE ||
        Catalog->Boot.Lba == 0) {
        continue;
      }

      SubBlockSize = 512;
      SectorCount = Catalog->Boot.SectorCount;      

      switch (Catalog->Boot.MediaType) {

      case ELTORITO_NO_EMULATION:
          SubBlockSize = Media->BlockSize;
          break;

      case ELTORITO_HARD_DISK:
          break;

      case ELTORITO_12_DISKETTE:
          SectorCount = 0x50*0x02*0x0F;
          break;
      case ELTORITO_14_DISKETTE:
          SectorCount = 0x50*0x02*0x12;
          break;
      case ELTORITO_28_DISKETTE:
          SectorCount = 0x50*0x02*0x24;
          break;

      default:
          DEBUG((EFI_D_INIT, "EltCheckDevice: unsupported El Torito boot media type %x\n", Catalog->Boot.MediaType));
          SectorCount = 0;
          SubBlockSize = Media->BlockSize;
          break;
      }

      //
      // Create child device handle
      //
      CdDev.Header.Type = MEDIA_DEVICE_PATH;
      CdDev.Header.SubType = MEDIA_CDROM_DP;
      SetDevicePathNodeLength (&CdDev.Header, sizeof(CdDev));

      if (Index == 1) {
        //
        // This is the initial/default entry
        //
        BootEntry = 0;
      }
      
      CdDev.BootEntry = (UINT32) BootEntry;
      BootEntry ++;
      CdDev.PartitionStart = Catalog->Boot.Lba;
      if (SectorCount < 2) {
      	CdDev.PartitionSize = VolSpaceSize;
      }	
      else {
        CdDev.PartitionSize = 
          DriverLibDivU64x32(
            DriverLibMultU64x32(SectorCount, SubBlockSize) + Media->BlockSize - 1,
            Media->BlockSize,
            &Dummy
          );
      }
      
      Status = PartitionInstallChildHandle (
                  This,
                  Handle,
                  DiskIo,
                  BlockIo,
                  DevicePath,
                  (EFI_DEVICE_PATH_PROTOCOL *) &CdDev,
                  Catalog->Boot.Lba,
                  Catalog->Boot.Lba + CdDev.PartitionSize - 1,
                  SubBlockSize,
                  FALSE
                  );
      if (!EFI_ERROR (Status)) {
        Found = TRUE;
      }
    }
  }

  gBS->FreePool (VolDescriptor);

  return Found;
}

