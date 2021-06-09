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

  rw.c
  
Abstract:

  Functions that perform file read/write

Revision History

--*/

#include "fat.h"


EFI_STATUS
FatOFileRead (
  IN FAT_OFILE             *OFile,
  IN UINT64                Position, 
  IN UINTN                 BufferSize,
  IN VOID                  *UserBuffer
  );

EFI_STATUS
EFIAPI
FatGetPosition (
  IN EFI_FILE  *FHand,
  OUT UINT64   *Position
  )
{
  FAT_IFILE               *IFile;
  FAT_OFILE               *OFile;

  IFile = IFILE_FROM_FHAND(FHand);
  OFile = IFile->OFile;
  
  if (OFile->Error == EFI_NOT_FOUND) {
    return EFI_DEVICE_ERROR;	
  }  


  if (IFile->OFile->DirType != IsFile) {
    return EFI_UNSUPPORTED;
  }

  *Position = IFile->Position;
  return EFI_SUCCESS;
}



EFI_STATUS
EFIAPI
FatSetPosition (
  IN EFI_FILE  *FHand,
  OUT UINT64   Position
  )
{
  FAT_IFILE               *IFile;
  FAT_OFILE               *OFile;

  IFile = IFILE_FROM_FHAND(FHand);
  OFile = IFile->OFile;
  
  if (OFile->Error == EFI_NOT_FOUND) {
    return EFI_DEVICE_ERROR;	
  }
  
  //
  // If this is a directory, we can only set back to position 0
  //

  if ((IFile->OFile->DirType == IsDir) && Position != 0) {
    return EFI_UNSUPPORTED;
  }

  //
  // Set the position
  // 
  if (Position == -1) {
    Position = IFile->OFile->FileSize;
  }

  //
  // Set the position
  //

  IFile->Position = Position;
  return EFI_SUCCESS;
}


EFI_STATUS
FatOFilePosition (
  IN FAT_OFILE            *OFile,
  IN UINT64               Position
  )
// Seek OFile to requested position
// N.B. the requested file position must exist
{
  FAT_VOLUME              *Vol;
  UINTN                   BlockSize, ClusterSize;
  UINTN                   Cluster;
  UINT64                  pos, run;
  UINTN                   Remainder;

  Vol = OFile->Vol;
  BlockSize = Vol->BlockSize;
  ClusterSize = Vol->ClusterSize;

  ASSERT_VOLUME_LOCKED(Vol);

  //
  // If this is the fixed root dir, then compute it's position
  // from it's fixed info in the fat bpb
  //

  if (OFile->IsFixedRootDir) {
    OFile->PosDisk = Vol->RootPos + Position;
    OFile->PosRem = (UINTN) (OFile->FileSize - Position);
    OFile->Position = Position;
    return EFI_SUCCESS;
  }


  //
  // Run the file's cluster chain to find the current position
  // If possible, run from the current cluster rather than
  // start from beginning
  // Assumption: OFile->Position is always consistent with
  // OFile->FileCurrentCluster.
  // OFile->Position is not modified outside this function;
  // OFile->FileCurrentCluster is modified outside this function
  // to be the same as OFile->FileCluster
  // when OFile->FileCluster is updated, so make a check of this
  // and invalidate the original OFile->Position in this case
  //

  if (Position < OFile->Position || 
      OFile->FileCluster == OFile->FileCurrentCluster) {
    pos = 0;
    Cluster = OFile->FileCluster; 
  } else {
    DriverLibDivU64x32(OFile->Position, Vol->ClusterSize, &Remainder);
    pos = OFile->Position - Remainder;
    Cluster = OFile->FileCurrentCluster; 
  }
  
  for (; ;) {
    if (Cluster == FAT_CLUSTER_FREE || 
      (Cluster >= FAT_CLUSTER_SPECIAL)) {
      DEBUG ((EFI_D_INIT|EFI_D_ERROR, "FatOFilePosition:"
                                      " cluster chain corrupt\n"));
      return EFI_VOLUME_CORRUPTED;
    }
    
    if  (pos + ClusterSize > Position) {
      break;
    }

    Cluster = FatGetFatEntry(Vol, Cluster);
    pos += ClusterSize;
  }

  if (Cluster < 2) {
    return EFI_VOLUME_CORRUPTED;
  }
  
  OFile->PosDisk = Vol->FirstClusterPos + 
                   DriverLibMultU64x32(Cluster-2, Vol->ClusterSize) + Position - pos;
  OFile->Position = Position;
  OFile->FileCurrentCluster = Cluster;

  //
  // Compute the number of consecutive clusters in the file
  //

  run = pos + ClusterSize;
  if (!FAT_END_OF_FAT_CHAIN(Cluster)) {
    while (FatGetFatEntry(Vol, Cluster) == Cluster + 1) {
      run += ClusterSize;
      Cluster += 1;
    }
  }

  run = run - Position;
  OFile->PosRem = (run > EFI_MAX_ADDRESS) ? EFI_MAX_ADDRESS : (UINTN) run;
  
  return EFI_SUCCESS;
}

