/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  init.c
  
Abstract:

  Initialization routines
  
--*/

#include "fat.h"

EFI_STATUS
FatAllocateVolume (
  IN  EFI_HANDLE                Handle,
  IN  EFI_DISK_IO_PROTOCOL      *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL     *BlockIo
  );

EFI_STATUS
FatAbandonVolume (
  IN FAT_VOLUME *Vol
  );

EFI_STATUS
FatOpenDevice (
  IN OUT FAT_VOLUME           *Vol
  );

EFI_STATUS
FatInitUnicodeCollationSupport (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  LC_ISO_639_2                   *LangCode,
  OUT EFI_UNICODE_COLLATION_PROTOCOL **UnicodeCollationInterface
  )
/*++

Routine Description:

  Initializes Unicode Collation support.
    
Arguments:

  LangCode                   - Language Code specified.
  UnicodeCollationInterface  - Unicode Collation protocol interface returned.

Returns:

  EFI_SUCCESS   - Successfully get the Unicode Collation protocol interface by 
                  specified LangCode.
  EFI_NOT_FOUND - Specified Unicode Collation protocol is not found.

--*/ 
{
  EFI_STATUS                      Status;
  LC_ISO_639_2                    *Languages;
  UINTN                           Index, Position, Length;
  UINTN                           NoHandles;
  EFI_HANDLE                      *Handles;
  BOOLEAN                         Found;
  EFI_UNICODE_COLLATION_PROTOCOL  *Uci;
  
  Found = FALSE;
  *UnicodeCollationInterface = NULL;
  
  if (!LangCode) {
    return EFI_NOT_FOUND;
  }
  
  //
  // Locate all Unicode Collation drivers.
  //
  Status = gBS->LocateHandleBuffer (
                      ByProtocol, 
                      &gEfiUnicodeCollationProtocolGuid, 
                      NULL, 
                      &NoHandles, 
                      &Handles
                    );
  
  if (EFI_ERROR(Status) || !NoHandles) {
    return EFI_NOT_FOUND;
  }

  //
  // Check all Unicode Collation drivers for a matching language code.
  //
  for (Index = 0; Index < NoHandles; Index++) {
    //
    // Open Unicode Collation Protocol
    //
    Status = gBS->OpenProtocol (
                  Handles[Index],   
                  &gEfiUnicodeCollationProtocolGuid,  
                  (VOID **)&Uci,
                  This->DriverBindingHandle,     
                  NULL,   
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  
    if (EFI_ERROR(Status)) {
      continue;
    }

    Found = FALSE;
  
    //
    // Check for a matching language code.
    //
    Languages = Uci->SupportedLanguages;
    Length = EfiAsciiStrLen (Languages);
    
    for (Position = 0; Position < Length; Position += LC_ISO_639_2_ENTRY_SIZE) {

      //
      // If this code matches, use this driver
      //
      if (EfiCompareMem (Languages + Position, LangCode, LC_ISO_639_2_ENTRY_SIZE) == 0) {
        Found = TRUE;
        *UnicodeCollationInterface = Uci;
        goto Done;
      }
    }
  }

Done:
  //
  // Cleanup
  //
  if (Handles) {
    gBS->FreePool (Handles);
  }
  
  if (Found) {
    return EFI_SUCCESS;
  }
  
  return EFI_NOT_FOUND;
}


EFI_STATUS
FatAllocateVolume (
  IN  EFI_HANDLE                Handle,
  IN  EFI_DISK_IO_PROTOCOL      *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL     *BlockIo
  )
/*++

Routine Description:
  Allocates volume structure, detects FAT file system, installs protocol,
  and initialize cache.
  
Arguments:
  Handle    - the handle of parent device
  DiskIo    - the DiskIo of parent device
  BlockIo   - the BlockIo of parent device
  
Returns:
  Status Code

--*/
{
  EFI_STATUS    Status;
  FAT_VOLUME    *Vol;
  UINTN         Index;
  BOOLEAN       LockedByMe;
  UINTN         CachePageSizeInSector;
  CHAR16        VolumeName[80];

  LockedByMe = FALSE;

  //
  // Allocate a volume structure 
  //
  Vol = EfiLibAllocateZeroPool (sizeof(FAT_VOLUME));
  if (Vol == NULL) {
  return EFI_OUT_OF_RESOURCES;
  }
  
  //
  // Acquire the lock.
  // If caller has already acquired the lock, cannot lock it again.
  //
  if (!FatIsLocked()) {
    LockedByMe = TRUE;
    FatAcquireLock ();
  }

  //
  // Initialize the structure
  //
  Vol->Signature = FAT_VOLUME_SIGNATURE;
  Vol->Handle = Handle;
  Vol->DiskIo = DiskIo;
  Vol->BlkIo = BlockIo;
  Vol->MediaId = BlockIo->Media->MediaId;
  Vol->VolInterface.Revision = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
  Vol->VolInterface.OpenVolume = FatOpenVolume;
  InitializeListHead (&Vol->CheckRef);
  Vol->FreeInfoValid = FALSE;

  //
  // Check to see if there's a file system on the volume
  //
  Status = FatOpenDevice (Vol);
  if (EFI_ERROR(Status)) {
    
    //
    // Unlock if locked by myself.
    //
    if (LockedByMe) {
      FatReleaseLock ();
    }

    gBS->FreePool (Vol);
    return Status;
  }

  //
  // Install our protocol interfaces on the device's handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
          &Vol->Handle,
          &gEfiSimpleFileSystemProtocolGuid,  &Vol->VolInterface,
          NULL
          );
  if (EFI_ERROR (Status)) {
    
    //
    // Unlock if locked by myself.
    //
    if (LockedByMe) {
      FatReleaseLock ();
    }

    gBS->FreePool (Vol);
    return Status;
  }
  
  //
  // Volume installed
  //
  DEBUG((EFI_D_INIT, "%HInstalled Fat filesystem on %x%N\n", Handle));
  Vol->Valid = TRUE;
  
  //
  // Initialize cache
  //

  //
  // Determine the proper cache page size
  //
  CachePageSizeInSector = Vol->SectorsPerCluster;

  while ((Vol->Sectors % CachePageSizeInSector != 0) ||
         (CachePageSizeInSector * Vol->SectorSize > FAT_CACHE_PAGE_MAX_SIZE)) {
    CachePageSizeInSector /= 2;
  }
  
  Vol->CachePageSize = CachePageSizeInSector * Vol->SectorSize;

  for (Index = 0; Index < FAT_CACHE_SIZE; Index++) {
    Vol->Cache[Index].Valid = FALSE;
    Vol->Cache[Index].Dirty = FALSE;
    Status = gBS->AllocatePool (
          EfiBootServicesData,
          Vol->CachePageSize,
          &Vol->Cache[Index].Data
          );
    if (EFI_ERROR (Status)) {
    
      //
      // Roll back allocated memories
      //
      if (Index > 0) {
        for (;(INTN)Index >= 0; Index --) {
          gBS->FreePool (&Vol->Cache[Index].Data);
        }
      }
      
      gBS->UninstallMultipleProtocolInterfaces (
          Vol->Handle,
          &gEfiSimpleFileSystemProtocolGuid,  &Vol->VolInterface,
          NULL
          );
      
      //
      // Unlock if locked by myself.
      //
      if (LockedByMe) {
        FatReleaseLock ();
      }
      
      gBS->FreePool (Vol);
      return Status;
    }
  }
  
  //
  // Unlock if locked by myself.
  //
  if (LockedByMe) {
    FatReleaseLock ();
  }

  //
  // Build name for this FAT volume
  //
  Vol->ControllerNameTable = NULL;
  EfiStrCpy (VolumeName, L"FAT File System [FAT");
  switch (Vol->FatType) {
  case FAT12 :
    EfiStrCat (VolumeName, L"12");
    break;
  case FAT16 :
    EfiStrCat (VolumeName, L"16");
    break;
  case FAT32 :
    EfiStrCat (VolumeName, L"32");
    break;
  default :
    EfiStrCat (VolumeName, L"xx");
    break;
  }
  EfiStrCat (VolumeName, L"] ");
  if (Vol->VolSize < 10000000) {
    EfiValueToString (&(VolumeName[EfiStrLen(VolumeName)]),
                       DriverLibRShiftU64(Vol->VolSize,10),
                       0,
                       0
                       );
    EfiStrCat (VolumeName, L" KB");
  } else if (Vol->VolSize < 10000000000) {
    EfiValueToString (&(VolumeName[EfiStrLen(VolumeName)]),
                       DriverLibRShiftU64(Vol->VolSize,20),
                       0,
                       0
                       );
    EfiStrCat (VolumeName, L" MB");
  } else {
    EfiValueToString (&(VolumeName[EfiStrLen(VolumeName)]),
                       DriverLibRShiftU64(Vol->VolSize,30),
                       0,
                       0
                       );
    EfiStrCat (VolumeName, L" GB");
  }

  EfiLibAddUnicodeString (
    "eng", 
    gFatComponentName.SupportedLanguages, 
    &Vol->ControllerNameTable, 
    VolumeName
    );

  return Status;
}


