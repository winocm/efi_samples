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

  Mbr.c
  
Abstract:

  Decode a hard disk partitioned with the legacy MBR found on most PC's

  MBR - Master Boot Record is in the first sector of a partitioned hard disk.
        The MBR supports four partitions per disk. The MBR also contains legacy
        code that is not run on an EFI system. The legacy code reads the 
        first sector of the active partition into memory and 

  BPB - Boot(?) Parameter Block is in the first sector of a FAT file system. 
        The BPB contains information about the FAT file system. The BPB is 
        always on the first sector of a media. The first sector also contains
        the legacy boot strap code.

--*/

#include "Partition.h"
#include "Mbr.h"

BOOLEAN
PartitionValidMbr (
  IN  MASTER_BOOT_RECORD      *Mbr,
  IN  EFI_LBA                 LastLba
  )
/*++

Routine Description:
  Test to see if the Mbr buffer is a valid MBR

Arguments:       
  Mbr     - Parent Handle 
  LastLba - Last Lba address on the device.

Returns:
  TRUE  - Mbr is a Valid MBR
  FALSE - Mbr is not a Valid MBR

--*/
{
  UINT32      StartingLBA;
  UINT32      EndingLBA;
  UINT32      NewEndingLBA;
  INTN        Index1;
  INTN        Index2;
  BOOLEAN     MbrValid;

  if (Mbr->Signature != MBR_SIGNATURE) {
    return FALSE;
  } 

  //
  // The BPB also has this signature, so it can not be used alone.
  //

  MbrValid = FALSE;
  for (Index1=0; Index1<MAX_MBR_PARTITIONS; Index1++) {
    if ( Mbr->Partition[Index1].OSIndicator == 0x00 || 
         UNPACK_UINT32(Mbr->Partition[Index1].SizeInLBA) == 0 ) {
      continue;
    }
    MbrValid = TRUE;
    StartingLBA = UNPACK_UINT32(Mbr->Partition[Index1].StartingLBA);
    EndingLBA = StartingLBA + UNPACK_UINT32(Mbr->Partition[Index1].SizeInLBA) - 1;
    if (EndingLBA > LastLba) {
      //
      // Compatibility Errata:
      //  Some systems try to hide drive space with their INT 13h driver
      //  This does not hide space from the OS driver. This means the MBR
      //  that gets created from DOS is smaller than the MBR created from 
      //  a real OS (NT & Win98). This leads to BlockIo->LastBlock being 
      //  wrong on some systems FDISKed by the OS.
      //
      // return FALSE since no block devices on a system are implemented
      // with INT 13h
      //
        return FALSE;
    }
    for (Index2 = Index1 + 1; Index2 < MAX_MBR_PARTITIONS; Index2++) {
      if (Mbr->Partition[Index2].OSIndicator == 0x00 ||
          UNPACK_UINT32(Mbr->Partition[Index2].SizeInLBA) == 0) {
        continue;
      }
      NewEndingLBA = UNPACK_UINT32(Mbr->Partition[Index2].StartingLBA) +
                     UNPACK_UINT32(Mbr->Partition[Index2].SizeInLBA) - 
                     1;
      if (NewEndingLBA >= StartingLBA && 
          UNPACK_UINT32(Mbr->Partition[Index2].StartingLBA) <= EndingLBA) {
        //
        // This region overlaps with the Index1'th region
        //
        return FALSE;
      } 
    }
  }
  //
  // Non of the regions overlapped so MBR is O.K.
  //
  return MbrValid;
} 