EFI_STATUS
FatIFileReadDir (
  FAT_IFILE               *IFile,
  IN OUT UINTN            *BufferSize,
  OUT VOID                *Buffer
  )
{
  FAT_OFILE               *OFile, *Entry;
  UINT64                  Position;
  EFI_STATUS              Status;   

  //
  // Skip over blank file entries
  //

  Position = IFile->Position;
  OFile = IFile->OFile;

  do {

    Status = FatGetDirOFile (
          OFile,
          &Position,
          &Entry
          );

    if (EFI_ERROR(Status)) {
      break;
    }

    if (Entry->IsBlank) {
      Status = EFI_NOT_FOUND;
      break;
    }

  } while (Entry->DirType != IsFile && Entry->DirType != IsDir);

  //
  // If we have an entry, convert it to the File info struct
  //

  if (!EFI_ERROR(Status)) {
    Status = FatGetFileInfo(OFile->Vol, Entry, BufferSize, Buffer);
  }

  //
  // If success, then update the IFile position
  //

  if (!EFI_ERROR(Status)) {
    IFile->Position = Position;
  }

  //
  // If status is not found, then return a zero length buffer
  //

  if (Status == EFI_NOT_FOUND) {
    *BufferSize = 0;
    Status = EFI_SUCCESS;
  }

  return Status;
}


EFI_STATUS
EFIAPI
FatRead (
  IN EFI_FILE   *FHand,
  IN OUT UINTN  *BufferSize,
  OUT VOID      *Buffer
  )
{
  FAT_IFILE               *IFile;
  FAT_OFILE               *OFile;
  FAT_VOLUME              *Vol;
  EFI_STATUS              Status;
  UINT64                  epos;

  IFile = IFILE_FROM_FHAND(FHand);
  OFile = IFile->OFile;
  Vol   = OFile->Vol;

  //
  //In case of the file is deleted EFI_DEVICE_ERROR should be returned. 
  //
  if (OFile->Error == EFI_NOT_FOUND){	
    return EFI_DEVICE_ERROR;    	
  }
  
  if (OFile->DirType != IsFile && OFile->DirType != IsDir) {
    return EFI_VOLUME_CORRUPTED;
  }

  //
  // If postion is beyond the EOF, then return EFI_DEVICE_ERROR
  //
  if(IFile->Position > OFile->FileSize) {
    return EFI_DEVICE_ERROR;
  }

  //
  // If position is at EOF, then read 0 bytes
  //

  if (IFile->Position == OFile->FileSize) {
    *BufferSize = 0;
    return EFI_SUCCESS;
  }

  FatAcquireLock ();

  //
  // Verify the source file handle isn't in an error state
  //

  Status = OFile->Error;
  if (!Status) {

    //
    // If this is a directory, read the directory entry
    //

    if (OFile->DirType == IsDir) {

      Status = FatIFileReadDir (IFile, BufferSize, Buffer);

    } else {

      //
      // Read from file.
      // If request transactions past the EOF, return EFI_DEVICE_ERROR.
      //

      epos = IFile->Position + *BufferSize;
      if (epos > OFile->FileSize) {
        *BufferSize -= (UINTN) (epos - OFile->FileSize);
      }
      
      Status = FatOFileRead (OFile, IFile->Position, *BufferSize, Buffer);

      if (!EFI_ERROR(Status)) {
        IFile->Position += *BufferSize;
      }
    }
  }
	

  if (Status == EFI_BUFFER_TOO_SMALL) {
    Status = FatCleanupVolume (OFile->Vol, NULL, Status);
  } else {    
    Status = FatCleanupVolume (OFile->Vol, OFile, Status);
  }
  
  FatReleaseLock ();
  
  return Status;
}

EFI_STATUS
FatOFileRead (
  IN FAT_OFILE             *OFile,
  IN UINT64                Position, 
  IN UINTN                 BufferSize,
  IN VOID                  *UserBuffer
  )
// file space must already exist
{
  UINTN               Len;
  INT8                *Buffer;
  EFI_STATUS          Status;
  FAT_VOLUME          *Vol;

  ASSERT_VOLUME_LOCKED (OFile->Vol);

  Vol = OFile->Vol;
  Buffer = UserBuffer;

  //
  // While there is data to write 
  //
  Status = EFI_SUCCESS;
  while (BufferSize) {

    //
    // Seek the OFile to the file position
    //

    Status = FatOFilePosition (OFile, Position);
    if (EFI_ERROR(Status)) {
      break;
    }
   
    //
    // Clip length to block run 
    // 

    Len = (BufferSize > OFile->PosRem) ? OFile->PosRem : BufferSize;

    //
    // Read the data
    //

    Status = FatDiskIo (Vol, READ_DISK, OFile->PosDisk, Len, Buffer);
    if (EFI_ERROR(Status)) {
      break;
    }

    //
    // Data was read
    //

    Position += Len;    
    Buffer += Len;
    BufferSize -= Len;  
  }
  
  return Status;
}



