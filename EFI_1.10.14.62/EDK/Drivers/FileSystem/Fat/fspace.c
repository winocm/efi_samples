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

  fspace.c
  
Abstract:

  Routines dealing with disk spaces and FAT table entries

Revision History

--*/

#include "fat.h"

EFI_STATUS
FatShrinkEof (
  IN FAT_OFILE            *OFile
  )
{
  FAT_VOLUME              *Vol;
  EFI_STATUS              Status;
  UINTN                   NewSize, CurSize;
  UINTN                   Cluster, LastCluster;

  Vol = OFile->Vol;
  ASSERT_VOLUME_LOCKED(Vol);

  Status = EFI_SUCCESS;
  NewSize = FatSizeToClusters (OFile->FileSize, Vol->ClusterSize);

  //
  // Find the address of the last cluster
  //

  Cluster = OFile->FileCluster;
  OFile->Dirty = TRUE;
  LastCluster = 0;

  if (NewSize) {

    for (CurSize=0; CurSize < NewSize; CurSize += 1) {
      if (Cluster == FAT_CLUSTER_FREE || 
         Cluster >= FAT_CLUSTER_SPECIAL) {

        DEBUG ((EFI_D_INIT|EFI_D_ERROR, 
               "FatShrinkEof: cluster chain corrupt\n"));
        Status = EFI_VOLUME_CORRUPTED;
        goto Done;
      }

      LastCluster = Cluster;
      Cluster = FatGetFatEntry (Vol, Cluster);
    }

    //
    // Set CurrentCluster == FileCluster 
    // to force a recalculation of Position related stuffs
    //    
    OFile->FileCurrentCluster = OFile->FileCluster;

    OFile->FileLastCluster = LastCluster;
    
    FatSetFatEntry (Vol, LastCluster, (UINTN)FAT_CLUSTER_LAST);

  } else {

    //
    // Check to see if the file is already completely truncated
    //

    if (OFile->FileCluster == FAT_CLUSTER_FREE) {
      return EFI_SUCCESS;
    }

    //
    // The file is being completely truncated.
    //

    OFile->FileCluster = FAT_CLUSTER_FREE;
    OFile->FileCurrentCluster = FAT_CLUSTER_FREE;
    OFile->FileLastCluster = FAT_CLUSTER_FREE;
  }

  //
  // Free the remaining cluster chain
  //

  while (!FAT_END_OF_FAT_CHAIN(Cluster)) {
    if (Cluster == FAT_CLUSTER_FREE || 
       Cluster >= FAT_CLUSTER_SPECIAL) {

      DEBUG ((EFI_D_INIT|EFI_D_ERROR, "FatShrinkEof: cluster chain corrupt\n"));
      Status = EFI_VOLUME_CORRUPTED;
      goto Done;
    }  
    
    LastCluster = Cluster;
    Cluster = FatGetFatEntry (Vol, Cluster);
    FatSetFatEntry (Vol, LastCluster, FAT_CLUSTER_FREE);
  }

Done:
  return Status;
}


EFI_STATUS
FatGrowEof (
  IN FAT_OFILE            *OFile,
  IN UINT64               NewSizeInBytes
  )
{
  FAT_VOLUME              *Vol;
  EFI_STATUS              Status;
  UINTN                   Cluster;
  UINTN                   CurSize, NewSize;
  UINTN                   LastCluster, NewCluster;
  UINTN                   ClusterCount;
  
  Vol = OFile->Vol;
  ASSERT_VOLUME_LOCKED(Vol);

  //
  // If the file is already large enough, do nothing
  //

  CurSize = FatSizeToClusters (OFile->FileSize, Vol->ClusterSize);
  NewSize = FatSizeToClusters (NewSizeInBytes, Vol->ClusterSize);
  if (CurSize >= NewSize) {
    return EFI_SUCCESS;
  }

  //
  // If we haven't found the files last cluster do it now
  //

  if (OFile->FileCluster && !OFile->FileLastCluster) {
    Cluster = OFile->FileCluster;
    ClusterCount = 0;

    while (!FAT_END_OF_FAT_CHAIN(Cluster)) {
      if (Cluster == FAT_CLUSTER_FREE || 
          Cluster >= FAT_CLUSTER_SPECIAL) {

          DEBUG ((EFI_D_INIT|EFI_D_ERROR,
                  "FatGrowEof: cluster chain corrupt\n"));
          return EFI_VOLUME_CORRUPTED;
      }

      ClusterCount += 1;
      OFile->FileLastCluster = Cluster;
      Cluster = FatGetFatEntry (Vol, Cluster);
    }

    if (ClusterCount != CurSize) {
      DEBUG ((EFI_D_INIT|EFI_D_ERROR, 
             "FatGrowEof: cluster chain size does not match file size\n"));
      return EFI_VOLUME_CORRUPTED;
    }

  }

  //
  // Loop until we've allocated enough space
  //

  Status = EFI_SUCCESS;
  LastCluster = OFile->FileLastCluster;

  while (CurSize < NewSize) {
    NewCluster = FatAllocateCluster(Vol);
    if (FAT_END_OF_FAT_CHAIN(NewCluster)) {
      if (LastCluster != FAT_CLUSTER_FREE) {
        FatSetFatEntry (Vol, LastCluster, (UINTN)FAT_CLUSTER_LAST);
        OFile->FileLastCluster = LastCluster;
      }
      return EFI_VOLUME_FULL;
    }

    if (LastCluster) {
      FatSetFatEntry (Vol, LastCluster, NewCluster);
    } else {
      OFile->FileCluster = NewCluster;
      OFile->FileCurrentCluster = NewCluster;
      OFile->Dirty = TRUE;
    }

    LastCluster = NewCluster;
    CurSize += 1;
  }

  // Terminate the cluster list
  FatSetFatEntry (Vol, LastCluster, (UINTN)FAT_CLUSTER_LAST);
  OFile->FileLastCluster = LastCluster;
  
  return Status;
}