EFI_STATUS
FatAbandonVolume (
  IN FAT_VOLUME *Vol
  )
/*++
  
Routine Description:
  Called by FatDriverBindingStop(), Abandon the volume.
  
Arguments:
  Vol     - the volume to be abandoned
  
Returns:
  Status code.
  
--*/
{
  EFI_STATUS  Status;
  BOOLEAN     LockedByMe;
  
  //
  // Uninstall the protocol interface.
  //
  if (Vol->Handle) {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                    Vol->Handle,
                    &gEfiSimpleFileSystemProtocolGuid,    &Vol->VolInterface,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  LockedByMe = FALSE;
  
  //
  // Acquire the lock.
  // If the caller has already acquired the lock (which
  // means we are in the process of some Fat operation),
  // we can not acquire again.
  //
  if (!FatIsLocked()) {
    LockedByMe = TRUE;
    FatAcquireLock ();
  }
  
  //
  // The volume is still being used. Hence, set error flag for all OFiles still in
  // use. In two cases, we could get here. One is EFI_MEDIA_CHANGED, the other is
  // EFI_NO_MEDIA.
  //
  if (Vol->Root) {
    FatSetVolumeError (
      Vol->Root, 
      Vol->BlkIo->Media->MediaPresent ? EFI_MEDIA_CHANGED : EFI_NO_MEDIA
      );
  }
  
  Vol->Valid = FALSE;
  
  EfiLibFreeUnicodeStringTable (Vol->ControllerNameTable);

  //
  // Release the lock.
  // If locked by me, this means DriverBindingStop is NOT 
  // called within an on-going Fat operation, so we should
  // take responsibility to cleanup and free the volume.
  // Otherwise, the DriverBindingStop is called within an on-going
  // Fat operation, we shouldn't check reference, so just let outer 
  // FatCleanupVolume do the task.
  //
  if (LockedByMe) {
    FatCleanupVolume (Vol, NULL, 0);
    FatReleaseLock ();
  }

  return EFI_SUCCESS;
}


EFI_STATUS
FatOpenDevice (
  IN OUT FAT_VOLUME           *Vol
  )
/*++

Routine Description:
  Detects FAT file system on Disk and set relevant fields of Volume
  
Arguments:
  Vol     - the volume structure

Returns:
  Status Code.
  
--*/
{
  EFI_DISK_IO_PROTOCOL        *DiskIo;
  EFI_BLOCK_IO_PROTOCOL       *BlockIo;
  EFI_BLOCK_IO_MEDIA          *BlkMedia;
  EFI_STATUS                  Status;
  UINT32                      BlockSize;
  UINT32                      MediaId;    
  FAT_BOOT_SECTOR             FatBPB;
  FAT_BOOT_SECTOR_EX          FatBPBEx;
  FAT_VOLUME_TYPE             FatType;
  UINT64                      LastBlock;
  UINTN                       RootDirSectors, RootSize, FatLba, RootLba,
                              FirstClusterLba;
  UINTN                       FsVersion, ExtendedFlags;
  BOOLEAN                     IsFat, ReadOnly;
  //BOOLEAN                     DebugThisOpen;
  
  DiskIo = Vol->DiskIo;
  BlockIo = Vol->BlkIo;
  BlkMedia = BlockIo->Media;
  //DebugThisOpen = FALSE;

  MediaId = BlkMedia->MediaId;
  BlockSize = BlkMedia->BlockSize;
  LastBlock = BlkMedia->LastBlock;
  ReadOnly = BlkMedia->ReadOnly;

  //
  // Read the FAT_BOOT_SECTOR BPB info
  // This is the only part of FAT code that uses parent DiskIo,
  // Others use FatDiskIo which utilizes a Cache.
  //
  Status = DiskIo->ReadDisk(DiskIo, MediaId, 0, sizeof(FatBPBEx), &FatBPBEx);

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_INIT, "FATOpenDevice: read of part_lba failed %r\n", Status));
    goto Done;
  }

  //
  // The sector may be in 1 of 2 forms.  Copy the data to the fat12, fat16
  // layout
  //

  FatType = FatUndefined;
  EfiCopyMem (&FatBPB, &FatBPBEx, sizeof (FatBPB));

  //
  // Unpack BPB to aligned datum
  //

  FsVersion = FatBPBEx.FsVersion;
  ExtendedFlags = FatBPBEx.ExtendedFlags;
  Vol->SectorSize = FatBPB.SectorSize;
  Vol->SectorsPerCluster = FatBPB.SectorsPerCluster;
  Vol->ReservedSectors = FatBPB.ReservedSectors;
  Vol->NoFats = FatBPB.NoFats;
  Vol->Media = FatBPB.Media;
  Vol->ReadOnly = ReadOnly;
  Vol->RootEntries = FatBPB.RootEntries;  
  BlockSize = (UINT32)Vol->SectorSize;

  //
  // Use LargeSectors if Sectors is 0
  //
  Vol->Sectors = FatBPB.Sectors;
  if (!Vol->Sectors) {
   Vol->Sectors = FatBPB.LargeSectors;
  }

  Vol->SectorsPerFat = FatBPB.SectorsPerFat;
  if (!Vol->SectorsPerFat) {
    Vol->SectorsPerFat = FatBPBEx.LargeSectorsPerFat;
    FatType = FAT32;
  }

  //
  // Is boot sector a fat sector?
  // (Note that so far we only know if the sector is FAT32 or not, we don't
  // know if the sector is Fat16 or Fat12 until later when we can compute
  // the volume size)
  //

  IsFat = TRUE;
  if (FatBPB.Ia32Jump[0] != 0xe9  &&
    FatBPB.Ia32Jump[0] != 0xeb  &&
    FatBPB.Ia32Jump[0] != 0x49) {
      IsFat = FALSE;
  }

  if (Vol->ReservedSectors == 0     ||
    Vol->NoFats == 0              ||
    Vol->Sectors == 0) {
      IsFat = FALSE;
  }

  if (Vol->SectorsPerCluster != 1   &&
    Vol->SectorsPerCluster != 2   &&
    Vol->SectorsPerCluster != 4   &&
    Vol->SectorsPerCluster != 8   &&
    Vol->SectorsPerCluster != 16  &&
    Vol->SectorsPerCluster != 32  &&
    Vol->SectorsPerCluster != 64  &&
    Vol->SectorsPerCluster != 128) {
      IsFat = FALSE;
  }

  if (FatType == FAT32  && (Vol->SectorsPerFat == 0 || FsVersion != 0)) {
    IsFat = FALSE;
  }

  if (FatBPB.Media != 0xf0    &&
    FatBPB.Media != 0xf8    &&
    FatBPB.Media != 0xf9    &&
    FatBPB.Media != 0xfb    &&
    FatBPB.Media != 0xfc    &&
    FatBPB.Media != 0xfd    &&
    FatBPB.Media != 0xfe    &&
    FatBPB.Media != 0xff    &&
    // FujitsuFMR
    FatBPB.Media != 0x00    &&
    FatBPB.Media != 0x01    &&
    FatBPB.Media != 0xfa) {
      IsFat = FALSE;
  }

  if (FatType != FAT32 && Vol->RootEntries == 0) {
    IsFat = FALSE;
  }

  //
  // If this is fat32, refuse to mount mirror-disabled volumes
  //
  if (FatType == FAT32 && (ExtendedFlags & 0x80)) {
    IsFat = FALSE;
  }

  //
  // Was it a fat boot sector?
  //
  if (!IsFat) {
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  //
  // Initialize fields the volume information for this FatType
  //

  if (FatType != FAT32) {
    //
    // Unpack fat12, fat16 info
    //
    Vol->RootCluster = 0;

    EfiCopyMem (FatBPBEx.Id, FatBPB.Id, sizeof(FatBPB.Id));
    EfiCopyMem (FatBPBEx.FatLabel, FatBPB.FatLabel, sizeof(FatBPB.FatLabel));
    EfiCopyMem (FatBPBEx.SystemId, FatBPB.SystemId, sizeof(FatBPB.SystemId));

    //
    // fat12 & fat16 fat-entries are 2 bytes
    //
    Vol->FatEntrySize = sizeof(UINT16);

  } else {
    //  
    // Unpack fat32 info
    //
    Vol->RootCluster = FatBPBEx.RootDirFirstCluster;
    Vol->RootEntries = 0;

    //
    // fat12 & fat16 fat-entries are 2 bytes
    //
    Vol->FatEntrySize = sizeof(UINT32);
  }

  //
  // Compute some fat locations
  //
  
  RootDirSectors = 
    ((Vol->RootEntries * sizeof(FAT_DIRECTORY_ENTRY)) + (BlockSize - 1)) / 
    BlockSize;
  RootSize = RootDirSectors * BlockSize;

  FatLba = Vol->ReservedSectors;
  RootLba = Vol->NoFats * Vol->SectorsPerFat + FatLba;
  FirstClusterLba = RootLba + RootDirSectors;

  Vol->VolSize = DriverLibMultU64x32(Vol->Sectors, BlockSize);
  Vol->FatPos = FatLba * BlockSize;
  Vol->FatSize = Vol->SectorsPerFat * BlockSize;
  Vol->RootPos = DriverLibMultU64x32(RootLba, BlockSize);
  Vol->FirstClusterPos = DriverLibMultU64x32(FirstClusterLba, BlockSize);
  Vol->MaxCluster = (Vol->Sectors - FirstClusterLba) / Vol->SectorsPerCluster;
  Vol->ClusterSize = Vol->SectorsPerCluster * BlockSize;

  //
  // If this is not a fat32, determine if it's a fat16 or fat12
  //

  if (FatType != FAT32) {
    if (Vol->MaxCluster >= 65525) {
      Status = EFI_VOLUME_CORRUPTED;
      goto Done;
    }    
    FatType = Vol->MaxCluster < 4085 ? FAT12 : FAT16;
  } else {
    if (Vol->MaxCluster < 65525) {
      Status = EFI_VOLUME_CORRUPTED;
      goto Done;
    }    
  }

  //
  // Get the DirtyValue and NotDirtyValue
  // We should keep the initial value as the NotDirtyValue
  // in case the volume is dirty already
  //
  if (FatType == FAT16) {
    Status = DiskIo->ReadDisk(
            DiskIo, 
            MediaId, 
            Vol->FatPos + 2, 
            2, 
            &Vol->NotDirtyValue
            );
    if (EFI_ERROR(Status)) {
      goto Done;
    }
    Vol->DirtyValue = Vol->NotDirtyValue & 0x7fff;
    
    if (Vol->DirtyValue == Vol->NotDirtyValue) {
      Vol->FatDirty = TRUE;
    }    
  } else if (FatType == FAT32) {
    Status = DiskIo->ReadDisk(
            DiskIo, 
            MediaId, 
            Vol->FatPos + 4, 
            4, 
            &Vol->NotDirtyValue
            );
    if (EFI_ERROR(Status)) {
      goto Done;
    }
    Vol->DirtyValue = Vol->NotDirtyValue & 0xf7ffffff;
    
    if (Vol->DirtyValue == Vol->NotDirtyValue) {
      Vol->FatDirty = TRUE;
    }
  } else {
    Vol->DirtyValue = 0xfff;
    Vol->NotDirtyValue = 0xfff;
  }
  
/*  if (DebugThisOpen) {
    DEBUG ((EFI_D_INIT, "SectorSize.: %x %x\n",  Vol->SectorSize, BlockSize));
    DEBUG ((EFI_D_INIT, "RootSize...: %x\n",  RootSize));
    DEBUG ((EFI_D_INIT, "VolSize....: %lx\n", Vol->VolSize));
    DEBUG ((EFI_D_INIT, "FatPos.....: %x\n",  Vol->FatPos));
    DEBUG ((EFI_D_INIT, "FatSize....: %x\n",  Vol->FatSize));
    DEBUG ((EFI_D_INIT, "RootPos....: %lx\n", Vol->RootPos));
    DEBUG ((EFI_D_INIT, "FirstClust.: %lx\n", Vol->FirstClusterPos));
    DEBUG ((EFI_D_INIT, "MaxCluster.: %x\n",  Vol->MaxCluster));
    DEBUG ((EFI_D_INIT, "ClusterSize: %x\n",  Vol->ClusterSize));
    DEBUG ((EFI_D_INIT, "LastBlock..: %x\n",  LastBlock));

    DEBUG ((EFI_D_INIT, "Ia32Jump...: %x\n",  FatBPBEx.Ia32Jump[0]));
    DEBUG ((EFI_D_INIT, "ReservedSec: %x\n",  Vol->ReservedSectors));

    DEBUG ((EFI_D_INIT, "OemId......: '%.*a'\n", 
          sizeof(FatBPBEx.OemId), FatBPBEx.OemId));
    DEBUG ((EFI_D_INIT, "FatLabel...: '%.*a'\n", 
          sizeof(FatBPBEx.FatLabel), FatBPBEx.FatLabel));
    DEBUG ((EFI_D_INIT, "SystemId...: '%.*a'\n",
          sizeof(FatBPBEx.SystemId), FatBPBEx.SystemId));
  }
*/
  //
  // If present, read the fat hint info
  //

  if (FatType == FAT32 && Vol->FreeInfoValid == FALSE) {

    Vol->FreeInfoPos = FatBPBEx.FsInfoSector * Vol->SectorSize;

    if (Vol->FreeInfoPos) {

      DiskIo->ReadDisk(
            DiskIo, 
            MediaId, 
            Vol->FreeInfoPos, 
            sizeof(FAT_INFO_SECTOR), 
            &Vol->FatInfoSector
            );
      if (Vol->FatInfoSector.Signature          == FAT_INFO_SIGNATURE &&
        Vol->FatInfoSector.InfoBeginSignature == FAT_INFO_BEGIN_SIGNATURE &&
        Vol->FatInfoSector.InfoEndSignature   == FAT_INFO_END_SIGNATURE) {

        if ( 0 <= (INT32)(Vol->FatInfoSector.FreeInfo.ClusterCount)
            && Vol->FatInfoSector.FreeInfo.ClusterCount <= Vol->MaxCluster ) {
          Vol->FreeInfoValid = TRUE;
        }       
        
        if ( 2 > Vol->FatInfoSector.FreeInfo.NextCluster
            || Vol->FatInfoSector.FreeInfo.NextCluster > Vol->MaxCluster + 1 ) {
          Vol->FatInfoSector.FreeInfo.NextCluster = 2;
        }       
          
      } else {
        EfiZeroMem (&Vol->FatInfoSector, sizeof (Vol->FatInfoSector));
        Vol->FreeInfoPos = 0;
      }
    }
  }
  
  //
  // Just make up a FreeInfo.NextCluster for use by allocate cluster
  //
  if ( Vol->FreeInfoValid == FALSE &&
     ( 2 > Vol->FatInfoSector.FreeInfo.NextCluster
       || Vol->FatInfoSector.FreeInfo.NextCluster > Vol->MaxCluster + 1 ) ) {
    Vol->FatInfoSector.FreeInfo.NextCluster = 2;
  }
  
  //
  // We are now in sync with the media id
  //

  Vol->BlockSize = BlockSize;
  Vol->MediaId = MediaId;
  Vol->FatType = FatType;

  Status = EFI_SUCCESS;
Done:
/*  if (DebugThisOpen) {
    DEBUG ((EFI_D_INIT, "FatOpenDevice: done - %r\n", Status));
  }
*/
  if (EFI_ERROR(Status)) {
    if (Vol->Valid) {
      BlockIo->FlushBlocks (BlockIo);
    }
  }
  return Status;
}