EFI_STATUS
EFIAPI
FatWrite (
  IN EFI_FILE   *FHand,
  IN OUT UINTN  *BufferSize,
  OUT VOID      *Buffer
  )
{
  FAT_IFILE               *IFile;
  FAT_OFILE               *OFile;
  FAT_VOLUME              *Vol;
  EFI_STATUS              Status;

  IFile = IFILE_FROM_FHAND(FHand);
  OFile = IFile->OFile;
  Vol   = OFile->Vol;

  //
  //In case of the file is deleted EFI_DEVICE_ERROR should be reutrned.
  //
  if(OFile->DirType == IsEmpty) {
    return EFI_DEVICE_ERROR;
  }
  
  if (OFile->DirType != IsFile) {
    return EFI_UNSUPPORTED;
  }

  if (IFile->ReadOnly) {
    return EFI_ACCESS_DENIED;
  }

  if (Vol->ReadOnly) {
    return EFI_WRITE_PROTECTED;
  }
  
  if (OFile->Error == EFI_NOT_FOUND){	
    return EFI_DEVICE_ERROR;    	
  }	

  FatAcquireLock ();


  //
  // Verify the source file handle isn't in an error state
  //

  Status = OFile->Error;
  if (!Status) {
    //
    // If write is past eof, grow the file
    //

    Status = FatGrowEof (OFile, IFile->Position + *BufferSize);
    if (!EFI_ERROR(Status)) {

      //
      // Write the requested data to the file
      //

      Status = FatOFileWrite (OFile, IFile->Position, BufferSize, Buffer);
    } else {
      
      //
      // Failed to allocate enough clusters, 0 bytes has been written
      //
      
      *BufferSize = 0;
    }

    if (EFI_ERROR(Status)) {
      FatShrinkEof (OFile);
    } 

    IFile->Position += *BufferSize;
  }
  
  if (Status == EFI_VOLUME_FULL) {
    FatCleanupVolume (OFile->Vol, NULL, 0);
  } else if (EFI_ERROR(Status)) {
    Status = FatCleanupVolume (OFile->Vol, OFile, Status);
  }
  
  //
  // On EFI_SUCCESS case, not calling FatCleanupVolume():
  // 1) The Cache flush operation is avoided to enhance
  // performance. Caller is responsible to call Flush() when necessary.
  // 2) The volume dirty bit is probably set already, and is expected to be
  // cleaned in subsequent Flush() or other operations.
  // 3) Write operation doesn't affect OFile/IFile structure, so 
  // Reference checking is not necessary.
  //
  
  FatReleaseLock ();
  return Status;
}


EFI_STATUS
FatOFileWrite (
  IN FAT_OFILE             *OFile,
  IN UINT64                Position,
  IN OUT UINTN             *DataBufferSize,
  IN VOID                  *UserBuffer
  )
// Write data
// file space must already exist
{
  FAT_VOLUME              *Vol;
  UINT8                   *Buffer;    
  UINTN                   Len;
  EFI_STATUS              Status;
  UINTN                   BufferSize;
  
  BufferSize = *DataBufferSize;
  *DataBufferSize = 0;
  Vol = OFile->Vol;
  Buffer = UserBuffer;
  ASSERT_VOLUME_LOCKED (OFile->Vol);

  //
  // Write there is data write it
  //
  Status = EFI_SUCCESS;
  while (BufferSize) {

    //
    // Seek the OFile to the file position
    //

    Status = FatOFilePosition (OFile, Position);
    if (EFI_ERROR(Status)) {
      break;
    }    

    //
    // Clip length to block run
    //

    Len = ((UINT64) BufferSize > OFile->PosRem) ? OFile->PosRem : BufferSize;

    //
    // Write the data
    //

    Status = FatDiskIo (Vol, WRITE_DISK, OFile->PosDisk, Len, Buffer);
    if (EFI_ERROR(Status)) {
      break;
    }

    //
    // Data was written
    //

    Position += Len;
    Buffer += Len;
    BufferSize -= Len;
    *DataBufferSize += Len;
    OFile->Attributes = (UINT8)(OFile->Attributes | FAT_ATTRIBUTE_ARCHIVE);
    OFile->Dirty = TRUE;

    if (Position > OFile->FileSize) {
      OFile->FileSize = Position;
    }
  }

  return Status;
}