UINT64
FatDirSize (
  IN FAT_OFILE            *OFile
  )
{
  FAT_VOLUME              *Vol;
  UINT64                  Size;
  UINTN                   Cluster;

  Vol = OFile->Vol;

  ASSERT_VOLUME_LOCKED(Vol);


  if (OFile->IsFixedRootDir) {
    //
    // This is a fixed root directory.  
    //

    Size = Vol->RootEntries * sizeof(FAT_DIRECTORY_ENTRY);

  } else {

    //
    // Run the cluster chain for the OFile
    //

    Size = 0;
    Cluster = OFile->FileCluster;

    // N.B. ".." directories on some media do not contain a starting
    // cluster.  In the case of "." or ".." we don't need the size anyway.

    if (Cluster) {
      while (!FAT_END_OF_FAT_CHAIN(Cluster)) {
        if (Cluster == FAT_CLUSTER_FREE || 
          Cluster >= FAT_CLUSTER_SPECIAL) {
          DEBUG ((EFI_D_INIT|EFI_D_ERROR, 
                  "FATDirSize: cluster chain corrupt\n"));
          Size = 0;
          break;
        }
  
        Size += Vol->ClusterSize;
        Cluster = FatGetFatEntry(Vol, Cluster);
      }
    }
  }

  return Size;
}

VOID *
FatLoadFatEntry (
  IN FAT_VOLUME       *Vol,
  IN UINTN            Index
  )
{
  UINTN               Pos;
  EFI_STATUS          Status;

  if (Index > (Vol->MaxCluster+1)) {
    Vol->FatEntryBuffer = (UINT32)-1;
    return &Vol->FatEntryBuffer;
  }  

  //
  // Compute buffer position needed
  //

  switch (Vol->FatType) {
  case FAT12:
    Pos = Index * 3 / 2;
    break;

  case FAT16:
    Pos = Index * 2;
    break;

  case FAT32:
    Pos = Index *4;
    break;

  default:
    Vol->FatEntryBuffer = (UINT32)-1;
    return &Vol->FatEntryBuffer;
  }

  //
  // Set the position and read the buffer
  //

  Vol->FatEntryPos = Vol->FatPos + Pos;
  Status = FatDiskIo (Vol,
                      READ_DISK,
                      Vol->FatEntryPos,
                      Vol->FatEntrySize,
                      &Vol->FatEntryBuffer
                      );
  if (EFI_ERROR(Status)) {
    Vol->FatEntryBuffer = (UINT32)-1;
    Vol->DiskError = TRUE;
    return &Vol->FatEntryBuffer;
  }

  return &Vol->FatEntryBuffer;
}


UINTN
FatGetFatEntry(
  IN FAT_VOLUME       *Vol,
  IN UINTN            Index
  )
{
  VOID            *Pos;
  UINT8           *E12;
  UINT16          *E16;
  UINT32          *E32;
  UINTN           Accum;    
  
  Pos = FatLoadFatEntry (Vol, Index);

  if (Index > (Vol->MaxCluster+1)) {
    return (UINTN)-1;
  }  

  switch (Vol->FatType) {
  case FAT12:
    E12 = Pos;
    Accum = E12[0] | (E12[1] << 8);
    Accum = (Index & 1) ? (Accum >> 4) : (Accum & 0xFFF);
    Accum = Accum | ((Accum >= 0xFF7) ? (-1 & ~0xF) : 0);
    break;

  case FAT16:
    E16 = Pos;
    Accum = *E16;
    Accum = Accum | ((Accum >= 0xFFF7) ? (-1 & ~0xF) : 0);
    break;

  case FAT32:
    E32 = Pos;
    Accum = *E32 & 0x0FFFFFFF;
    Accum = Accum | ((Accum >= 0x0FFFFFF7) ? (-1 & ~0xF) : 0);
    break;

  default:
    Accum = (UINTN)-1;
  }

  return Accum;
}