BOOLEAN
PartitionInstallMbrChildHandles (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DISK_IO_PROTOCOL         *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL        *BlockIo,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  )
/*++

Routine Description:
  Install child handles if the Handle supports MBR format.

Arguments:       
  Handle     - Parent Handle 
  DiskIo     - Parent DiskIo interface
  BlockIo    - Parent BlockIo interface
  DevicePath - Parent Device Path

Returns:
  EFI_SUCCESS - If a child handle was added
  other       - A child handle was not added

--*/
{
  EFI_STATUS                Status;
  MASTER_BOOT_RECORD        *Mbr;
  MASTER_BOOT_RECORD        *ExtMbr;
  UINT32                    ExtMbrStartingLba;
  UINTN                     Index;
  HARDDRIVE_DEVICE_PATH     HdDev;
  HARDDRIVE_DEVICE_PATH     ParentHdDev;
  BOOLEAN                   Found;
  UINT32                    PartitionNumber;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathNode;
  EFI_DEVICE_PATH_PROTOCOL  *LastDevicePathNode;

  Mbr = NULL;
  ExtMbr = NULL;
  Found = FALSE;
  PartitionNumber = 0;

  Mbr = EfiLibAllocatePool (BlockIo->Media->BlockSize);
  if (Mbr == NULL) {
    goto Done;
  }

  ExtMbr = EfiLibAllocatePool (BlockIo->Media->BlockSize);
  if (ExtMbr == NULL) {
    goto Done;
  }

  Status = BlockIo->ReadBlocks (BlockIo, 
                                BlockIo->Media->MediaId, 
                                0, 
                                BlockIo->Media->BlockSize, 
                                Mbr);
  if (EFI_ERROR (Status) || !PartitionValidMbr (Mbr, BlockIo->Media->LastBlock)) {
    goto Done;
  }

  //
  // We have a valid mbr - add each partition
  //

  //
  // Get starting and ending LBA of the parent block device.
  //
  LastDevicePathNode = NULL;
  EfiZeroMem (&ParentHdDev, sizeof (ParentHdDev));
  DevicePathNode = DevicePath;
  while (!EfiIsDevicePathEnd (DevicePathNode)) {
    LastDevicePathNode = DevicePathNode;
    DevicePathNode = EfiNextDevicePathNode (DevicePathNode);
  }
  if (LastDevicePathNode != NULL) {
    if (DevicePathType (LastDevicePathNode) == MEDIA_DEVICE_PATH && DevicePathSubType (LastDevicePathNode) == MEDIA_HARDDRIVE_DP) {
      gBS->CopyMem (&ParentHdDev, LastDevicePathNode, sizeof (ParentHdDev));
    } else {
      LastDevicePathNode = NULL;
    }
  }
  
  PartitionNumber = 1;
  for (Index=0; Index < MAX_MBR_PARTITIONS; Index++) {
    if (Mbr->Partition[Index].OSIndicator == 0x00 || UNPACK_UINT32 (Mbr->Partition[Index].SizeInLBA) == 0 ) {
      //
      // Don't use null MBR entries
      //
      continue;
    }
    
    if (Mbr->Partition[Index].OSIndicator == EXTENDED_DOS_PARTITION ||
        Mbr->Partition[Index].OSIndicator == EXTENDED_WINDOWS_PARTITION) {
      
      //
      // It's an extended partition. Follow the extended partition
      // chain to get all the logical drives
      //

      ExtMbrStartingLba = UNPACK_UINT32 (Mbr->Partition[Index].StartingLBA);
      
      do {

        //
        // Don't allow partition to be self referencing
        //
        if (ExtMbrStartingLba == 0) {
          break;
        }

        Status = BlockIo->ReadBlocks (
                    BlockIo,
                    BlockIo->Media->MediaId,
                    ExtMbrStartingLba,
                    BlockIo->Media->BlockSize,
                    ExtMbr
                    );
        if (EFI_ERROR (Status)) {
          goto Done;
        }        
        if (ExtMbr->Partition[0].OSIndicator == 0) {
          break;
        }        
        
        EfiZeroMem (&HdDev, sizeof(HdDev));
        HdDev.Header.Type = MEDIA_DEVICE_PATH;
        HdDev.Header.SubType = MEDIA_HARDDRIVE_DP;
        SetDevicePathNodeLength (&HdDev.Header, sizeof(HdDev));
    
        HdDev.PartitionNumber = PartitionNumber ++;
        HdDev.MBRType = MBR_TYPE_PCAT;
        HdDev.SignatureType = SIGNATURE_TYPE_MBR;
        HdDev.PartitionStart = UNPACK_UINT32 (ExtMbr->Partition[0].StartingLBA) 
                               + ExtMbrStartingLba;
        HdDev.PartitionSize  = UNPACK_UINT32 (ExtMbr->Partition[0].SizeInLBA);
        if (HdDev.PartitionStart + HdDev.PartitionSize - 1 
            >= (UNPACK_UINT32(Mbr->Partition[Index].SizeInLBA))
            + (UNPACK_UINT32(Mbr->Partition[Index].StartingLBA))
            ) {
          break;
        }            

        EfiCopyMem (HdDev.Signature, &(ExtMbr->UniqueMbrSignature[0]), sizeof(UINT32));
    
        Status = PartitionInstallChildHandle (
                    This,
                    Handle, DiskIo, BlockIo,  DevicePath,   
                    (EFI_DEVICE_PATH_PROTOCOL *) &HdDev,
                    HdDev.PartitionStart,   
                    HdDev.PartitionStart + HdDev.PartitionSize - 1,  
                    MBR_SIZE,
                    (BOOLEAN)(ExtMbr->Partition[0].OSIndicator == EFI_PARTITION)
                    );
        if (!EFI_ERROR (Status)) {
          Found = TRUE;
        }
        
        if (ExtMbr->Partition[1].OSIndicator != EXTENDED_DOS_PARTITION &&
            ExtMbr->Partition[1].OSIndicator != EXTENDED_WINDOWS_PARTITION) {
          break;
        }
        ExtMbrStartingLba = (UNPACK_UINT32(Mbr->Partition[Index].StartingLBA))
                            + (UNPACK_UINT32(ExtMbr->Partition[1].StartingLBA));
      } while(ExtMbrStartingLba 
              < (UNPACK_UINT32(Mbr->Partition[Index].SizeInLBA))
              + (UNPACK_UINT32(Mbr->Partition[Index].StartingLBA))
              );
      
    } else {
      
      //
      // Treat it as a primary partition
      //
        
      EfiZeroMem (&HdDev, sizeof(HdDev));
      HdDev.Header.Type = MEDIA_DEVICE_PATH;
      HdDev.Header.SubType = MEDIA_HARDDRIVE_DP;
      SetDevicePathNodeLength (&HdDev.Header, sizeof(HdDev));
  
      HdDev.PartitionNumber = PartitionNumber ++;
      HdDev.MBRType = MBR_TYPE_PCAT;
      HdDev.SignatureType = SIGNATURE_TYPE_MBR;
      HdDev.PartitionStart = UNPACK_UINT32 (Mbr->Partition[Index].StartingLBA);
      HdDev.PartitionSize  = UNPACK_UINT32 (Mbr->Partition[Index].SizeInLBA);
      EfiCopyMem (HdDev.Signature, &(Mbr->UniqueMbrSignature[0]), sizeof(UINT32));

      Status = EFI_SUCCESS;
      if (LastDevicePathNode != NULL) {
        if (HdDev.PartitionStart <= ParentHdDev.PartitionStart) {
          Status = EFI_NOT_FOUND;
        }
        if (HdDev.PartitionStart + HdDev.PartitionSize > ParentHdDev.PartitionStart + ParentHdDev.PartitionSize) {
          Status = EFI_NOT_FOUND;
        }
      }

      if (!EFI_ERROR (Status)) {
        Status = PartitionInstallChildHandle (
                    This,
                    Handle, DiskIo, BlockIo,  DevicePath,   
                    (EFI_DEVICE_PATH_PROTOCOL *) &HdDev,
                    HdDev.PartitionStart,   
                    HdDev.PartitionStart + HdDev.PartitionSize - 1,  
                    MBR_SIZE,
                    (BOOLEAN)(Mbr->Partition[Index].OSIndicator == EFI_PARTITION)
                    );
      }
      if (!EFI_ERROR (Status)) {
        Found = TRUE;
      }
    }
  }

Done:
  gBS->FreePool (Mbr);

  gBS->FreePool (ExtMbr);
  
  return Found;
}