EFI_STATUS
FatSetFatEntry(
  IN FAT_VOLUME       *Vol,
  IN UINTN            Index,
  IN UINTN            Value    
  )
{
  VOID            *Pos;    
  UINT8           *E12;
  UINT16          *E16;
  UINT32          *E32;
  UINTN           Accum;    
  EFI_STATUS      Status;
  UINT64          EntryPos;
  UINTN           OrigVal;

  if (Index < 2) {
    return EFI_VOLUME_CORRUPTED;
  }
  
  Pos = 0;
  Status = EFI_NOT_FOUND;

  OrigVal = FatGetFatEntry (Vol, Index);
  if (Value == FAT_CLUSTER_FREE && OrigVal != FAT_CLUSTER_FREE) {
    Vol->FatInfoSector.FreeInfo.ClusterCount += 1;
    if (Index < Vol->FatInfoSector.FreeInfo.NextCluster) {
      Vol->FatInfoSector.FreeInfo.NextCluster = (UINT32) Index;
    }
  } else if (Value != FAT_CLUSTER_FREE && OrigVal == FAT_CLUSTER_FREE) {
    if (Vol->FatInfoSector.FreeInfo.ClusterCount) {
      Vol->FatInfoSector.FreeInfo.ClusterCount -= 1;
    }
  }

  //
  // Make sure the entry is in memory
  //

  Pos = FatLoadFatEntry (Vol, Index);

  //
  // Update the value
  //

  switch (Vol->FatType) {
  case FAT12:
    E12 = Pos;
    Accum = E12[0] | (E12[1] << 8);
    Value = Value & 0xFFF;

    if (Index & 1) {
      Accum = (Value  << 4) | (Accum & 0xF);
    } else {
      Accum = Value | (Accum & 0xF000);
    }

    E12[0] = (UINT8) (Accum & 0xFF);
    E12[1] = (UINT8) (Accum >> 8);
    break;

  case FAT16:
    E16 = Pos;
    *E16 = (UINT16) Value;
    break;

  case FAT32:
    E32 = Pos;
    *E32 = (*E32 & 0xF0000000) | (UINT32) (Value & 0x0FFFFFFF);
    break;

  default:
    return EFI_VOLUME_CORRUPTED;
    
  }

  //
  // If the volume's dirty bit is not set, set it now
  //

  if (!Vol->FatDirty) {
    Vol->FatDirty = TRUE;
    FatSetVolumeDirty (Vol, TRUE);    
  }

  //
  // Write the updated fat(s) entry value to the volume
  //

  EntryPos = (UINTN) Vol->FatEntryPos;
  for (Index=0; Index < Vol->NoFats; Index++) {
    Status = FatDiskIo (Vol,
                        WRITE_DISK,
                        EntryPos,
                        Vol->FatEntrySize,
                        &Vol->FatEntryBuffer
                        );
    if (EFI_ERROR(Status)) {
      Vol->DiskError = TRUE;
      break;
    }
    EntryPos += Vol->FatSize;
  }

  return Status;
}



UINTN
FatAllocateCluster(
  IN FAT_VOLUME   *Vol
  )
{
  UINTN           Cluster;

  //
  // Start looking at FatFreePos for the next unallocated cluster
  //
  
  if (Vol->DiskError) {
    return (UINTN)FAT_CLUSTER_LAST;
  }
  
  for (; ;) {
    // If the end of the list, return no available cluster
    if (Vol->FatInfoSector.FreeInfo.NextCluster > (Vol->MaxCluster+1)) {
      FatComputeFreeInfo (Vol);
      if (Vol->FatInfoSector.FreeInfo.NextCluster > (Vol->MaxCluster+1)) {
        return (UINTN)FAT_CLUSTER_LAST;
      }      
    }

    Cluster = FatGetFatEntry(Vol, Vol->FatInfoSector.FreeInfo.NextCluster);
    if (Cluster == FAT_CLUSTER_FREE) {
      break;
    }

    // Try the next cluster
    Vol->FatInfoSector.FreeInfo.NextCluster += 1;
  }

  Cluster = Vol->FatInfoSector.FreeInfo.NextCluster;
  Vol->FatInfoSector.FreeInfo.NextCluster += 1;
  return Cluster;
}


VOID
FatComputeFreeInfo (
  IN FAT_VOLUME *Vol
  )
{
  UINTN Index;
  
  //
  // If we don't have valid info, compute it now
  //

  if (!Vol->FreeInfoValid) {
    
    Vol->FreeInfoValid = TRUE;
    Vol->FatInfoSector.FreeInfo.ClusterCount = 0;
    for (Index = Vol->MaxCluster + 1; Index >= 2; Index--) {
      if (Vol->DiskError) {
        break;
      }

      if (FatGetFatEntry(Vol, Index) == FAT_CLUSTER_FREE) {
        Vol->FatInfoSector.FreeInfo.ClusterCount += 1;
        Vol->FatInfoSector.FreeInfo.NextCluster = (UINT32) Index;
      }
    }
    Vol->FatInfoSector.Signature          = FAT_INFO_SIGNATURE;
    Vol->FatInfoSector.InfoBeginSignature = FAT_INFO_BEGIN_SIGNATURE;
    Vol->FatInfoSector.InfoEndSignature   = FAT_INFO_END_SIGNATURE;
  }
}

    
